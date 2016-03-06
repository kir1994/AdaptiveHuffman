#include "huff.h"

/* Реализация адаптивного алгоритма кодирования Хаффмена
 * АРГУМЕНТЫ:
 *   - класс для работы с файлами
 *       bitArray &bA
 *   - переменная критической секции для синхронизации потоков
 *       CRITICAL_SECTION &susp;
 * ВОЗВРАЩАЕТ:
 *   - ничего
 *       (VOID);
 */
VOID huffEncode( bitArray &bA, CRITICAL_SECTION &susp )        
{
  // Инициализация дерева Хаффмена
  hTree t;                                                         

  BYTE c;                                       

  // Читаем файл посимвольно, пока не достигнут конец файла
  while (bA.read(&c, 1) == TRUE)                           
  {
    if (TryEnterCriticalSection(&susp) == 0)
      return;
    // Кодирование прочитанного символа
    symEncode(bA, t, c);   
    // Обновление дерева прочитанным символом
    treeRefresh(t, c);                                             
    LeaveCriticalSection(&susp);
  }

  bList bL;

  // Записываем в выходной файл код EOF символа
  hTNode *el = t.getEOF();                                       

  EnterCriticalSection(&susp);
  while (el != t.getHead())                                       
  {
    if (el == el->par->left)                                      
      bL.addBit(1);
    else
      bL.addBit(0);
    el = el->par;
  }
  bL.endCode(bA);
  LeaveCriticalSection(&susp);                                            
} /* Конец функции huffEncode */

/* Функция кодирования символа по дереву Хаффмена
 * АРГУМЕНТЫ:
 *   - класс для работы с файлами
 *       bitArray &bA
 *   - дерево Хаффмена
 *       hTree& t
 *   - символ
 *       CHAR c
 * ВОЗВРАЩАЕТ:
 *   - указатель на узел дерева Хаффмена
 *       (hTNode *);
 */
hTNode * hcp::symEncode( bitArray& bA, hTree& t, CHAR c ) 
{
  bList bL;
  hTNode *el;
  // Ищем символ в дереве
  if (t.symSearch(c, &el) == TRUE)                                  
  {
    // Нашли узел, бежим от него к корню дерева, собирая биты. 
    // Если текущий узел - левое поддерево родителя, тогда добавляем 1, иначе - 0
    hTNode tempEl = *el;
    hTNode *retEl = &tempEl;
    while (el != t.head)                                            
    {
      if (el == el->par->left)                                      
        bL.addBit(1);
      else
        bL.addBit(0);
      el = el->par;
    }
    // Считали последовательность бит, теперь её надо записать в обратном порядке
    bL.endCode(bA);                                           
    return retEl;
  }
  else                                                     
  {
    // Символа в дереве не нашли

    // Переходим к ESC-символу, от него бежим к вершине, собирая биты
    hTNode *el = t.last;                                           
    while (el != t.head)                                            
    {
      if (el == el->par->left)                                      
        bL.addBit(1);
      else
        bL.addBit(0);
      el = el->par;
    }
    // Считали последовательность бит, теперь её надо записать в обратном порядке
    bL.endCode(bA);            
    // Этот символ встретился в первый раз - надо записать его без кодирования
    bA.addByte(c);                                             

    // Добавляем символ в дерево Хаффмена
    return t.addSym(c);                                                   
  }
}/* Конец функции symEncode */

/* Функция обновления дерева Хаффмена
 * АРГУМЕНТЫ:
 *   - дерево Хаффмена
 *       hTree& t
 *   - символ
 *       CHAR c
 * ВОЗВРАЩАЕТ:
 *   - ничего
 *       (VOID);
 */
VOID hcp::treeRefresh( hTree& t, CHAR c )                            
{
  hTNode *tmp;
  // Ищем символ в дереве. Он точно есть, так как был добавлен в предыдущем шаге алгоритма
  t.symSearch(c, &tmp);                                                               
  // Выполняем цикл, пока не достигнут корень дерева
  while (tmp != t.head)                                          
  {
    // Если предыдущий элемент имеет такой же вес, как и текущий, и не корень дерева, то выполняем
    if((*tmp).prev->key == (*tmp).key && (*tmp).prev != t.head)  
    {
      hTNode * chEl = ((*tmp).prev);        
      // Если элемент встречается впервые - то переставлять его будем с EOF-символом
      if ((*tmp).key == 0 && (*tmp).isLeafEsc == 1)
        while ((*chEl).isLeafEsc != 3)
          chEl = ((*chEl).prev);
      else
      {
        // Ищем минимальный элемент с таким же весом, и не являющийся головой дерева
        while ((*chEl).prev !=  t.head && ((*chEl).prev)->key == (*tmp).key)   
          chEl = ((*chEl).prev);
        {
        hTNode * tempEl = tmp->par;
        BOOL f = TRUE;
        // Проверка, находится ли наш элемент в поддереве того, с которым его мы будем переставлять
        while (tempEl != t.head && f == TRUE)
        {
          if (tempEl == chEl)
            chEl = chEl->next, f = FALSE;
          else
            tempEl = tempEl->par;
        }
        }
      // Если надо переставить с родителем, то пробуем менять со следующий в списке после родителя
      if (chEl == tmp->par)
        chEl = chEl->next;
      }
      // Если мы не пытаемся совершить перестановку узла самого с собой
      if (chEl != tmp)                                             
      {
        hTNode exEl = *tmp;           
        // Перестановка узлов
        // Если переставляем поддеревья одного родителя
        if (tmp->par == chEl->par)                                 
        {
          if (tmp->par->left == tmp)
            tmp->par->left = chEl, tmp->par->right = tmp;
          else
            tmp->par->right = chEl, tmp->par->left = tmp;
        }
        // Если переставляем не поддеревья одного родителя
        else
        {
          if ((*tmp).par->left == tmp)
            (*tmp).par->left = chEl;
          else
            (*tmp).par->right = chEl;

          if ((*chEl).par->left == chEl)
            (*chEl).par->left = tmp;
          else
            (*chEl).par->right = tmp;

          (*tmp).par = (*chEl).par;
          (*chEl).par = (exEl).par;
        }
        // Выправление указателей для сохранения упорядоченного списка
        // Обработка случая, когда текущий элемент является следующим в списке после того, с которым переставляем
        if (chEl->next == tmp)                                                 
          tmp->next = chEl;
        else
          (*tmp).next = (*chEl).next;
        (*tmp).prev = (*chEl).prev;
        (*tmp).prev->next = tmp;
        (*tmp).next->prev = tmp;      
        (*chEl).next = (exEl).next;   
        // Обработка случая, когда текущий элемент является следующим в списке после того, с которым переставляем
        if ((exEl).prev == chEl)
          chEl->prev = tmp;
        else
          (*chEl).prev = (exEl).prev;
        (*chEl).prev->next = chEl;
        (*chEl).next->prev = chEl; 
      }
    }
    // Увеличиваем вес корня на 1
    (*tmp).key++;     
    // Переходим к родителю текущего узла
    tmp = ((*tmp).par);                                          
  }
  // Увеличиваем вес вершины на 1
  (*tmp).key++;                                                   
}/* конец функции treeRefresh */

/* Функция реализации адаптивного алгоритма декодирования Хаффмена
 * АРГУМЕНТЫ:
 *   - класс для работы с файлами
 *       bitArray &bA
 *   - переменная критической секции для синхронизации потоков
 *       CRITICAL_SECTION &susp;
 * ВОЗВРАЩАЕТ:
 *   - ничего
 *       (VOID);
 */
VOID huffDecode( bitArray &bA, CRITICAL_SECTION &susp )        
{
  // Инициализация дерева Хаффмена
  hTree t;

  CHAR c;

  // Декодируем первый символ
  BOOL inc = symDecode(bA, t, c);
  // Пока есть, что декодировать
  while (inc != 0)                         
  {
    if (TryEnterCriticalSection(&susp) == 0)
      return;
    // Обновление дерева считанным символом
    treeRefresh(t, c);       
    // Декодируем следующий символ
    inc = symDecode(bA, t, c);
    LeaveCriticalSection(&susp);
  }
}/* конец функции huffDecode */

/* Функция декодирования символа по дереву Хаффмена
 * АРГУМЕНТЫ:
 *   - класс для работы с файлами
 *       bitArray &bA
 *   - дерево Хаффмена
 *       hTree &t
 *   - символ, который будем декодировать
 *       CHAR &c
 * ВОЗВРАЩАЕТ:
 *   - код завершения декодирования. 1 - всё ок. 0 - заканчиваем декодирование
 *       (INT);
 */
INT hcp::symDecode( bitArray &bA, hTree &t, CHAR &c)   
{
  hTNode **sym;
  try
  {
    // Находим узел по коду, считанному из файла
    sym = t.codeSearch(bA);         
    // Если найденный узел - ESC-символ, то считываем следующий символ из файла в переменную-параметр функции
    // Полученный символ записываем в выходной файл
    if ((**sym).isLeafEsc == 2)                                                      
    {
      c = bA.getByte();
      bA.write((BYTE *)&c, 1);
      // Добавляем символ в дерево Хаффмена
      t.addSym(c);
    }
    // Если найденный узел - лист дерева, то записываем символ из листа в выходной файл
    // Также записываем этот символ в переменную-параметр функции
    else if ((**sym).isLeafEsc == 1)                                                 
    {
      bA.write((BYTE *)&(**sym).sym, 1);
      c = (**sym).sym;
    }
    // Если найденный узел - EOF-символ, то конец декодирования
    else if ((**sym).isLeafEsc == 3)                                                 
      return 0;

    return 1;
  }
  // Возникли непредвиденные ошибки, заканчиваем декодирование
  catch(...)                                                                        
  {
    return 0;
  }
}/* конец функции huffDecode */