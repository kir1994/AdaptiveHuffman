#pragma comment(lib, "comctl32.lib")
#define WINVER 0x501

#include <string>
#include <sstream>
#include <iomanip> 
#include <time.h>

#include "appwin.h"
#include "utils\utils.h"
#include "resource.h"
#include <CommCtrl.h>
#include <Richedit.h>


using namespace hcp;

const CHAR * Application::WND_CLASS_NAME = "My window class"; 

Application::Application( HINSTANCE hInst, HINSTANCE hPrevInst, CHAR *cmdLine, INT cmdShow ) : hInst_(hInst), hWnd_(nullptr), procFlag_(FALSE)
{
  WNDCLASS wc;
  wc.style = 0;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = sizeof(this);
  wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
  wc.lpszMenuName = nullptr;
  wc.hInstance = hInst_;
  wc.lpfnWndProc = (WNDPROC)windowFunc;
  wc.lpszClassName = WND_CLASS_NAME;

  if (!RegisterClass(&wc))
  {
    MessageBox(nullptr, "Error register window class", "ERROR", MB_OK);
    return;
  }

  menu_ = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU2));
  accel_ = LoadAccelerators(hInst, MAKEINTRESOURCE(IDR_ACCELERATOR1));
  hWnd_ = CreateWindow(WND_CLASS_NAME, "Huffman Compression", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_EX_COMPOSITED,
                        CW_USEDEFAULT, CW_USEDEFAULT, 500, 200,
                        nullptr, menu_, hInst_, this);
  initComponents();
  ShowWindow(hWnd_, cmdShow);
  UpdateWindow(hWnd_);
}

INT Application::run( VOID )
{
  MSG msg;
  HACCEL acc = LoadAccelerators(hInst_, MAKEINTRESOURCE(IDR_ACCELERATOR1));
  while (GetMessage(&msg, nullptr, 0, 0))
  {
    if (TranslateAccelerator(hWnd_, accel_, &msg) != 0)
      continue;
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return msg.wParam;
}

INT CALLBACK Application::windowFunc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
  static Application* self = nullptr;
      
  if (msg == WM_CREATE)
  {
    self = (Application *)((CREATESTRUCT*)lParam)->lpCreateParams;
    self->hWnd_ = hWnd;
  }
  if (self == nullptr)
    return DefWindowProc(hWnd, msg, wParam, lParam);
  return self->myWindowFunc(msg, wParam, lParam);
}

VOID Application::initComponents( VOID )
{

  InitCommonControlsEx(nullptr);
  DragAcceptFiles(hWnd_, TRUE);
  get(HCP_T_PATH_INFO) = CreateWindow("Static", "  Select file:", WS_CHILD | WS_VISIBLE,
                                      5, 5, 100, 23, hWnd_, (HMENU)HCP_T_PATH_INFO, hInst_, nullptr);
  get(HCP_T_PATH) = CreateWindow("Edit", nullptr, WS_BORDER | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                          5, 30, 400, 23, hWnd_, (HMENU)HCP_T_PATH, hInst_, nullptr);
  get(HCP_B_SELECT) = CreateWindow("Button", "Select", WS_CHILD | WS_VISIBLE,
                          415, 30, 60, 23, hWnd_, (HMENU)HCP_B_SELECT, hInst_, nullptr);
  SetFocus(get(HCP_B_SELECT));
  get(HCP_B_COMPR) = CreateWindow("Button", "Compress", WS_CHILD | WS_VISIBLE | WS_DISABLED,
                          205, 60, 100, 23, hWnd_, (HMENU)HCP_B_COMPR, hInst_, nullptr);
  get(HCP_B_DECOMPR) = CreateWindow("Button", "Decompress", WS_CHILD | WS_VISIBLE | WS_DISABLED,
                          305, 60, 100, 23, hWnd_, (HMENU)HCP_B_DECOMPR, hInst_, nullptr);
  get(HCP_T_PROGR_INFO) = CreateWindow("Static", nullptr, WS_CHILD,
                                       5, 77, 200, 23, hWnd_, (HMENU)HCP_T_PROGR_INFO, hInst_, nullptr);
  get(HCP_P_PROGR) = CreateWindow(PROGRESS_CLASS, nullptr, WS_CHILD | WS_VISIBLE | PBS_SMOOTH | PBS_MARQUEE,
               5, 100, 485, 20, hWnd_, (HMENU)HCP_P_PROGR, hInst_, nullptr);
  SendMessage(get(HCP_P_PROGR), PBM_SETRANGE, 0, MAKELPARAM(0, 1000));
  get(HCP_B_CANCEL) = CreateWindow("Button", "Cancel", WS_CHILD | WS_VISIBLE | WS_DISABLED,
                                   420, 125, 50, 23, hWnd_, (HMENU)HCP_B_CANCEL, hInst_, nullptr);
  get(HCP_T_RES_INFO) = CreateWindow("Static", "Result:", WS_CHILD | WS_VISIBLE | WS_DISABLED,
                                5, 125, 60, 23, hWnd_, (HMENU)HCP_T_RES_INFO, hInst_, nullptr);
  LoadLibrary("riched32.dll");
  get(HCP_T_RES) = CreateWindow(RICHEDIT_CLASS, "", WS_CHILD | WS_VISIBLE | ES_READONLY, 
                                55, 125, 350, 23, hWnd_, (HMENU)HCP_T_RES, hInst_, nullptr);
  SendMessage(get(HCP_T_RES), EM_SETBKGNDCOLOR, 0, GetSysColor(COLOR_3DFACE));
}

INT Application::myWindowFunc( const UINT &msg, const WPARAM &wParam, const LPARAM &lParam )
{
  static INT pos = 0;
  switch (msg)
  {
  case WM_CREATE:
    ShowWindow(hWnd_, SW_SHOWNORMAL);
    hWndAbout_ = CreateDialog(hInst_, MAKEINTRESOURCE(HCP_DIALOG_ABOUT), hWnd_, aboutDialogFunc);
    ShowWindow(hWndAbout_ , SW_SHOW); 
    return 0;
  case WM_TIMER:
  {
    HUFF_STATE st = hm_.getState();
    if (st == HUFF_WORKING || st == HUFF_SUCCESS)
    {
      HDC hDC = GetDC(hWnd_);
      UINT64 fSize = hm_.getFileSize();

      UINT64 curRead = hm_.getReadState();
      UINT64 curWrite = hm_.getWriteState();
      std::stringstream ss;
      SendMessage(get(HCP_P_PROGR), PBM_SETPOS, (INT)(curRead * 1000. / fSize), 0);
      ss << fileSizeToString(curRead, 2) << "/" << fileSizeToString(fSize, 2) <<
        " ratio: " << std::fixed << std::setprecision(2) << 
        (LDBL)(curWrite == 0 ? 100: curWrite * 100. / curRead) << "% " << "time: " << 
        std::fixed << std::setprecision(2) << 
        (clock() - startTime_) / (DBL)CLOCKS_PER_SEC << "sec.";

      SendMessage(get(HCP_T_RES), WM_SETTEXT, 0, (LPARAM)ss.str().c_str());
      if (st == HUFF_SUCCESS)
        setLockState(FALSE);
    } else if (st == HUFF_FAILED)
    {
      printResult("File proccessing filed!");
      setLockState(FALSE);
    }
  }
    return 0;
  case WM_DESTROY:
    return 0;
  case WM_CLOSE:
    if (procFlag_ == TRUE)
    {
      if (MessageBox(hWnd_, "Stop file processing?", "Question", MB_YESNO | MB_ICONQUESTION) == IDYES)
      {
        hm_.stop();
        PostQuitMessage(0);
      }
    }
    else
      PostQuitMessage(0);
    return 0;
  case WM_DROPFILES:
    {
      CHAR fileName[MAX_STR];
      DragQueryFile((HDROP)wParam, 0, fileName, MAX_STR);
      SendMessage(get(HCP_T_PATH), WM_SETTEXT, 0, (WPARAM)fileName);
    }
    return 0;
  case WM_COMMAND:
    if (HIWORD(wParam) == 1 && IsWindowEnabled(hWnd_) == FALSE)
    {
      if (IsWindow(hWndAbout_) == TRUE)
        DestroyWindow(hWndAbout_);
      return 0;
    }
    switch (LOWORD(wParam))
    {
    case HCP_MENU_ABOUT:
      hWndAbout_ = CreateDialog(hInst_, MAKEINTRESOURCE(HCP_DIALOG_ABOUT), hWnd_, aboutDialogFunc);
      ShowWindow(hWndAbout_ , SW_SHOWNORMAL);
      return 0;
    case HCP_ACC_EXIT:
      SendMessage(hWnd_, WM_CLOSE, 0, 0);
      return 0;
    case HCP_MENU_SELECT:
    case HCP_B_SELECT:
      {
        OPENFILENAME set;
        memset(&set, 0, sizeof(OPENFILENAME));
        set.lStructSize = sizeof(OPENFILENAME);
        set.hwndOwner = hWnd_;
        set.hInstance = hInst_;
        CHAR selectedFile[MAX_STR] = "\0";
        set.lpstrFile = selectedFile; 
        set.nMaxFile = MAX_STR;
        CHAR initialDir[MAX_STR];
        GetCurrentDirectory(MAX_STR, initialDir);
        set.lpstrInitialDir = initialDir;
        BOOL res = GetOpenFileName(&set);
        if (res == TRUE)
          SendMessage(get(HCP_T_PATH), WM_SETTEXT, 0, (WPARAM)selectedFile);
      }
      return 0;
    case HCP_T_PATH:
      {
        printResult();
        SendMessage(get(HCP_P_PROGR), PBM_SETPOS, 0, 0);
        if (SendMessage(get(HCP_T_PATH), WM_GETTEXTLENGTH, 0, 0) != 0)
        {
          EnableWindow(get(HCP_B_COMPR), TRUE);
          EnableWindow(get(HCP_B_DECOMPR), TRUE);
          EnableMenuItem(menu_, HCP_MENU_COMPR, MF_ENABLED);
          EnableMenuItem(menu_, HCP_MENU_DECOMPR, MF_ENABLED);
        }
        else
        {
          EnableWindow(get(HCP_B_COMPR), FALSE);
          EnableWindow(get(HCP_B_DECOMPR), FALSE);
          EnableMenuItem(menu_, HCP_MENU_COMPR, MF_GRAYED);
          EnableMenuItem(menu_, HCP_MENU_DECOMPR, MF_GRAYED);
        }
      }
      return 0;
    case HCP_MENU_COMPR:
    case HCP_B_COMPR:
    {
      printResult();
      SetWindowText(get(HCP_T_PROGR_INFO), "Compressing, please wait...");
      setLockState(TRUE);
      CHAR str[MAX_STR];
      SendMessage(get(HCP_T_PATH), WM_GETTEXT, MAX_STR, (LPARAM)&str);
      hm_.compress(str);
    }
      return 0;
    case HCP_MENU_DECOMPR:
    case HCP_B_DECOMPR:
      printResult();
      SetWindowText(get(HCP_T_PROGR_INFO), "Decompressing, please wait...");
      setLockState(TRUE);
      CHAR str[MAX_STR];
      SendMessage(get(HCP_T_PATH), WM_GETTEXT, MAX_STR, (LPARAM)&str);
      hm_.decompress(str);
      return 0;
    case HCP_B_CANCEL:
      if (MessageBox(hWnd_, "Stop file processing?", "Question", MB_YESNO | MB_ICONQUESTION) == IDYES)
      {
        hm_.stop();
        pos = 0;
        printResult("Process cancelled by user.");
        setLockState(FALSE);
      }

    };
    break;
  };
  return DefWindowProc(hWnd_, msg, wParam, lParam);
}

VOID Application::setLockState( BOOL lock )
{
  static BOOL lockFlag = FALSE;
  if (lockFlag != lock)
  {
    lockFlag = lock;
    if (lock == TRUE)
    {
      procFlag_ = TRUE;
      EnableWindow(get(HCP_B_SELECT), FALSE);
      EnableWindow(get(HCP_B_COMPR), FALSE);
      EnableWindow(get(HCP_B_DECOMPR), FALSE);
      EnableWindow(get(HCP_T_PATH), FALSE);
      EnableWindow(get(HCP_T_PATH_INFO), FALSE);
      ShowWindow(get(HCP_T_PROGR_INFO), SW_NORMAL);
      EnableWindow(get(HCP_B_CANCEL), TRUE);
      EnableWindow(get(HCP_T_RES_INFO), TRUE);
      SendMessage(get(HCP_T_RES), WM_SETTEXT, 0, 0);
      startTime_ = clock();
      SetTimer(hWnd_, 1, 100, NULL);
    }
    else
    {
      procFlag_ = FALSE;
      EnableWindow(get(HCP_B_SELECT), TRUE);
      EnableWindow(get(HCP_B_COMPR), TRUE);
      EnableWindow(get(HCP_B_DECOMPR), TRUE);
      EnableWindow(get(HCP_T_PATH), TRUE);
      EnableWindow(get(HCP_T_PATH_INFO), TRUE);
      ShowWindow(get(HCP_T_PROGR_INFO), SW_HIDE);
      EnableWindow(get(HCP_B_CANCEL), FALSE);
      KillTimer(hWnd_, 1);
    }
  }
}

VOID Application::printResult( const string &str )
{
  SendMessage(get(HCP_T_RES), WM_SETTEXT, 0, (LPARAM)str.c_str());
}

BOOL CALLBACK Application::aboutDialogFunc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
  static UINT timerCount = 5;

  switch (msg)
  {
  case WM_INITDIALOG:
    if (timerCount != 0)
    {
      EnableWindow(GetDlgItem(hWnd, HCP_DIALOG_OK), FALSE);
      EnableWindow(GetParent(hWnd), FALSE);
      SetTimer(hWnd, 1, 1000, NULL);
      SetDlgItemInt(hWnd, HCP_DIALOG_OK, timerCount, FALSE);
    }
    return TRUE;
  case WM_DESTROY:
    EnableWindow(GetParent(hWnd), TRUE);
    SetFocus(GetParent(hWnd));
    return TRUE;
  case WM_COMMAND:
    switch (LOWORD(wParam))
    {
    case HCP_DIALOG_OK:
      DestroyWindow(hWnd);
      return TRUE;
    }
    break;
  case WM_TIMER:
    timerCount--;
    if (timerCount == 0)
    {
      SetDlgItemText(hWnd, HCP_DIALOG_OK, "Ok");
      EnableWindow(GetDlgItem(hWnd, HCP_DIALOG_OK), TRUE);
      KillTimer(hWnd, 1);
    }
    else
      SetDlgItemInt(hWnd, HCP_DIALOG_OK, timerCount, FALSE);
    return TRUE;
  };
  return FALSE;
}
