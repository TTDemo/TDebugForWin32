// TDebug.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <Commdlg.h>
#include "TDebug/TDebug.h"
#include "TDebugUI/TDebugUI.h"

//主菜单选项
DWORD Menu_DebugNewProcess();
DWORD Menu_DebugActiveProcess();
DWORD Menu_DoQuit();


int main(int argc, char* argv[])
{
  BOOL bQuit = FALSE;
  char ch = 0;
  while(!bQuit)
  {
    TDebugUI::ShowMainMenu();
    ch = getchar();  
    fflush(stdin);

    switch(ch)
    {
    case '1':
      Menu_DebugNewProcess();
      break;
    case '2':
      Menu_DebugActiveProcess();
      break;
    case '0':
      bQuit = TRUE;
      break;
    }
  }
  TDebugUI::ShowMsg("按任意键退出\r\n");
  system("pause");
	return 0;
}





DWORD Menu_DebugNewProcess()
{
  DWORD dwTid = NULL;
  static char szFilePath[MAX_PATH];
  HANDLE hTread = NULL;
  if(SelectFile(szFilePath, sizeof(szFilePath)))
  {  
    TDebug* lpDgber = TDebug::GetTDebug();
    lpDgber->DebugNewProcess(szFilePath); 
  }
  return ERRCODE_SUCESS;
}

DWORD Menu_DebugActiveProcess()
{
  DWORD dwTid = NULL;
  HANDLE hTread = NULL;
  DWORD dwPID = 0;
  TDebugUI::ShowMsg("Please Enter the PID:");
  system("taskmgr");
  scanf("%d", &dwPID);
  fflush(stdin);
  if((dwPID == 0 || dwPID == ULONG_MAX))
  {
    TDebugUI::ShowMsg("PID Error! !!\r\n");
  }
  TDebug* lpDgber = TDebug::GetTDebug();
  if(FALSE == lpDgber->DebugActiveProcess(dwPID))
  {
     TDebugUI::ShowError();
     TDebugUI::ShowMsg("Open %d Active Process Failed\r\n", dwPID);
     return ERRCODE_FAILED;
  }
  return ERRCODE_SUCESS;
}


DWORD Menu_Quit()
{
  return ERRCODE_SUCESS;
}

