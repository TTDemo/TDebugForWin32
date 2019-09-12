#pragma once
#include "TDebug.h"


class CDLLEvent 
{
public:
  CDLLEvent(  TDebug* lpDbger, LPDEBUG_EVENT_CONTEXT lpDEC)
  {
    m_lpDbger = lpDbger;
    m_lpDEC = lpDEC;
  }
  virtual DWORD OnUnload();
  virtual DWORD OnLoad();
  virtual DWORD OnOutputString();
private:
  TDebug* m_lpDbger;
  LPDEBUG_EVENT_CONTEXT m_lpDEC;
};




