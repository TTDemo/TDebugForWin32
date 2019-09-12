#pragma once
#include <vector>
#include <string>
#include <windows.h>

using namespace  std;
typedef vector<string> Command;

class ICmdline
{
public:
  // �û����������ӿ���
  virtual DWORD DoShowHelp(Command& cmd) = 0;         //�������0 
  virtual DWORD DoStepInto(Command& cmd)= 0;          //�������1 
  virtual DWORD DoStepOver(Command& cmd)= 0;          //�������2 
  virtual DWORD DoGo(Command& cmd)= 0;                //�������3 
  virtual DWORD DoTrace(Command& cmd)= 0;             //�������3.5
  virtual DWORD DoShowASM(Command& cmd)  = 0;         //�������4
  virtual DWORD DoShowData(Command& cmd) = 0;         //�������5 
  virtual DWORD DoShowRegs(Command& cmd) = 0;         //�������6
  virtual DWORD DoEditData(Command& cmd) = 0;         //�������7
 
  virtual DWORD DoSoftBreakPoint(Command& cmd) =0;    //�������8
  virtual DWORD DoListAllSoftBP(Command& cmd)  =0;    //�������9
  virtual DWORD DoClearSoftBP(Command& cmd)  =0;      //�������10

  virtual DWORD DoHardBreakPoint(Command& cmd) =0;    //�������11
  virtual DWORD DoListAllHardBP(Command& cmd)  =0;    //�������12
  virtual DWORD DoClearHardBP(Command& cmd)  = 0;     //�������13

  
  virtual DWORD DoMemBreakPoint(Command& cmd) =0;    //�������14
  virtual DWORD DoListAllMemBP(Command& cmd)  =0;    //�������15
  virtual DWORD DoListMemBpPage(Command& cmd) = 0;            //�������16
  virtual DWORD DoClearMemBP(Command& cmd)  = 0;     //�������17

  virtual DWORD DoLoadScript(Command& cmd) = 0;      //�������18  
  virtual DWORD DoExportScript(Command& cmd) = 0;    //�������19  
  virtual DWORD DoFinish(Command& cmd) = 0;          //�������20  
  virtual DWORD DoListModule(Command& cmd) = 0;      //�������21   
  virtual DWORD DoHistory(Command& cmd) = 0;
  virtual DWORD DoShowCallStack(Command& cmd) = 0;    //�������23
};

