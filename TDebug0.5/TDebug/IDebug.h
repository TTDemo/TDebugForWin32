/************************************************************************/
/* �ļ���:     IDebugger
/* ����ʱ��:   2016-08-05
/* ����:      Gesang
/* ����:      ���Խӿڶ����ļ�
/************************************************************************/
#pragma once
#include "../Comm/TDebugFunc.h"
//�������Ľӿ�, ������һ���������Ļ������� 
class IDebug
{
public:
  // �¼�������
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







