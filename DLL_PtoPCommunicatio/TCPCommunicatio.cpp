#include "StdAfx.h"
#include <IPHlpApi.h>  // 操作网卡相关的函数库头文件
#include "TCPCommunicatio.h"
/***************************************************************************
* 函数名称：[myFunction]
* 摘 要：简要描述本函数功能
* 全局影响：
* 参数：param1:[in/out] 参数作用描述
* 返回值：
*
* 修改记录：
*[日期]
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/
CTCPCommunicatio::CTCPCommunicatio(void)
:m_uiCurrentThdNum(0)
{
    InitializeCriticalSection(&(m_Crt_Thd_ClientObj));
    InitializeCriticalSection(&(m_Crt_ListenObj));
}
/***************************************************************************
* 函数名称：[myFunction]
* 摘 要：简要描述本函数功能
* 全局影响：
* 参数：param1:[in/out] 参数作用描述
* 返回值：
*
* 修改记录：
*[日期]
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/
CTCPCommunicatio::~CTCPCommunicatio(void)
{
    DeleteCriticalSection(&(m_Crt_Thd_ClientObj));
    DeleteCriticalSection(&(m_Crt_ListenObj));
}
/***************************************************************************
* 函数名称：[myFunction]
* 摘 要：简要描述本函数功能
* 全局影响：
* 参数：param1:[in/out] 参数作用描述
* 返回值：
*
* 修改记录：
*[日期]
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/
BOOL CTCPCommunicatio::InitTCPCom()
{
    //加载套接字版本库
    if (!InitialSocket())
    {
        return FALSE;
    }

    return TRUE;
}
/***************************************************************************
* 函数名称：[myFunction]
* 摘 要：简要描述本函数功能
* 全局影响：
* 参数：param1:[in/out] 参数作用描述
* 返回值：
*
* 修改记录：
*[日期]
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/
BOOL CTCPCommunicatio::UnInitTCPCom()
{

    //卸载套接字版本库
    if (!UnInitialSocket())
    {
        return FALSE;
    }
    return TRUE;
}
/***************************************************************************
* 函数名称：[myFunction]
* 摘 要：简要描述本函数功能
* 全局影响：
* 参数：param1:[in/out] 参数作用描述
* 返回值：
*
* 修改记录：
*[日期]
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/
BOOL  CTCPCommunicatio::InitialSocket()
{
    int iInitialError;
    WSADATA data;  //初始化套接字
    iInitialError=WSAStartup(MAKEWORD(2,2),&data); //2.2版本套接字
    if (0!=iInitialError)
    {
        CCsccLog::DebugPrint("套接字版本加载错误！\n");
        return FALSE;
    }
    return TRUE;
}
/***************************************************************************
* 函数名称：[UnInitialSocket]
* 摘 要：简要描述本函数功能
* 全局影响：
* 参数：param1:[in/out] 参数作用描述
* 返回值：
*
* 修改记录：
*[日期]
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/
BOOL  CTCPCommunicatio::UnInitialSocket()
{
    int iUnInitialErr=0;
    iUnInitialErr=WSACleanup();
    if (0!=iUnInitialErr)
    {
        if (WSANOTINITIALISED==WSAGetLastError())
        {
            CCsccLog::DebugPrint("卸载套接字版本库之前请正确加载版本库！\n");
        }	
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/***************************************************************************
* 函数名称：[SetListenObj]
* 摘 要：该函数创建一个监听套接字对象，一般在调用函数BeginListen时调用。
* 全局影响：
* 参数：PST_LISTEN_OBJ pstListenObj [out] 新创建的监听套接字对象
*       BYTE abyServerIP[IP_BYTE_LENGTH][in] 服务器监听IP地址
*       UINT uiServerPort [in] 服务器监听端口
*       UINT uiConnectNum [in] 服务器所支持的最大连接数
* 返回值：int 
*
* 修改记录：
*[日期]
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/
INT CTCPCommunicatio::SetListenObj(PST_LISTEN_OBJ &pstListenObj, BYTE abyServerIP[IP_BYTE_LENGTH],
                                   UINT uiServerPort, UINT uiConnectNum)
{
    USES_CONVERSION;				//在ATL下使用要包含头文件#include "atlconv.h"

    // 判断是否已经处于监听状态
    if (IsListen(abyServerIP, uiServerPort))
    {	
        // 已经处于监听状态
        return WSA_IS_MEMBER;
    }

    // 为监听套接字对象申请内存区
    pstListenObj = new ST_LISTEN_OBJ;

    //初始化访问线程对象关键段，每个监听套接字在接收到客户连接后将新建的套
    //接字对象加入空闲线程对象中，此时需要访问线程对象链表。在本库中，每个
    //监听套接字对应一个线程对象链表,以表明该线程对象链表中的套接字均是由
    //同一个监听套接字接受连接产生的。
    InitializeCriticalSection(&(pstListenObj->crt_ThreadObj));

    // 创建监听套接字
    pstListenObj->sktListen = socket(AF_INET,		// 协议地址家族，windows下一般为AF_INET
                                    SOCK_STREAM,	// 协议套接字类型，此处TCP采用流式套接字
                                    IPPROTO_TCP);	// 协议，此处采用TCP协议

    // 判断监听套接字是否创建成功
    if (INVALID_SOCKET == (pstListenObj->sktListen))
    {
        // 得到错误码
        int iSocketErr = WSAGetLastError();

        if (iSocketErr == WSANOTINITIALISED)
        {  
        }		
        // 创建套接字失败，返回不可用的套接字
        return FALSE;
    }

    // 定义错误代码 
    int iErrCode;

    // 待设置的套接字缓冲区大小
    UINT uiBuflen;

    // 缓冲区大小
    UINT uiOptlen = sizeof(uiBuflen);

    // 获取发送缓冲区参数
    iErrCode = getsockopt(pstListenObj->sktListen,	//套接字
                            SOL_SOCKET,				//套接字级别，此处选择套接字所在的应用层
                            SO_SNDBUF,				//将要设置的套接字选项名称，此处为缓冲区大小
                            (char *)&uiBuflen,		//设置的套接字选项值
                            (int*)&uiOptlen);		//缓冲区大小

    // 判断获取缓冲区参数是否异常
    if (SOCKET_ERROR == iErrCode)                                              
    {
        CCsccLog::DebugPrint("获取发送缓冲区大小失败！\n");
        return INVALID_SOCKET;
    }

    // 将原先的缓冲区大小扩大十倍，系统默认为8192byte
    uiBuflen *= 10;

    // 设定缓冲区大小                                                           
    iErrCode = setsockopt(pstListenObj->sktListen, SOL_SOCKET, SO_RCVBUF, (char *)&uiBuflen,uiOptlen);

    // 判断设置缓冲区大小是否异常
    if (SOCKET_ERROR == iErrCode)                                              
    {
        CCsccLog::DebugPrint("设置发送缓冲区大小失败！\n");
        return INVALID_SOCKET;
    }

    // 检查缓冲区大小是否修改成功
    unsigned int uiNewRcvBuf;                                               
    iErrCode = getsockopt(pstListenObj->sktListen, SOL_SOCKET, SO_SNDBUF, (char*)&uiNewRcvBuf, (int*)&uiOptlen);
    if (SOCKET_ERROR == iErrCode || uiNewRcvBuf == uiBuflen)
    {
        CCsccLog::DebugPrint("修改系统发送数据缓冲区失败！\n");
        return INVALID_SOCKET;
    }

    // 服务器IP地址
    CString strServerIp = _T("");
    strServerIp.Format(_T("%d.%d.%d.%d"), abyServerIP[0], abyServerIP[1], abyServerIP[2], abyServerIP[3]);
    char* pcServerIP = "";
    pcServerIP = W2A(strServerIp);

    // 绑定服务器IP地址
    sockaddr_in sdServerAddr;									// 服务器端地址簇
    sdServerAddr.sin_family = AF_INET;							// 协议地址家族，windows下一般为AF_INET
    sdServerAddr.sin_addr.S_un.S_addr = inet_addr(pcServerIP);	// 服务器IP
    sdServerAddr.sin_port = htons(uiServerPort);				// 端口号,主机字节序三转换为网络字节序

    // 绑定套接字
    int iBindErr = bind(pstListenObj->sktListen,(LPSOCKADDR)&sdServerAddr,sizeof(sdServerAddr));
    if (iBindErr == SOCKET_ERROR)
    {
        return FALSE;
    }

    // 初始线程对象个数
    pstListenObj->uiCurrentThdNum = 0;

    // 服务器监听地址及端口
    memcpy(pstListenObj->abyServerIp, abyServerIP, IP_BYTE_LENGTH);
    pstListenObj->uiServerPort = uiServerPort;

    // 创建监听套接字关联的事件
    pstListenObj->hListenEvent = WSACreateEvent();

    // 注册网络事件
    WSAEventSelect(pstListenObj->sktListen, pstListenObj->hListenEvent, FD_ACCEPT | FD_WRITE | FD_CLOSE);

    // 当前类对象指针
    pstListenObj->lpClsObj = this;

    // 服务器退出事件，自动复位，默认无信号
    pstListenObj->hServerExit = CreateEvent(NULL, FALSE, FALSE, NULL);
    pstListenObj->hServerHasExit = CreateEvent(NULL, FALSE, FALSE, NULL);

    // 启动监听服务
    int iErrListen = listen(pstListenObj->sktListen, uiConnectNum);
    if (iErrListen == SOCKET_ERROR)
    {
        return FALSE;
    }

    // 启动监听套接字网络事件处理线程
    pstListenObj->pclsThdListen = AfxBeginThread(Thread_Listen_Proc, (LPVOID)(pstListenObj)); 
    Sleep(100);

    EnterCriticalSection(&(m_Crt_ListenObj));	

    // 将监听套接字对象加入链表
    m_lstListenObj.AddHead(pstListenObj); 

    LeaveCriticalSection(&m_Crt_ListenObj);

    return TRUE;
}
/***************************************************************************
* 函数名称：[IsListen]
* 摘 要：该函数判断主机是否已经处于监听状态
* 全局影响：
* 参数：BYTE abyServerIP[IP_BYTE_LENGTH] 主机监听IP地址
*       UINT uiServerPort 主机监听端口
* 返回值：主机已经处于监听状态则返回TRUE,否则，返回FALSE
*
* 修改记录：
* [日期]
* [作者/修改者]陈洞滨
* [修改原因]
***************************************************************************/
BOOL CTCPCommunicatio::IsListen(BYTE abyServerIP[IP_BYTE_LENGTH], UINT uiServerPort)
{
    BOOL bRetVal = FALSE;			// 返回值，初始化为FALSE

    // 服务器IP地址
    CString strServerIp = _T("");
    strServerIp.Format(_T("%d.%d.%d.%d"), abyServerIP[0], abyServerIP[1], abyServerIP[2], abyServerIP[3]);

    // 取得访问监听对象链表权限
    EnterCriticalSection(&m_Crt_ListenObj);

    for (POSITION posListen = m_lstListenObj.GetHeadPosition(); // 取得监听链表头结点
         posListen != NULL;										// 判断节点是否为NULL
         m_lstListenObj.GetNext(posListen))						// 移动遍历监听链表指针
    {
        // 取出posListen位置的节点
        PST_LISTEN_OBJ pstListenObj = m_lstListenObj.GetAt(posListen);

        // 节点对应的服务器IP地址
        CString strTempServerIp = _T("");
        strTempServerIp.Format(_T("%d.%d.%d.%d"), pstListenObj->abyServerIp[0], pstListenObj->abyServerIp[1],
                                pstListenObj->abyServerIp[2], pstListenObj->abyServerIp[3]);

        // 判断节点服务器信息是否与待判定服务器信息一致
        if (strTempServerIp == strServerIp && uiServerPort == pstListenObj->uiServerPort)
        {
            // 主机已经处于监听状态
            bRetVal = TRUE;
            break;
        }
    }

    // 释放对监听对象链表的访问权限
    LeaveCriticalSection(&m_Crt_ListenObj);

    return bRetVal;
}
/***************************************************************************
* 函数名称：[FreeListenObj]
* 摘 要：该函数释放一个处于监听中的套接字
* 全局影响：
* 参数：PST_LISTEN_OBJ pstListenObj:[in] 待删除的监听套接字对象
* 返回值：
*
* 修改记录：
*[日期]
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/
BOOL CTCPCommunicatio::FreeListenObj(PST_LISTEN_OBJ pstListenObj)
{
    ST_LOG stTcpLog;
    stTcpLog.iType=LOG_INFO;

    EnterCriticalSection(&(pstListenObj->crt_ThreadObj));
    //m_clsCsccLog.MyNSprintf(stTcpLog.swzMsg,MAX_LOG_LENGTH,"FreeListenObj EnterCriticalSection------pstListenObj->crt_ThreadObj");
    //m_clsCsccLog.WriteLog(stTcpLog.iType,stTcpLog.swzMsg);

    //清空线程对象
    PST_TCP_SERVER_THD_OBJ pstTcpServerThdObj=NULL;
    for (POSITION posNodeThreadObj=pstListenObj->lstTcpServerThdObj.GetHeadPosition();
         posNodeThreadObj!=NULL;
         pstListenObj->lstTcpServerThdObj.GetNext(posNodeThreadObj))
    {
        if (pstListenObj->lstTcpServerThdObj.IsEmpty())
        {
            break;
        }
        //清空链表中所有的线程对象
        pstTcpServerThdObj=pstListenObj->lstTcpServerThdObj.GetAt(posNodeThreadObj);
        EnterCriticalSection(&pstTcpServerThdObj->crt_Tcp_Server);
        for (POSITION posServerObj=pstTcpServerThdObj->lstTcpServer.GetHeadPosition();
             !pstTcpServerThdObj->lstTcpServer.IsEmpty();
             pstTcpServerThdObj->lstTcpServer.GetNext(posServerObj))
        {
            PST_TCP_SERVER_OBJ pstTcpServerObj=pstTcpServerThdObj->lstTcpServer.GetAt(posServerObj);
            RemoveTcpServerObj(pstTcpServerThdObj,pstTcpServerObj);
            FreeTcpServerObj(pstTcpServerObj);		
        }
        LeaveCriticalSection(&pstTcpServerThdObj->crt_Tcp_Server);

    }

    LeaveCriticalSection(&(pstListenObj->crt_ThreadObj));
    //m_clsCsccLog.MyNSprintf(stTcpLog.swzMsg,MAX_LOG_LENGTH,"FreeListenObj LeaveCriticalSection------pstListenObj->crt_ThreadObj");
    //m_clsCsccLog.WriteLog(stTcpLog.iType,stTcpLog.swzMsg);


    //通知监听套接字处理线程函数监听套接字关闭
    SetEvent(pstListenObj->hServerExit);
    /*WaitForSingleObject(pstListenObj->pclsThdListen->m_hThread,INFINITE);*/
    WaitForSingleObject(pstListenObj->hServerHasExit,INFINITE);
    CloseHandle(pstListenObj->hServerExit);
    CloseHandle(pstListenObj->hServerHasExit);
    //CloseHandle(pstListenObj->pclsThdListen->m_hThread); // 关闭线程句柄
    //清除线程对象链表关键段
    DeleteCriticalSection(&(pstListenObj->crt_ThreadObj));

    //关闭与监听套接字相关联事件句柄
    if (pstListenObj->hListenEvent!=NULL)
    {
        WSACloseEvent(pstListenObj->hListenEvent);
        //m_clsCsccLog.MyNSprintf(stTcpLog.swzMsg,MAX_LOG_LENGTH,"FreeListenObj WSACloseEvent------pstListenObj->hListenEvent");
        //m_clsCsccLog.WriteLog(stTcpLog.iType,stTcpLog.swzMsg);
    }
    else
    {
        CCsccLog::DebugPrint("关闭监听套接字事件出错！\n");
        return FALSE;
    }

    
    pstListenObj->pclsThdListen=NULL;

    //关闭监听套接字
    if (pstListenObj->sktListen!=INVALID_SOCKET) 
    {
        //关闭监听套接字
        closesocket(pstListenObj->sktListen);
    }
    else
    {
        CCsccLog::DebugPrint("关闭监听套接字出错！\n");
        return FALSE;
    }
    pstListenObj->lpClsObj=NULL;
    //删除监听套接字对象
    delete pstListenObj;
    pstListenObj=NULL;


    return TRUE;
}

/***************************************************************************
* 函数名称：[HandleIO]
* 摘 要：该函数负责处理监听套接字接受新连接
* 全局影响：
* 参数：PST_LISTEN_OBJ pstListenObj:[in] 监听套接字对象
* 返回值：
*
* 修改记录：
*[日期]
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/
BOOL CTCPCommunicatio::HandleListenIO(PST_LISTEN_OBJ pstListenObj)
{
    BOOL bretVal = FALSE;			//返回值，初始化为FALSE

    WSANETWORKEVENTS  evntsNetWork; //网络事件结构

    //检索指定监听套接字上的网络事件，该函数将复位与套接字关联的事件
    if (WSAEnumNetworkEvents(pstListenObj->sktListen, pstListenObj->hListenEvent, &evntsNetWork) == SOCKET_ERROR)
    {
        CCsccLog::DebugPrint("获取监听网络事件错误！\n");
    }

    do 
    { 
        // 套接字有新的连接
        if (evntsNetWork.lNetworkEvents & FD_ACCEPT)     
        {
            if (evntsNetWork.iErrorCode[FD_ACCEPT_BIT] != 0)
            {
                CCsccLog::DebugPrint("监听套接字接受连接发生网络错误！\n");
                break;
            } 
            else		// 有新的连接
            {
                SOCKADDR_IN sdClient;
                int isdClientLen = sizeof(sdClient);

                // 建立连接
                SOCKET sktAccept = accept(pstListenObj->sktListen, (SOCKADDR*)&sdClient, &isdClientLen);
                if (sktAccept == SOCKET_ERROR)
                {
                    break;
                }

                DWORD dwClientIp = 0;					// 连接服务器的客户端IP地址
                UINT  uiClientPort = 0;					// 连接服务器的客户端端口

                dwClientIp = ntohl(sdClient.sin_addr.S_un.S_addr);
                uiClientPort = ntohs(sdClient.sin_port);
                BYTE abyClientIp[IP_BYTE_LENGTH];
                abyClientIp[0] = HIBYTE(HIWORD(dwClientIp));
                abyClientIp[1] = LOBYTE(HIWORD(dwClientIp));
                abyClientIp[2] = HIBYTE(LOWORD(dwClientIp));
                abyClientIp[3] = LOBYTE(LOWORD(dwClientIp));

                PST_TCP_SERVER_OBJ pstTcpServerObj = NULL;

                // 服务器新增一个与客户端通信的套接字对象
                SetServerObj(pstTcpServerObj, sktAccept, abyClientIp, uiClientPort, pstListenObj);
            }
        }
        else if (evntsNetWork.lNetworkEvents&FD_WRITE)
        {
            CCsccLog::DebugPrint("可以接受连接\n");
        }
        else if (evntsNetWork.lNetworkEvents & FD_CLOSE) //客户端断开链接
        {
            break;
        }
        bretVal = TRUE;

    } while (FALSE);

    return bretVal;
}
/***************************************************************************
* 函数名称：[Thread_Listen_Proc]
* 摘 要：该函数完成服务器端的监听过程
* 全局影响：
* 参数：param1:[in/out] 参数作用描述
* 返回值：
*
* 修改记录：
*[日期]
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/

unsigned int Thread_Listen_Proc(LPVOID lpvoid)
{
    // 取出线程参数
    PST_LISTEN_OBJ pstListenObj = (PST_LISTEN_OBJ)lpvoid;
    CTCPCommunicatio *pclsTcpCom = (CTCPCommunicatio*)pstListenObj->lpClsObj;

    while(TRUE)
    {
        // 如果服务器退出，则清理掉该监听套接字
        if (WAIT_OBJECT_0 == WaitForSingleObject(pstListenObj->hServerExit, 0))
        {
            break;
        } 

        // 逐个查看是否有网络事件
        int iEventIndex = WSAWaitForMultipleEvents(1,							//每次查看一个网络事件
                                                    &pstListenObj->hListenEvent,//网络事件对应的事件
                                                    TRUE,						//必须等待事件全部授信
                                                    SERVER_WAIT_TIME,			//等待的时间
                                                    FALSE);						//该模型中，此处为固定值

        //函数调用失败或者等待超时
        if (iEventIndex == WSA_WAIT_FAILED || iEventIndex == WSA_WAIT_TIMEOUT)
        {
            Sleep(SERVER_WAIT_TIME);
            continue;
        } 
        else //有感兴趣的网络事件发生
        {
            //处理各种网络事件
            pclsTcpCom->HandleListenIO(pstListenObj);
        }
    }

    // 将线程已推出事件设置为有效
    SetEvent(pstListenObj->hServerHasExit);

    return TRUE;
}
/***************************************************************************
* 函数名称：[InitialMutiCast]
* 摘 要：该函数初始化组播库
* 全局影响：
* 参数：void
* 返回值：成功卸载返回TRUE；否则，返回FALSE
*
* 修改记录：
* [日期]
* [作者/修改者]陈洞滨
* [修改原因]
***************************************************************************/
UINT  Thread_Tcp_Server_Proc(LPVOID lpvoid)
{
    //线程参数
    PST_TCP_SERVER_THD_OBJ pstTcpServerThdObj = (PST_TCP_SERVER_THD_OBJ)lpvoid;

    //取得当前线程函数所在类的对象
    CTCPCommunicatio *pclsTCPComObj = (CTCPCommunicatio*)(pstTcpServerThdObj->lpClassObj);
    PST_LISTEN_OBJ pstListenObj = (PST_LISTEN_OBJ)pstTcpServerThdObj->lpListenObj;

    while(TRUE)
    {
        //返回事件句柄数组中第一个授信的事件相对索引号
        int iEventIndex = WSAWaitForMultipleEvents(pstTcpServerThdObj->uiSocketNum+1,	//等待的网络事件句柄个数
                                                    pstTcpServerThdObj->hSelectEvents,	//等待的网络事件句柄数组
                                                    FALSE,								//不需要所有的事件均授信
                                                    WSA_INFINITE,						//无限等待
                                                    FALSE);								//该模型中，此处为固定值

        //取得授信事件的绝对索引号
        iEventIndex = iEventIndex - WSA_WAIT_EVENT_0;

        //逐个查看是哪个事件对象授信
        for (UINT i = iEventIndex; i < (pstTcpServerThdObj->uiSocketNum + 1); i++)
        {
            iEventIndex = WSAWaitForMultipleEvents(1,										//每次查看一个网络事件
                                                    &pstTcpServerThdObj->hSelectEvents[i],	//网络事件对应的事件对象句柄
                                                    TRUE,									//必须等待事件全部授信
                                                    SERVER_WAIT_TIME,						//等待的时间
                                                    FALSE);									//该模型中，此处为固定
            //函数调用失败或者等待超时
            if (iEventIndex == WSA_WAIT_FAILED || iEventIndex == WSA_WAIT_TIMEOUT) 
            {
               // Sleep(SERVER_WAIT_TIME);
                continue;
            } 
            else // 有感兴趣的网络事件发生
            {
                if (i == 0)   // 重构事件对象授信
                {
                    // 重构事件句柄数组，如果没有服务器I/O处理了，则退出线程
                    pclsTCPComObj->ReBuildServerEventArry(pstTcpServerThdObj);	

                    if (pstTcpServerThdObj->uiSocketNum == 0)
                    {
                        pclsTCPComObj->FreeServerThreadObj(pstListenObj, pstTcpServerThdObj);
                        return 0;
                    }

                    WSAResetEvent(pstTcpServerThdObj->hSelectEvents[0]);
                } 
                else //其它网络事件
                {
                    //查找网络事件对应的服务器端对象
                    PST_TCP_SERVER_OBJ pstTcpServerObj = NULL;
                    pclsTCPComObj->FindTcpServerObj(pstTcpServerObj, pstTcpServerThdObj, i);

                    //如果找到发生网络事件的对象，则对该事件进行处理
                    if (pstTcpServerObj != NULL)
                    {
                        if (!pclsTCPComObj->HandleServerIO(pstTcpServerThdObj,pstTcpServerObj))
                        {
							pclsTCPComObj->ReBuildServerEventArry(pstTcpServerThdObj);
                        }
                    }
                    else
                    {
                        CCsccLog::DebugPrint("当前线程对象中无法找到该套接字对象！\n");
                    }
                }
            }
        }
    }
    return 0;
}
/***************************************************************************
* 函数名称：[Thread_Tcp_Client_Proc]
* 摘 要：该函数初始化组播库
* 全局影响：
* 参数：void
* 返回值：成功卸载返回TRUE；否则，返回FALSE
*
* 修改记录：
* [日期]
* [作者/修改者]陈洞滨
* [修改原因]
***************************************************************************/
UINT  Thread_Tcp_Client_Proc(LPVOID lpvoid)
{
    // 线程参数
    PST_TCP_CLIENT_THD_OBJ pstTcpClientThdObj = (PST_TCP_CLIENT_THD_OBJ)lpvoid;

    // 取得当前线程函数所在类的对象
    CTCPCommunicatio *pclsTCPComObj = (CTCPCommunicatio*)(pstTcpClientThdObj->lpClassObj);

    while(TRUE)
    {
        // 返回事件句柄数组中第一个授信的事件相对索引号
        int iEventIndex = WSAWaitForMultipleEvents(pstTcpClientThdObj->uiSocketNum + 1,	// 等待的网络事件句柄个数
                                                    pstTcpClientThdObj->hSelectEvents,	// 等待的网络事件句柄数组
                                                    FALSE,								// 不需要所有的事件均授信
                                                    WSA_INFINITE,						// 无限等待
                                                    FALSE);								// 该模型中，此处为固定值

        // 取得授信事件的绝对索引号
        iEventIndex = iEventIndex - WSA_WAIT_EVENT_0;

        // 逐个查看是哪个事件对象授信
        for (UINT i = iEventIndex; i < (pstTcpClientThdObj->uiSocketNum + 1); i++)
        {
            iEventIndex = WSAWaitForMultipleEvents(1,										//每次查看一个网络事件
                                                   &pstTcpClientThdObj->hSelectEvents[i],	//网络事件对应的事件
                                                   TRUE,									//必须等待事件全部授信
                                                   SERVER_WAIT_TIME,						//等待的时间
                                                   FALSE);									//该模型中，此处为固定

            // 函数调用失败或者等待超时
            if (iEventIndex == WSA_WAIT_FAILED || iEventIndex == WSA_WAIT_TIMEOUT) 
            {
                Sleep(SERVER_WAIT_TIME);
                continue;
            } 
            else // 有感兴趣的网络事件发生
            {
                if (i == 0)   // 重构事件对象授信
                {
                    // 重构事件句柄数组，如果没有服务器I/O处理了，则退出线程
                    pclsTCPComObj->ReBuildClientEventArry(pstTcpClientThdObj);	

                    if (pstTcpClientThdObj->uiSocketNum == 0)
                    {
                        pclsTCPComObj->FreeClientThreadObj(pstTcpClientThdObj);

                        return 0;
                    }

                    WSAResetEvent(pstTcpClientThdObj->hSelectEvents[0]);
                } 
                else // 其它网络事件
                {
                    // 查找网络事件对应的服务器端对象
                    PST_TCP_CLIENT_OBJ pstTcpClientObj = NULL;

                    pclsTCPComObj->FindClientObj(pstTcpClientObj, pstTcpClientThdObj,i);

                    // 如果找到发生网络事件的对象，则对该事件进行处理
                    if (pstTcpClientObj != NULL)
                    {
                        if (!pclsTCPComObj->HandleClientIO(pstTcpClientThdObj,pstTcpClientObj))
                        {
                        }
                    }
                    else
                    {
                        CCsccLog::DebugPrint("当前线程对象中无法找到该套接字对象！\n");
                    }
                }
            }
        }
    }
    return 0;
}
/***************************************************************************
* 函数名称：[SetServerThreadObj]
* 摘 要：该函数为监听服务器创建一个管理与客户端通信套接字的线程
* 全局影响：
* 参数：PST_LISTEN_OBJ pstListenObj:[in] 监听套接字对象
* 返回值：PST_THD_OBJ
*
* 修改记录：
*[日期]
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/
INT CTCPCommunicatio::SetServerThreadObj(PST_LISTEN_OBJ pstListenObj, PST_TCP_SERVER_THD_OBJ &pstTcpServerThdObj)
{
    pstTcpServerThdObj = new ST_TCP_SERVER_THD_OBJ; //TODO：记得释放，已经在FreeThreadObj中释放
    if (pstTcpServerThdObj != NULL)
    {
        // 初始化访问服务器端对象链表关键代码段
        InitializeCriticalSection(&(pstTcpServerThdObj->crt_Tcp_Server));

        // 创建一个事件对象，用于标识该线程对应的事件句柄数组是否需要重建
        // 该事件默认无信号，手动复位，用于在删除服务器标识Event数组的重构
        pstTcpServerThdObj->hSelectEvents[0] = WSACreateEvent();

        // 线程退出事件
        //pstTcpServerThdObj->hThreadExit=CreateEvent(NULL,TRUE,FALSE,NULL);

        // 线程对象初始套接字个数
        pstTcpServerThdObj->uiSocketNum = 0;
        pstTcpServerThdObj->uiThreadIdx = pstListenObj->uiCurrentThdNum;

        // TCP通信类对象指针
        pstTcpServerThdObj->lpClassObj = this;
        pstTcpServerThdObj->lpListenObj = (LPVOID)pstListenObj;

        //启动线程
        pstTcpServerThdObj->pThread = AfxBeginThread(Thread_Tcp_Server_Proc,(LPVOID)pstTcpServerThdObj/*,THREAD_PRIORITY_NORMAL,0,CREATE_SUSPENDED*/);

        Sleep(100);

        // 线程对象链表中增加一个节点
        pstListenObj->lstTcpServerThdObj.AddTail(pstTcpServerThdObj);

        // 新增一个线程对象，则线程链表节点个数加一
        pstListenObj->uiCurrentThdNum++;

        return TRUE;
    }
    else
    {
        return FALSE;
    }

}

/***************************************************************************
* 函数名称：SetThreadObj
* 摘 要：新添加一个客户端通信套接字管理线程，
* 全局影响：
* 参数：void
* 返回值：PST_THD_OBJ 新增的线程对象结构体
*
* 修改记录：
*[日期]2011-12-08
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/
BOOL CTCPCommunicatio::SetClientThreadObj(PST_TCP_CLIENT_THD_OBJ &pstTcpClientThdObj)
{
    pstTcpClientThdObj = new ST_TCP_CLIENT_THD_OBJ; //TODO：记得释放，已经在FreeThreadObj中释放

    if (pstTcpClientThdObj != NULL)
    {
        // 初始化访问客户端对象链表关键代码段
        InitializeCriticalSection(&(pstTcpClientThdObj->crt_Tcp_Client));

        // 创建一个事件对象，用于标识该线程对应的事件句柄数组是否需要重建
        // 该事件默认无信号，手动复位，用于在删除服务器标识Event数组的重构
        pstTcpClientThdObj->hSelectEvents[0]=WSACreateEvent();

        // 线程对象初始套接字个数
        pstTcpClientThdObj->uiSocketNum = 0;

        // 线程对象的索引号
        pstTcpClientThdObj->uiThreadIdx = m_uiCurrentThdNum;

        // 线程对象所在的类
        pstTcpClientThdObj->lpClassObj = this;

        // 启动线程
        pstTcpClientThdObj->pThread = AfxBeginThread(Thread_Tcp_Client_Proc, (LPVOID)pstTcpClientThdObj);

        Sleep(100);

        m_TcpClientThdList.AddTail(pstTcpClientThdObj);
        m_uiCurrentThdNum++;                            //UNDONE:这里需不需要互斥？
    }

    return TRUE;
}
/***************************************************************************
* 函数名称：[InitialMutiCast]
* 摘 要：该函数初始化组播库
* 全局影响：
* 参数：void
* 返回值：成功卸载返回TRUE；否则，返回FALSE
*
* 修改记录：
* [日期]
* [作者/修改者]陈洞滨
* [修改原因]
***************************************************************************/
BOOL CTCPCommunicatio::FreeClientThreadObj(PST_TCP_CLIENT_THD_OBJ pstTcpClientThdObj)
{
    BOOL bretVal=FALSE;
    ST_LOG stTcpLog;
    stTcpLog.iType=LOG_INFO;

    // 清空线程管理的客户端对象
    EnterCriticalSection(&m_Crt_Thd_ClientObj);
    //m_clsCsccLog.MyNSprintf(stTcpLog.swzMsg,MAX_LOG_LENGTH,"FreeClientThreadObj EnterCriticalSection ~~~~~~~~~~~~~~~~~~m_Crt_Thd_ClientObj");
    //m_clsCsccLog.WriteLog(stTcpLog.iType,stTcpLog.swzMsg);

    for (POSITION posClientThdObj=m_TcpClientThdList.GetHeadPosition();
         posClientThdObj!=NULL;
         m_TcpClientThdList.GetNext(posClientThdObj))
    {
        if (m_TcpClientThdList.IsEmpty())
        {
            break;
        }
        PST_TCP_CLIENT_THD_OBJ pstTempClientThdObj=m_TcpClientThdList.GetAt(posClientThdObj);
        if (pstTempClientThdObj==pstTcpClientThdObj) // HACK:这个地方要怎么判断，是利用索引号，还是直接利用指针的地址？需要测试
        {
            // 如果客户端对象链表不为空，则将它清空
            EnterCriticalSection(&pstTempClientThdObj->crt_Tcp_Client);
            //m_clsCsccLog.MyNSprintf(stTcpLog.swzMsg,MAX_LOG_LENGTH,"FreeClientThreadObj EnterCriticalSection ~~~~~~~~~~~~~~~~~~pstTempClientThdObj->crt_Tcp_Client");
            //m_clsCsccLog.WriteLog(stTcpLog.iType,stTcpLog.swzMsg);
        
            for (POSITION posClientObj=pstTempClientThdObj->lstTcpClient.GetHeadPosition();
                 posClientObj!=NULL;
                 pstTempClientThdObj->lstTcpClient.GetNext(posClientObj))
            {
                if (pstTempClientThdObj->lstTcpClient.IsEmpty())
                {
                    break;
                }
                PST_TCP_CLIENT_OBJ pstTempTcpClientObj=pstTempClientThdObj->lstTcpClient.GetAt(posClientObj);
                RemoveTcpClientObj(pstTempClientThdObj,pstTempTcpClientObj);
                FreeClientObj(pstTempTcpClientObj);
            }
        
            LeaveCriticalSection(&pstTempClientThdObj->crt_Tcp_Client);
            //m_clsCsccLog.MyNSprintf(stTcpLog.swzMsg,MAX_LOG_LENGTH,"FreeClientThreadObj LeaveCriticalSection ~~~~~~~~~~~~~~~~~~pstTempClientThdObj->crt_Tcp_Client");
            //m_clsCsccLog.WriteLog(stTcpLog.iType,stTcpLog.swzMsg);

            // 将线程对象从链表中移除
            m_TcpClientThdList.RemoveAt(posClientThdObj);
            // 删除临界区
            DeleteCriticalSection(&pstTempClientThdObj->crt_Tcp_Client);
            // 关闭数组重构事件
            WSACloseEvent(pstTempClientThdObj->hSelectEvents[0]);

            //WaitForSingleObject(pstTempClientThdObj->pThread->m_hThread,INFINITE); // 关闭线程句柄
            //CloseHandle(pstTempClientThdObj->pThread->m_hThread);
            
            // 删除线程对象
            delete pstTempClientThdObj;
            pstTempClientThdObj=NULL;

            bretVal=TRUE;
        }
    }
    m_uiCurrentThdNum--;
    LeaveCriticalSection(&m_Crt_Thd_ClientObj);
    //m_clsCsccLog.MyNSprintf(stTcpLog.swzMsg,MAX_LOG_LENGTH,"FreeClientThreadObj LeaveCriticalSection ~~~~~~~~~~~~~~~~~~m_Crt_Thd_ClientObj");
    //m_clsCsccLog.WriteLog(stTcpLog.iType,stTcpLog.swzMsg);

    return bretVal;
}
/***************************************************************************
* 函数名称：[myFunction]
* 摘 要：简要描述本函数功能
* 全局影响：
* 参数：param1:[in/out] 参数作用描述
* 返回值：
*
* 修改记录：
*[日期]
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/
BOOL CTCPCommunicatio::FreeServerThreadObj(PST_LISTEN_OBJ pstListenObj,PST_TCP_SERVER_THD_OBJ pstTcpServerThdObj)
{
    ST_LOG stTcpLog;
    stTcpLog.iType=LOG_INFO;
    
    PST_TCP_SERVER_THD_OBJ pstThdListNode=NULL;
    for (POSITION posThdObj=pstListenObj->lstTcpServerThdObj.GetHeadPosition();  //取得链表头部节点，通常为最后建立的线程对象
        posThdObj!=NULL;                                                         //链表节点指针已经到达链表末尾
        pstListenObj->lstTcpServerThdObj.GetNext(posThdObj))                     //将链表节点指针向后移动
    {
        if (pstListenObj->lstTcpServerThdObj.IsEmpty())
        {
            break;
        }
        pstThdListNode=pstListenObj->lstTcpServerThdObj.GetAt(posThdObj);        //取出链表中的节点值
        
        
        if (pstThdListNode==pstTcpServerThdObj)        //在线程对象链表中找到与待删除线程对象一致的节点
        {
            EnterCriticalSection(&(pstThdListNode->crt_Tcp_Server));
            //m_clsCsccLog.MyNSprintf(stTcpLog.swzMsg,MAX_LOG_LENGTH,"FreeServerThreadObj EnterCriticalSection ------------------pstThdListNode->crt_Tcp_Server");
            //m_clsCsccLog.WriteLog(stTcpLog.iType,stTcpLog.swzMsg);

            for (POSITION posServerObj=pstThdListNode->lstTcpServer.GetHeadPosition();
                 posServerObj!=NULL;
                 pstThdListNode->lstTcpServer.GetNext(posServerObj))
            {
                if (pstThdListNode->lstTcpServer.IsEmpty())
                {
                    break;
                }
                PST_TCP_SERVER_OBJ pstTempServerObj=pstThdListNode->lstTcpServer.GetAt(posServerObj);
                RemoveTcpServerObj(pstTcpServerThdObj,pstTempServerObj);
                FreeTcpServerObj(pstTempServerObj);	

            }
            LeaveCriticalSection(&(pstThdListNode->crt_Tcp_Server));
    /*		m_clsCsccLog.MyNSprintf(stTcpLog.swzMsg,MAX_LOG_LENGTH,"FreeServerThreadObj LeaveCriticalSection ------------------pstThdListNode->crt_Tcp_Server");
            m_clsCsccLog.WriteLog(stTcpLog.iType,stTcpLog.swzMsg);*/

            pstListenObj->lstTcpServerThdObj.RemoveAt(posThdObj);                //将节点从链表中移除
            pstListenObj->uiCurrentThdNum--;
            WSACloseEvent(pstThdListNode->hSelectEvents[0]);                     //关闭重构线程对象数组事件
            DeleteCriticalSection(&pstThdListNode->crt_Tcp_Server);
            //WaitForSingleObject(pstThdListNode->pThread->m_hThread,INFINITE);
            //CloseHandle(pstThdListNode->pThread->m_hThread);
            delete pstThdListNode;                                               //释放节点占用的内存
            pstThdListNode=NULL;	
            break;
        }
        
    }

    return TRUE;
}
/***************************************************************************
* 函数名称：[myFunction]
* 摘 要：简要描述本函数功能
* 全局影响：
* 参数：param1:[in/out] 参数作用描述
* 返回值：
*
* 修改记录：
*[日期]
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/
INT CTCPCommunicatio::AddClientObj(SOCKET &sktConnectServer, BYTE abyServerIp[IP_BYTE_LENGTH], UINT uiServerPort,
                                   BYTE abyClientIp[IP_BYTE_LENGTH], UINT uiClientPort)
{
    USES_CONVERSION;

    // 服务器端IP
    CString strServerIp = _T("");
    strServerIp.Format(_T("%d.%d.%d.%d"), abyServerIp[0], abyServerIp[1], abyServerIp[2], abyServerIp[3]);
    char* pcServerIp = "";
    pcServerIp = W2A(strServerIp);

    // 客户端IP
    CString strClientIp = _T("");
    strClientIp.Format(_T("%d.%d.%d.%d"), abyClientIp[0], abyClientIp[1], abyClientIp[2], abyClientIp[3]);
    char* pcClientIp = "";
    pcClientIp = W2A(strClientIp);

    // 创建TCP通信套接字
    sktConnectServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
    if (INVALID_SOCKET == sktConnectServer)
    {
        return WSA_CREATE_SOCKET_ERROR;
    }

    int ireVal = 0;					// 函数返回值

    SOCKADDR_IN sdClientAddr;		// 客户端地址族
    sdClientAddr.sin_family = AF_INET;
    sdClientAddr.sin_addr.S_un.S_addr = inet_addr(pcClientIp);
    sdClientAddr.sin_port = htons(uiClientPort);

    SOCKADDR_IN sdServerAddr;									//服务器端地址簇
    sdServerAddr.sin_family = AF_INET;
    sdServerAddr.sin_addr.S_un.S_addr = inet_addr(pcServerIp);	//发送本机IP
    sdServerAddr.sin_port = htons(uiServerPort);				//端口号,主机字节序三转换为网络字节序

    int iServerLength = sizeof(SOCKADDR_IN);
    
    //设置发送端端口重用
    BOOL bIsReuse = TRUE;
    BOOL bDonntWait = FALSE;
    int iReusePortLen = sizeof(bIsReuse);
    ireVal = setsockopt(sktConnectServer, SOL_SOCKET, SO_REUSEADDR, (char*)&bIsReuse, iReusePortLen);
    if (SOCKET_ERROR == ireVal)
    {
        CCsccLog::DebugPrint("设置端口重用失败！\n");
        return FALSE;
    }

    // 设置套接字直接执行，不等待
    ireVal = setsockopt(sktConnectServer, SOL_SOCKET, SO_DONTLINGER, (char*)&bDonntWait, sizeof(bDonntWait));
    if (SOCKET_ERROR == ireVal)
    {
        CCsccLog::DebugPrint("设置无需等待失败！\n");
        return FALSE;
    }

    // 定义错误代码
    int iErrCode;

    // 待设置的套接字缓冲区大小
    UINT uiBuflen;

    // 缓冲区大小
    UINT uiOptlen = sizeof(uiBuflen);

    // 获取发送缓冲区参数
    iErrCode = getsockopt(sktConnectServer,			// 套接字
                            SOL_SOCKET,				// 套接字级别，此处选择套接字所在的应用层
                            SO_SNDBUF,				// 将要设置的套接字选项名称，此处为缓冲区大小
                            (char *)&uiBuflen,		// 设置的套接字选项值
                            (int*)&uiOptlen);		// 缓冲区大小

    // 判断获取缓冲区参数是否异常
    if (SOCKET_ERROR == iErrCode)                                              
    {
        CCsccLog::DebugPrint("获取发送缓冲区大小失败！\n");
        return INVALID_SOCKET;
    }

    // 将原先的缓冲区大小扩大十倍，系统默认为8192byte
    uiBuflen *= 100;

    // 设定缓冲区大小                                                           
    iErrCode = setsockopt(sktConnectServer, SOL_SOCKET, SO_RCVBUF, (char*)&uiBuflen, uiOptlen);

    // 判断设置缓冲区大小是否异常
    if (SOCKET_ERROR == iErrCode)                                              
    {
        CCsccLog::DebugPrint("设置发送缓冲区大小失败！\n");
        return INVALID_SOCKET;
    }

    //检查缓冲区大小是否修改成功
    unsigned int uiNewRcvBuf;                                               
    iErrCode = getsockopt(sktConnectServer, SOL_SOCKET, SO_SNDBUF, (char*)&uiNewRcvBuf, (int*)&uiOptlen);
    if (SOCKET_ERROR == iErrCode || uiNewRcvBuf == uiBuflen)
    {
        CCsccLog::DebugPrint("修改系统发送数据缓冲区失败！\n");
        return INVALID_SOCKET;
    }

    //绑定套接字
    int iBindErr = bind(sktConnectServer, (LPSOCKADDR)&sdClientAddr, sizeof(sdClientAddr));
    if (iBindErr == SOCKET_ERROR)
    {
        return FALSE;
    }
        
    // 连接服务器，由于WSAEventSelect模型是非阻塞的，该函数会立即返回。故应该放在WSAEventSelect函数调用之前，否则，会报SOCKET_ERROR
    ireVal = connect(sktConnectServer, (LPSOCKADDR)&sdServerAddr, iServerLength);

    if (SOCKET_ERROR == ireVal)
    {
        int iErrCode = WSAGetLastError();
        if (iErrCode == WSAEADDRINUSE)
        {
            return WSA_CONNECT_ERROR;
        }
    }

    //将TCP通信套接字加入到客户端对象中
    PST_TCP_CLIENT_OBJ pstTcpClientObj = NULL;
    if (!SetClientObj(pstTcpClientObj, sktConnectServer, abyClientIp, uiClientPort))
    {
        return FALSE;
    }

    return TRUE;
}
/***************************************************************************
* 函数名称：[SetServerObj]
* 摘 要：该函数创建一个通信对象，对于服务器端而言则是依据accept所创建
*        的套接字建立对象
* 全局影响：
* 参数：SOCKET ClinetSocket:[in] 通信套接字
*       SOCKADDR_IN sdServerObj:[in] 对于服务器而言，该参数表示与之连接的客
*                               户端地址族
*       PST_LISTEN_OBJ pstListenObj:[in] 服务器通信套接字对应的
* 返回值：PST_SERVER_OBJ 新建的通信对象
*
* 修改记录：
* [日期]
* [作者/修改者]陈洞滨
* [修改原因]
***************************************************************************/
BOOL CTCPCommunicatio::SetServerObj(PST_TCP_SERVER_OBJ &pstTcpServerObj, SOCKET sktAccept,
                                    BYTE abyClientIp[IP_BYTE_LENGTH], UINT uiClientPort, PST_LISTEN_OBJ pstListenObj)
{
    pstTcpServerObj = new ST_TCP_SERVER_OBJ;	//TODO:当用户取出数据之后，本库需将其释放掉

    // 服务器端对象指针不为空
    if (pstTcpServerObj != NULL)
    {
        // 初始化访问服务器链表临界区
        InitializeCriticalSection(&(pstTcpServerObj->crt_RcvPacket));

        // 创建服务器端通信套接字
        pstTcpServerObj->sktAccept = sktAccept;

        // 服务器端地址族
        memcpy(pstTcpServerObj->abyClientIp, abyClientIp, IP_BYTE_LENGTH);

        // 服务器端端口号
        pstTcpServerObj->uiClientPort = uiClientPort;

        // 创建与套接字相关的事件，初始无信号，手动复位
        pstTcpServerObj->hAcceptEvent = WSACreateEvent();

        // 为连接套接字注册网络事件
        WSAEventSelect(pstTcpServerObj->sktAccept, pstTcpServerObj->hAcceptEvent, FD_READ | FD_CLOSE | FD_WRITE);

        // 将与客服端通信的服务器对象添加到监听套接字对象的空闲线程中
        AssignToFreeServerThread(pstListenObj, pstTcpServerObj);

        return TRUE;
    }
    else
    {
        return FALSE;
    }

}
/***************************************************************************
* 函数名称：[SetClientObj]
* 摘 要：该函数依据AddClientObj函数创建的套接字建立客户端通信套接字对象。
* 全局影响：
* 参数：SOCKADDR_IN sdServerObj:[in]对于客户端而言，该参数表示与之所连接的服务器地址族
* 返回值：PST_SERVER_OBJ 新创建的客户端通信套接字对象
*
* 修改记录：
* [日期]
* [作者/修改者]陈洞滨
* [修改原因]
***************************************************************************/
BOOL  CTCPCommunicatio::SetClientObj(PST_TCP_CLIENT_OBJ &pstTcpClientObj, SOCKET sktClinet,
                                     BYTE abyClientIp[IP_BYTE_LENGTH], UINT uiClientPort)
{
    pstTcpClientObj = new ST_TCP_CLIENT_OBJ;  //TODO：记得释放,已经在FreeServerObj进行释放

    //服务器端对象指针不为空
    if (pstTcpClientObj != NULL)
    {
        // 初始化访问客户端链表临界区
        InitializeCriticalSection(&(pstTcpClientObj->crt_RcvPacket));

        // 创建客户端通信套接字
        pstTcpClientObj->sktClient = sktClinet;

        // 客户端地址族
        memcpy(pstTcpClientObj->abyClientIp, abyClientIp, IP_BYTE_LENGTH);

        // 客户端端口号
        pstTcpClientObj->uiClientPort = uiClientPort;

        // 创建与套接字相关的事件，初始无信号，手动复位
        pstTcpClientObj->hClientEvent = WSACreateEvent();

        // 为套接字注册网络事件
        WSAEventSelect(pstTcpClientObj->sktClient, pstTcpClientObj->hClientEvent, FD_WRITE | FD_READ | FD_CLOSE);

        // 将客户端对象加入到空闲的客服端线程对象中
        AssignToFreeClientThreadObj(pstTcpClientObj);
    }

    return TRUE;
}
/***************************************************************************
* 函数名称：[myFunction]
* 摘 要：简要描述本函数功能
* 全局影响：
* 参数：param1:[in/out] 参数作用描述
* 返回值：
*
* 修改记录：
* [日期]
* [作者/修改者]陈洞滨
* [修改原因]
***************************************************************************/
BOOL CTCPCommunicatio::FreeTcpServerObj(PST_TCP_SERVER_OBJ pstTcpServerObj)
{
    ST_LOG stTcpLog;
    stTcpLog.iType=LOG_INFO;

    // 客户端接收的数据链表是否为空，不为空的话则清空
    EnterCriticalSection(&pstTcpServerObj->crt_RcvPacket);
    //m_clsCsccLog.MyNSprintf(stTcpLog.swzMsg,MAX_LOG_LENGTH,"FreeTcpServerObj EnterCriticalSection----------pstTcpServerObj->crt_RcvPacket");
    //m_clsCsccLog.WriteLog(stTcpLog.iType,stTcpLog.swzMsg);

    for (POSITION posSegPacket=pstTcpServerObj->lstSegPacket.GetHeadPosition();
        posSegPacket!=NULL;
        pstTcpServerObj->lstSegPacket.GetNext(posSegPacket))
    {
        if (pstTcpServerObj->lstSegPacket.IsEmpty())
        {
            break;
        }
        PST_TCP_SEG_PACKET pstTcpSegPacket=pstTcpServerObj->lstSegPacket.GetAt(posSegPacket);
        pstTcpServerObj->lstSegPacket.RemoveAt(posSegPacket);
        delete[] pstTcpSegPacket->pbyRcvData;
        pstTcpSegPacket->pbyRcvData=NULL;
        delete pstTcpSegPacket;
        pstTcpSegPacket=NULL;  
    }

    LeaveCriticalSection(&pstTcpServerObj->crt_RcvPacket);
    //m_clsCsccLog.MyNSprintf(stTcpLog.swzMsg,MAX_LOG_LENGTH,"FreeTcpServerObj LeaveCriticalSection----------pstTcpServerObj->crt_RcvPacket");
    //m_clsCsccLog.WriteLog(stTcpLog.iType,stTcpLog.swzMsg);

    //关闭与服务器端套接字相关的事件
    if (pstTcpServerObj->hAcceptEvent)
    {
        WSACloseEvent(pstTcpServerObj->hAcceptEvent);
        //m_clsCsccLog.MyNSprintf(stTcpLog.swzMsg,MAX_LOG_LENGTH,"FreeTcpServerObj WSACloseEvent----------pstTcpServerObj->hAcceptEvent");
        //m_clsCsccLog.WriteLog(stTcpLog.iType,stTcpLog.swzMsg);
    }

    //关闭套接字
    if (pstTcpServerObj->sktAccept!=INVALID_SOCKET)
    {
        closesocket(pstTcpServerObj->sktAccept);
    }

    DeleteCriticalSection(&(pstTcpServerObj->crt_RcvPacket));
    //m_clsCsccLog.MyNSprintf(stTcpLog.swzMsg,MAX_LOG_LENGTH,"FreeTcpServerObj DeleteCriticalSection----------pstTcpServerObj->crt_RcvPacket");
    //m_clsCsccLog.WriteLog(stTcpLog.iType,stTcpLog.swzMsg);

    //删除服务器端对象
    delete pstTcpServerObj;

    return TRUE;

}

BOOL CTCPCommunicatio::FreeClientObj(PST_TCP_CLIENT_OBJ pstTcpClientObj)
{
    ST_LOG stTcpLog;
    stTcpLog.iType=LOG_INFO;

    // 客户端接收数据链表是否为空，如果不是，则将链表清空
    EnterCriticalSection(&pstTcpClientObj->crt_RcvPacket);
    //m_clsCsccLog.MyNSprintf(stTcpLog.swzMsg,MAX_LOG_LENGTH,"FreeClientObj EnterCriticalSection~~~~~~~~~~~~~~~~pstTcpClientObj->crt_RcvPacket");
    //m_clsCsccLog.WriteLog(stTcpLog.iType,stTcpLog.swzMsg);

    for (POSITION posSegPacket=pstTcpClientObj->lstSegPacket.GetHeadPosition();
         posSegPacket!=NULL;
         pstTcpClientObj->lstSegPacket.GetNext(posSegPacket))
    {
        if (pstTcpClientObj->lstSegPacket.IsEmpty())
        {
            break;
        }
        PST_TCP_SEG_PACKET pstTempPacket=pstTcpClientObj->lstSegPacket.GetAt(posSegPacket);
        pstTcpClientObj->lstSegPacket.RemoveAt(posSegPacket);
        delete[] pstTempPacket->pbyRcvData;
        pstTempPacket->pbyRcvData=NULL;
        delete pstTempPacket;
        pstTempPacket=NULL;
    }
    
    LeaveCriticalSection(&pstTcpClientObj->crt_RcvPacket);
    //m_clsCsccLog.MyNSprintf(stTcpLog.swzMsg,MAX_LOG_LENGTH,"FreeClientObj LeaveCriticalSection~~~~~~~~~~~~~~~~pstTcpClientObj->crt_RcvPacket");
    //m_clsCsccLog.WriteLog(stTcpLog.iType,stTcpLog.swzMsg);

    //关闭与服务器端套接字相关的事件
    if (pstTcpClientObj->hClientEvent)
    {
        WSACloseEvent(pstTcpClientObj->hClientEvent);
        //m_clsCsccLog.MyNSprintf(stTcpLog.swzMsg,MAX_LOG_LENGTH,"FreeClientObj WSACloseEvent~~~~~~~~~~~~~~~~pstTcpClientObj->hClientEvent");
        //m_clsCsccLog.WriteLog(stTcpLog.iType,stTcpLog.swzMsg);

    }

    //关闭套接字
    if (pstTcpClientObj->sktClient!=INVALID_SOCKET)
    {
        shutdown(pstTcpClientObj->sktClient,SD_BOTH);
        closesocket(pstTcpClientObj->sktClient);	
    }

    // 删除临界区
    DeleteCriticalSection(&(pstTcpClientObj->crt_RcvPacket));
    //m_clsCsccLog.MyNSprintf(stTcpLog.swzMsg,MAX_LOG_LENGTH,"FreeClientObj DeleteCriticalSection~~~~~~~~~~~~~~~~pstTcpClientObj->crt_RcvPacket");
    //m_clsCsccLog.WriteLog(stTcpLog.iType,stTcpLog.swzMsg);

    //删除服务器端对象
    delete pstTcpClientObj;

    return TRUE;

}
/***************************************************************************
* 函数名称：[ReBuildEventArry]
* 摘 要：该函数重构套接字管理线程中的事件句柄数组
* 全局影响：
* 参数：PST_THD_OBJ pstThreadObj:[in] 套接字管理线程对象
* 返回值：void
*
* 修改记录：
* [日期]
* [作者/修改者]陈洞滨
* [修改原因]
***************************************************************************/
BOOL CTCPCommunicatio::ReBuildServerEventArry(PST_TCP_SERVER_THD_OBJ pstTcpServerThdObj)
{
    // 取得服务器线程对象的访问权限
    EnterCriticalSection(&(pstTcpServerThdObj->crt_Tcp_Server));

    int iNum = 1;
    PST_TCP_SERVER_OBJ pstTcpServerObj = NULL;
    for (POSITION posServerObj = pstTcpServerThdObj->lstTcpServer.GetHeadPosition();// 线程对象中服务器端对象链表头
        posServerObj != NULL;														// 未到服务器端链表尾
        pstTcpServerThdObj->lstTcpServer.GetNext(posServerObj))						// 服务器端链表指针后移
    {
        // 取出每个服务器端对应的事件句柄
        pstTcpServerObj = pstTcpServerThdObj->lstTcpServer.GetAt(posServerObj);

        // 重构事件句柄数组
        pstTcpServerThdObj->hSelectEvents[iNum] = pstTcpServerObj->hAcceptEvent; 

        iNum++;
    }

    LeaveCriticalSection(&(pstTcpServerThdObj->crt_Tcp_Server));

    return TRUE;
}
/***************************************************************************
* 函数名称：[ReBuildClientEventArry]
* 摘 要：该函数初始化组播库
* 全局影响：
* 参数：void
* 返回值：成功卸载返回TRUE；否则，返回FALSE
*
* 修改记录：
* [日期]
* [作者/修改者]陈洞滨
* [修改原因]
***************************************************************************/
BOOL CTCPCommunicatio::ReBuildClientEventArry(PST_TCP_CLIENT_THD_OBJ pstTcpClientThdObj)
{
    EnterCriticalSection(&(pstTcpClientThdObj->crt_Tcp_Client));

    int iNum = 1;
    PST_TCP_CLIENT_OBJ pstTcpClientObj = NULL;
    for (POSITION posServerObj = pstTcpClientThdObj->lstTcpClient.GetHeadPosition();//线程对象中服务器端对象链表头
        posServerObj != NULL;														//未到服务器端链表尾
        pstTcpClientThdObj->lstTcpClient.GetNext(posServerObj))						//服务器端链表指针后移
    {
        if (pstTcpClientThdObj->lstTcpClient.IsEmpty())
        {
            break;
        }

        // 取出每个服务器端对应的事件句柄
        pstTcpClientObj = pstTcpClientThdObj->lstTcpClient.GetAt(posServerObj);

        // 重构事件句柄数组         
        pstTcpClientThdObj->hSelectEvents[iNum] = pstTcpClientObj->hClientEvent; 
        pstTcpClientObj->uiClientIdx=iNum;
        iNum++;
    }

    LeaveCriticalSection(&(pstTcpClientThdObj->crt_Tcp_Client));

    return TRUE;
}


/***************************************************************************
* 函数名称：[myFunction]
* 摘 要：简要描述本函数功能
* 全局影响：
* 参数：param1:[in/out] 参数作用描述
* 返回值：
*
* 修改记录：
*[日期]
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/
BOOL CTCPCommunicatio::RemoveTcpServerObj(PST_TCP_SERVER_THD_OBJ pstTcpThreadInfo,PST_TCP_SERVER_OBJ pstTcpServerObj)
{
    BOOL bRetVal=FALSE;
    ST_LOG stTcpLog;
    stTcpLog.iType=LOG_INFO;

    EnterCriticalSection(&(pstTcpThreadInfo->crt_Tcp_Server));
    //m_clsCsccLog.MyNSprintf(stTcpLog.swzMsg,MAX_LOG_LENGTH,"RemoveTcpServerObj EnterCriticalSection----------pstTcpServerThdObj->crt_Tcp_Server");
    //m_clsCsccLog.WriteLog(stTcpLog.iType,stTcpLog.swzMsg);

    PST_TCP_SERVER_OBJ pstTempServerObj=NULL;
    for (POSITION posServerObj=pstTcpThreadInfo->lstTcpServer.GetHeadPosition(); // 取得服务器端链表头节点
        posServerObj!=NULL;                                                      // 链表节点指针未到尾部
        pstTcpThreadInfo->lstTcpServer.GetNext(posServerObj))                    // 向后移动链表节点指针
    {
        if (pstTcpThreadInfo->lstTcpServer.IsEmpty())
        {
            break;
        }
        //取出服务器端链表节点
        pstTempServerObj=pstTcpThreadInfo->lstTcpServer.GetAt(posServerObj);

        //如果找到服务器端则将它从链表中移除
        if (pstTempServerObj->sktAccept==pstTcpServerObj->sktAccept)
        {
            pstTcpThreadInfo->lstTcpServer.RemoveAt(posServerObj);

            //线程对象管理的套接字个数减一
            pstTcpThreadInfo->uiSocketNum--;    // TODO:这里会不会导致连接套接字对象索引号出现重复？
            //指示服务器端对象更新事件句柄数组
            WSASetEvent(pstTcpThreadInfo->hSelectEvents[0]);
            //m_clsCsccLog.MyNSprintf(stTcpLog.swzMsg,MAX_LOG_LENGTH,"RemoveTcpServerObj WSASetEvent----------pstTcpThreadInfo->hSelectEvents[0]");
            //m_clsCsccLog.WriteLog(stTcpLog.iType,stTcpLog.swzMsg);
            bRetVal=TRUE;
            break;
        }
    }
    LeaveCriticalSection(&(pstTcpThreadInfo->crt_Tcp_Server));
    //m_clsCsccLog.MyNSprintf(stTcpLog.swzMsg,MAX_LOG_LENGTH,"RemoveTcpServerObj LeaveCriticalSection----------pstTcpServerThdObj->crt_Tcp_Server");
    //m_clsCsccLog.WriteLog(stTcpLog.iType,stTcpLog.swzMsg);

    return bRetVal;

}
/***************************************************************************
* 函数名称：[RemoveTcpClientObj]
* 摘 要：移除TCP客户端对象
* 全局影响：
* 参数：void
* 返回值：成功卸载返回TRUE；否则，返回FALSE
*
* 修改记录：
* [日期]
* [作者/修改者]陈洞滨
* [修改原因]
***************************************************************************/
BOOL CTCPCommunicatio::RemoveTcpClientObj(PST_TCP_CLIENT_THD_OBJ pstTcpClientThdObj, PST_TCP_CLIENT_OBJ pstTcpClientObj)
{
    BOOL bRetVal=FALSE;

    EnterCriticalSection(&(pstTcpClientThdObj->crt_Tcp_Client));

    PST_TCP_CLIENT_OBJ pstTempClientObj = NULL;
    for (POSITION posClientObj = pstTcpClientThdObj->lstTcpClient.GetHeadPosition();//取得服务器端链表头节点
        posClientObj != NULL;														//链表节点指针未到尾部
        pstTcpClientThdObj->lstTcpClient.GetNext(posClientObj))						//向后移动链表节点指针
    {
        if (pstTcpClientThdObj->lstTcpClient.IsEmpty())
        {
            break;
        }

        // 取出服务器端链表节点
        pstTempClientObj = pstTcpClientThdObj->lstTcpClient.GetAt(posClientObj);

        // 如果找到服务器端则将它从链表中移除
        if (pstTcpClientObj->sktClient == pstTempClientObj->sktClient)
        {
            // 移除节点
            pstTcpClientThdObj->lstTcpClient.RemoveAt(posClientObj);

            // 线程对象管理的套接字个数减一
            pstTcpClientThdObj->uiSocketNum--;

            // 指示服务器端对象更新事件句柄数组
            WSASetEvent(pstTcpClientThdObj->hSelectEvents[0]);
            bRetVal = TRUE;
            break;
        }
    }

    LeaveCriticalSection(&(pstTcpClientThdObj->crt_Tcp_Client));

    return bRetVal;

}
/***************************************************************************
* 函数名称：[myFunction]
* 摘 要：简要描述本函数功能
* 全局影响：
* 参数：param1:[in/out] 参数作用描述
* 返回值：
*
* 修改记录：
*[日期]
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/
BOOL CTCPCommunicatio::InsertTcpServerObj(PST_TCP_SERVER_THD_OBJ pstTcpServerThread, PST_TCP_SERVER_OBJ pstTcpServerObj)
{
    BOOL bRet = FALSE;			// 返回值，初始化为FALSE

    // 取得服务器线程对象的访问权限
    EnterCriticalSection(&pstTcpServerThread->crt_Tcp_Server);

    // 判断服务器端线程对象pstTcpServerThread当前管理的套接字数量是否小于64个
    if (pstTcpServerThread->uiSocketNum < WSA_MAXIMUM_WAIT_EVENTS-1)
    {
        // 线程对象管理的套接字个数加一
        pstTcpServerThread->uiSocketNum++;

        // 为服务器端对象分配ID索引号，该索引号从1开始
        pstTcpServerObj->uiServerIdx = pstTcpServerThread->uiSocketNum;

        // 新节点加入到线程对象链表中
        pstTcpServerThread->lstTcpServer.AddHead(pstTcpServerObj);

        // 服务器对应的事件对象设置为有信号状态
        WSASetEvent(pstTcpServerThread->hSelectEvents[0]);

        // 插入成功
        bRet = TRUE;
    }

    // 释放服务器线程对象的访问权限
    LeaveCriticalSection(&pstTcpServerThread->crt_Tcp_Server);

    //插入成功返回TRUE
    return bRet;
}
/***************************************************************************
* 函数名称：[InitialMutiCast]
* 摘 要：该函数初始化组播库
* 全局影响：
* 参数：void
* 返回值：成功卸载返回TRUE；否则，返回FALSE
*
* 修改记录：
* [日期]
* [作者/修改者]陈洞滨
* [修改原因]
***************************************************************************/
BOOL CTCPCommunicatio::InsertClientObj(PST_TCP_CLIENT_THD_OBJ pstTcpClientThdObj, PST_TCP_CLIENT_OBJ pstTcpClientObj)
{
    BOOL bRet = FALSE;				// 返回值，初始化为FALSE

    // 取得客户端线程对象的访问权限
    EnterCriticalSection(&(pstTcpClientThdObj->crt_Tcp_Client));

    // 判断该线程对象管理的套接字数量是否小于63
    if (pstTcpClientThdObj->uiSocketNum < WSA_MAXIMUM_WAIT_EVENTS - 1)
    {
        // 线程对象管理的套接字个数加一
        pstTcpClientThdObj->uiSocketNum++;

        // 为客户端对象分配ID索引号，该索引号从1开始
        pstTcpClientObj->uiClientIdx = pstTcpClientThdObj->uiSocketNum;

        // 新节点加入到线程对象链表中
        pstTcpClientThdObj->lstTcpClient.AddHead(pstTcpClientObj);

        // 插入成功
        bRet = TRUE;
    }

    // 释放客户端线程对象的访问权限
    LeaveCriticalSection(&(pstTcpClientThdObj->crt_Tcp_Client));

    // 更新线程对象链表
    WSASetEvent(pstTcpClientThdObj->hSelectEvents[0]);

    //插入成功返回TRUE
    return bRet;

}

/***************************************************************************
* 函数名称：[AssignToFreeThreadObj]
* 摘 要：将新创建的服务器端对象加入到一个空闲线程对象中
* 全局影响：
* 参数：PST_SERVER_OBJ pstServerObj:[in] 新创建的服务器端对象
* 返回值：void
*
* 修改记录：
*[日期]2011-12-08
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/
BOOL CTCPCommunicatio::AssignToFreeClientThreadObj(PST_TCP_CLIENT_OBJ pstTcpClientObj)
{
    // 取得客户端通信套接字管理线程对象链表的访问权限
    EnterCriticalSection(&(m_Crt_Thd_ClientObj));

    PST_TCP_CLIENT_THD_OBJ pstTcpClientThdObj = NULL;

    POSITION posThdObj = NULL;

    for (posThdObj = m_TcpClientThdList.GetHeadPosition();
        posThdObj != NULL;
        m_TcpClientThdList.GetNext(posThdObj))
    {	
        if (m_TcpClientThdList.IsEmpty()) //链表不为空时，在链表所有节点中查找空闲节点
        {
            break;
        }

        pstTcpClientThdObj = m_TcpClientThdList.GetAt(posThdObj);

        //试图在当前线程对象中插入新的服务器端节点
        if (InsertClientObj(pstTcpClientThdObj, pstTcpClientObj))
        {
            //插入成功则跳出
            break;
        }
    }

    //如果链表中每个节点均没有空闲或者链表本身为空，则创建新线程
    if (posThdObj == NULL || m_TcpClientThdList.IsEmpty())
    {
        if (!SetClientThreadObj(pstTcpClientThdObj))
        {
            return FALSE;
        }
        InsertClientObj(pstTcpClientThdObj,pstTcpClientObj);
    }

    LeaveCriticalSection(&(m_Crt_Thd_ClientObj));

    return TRUE;
}
/***************************************************************************
* 函数名称：[AssignToFreeServerThread]
* 摘 要：简要描述本函数功能
* 全局影响：
* 参数：PST_LISTEN_OBJ pstListenObj:[in/out] 参数作用描述
*       PST_TCP_SERVER_OBJ pstTcpServerObj:[in/out] 参数作用描述
* 返回值：
*
* 修改记录：
*[日期]
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/
BOOL CTCPCommunicatio::AssignToFreeServerThread(PST_LISTEN_OBJ pstListenObj, PST_TCP_SERVER_OBJ pstTcpServerObj)
{
    // 取得监听套接字对象的访问权限
    EnterCriticalSection(&(pstListenObj->crt_ThreadObj));

    PST_TCP_SERVER_THD_OBJ pstTcpServerThdObj = NULL;

    POSITION posThdObj = NULL;

    // 链表不为空时，在链表所有节点中查找空闲节点
    if (!pstListenObj->lstTcpServerThdObj.IsEmpty()) 
    {
        // 遍历链表中的所有节点
        for (posThdObj = pstListenObj->lstTcpServerThdObj.GetHeadPosition();
             posThdObj != NULL;
             pstListenObj->lstTcpServerThdObj.GetNext(posThdObj))
        {
            // 取出当前节点
            pstTcpServerThdObj = pstListenObj->lstTcpServerThdObj.GetAt(posThdObj);

            //试图在当前线程对象中插入新的服务器端节点
            if (InsertTcpServerObj(pstTcpServerThdObj, pstTcpServerObj))
            {
                //插入成功则跳出
                break;
            }
        }
    } 

    // 如果链表中每个节点均没有空闲或者链表本身为空，则创建新线程
    if (posThdObj == NULL || pstListenObj->lstTcpServerThdObj.IsEmpty())
    {
        // 创建新的服务端线程
        if (!SetServerThreadObj(pstListenObj, pstTcpServerThdObj))
        {
            return FALSE;
        }

        // 插入新的服务器端节点
        InsertTcpServerObj(pstTcpServerThdObj,pstTcpServerObj);
    }

    // 释放监听套接字对象的访问权限
    LeaveCriticalSection(&(pstListenObj->crt_ThreadObj));

    return TRUE;
}

/***************************************************************************
* 函数名称：[myFunction]
* 摘 要：简要描述本函数功能
* 全局影响：
* 参数：param1:[in/out] 参数作用描述
* 返回值：
*
* 修改记录：
*[日期]
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/
BOOL CTCPCommunicatio::FindTcpServerObj(PST_TCP_SERVER_OBJ &pstTcpServerObj, PST_TCP_SERVER_THD_OBJ pstTcpServerThdObj, int iEventIndex)
{
    BOOL bretVal = FALSE;
    
    //遍历链表，依据事件ID号查找对应的服务器端对象
    for (POSITION posTcpServer=pstTcpServerThdObj->lstTcpServer.GetHeadPosition();
        posTcpServer!=NULL;
        pstTcpServerThdObj->lstTcpServer.GetNext(posTcpServer))
    {
        //取得链表节点
        PST_TCP_SERVER_OBJ pstTempServerObj = pstTcpServerThdObj->lstTcpServer.GetAt(posTcpServer);

        //如果查找到对应的服务器端则将该服务器端返回
        if (pstTempServerObj->uiServerIdx == iEventIndex)
        {
            pstTcpServerObj = pstTempServerObj;
            bretVal=TRUE;
        }
    }

    return bretVal;
}

BOOL CTCPCommunicatio::FindClientObj(PST_TCP_CLIENT_OBJ &pstTcpClientObj,PST_TCP_CLIENT_THD_OBJ pstTcpClientThdObj,int iEventIndex)
{
    BOOL bRetVal=FALSE;
    PST_TCP_CLIENT_OBJ pstTempClientObj=NULL;

    //遍历链表，依据事件ID号查找对应的服务器端对象
    for (POSITION posClient=pstTcpClientThdObj->lstTcpClient.GetHeadPosition();
        posClient!=NULL;
        pstTcpClientThdObj->lstTcpClient.GetNext(posClient))
    {
        //取得链表节点
        pstTempClientObj=pstTcpClientThdObj->lstTcpClient.GetAt(posClient);

        //如果查找到对应的服务器端则将该服务器端返回
        if (pstTempClientObj->uiClientIdx==iEventIndex)
        {
            pstTcpClientObj=pstTempClientObj;
            bRetVal=TRUE;
        }
    }

    return bRetVal;
}

/***************************************************************************
* 函数名称：[BeginListen]
* 摘 要：添加服务器端
* 全局影响：
* 参数：abyServerIP：服务器的IP地址
        uiServerPort：服务器的端口号
* 返回值：true：添加成功；false：添加失败。
*
* 修改记录：
*[日期]
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/
BOOL  CTCPCommunicatio::BeginListen(BYTE abyServerIP[IP_BYTE_LENGTH], UINT uiServerPort)
{
    BOOL bRetVal = FALSE;						// 返回值，初始化为FALSE

    PST_LISTEN_OBJ pstListenObj = NULL;			// 监听套接字对象结构体指针

    int iErrorCode = 0;							// 错误代码

    // 创建监听套接字对象
    iErrorCode = SetListenObj(pstListenObj, abyServerIP, uiServerPort);

    // 判断是否成功
    if (iErrorCode == WSA_IS_MEMBER)
    {
        CCsccLog::DebugPrint("套接字已经处于监听状态！\n");
    }

    if (pstListenObj != NULL)
    {
        bRetVal = TRUE;
    }

    return bRetVal;

}
/***************************************************************************
* 函数名称：[TcpServerRcv]
* 摘 要：客户端或者服务器端接收数据
* 全局影响：
* 参数：param1:[in/out] 参数作用描述
* 返回值：
*
* 修改记录：
*[日期]
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/
BOOL CTCPCommunicatio::TcpServerRcv(PST_TCP_SERVER_OBJ pstTcpServerObj)
{
    int ireVal;	//返回值

    //取消网络事件
    WSAEventSelect(pstTcpServerObj->sktAccept, pstTcpServerObj->hAcceptEvent, 0);

    //接收数据
    ST_TCP_PACKET_HEAD stTcpHead;    //接收数据缓存
    ireVal = recv(pstTcpServerObj->sktAccept, (char*)&stTcpHead, sizeof(ST_TCP_PACKET_HEAD), 0);

    int	iDataLen = 0;				                // 已经读取字符数量
    if (0 == ireVal) //未接收到数据包
    {
        return 0;
    }
    else if(SOCKET_ERROR == ireVal)
    {
        int iErrCode = WSAGetLastError();
        if (WSAEWOULDBLOCK == iErrCode)
        {
            CCsccLog::DebugPrint("TCP接收数据包头出错！\n");
            return 0;
        }
        
    }
    else
    {
        int	iRcvLen = 0;				                // 每次接收的数据长度

        //接收数据缓存
        PST_TCP_SEG_PACKET pstTcpSegPacket = new ST_TCP_SEG_PACKET;			//TODO:应在取出数据后删除
        pstTcpSegPacket->pbyRcvData = new byte[stTcpHead.uiDataLength];		//TODO:应在取出数据后删除
		memset(pstTcpSegPacket->pbyRcvData,0,stTcpHead.uiDataLength);
        pstTcpSegPacket->uiDataLength = stTcpHead.uiDataLength;

        pstTcpSegPacket->uiTotalDataLength = stTcpHead.uiTotalDataLength;

        memcpy(pstTcpSegPacket->abySrcIp, pstTcpServerObj->abyClientIp, IP_BYTE_LENGTH);
        pstTcpSegPacket->uiSrcPort = pstTcpServerObj->uiClientPort;

        do
        {
            //接收指定长度的数据
            iRcvLen = recv(pstTcpServerObj->sktAccept,(char*)pstTcpSegPacket->pbyRcvData + iDataLen, stTcpHead.uiDataLength, 0);

            //接收出错
            if (iRcvLen==SOCKET_ERROR)
            {
				int iErrCode = WSAGetLastError();
				if (WSAEWOULDBLOCK==iErrCode)
				{
					WSAEventSelect(pstTcpServerObj->sktAccept,pstTcpServerObj->hAcceptEvent,FD_READ|FD_CLOSE);
					continue;
				}
		/*		switch(iErrCode)
				{
				case WSANOTINITIALISED:
					break;
				case WSAENETDOWN:
					break;
				case WSAEFAULT:
					break;
				case WSAENOTCONN:
					break;
				case WSAEINTR:
					break;
				case WSAEINPROGRESS:
					break;
				case WSAENETRESET:
					break;
				case WSAENOTSOCK:
					break;
				case WSAEOPNOTSUPP:
					break;
				case WSAESHUTDOWN:
					break;
				case WSAEWOULDBLOCK:
					break;
				case WSAEMSGSIZE:
					break;
				case WSAEINVAL:
					break;
				case WSAECONNABORTED:
					break;

				case WSAETIMEDOUT:
					break;
				case WSAECONNRESET:
					break;
				}*/
                CCsccLog::DebugPrint("TCP接收数据出错！\n");
                return 0;
            } 
            else if(0 == iRcvLen)
            {
                continue; //HACK:此处是否是这样子，需要在考虑
            }

            //累计本次已经接收到的数据长度
            iDataLen += iRcvLen;

        }while(stTcpHead.uiDataLength != iDataLen);

        EnterCriticalSection(&(pstTcpServerObj->crt_RcvPacket));

        pstTcpServerObj->lstSegPacket.AddTail(pstTcpSegPacket);

        LeaveCriticalSection(&(pstTcpServerObj->crt_RcvPacket));

        //数据接收完成，重新注册网络事件
        WSAEventSelect(pstTcpServerObj->sktAccept, pstTcpServerObj->hAcceptEvent, FD_READ|FD_CLOSE);
    }

    return iDataLen;
}
/***************************************************************************
* 函数名称：[InitialMutiCast]
* 摘 要：该函数初始化组播库
* 全局影响：
* 参数：void
* 返回值：成功卸载返回TRUE；否则，返回FALSE
*
* 修改记录：
* [日期]
* [作者/修改者]陈洞滨
* [修改原因]
***************************************************************************/
INT CTCPCommunicatio::TcpClientRcv(PST_TCP_CLIENT_OBJ pstTcpClientObj)
{
    ST_LOG stTcpLog;
    stTcpLog.iType=LOG_INFO;

    int ireVal;	//返回值
    WSAEventSelect(pstTcpClientObj->sktClient, pstTcpClientObj->hClientEvent, 0);	//取消网络事件

    //接收数据
    ST_TCP_PACKET_HEAD stTcpHead;    //接收数据缓存
    ireVal=recv(pstTcpClientObj->sktClient,(char*)&stTcpHead,sizeof(ST_TCP_PACKET_HEAD),0);

    int	iDataLen = 0;				                // 已经读取字符数量
    if ( 0 == ireVal) //未接收到数据包
    {
        return FALSE;
    }
    else if(SOCKET_ERROR==ireVal)
    {
        int nErrCode = WSAGetLastError();
        if (WSAEWOULDBLOCK == nErrCode)
        {
            CCsccLog::DebugPrint("TCP接收数据包头出错！\n");
            return 0;
        }
        
    }else
    {
        int	iRcvLen = 0;				                // 每次接收的数据长度

        //接收数据缓存
        PST_TCP_SEG_PACKET pstTcpSegPacket=new ST_TCP_SEG_PACKET;   //TODO:应在组包之后删除
        pstTcpSegPacket->pbyRcvData=new byte[stTcpHead.uiDataLength];  //TODO:应在组包之后删除
        pstTcpSegPacket->uiDataLength=stTcpHead.uiDataLength;
        pstTcpSegPacket->uiTotalDataLength=stTcpHead.uiTotalDataLength;

        do
        {
            //接收指定长度的数据
            iRcvLen=recv(pstTcpClientObj->sktClient,(char*)pstTcpSegPacket->pbyRcvData+iDataLen,stTcpHead.uiDataLength,0);
            //接收出错
            if (iRcvLen==SOCKET_ERROR)
            {
				int iErrCode = WSAGetLastError();
				if (WSAEWOULDBLOCK==iErrCode)
				{
					WSAEventSelect(pstTcpClientObj->sktClient,pstTcpClientObj->hClientEvent,FD_READ|FD_CLOSE);
					continue;
				}
            } 
            else if(0==iRcvLen)
            {
                continue; //HACK:此处是否是这样子，需要在考虑
            }

            //累计本次已经接收到的数据长度
            iDataLen+=iRcvLen;

        }while(stTcpHead.uiDataLength != iDataLen);

        EnterCriticalSection(&(pstTcpClientObj->crt_RcvPacket));
        /*m_clsCsccLog.MyNSprintf(stTcpLog.swzMsg,MAX_LOG_LENGTH,"TcpClientRcv  EnterCriticalSection~~~~~~~~~~~~pstTcpClientObj->crt_RcvPacket");
        m_clsCsccLog.WriteLog(stTcpLog.iType,stTcpLog.swzMsg);*/

        pstTcpClientObj->lstSegPacket.AddTail(pstTcpSegPacket);
        LeaveCriticalSection(&(pstTcpClientObj->crt_RcvPacket));
    /*	m_clsCsccLog.MyNSprintf(stTcpLog.swzMsg,MAX_LOG_LENGTH,"TcpClientRcv  LeaveCriticalSection~~~~~~~~~~~~pstTcpClientObj->crt_RcvPacket");
        m_clsCsccLog.WriteLog(stTcpLog.iType,stTcpLog.swzMsg);*/

        //数据接收完成，重新注册网络事件
        WSAEventSelect(pstTcpClientObj->sktClient, pstTcpClientObj->hClientEvent, FD_READ|FD_CLOSE);
    }

    return TRUE;
}
/***************************************************************************
* 函数名称：[myFunction]
* 摘 要：简要描述本函数功能
* 全局影响：
* 参数：param1:[in/out] 参数作用描述
* 返回值：
*
* 修改记录：
*[日期]
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/
BOOL CTCPCommunicatio::HandleServerIO(PST_TCP_SERVER_THD_OBJ pstTcpServerThdObj,PST_TCP_SERVER_OBJ pstTcpServerObj)
{ 
    WSANETWORKEVENTS  evntsNetWork; //网络事件结构
    if (WSAEnumNetworkEvents(pstTcpServerObj->sktAccept,pstTcpServerObj->hAcceptEvent,&evntsNetWork)==SOCKET_ERROR)
    {
        CCsccLog::DebugPrint("获取具体网络事件错误！\n");
    }

    do 
    {
        if (evntsNetWork.lNetworkEvents&FD_READ)      //套接字有数据可读
        {
            if (evntsNetWork.iErrorCode[FD_READ_BIT]!=0)
            {
                CCsccLog::DebugPrint("套接字可读发生网络错误！\n");
                break;
            } 
            else
            {
                TcpServerRcv(pstTcpServerObj);
            }
        }else if (evntsNetWork.lNetworkEvents&FD_CLOSE)  //客户端断开连接
        {
            if (evntsNetWork.iErrorCode[FD_CLOSE_BIT]!=0)
            {
                CCsccLog::DebugPrint("套接字可关闭发生网络错误！\n");
                break;
            }
            else
            {		
                break;
            }
        }
        return TRUE;

    } while (FALSE);

    //客户端关闭，则清除与之通信的套接字
    RemoveTcpServerObj(pstTcpServerThdObj,pstTcpServerObj);
    FreeTcpServerObj(pstTcpServerObj);

    return FALSE;

}

BOOL CTCPCommunicatio::HandleClientIO(PST_TCP_CLIENT_THD_OBJ pstTcpClientThdObj,PST_TCP_CLIENT_OBJ pstTcpClientObj)
{ 
    WSANETWORKEVENTS  evntsNetWork; //网络事件结构
    if (WSAEnumNetworkEvents(pstTcpClientObj->sktClient,pstTcpClientObj->hClientEvent,&evntsNetWork)==SOCKET_ERROR)
    {
        CCsccLog::DebugPrint("获取具体网络事件错误！\n");
    }

    do 
    {
        if (evntsNetWork.lNetworkEvents&FD_READ)      //套接字有数据可读
        {
            if (evntsNetWork.iErrorCode[FD_READ_BIT]!=0)
            {
                CCsccLog::DebugPrint("套接字可读发生网络错误！\n");
                break;
            } 
            else
            {

                TcpClientRcv(pstTcpClientObj);
            }
        }else if (evntsNetWork.lNetworkEvents&FD_CLOSE)  //客户端断开连接
        {
            if (evntsNetWork.iErrorCode[FD_CLOSE_BIT]!=0)
            {
                CCsccLog::DebugPrint("套接字可关闭发生网络错误！\n");
                break;
            }
            else
            {
                break;
            }
        }
        return TRUE;

    } while (FALSE);

    //客户端关闭，则清除与之通信的套接字
    RemoveTcpClientObj(pstTcpClientThdObj,pstTcpClientObj);
    FreeClientObj(pstTcpClientObj);

    return FALSE;

}


/***************************************************************************
* 函数名称：[myFunction]
* 摘 要：简要描述本函数功能
* 全局影响：
* 参数：param1:[in/out] 参数作用描述
* 返回值：
*
* 修改记录：
*[日期]
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/
BOOL CTCPCommunicatio::GetPacketFromServer(BYTE abyClientIP[IP_BYTE_LENGTH], UINT uiClientPort, ST_TCP_SEG_PACKET &stTcpSegPacket)
{
    USES_CONVERSION;
    BOOL bRetVal=FALSE;

    // 与服务器连接的客户端IP地址
    CString strClientIp = _T("");
    strClientIp.Format(_T("%d.%d.%d.%d"), abyClientIP[0], abyClientIP[1], abyClientIP[2], abyClientIP[3]);
    char* pcClientIP = "";
    pcClientIP = W2A(strClientIp);

    //如果没有客户端则直接返回
    if (m_lstListenObj.IsEmpty())
    {
        bRetVal = FALSE;
        return bRetVal;
    }

    // 获取监听套接字对象链表的访问权限
    EnterCriticalSection(&(m_Crt_ListenObj));

    //遍历线程对象链表，查找服务端对象所在的线程对象
    for (POSITION posListenObj = m_lstListenObj.GetHeadPosition();
        posListenObj != NULL;
        m_lstListenObj.GetNext(posListenObj))
    {
        if (m_lstListenObj.IsEmpty())
        {
            break;
        }

        // 取出一个管理客户端线程对象
        PST_LISTEN_OBJ pstListenObj = m_lstListenObj.GetAt(posListenObj);

        // 获取线程对象链表的访问权限
        EnterCriticalSection(&(pstListenObj->crt_ThreadObj));

        // 遍历服务端线程对象链表
        for (POSITION posThdObj = pstListenObj->lstTcpServerThdObj.GetHeadPosition();	// 获取TCP服务端线程链表的头部
            posThdObj != NULL;
            pstListenObj->lstTcpServerThdObj.GetNext(posThdObj))
        {
            if (pstListenObj->lstTcpServerThdObj.IsEmpty())
            {
                break;
            }

            // 得到posThdObj位置的服务端线程链表中的线程对象
            PST_TCP_SERVER_THD_OBJ pstTcpServerThdObj = pstListenObj->lstTcpServerThdObj.GetAt(posThdObj);

            // 取得访问与客户端通信套接字链表权限
            EnterCriticalSection(&(pstTcpServerThdObj->crt_Tcp_Server));

            // 遍历服务端对象链表，查找指定的服务器节点
            for (POSITION posServer = pstTcpServerThdObj->lstTcpServer.GetHeadPosition();
                posServer != NULL;
                pstTcpServerThdObj->lstTcpServer.GetNext(posServer))
            {
                if (pstTcpServerThdObj->lstTcpServer.IsEmpty())
                {
                    break;
                }

                // 得到posServer位置的服务端链表中的服务端对象
                PST_TCP_SERVER_OBJ pstTcpServerObj = pstTcpServerThdObj->lstTcpServer.GetAt(posServer);

                CString strTempClientIp = _T("");
                strTempClientIp.Format(_T("%d.%d.%d.%d"), pstTcpServerObj->abyClientIp[0], pstTcpServerObj->abyClientIp[1],
                                        pstTcpServerObj->abyClientIp[2], pstTcpServerObj->abyClientIp[3]);

                // 比对链表中每一个节点的IP和Port值是否和目标客户端IP、Port值一致，如果一致则将该客户端从链表中取出
                //if ((strTempClientIp == strClientIp) && (pstTcpServerObj->uiClientPort == uiClientPort))
                //{
                    // 取得访问客户端接收数据链表权限
                    EnterCriticalSection(&(pstTcpServerObj->crt_RcvPacket));

                    // 从客户端对象中取得接收数据
                    if (!(pstTcpServerObj->lstSegPacket.IsEmpty()))
                    {
                        // 接收的TCP数据包，该包可能会是不完整的，需要用户自己组包
                        PST_TCP_SEG_PACKET pstTcpSegPacket = pstTcpServerObj->lstSegPacket.GetTail();

                        // 接收到的数据包信息
                        memcpy(stTcpSegPacket.pbyRcvData, pstTcpSegPacket->pbyRcvData, pstTcpSegPacket->uiTotalDataLength);	// 数据包
                        memcpy(stTcpSegPacket.abySrcIp, pstTcpSegPacket->abySrcIp, IP_BYTE_LENGTH);							// 数据源IP地址
                        stTcpSegPacket.uiSrcPort = pstTcpSegPacket->uiSrcPort;												// 数据源端口号
                        stTcpSegPacket.uiDataLength = pstTcpSegPacket->uiDataLength;										// 本次数据包的长度
                        stTcpSegPacket.uiTotalDataLength = pstTcpSegPacket->uiTotalDataLength;								// 本数据包对应数据的总长度（本数据包中数据可能不完整，仅是整个数据的一部分）
                        pstTcpServerObj->lstSegPacket.RemoveTail();															// 取出数据后将节点从链表中移除
                        
                        delete[] pstTcpSegPacket->pbyRcvData;
                        pstTcpSegPacket->pbyRcvData = NULL;
                        delete pstTcpSegPacket;
                        pstTcpSegPacket = NULL;
                        bRetVal = TRUE;
                    }

                    // 释放访问客户端接收数据链表权限
                    LeaveCriticalSection(&(pstTcpServerObj->crt_RcvPacket));
                //}
            }

            // 释放访问与客户端通信套接字链表权限
            LeaveCriticalSection(&(pstTcpServerThdObj->crt_Tcp_Server));
        }

        // 释放线程对象链表的访问权限
        LeaveCriticalSection(&(pstListenObj->crt_ThreadObj));
    }

    // 释放监听套接字对象链表的访问权限
    LeaveCriticalSection(&(m_Crt_ListenObj));

    return bRetVal;
}
/***************************************************************************
* 函数名称：[EndListen]
* 摘 要：简要描述本函数功能
* 全局影响：
* 参数：param1:[in/out] 参数作用描述
* 返回值：
*
* 修改记录：
*[日期]
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/
BOOL CTCPCommunicatio::EndListen(BYTE abyServerIP[IP_BYTE_LENGTH], UINT uiServerPort)
{
    // 服务器IP地址
    CString strServerIp = _T("");
    strServerIp.Format(_T("%d.%d.%d.%d"), abyServerIP[0], abyServerIP[1], abyServerIP[2], abyServerIP[3]);

    EnterCriticalSection(&(m_Crt_ListenObj));

    for (POSITION posListenObj = m_lstListenObj.GetHeadPosition();
        posListenObj != NULL;
        m_lstListenObj.GetNext(posListenObj))
    {
        if (m_lstListenObj.IsEmpty())
        {
            break;
        }

        // 清除监听套接字链表
        PST_LISTEN_OBJ pstListenObj = m_lstListenObj.GetAt(posListenObj);

        // 服务器IP地址
        CString strTempServerIp = _T("");
        strTempServerIp.Format(_T("%d.%d.%d.%d"), pstListenObj->abyServerIp[0], pstListenObj->abyServerIp[1],
                                pstListenObj->abyServerIp[2], pstListenObj->abyServerIp[3]);
        if (strServerIp == strTempServerIp && uiServerPort == pstListenObj->uiServerPort)
        {
            FreeListenObj(pstListenObj);
            m_lstListenObj.RemoveAt(posListenObj);
        }
    
    }
    LeaveCriticalSection(&(m_Crt_ListenObj));

    return TRUE;
}
/***************************************************************************
* 函数名称：[myFunction]
* 摘 要：简要描述本函数功能
* 全局影响：
* 参数：param1:[in/out] 参数作用描述
* 返回值：
*
* 修改记录：
*[日期]
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/
SOCKET CTCPCommunicatio::ConnetServer(BYTE abyServerIp[IP_BYTE_LENGTH], unsigned int uiServerPort,
                                      BYTE abyClientIp[IP_BYTE_LENGTH], UINT uiClientPort)
{
    SOCKET sktClient = INVALID_SOCKET;
    AddClientObj(sktClient, abyServerIp, uiServerPort, abyClientIp, uiClientPort);
    return sktClient;
}

/***************************************************************************
* 函数名称：[TCPSend]
* 摘 要：简要描述本函数功能
* 全局影响：
* 参数：param1:[in/out] 参数作用描述
* 返回值：
*
* 修改记录：
*[日期]
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/
BOOL CTCPCommunicatio::TCPSend(SOCKET skClient,BYTE *pbyBuffer,UINT uiDataLength)
{
    int  iSendError;										//错误码
    UINT uiSendTimes = uiDataLength / DATA_BUF_LENGTH + 1;	//所有数据发送完所需要的次数
    UINT uiSendLength = 0;									//每次发送数据的长度
    UINT uiSendIdx = 0;										//当前发送的次数
    UINT uiTotalDataLen = uiDataLength;						//待发送数据的总长度

    while (TRUE)
    {
        uiSendIdx++;

        //计算每次发送数据的长度
        if (uiSendIdx != uiSendTimes)
        {
            uiSendLength = DATA_BUF_LENGTH;  
        } 
        else  
        {
            uiSendLength = uiDataLength - (uiSendIdx - 1) * DATA_BUF_LENGTH;//最后一次发送的数据量
        }

        ST_TCP_PACKET_HEAD stTcpHead;				//TCP数据包包头
        stTcpHead.uiDataLength = uiSendLength;		// 本次发送数据长度
        stTcpHead.uiTotalDataLength = uiDataLength;	// 待发送数据的总长度

        //发送数据包包头，其中指明了这次将要发送的数据长度
        iSendError = send(skClient, (char*)&stTcpHead, sizeof(ST_TCP_PACKET_HEAD), 0);
        if (iSendError == SOCKET_ERROR)
        {
            return FALSE;
        }

        //发送纯数据
        iSendError = send(skClient, (char*)(pbyBuffer + DATA_BUF_LENGTH * (uiSendIdx - 1)), uiSendLength, 0);
        if (iSendError == SOCKET_ERROR)
        {
            CCsccLog::DebugPrint("发送数据包包体出错!\n");
            return FALSE;
        }

        //剩余待发送数据量
        uiTotalDataLen -= uiSendLength;

        //数据全部发送完成，退出循环
        if (0 == -uiTotalDataLen)
        {
            break;
        }
    }
    return TRUE;
}
/***************************************************************************
* 函数名称：[myFunction]
* 摘 要：简要描述本函数功能
* 全局影响：
* 参数：param1:[in/out] 参数作用描述
* 返回值：
*
* 修改记录：
*[日期]
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/
BOOL CTCPCommunicatio::GetPacketFromClient(BYTE abyClientIP[IP_BYTE_LENGTH],UINT uiClientPort,ST_TCP_SEG_PACKET &stTcpSegPacket)
{

    USES_CONVERSION;

    ST_LOG stTcpLog;
    stTcpLog.iType=LOG_INFO;
    BOOL bRetVal=FALSE;

    CString strClientIp=_T("");
    strClientIp.Format(_T("%d.%d.%d.%d"),abyClientIP[0],abyClientIP[1],abyClientIP[2],abyClientIP[3]);


    //取得访问客户端链表权限
    EnterCriticalSection(&m_Crt_Thd_ClientObj);

    //如果没有客户端则直接返回
    if (m_TcpClientThdList.IsEmpty())
    {
        return FALSE;
    }

    //遍历线程对象链表，查找服务端对象所在的线程对象
    for (POSITION posThread=m_TcpClientThdList.GetHeadPosition();posThread!=NULL;)
    {
        //取出一个管理客户端线程对象
        PST_TCP_CLIENT_THD_OBJ pstClientThdObj=m_TcpClientThdList.GetNext(posThread);

        //取得访问客户端链表权限
        EnterCriticalSection(&(pstClientThdObj->crt_Tcp_Client));
        //遍历服务端对象链表，查找指定的服务器节点
        for (POSITION posServer=pstClientThdObj->lstTcpClient.GetHeadPosition();posServer!=NULL;)
        {
            PST_TCP_CLIENT_OBJ pstClientObj=pstClientThdObj->lstTcpClient.GetNext(posServer);

            CString strTempClientIp=_T("");
            strTempClientIp.Format(_T("%d.%d.%d.%d"),pstClientObj->abyClientIp[0],pstClientObj->abyClientIp[1],pstClientObj->abyClientIp[2],pstClientObj->abyClientIp[3]);

            //比对链表中每一个节点的IP和Port值是否和目标客户端IP、Port值一致，如果一致则将该客户端从链表中取出
            if ((strTempClientIp==strClientIp)&&(pstClientObj->uiClientPort==uiClientPort))
            {
                //取得访问客户端接收数据链表权限
                EnterCriticalSection(&(pstClientObj->crt_RcvPacket));
                m_clsCsccLog.MyNSprintf(stTcpLog.swzMsg,MAX_LOG_LENGTH,"GetPacketFromClient EnterCriticalSection~~~~~~~~~~~~~~pstClientObj->crt_RcvPacket");
                m_clsCsccLog.WriteLog(stTcpLog.iType,stTcpLog.swzMsg);

                //从客户端对象中取得接收数据
                if (!(pstClientObj->lstSegPacket.IsEmpty()))
                {
                    //接收的TCP数据包，该包可能会是不完整的，需要用户自己组包
                    PST_TCP_SEG_PACKET pstTcpSegPacket=pstClientObj->lstSegPacket.GetTail();

                    //接收到的数据包信息
                    memcpy(stTcpSegPacket.pbyRcvData,pstTcpSegPacket->pbyRcvData,pstTcpSegPacket->uiTotalDataLength); //数据包
                    memcpy(stTcpSegPacket.abySrcIp,pstTcpSegPacket->abySrcIp,IP_BYTE_LENGTH);     //数据源IP地址
                    stTcpSegPacket.uiSrcPort=pstTcpSegPacket->uiSrcPort; //数据源端口号
                    stTcpSegPacket.uiTotalDataLength=pstTcpSegPacket->uiTotalDataLength; //本数据包对应数据的总长度（本数据包中数据可能不完整，仅是整个数据的一部分）
                    pstClientObj->lstSegPacket.RemoveTail(); //取出数据后将节点从链表中移除
                    delete[] pstTcpSegPacket->pbyRcvData;
                    pstTcpSegPacket->pbyRcvData=NULL;
                    delete pstTcpSegPacket;   
                    pstTcpSegPacket=NULL;
                    
                    bRetVal=TRUE;
                }
            
                LeaveCriticalSection(&(pstClientObj->crt_RcvPacket));
                m_clsCsccLog.MyNSprintf(stTcpLog.swzMsg,MAX_LOG_LENGTH,"GetPacketFromClient LeaveCriticalSection~~~~~~~~~~~~~~pstClientObj->crt_RcvPacket");
                m_clsCsccLog.WriteLog(stTcpLog.iType,stTcpLog.swzMsg);
            }
        }
        LeaveCriticalSection(&(pstClientThdObj->crt_Tcp_Client));
    }

    LeaveCriticalSection(&m_Crt_Thd_ClientObj);

    return bRetVal;
}
/***************************************************************************
* 函数名称：[myFunction]
* 摘 要：简要描述本函数功能
* 全局影响：
* 参数：param1:[in/out] 参数作用描述
* 返回值：
*
* 修改记录：
*[日期]
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/
BOOL CTCPCommunicatio::DisConnetServer(BYTE abyClientIp[IP_BYTE_LENGTH],unsigned int uiClientPort)
{

    USES_CONVERSION;

    // 客户端IP
    CString strClientIp = _T("");
    strClientIp.Format(_T("%d.%d.%d.%d"), abyClientIp[0], abyClientIp[1], abyClientIp[2], abyClientIp[3]);

    // 获得客户端通信套接字管理线程对象链表的访问权限
    EnterCriticalSection(&(m_Crt_Thd_ClientObj));

    // 遍历链表中的所有对象，查找当前需要释放的服务器端对象所在的线程
    for (POSITION posThdClientObj = m_TcpClientThdList.GetHeadPosition();
        posThdClientObj != NULL;
        m_TcpClientThdList.GetNext(posThdClientObj))
    {
        if (m_TcpClientThdList.IsEmpty())
        {
            break;
        }

        // 得到posThdClientObj位置的套接字管理线程对象
        PST_TCP_CLIENT_THD_OBJ pstTempClientThdObj = m_TcpClientThdList.GetAt(posThdClientObj);

        // 遍历服务器端对象链表中的所有对象，查找当前需要释放的服务器端对象
        for (POSITION posServerObj = pstTempClientThdObj->lstTcpClient.GetHeadPosition();
            posServerObj != NULL;
            pstTempClientThdObj->lstTcpClient.GetNext(posServerObj))
        {
            if (pstTempClientThdObj->lstTcpClient.IsEmpty())
            {
                break;
            }

            // 得到posServerObj位置的服务器端对象
            PST_TCP_CLIENT_OBJ pstClientObj = pstTempClientThdObj->lstTcpClient.GetAt(posServerObj);

            CString strTempClientIp = _T("");
            strTempClientIp.Format(_T("%d.%d.%d.%d"), pstClientObj->abyClientIp[0], pstClientObj->abyClientIp[1],
                                    pstClientObj->abyClientIp[2], pstClientObj->abyClientIp[3]);

            // 根据端口号和IP判断是不是当前需要释放的服务器端对象
            if ((uiClientPort == uiClientPort) && (strTempClientIp == strClientIp))
            {
                RemoveTcpClientObj(pstTempClientThdObj,pstClientObj);
                FreeClientObj(pstClientObj);
            }
        }
    }

    // 释放客户端通信套接字管理线程对象链表的访问权限
    LeaveCriticalSection(&(m_Crt_Thd_ClientObj));

    return TRUE;
}

/***************************************************************************
* 函数名称：[myFunction]
* 摘 要：简要描述本函数功能
* 全局影响：
* 参数：param1:[in/out] 参数作用描述
* 返回值：
*
* 修改记录：
* [日期]
* [作者/修改者]陈洞滨
* [修改原因]
***************************************************************************/
void CTCPCommunicatio::GetIpInfo(PST_IP_INFO pstIpInfo)
{

    int iAdapterIdx=0; // 本机上各个网卡的索引号
    PIP_ADAPTER_INFO pAdapterInfo;    // 网卡信息结构体
    PIP_ADAPTER_INFO pAdapter = NULL;
    ULONG ulOutBufLen;    // 接收网卡信息缓冲区的大小
    pAdapterInfo= new IP_ADAPTER_INFO;   // 为网卡信息结构体分配内存
    ulOutBufLen = sizeof(IP_ADAPTER_INFO);

    DWORD dwRetVal = 0; // 返回值

    // 第一次调用GetAdapterInfo获取ulOutBufLen大小，此时调用会出错
    if (GetAdaptersInfo( pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
    {
        delete pAdapterInfo;
        // 分配接收缓存大小
        pAdapterInfo = (IP_ADAPTER_INFO *) malloc (ulOutBufLen); 
    }

    // 第二次调用GetAdapterInfo获取网卡信息，如果成功则可以通过该函数第一个参数所带出来的值获取相关信息
    if ((dwRetVal = GetAdaptersInfo( pAdapterInfo, &ulOutBufLen)) == NO_ERROR) 
    {
        pAdapter = pAdapterInfo;

        // 依次获取各个网卡信息
        while (pAdapter) 
        {
            // 网卡的MAC地址信息
            pstIpInfo[iAdapterIdx].abyMacAddr[0] = pAdapter->Address[0];
            pstIpInfo[iAdapterIdx].abyMacAddr[1] = pAdapter->Address[1];
            pstIpInfo[iAdapterIdx].abyMacAddr[2] = pAdapter->Address[2];
            pstIpInfo[iAdapterIdx].abyMacAddr[3] = pAdapter->Address[3];
            pstIpInfo[iAdapterIdx].abyMacAddr[4] = pAdapter->Address[4];
            pstIpInfo[iAdapterIdx].abyMacAddr[5] = pAdapter->Address[5];

            // IP地址
            IPStringtoByteArray(pAdapter->IpAddressList.IpAddress.String,pstIpInfo[iAdapterIdx].abyIPAddr);
            // 获取子网掩码信息,并存入结构体
            IPStringtoByteArray(pAdapter->IpAddressList.IpMask.String,pstIpInfo[iAdapterIdx].abySubNetMask);
            // 获取IP地址信息,并存入结构体
            IPStringtoByteArray(pAdapter->GatewayList.IpAddress.String,pstIpInfo[iAdapterIdx].abyGateWay);    

            // 计算广播地址，由位运算BroadCastAddr=IPAddr+ !(子网掩码)求得。其中+表示或，!表示非（即，取反）
            for (int i=0;i<4;i++)
            {
                pstIpInfo[iAdapterIdx].abyBroadCastAddr[i]=(pstIpInfo[iAdapterIdx].abyIPAddr[i])|~(pstIpInfo[iAdapterIdx].abySubNetMask[i]);
            }

            // 获取下一块网卡
            pAdapter = pAdapter->Next;
            // 网卡个数加一
            iAdapterIdx++;
        }
    }
    delete pAdapterInfo;
    pAdapterInfo=NULL;
}
/***************************************************************************
* 函数名称：[IPStringtoByteArray]
* 摘 要：该函数将点分格式的IP地址各个字段取出，存放到一个Byte型数组中
* 全局影响：
* 参数：char szIp[16]:[in] 输入的点分格式IP地址
*       byte abyByteArray[4]:[out] 存放IP地址各个字段的数组
* 返回值：void
*
* 修改记录：
* [日期]
* [作者/修改者]陈洞滨
* [修改原因]
***************************************************************************/
void CTCPCommunicatio::IPStringtoByteArray(char szIp[16],byte abyByteArray[4])
{
    CString cstrTemp=_T("");
    byte byIp;
    int iIpByteNum=0; // IP字段号

    for(int i=0;i<16;i++)
    {
        if (szIp[i]=='.'||szIp[i]=='\0')
        {
            if(iIpByteNum==4)
                return;
            // 将IP的一个字段有CSting转化成为整型
            byIp=_wtoi(cstrTemp);
            // 保存IP当前字段
            abyByteArray[iIpByteNum]=byIp;
            // 拷贝下一个字段
            cstrTemp=_T("");

            iIpByteNum++;
        }
        else
            // 拷贝IP字段
            cstrTemp=cstrTemp+szIp[i];
    }
}
/***************************************************************************
* 函数名称：[myFunction]
* 摘 要：简要描述本函数功能
* 全局影响：
* 参数：param1:[in/out] 参数作用描述
* 返回值：
*
* 修改记录：
* [日期]
* [作者/修改者]陈洞滨
* [修改原因]
***************************************************************************/
void CTCPCommunicatio::ByteIpToStringIp(byte abyByteArray[4],char* pcStringIp)
{
    USES_CONVERSION;

    CString cstrTempIp=_T("");
    // 将BYTE型IP转化成为CString型
    cstrTempIp.Format(_T("%d.%d.%d.%d"),abyByteArray[0],abyByteArray[1],abyByteArray[2],abyByteArray[3]);
    // 将CSting型IP转化为char*型
    pcStringIp=W2A(cstrTempIp);
    return;
}
/***************************************************************************
* 函数名称：[GetIpAddr]
* 摘 要：该函数用于获取本地IP地址
* 全局影响：
* 参数：PST_IP_INFO pstIpInfo:[in] IP地址信息指针，指向一个数组，该数组每个
*                                  元素都是IP地址信息
*       UINT &uiIpNum:[out] 本机上网卡个数
* 返回值：void 
*
* 修改记录：
* [日期]
* [作者/修改者]陈洞滨
* [修改原因]
***************************************************************************/
void CTCPCommunicatio::GetIpAddr(PST_IP_INFO pstIpInfo,UINT &uiIpNum)
{
    char szHostName[HOST_NAME_LENGTH]; // 主机名字

    // 获取本地主机地址
    if (gethostname(szHostName,HOST_NAME_LENGTH)==0)
    {
        // 接收主机地址信息
        struct hostent* pstHostAddrInfo; 
        // 依据主机名字获取其地址信息
        pstHostAddrInfo=gethostbyname(szHostName);

        // 针对有多个网卡的主机，依次获取它的各个IP地址
        for (UINT i=0;pstHostAddrInfo!=NULL && pstHostAddrInfo->h_addr_list[i]!=NULL;i++)
        {
            BYTE szTempIpBuf[IP_BYTE_LENGTH]; // 存放IP的缓存
            memset(szTempIpBuf,0,IP_BYTE_LENGTH); // 初始化IP缓存

            // 存放IP的四个字段
            for (int j=0;j<pstHostAddrInfo->h_length;j++)
            {
                szTempIpBuf[j]=(BYTE)((unsigned char*)pstHostAddrInfo->h_addr_list[i])[j];
            }

            // 保存有效IP
            if (szTempIpBuf[0]!=127&& szTempIpBuf[0]!=0 && szTempIpBuf[0]<224)
            {
                memcpy(pstIpInfo[uiIpNum].abyIPAddr, szTempIpBuf, IP_BYTE_LENGTH);
                // IP个数加一
                uiIpNum ++;
            }

        }

    }

    return ;
}
/***************************************************************************
* 函数名称：[]
* 摘 要：
* 全局影响：
* 参数：[in/out] 
* 返回值：
*
* 修改记录：
* [日期]
* [作者/修改者]陈洞滨
* [修改原因]
***************************************************************************/
CTCPComBase* TCPComExports(LPVOID lpvoid)
{
    return (CTCPComBase*)new CTCPCommunicatio();
}
/***************************************************************************
* 函数名称：[]
* 摘 要：
* 全局影响：
* 参数：[in/out] 
* 返回值：
*
* 修改记录：
* [日期]
* [作者/修改者]陈洞滨
* [修改原因]
***************************************************************************/
BOOL CTCPCommunicatio::ReleseTCPCom(CTCPComBase*  pclsTCPComBase)
{
    if (pclsTCPComBase != NULL)
    {
        delete pclsTCPComBase;
        pclsTCPComBase = NULL;
        return TRUE;
    }
    else
    {
        CCsccLog::DebugPrint("卸载DLL错误！\n");
        return FALSE;
    }
}




