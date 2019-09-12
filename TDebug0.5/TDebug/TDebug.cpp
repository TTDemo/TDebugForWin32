#include "TDebug.h"
#include "DllEvent.h"
#include "ExceptionEvent.h"
#include "ThreadProcessEvent.h"
#include "../TDebugUI/TDebugUI.h"

TDebug*        TDebug::g_lpTDgber = NULL;
DispatchEventMap  TDebug::g_DisEvMap;
TDebug*  TDebug::GetTDebug()
{
  if (g_lpTDgber == NULL)
  {
    g_lpTDgber = new TDebug();
    DISPATCHEVENT(EXCEPTION_DEBUG_EVENT,      &TDebug::OnExceptionDispatch)
    DISPATCHEVENT(CREATE_THREAD_DEBUG_EVENT,  &TDebug::OnCreateThread)
    DISPATCHEVENT(CREATE_PROCESS_DEBUG_EVENT, &TDebug::OnCreateProcess)
    DISPATCHEVENT(EXIT_THREAD_DEBUG_EVENT,    &TDebug::OnExitThread)
    DISPATCHEVENT(EXIT_PROCESS_DEBUG_EVENT,   &TDebug::OnExitProcess)
    DISPATCHEVENT(LOAD_DLL_DEBUG_EVENT,       &TDebug::OnLoadDLL)
    DISPATCHEVENT(UNLOAD_DLL_DEBUG_EVENT,     &TDebug::OnUnLoadDLL)
    DISPATCHEVENT(OUTPUT_DEBUG_STRING_EVENT,  &TDebug::OnOutputDebugString)
    DISPATCHEVENT(EXCEPTION_ACCESS_VIOLATION, &TDebug::OnAccessViolation)
    DISPATCHEVENT(EXCEPTION_BREAKPOINT,       &TDebug::OnBreakPoint)
    DISPATCHEVENT(EXCEPTION_SINGLE_STEP,      &TDebug::OnSingleStep)
  }
  return g_lpTDgber;
}

void TDebug::DestotyDebug()
{
  if(g_lpTDgber != NULL)
  {
     delete g_lpTDgber;
     g_lpTDgber = NULL;
     g_DisEvMap.clear();
  }
}


TDebug::TDebug()
{
  ZeroMemory(m_szTargetName, sizeof(m_szTargetName));
  m_hCmdTread = 0;
  m_dwTargetPID = 0;
  m_bPause = FALSE;
  m_bQuit = FALSE;
  m_dwQuitCode = QUIT_NOMAL;
  m_pDEC = new DEBUG_EVENT_CONTEXT();
  ZeroMemory(m_pDEC, 0);


}
TDebug::~TDebug()
{}


DWORD TDebug::DebugNewProcess(const char* lpstrApp)
{
  //////////////////////////////////////////////////////////////////////////
  //Select file u want to debug
  BOOL bRet = TRUE;
  PROCESS_INFORMATION pi  = {0};
  STARTUPINFO si			= {0};
  si.cb = sizeof(STARTUPINFO);

  strcpy_s(m_szTargetName, lpstrApp);
  bRet = ::CreateProcess(NULL, (LPSTR)lpstrApp, NULL, NULL, FALSE, 
    DEBUG_ONLY_THIS_PROCESS|CREATE_NEW_CONSOLE ,NULL, 
    NULL, &si, &pi);
  if(!bRet)
  {

    return FALSE;
  }
  this->DebugProcess();

  return TRUE;
}


DWORD TDebug::DebugActiveProcess(DWORD dwPID)
{
  BOOL bret = ::DebugActiveProcess(dwPID);
  if (bret)
  {
    this->DebugProcess();
    return TRUE;
  }
  return FALSE;
}

DWORD TDebug::PauseTarget()
{
  BOOL bRet =  ::SuspendProcess(m_dwTargetPID);
  if(bRet ==FALSE)
  {
    return FALSE;
  }
  m_bPause = TRUE;
  return bRet; 
}


DWORD TDebug::RunTarget()
{
  BOOL bRet = ::ResumeProcess(m_dwTargetPID);
  if(bRet== FALSE)
  {
    return FALSE;
  }
  m_bPause = FALSE;
  return TRUE;
}

DWORD TDebug::SetTrapFlag()
{
  CONTEXT       stContext;
  HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, m_dwTargetTID);
  if (NULL == hThread)
  {
    return FALSE;
  }

  stContext.ContextFlags = CONTEXT_ALL;
  if (GetThreadContext(hThread, &stContext) == FALSE)
  {
    return FALSE; 
  }
  stContext.EFlags |= 0x100;
  if(SetThreadContext(hThread, &stContext) == FALSE)
  {
    return FALSE;
  }
  ::SafeClose(hThread);
  return TRUE;
}






DWORD TDebug::GetTargetPID()
{
  return m_dwTargetPID;
}
DWORD TDebug::GetTargetTID()
{
  return m_dwTargetTID;
}

LPSTR TDebug::GetTargetName()
{
  return m_szTargetName;
}

DWORD TDebug::FinishTarget()
{
//  OpenProcess()
 // TerminateProcess();
  return ERRCODE_NOT_SUPPORT;
}

VOID TDebug::NotifyTalk()
{
  ::ResumeThread(m_hCmdTread);
}
VOID   TDebug::ExitTalk()
{
  ::TerminateThread(m_hCmdTread, 0);
}


BOOL  TDebug::IsPause()
{
  return m_bPause == TRUE;
}

BOOL TDebug::GetTragetContext(LPTARGET_CONTEXT lpTragetContext)
{
   HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_dwTargetPID);
   if (NULL == hProcess)
   {
     return FALSE;
   }
   lpTragetContext->hProcess = hProcess;

   HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, m_dwTargetTID);
   
   if (NULL == hThread)
   {
     return FALSE;
   }
   
   lpTragetContext->hThread = hThread;

   lpTragetContext->stContext.ContextFlags = CONTEXT_ALL;
   if (GetThreadContext(hThread, &lpTragetContext->stContext) == FALSE)
   {
     return FALSE;
   }
   return TRUE;
 }


DWORD WINAPI DebugCmdLineProc(LPVOID lpParam)
{
  TDebugCmdLine* lpDbgCmd = TDebugCmdLine::GetDebugCmdline();
  lpDbgCmd->Run();
  return 0;
}

DWORD TDebug::DebugProcess()
{
  DWORD dwResult          = NULL;                      
  DWORD dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED; 
  DEBUG_EVENT debugEv = {0};

  m_hCmdTread = CreateThread(NULL, NULL, DebugCmdLineProc, NULL, CREATE_SUSPENDED, NULL);
  while (!m_bQuit)
  {         
    ZeroMemory(&debugEv, sizeof(debugEv));
    dwResult = ::WaitForDebugEvent(&debugEv, INFINITE); 
    if (dwResult == NULL)
    {
      Quit(QUIT_WAITFORDEBUGEVENT);

      return FALSE;
    }
    
    m_dwTargetPID = debugEv.dwProcessId;
    m_dwTargetTID = debugEv.dwThreadId;
    TDebugCmdLine::PauseTalk();
    dwContinueStatus = OnEvent(&debugEv);
    TDebugCmdLine::ContinueTalk();
    dwResult = ContinueDebugEvent(debugEv.dwProcessId, debugEv.dwThreadId, dwContinueStatus);
    if (dwResult == NULL)
    {
      Quit(QUIT_WAITFORDEBUGEVENT);
      return FALSE;
    } 
  } 
  OnQuit();

  return TRUE;
}



DWORD TDebug::OnEvent(const LPDEBUG_EVENT lpDebugEv)
{
  HANDLE        hProcess = NULL;
  HANDLE        hThread  = NULL; 
  CONTEXT       tagContext  = {0};
  DispatchEventMap::iterator it;
  PFNDispatchEvent pfnEvent = NULL;
  DWORD dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;

  hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, lpDebugEv->dwProcessId);
  if (NULL == hProcess)
  {
    Quit(QUIT_OPENPROCESS);
    dwContinueStatus =  DBG_EXCEPTION_NOT_HANDLED;
  }

  hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, lpDebugEv->dwThreadId);
  if (NULL == hThread)
  {
    Quit(QUIT_OPENTHREAD);
    dwContinueStatus =  DBG_EXCEPTION_NOT_HANDLED;
    goto  SAFE_EXIT_PROC;
  }

  tagContext.ContextFlags = CONTEXT_ALL;
  if (GetThreadContext(hThread, &tagContext) == FALSE)
  {
    Quit(QUIT_GETTHREADCONTEXT);
    dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
    goto  SAFE_EXIT_PROC;
  }
  
  m_pDEC->hProcess = hProcess;
  m_pDEC->hThread =  hThread;
  m_pDEC->lpContext = &tagContext;
  m_pDEC->lpDebugEv =  lpDebugEv;


  it = g_DisEvMap.find(lpDebugEv->dwDebugEventCode);  
  if(it ==g_DisEvMap.end())
  {
    Quit(QUIT_NOTFOUND_EVENRDISPATCH);
    dwContinueStatus =DBG_EXCEPTION_NOT_HANDLED;
    goto SAFE_EXIT_PROC;
  }

  pfnEvent = (*it).second;
  dwContinueStatus = (this->*pfnEvent)();   

  if (SetThreadContext(hThread, &tagContext) == FALSE)
  {
      Quit(QUIT_SETTHREADCONTEXT);
      dwContinueStatus =DBG_EXCEPTION_NOT_HANDLED;
      goto SAFE_EXIT_PROC;
  }


SAFE_EXIT_PROC:
  if (hThread != NULL)
  {
    CloseHandle(hThread);
    hThread = NULL;
  }
  if(hProcess != NULL)
  {
    CloseHandle(hProcess);
    hProcess = NULL;
  }
  return dwContinueStatus;
}


DWORD TDebug::Quit(DWORD dwQuitCode)
{
  ExitTalk();
  m_dwQuitCode = dwQuitCode;
  m_bQuit = TRUE;

  return 0;
}
//根据退出码(退出原因)做不同的处理
DWORD TDebug::OnQuit()
{
  switch(m_dwQuitCode)
  {
  case QUIT_BASE:
    break;
  case QUIT_NOMAL:
    break;              
  case QUIT_OPENPROCESS:
    break;        
  case QUIT_OPENTHREAD:
    break;         
  case QUIT_GETTHREADCONTEXT:
    break;   
  case QUIT_SETTHREADCONTEXT:
    break;   
  case QUIT_CREATEPROCESS:
    break;   
  case QUIT_WAITFORDEBUGEVENT:
    break;
  case QUIT_CONTIUEDEBUGEVENT:
    break;
  }
  TDebug::DestotyDebug();
  TDebugUI::ShowMsg("TDebug Quit , Press Any Key To Main Menu\r\n");
  getchar();
  return 0;
}


/************************************************************************/
/* 
Function : dispatch exception event                                                                     */
/************************************************************************/
DWORD TDebug::OnExceptionDispatch()
{
    HANDLE        hProcess  =  m_pDEC->hProcess;
    HANDLE        hThread   =  m_pDEC->hThread; 
    LPCONTEXT     lpContext =  m_pDEC->lpContext;
    LPDEBUG_EVENT lpDebugEv =  m_pDEC->lpDebugEv;

    map<DWORD, PFNDispatchEvent>::iterator it;
    map<DWORD, PFNDispatchEvent>::iterator itend = g_DisEvMap.end();
    PFNDispatchEvent pfnEvent = NULL;
    DWORD dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;

    it = g_DisEvMap.find(lpDebugEv->u.Exception.ExceptionRecord.ExceptionCode);
    if (it != itend)
    {
        pfnEvent = (*it).second;
        dwContinueStatus = (this->*pfnEvent)();   
    }
    return dwContinueStatus;
}

/************************************************************************/
/* 
Function : all these used for event dispatch,
          dispatch into different event processing functions
1) Create(/Exit)Process(/Thread) --->CProcessEvent
2) Load(/Unload)Dll --> CDllEvent
3) DebugString --> CDllEvent
4) Exception (BreakPoint, AccessViolation, SingleStep) --> CExceptEvent*/
/************************************************************************/
DWORD TDebug::OnCreateThread()
{
  CThreadProcessEvent mTPE(this, m_pDEC);
  return mTPE.OnCreateThread();
}

DWORD TDebug::OnCreateProcess()
{
  CThreadProcessEvent mTPE(this, m_pDEC);
  return mTPE.OnCreateProcess();
}

DWORD TDebug::OnExitThread()
{
  CThreadProcessEvent mTPE(this, m_pDEC);
  return mTPE.OnExitThread();
}

DWORD TDebug::OnExitProcess()
{
  CThreadProcessEvent mTPE(this, m_pDEC);
  return mTPE.OnExitProcess();
}

DWORD TDebug::OnLoadDLL()
{    
    CDLLEvent objDllEvent(this, m_pDEC);
    return objDllEvent.OnLoad();
}

DWORD TDebug::OnUnLoadDLL()
{
  CDLLEvent objDllEvent(this, m_pDEC);
  return objDllEvent.OnUnload();
}

DWORD TDebug::OnOutputDebugString()
{
    CDLLEvent objDllEvent(this, m_pDEC);
    return objDllEvent.OnOutputString();
}


DWORD TDebug::OnAccessViolation()
{
  CExceptionEvent objExceptEv(this, m_pDEC);
  return objExceptEv.OnAccessViolation();
}

DWORD TDebug::OnBreakPoint()
{
  CExceptionEvent objExceptEv(this, m_pDEC);
  return objExceptEv.OnBreakPoint();
}

DWORD TDebug::OnSingleStep()
{
  CExceptionEvent objExceptEv(this, m_pDEC);
  return objExceptEv.OnSingleStep();
}




TStepFlags* TStepFlags::g_lpDebugFlags = NULL;

TStepFlags*  TStepFlags::GetStepFlags()
{
  if(g_lpDebugFlags == NULL)
  {
    g_lpDebugFlags = new TStepFlags();
  }
  return g_lpDebugFlags;
}

void  TStepFlags::DestoryStepFlags()
{
  if(g_lpDebugFlags != NULL)
  {
    delete g_lpDebugFlags;
    g_lpDebugFlags = NULL;
  }
}