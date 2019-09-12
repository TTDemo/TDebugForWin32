#include "TDebugFunc.h"

//1)保存地址到DR0~DR3
//2)设置DR7的 L0~L3
//3)设置DR7的 RW0~RW3, 表示断点类型
//4)设置DR7的Len0~Len3, 表示断点的长度 (执行断点设置为1字节, 设置断点长度为 2 /4, 需要断点起始地址 对齐 )
void EnableHardBP(LPCONTEXT lpContext, LPVOID lpBpAddr, DWORD dwReg,  DWORD dwBPType,  DWORD dwLen)
{
  LPDR7 lpDR7 = (LPDR7)&lpContext->Dr7;
  switch(dwReg)
  {
  case DR0:
    {
      lpContext->Dr0 = (DWORD)lpBpAddr;
      lpDR7->L0 = 1;
      lpDR7->RW0  = dwBPType;
      lpDR7->LEN0 = dwLen;
    }

    break;
  case DR1:
    {
      lpContext->Dr1 = (DWORD)lpBpAddr;
      lpDR7->L1 = 1;
      lpDR7->RW1  = dwBPType;
      lpDR7->LEN1 = dwLen;
    }

    break;
  case DR2:
    {
      lpContext->Dr2 = (DWORD)lpBpAddr;
      lpDR7->L2 = 1;
      lpDR7->RW2  = dwBPType;
      lpDR7->LEN2 = dwLen;
    }
    break;
  case DR3:
    {
      lpContext->Dr3 = (DWORD)lpBpAddr;
      lpDR7->L3   = 1;
      lpDR7->RW3  = dwBPType;
      lpDR7->LEN3 = dwLen;
    }
    break;
  }
}  

void  DisableHardBP(LPCONTEXT lpContext, DWORD dwReg)
{
  LPDR7 pMyDr7 = NULL;
  LPDR7 lpDR7 = (LPDR7)&lpContext->Dr7;
  switch(dwReg)
  {
  case DR0:
    {
      lpContext->Dr0 = (DWORD)0;
      lpDR7->L0 = 0;
    }

    break;
  case DR1:
    {
      lpContext->Dr1 = 0;
      lpDR7->L1 = 0;
    }

    break;
  case DR2:
    {
      lpContext->Dr2 = 0;
      lpDR7->L2 = 0;
    }
    break;
  case DR3:
    {
      lpContext->Dr3 = 0;
      lpDR7->L3   = 0;
    }
    break;
  }
}


DWORD GetSegBase(HANDLE hThread, PCONTEXT pContext)
{
  DWORD dwSegBase = NULL;
  LDT_ENTRY ldtFS;

  BOOL bRet = GetThreadSelectorEntry(hThread, 
    pContext->SegFs, &ldtFS);

  dwSegBase = ldtFS.HighWord.Bits.BaseHi << 24 |
    ldtFS.HighWord.Bits.BaseMid << 16 |
    ldtFS.BaseLow;

  return dwSegBase;
}


DWORD GetLastErrorEx(HANDLE hProcess, HANDLE hThread, PCONTEXT pContext)
{
  DWORD dwLastErr = 0;
  DWORD dwSegBase =  GetSegBase(hThread, pContext);
  if (NULL== ReadProcessMemory(hProcess, (LPVOID)(dwSegBase+0x34), &dwLastErr, sizeof(dwLastErr), NULL))
  {
    return NULL;
  }
  return dwLastErr;
  
}



DWORD SuspendProcess(DWORD dwPid)
{
  HANDLE        hThreadSnap = NULL; 
  BOOL          bRet        = FALSE; 
  THREADENTRY32 te32        = {0}; 
  hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0); 
  if (hThreadSnap == INVALID_HANDLE_VALUE) 
    return (FALSE); 

  te32.dwSize = sizeof(THREADENTRY32); 

  HANDLE hThread = NULL;
  if (Thread32First(hThreadSnap, &te32)) 
  { 
    do 
    { 
      if (te32.th32OwnerProcessID == dwPid) 
      { 
        hThread  = OpenThread(THREAD_ALL_ACCESS, FALSE, te32.th32ThreadID);
        SuspendThread(hThread);
      }
    } 
    while (Thread32Next(hThreadSnap, &te32)); 
    bRet = TRUE; 
  } 
  else 
    bRet = FALSE;

  CloseHandle (hThreadSnap);
  return (bRet); 
}
DWORD ResumeProcess( DWORD dwPid)
{
  HANDLE        hThreadSnap = NULL; 
  BOOL          bRet        = FALSE; 
  THREADENTRY32 te32        = {0}; 
  hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0); 
  if (hThreadSnap == INVALID_HANDLE_VALUE) 
    return (FALSE); 

  te32.dwSize = sizeof(THREADENTRY32); 

  HANDLE hThread = NULL;
  if (Thread32First(hThreadSnap, &te32)) 
  { 
    do 
    { 
      if (te32.th32OwnerProcessID == dwPid) 
      { 
        hThread  = OpenThread(THREAD_ALL_ACCESS, FALSE, te32.th32ThreadID);
        ResumeThread(hThread);
      }
    } 
    while (Thread32Next(hThreadSnap, &te32)); 
    bRet = TRUE; 
  } 
  else 
    bRet = FALSE;

  CloseHandle (hThreadSnap);
  return (bRet); 
}

BOOL GetProcessName(DWORD processID, char* lpszProName)
{
  HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);

  return GetModuleFileNameEx(hProcess, NULL, lpszProName, MAX_PATH);
}



void SafeClose(HANDLE handle)
{
  if (handle != NULL)
  {
    CloseHandle(handle);
    handle = NULL;
  }
}


/************************************************************************/
/* 
Function : Try to load the specified file into memory,
Params   : pszFileName used to indicate the filepath
           ppFileBuf used to receive the buffer pointer
                Do not forget to free the buf if no longer used
Return   : TRUE if success, FALSE otherwise
*/
/************************************************************************/
BOOL LoadFile(char *pszFileName, char **ppFileBuf, long *pnFileSize)
{
    assert(pszFileName != NULL);
    assert(ppFileBuf != NULL);
    *ppFileBuf = NULL;

    FILE *fp    = NULL;
    char *pBuf  = NULL;
    int  nRet   = 0;
    BOOL bRet   = TRUE;
    long nFileSize = 0;

    fp = fopen(pszFileName, "rb");
    if (NULL == fp)
    {
        bRet = FALSE;
        goto END;
    }

    //get file size
    nRet = fseek(fp, 0, SEEK_END);
    if (nRet)
    {   
        bRet = FALSE;
        goto END;
    }

    nFileSize = ftell(fp);
    if (-1L == nFileSize)
    {
        bRet = FALSE;
        goto END;
    }

    //rollback
    nRet = fseek(fp, 0, SEEK_SET);
    if (nRet)
    {
        bRet = FALSE;
        goto END;
    }

    //alloc mem and load file into
    pBuf = (char *)malloc(nFileSize);
    if (NULL == pBuf)
    {
        bRet = FALSE;
        goto END;
    }

    fread(pBuf, sizeof(char), nFileSize, fp);
    if(ferror(fp))
    {
        bRet = FALSE;
        goto END;
    }

    *ppFileBuf  = pBuf;
    *pnFileSize = nFileSize;

END:
    if (fp != NULL)
    {
        fclose(fp);
        fp = NULL;
    }
    
    return bRet;
}



DWORD GetAsmCode(IN HANDLE hProcess, IN LPVOID lpStartAddr, OUT LPASM_CODE lpAsmCode, IN DWORD dwAsmCnt)
{
  DWORD dwReadLen = 0;
  INT   nCodeAddress = (INT)lpStartAddr;
  BYTE btInsBuffer[80] = {0};
  char szASMBuffer[80] = {0};
  char szOpcodeBuffer[80] = {0};
  UINT nCodeSize = 0;
  BOOL bRet = FALSE;
  for(DWORD i=0; i<dwAsmCnt; i++)
  {
    bRet = ReadProcessMemory(hProcess, (LPVOID)nCodeAddress, btInsBuffer, sizeof(btInsBuffer),  &dwReadLen);
    if (bRet == 0)
    {
      return FALSE;
    }

    Decode2AsmOpcode(btInsBuffer, szASMBuffer, szOpcodeBuffer, &nCodeSize, nCodeAddress);
    
    lpAsmCode[i].m_dwCodeAddr = nCodeAddress;
    lpAsmCode[i].m_dwCodeSize = nCodeSize;
    memcpy(lpAsmCode[i].m_szASM,  szASMBuffer,  strlen(szASMBuffer)+1);
    memcpy(lpAsmCode[i].m_szOpcode,  szOpcodeBuffer, strlen(szOpcodeBuffer)+1);

    nCodeAddress += nCodeSize;
  }
  return TRUE;
}


DWORD GetAsmStr(IN HANDLE hProcess, IN LPVOID lpStartAddr, OUT LPSTR lpszAsmBuf)
{
  DWORD dwReadLen = 0;
  INT   nCodeAddress = (INT)lpStartAddr;
  BYTE btInsBuffer[80] = {0};
  char szOpcodeBuffer[80] = {0};
  UINT nCodeSize = 0;
  BOOL bRet = FALSE;
  bRet = ReadProcessMemory(hProcess, (LPVOID)nCodeAddress, btInsBuffer, sizeof(btInsBuffer),  &dwReadLen);
  if (bRet == 0)
  {
    return FALSE;
  }
  Decode2AsmOpcode(btInsBuffer, lpszAsmBuf, szOpcodeBuffer, &nCodeSize, nCodeAddress);
  return TRUE;
}



void SafeCloseTargetContext(TARGET_CONTEXT stTargetContext)
{
  SafeClose(stTargetContext.hThread);
  SafeClose(stTargetContext.hProcess);
}


DWORD IsCallInstruction(HANDLE hProcess, LPVOID lpAddr) 
{
  BYTE instruction[10];
  ReadProcessMemory(hProcess, lpAddr,instruction, sizeof(instruction) / sizeof(BYTE), NULL);
  switch (instruction[0]) 
  {
  case 0xE8:
    return 5;

  case 0x9A:
    return 7;

  case 0xFF:
    switch (instruction[1]) {

    case 0x10:
    case 0x11:
    case 0x12:
    case 0x13:
    case 0x16:
    case 0x17:
    case 0xD0:
    case 0xD1:
    case 0xD2:
    case 0xD3:
    case 0xD4:
    case 0xD5:
    case 0xD6:
    case 0xD7:
      return 2;

    case 0x14:
    case 0x50:
    case 0x51:
    case 0x52:
    case 0x53:
    case 0x54:
    case 0x55:
    case 0x56:
    case 0x57:
      return 3;

    case 0x15:
    case 0x90:
    case 0x91:
    case 0x92:
    case 0x93:
    case 0x95:
    case 0x96:
    case 0x97:
      return 6;

    case 0x94:
      return 7;
    }
  default:
    return 0;
  }
}



BOOL SelectFile(char *pszFilePath, SIZE_T nMaxPath)
{
  BOOL bRet = TRUE;
  char appPath[MAX_PATH] = {0};	
  OPENFILENAME ofn;

  RtlZeroMemory(&ofn,sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  ofn.lpstrFilter = TEXT("All Files(*.*)\0*.*\0");
  ofn.lpstrFile   = appPath;
  ofn.nMaxFile    = MAX_PATH;
  ofn.Flags       = OFN_PATHMUSTEXIST|OFN_HIDEREADONLY|OFN_LONGNAMES;
  bRet = GetOpenFileName(&ofn);	
  if (bRet)
  {
    lstrcpynA(pszFilePath, appPath, nMaxPath);
  }   

  return bRet;
}


BOOL SaveFile(char *pszFilePath, SIZE_T nMaxPath)
{
  TCHAR   szFilename[MAX_PATH]   =   TEXT("");   
  BOOL   bRet   =   FALSE;   
  OPENFILENAME   ofn   =   {0};  
  ofn.lStructSize   =   sizeof   (OPENFILENAME);   
  /// 在保存窗体 （类型那栏） 下面有显示；
  ofn.lpstrFilter = TEXT("All   Files\0*.*\0\0"); 
  /// 保存整个路径和文件名在szFilename里面；
  ofn.lpstrFile   =   szFilename;   
  ofn.nMaxFile   =   MAX_PATH;   
  ofn.Flags   =   OFN_EXPLORER|OFN_ENABLEHOOK|OFN_HIDEREADONLY|OFN_NOCHANGEDIR|OFN_PATHMUSTEXIST;   

  bRet   =   GetSaveFileName   (&ofn);   
  if (bRet != FALSE)
  {
    lstrcpynA(pszFilePath, szFilename, nMaxPath);
  }   
  return bRet;
}