#pragma once
#include <vector>
#include <string>
#include <windows.h>

using namespace  std;
typedef vector<string> Command;

class ICmdline
{
public:
  // 用户交互操作接口类
  virtual DWORD DoShowHelp(Command& cmd) = 0;         //功能序号0 
  virtual DWORD DoStepInto(Command& cmd)= 0;          //功能序号1 
  virtual DWORD DoStepOver(Command& cmd)= 0;          //功能序号2 
  virtual DWORD DoGo(Command& cmd)= 0;                //功能序号3 
  virtual DWORD DoTrace(Command& cmd)= 0;             //功能序号3.5
  virtual DWORD DoShowASM(Command& cmd)  = 0;         //功能序号4
  virtual DWORD DoShowData(Command& cmd) = 0;         //功能序号5 
  virtual DWORD DoShowRegs(Command& cmd) = 0;         //功能序号6
  virtual DWORD DoEditData(Command& cmd) = 0;         //功能序号7
 
  virtual DWORD DoSoftBreakPoint(Command& cmd) =0;    //功能序号8
  virtual DWORD DoListAllSoftBP(Command& cmd)  =0;    //功能序号9
  virtual DWORD DoClearSoftBP(Command& cmd)  =0;      //功能序号10

  virtual DWORD DoHardBreakPoint(Command& cmd) =0;    //功能序号11
  virtual DWORD DoListAllHardBP(Command& cmd)  =0;    //功能序号12
  virtual DWORD DoClearHardBP(Command& cmd)  = 0;     //功能序号13

  
  virtual DWORD DoMemBreakPoint(Command& cmd) =0;    //功能序号14
  virtual DWORD DoListAllMemBP(Command& cmd)  =0;    //功能序号15
  virtual DWORD DoListMemBpPage(Command& cmd) = 0;            //功能序号16
  virtual DWORD DoClearMemBP(Command& cmd)  = 0;     //功能序号17

  virtual DWORD DoLoadScript(Command& cmd) = 0;      //功能序号18  
  virtual DWORD DoExportScript(Command& cmd) = 0;    //功能序号19  
  virtual DWORD DoFinish(Command& cmd) = 0;          //功能序号20  
  virtual DWORD DoListModule(Command& cmd) = 0;      //功能序号21   
  virtual DWORD DoHistory(Command& cmd) = 0;
  virtual DWORD DoShowCallStack(Command& cmd) = 0;    //功能序号23
};

