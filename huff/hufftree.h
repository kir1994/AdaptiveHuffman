#pragma once

#include <fstream>

#include "..\commondf.h"

using namespace std;
/* Начало пространства имен hcp */
namespace hcp
{
  /* Класс для работы с файлами */
  class bitArray                                      
  {                                                   
  private:
    CRITICAL_SECTION cs_;
    UINT64 readNum_;
    UINT64 writeNum_;
    
    BYTE rB_, wB_;
    CHAR rP_, wP_;
    ifstream ifs_;
    ofstream ofs_;
                                 
  public:
    /* Конструктор класса bitArray
     * АРГУМЕНТЫ:
     *   - ничего
     *       (VOID)
     * ВОЗВРАЩАЕТ:
     *   - ничего 
     */
    bitArray( VOID ) :  readNum_(0), writeNum_(0), rB_(0), wB_(0), 
                       rP_(8), wP_(8)
    {
      // Начало работы с критической секцией
      InitializeCriticalSection(&cs_);
    }/* конец конструктора класса bitArray */

    /* Деструктор класса bitArray
     * АРГУМЕНТЫ:
     *   - ничего
     *       (VOID)
     * ВОЗВРАЩАЕТ:
     *   - ничего 
     */
    ~bitArray( VOID )
    {
      // Завершение работы с файлами
      close();
      // Завершение работы с критической секцией
      DeleteCriticalSection(&cs_);
    }/* конец деструктора класса bitArray */

    /* Функция завершение работы с файлами
     * АРГУМЕНТЫ:
     *   - ничего
     *       (VOID)
     * ВОЗВРАЩАЕТ:
     *   - ничего 
     *       (VOID)
     */
    VOID close( VOID )
    {
      // Записываем остаток бит в файл
      if (wP_ != 8)
      {
        ofs_.write((CHAR *)&wB_, 1);
        EnterCriticalSection(&cs_);
        ++writeNum_;
        LeaveCriticalSection(&cs_);
      }
      // Закрываем файловые потоки
      ofs_.close();
      ifs_.close();
    }/* конец функции close */
    
    /* Функция инициализации класса для работы
     * АРГУМЕНТЫ:
     *   - имя входного файла 
     *       CHAR *
     *   - имя выходного файла
     *       CHAR *
     * ВОЗВРАЩАЕТ:
     *   - результат инициализации
     *       (BOOL)
     */
    BOOL init( CHAR *inName, CHAR *outName  )
    {
      // Закрываем потоки
      ifs_.close();
      ofs_.close();
      // Заново открываем их
      ifs_.open(inName, ios::binary);
      // Входной файл не открылся
      if (ifs_.is_open() == FALSE)
        return FALSE;
      ofs_.open(outName, ios::binary);
      // Выходной файл не открылся
      if (ofs_.is_open() == FALSE)
        return FALSE;
      // Сброс счетчиков на начальные значения
      rB_ = wB_ = rP_ = 0;
      wP_ = 8;
      EnterCriticalSection(&cs_);
      readNum_ = 0, writeNum_ = 0;
      LeaveCriticalSection(&cs_);

      // Инициализация прошла успешно
      return TRUE;
    }/* конец функции init */

    /* Функция добавления бита в буфер
     * АРГУМЕНТЫ:
     *   - значение бита
     *       BOOL bit
     * ВОЗВРАЩАЕТ:
     *   - ничего
     *       (VOID)
     */
    VOID addBit( BOOL bit )                  
    {
      // Добавляем бит в буфер
      wB_ |= (bit << --wP_);
      // Если в буфере уже 8 бит, то пишем байт в файл и сбрасываем счетчики буфера
      if (wP_ == 0)
      {
        ofs_.write((CHAR *)&wB_, 1);
        wP_ = 8;
        wB_ = 0;
        EnterCriticalSection(&cs_);
        ++writeNum_;
        LeaveCriticalSection(&cs_);
      } 
    }/* конец функции addBit */

    /* Функция добавления байта в буфер
     * АРГУМЕНТЫ:
     *   - байт
     *       BYTE byte
     * ВОЗВРАЩАЕТ:
     *   - ничего
     *       (VOID)
     */
    VOID addByte( BYTE byte )             
    {
      for (INT i = 7; i >= 0; i--)                 
        addBit((byte & (1 << i)) == 0 ? FALSE: TRUE);
    }/* конец функции addByte */

    /* Функция записи нескольких байт в файл
     * АРГУМЕНТЫ:
     *   - массив байт
     *       BYTE *bytes
     *   - количество байт
     *       UINT num
     * ВОЗВРАЩАЕТ:
     *   - ничего
     *       (VOID)
     */
    VOID write( BYTE *bytes, UINT num )
    {
      ofs_.write((CHAR *)bytes, num);
      EnterCriticalSection(&cs_);
      writeNum_ += num;
      LeaveCriticalSection(&cs_);
    }/* конец функции write */

    /* Функция возврата бита
     * АРГУМЕНТЫ:
     *   - ничего
     *       (VOID)
     * ВОЗВРАЩАЕТ:
     *   - бит
     *       (BOOL)
     */
    BOOL getBit( VOID )                  
    {
      // Если считанный байт закончился, считываем следующий
      if (rP_ == 0)
      {
        // Проверка, есть ли, что считывать еще
        if (ifs_.eof() == TRUE)
          throw exc::EndOfFile();
        ifs_.read((CHAR *)&rB_, 1);
        rP_ = 8;
        EnterCriticalSection(&cs_);
        ++readNum_;
        LeaveCriticalSection(&cs_);
      }

      // Возвращаем очередной бит из считанного байта
      return ((rB_ & (1 << --rP_)) == 0 ? FALSE: TRUE);
    }/* конец функции getBit */

    /* Функция получения байта
     * АРГУМЕНТЫ:
     *   - ничего
     *       (VOID)
     * ВОЗВРАЩАЕТ:
     *   - считанный байт
     *       (BYTE)
     */
    BYTE getByte( VOID )
    {
      // Нечего считывать, конец файла
      if (ifs_.eof() == TRUE)
        throw exc::EndOfFile();
      BYTE b = 0;
      for (INT i = 7; i >= 0; --i)
        b |= getBit() << i;
      
      return b;
    }/* конец функции getByte */

    /* Функция считывания нескольких байт из файла
     * АРГУМЕНТЫ:
     *   - массив байт
     *       BYTE *
     *   - количество считываемых байт
     *       UINT num
     * ВОЗВРАЩАЕТ:
     *   - результат чтения
     *       (BOOL)
     */
    BOOL read( BYTE *bytes, UINT num )
    {
      if (!ifs_.read((CHAR *)bytes, num))
        // Считать не получилось
        return FALSE;
      EnterCriticalSection(&cs_);
      readNum_ += num;
      LeaveCriticalSection(&cs_);
      
      return TRUE;
    }/* конец функции read */
    
    /* Функция получения количества прочитанных байт
     * АРГУМЕНТЫ:
     *   - ничего
     *       (VOID)
     * ВОЗВРАЩАЕТ:
     *   - количество прочитанных байт
     *       (UINT64)
     */
    UINT64 getReadNum( VOID )
    {
      EnterCriticalSection(&cs_);
      UINT64 res = readNum_;
      LeaveCriticalSection(&cs_);
      
      return res;
    }/* конец функции getReadNum */

    /* Функция получения количества записанных байт
     * АРГУМЕНТЫ:
     *   - ничего
     *       (VOID)
     * ВОЗВРАЩАЕТ:
     *   - количество записанных байт
     *       (UINT64)
     */
    UINT64 getWriteNum( VOID )
    {
      EnterCriticalSection(&cs_);
      UINT64 res = writeNum_;
      LeaveCriticalSection(&cs_);
      
      return res;
    }/* конец функции getWriteNum */
  };/* конец класса bitArray */

  /* начало класса bList, хранящего последовательность бит */
  class bList                                     
  {
    // Узел списка
    struct bLNode
    {
      bLNode *next;                               
      bLNode *prev;                               
      BOOL bit;                                   

      bLNode (BOOL b = 0): bit(b), next(NULL), prev(NULL)
      {
      }
    };/* конец структуры bLNode */

    // Голова списка
    bLNode *head;                
    // Хвост списка
    bLNode *tail;                                 

  public:
    /* Конструктор класса bList
     * АРГУМЕНТЫ:
     *   - ничего
     *       (VOID)
     * ВОЗВРАЩАЕТ:
     *   - ничего
     *       (VOID)
     */
    bList( VOID ): head(NULL), tail(NULL) 
    {
    }/* конец конструктора класса bitArray */

    /* Деструктор класса bList
     * АРГУМЕНТЫ:
     *   - ничего
     *       (VOID)
     * ВОЗВРАЩАЕТ:
     *   - ничего
     *       (VOID)
     */
    ~bList( VOID )
    {
      clear();
    }/* конец деструктора класса bList */

     /* Добавление нового бита в конец списка
     * АРГУМЕНТЫ:
     *   - бит
     *       bool b
     * ВОЗВРАЩАЕТ:
     *   - ничего
     *       (VOID)
     */
    VOID addBit( BOOL b )                           
    {
      bLNode *newEl = new bLNode(b);
      if(newEl == NULL)
        throw exc::BadAlloc();
      newEl->prev = tail;
      if (head == NULL)
      {
        head = newEl;
        tail = newEl;
      }
      else
      {
        tail->next = newEl;
        tail = newEl;
      }     
    }/* конец функции addBit */
    
    /* Функция очистки списка, начиная с конца
     * АРГУМЕНТЫ:
     *   - ничего
     *       (VOID)
     * ВОЗВРАЩАЕТ:
     *   - ничего
     *       (VOID)
     */
    VOID clear( VOID )                                      
    {
      bLNode *p = tail;
      if(p != NULL)
      {
        while(p->prev != NULL)
        {
          p = p->prev;
          delete p->next;
        }
        delete p;
        head = NULL;
        tail = NULL;
      }
    }/* конец функции clear */

    /* Функция, записывающая последовательность бит в обратном порядке и очищающая список
     * АРГУМЕНТЫ:
     *   - класс работы с файлами
     *       bitArray& bA
     * ВОЗВРАЩАЕТ:
     *   - ничего
     *       (VOID)
     */
    VOID endCode( bitArray& bA )         
    {
      bLNode* p = tail;
      while (p != NULL)                                // Запись последовательности в файл в обратном порядке
      {
        bA.addBit(p->bit);
        p = p->prev;
      }
      clear();
    }/* конец функции endCode */
  };/* конец класса bList */

  /* начало структуры hTNode, узла/листа дерева Хаффмена */
  struct hTNode                                      
  {
    // Статус узла/листа. 0 - узел, 1 - лист, 2 - ESC-символ, 3 - EOF-символ
    INT isLeafEsc;   
    // Вес узла/листа
    unsigned long key; 
    // Символ листа
    CHAR sym;                                        
    hTNode *next;                                    
    hTNode *prev;                                    
    hTNode *par;                                     
    hTNode *left;                                    
    hTNode *right;                 
    /* Конструктор структуры hTNode
     * АРГУМЕНТЫ:
     *   - вес узла
     *       INT k = 0
     *   - статус узла
     *       INT iLE = 0
     *   - указатель на родителя
     *       hTNode *p = NULL
     *   - указатель на следующий по списку узел
     *       hTNode *n = NULL
     * ВОЗВРАЩАЕТ:
     *   - ничего
     */
    hTNode ( INT k = 0, INT iLE = 0, hTNode *p = NULL, hTNode *n = NULL ): key(k), isLeafEsc(iLE), next(n), par(p),  
                                                                           left(NULL), right(NULL), prev(NULL) {}
    // конец конструктора структуры hTNode
  };/* конец структуры hTNode */

  /* Класс hTree, дерево Хаффмена */
  class hTree
  {
    // Корень дерева
    hTNode *head;          
    // ESC-символ
    hTNode *last;     
    // EOF-символ
    hTNode *eof;
  public:
    /* Конструктор класса hTree
     * АРГУМЕНТЫ:
     *   - ничего
     *       (VOID)
     * ВОЗВРАЩАЕТ:
     *   - ничего
     *       (VOID)
     */
    hTree( VOID )                                                      
    {
      head = new hTNode();   
      if(head == NULL)
        throw exc::BadAlloc();
      head->left = new hTNode(0, 3, head);                       
      head->right = new hTNode(0, 2, head);  
      if(head->right == NULL || head->left ==NULL)
        throw exc::BadAlloc();

      head->next = head->left;

      head->left->next = head->right;
      head->left->par = head;
      head->left->prev = head;

      head->right->par = head;
      head->right->prev = head->left;
      last = head->right;
      eof = head->left;      
    }/* конец конструктора класса hTree */

    /* Деструктор класса bList
     * АРГУМЕНТЫ:
     *   - ничего
     *       (VOID)
     * ВОЗВРАЩАЕТ:
     *   - ничего
     *       (VOID)
     */
    ~hTree( VOID )
    {
      if (head != NULL)
      {
        hTNode * delEl = last;
        while (delEl->prev != NULL)
        {
          delEl = delEl->prev;
          delete delEl->next;
        }
        delete delEl;
        head = NULL;
      }
    }/* конец деструктора класса hTree */

    /* Функция получения EOF-символа
     * АРГУМЕНТЫ:
     *   - ничего
     *       (VOID)
     * ВОЗВРАЩАЕТ:
     *   - указатель на EOF-символ
     *       (hTNode *)
     */
    hTNode * getEOF( VOID )                                              
    {
      return eof;
    }/* конец функции getEOF */

    /* Функция получения корня дерева
     * АРГУМЕНТЫ:
     *   - ничего
     *       (VOID)
     * ВОЗВРАЩАЕТ:
     *   - указатель на корень дерева
     *       (hTNode *)
     */
    hTNode * getHead( VOID )                                            
    {
      return head;
    }/* конец функции getHead */

    /* Функция добавления символа в дерево
     * АРГУМЕНТЫ:
     *   - символ
     *       CHAR c
     * ВОЗВРАЩАЕТ:
     *   - указатель на созданный лист
     *       (hTNode *)
     */
    hTNode * addSym( CHAR c )                                           
    {
      // Создаем узел и лист
      hTNode *newLeaf = new hTNode(0, 1);                         
      hTNode *newNode = new hTNode;   
      if(newLeaf == NULL || newNode == NULL)
        throw exc::BadAlloc();

      // ESC-символ и новый лист - поддеревья нового узла
      if (last == head)
        head = newNode;                                                                                                        
      newLeaf->sym = c;                                          
      newLeaf->next = last;
      newLeaf->par = newNode;
      newLeaf->prev = newNode;
      newNode->next = newLeaf;
      newNode->left = newLeaf;
      newNode->right = last;
      newNode->par = last->par;
      newNode->prev = last->prev;

      if (last->prev != NULL)                                     
        last->prev->next = newNode;
      if (last->par != NULL)
      {
        if(last->par->left == last)
          last->par->left = newNode;
        else
          last->par->right = newNode;
      }
      last->par = newNode;                                  
      last->prev = newLeaf;

      return newLeaf;
    }/* конец функции addSym */

    /* Функция поиска символа в дереве
     * АРГУМЕНТЫ:
     *   - символ
     *       CHAR c
     *   - результат поиска
     *       htNode **result
     * ВОЗВРАЩАЕТ:
     *   - статус поиска
     *       (BOOL)
     */
    BOOL symSearch( CHAR c, hTNode **result )                              
    {
      hTNode **res = &head;
      while (*res != NULL)
      {
        if ((**res).isLeafEsc == 1 && (**res).sym == c)   
        {
          *result = *res;
          return TRUE;
        }
        res = &((**res).next);
      }
      return FALSE;
    }/* конец функции symSearch */

    /* Функция поиска символа по считываемому коду
     * АРГУМЕНТЫ:
     *   - класс работы с файлами
     *       bitArray& bA
     * ВОЗВРАЩАЕТ:
     *   - указатель на найденный символ
     *       (hTNode **)
     */
    hTNode ** codeSearch(bitArray& bA)      
    {
      hTNode ** sym = &head;

      while ((**sym).left != NULL)
      {
        if (bA.getBit() == 1)                        
          sym = &((**sym).left);                         
        else
          sym = &((**sym).right);
      }

      return sym;
    }/* конец функции codeSearch */

    friend hTNode * symEncode( bitArray& bA, hTree& t, CHAR c );                 
    friend INT symDecode( bitArray& bA, hTree& t, CHAR &c );  
    friend VOID treeRefresh( hTree& t, CHAR c );                                            
  };
}/* Конец пространства имен hcp */
