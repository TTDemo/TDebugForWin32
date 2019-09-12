/************************************************************************/
/* �����ƣ�     TDebugCmd
/* �����ռ�     NONE
/* �ļ�����     TDebugCmd.h TDebugCmd.cpp
/* ����ʱ��:    2016-08-05
/* ����:       Gesang
/* ��˵��:     ��������()
/************************************************************************/
#pragma once
#include "ICmdline.h"
#include "../Comm/TDebugFunc.h"
#include "../TDebug/TDebug.h"

using namespace std;

class Compare
{
public:
  bool operator() (const char * pszSRC, const char * pszDST) const
  {
    return strcmp(pszSRC, pszDST) < 0;
  }
};

class TDebugCmdLine;

typedef DWORD (TDebugCmdLine::*PFNDispatchCmd)(Command& cmd);
typedef map<const char *, PFNDispatchCmd, Compare> DispatchCmdMap; 
#define DISPATCHCMD(str, pfn)  TDebugCmdLine::g_DisCmdMap[str] = pfn;

class TDebugCmdLine : public ICmdline
{
private:
  TDebugCmdLine();
public:
  ~TDebugCmdLine();
  static  TDebugCmdLine*  GetDebugCmdline();
  static  void DestoryDebugCmdline();
public:
  // �û����������ӿ���
  void Run();
  static void ContinueTalk();
  static void PauseTalk();

private:
  void  ParseCommand(const string& cmdLine, Command& cmd) ;
  DWORD ExcuteCmd(Command& cmd);
  BOOL  SelectFile(char *pszPath, SIZE_T nMaxPath = MAX_PATH);
private:

  virtual DWORD DoShowHelp(Command& cmd) ;         //�������0 
  virtual DWORD DoStepInto(Command& cmd);          //�������1 
  virtual DWORD DoStepOver(Command& cmd);          //�������2 
  virtual DWORD DoGo(Command& cmd);                //�������3 
  virtual DWORD DoTrace(Command& cmd);             //�������3.5
  virtual DWORD DoShowASM(Command& cmd)  ;         //�������4
  virtual DWORD DoShowData(Command& cmd) ;         //�������5 
  virtual DWORD DoShowRegs(Command& cmd) ;         //�������6
  virtual DWORD DoEditData(Command& cmd) ;         //�������7

  virtual DWORD DoSoftBreakPoint(Command& cmd);    //�������8
  virtual DWORD DoListAllSoftBP(Command& cmd);     //�������9
  virtual DWORD DoClearSoftBP(Command& cmd);       //�������10

  virtual DWORD DoHardBreakPoint(Command& cmd);    //�������11
  virtual DWORD DoListAllHardBP(Command& cmd);     //�������12
  virtual DWORD DoClearHardBP(Command& cmd);       //�������13


  virtual DWORD DoMemBreakPoint(Command& cmd);    //�������14
  virtual DWORD DoListAllMemBP(Command& cmd);     //�������15
  virtual DWORD DoListMemBpPage(Command& cmd);             //�������16
  virtual DWORD DoClearMemBP(Command& cmd);       //�������17

  virtual DWORD DoLoadScript(Command& cmd);       //�������18  
  virtual DWORD DoExportScript(Command& cmd);     //�������19  
  virtual DWORD DoFinish(Command& cmd);           //�������20  
  virtual DWORD DoListModule(Command& cmd);       //�������21  
  virtual DWORD DoHistory(Command& cmd);          //�������22
  virtual DWORD DoShowCallStack(Command& cmd);    //�������23

  static TDebugCmdLine*  g_lpTDebugCmdline ;
  static DispatchCmdMap  g_DisCmdMap;
  static HANDLE g_hTalk;
private:
  BOOL m_bQuit;
  string m_strHisCmds;
  

};


