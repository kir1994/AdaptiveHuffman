#pragma once

#include <Windows.h>

#include "..\commondf.h"
#include "huff.h"

/* Начало пространства имен hcp */
namespace hcp
{
  enum HUFF_STATE
  {
    HUFF_WORKING, 
    HUFF_IDLING, 
    HUFF_SUCCESS,
    HUFF_FAILED,
    HUFF_CANCELED
  };

  /* Начало класса huffMain */
  class huffMain
  {
    CRITICAL_SECTION suspCS;
    CHAR fileName[MAX_STR];
    HANDLE h;
    bitArray bA;
    HUFF_STATE hs;

    UINT64 inFileSize;
    DWORD id;

    static DWORD WINAPI encode( VOID *lPar );
    static DWORD WINAPI decode( VOID *lPar );
  public:

    huffMain( VOID );
    ~huffMain( VOID );

    CHAR * getFName( VOID );
    bitArray & getBitArray( VOID );
    UINT64 getReadState( VOID );
    UINT64 getWriteState( VOID );
    UINT64 getFileSize( VOID );
    INT compress( CHAR *fName );
    INT decompress( CHAR *fName );
    VOID stop( VOID );
    HUFF_STATE getState( VOID );
  };/* Конец класса huffMain */
}/* Конец пространства имен hcp */


