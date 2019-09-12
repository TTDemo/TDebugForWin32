#include "ThreadProcessEvent.h"
#include "BreakPoint.h"
#include "../TDebugUI/TDebugUI.h"

DWORD  CThreadProcessEvent::OnCreateProcess()
{
  HANDLE        hProcess  = m_lpDEC->hProcess;
  HANDLE        hThread   = m_lpDEC->hThread; 
  CONTEXT*      lpContext = m_lpDEC->lpContext;
  LPDEBUG_EVENT lpDebugEv = m_lpDEC->lpDebugEv;
  CREATE_PROCESS_DEBUG_INFO& pi = m_lpDEC->lpDebugEv->u.CreateProcessInfo;
 
  TDebugUI::ShowMsg("**************************************************************************\r\n");
  TDebugUI::ShowMsg("��������:%s \r\n",  m_lpDbger->GetTargetName());
  TDebugUI::ShowMsg("��ڵ�ַ:%p \r\n", pi.lpStartAddress);
  TDebugUI::ShowMsg("ģ���ַ:%p \r\n", pi.lpBaseOfImage);
  TDebugUI::ShowMsg("����ID  :%p \r\n", lpDebugEv->dwProcessId);
  TDebugUI::ShowMsg("���߳�ID:%p \r\n",lpDebugEv->dwThreadId);
  TDebugUI::ShowMsg("**************************************************************************\r\n");
  TDebugUI::ShowMsg("\r\n");

  return DBG_CONTINUE;
}
DWORD  CThreadProcessEvent::OnExitProcess()
{
  DWORD dwContinueStatus = DBG_CONTINUE;
 
  TStepFlags::DestoryStepFlags();
  TBPManager::DestotyBPManager();
 
  m_lpDbger->Quit(QUIT_NOMAL);
  TDebugCmdLine::DestoryDebugCmdline();
  return dwContinueStatus;
}
DWORD  CThreadProcessEvent::OnCreateThread()
{
  DWORD dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;

  return dwContinueStatus;
}
DWORD  CThreadProcessEvent::OnExitThread()
{
  DWORD dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;

  return dwContinueStatus;
}