#pragma once
#include "TDebug.h"
#include "../Comm/TDebugFunc.h"


class CThreadProcessEvent 
{
public:
  CThreadProcessEvent(TDebug* lpDbger, LPDEBUG_EVENT_CONTEXT lpDEC)
  {
    m_lpDbger = lpDbger;
    m_lpDEC = lpDEC;
  }

public:
  virtual DWORD  OnCreateProcess();
  virtual DWORD  OnExitProcess();
  virtual DWORD  OnCreateThread();
  virtual DWORD  OnExitThread();

private:
  TDebug* m_lpDbger;
  LPDEBUG_EVENT_CONTEXT m_lpDEC;
};
