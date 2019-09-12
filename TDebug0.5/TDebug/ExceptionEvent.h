#pragma once
#include "TDebug.h"

class CExceptionEvent
{
public:
  CExceptionEvent(TDebug* lpDbger, LPDEBUG_EVENT_CONTEXT lpDEC)
  {
    m_lpDbger = lpDbger;
    m_lpDEC = lpDEC;
  }
  //��Ӧ�쳣����
  DWORD OnBreakPoint();
  DWORD OnSingleStep();
  DWORD OnAccessViolation();
private:
  void OnSysInitBP(); 

private:

  TDebug* m_lpDbger;
  LPDEBUG_EVENT_CONTEXT m_lpDEC;

  static BOOL g_bSysInitBP; 
};




