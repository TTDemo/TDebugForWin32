/************************************************************************/
/* ������      GDebugger
/* �����ռ�     NONE
/* �ļ���:     GDebugger.h GDebugger.cpp
/* ����ʱ��:    2016-08-05
/* ����:       Gesang
/* ��˵��:     ��������()
/************************************************************************/

#pragma once
#include <windows.h>
#include <list>
#include <map>
#include <psapi.h>
#include "BreakPoint.h"
#include "TDebugCmdline.h"

#pragma  comment(lib, "psapi.lib")

using namespace std;

class TDebug;
typedef DWORD (TDebug::*PFNDispatchEvent)(void);
typedef map<DWORD, PFNDispatchEvent>  DispatchEventMap;
#define DISPATCHEVENT(ID, pfn)  TDebug::g_DisEvMap[ID] = pfn;

class TDebug
{
public:
  static TDebug*  GetTDebug();
  static void DestotyDebug(); 
public:
  
  ~TDebug();
  DWORD DebugNewProcess(const char* lpstrApp);
  DWORD DebugActiveProcess(DWORD dwPID);

  BOOL GetTragetContext(LPTARGET_CONTEXT lpTargCon);
  virtual DWORD PauseTarget();
  virtual DWORD RunTarget();
  virtual DWORD FinishTarget();
  virtual BOOL  IsPause();
  virtual DWORD SetTrapFlag();
  
  virtual DWORD GetTargetPID();
  virtual DWORD GetTargetTID();
  virtual LPSTR GetTargetName();
  virtual VOID   NotifyTalk();
  virtual DWORD Quit(DWORD dwQuitCode);
private:
  virtual DWORD OnExceptionDispatch();   
  virtual DWORD OnCreateThread();     
  virtual DWORD OnCreateProcess();    
  virtual DWORD OnExitThread();       
  virtual DWORD OnExitProcess();      
  virtual DWORD OnLoadDLL();          
  virtual DWORD OnUnLoadDLL();        
  virtual DWORD OnOutputDebugString();
  virtual DWORD OnAccessViolation();  
  virtual DWORD OnBreakPoint();       
  virtual DWORD OnSingleStep();  

  TDebug();
  DWORD OnEvent(const LPDEBUG_EVENT lpDebugEv);
  DWORD DebugProcess();
  virtual VOID   ExitTalk();

  DWORD OnQuit();
 

private:
  char   m_szTargetName[MAX_PATH];
  HANDLE m_hCmdTread;
  DWORD m_dwTargetPID;
  DWORD m_dwTargetTID;
  BOOL  m_bPause;
  LPDEBUG_EVENT_CONTEXT m_pDEC;
  BOOL   m_bQuit;
  DWORD  m_dwQuitCode;
  //��̬��Ա��
  static DispatchEventMap  g_DisEvMap;
  static TDebug*        g_lpTDgber;

};

//������־
class TStepFlags
{
public:
  static TStepFlags*  GetStepFlags();
  static void     DestoryStepFlags();
private:
  TStepFlags()
  {
    m_bBpStep = FALSE;
    m_lpBpStepAddr = NULL;
    m_bStepIn = FALSE;
    m_bTraceStep = FALSE;
    m_lpTraceEndAddr = NULL;
    m_TraceFile = ::CreateFile("TraceLog.log",GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,
      NULL,CREATE_NEW, FILE_ATTRIBUTE_NORMAL,0);
    if(m_TraceFile !=INVALID_HANDLE_VALUE)
    {
      SetFilePointer(m_TraceFile,NULL, NULL, FILE_END);
    } 
  }
  ~TStepFlags()
  {
    SafeClose(m_TraceFile);
    m_TraceFile = NULL;
  }
private:
  static TStepFlags* g_lpDebugFlags;
public:
  BOOL m_bBpStep;
  LPVOID m_lpBpStepAddr;

  BOOL m_bStepIn;
  BOOL m_bTraceStep;
  LPVOID m_lpTraceEndAddr;
  HANDLE m_TraceFile;
};