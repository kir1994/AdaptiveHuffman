#pragma once

#include <fstream>

#include "..\commondf.h"

using namespace std;
/* ������ ������������ ���� hcp */
namespace hcp
{
  /* ����� ��� ������ � ������� */
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
    /* ����������� ������ bitArray
     * ���������:
     *   - ������
     *       (VOID)
     * ����������:
     *   - ������ 
     */
    bitArray( VOID ) :  readNum_(0), writeNum_(0), rB_(0), wB_(0), 
                       rP_(8), wP_(8)
    {
      // ������ ������ � ����������� �������
      InitializeCriticalSection(&cs_);
    }/* ����� ������������ ������ bitArray */

    /* ���������� ������ bitArray
     * ���������:
     *   - ������
     *       (VOID)
     * ����������:
     *   - ������ 
     */
    ~bitArray( VOID )
    {
      // ���������� ������ � �������
      close();
      // ���������� ������ � ����������� �������
      DeleteCriticalSection(&cs_);
    }/* ����� ����������� ������ bitArray */

    /* ������� ���������� ������ � �������
     * ���������:
     *   - ������
     *       (VOID)
     * ����������:
     *   - ������ 
     *       (VOID)
     */
    VOID close( VOID )
    {
      // ���������� ������� ��� � ����
      if (wP_ != 8)
      {
        ofs_.write((CHAR *)&wB_, 1);
        EnterCriticalSection(&cs_);
        ++writeNum_;
        LeaveCriticalSection(&cs_);
      }
      // ��������� �������� ������
      ofs_.close();
      ifs_.close();
    }/* ����� ������� close */
    
    /* ������� ������������� ������ ��� ������
     * ���������:
     *   - ��� �������� ����� 
     *       CHAR *
     *   - ��� ��������� �����
     *       CHAR *
     * ����������:
     *   - ��������� �������������
     *       (BOOL)
     */
    BOOL init( CHAR *inName, CHAR *outName  )
    {
      // ��������� ������
      ifs_.close();
      ofs_.close();
      // ������ ��������� ��
      ifs_.open(inName, ios::binary);
      // ������� ���� �� ��������
      if (ifs_.is_open() == FALSE)
        return FALSE;
      ofs_.open(outName, ios::binary);
      // �������� ���� �� ��������
      if (ofs_.is_open() == FALSE)
        return FALSE;
      // ����� ��������� �� ��������� ��������
      rB_ = wB_ = rP_ = 0;
      wP_ = 8;
      EnterCriticalSection(&cs_);
      readNum_ = 0, writeNum_ = 0;
      LeaveCriticalSection(&cs_);

      // ������������� ������ �������
      return TRUE;
    }/* ����� ������� init */

    /* ������� ���������� ���� � �����
     * ���������:
     *   - �������� ����
     *       BOOL bit
     * ����������:
     *   - ������
     *       (VOID)
     */
    VOID addBit( BOOL bit )                  
    {
      // ��������� ��� � �����
      wB_ |= (bit << --wP_);
      // ���� � ������ ��� 8 ���, �� ����� ���� � ���� � ���������� �������� ������
      if (wP_ == 0)
      {
        ofs_.write((CHAR *)&wB_, 1);
        wP_ = 8;
        wB_ = 0;
        EnterCriticalSection(&cs_);
        ++writeNum_;
        LeaveCriticalSection(&cs_);
      } 
    }/* ����� ������� addBit */

    /* ������� ���������� ����� � �����
     * ���������:
     *   - ����
     *       BYTE byte
     * ����������:
     *   - ������
     *       (VOID)
     */
    VOID addByte( BYTE byte )             
    {
      for (INT i = 7; i >= 0; i--)                 
        addBit((byte & (1 << i)) == 0 ? FALSE: TRUE);
    }/* ����� ������� addByte */

    /* ������� ������ ���������� ���� � ����
     * ���������:
     *   - ������ ����
     *       BYTE *bytes
     *   - ���������� ����
     *       UINT num
     * ����������:
     *   - ������
     *       (VOID)
     */
    VOID write( BYTE *bytes, UINT num )
    {
      ofs_.write((CHAR *)bytes, num);
      EnterCriticalSection(&cs_);
      writeNum_ += num;
      LeaveCriticalSection(&cs_);
    }/* ����� ������� write */

    /* ������� �������� ����
     * ���������:
     *   - ������
     *       (VOID)
     * ����������:
     *   - ���
     *       (BOOL)
     */
    BOOL getBit( VOID )                  
    {
      // ���� ��������� ���� ����������, ��������� ���������
      if (rP_ == 0)
      {
        // ��������, ���� ��, ��� ��������� ���
        if (ifs_.eof() == TRUE)
          throw exc::EndOfFile();
        ifs_.read((CHAR *)&rB_, 1);
        rP_ = 8;
        EnterCriticalSection(&cs_);
        ++readNum_;
        LeaveCriticalSection(&cs_);
      }

      // ���������� ��������� ��� �� ���������� �����
      return ((rB_ & (1 << --rP_)) == 0 ? FALSE: TRUE);
    }/* ����� ������� getBit */

    /* ������� ��������� �����
     * ���������:
     *   - ������
     *       (VOID)
     * ����������:
     *   - ��������� ����
     *       (BYTE)
     */
    BYTE getByte( VOID )
    {
      // ������ ���������, ����� �����
      if (ifs_.eof() == TRUE)
        throw exc::EndOfFile();
      BYTE b = 0;
      for (INT i = 7; i >= 0; --i)
        b |= getBit() << i;
      
      return b;
    }/* ����� ������� getByte */

    /* ������� ���������� ���������� ���� �� �����
     * ���������:
     *   - ������ ����
     *       BYTE *
     *   - ���������� ����������� ����
     *       UINT num
     * ����������:
     *   - ��������� ������
     *       (BOOL)
     */
    BOOL read( BYTE *bytes, UINT num )
    {
      if (!ifs_.read((CHAR *)bytes, num))
        // ������� �� ����������
        return FALSE;
      EnterCriticalSection(&cs_);
      readNum_ += num;
      LeaveCriticalSection(&cs_);
      
      return TRUE;
    }/* ����� ������� read */
    
    /* ������� ��������� ���������� ����������� ����
     * ���������:
     *   - ������
     *       (VOID)
     * ����������:
     *   - ���������� ����������� ����
     *       (UINT64)
     */
    UINT64 getReadNum( VOID )
    {
      EnterCriticalSection(&cs_);
      UINT64 res = readNum_;
      LeaveCriticalSection(&cs_);
      
      return res;
    }/* ����� ������� getReadNum */

    /* ������� ��������� ���������� ���������� ����
     * ���������:
     *   - ������
     *       (VOID)
     * ����������:
     *   - ���������� ���������� ����
     *       (UINT64)
     */
    UINT64 getWriteNum( VOID )
    {
      EnterCriticalSection(&cs_);
      UINT64 res = writeNum_;
      LeaveCriticalSection(&cs_);
      
      return res;
    }/* ����� ������� getWriteNum */
  };/* ����� ������ bitArray */

  /* ������ ������ bList, ��������� ������������������ ��� */
  class bList                                     
  {
    // ���� ������
    struct bLNode
    {
      bLNode *next;                               
      bLNode *prev;                               
      BOOL bit;                                   

      bLNode (BOOL b = 0): bit(b), next(NULL), prev(NULL)
      {
      }
    };/* ����� ��������� bLNode */

    // ������ ������
    bLNode *head;                
    // ����� ������
    bLNode *tail;                                 

  public:
    /* ����������� ������ bList
     * ���������:
     *   - ������
     *       (VOID)
     * ����������:
     *   - ������
     *       (VOID)
     */
    bList( VOID ): head(NULL), tail(NULL) 
    {
    }/* ����� ������������ ������ bitArray */

    /* ���������� ������ bList
     * ���������:
     *   - ������
     *       (VOID)
     * ����������:
     *   - ������
     *       (VOID)
     */
    ~bList( VOID )
    {
      clear();
    }/* ����� ����������� ������ bList */

     /* ���������� ������ ���� � ����� ������
     * ���������:
     *   - ���
     *       bool b
     * ����������:
     *   - ������
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
    }/* ����� ������� addBit */
    
    /* ������� ������� ������, ������� � �����
     * ���������:
     *   - ������
     *       (VOID)
     * ����������:
     *   - ������
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
    }/* ����� ������� clear */

    /* �������, ������������ ������������������ ��� � �������� ������� � ��������� ������
     * ���������:
     *   - ����� ������ � �������
     *       bitArray& bA
     * ����������:
     *   - ������
     *       (VOID)
     */
    VOID endCode( bitArray& bA )         
    {
      bLNode* p = tail;
      while (p != NULL)                                // ������ ������������������ � ���� � �������� �������
      {
        bA.addBit(p->bit);
        p = p->prev;
      }
      clear();
    }/* ����� ������� endCode */
  };/* ����� ������ bList */

  /* ������ ��������� hTNode, ����/����� ������ �������� */
  struct hTNode                                      
  {
    // ������ ����/�����. 0 - ����, 1 - ����, 2 - ESC-������, 3 - EOF-������
    INT isLeafEsc;   
    // ��� ����/�����
    unsigned long key; 
    // ������ �����
    CHAR sym;                                        
    hTNode *next;                                    
    hTNode *prev;                                    
    hTNode *par;                                     
    hTNode *left;                                    
    hTNode *right;                 
    /* ����������� ��������� hTNode
     * ���������:
     *   - ��� ����
     *       INT k = 0
     *   - ������ ����
     *       INT iLE = 0
     *   - ��������� �� ��������
     *       hTNode *p = NULL
     *   - ��������� �� ��������� �� ������ ����
     *       hTNode *n = NULL
     * ����������:
     *   - ������
     */
    hTNode ( INT k = 0, INT iLE = 0, hTNode *p = NULL, hTNode *n = NULL ): key(k), isLeafEsc(iLE), next(n), par(p),  
                                                                           left(NULL), right(NULL), prev(NULL) {}
    // ����� ������������ ��������� hTNode
  };/* ����� ��������� hTNode */

  /* ����� hTree, ������ �������� */
  class hTree
  {
    // ������ ������
    hTNode *head;          
    // ESC-������
    hTNode *last;     
    // EOF-������
    hTNode *eof;
  public:
    /* ����������� ������ hTree
     * ���������:
     *   - ������
     *       (VOID)
     * ����������:
     *   - ������
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
    }/* ����� ������������ ������ hTree */

    /* ���������� ������ bList
     * ���������:
     *   - ������
     *       (VOID)
     * ����������:
     *   - ������
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
    }/* ����� ����������� ������ hTree */

    /* ������� ��������� EOF-�������
     * ���������:
     *   - ������
     *       (VOID)
     * ����������:
     *   - ��������� �� EOF-������
     *       (hTNode *)
     */
    hTNode * getEOF( VOID )                                              
    {
      return eof;
    }/* ����� ������� getEOF */

    /* ������� ��������� ����� ������
     * ���������:
     *   - ������
     *       (VOID)
     * ����������:
     *   - ��������� �� ������ ������
     *       (hTNode *)
     */
    hTNode * getHead( VOID )                                            
    {
      return head;
    }/* ����� ������� getHead */

    /* ������� ���������� ������� � ������
     * ���������:
     *   - ������
     *       CHAR c
     * ����������:
     *   - ��������� �� ��������� ����
     *       (hTNode *)
     */
    hTNode * addSym( CHAR c )                                           
    {
      // ������� ���� � ����
      hTNode *newLeaf = new hTNode(0, 1);                         
      hTNode *newNode = new hTNode;   
      if(newLeaf == NULL || newNode == NULL)
        throw exc::BadAlloc();

      // ESC-������ � ����� ���� - ���������� ������ ����
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
    }/* ����� ������� addSym */

    /* ������� ������ ������� � ������
     * ���������:
     *   - ������
     *       CHAR c
     *   - ��������� ������
     *       htNode **result
     * ����������:
     *   - ������ ������
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
    }/* ����� ������� symSearch */

    /* ������� ������ ������� �� ������������ ����
     * ���������:
     *   - ����� ������ � �������
     *       bitArray& bA
     * ����������:
     *   - ��������� �� ��������� ������
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
    }/* ����� ������� codeSearch */

    friend hTNode * symEncode( bitArray& bA, hTree& t, CHAR c );                 
    friend INT symDecode( bitArray& bA, hTree& t, CHAR &c );  
    friend VOID treeRefresh( hTree& t, CHAR c );                                            
  };
}/* ����� ������������ ���� hcp */
