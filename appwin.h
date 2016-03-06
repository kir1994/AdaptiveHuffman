#pragma once

#include "commondf.h"
#include "huff\hcp.h"

namespace hcp
{
  class Application
  {
  private:
    static const CHAR *WND_CLASS_NAME;
    HWND hWnd_;
    HWND hWndAbout_;
    HMENU menu_;
    HACCEL accel_;
    HINSTANCE hInst_;

    huffMain hm_;
    UINT startTime_;

    enum HCP_ITEM_ID
    {
      HCP_B_SELECT = 1,
      HCP_B_COMPR,
      HCP_B_DECOMPR,
      HCP_B_CANCEL,
      HCP_T_PATH,
      HCP_T_PATH_INFO,
      HCP_T_PROGR_INFO,
      HCP_T_RES,
      HCP_T_RES_INFO,
      HCP_P_PROGR,
    };
    HWND items[10];
    inline HWND & get( HCP_ITEM_ID id )
    {
      if (id <= 0 || id > sizeof(items) / sizeof(HWND))
        throw exc::OutOfRange("HCP_ITEM_ID is out of range");
      return items[(INT)id - 1];
    }

    BOOL procFlag_;
    
    static INT CALLBACK windowFunc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
    static BOOL CALLBACK aboutDialogFunc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
    INT myWindowFunc( const UINT &msg, const WPARAM &wParam, const LPARAM &lParam );
    
    VOID initComponents( VOID );
    VOID setLockState( BOOL lock );
    VOID printResult( const string &str = "\0" );
  public:
    Application( HINSTANCE hInst, HINSTANCE hPrevInst, CHAR *cmdLine, INT cmdShow );

    INT run( VOID );
  };
}