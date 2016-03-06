#include "commondf.h"
#include "appwin.h"
#include "resource.h"

INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, CHAR *cmdLine, INT cmdShow )
{
  _CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG));
  HANDLE runMutex = CreateMutex(nullptr, FALSE, "HCP");
  if (GetLastError() == ERROR_ALREADY_EXISTS)
  {
    MessageBox(nullptr, "Program is already running.", nullptr, MB_OK);
    return 1;
  }
  hcp::Application app(hInst, hPrevInst, cmdLine, cmdShow);
  INT ret = app.run();
  CloseHandle(runMutex);
  return ret;
}