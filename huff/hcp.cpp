#include <stdlib.h>
#include <string.h>

#include "hcp.h"

using namespace hcp;

/* ����������� ������ huffMain, ����������� �� ������ � ������������/�������������� �� ��������
 * ���������:
 *   - ������
 * ����������:
 *   - ������ 
 */
huffMain::huffMain( VOID ): inFileSize(0), hs(HUFF_IDLING)
{
  fileName[0] = 0;
  InitializeCriticalSection(&suspCS);
}/* ����� ������������ ������ huffMain */

/* ���������� ������ huffMain
 * ���������:
 *   - ������
 * ����������:
 *   - ������ 
 */
huffMain::~huffMain( VOID )
{
  stop();
  DeleteCriticalSection(&suspCS);
}/* ����� ����������� ������ huffMain */

/* ������� ��������� ����� ��������������� � ������ ������ �����
 * ���������:
 *   - ������
 *       (VOID)
 * ����������:
 *   - ��� �����
 *       (CHAR *)
 */
CHAR * huffMain::getFName( VOID )
{
  return fileName;
}/* ����� ������� getFName */

/* ������� ��������� ������ ������ � �������
 * ���������:
 *   - ������
 *       (VOID)
 * ����������:
 *   - ����� ������ � �������
 *       (bitArray &)
 */
bitArray & huffMain::getBitArray( VOID )
{
  return bA;
}/* ����� ������� getBitArray */

/* ������� ��������� ���������� ��������� ����
 * ���������:
 *   - ������
 *       (VOID)
 * ����������:
 *   - ���������� ��������� ����
 *       (UINT64)
 */
UINT64 huffMain::getReadState( VOID )
{
  return bA.getReadNum();
}/* ����� ������� getReadState */

/* ������� ��������� ���������� ���������� ����
 * ���������:
 *   - ������
 *       (VOID)
 * ����������:
 *   - ���������� ���������� ����
 *       (UINT64)
 */
UINT64 huffMain::getWriteState( VOID )
{
  return bA.getWriteNum();
}/* ����� ������� getWriteState */

/* ������� ��������� ������� ����� � ������
 * ���������:
 *   - ������
 *       (VOID)
 * ����������:
 *   - ������ ����� � ������
 *       (UINT64)
 */
UINT64 huffMain::getFileSize( VOID )
{
  return inFileSize;
}/* ����� ������� getFileSize */

/* �������, ���������� �� ������ �����
 * ���������:
 *   - ��� �����
 *       CHAR *fName
 * ����������:
 *   - ��� ����������
 *       (INT)
 */
INT huffMain::compress( CHAR *fName )
{
  // ���� ���������� ������
  WaitForSingleObject(h, INFINITE);

  ifstream i(fName, ios_base::binary);
  // ��������� ������� �����
  if (i.good() == FALSE)
  {
    hs = HUFF_FAILED;
    return -1;     
  }
  else
  {
    hs = HUFF_WORKING;
    // ���������� ������� �����
    i.seekg (0, ios::end);
    inFileSize = i.tellg();
    i.close();

    strcpy_s(fileName, MAX_STR, fName);
    // ������� �����, ��������� ����
    h = CreateThread(NULL, 0, &encode, this, 0, &id);       
      
    return 0;
  }    
}/* ����� ������� compress */

/* �������, ���������� �� �������� �����
 * ���������:
 *   - ��� �����
 *       CHAR *fName
 * ����������:
 *   - ��� ����������
 *       (INT)
 */
INT huffMain::decompress( CHAR *fName )
{
  // ���� ���������� ���������� ��������
  WaitForSingleObject(h, INFINITE);

  ifstream i(fName, ios_base::binary);
  // ���������, ���� �� ����
  if (i.good() == FALSE)
  {
    hs = HUFF_FAILED;
    return -1;
  }
  else
  {
    hs = HUFF_WORKING;
    // ��������� ������ �����
    i.seekg (0, ios::end);
    inFileSize = i.tellg();
    i.close();
    strcpy_s(fileName, MAX_STR, fName);
    // ������� �����, ������������ ����
    h = CreateThread(NULL, 0, &decode, this, 0, &id);

    return 0;
  }
}/* ����� ������� decompress */

/* �������, ���������� �� ��������� �������� ������/��������
 * ���������:
 *   - ������
 *       (VOID)
 * ����������:
 *   - ������
 *       (VOID)
 */
VOID huffMain::stop( VOID )
{
  EnterCriticalSection(&suspCS);
  WaitForSingleObject(h, INFINITE);
  bA.close();
  SuspendThread(h);
  LeaveCriticalSection(&suspCS);
  hs = HUFF_CANCELED;
}/* ����� ������� stop */

/* �������, ������������ ������� ��������� ���������
 * ���������:
 *   - ������
 *       (VOID)
 * ����������:
 *   - ��������� ���������
 *       (HUFF_STATE)
 */
HUFF_STATE huffMain::getState( VOID )
{
  HUFF_STATE res = hs;
  // ����� ���������, ���� ������ � ������ �� ������������
  if (hs != HUFF_WORKING)
    hs = HUFF_IDLING;

  return res;
}/* ����� ������� getState */

/* ������� ������, ����������� �� ������ �����
 * ���������:
 *   - �������� ����� ������ �� ������� ��������
 *       VOID *lPar
 * ����������:
 *   - ��� ����������
 *       (DWORD)
 */
DWORD WINAPI huffMain::encode( VOID *lPar )
{ 
  CHAR *fileName = ((huffMain *)lPar)->getFName(); 
  // ��� ��������������� ����� - ��� �������� � ����������� + ".hcf"
  CHAR formatMask[] = "HCF\0\0\0";
  BOOL endFl = FALSE;
  CHAR resFileName[MAX_STR];

  CHAR drv[10], dir[MAX_STR], fName[MAX_STR], ext[20];
  _splitpath_s(fileName, drv, sizeof(drv), dir, MAX_STR, fName, MAX_STR, ext, sizeof(ext));
  
  strcat_s(ext, ".hcf");
  _makepath_s(resFileName, MAX_STR, drv, dir, fName, ext);

  ((huffMain *)lPar)->getBitArray().init(fileName, resFileName);
  ((huffMain *)lPar)->getBitArray().write((BYTE *)formatMask, 6);

  huffEncode(((huffMain *)lPar)->getBitArray(), ((huffMain *)lPar)->suspCS);

  ((huffMain *)lPar)->getBitArray().close();
  ((huffMain *)lPar)->hs = HUFF_SUCCESS;

  return 0;
}/* ����� ������� encode */

/* ������� ������, ����������� �� ��������� �����
 * ���������:
 *   - �������� ����� ������ �� ������� ��������
 *       VOID *lPar
 * ����������:
 *   - ��� ����������
 *       (DWORD)
 */
DWORD WINAPI huffMain::decode( VOID *lPar )
{
  CHAR *fileName = ((huffMain *)lPar)->getFName();
  // ��� ��������������� ����� - ��� �������� - ".hcf"
  CHAR resFileName[MAX_STR];
  CHAR drv[10], dir[MAX_STR], fName[MAX_STR], ext[20];
  _splitpath_s(fileName, drv, sizeof(drv), dir, MAX_STR, fName, MAX_STR, ext, sizeof(ext));
  
  if (ext[0] == '.')
    *strrchr(ext, '.') = 0;
  else
    strcpy(ext, ".unc");
  _makepath_s(resFileName, MAX_STR, drv, dir, fName, ext);
  ((huffMain *)lPar)->getBitArray().init(fileName, resFileName);

  CHAR inputFormatMask[6];
  ((huffMain *)lPar)->getBitArray().read((BYTE *)inputFormatMask, 6);
  CHAR formatMask[] = "HCF\0\0\0";
  if (memcmp(formatMask, inputFormatMask, 6) != 0)
  {
    ((huffMain *)lPar)->getBitArray().close();
    ((huffMain*)lPar)->hs = HUFF_FAILED;
    return 1;
  }

  
  huffDecode(((huffMain *)lPar)->getBitArray(), ((huffMain *)lPar)->suspCS);
  ((huffMain *)lPar)->getBitArray().close();
  ((huffMain *)lPar)->hs = HUFF_SUCCESS;

  return 0;
}/* ����� ������� decode */