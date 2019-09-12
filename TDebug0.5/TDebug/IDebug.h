/************************************************************************/
/* 文件名:     IDebugger
/* 创建时间:   2016-08-05
/* 作者:      Gesang
/* 描述:      调试接口定义文件
/************************************************************************/
#pragma once
#include "../Comm/TDebugFunc.h"
//调试器的接口, 定义了一个调试器的基本功能 
class IDebug
{
public:
  // 事件处理类
  virtual DWORD PauseTarget() = 0;
  virtual DWORD RunTarget() = 0;
  virtual DWORD FinishTarget()=0;
  virtual BOOL  IsPause() = 0;
  virtual DWORD SetTrapFlag()=0;
  virtual BOOL  IsStepIn() =0;
  virtual void SetStepInFlag(BOOL bStepIn)=0;
  virtual BOOL  IsBpStep() =0;
  virtual void  SetBPStep(BOOL bStepIn)=0;
  virtual void  SetBPStepAddr(LPVOID lpBpAddr)=0;
  virtual LPVOID  GetBPStepAddr()=0;
};







