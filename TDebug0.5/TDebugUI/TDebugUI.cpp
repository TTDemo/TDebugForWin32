#include "TDebugUI.h"

HANDLE TDebugUI::g_hConsole = NULL;


void TDebugUI::ShowReg(IN CONTEXT*      lpContext)
{
  WORD dwCol1 = FOREGROUND_BLUE  | FOREGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE| BACKGROUND_INTENSITY;
  WORD dwCol2 = BACKGROUND_RED | BACKGROUND_GREEN| BACKGROUND_BLUE|BACKGROUND_INTENSITY;
  SetConsoleColor(dwCol2); ShowMsg("EAX:    ");  SetConsoleColor(dwCol1); ShowMsg("%08X\r\n", lpContext->Eax);
  SetConsoleColor(dwCol2); ShowMsg("EBX:    ");  SetConsoleColor(dwCol1); ShowMsg("%08X\r\n", lpContext->Ebx);
  SetConsoleColor(dwCol2); ShowMsg("ECX:    ");  SetConsoleColor(dwCol1); ShowMsg("%08X\r\n", lpContext->Ecx);
  SetConsoleColor(dwCol2); ShowMsg("EDX:    ");  SetConsoleColor(dwCol1); ShowMsg("%08X\r\n", lpContext->Edx);
  SetConsoleColor(dwCol2); ShowMsg("ESI:    ");  SetConsoleColor(dwCol1); ShowMsg("%08X\r\n", lpContext->Edx); 
  SetConsoleColor(dwCol2); ShowMsg("EDI:    ");  SetConsoleColor(dwCol1); ShowMsg("%08X\r\n", lpContext->Edi);
  SetConsoleColor(dwCol2); ShowMsg("EIP:    ");  SetConsoleColor(dwCol1); ShowMsg("%08X\r\n", lpContext->Eip);
  SetConsoleColor(dwCol2); ShowMsg("ESP:    ");  SetConsoleColor(dwCol1); ShowMsg("%08X\r\n", lpContext->Esp);
  SetConsoleColor(dwCol2); ShowMsg("EBP:    ");  SetConsoleColor(dwCol1); ShowMsg("%08X\r\n", lpContext->Ebp);
  SetConsoleColor(dwCol2); ShowMsg("DS :    ");  SetConsoleColor(dwCol1); ShowMsg("%08X\r\n", lpContext->SegDs);
  SetConsoleColor(dwCol2); ShowMsg("ES :    ");  SetConsoleColor(dwCol1); ShowMsg("%08X\r\n", lpContext->SegEs);
  SetConsoleColor(dwCol2); ShowMsg("SS :    ");  SetConsoleColor(dwCol1); ShowMsg("%08X\r\n",lpContext->SegSs);
  SetConsoleColor(dwCol2); ShowMsg("FS :    ");  SetConsoleColor(dwCol1); ShowMsg("%08X\r\n",lpContext->SegFs);
  SetConsoleColor(dwCol2); ShowMsg("GS :    ");  SetConsoleColor(dwCol1); ShowMsg("%08X\r\n",lpContext->SegGs);
  SetConsoleColor(dwCol2); ShowMsg("CS :    ");  SetConsoleColor(dwCol1); ShowMsg("%08X\r\n",lpContext->SegCs);
  SetConsoleColor(dwCol2);
  ShowMsg("OF  DF  IF  SF  ZF  AF  PF  CF \r\n");
  SetConsoleColor(dwCol2);
  LPEflags lpEflags = (LPEflags)&lpContext->EFlags;
  ShowMsg("%02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X \r\n",
          lpEflags->OF, lpEflags->DF,lpEflags->IF, 
          lpEflags->SF, lpEflags->ZF, lpEflags->AF,
          lpEflags->PF, lpEflags->CF);
  SetConsoleColor(dwCol2);
}

void TDebugUI::ShowAsm(OUT LPASM_CODE lpAsmCode, IN DWORD dwAsmCnt)
{
  for(DWORD i=0; i<dwAsmCnt; i++)
  {
    SetConsoleColor(FOREGROUND_BLUE | FOREGROUND_RED  | FOREGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE| BACKGROUND_INTENSITY);
    ShowMsg("%p:   ", lpAsmCode[i].m_dwCodeAddr);

    SetConsoleColor(FOREGROUND_GREEN  | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE| BACKGROUND_INTENSITY);
    ShowMsg("%-16s",lpAsmCode[i].m_szOpcode);

    SetConsoleColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY |  BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE| BACKGROUND_INTENSITY);
    ShowMsg("%-20s\r\n",lpAsmCode[i].m_szASM);
  }

   SetConsoleColor(BACKGROUND_RED | BACKGROUND_GREEN| BACKGROUND_BLUE|BACKGROUND_INTENSITY);
  return ;
}

void  TDebugUI::ShowError()
{
  LPVOID lpMsgBuf;
  FormatMessage( 
    FORMAT_MESSAGE_ALLOCATE_BUFFER | 
    FORMAT_MESSAGE_FROM_SYSTEM | 
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    ::GetLastError(),
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
    (LPSTR) &lpMsgBuf,
    0,
    NULL 
    );
  ShowMsg("%s\r\n", lpMsgBuf);
  LocalFree(lpMsgBuf);
}

void TDebugUI::ShowMainMenu(void)
{
  //SetConsoleColor(BACKGROUND_INTENSITY|BACKGROUND_BLUE |BACKGROUND_GREEN| BACKGROUND_RED);
  system("cls");
  ShowMsg("============科锐第三阶段项目调试器============\r\n" 
          "   软件名称: TDebug   \r\n"
          "   作者   : 一路格桑花  \r\n"
          "   版本   : Ver0.3    \r\n"
          "   联系方式: changle517318@sohu.com"
          "                                          \r\n"
          "******************************************\r\n"
          "   1. Debug New Process\r\n"
          "   2. Debug Active Process\r\n"
          "   0. Quit\r\n"
          "******************************************\r\n");
  SetConsoleColor(FOREGROUND_RED | FOREGROUND_INTENSITY| BACKGROUND_RED | BACKGROUND_GREEN| BACKGROUND_BLUE|BACKGROUND_INTENSITY);
  ShowMsg("Choose your Option:");
  SetConsoleColor(BACKGROUND_RED | BACKGROUND_GREEN| BACKGROUND_BLUE|BACKGROUND_INTENSITY);
}

HANDLE  TDebugUI::GetConsoleHandle()
{
  if(g_hConsole == NULL)
  {
    g_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  }
  return g_hConsole;
}

BOOL TDebugUI::SetConsoleColor(WORD wAttributes)
{
  g_hConsole = GetConsoleHandle();
  if (g_hConsole == INVALID_HANDLE_VALUE)
    return FALSE;
  BOOL bResult = SetConsoleTextAttribute(g_hConsole, wAttributes);
  return bResult;
}


void TDebugUI::ShowMsg(const char *lpszMsg, ...)
{
    char szMsg[0x1000] = {0};
    va_list args;
    va_start(args, lpszMsg);
    vsnprintf_s(szMsg, sizeof(szMsg), lpszMsg, args);
    va_end(args);
    printf(szMsg);
}

void TDebugUI::ShowMemory(LPVOID lpStartAddr, BYTE* lpData, DWORD dwLen)
{
  TDebugUI::ShowMsg("Address : 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F   ASCII     STRING\r\n");
  TDebugUI::ShowMsg("----------------------------------------------------------------------------\r\n");
  for(int i=0; i<8; i++)
  {
    TDebugUI::ShowMsg("%08X: ", (DWORD)lpStartAddr + 16*i);
    for(int j=0; j<16; j++)
    {
      TDebugUI::ShowMsg("%02X ", *(lpData+16*i+j));
    }
    TDebugUI::ShowMsg("  ");
    for(int j=0; j<16; j++)
    {
      BYTE btData = *(lpData+16*i+j);
      if(btData < 32 || btData >126)
        btData = '.';
      TDebugUI::ShowMsg("%C", btData);
    }
    TDebugUI::ShowMsg("\r\n");
  }
  TDebugUI::ShowMsg("\r\n");
}