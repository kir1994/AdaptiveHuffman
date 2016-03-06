#include <stdlib.h>
#include <string.h>

#include "hcp.h"

using namespace hcp;

/* Конструктор класса huffMain, отвечающего за работу с кодированием/декодированием по Хаффмену
 * АРГУМЕНТЫ:
 *   - ничего
 * ВОЗВРАЩАЕТ:
 *   - ничего 
 */
huffMain::huffMain( VOID ): inFileSize(0), hs(HUFF_IDLING)
{
  fileName[0] = 0;
  InitializeCriticalSection(&suspCS);
}/* конец конструктора класса huffMain */

/* Деструктор класса huffMain
 * АРГУМЕНТЫ:
 *   - ничего
 * ВОЗВРАЩАЕТ:
 *   - ничего 
 */
huffMain::~huffMain( VOID )
{
  stop();
  DeleteCriticalSection(&suspCS);
}/* конец деструктора класса huffMain */

/* Функция получения имени обрабатываемого в данный момент файла
 * АРГУМЕНТЫ:
 *   - ничего
 *       (VOID)
 * ВОЗВРАЩАЕТ:
 *   - имя файла
 *       (CHAR *)
 */
CHAR * huffMain::getFName( VOID )
{
  return fileName;
}/* конец функции getFName */

/* Функция получения класса работы с файлами
 * АРГУМЕНТЫ:
 *   - ничего
 *       (VOID)
 * ВОЗВРАЩАЕТ:
 *   - класс работы с файлами
 *       (bitArray &)
 */
bitArray & huffMain::getBitArray( VOID )
{
  return bA;
}/* конец функции getBitArray */

/* Функция получения количества считанных байт
 * АРГУМЕНТЫ:
 *   - ничего
 *       (VOID)
 * ВОЗВРАЩАЕТ:
 *   - количество считанных байт
 *       (UINT64)
 */
UINT64 huffMain::getReadState( VOID )
{
  return bA.getReadNum();
}/* конец функции getReadState */

/* Функция получения количества записанных байт
 * АРГУМЕНТЫ:
 *   - ничего
 *       (VOID)
 * ВОЗВРАЩАЕТ:
 *   - количество записанных байт
 *       (UINT64)
 */
UINT64 huffMain::getWriteState( VOID )
{
  return bA.getWriteNum();
}/* конец функции getWriteState */

/* Функция получения размера файла в байтах
 * АРГУМЕНТЫ:
 *   - ничего
 *       (VOID)
 * ВОЗВРАЩАЕТ:
 *   - размер файла в байтах
 *       (UINT64)
 */
UINT64 huffMain::getFileSize( VOID )
{
  return inFileSize;
}/* конец функции getFileSize */

/* Функция, отвечающая за сжатие файла
 * АРГУМЕНТЫ:
 *   - имя файла
 *       CHAR *fName
 * ВОЗВРАЩАЕТ:
 *   - код завершения
 *       (INT)
 */
INT huffMain::compress( CHAR *fName )
{
  // Ждем завершения потока
  WaitForSingleObject(h, INFINITE);

  ifstream i(fName, ios_base::binary);
  // Проверяем наличие файла
  if (i.good() == FALSE)
  {
    hs = HUFF_FAILED;
    return -1;     
  }
  else
  {
    hs = HUFF_WORKING;
    // Вычесление размера файла
    i.seekg (0, ios::end);
    inFileSize = i.tellg();
    i.close();

    strcpy_s(fileName, MAX_STR, fName);
    // Создаем поток, сжимающий файл
    h = CreateThread(NULL, 0, &encode, this, 0, &id);       
      
    return 0;
  }    
}/* конец функции compress */

/* Функция, отвечающая за расжатие файла
 * АРГУМЕНТЫ:
 *   - имя файла
 *       CHAR *fName
 * ВОЗВРАЩАЕТ:
 *   - код завершения
 *       (INT)
 */
INT huffMain::decompress( CHAR *fName )
{
  // Ждем завершения предыдущей операции
  WaitForSingleObject(h, INFINITE);

  ifstream i(fName, ios_base::binary);
  // Проверяем, есть ли файл
  if (i.good() == FALSE)
  {
    hs = HUFF_FAILED;
    return -1;
  }
  else
  {
    hs = HUFF_WORKING;
    // Вычисляем размер файла
    i.seekg (0, ios::end);
    inFileSize = i.tellg();
    i.close();
    strcpy_s(fileName, MAX_STR, fName);
    // Создаем поток, декодирующий файл
    h = CreateThread(NULL, 0, &decode, this, 0, &id);

    return 0;
  }
}/* конец функции decompress */

/* Функция, отвечающая за остановку процесса сжатия/расжатия
 * АРГУМЕНТЫ:
 *   - ничего
 *       (VOID)
 * ВОЗВРАЩАЕТ:
 *   - ничего
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
}/* конец функции stop */

/* Функция, возвращающая текущее состояние программы
 * АРГУМЕНТЫ:
 *   - ничего
 *       (VOID)
 * ВОЗВРАЩАЕТ:
 *   - состояние программы
 *       (HUFF_STATE)
 */
HUFF_STATE huffMain::getState( VOID )
{
  HUFF_STATE res = hs;
  // Сброс состояния, если работа с файлом не производится
  if (hs != HUFF_WORKING)
    hs = HUFF_IDLING;

  return res;
}/* конец функции getState */

/* Функция потока, отвечающего за сжатие файла
 * АРГУМЕНТЫ:
 *   - основной класс работы со сжатием Хаффмена
 *       VOID *lPar
 * ВОЗВРАЩАЕТ:
 *   - код завершения
 *       (DWORD)
 */
DWORD WINAPI huffMain::encode( VOID *lPar )
{ 
  CHAR *fileName = ((huffMain *)lPar)->getFName(); 
  // Имя результирующего файла - имя текущего с расширением + ".hcf"
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
}/* конец функции encode */

/* Функция потока, отвечающего за рассжатие файла
 * АРГУМЕНТЫ:
 *   - основной класс работы со сжатием Хаффмена
 *       VOID *lPar
 * ВОЗВРАЩАЕТ:
 *   - код завершения
 *       (DWORD)
 */
DWORD WINAPI huffMain::decode( VOID *lPar )
{
  CHAR *fileName = ((huffMain *)lPar)->getFName();
  // Имя результирующего файла - имя текущего - ".hcf"
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
}/* конец функции decode */