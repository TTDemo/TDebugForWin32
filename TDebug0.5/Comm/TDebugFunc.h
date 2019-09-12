/************************************************************************/
/* 文件名:    Funcs.h
/* 创建时间:   2016-08-05
/* 作者:      Gesang
/* 描述:      调试接口定义文件
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

//退出码, 用于快速查找错误原因
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




//单步异常原因
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


//调试器的接口, 定义了一个的调试事件的基本功能 
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

  unsigned int BD:1;  //单步异常由于 DR7的GD位置位, 而目标线程有操作调试寄存器
  unsigned int BS:1;  //置位表示单步异常由于Elags的TF置位引起
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
  unsigned int G3:1;    //对应DR0~DR3, 是否起作用

  unsigned int LE:1;    //建议设置为1
  unsigned int GE:1;    //建议设置为1

  unsigned int Reserv0:3;    //保留的3位
  unsigned int GD:1;    //调试寄存器保护标志
  unsigned int Reserv1:2;    //保留的2位

  unsigned int RW0:2;    //表示硬件断点类型, 00 执行, 01 写入, 11 访问
  unsigned int LEN0:2;    //表示断点长度,    00-->1字节, 01-->2字节, 11-->四字节 
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

//如果指定地址处的指令是CALL，返回CALL指令的长度
//否则返回0
//判断的方法参考了《CALL指令有多少种写法》一文
// http://blog.ftofficer.com/2010/04/n-forms-of-call-instructions
DWORD IsCallInstruction(HANDLE hProcess, LPVOID lpAddr); 



BOOL SelectFile(char *pszFilePath, SIZE_T nMaxPath);
BOOL SaveFile(char *pszFilePath, SIZE_T nMaxPath);