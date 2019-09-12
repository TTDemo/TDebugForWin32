#include "../Comm/TDebugFunc.h"

class TDebugUI
{
public:
  static void ShowReg(IN CONTEXT* lpContext);
  static void ShowAsm(OUT LPASM_CODE lpAsmCode, IN DWORD dwAsmCnt);
  static void ShowError();
  static void ShowMainMenu();
  static void ShowMsg(const char *lpszMsg, ...);
  static BOOL SetConsoleColor(WORD wAttributes);
  static HANDLE GetConsoleHandle();
  static void ShowMemory(LPVOID lpStartAddr, BYTE* lpData, DWORD dwLen);

private:
  static HANDLE g_hConsole;
};

