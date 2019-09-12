/************************************************************************/
/* 类名称：     TDebugCmd
/* 命名空间     NONE
/* 文件名：     TDebugCmd.h TDebugCmd.cpp
/* 创建时间:    2016-08-05
/* 作者:       Gesang
/* 类说明:     调试器类()
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
  // 用户交互操作接口类
  void Run();
  static void ContinueTalk();
  static void PauseTalk();

private:
  void  ParseCommand(const string& cmdLine, Command& cmd) ;
  DWORD ExcuteCmd(Command& cmd);
  BOOL  SelectFile(char *pszPath, SIZE_T nMaxPath = MAX_PATH);
private:

  virtual DWORD DoShowHelp(Command& cmd) ;         //功能序号0 
  virtual DWORD DoStepInto(Command& cmd);          //功能序号1 
  virtual DWORD DoStepOver(Command& cmd);          //功能序号2 
  virtual DWORD DoGo(Command& cmd);                //功能序号3 
  virtual DWORD DoTrace(Command& cmd);             //功能序号3.5
  virtual DWORD DoShowASM(Command& cmd)  ;         //功能序号4
  virtual DWORD DoShowData(Command& cmd) ;         //功能序号5 
  virtual DWORD DoShowRegs(Command& cmd) ;         //功能序号6
  virtual DWORD DoEditData(Command& cmd) ;         //功能序号7

  virtual DWORD DoSoftBreakPoint(Command& cmd);    //功能序号8
  virtual DWORD DoListAllSoftBP(Command& cmd);     //功能序号9
  virtual DWORD DoClearSoftBP(Command& cmd);       //功能序号10

  virtual DWORD DoHardBreakPoint(Command& cmd);    //功能序号11
  virtual DWORD DoListAllHardBP(Command& cmd);     //功能序号12
  virtual DWORD DoClearHardBP(Command& cmd);       //功能序号13


  virtual DWORD DoMemBreakPoint(Command& cmd);    //功能序号14
  virtual DWORD DoListAllMemBP(Command& cmd);     //功能序号15
  virtual DWORD DoListMemBpPage(Command& cmd);             //功能序号16
  virtual DWORD DoClearMemBP(Command& cmd);       //功能序号17

  virtual DWORD DoLoadScript(Command& cmd);       //功能序号18  
  virtual DWORD DoExportScript(Command& cmd);     //功能序号19  
  virtual DWORD DoFinish(Command& cmd);           //功能序号20  
  virtual DWORD DoListModule(Command& cmd);       //功能序号21  
  virtual DWORD DoHistory(Command& cmd);          //功能序号22
  virtual DWORD DoShowCallStack(Command& cmd);    //功能序号23

  static TDebugCmdLine*  g_lpTDebugCmdline ;
  static DispatchCmdMap  g_DisCmdMap;
  static HANDLE g_hTalk;
private:
  BOOL m_bQuit;
  string m_strHisCmds;
  

};


