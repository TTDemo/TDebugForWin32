#include <stdio.h>
#include "DllEvent.h"
#include "BreakPoint.h"

DWORD CDLLEvent::OnUnload()
{
  DWORD dwContinueStatus = DBG_CONTINUE;

  return dwContinueStatus;
}

DWORD CDLLEvent::OnLoad()
{
  //HANDLE        hProcess  = lpDEC->hProcess;
  //HANDLE        hThread   = lpDEC->hThread; 
  //CONTEXT*      lpContext = lpDEC->lpContext;
  //LOAD_DLL_DEBUG_INFO& LoadDll = lpDEC->lpDebugEv->u.LoadDll;
  //
  //DWORD dwResult =0;
  DWORD dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
  //char szDllName[MAX_PATH];
  //DWORD dwRealAddr = 0;
  //DWORD dwRead = 0;

  //ZeroMemory(szDllName, sizeof(szDllName));

  //dwResult = ReadProcessMemory(hProcess, LoadDll.lpImageName, &dwRealAddr, 4, &dwRead);
  //if ( dwResult == FALSE)
  //{
  //  return dwContinueStatus;
  //}
  //dwResult = ReadProcessMemory(hProcess, (LPVOID)dwRealAddr, szDllName, sizeof(szDllName), NULL);
  //if(dwResult == NULL)
  //{
  //  return dwContinueStatus;
  //}
  //if(LoadDll.fUnicode == 0)
  //{
  //  printf("Load Dll %s\r\n",szDllName);
  //}
  //else
  //{
  //  printf("Load Dll %ls\r\n",szDllName);
  //}
  dwContinueStatus = DBG_CONTINUE;

  return dwContinueStatus;
}


DWORD CDLLEvent::OnOutputString()
{
  DWORD dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;

  return dwContinueStatus;
}

