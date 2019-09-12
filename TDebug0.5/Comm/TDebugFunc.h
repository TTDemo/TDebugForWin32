/************************************************************************/
/* �ļ���:    Funcs.h
/* ����ʱ��:   2016-08-05
/* ����:      Gesang
/* ����:      ���Խӿڶ����ļ�
/************************************************************************/

#pragma once
#include <windows.h>
#include <windows.h>
#include <Tlhelp32.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include <map>
#include <list>
#include <vector>
#include <algorithm>
#include <Psapi.h>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <DbgHelp.h>
#include "Decode2Asm.h"
using namespace std;

#pragma  comment(lib, "DbgHelp.lib")
#pragma  comment(lib, "DisamDll.lib")


void SafeClose(HANDLE handle);
BOOL LoadFile(char *pszFileName, char **ppFileBuf, long *pnFileSize);



#define MAX_EVENT_CNT           10

//�˳���, ���ڿ��ٲ��Ҵ���ԭ��
#define QUIT_BASE                     0x10000000
#define QUIT_NOMAL                    (QUIT_BASE + 1)
#define QUIT_OPENPROCESS              (QUIT_BASE + 2)
#define QUIT_OPENTHREAD               (QUIT_BASE + 3)
#define QUIT_GETTHREADCONTEXT         (QUIT_BASE + 4)
#define QUIT_SETTHREADCONTEXT         (QUIT_BASE + 5)
#define QUIT_CREATEPROCESS            (QUIT_BASE + 6)
#define QUIT_WAITFORDEBUGEVENT        (QUIT_BASE + 7)
#define QUIT_CONTIUEDEBUGEVENT        (QUIT_BASE + 8)
#define QUIT_NOTFOUND_EVENRDISPATCH   (QUIT_BASE + 9)




//�����쳣ԭ��
#define STEP_CAUSEBY_BASE          0x20000000
#define STEP_CAUSEBY_TRACE         (STEP_CAUSEBY_BASE + 1)
#define STEP_CAUSEBY_STEP          (STEP_CAUSEBY_BASE + 2)
#define STEP_CAUSEBY_BREAKPOINT    (STEP_CAUSEBY_BASE + 3)

#define DR0         (0) 
#define DR1         (1) 
#define DR2         (2) 
#define DR3         (3) 
#define DR_UNKOWN   (-1) 

#define MAX_DR_CNT            4

#define DR_STATUS_UNUSED      0
#define DR_STATUS_USED        1


#define BP_LEN_BYTE      0x00 
#define BP_LEN_WORD      0x01
#define BP_LEN_DWORD     0x03

#define BP_ACTION_EXCUTE   0
#define BP_ACTION_WRITE    1
#define BP_ACTION_ACCESS   3
#define BP_ACTION_UNKOWN   -1

#define ERRCODE_BASE                  0x40000000
#define ERRCODE_SUCESS                (ERRCODE_BASE + 0)
#define ERRCODE_FAILED                (ERRCODE_BASE + 1)
#define ERRCODE_NOT_SUPPORT           (ERRCODE_BASE + 2)
#define ERRCODE_BP_EXISTED            (ERRCODE_BASE + 3)
#define ERRCODE_BP_NOTFOUND           (ERRCODE_BASE + 4)
#define ERRCODE_NOT_CMD               (ERRCODE_BASE + 5)
#define ERRCODE_OVER_NUM              (ERRCODE_BASE + 6)
#define ERRCODE_BP_ENABLE_FAILED      (ERRCODE_BASE + 7)
#define ERRCODE_NOT_FOUND             (ERRCODE_BASE + 8)
#define ERRCODE_MEM_BP_CONFILT        (ERRCODE_BASE + 9)



#define  MEM_ACCESS (0)
#define  MEM_WRITE  (1)
#define  MEM_DEP    (8)


//�������Ľӿ�, ������һ���ĵ����¼��Ļ������� 
typedef struct tagDEBUG_EVENT_CONTEXT
{
  HANDLE        hProcess;
  HANDLE        hThread; 
  LPCONTEXT     lpContext;
  LPDEBUG_EVENT lpDebugEv;
}DEBUG_EVENT_CONTEXT, *LPDEBUG_EVENT_CONTEXT;


typedef  struct tagTARGET_CONTEXT
{
  HANDLE        hProcess;
  HANDLE        hThread; 
  CONTEXT       stContext;
}TARGET_CONTEXT, *LPTARGET_CONTEXT;


typedef struct _tag_DR6 
{
  unsigned int B0:1;
  unsigned int B1:1;
  unsigned int B2:1;
  unsigned int B3:1;

  unsigned int Reserv0:9;

  unsigned int BD:1;  //�����쳣���� DR7��GDλ��λ, ��Ŀ���߳��в������ԼĴ���
  unsigned int BS:1;  //��λ��ʾ�����쳣����Elags��TF��λ����
  unsigned int BT:1;  

  unsigned int Reserv1:16;
}DR6, *LPDR6;

typedef struct _tag_DR7 
{
  unsigned int L0:1;
  unsigned int G0:1;
  unsigned int L1:1;
  unsigned int G1:1;
  unsigned int L2:1;
  unsigned int G2:1;
  unsigned int L3:1;
  unsigned int G3:1;    //��ӦDR0~DR3, �Ƿ�������

  unsigned int LE:1;    //��������Ϊ1
  unsigned int GE:1;    //��������Ϊ1

  unsigned int Reserv0:3;    //������3λ
  unsigned int GD:1;    //���ԼĴ���������־
  unsigned int Reserv1:2;    //������2λ

  unsigned int RW0:2;    //��ʾӲ���ϵ�����, 00 ִ��, 01 д��, 11 ����
  unsigned int LEN0:2;    //��ʾ�ϵ㳤��,    00-->1�ֽ�, 01-->2�ֽ�, 11-->���ֽ� 
  unsigned int RW1:2;
  unsigned int LEN1:2;
  unsigned int RW2:2;
  unsigned int LEN2:2;
  unsigned int RW3:2;
  unsigned int LEN3:2;
}DR7, *LPDR7;

typedef struct _tag_Eflags 
{
  unsigned int CF:1;
  unsigned int Reserv0:1; 
  unsigned int PF:1;
  unsigned int Reserv1:1;
  unsigned int AF:1;
  unsigned int Reserv2:1;
  unsigned int ZF:1;
  unsigned int SF:1;

  unsigned int TF:1;
  unsigned int IF:1;
  unsigned int DF:1;
  unsigned int OF:1;    
  unsigned int TOPL:2; 
  unsigned int NT:1; 
  unsigned int Reserv3:1; 

  unsigned int RF:1;
  unsigned int VM:1;
  unsigned int AC:1;
  unsigned int VIF:1;
  unsigned int VIP:1;
  unsigned int TD:1;
  unsigned int Reserv4:10; 
}Eflags , *LPEflags;




typedef struct tagAsmCode
{
  DWORD m_dwCodeAddr; 
  DWORD m_dwCodeSize;
  char  m_szOpcode[80]; 
  char  m_szASM[80];
 
}ASM_CODE,*LPASM_CODE;

void  EnableHardBP(LPCONTEXT lpContext, LPVOID lpBpAddr, DWORD dwReg,  DWORD dwBPType,  DWORD dwLen);
void  DisableHardBP(LPCONTEXT lpContext, DWORD dwReg);

DWORD GetSegBase(HANDLE hThread, PCONTEXT pContext);
DWORD GetLastErrorEx(HANDLE hProcess, HANDLE hThread, PCONTEXT pContext);

DWORD SuspendProcess(DWORD dwPid);
DWORD ResumeProcess( DWORD dwPid);
DWORD TerminateProcess(DWORD dwPid);

BOOL  GetProcessName(DWORD dwPid, char* lpszProName);

DWORD GetAsmCode(IN HANDLE hProcess, IN LPVOID lpStartAddr, OUT LPASM_CODE lpAsmCode, IN DWORD dwAsmCnt);
DWORD GetAsmStr(IN HANDLE hProcess, IN LPVOID lpStartAddr, OUT LPSTR lpszAsmBuf);
void  SafeCloseTargetContext(TARGET_CONTEXT stTargetContext);

//���ָ����ַ����ָ����CALL������CALLָ��ĳ���
//���򷵻�0
//�жϵķ����ο��ˡ�CALLָ���ж�����д����һ��
// http://blog.ftofficer.com/2010/04/n-forms-of-call-instructions
DWORD IsCallInstruction(HANDLE hProcess, LPVOID lpAddr); 



BOOL SelectFile(char *pszFilePath, SIZE_T nMaxPath);
BOOL SaveFile(char *pszFilePath, SIZE_T nMaxPath);