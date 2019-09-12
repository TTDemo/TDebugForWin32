#include "BreakPoint.h"


DWORD TSoftBreakPoint::Enable()   //断点生效   
{
  BOOL bRet = FALSE;
  BYTE btCC = 0xCC;
  HANDLE hProcess = NULL;

  hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_dwPID);
  if (NULL == hProcess)
  {
    return FALSE;
  }
  //保存原来的指令
  bRet = ReadProcessMemory(hProcess, m_lpBpAddr, &m_btOldInst, sizeof(m_btOldInst), NULL);
  if (bRet == NULL)
  {
    return FALSE;
  }

  //替换指令, 制造异常
  bRet = WriteProcessMemory(hProcess, m_lpBpAddr, &m_btSoftInst, sizeof(m_btSoftInst), NULL);
  if (bRet == NULL)
  {
    return FALSE;
  }

  if(hProcess != NULL)
  {
    CloseHandle(hProcess);
    hProcess = NULL;
  }
  return TRUE;
}


DWORD TSoftBreakPoint::Disable()  //断点失效 
{
  BOOL bRet = FALSE;
  HANDLE hProcess = NULL;

  hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_dwPID);
  if (NULL == hProcess)
  {
    return FALSE;
  }
  //保存原来的指令
  bRet = WriteProcessMemory(hProcess, m_lpBpAddr, &m_btOldInst, sizeof(m_btOldInst), NULL);
  if (bRet == NULL)
  {
    return FALSE;
  }

  if(hProcess != NULL)
  {
    CloseHandle(hProcess);
    hProcess = NULL;
  }
  return TRUE;
}



DWORD THardBreakPoint::Enable()   //断点生效 
{
  HANDLE        hThreadSnap = NULL; 
  THREADENTRY32 te32        = {0}; 
  HANDLE hThread = NULL;
  CONTEXT stContext = {0};

  hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, m_dwPID); 
  if (hThreadSnap == INVALID_HANDLE_VALUE)
  {
    return (FALSE);
  }
  te32.dwSize = sizeof(THREADENTRY32);
  if (Thread32First(hThreadSnap, &te32)) 
  { 
    do 
    { 
      if (te32.th32OwnerProcessID == m_dwPID) 
      { 
        stContext.ContextFlags = CONTEXT_ALL;
        hThread  = OpenThread(THREAD_ALL_ACCESS, FALSE, te32.th32ThreadID);
        if (GetThreadContext(hThread, &stContext))
        {
          ::EnableHardBP(&stContext, m_lpBpAddr, m_dwReg, m_dwBPType, m_dwLen);
        }
        SetThreadContext(hThread, &stContext);
        ::SafeClose(hThread);
        hThread = NULL;

      }
    } 
    while (Thread32Next(hThreadSnap, &te32)); 
  } 

  ::SafeClose(hThreadSnap);
  hThreadSnap = NULL;
  return TRUE;
}

LPDR7 pMyDr7 = NULL;



DWORD THardBreakPoint::Disable()  //断点失效
{
  HANDLE        hThreadSnap = NULL; 
  THREADENTRY32 te32        = {0}; 
  HANDLE hThread = NULL;
  CONTEXT stContext = {0};

  hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, m_dwPID); 
  if (hThreadSnap == INVALID_HANDLE_VALUE)
  {
    return (FALSE);
  }
  te32.dwSize = sizeof(THREADENTRY32);
  if (Thread32First(hThreadSnap, &te32)) 
  { 
    do 
    { 
      if (te32.th32OwnerProcessID == m_dwPID) 
      { 
        stContext.ContextFlags = CONTEXT_ALL;
        hThread  = OpenThread(THREAD_ALL_ACCESS, FALSE, te32.th32ThreadID);
        if (GetThreadContext(hThread, &stContext))
        {
          ::DisableHardBP(&stContext, m_dwReg);
        }
        SetThreadContext(hThread, &stContext);
        ::SafeClose(hThread);
        hThread = NULL;

      }
    } 
    while (Thread32Next(hThreadSnap, &te32)); 
  } 

  ::SafeClose(hThreadSnap);
  hThreadSnap = NULL;
  return TRUE;
}


DWORD TMemBreakPoint::Enable()   //断点生效   
{
  HANDLE hProcess = NULL;

 // bRet = VirtualProtectEx(hProcess, (LPVOID)dwCurPageAddr, 1, PAGE_NOACCESS, &dwOldProtect);
 // if(bRet == FALSE)
 // {
 //   return FALSE;
 // }
    return ERRCODE_FAILED;
}
DWORD TMemBreakPoint::Disable()  //断点失效
{
  return ERRCODE_FAILED;
}




//
TBPManager* TBPManager::g_lpBPMangaer = NULL;
TBPManager* TBPManager::GetBPManager()
{
  if(g_lpBPMangaer == NULL)
  {
    g_lpBPMangaer = new TBPManager();
  }
  return g_lpBPMangaer;
}

void TBPManager::DestotyBPManager()
{
  if(g_lpBPMangaer != NULL)
  {
   delete g_lpBPMangaer;
   g_lpBPMangaer = NULL;
  }
}

TInnerBreakPoint* TBPManager::FindInnerBP(DWORD dwPid, LPVOID lpBpAddr)
{
  TInnerBreakPoint* lpInnerBP = NULL;
  TInnerBpList::iterator it = m_InnerBpList.begin();

  while (it != m_InnerBpList.end())
  {
    if((*it)->m_lpBpAddr == lpBpAddr && (*it)->m_dwPID == dwPid)
    {
      lpInnerBP = *it;
      break;
    }
    it++;
  }
  return lpInnerBP;
}

DWORD TBPManager::AddInnerBP(DWORD dwPID, LPVOID lpBpAddr, DWORD dwInnerBpType)
{
  TInnerBreakPoint* lpFindBp = FindInnerBP(dwPID, lpBpAddr);
  if(lpFindBp != NULL)
  {
    return ERRCODE_FAILED;
  }
  TInnerBreakPoint* lpNewInnerBp = new TInnerBreakPoint(dwPID, lpBpAddr, dwInnerBpType);
  lpNewInnerBp->Enable();

  m_InnerBpList.push_back(lpNewInnerBp);

  return ERRCODE_SUCESS;
}
DWORD TBPManager::DelInnerBP(DWORD dwPID, LPVOID lpBpAddr)
{
  TInnerBreakPoint* lpInnerBP = NULL;
  TInnerBpList::iterator it = m_InnerBpList.begin();
  while (it != m_InnerBpList.end())
  {
    if((*it)->m_lpBpAddr == lpBpAddr && (*it)->m_dwPID == dwPID)
    {
      lpInnerBP = *it;
      break;
    }
    it++;
  }
  if(lpInnerBP == NULL)
  {
    return ERRCODE_FAILED;
  }
  lpInnerBP->Disable();
  m_InnerBpList.erase(it);
  delete lpInnerBP;
  lpInnerBP = NULL;
  return ERRCODE_SUCESS;
}





TSoftBreakPoint* TBPManager::FindSoftBP(DWORD dwPid, LPVOID lpBpAddr)
{
  TSoftBreakPoint* lpSoftBP = NULL;
  TSoftBpList::iterator it = m_SoftBpList.begin();

  while (it != m_SoftBpList.end())
  {
    if((*it)->m_lpBpAddr == lpBpAddr && (*it)->m_dwPID == dwPid)
    {
      lpSoftBP = *it;
      break;
    }
    it++;
  }
  return lpSoftBP;
}

DWORD TBPManager::AddSoftBP(DWORD dwPid, LPVOID lpBpAddr)
{ 

  TSoftBreakPoint* lpFindSoft = g_lpBPMangaer->FindSoftBP(dwPid, lpBpAddr);
  if (lpFindSoft != NULL)
  {
    //TDebugUI::ShowMsg("Address:%p have Soft Break Point! \r\n", dwBpAddr);
    return ERRCODE_FAILED;
  }
  TSoftBreakPoint* lpSoftBP = new TSoftBreakPoint(dwPid, lpBpAddr);
  if(!lpSoftBP->Enable())
  {
     return ERRCODE_FAILED;
  }
  m_SoftBpList.push_back(lpSoftBP);
  return ERRCODE_SUCESS;
}

DWORD TBPManager::DelSoftBP(DWORD dwOrdinal)
{ 
  DWORD i=0;
  TSoftBpList::iterator it = m_SoftBpList.begin();
  while (it != m_SoftBpList.end())
  {
    if(i == dwOrdinal)
    {
      (*it)->Disable();
      m_SoftBpList.erase(it);
      break;
    }
    i++;
    it++;
  }
  return ERRCODE_SUCESS;
}

TSoftBpList& TBPManager::GetAllSoftBP()
{
  return m_SoftBpList;
}


DWORD TBPManager::FindUnusedDR()
{
  for (DWORD i=0; i<MAX_DR_CNT; i++)
  {
    if(m_dwDRStatus[i] == DR_STATUS_UNUSED)
    {
      return i;
    }
  }
  return DR_UNKOWN;
}

THardBreakPoint* TBPManager::FindHardBP(DWORD dwPid, LPVOID lpBpAddr)
{
  THardBreakPoint* lpHardBP = NULL;
  THardBpList::iterator it = m_HardBpList.begin();

  while (it != m_HardBpList.end())
  {
    if((*it)->m_lpBpAddr == lpBpAddr && (*it)->m_dwPID == dwPid)
    {
      lpHardBP = *it;
      break;
    }
    it++;
  }
  return lpHardBP;
}

THardBreakPoint* TBPManager::FindHardBP(DWORD dwDR6)
{
  DWORD dwReg = 0;
  if ((dwDR6 & 0x01) != 0)
  {
    dwReg = DR0;
  }
  else if((dwDR6& 0x02) != 0)
  {
    dwReg = DR1;
  }
  else if((dwDR6 & 0x04) != 0)
  {
    dwReg = DR2;
  }
  else if((dwDR6 & 0x08) != 0)
  {
    dwReg = DR3;
  }
  
  THardBreakPoint* lpHardBP = NULL;
  THardBpList::iterator it = m_HardBpList.begin();
  while (it != m_HardBpList.end())
  {
    if((*it)->m_dwReg == dwReg)
    {
      lpHardBP = *it;
      break;
    }
    it++;
  }

  return lpHardBP;
}

DWORD TBPManager::AddHardBP(DWORD dwPID, LPVOID lpBpAddr, DWORD dwBPType, DWORD dwLen)
{
  DWORD dwReg = FindUnusedDR();
  if (dwReg == DR_UNKOWN)
  {
    return ERRCODE_OVER_NUM;
  }
  
  THardBreakPoint* lpHardBP = FindHardBP(dwPID, lpBpAddr);
  if(lpHardBP !=NULL)
  {
    return ERRCODE_BP_EXISTED; 
  }

  lpHardBP = new THardBreakPoint(dwPID, lpBpAddr, dwReg, dwBPType, dwLen);
  if(!lpHardBP->Enable())
  {
    return ERRCODE_BP_ENABLE_FAILED;
  }
  m_HardBpList.push_back(lpHardBP);
  m_dwDRStatus[lpHardBP->m_dwReg] = DR_STATUS_USED;
  return ERRCODE_SUCESS;
}



DWORD TBPManager::DelHardBP(DWORD dwOrdinal)
{
  THardBreakPoint* lpHardBP = NULL;
  DWORD i=0;
  THardBpList::iterator it = m_HardBpList.begin();
  while (it != m_HardBpList.end())
  {
    if(i == dwOrdinal)
    {
      lpHardBP = *it;
      lpHardBP->Disable();
      m_dwDRStatus[lpHardBP->m_dwReg] = DR_STATUS_UNUSED;
      m_HardBpList.erase(it);
      break;
    }
    i++;
    it++;
  }
  return TRUE;
}

THardBpList& TBPManager::GetAllHardBP()
{
  return m_HardBpList;
}
BOOL TBPManager::CanAddMemBP(DWORD dwBPAddr, DWORD dwMemLen)
{
  TMemBpList::iterator it = m_MemBpList.begin();
  DWORD dwStartAddr = 0;
  DWORD dwEndAddr = 0;
  while(it != m_MemBpList.end())
  {
    dwStartAddr = (DWORD)(*it)->m_lpBPAddr;
    dwEndAddr = (DWORD)(*it)->m_lpBPAddr + (DWORD)(*it)->m_dwMemLen;

    if( (dwBPAddr >= dwStartAddr && dwBPAddr < dwEndAddr) ||
       ((dwBPAddr+dwMemLen >= dwStartAddr && dwBPAddr+dwMemLen < dwEndAddr)))
    {
      return FALSE;
    }
    it++;
  }
  return TRUE;
}

DWORD TBPManager::AddMemBP(DWORD dwPID, LPVOID lpBPAddr, DWORD dwMemLen, DWORD dwBPType)
{
  BOOL bRet = NULL;
  DWORD dwOldProtect = NULL;

  DWORD dwCurPageAddr  =  (DWORD)lpBPAddr & 0xfffff000;
  DWORD dwLastPageAddr  = ((DWORD)lpBPAddr+dwMemLen-1) & 0xfffff000;

  if(!CanAddMemBP((DWORD)lpBPAddr, dwMemLen))
  {
    return ERRCODE_MEM_BP_CONFILT;
  }

  TMemBreakPoint* lpMemBP = new TMemBreakPoint(dwPID, lpBPAddr, dwMemLen, dwBPType);

  HANDLE hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID);
  if(hProcess == NULL)
  {
    return ERRCODE_FAILED;
  }
  while (dwCurPageAddr <= dwLastPageAddr)
  {
    bRet = VirtualProtectEx(hProcess, (LPVOID)dwCurPageAddr, 1, PAGE_NOACCESS, &dwOldProtect);
    if(bRet == FALSE)
    {
      return ERRCODE_FAILED;
    }

    LPPage lpPage = new Page();
    lpPage->m_dwPageBase = dwCurPageAddr;
    lpPage->m_dwOldProtect = dwOldProtect;

    LPMemBPPage lpMemBPPage = new MemBPPage();
    lpMemBPPage->m_lpMemBP = lpMemBP;
    lpMemBPPage->m_lpPage = lpPage;

    m_PageList.push_back(lpPage);
    m_MembpPageList.push_back(lpMemBPPage);

    dwCurPageAddr += 0x1000;
  }
  m_MemBpList.push_back(lpMemBP);
  
  SafeClose(hProcess);
  hProcess = NULL;
  return ERRCODE_SUCESS;
}

DWORD TBPManager::DelMemBP(DWORD dwOrdinal)
{
  TMemBreakPoint* lpMemBP = NULL;
  DWORD i=0;
  TMemBpList::iterator it = m_MemBpList.begin();
  while (it != m_MemBpList.end())
  {
    if(i == dwOrdinal)
    {
      lpMemBP = *it;
      break;
    }
    i++;
    it++;
  }

  if(lpMemBP == NULL)
  {
    return ERRCODE_NOT_FOUND;
  }

  DWORD  dwOldProtected =0;
  DWORD  dwPID    = lpMemBP->m_dwPID;
  LPVOID lpBPAddr = lpMemBP->m_lpBPAddr;  
  DWORD  dwMemLen = lpMemBP->m_dwMemLen;  
  DWORD  dwBPType = lpMemBP->m_dwBPType;  

  DWORD  dwCurPageAddr  =  (DWORD)lpBPAddr & 0xfffff000;
  DWORD  dwLastPageAddr  = ((DWORD)lpBPAddr+dwMemLen-1) & 0xfffff000;
  HANDLE hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID);
  BOOL   bRet=FALSE;
  LPPage lpPage = NULL;
  if(hProcess == NULL)
  {
    return ERRCODE_FAILED;
  }
  while (dwCurPageAddr <= dwLastPageAddr)
  {
    TPageList::iterator it = m_PageList.begin();
    while(it != m_PageList.end())
    {
      lpPage = *it;
      if(lpPage->m_dwPageBase == dwCurPageAddr)
      {
        bRet = VirtualProtectEx(hProcess, (LPVOID)dwCurPageAddr, 1, lpPage->m_dwOldProtect, &dwOldProtected);
        if(bRet == FALSE)
        {
          return ERRCODE_FAILED;
        }
        delete lpPage;
        lpPage = NULL;
        it = m_PageList.erase(it);
      }
      else
      {
        it++;
      }
    
    }

    LPMemBPPage lpMemBPPage = NULL;
    TMembpPageList::iterator it1 = m_MembpPageList.begin();
    while(it1 != m_MembpPageList.end())
    {
      lpMemBPPage = *it1;
      if(lpMemBPPage->m_lpMemBP == lpMemBP)
      {
        delete lpMemBPPage;
        lpMemBPPage = NULL;
        it1 = m_MembpPageList.erase(it1);
      }
      else
      {
        it1++;
      }
    }
    dwCurPageAddr += 0x1000;
  } 

  delete lpMemBP;
  lpMemBP = NULL;
  m_MemBpList.erase(it);

  SafeClose(hProcess);
  hProcess = NULL;
  return ERRCODE_SUCESS;
}

TMemBpList& TBPManager::GetMemBPList()
{
  return m_MemBpList;
}

TMembpPageList& TBPManager::GetMembpPageList()
{
  return m_MembpPageList;
}

TPageList& TBPManager::GetPageList()
{
  return m_PageList;
}

LPPage TBPManager::FindPageDataList(DWORD dwDestAddr)
{
  DWORD dwDestBase = dwDestAddr & 0xfffff000;
  list<LPPage>::iterator it = m_PageList.begin();


  while(it!= m_PageList.end())
  {
    LPPage pPage = *it;
    if (dwDestBase == pPage->m_dwPageBase)
    {
      return pPage;
    }
    it++;
  }
  return NULL;
}

// 获取命中的内存断点
TMemBreakPoint* TBPManager::FindHitMemBP(DWORD dwDestAddr, DWORD dwActionType)
{
  TMemBpList::iterator it = m_MemBpList.begin();
  TMemBreakPoint* pMemBP = NULL; 
  while (it != m_MemBpList.end())
  {
    pMemBP = *it;

    if ( dwDestAddr >= (DWORD)pMemBP->m_lpBPAddr &&  (dwDestAddr < (DWORD)pMemBP->m_lpBPAddr + pMemBP->m_dwMemLen))
    {
      if ((pMemBP->m_dwBPType == MEM_ACCESS) || (dwActionType == MEM_DEP)
        ||((pMemBP->m_dwBPType == MEM_WRITE) && (dwActionType == MEM_WRITE)))
      {
        return pMemBP;
      }
    }
    it++;
  }

  return NULL;
}

DWORD TBPManager::DisableMemBp(TMemBreakPoint* lpMembp)
{
    LPPage lpPage = NULL;
    DWORD dwOld;
    list<LPMemBPPage>::iterator it = m_MembpPageList.begin();
    HANDLE hProcess = NULL;
    LPMemBPPage pMemBPPage = NULL;

    hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, lpMembp->m_dwPID);
    while (it != m_MembpPageList.end())
    {
      pMemBPPage = *it;
      if( pMemBPPage->m_lpMemBP == lpMembp)
      {
        lpPage = pMemBPPage->m_lpPage;
        VirtualProtectEx(hProcess, (LPVOID)lpPage->m_dwPageBase,  1, lpPage->m_dwOldProtect, &dwOld);
      }
      it++;
    }
    SafeClose(hProcess);
    hProcess = NULL;
    return ERRCODE_SUCESS;
}

DWORD TBPManager::EnableMemBp(TMemBreakPoint* lpMembp)
{
  LPPage lpPage = NULL;
  DWORD dwOld;
  list<LPMemBPPage>::iterator it = m_MembpPageList.begin();
  HANDLE hProcess = NULL;
  LPMemBPPage pMemBPPage = NULL;

  hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, lpMembp->m_dwPID);
  while (it != m_MembpPageList.end())
  {
    pMemBPPage = *it;
    if( pMemBPPage->m_lpMemBP == lpMembp)
    {
      lpPage = pMemBPPage->m_lpPage;
      VirtualProtectEx(hProcess, (LPVOID)lpPage->m_dwPageBase,  1, PAGE_NOACCESS, &dwOld);
    }
    it++;
  }
  SafeClose(hProcess);
  hProcess = NULL;
  return ERRCODE_SUCESS;
}