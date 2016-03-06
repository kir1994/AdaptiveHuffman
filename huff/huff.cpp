#include "huff.h"

/* ���������� ����������� ��������� ����������� ��������
 * ���������:
 *   - ����� ��� ������ � �������
 *       bitArray &bA
 *   - ���������� ����������� ������ ��� ������������� �������
 *       CRITICAL_SECTION &susp;
 * ����������:
 *   - ������
 *       (VOID);
 */
VOID huffEncode( bitArray &bA, CRITICAL_SECTION &susp )        
{
  // ������������� ������ ��������
  hTree t;                                                         

  BYTE c;                                       

  // ������ ���� �����������, ���� �� ��������� ����� �����
  while (bA.read(&c, 1) == TRUE)                           
  {
    if (TryEnterCriticalSection(&susp) == 0)
      return;
    // ����������� ������������ �������
    symEncode(bA, t, c);   
    // ���������� ������ ����������� ��������
    treeRefresh(t, c);                                             
    LeaveCriticalSection(&susp);
  }

  bList bL;

  // ���������� � �������� ���� ��� EOF �������
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
} /* ����� ������� huffEncode */

/* ������� ����������� ������� �� ������ ��������
 * ���������:
 *   - ����� ��� ������ � �������
 *       bitArray &bA
 *   - ������ ��������
 *       hTree& t
 *   - ������
 *       CHAR c
 * ����������:
 *   - ��������� �� ���� ������ ��������
 *       (hTNode *);
 */
hTNode * hcp::symEncode( bitArray& bA, hTree& t, CHAR c ) 
{
  bList bL;
  hTNode *el;
  // ���� ������ � ������
  if (t.symSearch(c, &el) == TRUE)                                  
  {
    // ����� ����, ����� �� ���� � ����� ������, ������� ����. 
    // ���� ������� ���� - ����� ��������� ��������, ����� ��������� 1, ����� - 0
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
    // ������� ������������������ ���, ������ � ���� �������� � �������� �������
    bL.endCode(bA);                                           
    return retEl;
  }
  else                                                     
  {
    // ������� � ������ �� �����

    // ��������� � ESC-�������, �� ���� ����� � �������, ������� ����
    hTNode *el = t.last;                                           
    while (el != t.head)                                            
    {
      if (el == el->par->left)                                      
        bL.addBit(1);
      else
        bL.addBit(0);
      el = el->par;
    }
    // ������� ������������������ ���, ������ � ���� �������� � �������� �������
    bL.endCode(bA);            
    // ���� ������ ���������� � ������ ��� - ���� �������� ��� ��� �����������
    bA.addByte(c);                                             

    // ��������� ������ � ������ ��������
    return t.addSym(c);                                                   
  }
}/* ����� ������� symEncode */

/* ������� ���������� ������ ��������
 * ���������:
 *   - ������ ��������
 *       hTree& t
 *   - ������
 *       CHAR c
 * ����������:
 *   - ������
 *       (VOID);
 */
VOID hcp::treeRefresh( hTree& t, CHAR c )                            
{
  hTNode *tmp;
  // ���� ������ � ������. �� ����� ����, ��� ��� ��� �������� � ���������� ���� ���������
  t.symSearch(c, &tmp);                                                               
  // ��������� ����, ���� �� ��������� ������ ������
  while (tmp != t.head)                                          
  {
    // ���� ���������� ������� ����� ����� �� ���, ��� � �������, � �� ������ ������, �� ���������
    if((*tmp).prev->key == (*tmp).key && (*tmp).prev != t.head)  
    {
      hTNode * chEl = ((*tmp).prev);        
      // ���� ������� ����������� ������� - �� ������������ ��� ����� � EOF-��������
      if ((*tmp).key == 0 && (*tmp).isLeafEsc == 1)
        while ((*chEl).isLeafEsc != 3)
          chEl = ((*chEl).prev);
      else
      {
        // ���� ����������� ������� � ����� �� �����, � �� ���������� ������� ������
        while ((*chEl).prev !=  t.head && ((*chEl).prev)->key == (*tmp).key)   
          chEl = ((*chEl).prev);
        {
        hTNode * tempEl = tmp->par;
        BOOL f = TRUE;
        // ��������, ��������� �� ��� ������� � ��������� ����, � ������� ��� �� ����� ������������
        while (tempEl != t.head && f == TRUE)
        {
          if (tempEl == chEl)
            chEl = chEl->next, f = FALSE;
          else
            tempEl = tempEl->par;
        }
        }
      // ���� ���� ����������� � ���������, �� ������� ������ �� ��������� � ������ ����� ��������
      if (chEl == tmp->par)
        chEl = chEl->next;
      }
      // ���� �� �� �������� ��������� ������������ ���� ������ � �����
      if (chEl != tmp)                                             
      {
        hTNode exEl = *tmp;           
        // ������������ �����
        // ���� ������������ ���������� ������ ��������
        if (tmp->par == chEl->par)                                 
        {
          if (tmp->par->left == tmp)
            tmp->par->left = chEl, tmp->par->right = tmp;
          else
            tmp->par->right = chEl, tmp->par->left = tmp;
        }
        // ���� ������������ �� ���������� ������ ��������
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
        // ����������� ���������� ��� ���������� �������������� ������
        // ��������� ������, ����� ������� ������� �������� ��������� � ������ ����� ����, � ������� ������������
        if (chEl->next == tmp)                                                 
          tmp->next = chEl;
        else
          (*tmp).next = (*chEl).next;
        (*tmp).prev = (*chEl).prev;
        (*tmp).prev->next = tmp;
        (*tmp).next->prev = tmp;      
        (*chEl).next = (exEl).next;   
        // ��������� ������, ����� ������� ������� �������� ��������� � ������ ����� ����, � ������� ������������
        if ((exEl).prev == chEl)
          chEl->prev = tmp;
        else
          (*chEl).prev = (exEl).prev;
        (*chEl).prev->next = chEl;
        (*chEl).next->prev = chEl; 
      }
    }
    // ����������� ��� ����� �� 1
    (*tmp).key++;     
    // ��������� � �������� �������� ����
    tmp = ((*tmp).par);                                          
  }
  // ����������� ��� ������� �� 1
  (*tmp).key++;                                                   
}/* ����� ������� treeRefresh */

/* ������� ���������� ����������� ��������� ������������� ��������
 * ���������:
 *   - ����� ��� ������ � �������
 *       bitArray &bA
 *   - ���������� ����������� ������ ��� ������������� �������
 *       CRITICAL_SECTION &susp;
 * ����������:
 *   - ������
 *       (VOID);
 */
VOID huffDecode( bitArray &bA, CRITICAL_SECTION &susp )        
{
  // ������������� ������ ��������
  hTree t;

  CHAR c;

  // ���������� ������ ������
  BOOL inc = symDecode(bA, t, c);
  // ���� ����, ��� ������������
  while (inc != 0)                         
  {
    if (TryEnterCriticalSection(&susp) == 0)
      return;
    // ���������� ������ ��������� ��������
    treeRefresh(t, c);       
    // ���������� ��������� ������
    inc = symDecode(bA, t, c);
    LeaveCriticalSection(&susp);
  }
}/* ����� ������� huffDecode */

/* ������� ������������� ������� �� ������ ��������
 * ���������:
 *   - ����� ��� ������ � �������
 *       bitArray &bA
 *   - ������ ��������
 *       hTree &t
 *   - ������, ������� ����� ������������
 *       CHAR &c
 * ����������:
 *   - ��� ���������� �������������. 1 - �� ��. 0 - ����������� �������������
 *       (INT);
 */
INT hcp::symDecode( bitArray &bA, hTree &t, CHAR &c)   
{
  hTNode **sym;
  try
  {
    // ������� ���� �� ����, ���������� �� �����
    sym = t.codeSearch(bA);         
    // ���� ��������� ���� - ESC-������, �� ��������� ��������� ������ �� ����� � ����������-�������� �������
    // ���������� ������ ���������� � �������� ����
    if ((**sym).isLeafEsc == 2)                                                      
    {
      c = bA.getByte();
      bA.write((BYTE *)&c, 1);
      // ��������� ������ � ������ ��������
      t.addSym(c);
    }
    // ���� ��������� ���� - ���� ������, �� ���������� ������ �� ����� � �������� ����
    // ����� ���������� ���� ������ � ����������-�������� �������
    else if ((**sym).isLeafEsc == 1)                                                 
    {
      bA.write((BYTE *)&(**sym).sym, 1);
      c = (**sym).sym;
    }
    // ���� ��������� ���� - EOF-������, �� ����� �������������
    else if ((**sym).isLeafEsc == 3)                                                 
      return 0;

    return 1;
  }
  // �������� �������������� ������, ����������� �������������
  catch(...)                                                                        
  {
    return 0;
  }
}/* ����� ������� huffDecode */