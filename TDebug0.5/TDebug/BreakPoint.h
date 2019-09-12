#pragma once
#include "../Comm/TDebugFunc.h"

#define INNER_BP_GO        1
#define INNER_BP_TRACE     2
#define INNER_BP_STEPOVER  3

//�������Ľӿ�, ������һ���ӿڵĻ������� 
class IBreakPoint
{
public:
  virtual DWORD   Enable()= 0;    //�ϵ���Ч
  virtual DWORD   Disable()= 0;   //�̵�ʧЧ
};


class TSoftBreakPoint : public IBreakPoint
{
public:
  TSoftBreakPoint(DWORD dwPID, LPVOID lpBpAddr)
  {
    m_dwPID      =  dwPID;
    m_lpBpAddr   =  lpBpAddr;
    m_btOldInst  =  0;
    m_btSoftInst =  0xCC;
  }
  virtual DWORD Enable();   //�ϵ���Ч   
  virtual DWORD Disable();  //�ϵ�ʧЧ
public:
  DWORD  m_dwPID;
  LPVOID m_lpBpAddr; //�ϵ��ַ
  BYTE   m_btOldInst;
  BYTE   m_btSoftInst;
};



class TInnerBreakPoint : public TSoftBreakPoint
{
public:
  TInnerBreakPoint(DWORD dwPID, LPVOID lpBpAddr, DWORD dwType)
    :TSoftBreakPoint(dwPID, lpBpAddr), m_dwType(dwType)
  {
  }
public:
  DWORD  m_dwType;
};


class THardBreakPoint : public IBreakPoint
{  
public:
  THardBreakPoint(DWORD dwPID, LPVOID lpBpAddr, DWORD dwReg , DWORD dwBPType, DWORD dwLen)
  {
    m_dwPID = dwPID;
    m_lpBpAddr = lpBpAddr;
    m_dwReg = dwReg;
    m_dwBPType = dwBPType;
    m_dwLen = dwLen;

  }
  virtual DWORD Enable();   //�ϵ���Ч   
  virtual DWORD Disable();  //�ϵ�ʧЧ
public:
  DWORD  m_dwPID;
  LPVOID m_lpBpAddr;
  DWORD  m_dwReg;  
  DWORD  m_dwBPType;
  DWORD  m_dwLen;
};


class TMemBreakPoint : public IBreakPoint
{
public:
  TMemBreakPoint(DWORD dwPID, LPVOID lpBPAddr, DWORD dwMemLen, DWORD dwBPType)
  {
    m_dwPID    = dwPID;
    m_lpBPAddr = lpBPAddr;  //�ϵ��ַ
    m_dwMemLen = dwMemLen;  //�ϵ㷶Χ
    m_dwBPType = dwBPType;  //�ϵ����� MEM_ACCESS, MEM_WRITE
  }
public:
  virtual DWORD Enable();   //�ϵ���Ч   
  virtual DWORD Disable();  //�ϵ�ʧЧ
public:
  DWORD  m_dwPID;
  LPVOID m_lpBPAddr;  //�ϵ��ַ
  DWORD  m_dwMemLen;  //�ϵ㷶Χ
  DWORD  m_dwBPType;  //�ϵ����� MEM_ACCESS, MEM_WRITE
};


typedef struct tagPage 
{
  DWORD m_dwPageBase;    //ҳ��ַ
  DWORD m_dwOldProtect;  //ҳԭ��������
}Page, *LPPage;

//�ϵ�-ҳ��
typedef struct tagMemBPPage
{
  TMemBreakPoint* m_lpMemBP;
  LPPage  m_lpPage;
}MemBPPage, *LPMemBPPage;



typedef list<TInnerBreakPoint*>    TInnerBpList;
typedef list<TSoftBreakPoint*> TSoftBpList;
typedef list<THardBreakPoint*> THardBpList;
typedef list<TMemBreakPoint*>  TMemBpList;
typedef list<LPPage>           TPageList;
typedef list<LPMemBPPage>      TMembpPageList;
class TBPManager
{
private:
  TBPManager()
  {
    memset(&m_dwDRStatus, DR_STATUS_UNUSED, sizeof(m_dwDRStatus));
  }
  ~TBPManager()
  {
     m_HardBpList.clear();
     m_MemBpList.clear();
     m_PageList.clear();
     m_MembpPageList.clear();
  }
public:
  static TBPManager* GetBPManager();
  static void DestotyBPManager();
public:
  //�ڲ��ϵ�
  TInnerBreakPoint* FindInnerBP(DWORD dwPid, LPVOID lpBpAddr);
  DWORD AddInnerBP(DWORD dwPID, LPVOID lpBpAddr, DWORD dwInnerBpType);
  DWORD DelInnerBP(DWORD dwPID, LPVOID lpBpAddr);
 
  //int3 �ϵ�
  TSoftBreakPoint* FindSoftBP(DWORD dwPid, LPVOID lpBpAddr);
  DWORD AddSoftBP(DWORD dwPID, LPVOID lpBpAddr);
  DWORD DelSoftBP(DWORD dwOrdinal);
  TSoftBpList& GetAllSoftBP();

  //Ӳ���ϵ�
 
  THardBreakPoint* FindHardBP(DWORD dwPid, LPVOID lpBpAddr);
  THardBreakPoint* FindHardBP(DWORD dwDR6);
  DWORD AddHardBP(DWORD dwPID, LPVOID lpBpAddr, DWORD dwBPType, DWORD dwLen);
  DWORD DelHardBP(DWORD dwOrdinal);
  THardBpList& GetAllHardBP();
  
  //�ڴ�ϵ�
  DWORD DisableMemBp(TMemBreakPoint* lpMembp);
  DWORD EnableMemBp(TMemBreakPoint* lpMembp);
  LPPage FindPageDataList(DWORD dwDestAddr);
  TMemBreakPoint* FindHitMemBP(DWORD dwDestAddr, DWORD dwActionType);

  BOOL CanAddMemBP(DWORD lpBPAddr, DWORD dwMemLen);
  DWORD AddMemBP(DWORD dwPid,LPVOID lpBPAddr, DWORD dwMemLen, DWORD dwBPType);
  DWORD DelMemBP(DWORD dwOrdinal);
  TMemBpList& GetMemBPList();
  TMembpPageList& GetMembpPageList();
  TPageList& GetPageList();


private:
   DWORD FindUnusedDR();
   
private:
  TInnerBpList m_InnerBpList;
  TSoftBpList  m_SoftBpList;
  
  DWORD  m_dwDRStatus[MAX_DR_CNT];
  THardBpList m_HardBpList;

  TMemBpList  m_MemBpList;
  TPageList   m_PageList;
  TMembpPageList m_MembpPageList;

  static TBPManager* g_lpBPMangaer;
};

