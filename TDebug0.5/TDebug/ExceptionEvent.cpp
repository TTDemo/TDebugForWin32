#include "ExceptionEvent.h"
#include "BreakPoint.h"
#include "../TDebugUI/TDebugUI.h"


BOOL CExceptionEvent:: g_bSysInitBP = TRUE;

DWORD CExceptionEvent::OnBreakPoint()
{
  HANDLE        hProcess  = m_lpDEC->hProcess;
  HANDLE        hThread   = m_lpDEC->hThread; 
  CONTEXT*      lpContext = m_lpDEC->lpContext;
  LPDEBUG_EVENT lpDebugEv = m_lpDEC->lpDebugEv;
  EXCEPTION_DEBUG_INFO& stExceptionInfo = m_lpDEC->lpDebugEv->u.Exception;
  DWORD dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;

  TStepFlags* lpDbgFlags = TStepFlags::GetStepFlags();
  TBPManager* lpBpManger = TBPManager::GetBPManager();
  TDebugCmdLine* lpCmdLine = TDebugCmdLine::GetDebugCmdline();
  LPVOID lpBpAddr = stExceptionInfo.ExceptionRecord.ExceptionAddress;
  if (g_bSysInitBP == TRUE)
  {
    g_bSysInitBP = FALSE;
    OnSysInitBP();
    m_lpDbger->PauseTarget();
    return DBG_CONTINUE;
  }
  // 用户BP 断点

  TSoftBreakPoint* lpSoftBp = lpBpManger->FindSoftBP(lpDebugEv->dwProcessId, lpBpAddr);
  if(lpSoftBp != NULL )
  {
    lpSoftBp->Disable();
    TDebugUI::ShowMsg("Soft BreakPoint Hit , Address: %p \r\n", lpSoftBp->m_lpBpAddr);
    ASM_CODE asmCode[1];
    GetAsmCode(hProcess, lpBpAddr, asmCode, 1);
    TDebugUI::ShowAsm(asmCode, 1);

    lpContext->Eip = (DWORD)lpBpAddr;
    lpContext->EFlags |= 0x100;

    lpDbgFlags->m_bBpStep = TRUE;
    lpDbgFlags->m_lpBpStepAddr = lpBpAddr;
    return DBG_CONTINUE;
  }

  // 调试器内部BP 断点
  TInnerBreakPoint* lpInnerBp = lpBpManger->FindInnerBP(lpDebugEv->dwProcessId, lpBpAddr);
  if(lpInnerBp != NULL)
  {
    if(lpInnerBp->m_dwType == INNER_BP_GO)
    {
      lpBpManger->DelInnerBP(lpDebugEv->dwProcessId, lpBpAddr);
      ASM_CODE asmCode[1];
      GetAsmCode(hProcess, lpBpAddr, asmCode, 1);
      TDebugUI::ShowAsm(asmCode, 1);
      lpContext->Eip = (DWORD)lpBpAddr;
      m_lpDbger->PauseTarget();
      return DBG_CONTINUE;
    }
    else if(lpInnerBp->m_dwType == INNER_BP_STEPOVER)
    {
      lpBpManger->DelInnerBP(lpDebugEv->dwProcessId, lpBpAddr);
      ASM_CODE asmCode[1];
      GetAsmCode(hProcess, lpBpAddr, asmCode, 1);
      TDebugUI::ShowAsm(asmCode, 1);
      lpContext->Eip = (DWORD)lpBpAddr;
      m_lpDbger->PauseTarget();
      return DBG_CONTINUE;
    }
    else if(lpInnerBp->m_dwType == INNER_BP_TRACE)
    {
      lpBpManger->DelInnerBP(lpDebugEv->dwProcessId, lpBpAddr);
      lpContext->EFlags |= 0x100;
      lpDbgFlags->m_bTraceStep = TRUE;
      lpContext->Eip = (DWORD)lpBpAddr;
      return DBG_CONTINUE;
    }
  }
  return DBG_EXCEPTION_NOT_HANDLED;
}



extern BOOL g_isMemBPStep;
extern TMemBreakPoint* g_lpMemBPStep;
DWORD CExceptionEvent::OnSingleStep()
{
  static BOOL g_bResetHardBP = FALSE;
  static LPVOID g_lpResetHardBPAddr = NULL;
  TDebugCmdLine* lpCmdLine = TDebugCmdLine::GetDebugCmdline();
  
  HANDLE        hProcess  = m_lpDEC->hProcess;
  HANDLE        hThread   = m_lpDEC->hThread; 
  CONTEXT*      lpContext = m_lpDEC->lpContext;
  LPDEBUG_EVENT lpDebugEv = m_lpDEC->lpDebugEv;
  DWORD dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;

  LPVOID lpBpAddr = lpDebugEv->u.Exception.ExceptionRecord.ExceptionAddress;
  TStepFlags* lpDbgFlags = TStepFlags::GetStepFlags();
  
  if(lpDbgFlags->m_bTraceStep)
  {
    char szAsmBuf[80];
    char szWritebuf[80];
    DWORD dwWrittenByte;
    ::GetAsmStr(hProcess, lpBpAddr, szAsmBuf);
    sprintf(szWritebuf, "%08X: %s \r\n", lpBpAddr, szAsmBuf);
    ::WriteFile(lpDbgFlags->m_TraceFile, szWritebuf, strlen(szWritebuf),&dwWrittenByte, NULL);
    if(lpBpAddr == lpDbgFlags->m_lpTraceEndAddr)
    {
      lpDbgFlags->m_lpTraceEndAddr = NULL;
      lpDbgFlags->m_bTraceStep = FALSE;
    }
    else
    {
       lpContext->EFlags |= 0x100;
       lpDbgFlags->m_bTraceStep = TRUE;
    }
    return DBG_CONTINUE;
  }

  if (lpDbgFlags->m_bStepIn)
  {
    lpDbgFlags->m_bStepIn =FALSE;
    ASM_CODE asmCode[1];
    GetAsmCode(hProcess, lpBpAddr, asmCode, 1);
    TDebugUI::ShowAsm(asmCode, 1);
    m_lpDbger->PauseTarget();
    dwContinueStatus = DBG_CONTINUE;
    return dwContinueStatus;
  }

  if(lpDbgFlags->m_bBpStep)
  {
    LPVOID lpBpAddr = lpDbgFlags->m_lpBpStepAddr;

    lpDbgFlags->m_bBpStep = FALSE;
    lpDbgFlags->m_lpBpStepAddr = NULL;

    TBPManager* lpBpManger = TBPManager::GetBPManager();

    TSoftBreakPoint* lpSoftBp = lpBpManger->FindSoftBP(lpDebugEv->dwProcessId, lpBpAddr);
    if(lpSoftBp != NULL )
    {
      lpSoftBp->Enable();
      m_lpDbger->PauseTarget();
    }
    return DBG_CONTINUE;
  }

  if(g_isMemBPStep == TRUE)
  {
    TBPManager* lpBpManger = TBPManager::GetBPManager();
    lpBpManger->EnableMemBp(g_lpMemBPStep);
    g_lpMemBPStep = NULL;
    g_isMemBPStep = FALSE;
    return DBG_CONTINUE;
  }

  LPDR6 lpDR6 = NULL;
  LPDR7 lpDR7 = NULL;

  TBPManager* lpBpManger = TBPManager::GetBPManager();
  THardBreakPoint* lpHardBp = NULL;
  lpDR6 = (LPDR6)&lpContext->Dr6;
  if (lpDR6->BS == 0) 
  {
    lpHardBp = lpBpManger->FindHardBP(lpContext->Dr6);
    if(lpHardBp == NULL)
    {
      return DBG_EXCEPTION_NOT_HANDLED;
    }
    if(lpHardBp->m_dwBPType == BP_ACTION_EXCUTE)
    {
      lpDR7 = (LPDR7)&lpContext->Dr7;
      lpDR7->L0 = 0;
      lpContext->EFlags |= 0x100;
      g_bResetHardBP = TRUE;
      g_lpResetHardBPAddr = lpHardBp->m_lpBpAddr; 
    }
    else
    {
      m_lpDbger->PauseTarget();
      TDebugUI::ShowMsg("Hard BreakPoint Hit , Address: %p  Type%d: \r\n", 
        lpHardBp->m_lpBpAddr, lpHardBp->m_dwBPType );
      ASM_CODE asmCode[1];
      GetAsmCode(hProcess, lpBpAddr, asmCode, 1);
      TDebugUI::ShowAsm(asmCode, 1);
    }
    return DBG_CONTINUE ;
  }
  //硬件执行断点
  if (g_bResetHardBP == TRUE)
  {
    g_bResetHardBP = FALSE;

    LPVOID lpBpAddr = g_lpResetHardBPAddr;
    g_lpResetHardBPAddr = NULL;;
    TBPManager* lpBpManger = TBPManager::GetBPManager();
    THardBreakPoint* lpHardBp = lpBpManger->FindHardBP(lpDebugEv->dwProcessId, lpBpAddr);
    if(lpHardBp != NULL )
    {
      ::EnableHardBP(lpContext, lpHardBp->m_lpBpAddr, lpHardBp->m_dwReg, lpHardBp->m_dwBPType, lpHardBp->m_dwLen);
      m_lpDbger->PauseTarget();
      
      TDebugUI::ShowMsg("Hard BreakPoint Hit , Address: %p  Type%d: \r\n", 
        lpHardBp->m_lpBpAddr, lpHardBp->m_dwBPType );
      ASM_CODE asmCode[1];
      GetAsmCode(hProcess, lpBpAddr, asmCode, 1);
      TDebugUI::ShowAsm(asmCode, 1);
    }
    return DBG_CONTINUE;
  }
  return DBG_EXCEPTION_NOT_HANDLED;
}

BOOL g_isMemBPStep = FALSE;
TMemBreakPoint* g_lpMemBPStep = NULL;
DWORD CExceptionEvent::OnAccessViolation()
{
  HANDLE        hProcess  = m_lpDEC->hProcess;
  HANDLE        hThread   = m_lpDEC->hThread; 
  CONTEXT*      lpContext = m_lpDEC->lpContext;
  LPDEBUG_EVENT lpDebugEv = m_lpDEC->lpDebugEv;
  DWORD dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
  LPVOID lpBpAddr = (LPVOID)lpDebugEv->u.Exception.ExceptionRecord.ExceptionAddress;

  EXCEPTION_RECORD* pRecord = &lpDebugEv->u.Exception.ExceptionRecord;
  
  DWORD dwActionType = pRecord->ExceptionInformation[0];
  DWORD dwDestAddr = pRecord->ExceptionInformation[1]; 
  DWORD dwDestBase = dwDestAddr & 0xfffff000;
  LPPage  lpPageData  = NULL;
  TMemBreakPoint* lpMemBPData = NULL;
  TBPManager* lpBpManger = TBPManager::GetBPManager();

  /*
    判断操作目标地址是否在 某个页内
  */
  lpPageData = lpBpManger->FindPageDataList(dwDestAddr);

  if (lpPageData != NULL)
  {
    //判断断点是否命中
    TMemBreakPoint* lpHitMemBp = lpBpManger->FindHitMemBP(dwDestAddr, dwActionType);
    if (lpHitMemBp == NULL)
    {
        return DBG_EXCEPTION_NOT_HANDLED; 
    }
    m_lpDbger->PauseTarget();
    TDebugUI::ShowMsg("Mem  BreakPoint Hit:  EIP:%08X Adddress:%08X, ActionType:%d\n", 
                      lpContext->Eip,
                      lpHitMemBp->m_lpBPAddr, 
                      dwActionType);

    ASM_CODE asmCode[1];
    GetAsmCode(hProcess, lpBpAddr, asmCode, 1);
    TDebugUI::ShowAsm(asmCode, 1);
    
    lpBpManger->DisableMemBp(lpHitMemBp);
    //恢复内存保护属性
    g_isMemBPStep = TRUE;
    g_lpMemBPStep = lpHitMemBp;
    lpContext->EFlags |= 0x100;
    dwContinueStatus = DBG_CONTINUE; 
  }

  return dwContinueStatus;
}


void  CExceptionEvent::OnSysInitBP()
{
  HANDLE        hProcess  = m_lpDEC->hProcess;
  HANDLE        hThread   = m_lpDEC->hThread; 
  CONTEXT*      lpContext = m_lpDEC->lpContext;
  EXCEPTION_DEBUG_INFO& stExceptionInfo = m_lpDEC->lpDebugEv->u.Exception;

  LPVOID lpBpAddr = stExceptionInfo.ExceptionRecord.ExceptionAddress;
  
  ASM_CODE asmCode[2]={0};
  GetAsmCode(hProcess, lpBpAddr, asmCode, 2);
  TDebugUI::ShowAsm(asmCode, 2);
  m_lpDbger->NotifyTalk();
}
