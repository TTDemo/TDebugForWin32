#include "TDebugCmdline.h"
#include "../TDebugUI/TDebugUI.h"
#include "../TDebug/TDebug.h"




TDebugCmdLine*    TDebugCmdLine::g_lpTDebugCmdline = NULL;
DispatchCmdMap    TDebugCmdLine::g_DisCmdMap;
HANDLE            TDebugCmdLine::g_hTalk = NULL;

TDebugCmdLine*  TDebugCmdLine::GetDebugCmdline()
{
  if (g_lpTDebugCmdline == NULL)
  {
    g_lpTDebugCmdline = new TDebugCmdLine();  
    g_hTalk = ::CreateEvent(NULL, FALSE, TRUE, NULL);
    if(g_hTalk  == NULL)
    {
      TDebugUI::ShowError();
    }
    //调试器选项
    DISPATCHCMD("?",      &TDebugCmdLine::DoShowHelp);//*
    DISPATCHCMD("help",   &TDebugCmdLine::DoShowHelp);//*

    DISPATCHCMD("t",      &TDebugCmdLine::DoStepInto);//*
    DISPATCHCMD("p",      &TDebugCmdLine::DoStepOver);//*
    DISPATCHCMD("g",      &TDebugCmdLine::DoGo);      //*
    DISPATCHCMD("trace",  &TDebugCmdLine::DoTrace);   //*
    DISPATCHCMD("u",      &TDebugCmdLine::DoShowASM); //*
    DISPATCHCMD("d",      &TDebugCmdLine::DoShowData); //*
    DISPATCHCMD("r",      &TDebugCmdLine::DoShowRegs); //*
    DISPATCHCMD("e",      &TDebugCmdLine::DoEditData); //*
   
    DISPATCHCMD("bp",     &TDebugCmdLine::DoSoftBreakPoint);       
    DISPATCHCMD("bpl",    &TDebugCmdLine::DoListAllSoftBP);     
    DISPATCHCMD("bpc",    &TDebugCmdLine::DoClearSoftBP);

    DISPATCHCMD("bh",     &TDebugCmdLine::DoHardBreakPoint);     
    DISPATCHCMD("bhl",    &TDebugCmdLine::DoListAllHardBP);      
    DISPATCHCMD("bhc",    &TDebugCmdLine::DoClearHardBP);      
    
    DISPATCHCMD("bm",     &TDebugCmdLine::DoMemBreakPoint);       
    DISPATCHCMD("bml",    &TDebugCmdLine::DoListAllMemBP);      
    DISPATCHCMD("bmpl",   &TDebugCmdLine::DoListMemBpPage);     
    DISPATCHCMD("bmc",    &TDebugCmdLine::DoClearMemBP);      
    
    DISPATCHCMD("ls",     &TDebugCmdLine::DoLoadScript); //      
    DISPATCHCMD("es",     &TDebugCmdLine::DoExportScript); //      
    DISPATCHCMD("q",      &TDebugCmdLine::DoFinish);     //*
    DISPATCHCMD("ml",     &TDebugCmdLine::DoListModule); //*
    DISPATCHCMD("his",    &TDebugCmdLine::DoHistory); //*
    DISPATCHCMD("cs",     &TDebugCmdLine::DoShowCallStack);
  }
  return g_lpTDebugCmdline;
}


TDebugCmdLine::TDebugCmdLine()
{
  m_bQuit = FALSE;
}

void  TDebugCmdLine::DestoryDebugCmdline()
{
  if(g_lpTDebugCmdline != NULL)
  {
    delete g_lpTDebugCmdline;
    g_lpTDebugCmdline = NULL;
    g_DisCmdMap.clear();
  }
}

TDebugCmdLine::~TDebugCmdLine()
{
  ::SafeClose(g_hTalk);
 
}

void TDebugCmdLine::Run()
{
  string cmdLine;
  DWORD dwRet = ERRCODE_SUCESS;
  BOOL bInput = TRUE;
  Command cmd;
  while (!m_bQuit) 
  {
    WaitForSingleObject(g_hTalk, INFINITE);
    cout << TEXT("TDebug>");
    getline(cin, cmdLine);
    fflush(stdin); 
    ParseCommand(cmdLine, cmd);
    if(!cmd.empty())
    {
      dwRet = ExcuteCmd(cmd);
      if(dwRet == ERRCODE_NOT_CMD)
      {
        TDebugUI::ShowMsg("Error: INVALID CMD !\r\n");
      }
      else if(dwRet == ERRCODE_SUCESS)
      {
        m_strHisCmds += cmdLine;
        m_strHisCmds += "\r\n";
      }
      cmd.clear();
    }
    Sleep(100);  //等待异常处理完成；
    ContinueTalk();
  }
}


void TDebugCmdLine::ContinueTalk()
{
  SetEvent(g_hTalk);
}

void TDebugCmdLine::PauseTalk()
{
  ResetEvent(g_hTalk);
}

DWORD TDebugCmdLine::ExcuteCmd(Command& cmd)
{
  DispatchCmdMap::iterator it = g_DisCmdMap.find(cmd[0].c_str());
  if (it == g_DisCmdMap.end())
  {
    return ERRCODE_NOT_CMD;
  }
  PFNDispatchCmd pfnDispatchCmd = it->second;
  return (this->*pfnDispatchCmd)(cmd);
}


//以空白字符作为分隔符，将命令行拆分成几个部分，添加到Command中。
//两个双引号内的内容作为一部分。
//该方法未经完全测试，可能有BU。。
void TDebugCmdLine::ParseCommand(const string& cmdLine, Command& cmd) 
{
  string partialArg;
  istringstream cmdStream(cmdLine);
  while (cmdStream >> partialArg)
  {
    if(!partialArg.empty())
    {
      cmd.push_back(partialArg);
    }
    
  }
}





/************调试器支持命令**************/
DWORD TDebugCmdLine::DoShowHelp(Command& cmd)          //功能序号0 
{
  DWORD dwRet=ERRCODE_SUCESS;
  static char szBuf[1024];
  _snprintf(szBuf, 1024, "----------------帮助-----------------\r\n"
                         "命令   格式                 作用\r\n" 
                         "t      t                   步入        \r\n"
                         "p      p                   步过\r\n"
                         "g      g [addr]            运行\r\n"
                         "w      w"
                         "r      r                   寄存器查看\r\n"
                         "u      u [addr]            汇编查看\r\n"
                         "d      d [addr]            内存数据查看\r\n"
                         "q      q                   退出当前调试\r\n"
                         "his    his                 查看最近输入的命令\r\n"
                         "bm     bm addr a|w len     内存断点设置\r\n"			 
                         "bml    bml                 内存断点查看\r\n"
                         "bmpl   bmpl                分页内内存断点查看\r\n"
                         "bmc    bmc id (from bml)   硬件断点删除\r\n"
                         "bp     bp addr             一般断点设置\r\n"
                         "bpl    bpl                 一般断点查看\r\n"
                         "bpc    bpc id (from bpl)   一般断点删除\r\n"
                         "bh     bh addr e|w|a 1|2|4 硬件断点设置\r\n"
                         "bhl    bhl                 硬件断点查看\r\n"
                         "bhc    bhc id (from bhl)   硬件断点删除\r\n"
                         "help   help                帮助\r\n"
                         "trace  trace addrbegin addrend [dll1] [dll2]  对指定区间的代码进行trace\r\n"
                         "?      ?                   帮助\r\n"
    );
  TDebugUI::ShowMsg(szBuf);
  return ERRCODE_SUCESS;
}

DWORD TDebugCmdLine::DoStepInto(Command& cmd)          //功能序号1 
{
  TDebug* lpDbg =  TDebug::GetTDebug();
  TStepFlags* lpDbgFlags = TStepFlags::GetStepFlags();
  if (!lpDbg->IsPause())
  {
    TDebugUI::ShowMsg("目标程序正在运行, 单步命令无效\r\n");
    return ERRCODE_FAILED;
  }
  if(!lpDbg->SetTrapFlag())
  {
    TDebugUI::ShowMsg("SetTrapFlag 失败\r\n");
    return ERRCODE_FAILED;
  }
  lpDbgFlags->m_bStepIn = TRUE;
  if(!lpDbg->RunTarget())
  {
    TDebugUI::ShowMsg("RunTarget 失败\r\n");
    return ERRCODE_FAILED;
  }
  return ERRCODE_SUCESS;
}

DWORD TDebugCmdLine::DoStepOver(Command& cmd)//功能序号2 
{
  DWORD dwCallLen = 0;
  TDebug* lpDbg =  TDebug::GetTDebug();
  TBPManager*  lpBpManager = TBPManager::GetBPManager();
  if(cmd[0].size() != 1)
  {
    TDebugUI::ShowMsg("CMD format not Correct !!!");
    return ERRCODE_FAILED;
  }
  TStepFlags* lpDbgFlags = TStepFlags::GetStepFlags();
  if (!lpDbg->IsPause())
  {
    TDebugUI::ShowMsg("目标程序正在运行, 单步命令无效\r\n");
    return ERRCODE_FAILED;
  }
  TARGET_CONTEXT stTarCon;
  if(!lpDbg->GetTragetContext(&stTarCon))
  {
    TDebugUI::ShowMsg("GetTragetContext Failed\r\n");
    return ERRCODE_FAILED;
  }

  dwCallLen = ::IsCallInstruction(stTarCon.hProcess, (LPVOID)(stTarCon.stContext.Eip));
  if(dwCallLen == 0)
  {
    if(!lpDbg->SetTrapFlag())
    {
      TDebugUI::ShowMsg("SetTrapFlag 失败\r\n");
      return ERRCODE_FAILED;
    }
    lpDbgFlags->m_bStepIn = TRUE;
  }
  else
  {
    LPVOID lpBpAddr = (LPVOID) (stTarCon.stContext.Eip+dwCallLen);
    lpBpManager->AddInnerBP( lpDbg->GetTargetPID(), lpBpAddr, INNER_BP_STEPOVER);
  }
  if(!lpDbg->RunTarget())
  {
    TDebugUI::ShowMsg("RunTarget 失败\r\n");
    return ERRCODE_FAILED;
  }
  return ERRCODE_SUCESS;
}


DWORD TDebugCmdLine::DoGo(Command& cmd)                //功能序号3 
{
  LPVOID lpPauseAddr = NULL;
  DWORD dwParamCnt = cmd.size(); 
  TDebug* lpDbg =  TDebug::GetTDebug();
  TStepFlags* lpDbgFlags  = TStepFlags::GetStepFlags();
  TBPManager*  lpBpManager = TBPManager::GetBPManager();
  if (!lpDbg->IsPause())
  {
    TDebugUI::ShowMsg("Target Process is Going!!\r\n");
  }
  if(dwParamCnt == 2)
  {
    lpPauseAddr = (LPVOID)strtol(cmd[1].c_str(), NULL, 16);
    lpBpManager->AddInnerBP( lpDbg->GetTargetPID(), lpPauseAddr, INNER_BP_GO);
    lpDbg->RunTarget();
  }
  else if(dwParamCnt == 1)
  {
 
    lpDbg->RunTarget();
  }
  else
  {
    TDebugUI::ShowMsg("CMD:GO Format Error , You can look up Help!!\r\n");
  }

  return ERRCODE_SUCESS;
}


DWORD TDebugCmdLine::DoTrace(Command& cmd)             //功能序号3.5
{
  LPVOID lpTraceBeginAddr = NULL;
  LPVOID lpTraceEndAddr   = NULL;
  DWORD dwParamCnt =  cmd.size();
  TDebug* lpDbg = TDebug::GetTDebug();
  TStepFlags* lpDbgFlags = TStepFlags::GetStepFlags();
  TBPManager*  lpBpManager = TBPManager::GetBPManager();
  if(dwParamCnt <3 || dwParamCnt >4)
  {
    TDebugUI::ShowMsg("too more/less Param !!\r\n");
    return ERRCODE_FAILED;
  }
  lpTraceBeginAddr = (LPVOID)strtol(cmd[1].c_str(), NULL, 16);
  lpTraceEndAddr   = (LPVOID)strtol(cmd[2].c_str(), NULL, 16);

  if(dwParamCnt==4)  
  {}else{}

  BOOL bPasue = lpDbg->IsPause();
  if(!bPasue)
  {
    lpDbg->PauseTarget();
  }
  lpBpManager->AddInnerBP(lpDbg->GetTargetPID(), lpTraceBeginAddr, INNER_BP_TRACE);
  lpDbgFlags->m_lpTraceEndAddr = lpTraceEndAddr;
  if(!bPasue)
  {
    lpDbg->RunTarget();
  }
  return ERRCODE_SUCESS;
}


DWORD TDebugCmdLine::DoShowASM(Command& cmd)           //功能序号4
{
  static LPVOID g_lpLastStartAddr = 0;
  TDebug* lpDbg =  TDebug::GetTDebug();
  TARGET_CONTEXT stTarCon;
  lpDbg->GetTragetContext(&stTarCon);
  LPVOID lpStartAddr = NULL;
  DWORD dwParamCnt = cmd.size(); 
  if(dwParamCnt == 1)
  {
    if(g_lpLastStartAddr == 0)
    {
      lpStartAddr = (LPVOID)stTarCon.stContext.Eip;
    }
    else
    {
      lpStartAddr = g_lpLastStartAddr;
    }
  }
  else if(dwParamCnt == 2)
  {
    lpStartAddr = (LPVOID)strtol(cmd[1].c_str(), NULL, 16);;
  }
  else
  {
    TDebugUI::ShowMsg("CMD　Format Not Correct !!!\r\n");
    return ERRCODE_FAILED;
  }
  
  ASM_CODE asmCode[10] = {0};
  if (lpDbg->IsPause())
  {
    ::GetAsmCode(stTarCon.hProcess, lpStartAddr, asmCode, sizeof(asmCode)/sizeof(asmCode[0]));
  }
  g_lpLastStartAddr = (LPVOID)(asmCode[9].m_dwCodeAddr  +  asmCode[9].m_dwCodeSize);

  TDebugUI::ShowAsm(asmCode, sizeof(asmCode)/sizeof(asmCode[0]));
  return ERRCODE_SUCESS;
}


DWORD TDebugCmdLine::DoShowData(Command& cmd)          //功能序号5 
{
  static BYTE g_btBuf[128];
  static LPVOID g_lpLastAddr = 0;
  TDebug* lpDbg =  TDebug::GetTDebug();
  TARGET_CONTEXT stTarCon;
  lpDbg->GetTragetContext(&stTarCon);
  LPVOID lpStartAddr = NULL;
  
  DWORD dwParamCnt = cmd.size(); 
  if(dwParamCnt == 1)
  {
    if(g_lpLastAddr == 0)
    {
      lpStartAddr = (LPVOID)stTarCon.stContext.Eip;
    }
    else
    {
      lpStartAddr = g_lpLastAddr;
    }
  }
  else if(dwParamCnt == 2)
  {
    lpStartAddr = (LPVOID)strtol(cmd[1].c_str(), NULL, 16);
  }
  else
  {
    TDebugUI::ShowMsg("CMD　Format Not Correct !!!\r\n");
    return ERRCODE_FAILED;
  }
  DWORD dwRead = NULL;
  BOOL bRet = ReadProcessMemory(stTarCon.hProcess, (LPVOID)lpStartAddr, g_btBuf, sizeof(g_btBuf), &dwRead);
  if (!bRet)
  {
    TDebugUI::ShowError();
  }
  g_lpLastAddr = (LPVOID)((DWORD)lpStartAddr + dwRead);

  TDebugUI::ShowMemory(lpStartAddr, g_btBuf, dwRead);
  return ERRCODE_SUCESS;
}

DWORD TDebugCmdLine::DoShowRegs(Command& cmd)          //功能序号6
{
  TDebug* lpDbg =  TDebug::GetTDebug();
  TARGET_CONTEXT stTarCon;
  lpDbg->GetTragetContext(&stTarCon);
  TDebugUI::ShowReg(&stTarCon.stContext);

  ::SafeCloseTargetContext(stTarCon);
  return ERRCODE_SUCESS;
}
DWORD TDebugCmdLine::DoEditData(Command& cmd)          //功能序号7
{
  DWORD dwParamCnt = cmd.size(); 
  LPVOID lpStartAddr=NULL;
  if(dwParamCnt != 2)
  {
    TDebugUI::ShowMsg("CMD　Format Not Correct !!!\r\n");
    return ERRCODE_FAILED;
   
  }
  lpStartAddr = (LPVOID)strtol(cmd[1].c_str(), NULL, 16);
  
  BYTE szBuf[0x100];
  BYTE bytData=0;
  TDebug* lpDbg =  TDebug::GetTDebug();
  TARGET_CONTEXT stTarCon;
  lpDbg->GetTragetContext(&stTarCon);
  DWORD dwRead = NULL;
  BOOL bRet = ReadProcessMemory(stTarCon.hProcess, (LPVOID)lpStartAddr, szBuf, sizeof(szBuf), &dwRead);
  if (!bRet)
  {
    TDebugUI::ShowError();
    return ERRCODE_FAILED;
  }
 
  char szData1 = 0;
  char szData2 = 0;
  char szData3 = 0;
  printf("%08X: ", lpStartAddr);
  for(int i =0; i<sizeof(szBuf)/sizeof(szBuf[0]); i++)
  {
    szData1 = getchar();
    szData2 = getchar();

    szData3 = getchar();
    if(szData3 != '\n' )
    {
       break;
    }
    else if(szData3 != ' ')
    {
      szBuf[i] = 16*szData1+szData2;
    }
    else
    {
      TDebugUI::ShowMsg("DATA Format !!!\r\n");
      return ERRCODE_FAILED;
    }    
  }

  bRet = WriteProcessMemory(stTarCon.hProcess, (LPVOID)lpStartAddr, szBuf, sizeof(szBuf), &dwRead);
  if (!bRet)
  {
    TDebugUI::ShowError();
    return ERRCODE_FAILED;
  }
  TDebugCmdLine::ContinueTalk();
  return ERRCODE_SUCESS;
}

DWORD TDebugCmdLine::DoSoftBreakPoint(Command& cmd)    //功能序号8
{
  LPVOID lpBpAddr = NULL;
  DWORD dwPid    = 0; 
  if(cmd.size() != 2)
  {
    TDebugUI::ShowMsg("too more/less Param !!\r\n");
    return ERRCODE_FAILED;
  }
  TDebug* lpDbg =  TDebug::GetTDebug();
  TARGET_CONTEXT stTarCon;
  lpDbg->GetTragetContext(&stTarCon);
  dwPid = lpDbg->GetTargetPID();
  lpBpAddr = (LPVOID)strtol(cmd[1].c_str(), NULL, 16);

  TBPManager* lpBpManager =   TBPManager::GetBPManager();
  BOOL bPasue = lpDbg->IsPause();
  if(!bPasue)
  {
    lpDbg->PauseTarget();
  }
  
  if(lpBpManager->AddSoftBP(dwPid, lpBpAddr) == ERRCODE_SUCESS)
  {
    TDebugUI::ShowMsg("Add Soft BP Successfully  !!\r\n");
  }
  else
  {
     TDebugUI::ShowMsg("Add Soft BP  Failed !!\r\n");
  }

  if(!bPasue)
  {
    lpDbg->RunTarget();
  }
  ::SafeCloseTargetContext(stTarCon);
  return ERRCODE_SUCESS;
}

DWORD TDebugCmdLine::DoListAllSoftBP(Command& cmd)     //功能序号9
{
  TBPManager* lpBpManager =   TBPManager::GetBPManager();

  TSoftBpList&  tSoftBpList = lpBpManager->GetAllSoftBP();
  if (tSoftBpList.empty())
  {
    TDebugUI::ShowMsg("No Soft Break Point! \r\n");
  }

  TSoftBpList::iterator it = tSoftBpList.begin();
  DWORD dwOrdinal = 0;

  TDebugUI::ShowMsg("\r\nOrdinal  Pid      Address    OldByte \r\n");
  while(it != tSoftBpList.end())
  {
    DWORD  dwPID = (*it)->m_dwPID;
    LPVOID lpBpAddr = (*it)->m_lpBpAddr;
    BYTE   btOldIns = (*it)->m_btOldInst;
    TDebugUI::ShowMsg("%-08d %-08d %p   %02X \r\n", dwOrdinal, dwPID, lpBpAddr, btOldIns);
    it++;
    dwOrdinal++;
  }
  return ERRCODE_SUCESS;
}

DWORD TDebugCmdLine::DoClearSoftBP(Command& cmd)       //功能序号10
{
  DWORD dwOrdin = (DWORD)strtol(cmd[1].c_str(), NULL, 10);
  TBPManager* lpBpManager =   TBPManager::GetBPManager();
  TSoftBpList&  tSoftBpList = lpBpManager->GetAllSoftBP();

  if (dwOrdin >=tSoftBpList.size())
  {
     TDebugUI::ShowMsg("No This Break Point! \r\n");
     return ERRCODE_FAILED;
  }
  lpBpManager->DelSoftBP(dwOrdin);
  
  TDebugUI::ShowMsg("Del  dwOrdin:%d Soft BP Sucess !\r\n", dwOrdin);
  return ERRCODE_FAILED;
} 


/*命令：bh
 *参数一：断点地址
 *参数二：断点类型。访问、写入、执行三者之一。
 *参数三：断点长度(可为1,2,4中的一个)。
 *说明：设置一个由参数所指定的硬件断点。
 *示例：
 *bh 0400321A e(硬件断点如果是执行类型，可不指定断点长度)
 *bh 0400321A a 2
 *bh 0400321A w 4
 */
DWORD TDebugCmdLine::DoHardBreakPoint(Command& cmd)    //功能序号11
{
  DWORD  dwPID = 0;
  LPVOID lpBpAddr = NULL;
  DWORD  dwReg = DR_UNKOWN;  
  DWORD  dwBPType = BP_ACTION_UNKOWN;
  DWORD  dwLen  = BP_LEN_BYTE;
  DWORD  dwTemp = 0;
  DWORD  dwResult=0;
  DWORD dwParamCnt =  cmd.size();

  TDebug* lpDbg = TDebug::GetTDebug();
  TBPManager* lpBPManager = TBPManager::GetBPManager();
  if(dwParamCnt <3 || dwParamCnt >4)
  {
    TDebugUI::ShowMsg("too more/less Param !!\r\n");
    return ERRCODE_FAILED;
  }
  lpBpAddr = (LPVOID)strtol(cmd[1].c_str(), NULL, 16);

  if(dwParamCnt==3  && (cmd[2]=="e"||cmd[2]=="E"))  //设置硬件执行断点
  {
    dwBPType = BP_ACTION_EXCUTE;
  }
  
  if(dwParamCnt == 4)
  {
    if(cmd[2] == "a" || cmd[2] == "A")
    {
      dwBPType = BP_ACTION_ACCESS;
    }
    else if(cmd[2] == "w" || cmd[2] == "W")
    {
      dwBPType = BP_ACTION_WRITE;
    }
  }
  if(dwBPType== BP_ACTION_UNKOWN)
  {
    TDebugUI::ShowMsg("Cmd format not correct !!\r\n");
    return ERRCODE_FAILED;
  }

  if( dwBPType == BP_ACTION_EXCUTE)
  {
    dwLen = BP_LEN_BYTE;
  }
  else
  {
    dwTemp = (DWORD)strtol(cmd[3].c_str(), NULL, 10);
    if(dwTemp == 4)
    {
      dwLen = BP_LEN_DWORD;
    }
    else if(dwTemp ==2)
    {
      dwLen = BP_LEN_WORD;
    }
    else if(dwTemp ==1)
    {
      dwLen = BP_LEN_BYTE;
    }
    else 
    {
      TDebugUI::ShowMsg("Cmd format not correct !!\r\n");
      return ERRCODE_FAILED;
    }
  }

  if ((DWORD)lpBpAddr % 2 !=0 && dwLen != BP_LEN_BYTE)
  {
    TDebugUI::ShowMsg("Address  %p Hard BrakPoint's len must be BYTE\r\n",lpBpAddr);
    return ERRCODE_FAILED;
  }
  if ((DWORD)lpBpAddr % 4 !=0 && dwLen == BP_LEN_DWORD)
  {
    TDebugUI::ShowMsg("Address: %p Hard BrakPoint's len must be BYTE or WORD\r\n", lpBpAddr);
    return ERRCODE_FAILED;
  }


  BOOL bPasue = lpDbg->IsPause();
  if(!bPasue)
  {
    lpDbg->PauseTarget();
  }
  dwPID = lpDbg->GetTargetPID();

  dwResult = lpBPManager->AddHardBP(dwPID, lpBpAddr, dwBPType, dwLen); 
  if(dwResult != ERRCODE_SUCESS)
  {
    if(dwResult == ERRCODE_OVER_NUM)
    {
      TDebugUI::ShowMsg("hard breakPoint num over four !!!\r\n");
      return ERRCODE_FAILED;
    }
    else if(dwResult == ERRCODE_BP_EXISTED)
    {
      TDebugUI::ShowMsg("Break Point has existed!!!\r\n");
      return ERRCODE_FAILED;
    }
    else if(dwResult == ERRCODE_BP_ENABLE_FAILED)
    {
      TDebugUI::ShowMsg("Hard Bp Enable Failed!!!\r\n");
      return ERRCODE_FAILED;
    }
  }


  if(!bPasue)
  {
    lpDbg->RunTarget();
  }

  return ERRCODE_SUCESS;
}

static char* g_alpHardBpType[4] ={ "EXECUTE", "WRITE", "UNKNOWN", "ACCESS"};
static char* g_alpHardBpLen[4]  ={"BYTE", "WORD", "UNKOWN", "DWORD"};


DWORD TDebugCmdLine::DoListAllHardBP(Command& cmd)     //功能序号12
{
  DWORD  dwPID     =    0;
  LPVOID lpBpAddr  =    0;
  DWORD  dwReg     =    0;
  DWORD  dwBPType  =    0;
  DWORD  dwLen     =    0;
  TBPManager* lpBPManager = TBPManager::GetBPManager();
  THardBpList& tHardBpList =  lpBPManager->GetAllHardBP();
  if (tHardBpList.empty())
  {
    TDebugUI::ShowMsg("No Soft Break Point! \r\n");
    return ERRCODE_SUCESS;
  }
  THardBpList::iterator it = tHardBpList.begin();
  DWORD dwOrdinal = 0;
  TDebugUI::ShowMsg("\r\nOrdinal  Pid      Address    Register  Type      Length\r\n");
  while(it != tHardBpList.end())
  {
    dwPID    = (*it)->m_dwPID;
    lpBpAddr = (*it)->m_lpBpAddr;
    dwReg    = (*it)->m_dwReg;  
    dwBPType = (*it)->m_dwBPType;
    dwLen    = (*it)->m_dwLen;
    
    TDebugUI::ShowMsg("%-08X %-08d %08X   DR%d       %-8s  %-8s\r\n", 
                     dwOrdinal, dwPID, lpBpAddr, dwReg, 
                     g_alpHardBpType[dwBPType], 
                     g_alpHardBpLen[dwLen]);
    it++;
    dwOrdinal++;
  }
  return ERRCODE_SUCESS;
}
DWORD TDebugCmdLine::DoClearHardBP(Command& cmd)       //功能序号13
{
  DWORD dwOrdin = (DWORD)strtol(cmd[1].c_str(), NULL, 10);
  TBPManager* lpBpManager =   TBPManager::GetBPManager();
  THardBpList&  tSoftBpList = lpBpManager->GetAllHardBP();

  if (dwOrdin >=tSoftBpList.size())
  {
    TDebugUI::ShowMsg("No This Break Point! \r\n");
    return ERRCODE_FAILED;
  }
  lpBpManager->DelHardBP(dwOrdin);

  TDebugUI::ShowMsg("Del  dwOrdin:%d Hard BP Sucess !\r\n", dwOrdin);
  return ERRCODE_SUCESS;
}



DWORD TDebugCmdLine::DoMemBreakPoint(Command& cmd)    //功能序号14
{
  DWORD  dwPID = 0;
  LPVOID lpBpAddr = NULL;
  DWORD  dwBpLen = 0 ;
  DWORD  dwBpType = 0;
  DWORD  dwRet = 0;

  DWORD dwParamCnt =  cmd.size();

  if(dwParamCnt != 4)
  {
    TDebugUI::ShowMsg("too more/less Param !!\r\n");
    return ERRCODE_FAILED;
  }

  lpBpAddr = (LPVOID)strtol(cmd[1].c_str(), NULL, 16);
  dwBpLen  = (DWORD)strtol(cmd[3].c_str(), NULL, 16);

  if(cmd[2] == "a" || cmd[2] == "A")
  {
    dwBpType = MEM_ACCESS;
  }
  else if(cmd[2] == "w" || cmd[2] == "W")
  {
    dwBpType = MEM_WRITE;
  }
  else
  {
    TDebugUI::ShowMsg("cmd format not coorect ！！！\r\n");
    return ERRCODE_FAILED;
  }

  TDebug* lpDbg = TDebug::GetTDebug();
  TBPManager* lpBpManager = TBPManager::GetBPManager();
  BOOL bPasue = lpDbg->IsPause();
  dwPID = lpDbg->GetTargetPID();
  if(!bPasue)
  {
    lpDbg->PauseTarget();
  }
  
  dwRet = lpBpManager->AddMemBP(dwPID, lpBpAddr, dwBpLen,dwBpType);
  
  if(dwRet != ERRCODE_SUCESS)
  {
    if(dwRet == ERRCODE_MEM_BP_CONFILT)
    {
        TDebugUI::ShowMsg("Mem BreakPoint Confict ！！！\r\n");
    }
    else
    {
      TDebugUI::ShowMsg("AddMemBP Failed ！！！\r\n");
    }
    
    return ERRCODE_FAILED;
  }

  if(!bPasue)
  {
    lpDbg->RunTarget();
  }
  return ERRCODE_SUCESS;
}

static char* g_alpMemType[2]  ={"MEM_ACCESS", "MEM_WRITE"};
DWORD TDebugCmdLine::DoListAllMemBP(Command& cmd)     //功能序号15
{
  TBPManager* lpBPManager = TBPManager::GetBPManager();
  TMemBpList& tMemBpList =  lpBPManager->GetMemBPList();
  if (tMemBpList.empty())
  {
    TDebugUI::ShowMsg("No Mem Break Point! \r\n");
    return ERRCODE_SUCESS;
  }

  TMemBpList::iterator it = tMemBpList.begin();
  DWORD dwOrdinal = 0;
  DWORD  dwPID    = 0;
  LPVOID lpBpAddr = 0;
  DWORD  dwBpLen  = 0;
  DWORD  dwBPType = 0;

  TDebugUI::ShowMsg("%-8s %-8s %-8s    %-8s  %-8s\r\n","Ordinal","Pid","Address","Len", "TYPE");
  while(it != tMemBpList.end())
  {
    dwPID    = (*it)->m_dwPID;
    lpBpAddr = (*it)->m_lpBPAddr;
    dwBpLen  = (*it)->m_dwMemLen;  
    dwBPType = (*it)->m_dwBPType;

    TDebugUI::ShowMsg("%-08d %08X %08X    %08X  %8s\r\n", 
      dwOrdinal, dwPID, lpBpAddr, dwBpLen,  g_alpMemType[dwBPType]);
    it++;
    dwOrdinal++;
  }
  return ERRCODE_SUCESS;
}

DWORD TDebugCmdLine::DoListMemBpPage(Command& cmd)             //功能序号16
{
  TBPManager* lpBPManager = TBPManager::GetBPManager();
  TMemBpList& tMemBpList =  lpBPManager->GetMemBPList();
  TMembpPageList& tMemBpPageList = lpBPManager->GetMembpPageList();
  TPageList& tPageList = lpBPManager->GetPageList();
  if (tMemBpList.empty())
  {
    TDebugUI::ShowMsg("No Mem Break Point! \r\n");
    return ERRCODE_SUCESS;
  }

  DWORD  dwOrdinal = 0;
  DWORD  dwPID    = 0;
  LPVOID lpBpAddr = 0;
  DWORD  dwBpLen  = 0;
  DWORD  dwBPType = 0;
  DWORD  dwPageBase =0;
  DWORD  dwOldProtectd=0;
  TDebugUI::ShowMsg("%-8s %-8s %-8s %-8s    %-8s    %-12s %-8s %-8s\r\n",
                   "PageBase", 
                   "Ordinal", 
                   "Pid",
                   "Address",
                   "Len", 
                   "TYPE",
                   "Old",
                   "New");
  TPageList::iterator itPage = tPageList.begin();
  while(itPage != tPageList.end())
  {
    dwPageBase    = (*itPage)->m_dwPageBase;
    dwOldProtectd = (*itPage)->m_dwOldProtect;
    TMembpPageList::iterator itMembpPage = tMemBpPageList.begin();
    while (itMembpPage != tMemBpPageList.end())
    {
      if(*itPage == (*itMembpPage)->m_lpPage)
      {
        
        TMemBpList::iterator itMembp = tMemBpList.begin();
        DWORD i =0;
        while (itMembp != tMemBpList.end())
        {
          if((*itMembpPage)->m_lpMemBP  == *itMembp)
          {
            dwOrdinal = i;
            dwPID    =  (*itMembp)->m_dwPID;
            lpBpAddr =  (*itMembp)->m_lpBPAddr;
            dwBpLen  =  (*itMembp)->m_dwMemLen;
            dwBPType =  (*itMembp)->m_dwBPType;
            dwPageBase = (*itPage)->m_dwPageBase;
            dwOldProtectd=(*itPage)->m_dwOldProtect;
            TDebugUI::ShowMsg("%08X %-08d %08X %08X    %08X  %12s %08X %08X\r\n", 
                              dwPageBase,
                              dwOrdinal, 
                              dwPID,
                              lpBpAddr, 
                              dwBpLen,
                              g_alpMemType[dwBPType],
                              dwOldProtectd,
                              PAGE_NOACCESS);
           

          }
          itMembp++;
          i++;
        }

      }
      itMembpPage++;
    }



 
    itPage++;

  }
  return ERRCODE_SUCESS;
  return ERRCODE_SUCESS;
} 



DWORD TDebugCmdLine::DoClearMemBP(Command& cmd)       //功能序号17
{
  DWORD dwOrdin = (DWORD)strtol(cmd[1].c_str(), NULL, 10);
  TBPManager* lpBpManager =   TBPManager::GetBPManager();
  TMemBpList& tMemBpList =  lpBpManager->GetMemBPList();
  
  if(cmd.size()!=2)
  {
    TDebugUI::ShowMsg("too more/less Param !!\r\n");
    return ERRCODE_FAILED;
  }

  if (dwOrdin >=tMemBpList.size())
  {
    TDebugUI::ShowMsg("No This Break Point! \r\n");
    return ERRCODE_FAILED;
  }
  
  if( lpBpManager->DelMemBP(dwOrdin) != ERRCODE_SUCESS)
  {
    TDebugUI::ShowMsg("Del  dwOrdin:%d Mem Filed Sucess  \r\n",dwOrdin);
    return ERRCODE_FAILED;
  }
  TDebugUI::ShowMsg("Del  dwOrdin:%d Mem BreakPoint Sucess !\r\n", dwOrdin);
  return ERRCODE_SUCESS;
} 


DWORD TDebugCmdLine::DoLoadScript(Command& )        //功能序号18  
{
  char szFilePath[MAX_PATH];
  if(!::SelectFile(szFilePath, sizeof(szFilePath)))
  {  
    return ERRCODE_FAILED;
  }

  std::fstream fin(szFilePath,std::ios::in);
  char line[1024]={0};
  ContinueTalk();
  while(fin.getline(line, sizeof(line)))
  {
    WaitForSingleObject(g_hTalk, INFINITE);
    Command loadCmd;
    std::stringstream word(line);
    ParseCommand(line, loadCmd);
    if(loadCmd[0] == "ls" || loadCmd[0] == "es" )
    {
      TDebugUI::ShowMsg("Script cannot exit cmd:%s  \r\n", loadCmd[0].c_str());
      continue;
    }
    cout << "TDebug>" << line <<endl;
    ExcuteCmd(loadCmd);
    
    Sleep(100);  //等待异常处理完成；
    ContinueTalk();
  }
  return ERRCODE_SUCESS;
}

DWORD TDebugCmdLine::DoExportScript(Command& cmd)     //功能序号19  
{
  char szFilePath[MAX_PATH];
  HANDLE hExportFile=NULL;
  DWORD dwWrittenByte;
  if(!SaveFile(szFilePath, sizeof(szFilePath)))
  {  
    return ERRCODE_FAILED;
  }
  hExportFile=CreateFile(szFilePath,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,
    NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL,0);

  if(!WriteFile(hExportFile,m_strHisCmds.c_str(), m_strHisCmds.length(), &dwWrittenByte, NULL))
  {
    TDebugUI::ShowError();
  }
  m_strHisCmds = "";
  TDebugUI::ShowMsg("EXPORT Script Sucssfully ！！！\r\n");

  return ERRCODE_SUCESS;
}

DWORD TDebugCmdLine::DoFinish(Command& cmd)           //功能序号20  
{

  TDebug* lpDbg =  TDebug::GetTDebug();
  DWORD dwPID = lpDbg->GetTargetPID();
  m_bQuit = TRUE;
  HANDLE hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID);
  ::TerminateProcess(hProcess, 0);
  SafeClose(hProcess);
  return ERRCODE_SUCESS;
}

DWORD TDebugCmdLine::DoListModule(Command& cmd)      //功能序号21
{
  TDebug* lpDbg =  TDebug::GetTDebug();
  DWORD dwPID = lpDbg->GetTargetPID();

  MODULEENTRY32 me32        = {0}; 
  HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID); 
  if (hThreadSnap == INVALID_HANDLE_VALUE) 
    return (FALSE); 

  me32.dwSize = sizeof(me32); 
  TDebugUI::ShowMsg("Modules\r\n");
  if (Module32First(hThreadSnap, &me32)) 
  { 
    do 
    {
      if(me32.th32ProcessID == dwPID)
      {
        
        TDebugUI::ShowMsg("%s\r\n", me32.szExePath);
      }

    } while (Module32Next(hThreadSnap, &me32));
  }
  SafeClose(hThreadSnap);
  return ERRCODE_SUCESS;
}

DWORD TDebugCmdLine::DoHistory(Command& cmd)
{
  TDebugUI::ShowMsg(m_strHisCmds.c_str());
  return ERRCODE_SUCESS;
}

typedef map<DWORD, string> ModuleBaseToNameMap;

DWORD TDebugCmdLine::DoShowCallStack(Command& cmd)
{
  TDebug* lpDbg =  TDebug::GetTDebug();
  //枚举模块，建立模块的基址-名称表
  ModuleBaseToNameMap moduleMap;
  DWORD dwPID = lpDbg->GetTargetPID();
  MODULEENTRY32 me32        = {0}; 
  HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID); 
  if (hThreadSnap == INVALID_HANDLE_VALUE) 
    return (FALSE); 

  me32.dwSize = sizeof(me32); 
  if (Module32First(hThreadSnap, &me32)) 
  { 
    do 
    {
      if(me32.th32ProcessID == dwPID)
      {
        moduleMap[(DWORD)me32.modBaseAddr] = me32.szModule;
      }

    } while (Module32Next(hThreadSnap, &me32));
  }
  SafeClose(hThreadSnap);


  TARGET_CONTEXT stTarCon;
  lpDbg->GetTragetContext(&stTarCon);
  HANDLE hProcess = stTarCon.hProcess;
  HANDLE hThread  = stTarCon.hThread;
  STACKFRAME stackFrame = { 0 };

  stackFrame.AddrPC.Mode = AddrModeFlat;
  stackFrame.AddrPC.Offset = stTarCon.stContext.Eip;
  stackFrame.AddrStack.Mode = AddrModeFlat;
  stackFrame.AddrStack.Offset = stTarCon.stContext.Esp;
  stackFrame.AddrFrame.Mode = AddrModeFlat;
  stackFrame.AddrFrame.Offset = stTarCon.stContext.Ebp;

  while (true) {

    //获取栈帧
    if (StackWalk(
      IMAGE_FILE_MACHINE_I386,
      hProcess,
      hThread,
      &stackFrame,
      &stTarCon.stContext,
      NULL,
      SymFunctionTableAccess,
      SymGetModuleBase,
      NULL) == FALSE) {

        break;
    }

    printf("%08X", (DWORD)stackFrame.AddrPC.Offset);
    std::wcout << TEXT("  ");

    //显示模块名称
    DWORD moduleBase = (DWORD)SymGetModuleBase(hProcess, stackFrame.AddrPC.Offset);

    string moduleName = moduleMap[moduleBase];

   if (moduleName.length() != 0) {
      std::cout << moduleName;
    }
    else {
      std::wcout << TEXT("??");
    }

    std::wcout << TEXT('!');

    //显示函数名称
    BYTE buffer[sizeof(SYMBOL_INFO) + 128 * sizeof(TCHAR)] = { 0 };
    PSYMBOL_INFO pSymInfo = (PSYMBOL_INFO)buffer;
    pSymInfo->SizeOfStruct = sizeof(SYMBOL_INFO);
    pSymInfo->MaxNameLen = 128;

    DWORD64 displacement;

    if (SymFromAddr(
      hProcess,
      stackFrame.AddrPC.Offset,
      &displacement,
      pSymInfo) == TRUE) {

        std::wcout << pSymInfo->Name << std::endl;
    }
    else {

      std::wcout << TEXT("??") << std::endl;
    }
  }


  SafeCloseTargetContext(stTarCon);
  hProcess = NULL;
  return ERRCODE_SUCESS;
}

