#include "StdAfx.h"
#include <atlbase.h>
#include <IPHlpApi.h>  // 操作网卡相关的函数库头文件
#include "UDPCommunicatio.h"

#define  BUFFER_SIZE 1024   //接收缓冲大小

CUDPCommunicatio::CUDPCommunicatio(void):
 m_uiCurrentThdNum(0)
{
	//初始化访问线程链表关键代码段
	InitializeCriticalSection(&m_crt_Thread);

}

CUDPCommunicatio::~CUDPCommunicatio(void)
{
	//清除访问线程链表关键代码段
	DeleteCriticalSection(&m_crt_Thread);
}


#pragma region Initialize
/***************************************************************************
* 函数名称：[InitUDPCom]
* 摘 要：UDP通信类的初始化，在该函数中加载windows套接字版本库
* 全局影响：
* 参数：void
* 返回值：BOOL 成功初始化返回TRUE;否则，返回FALSE
*
* 修改记录：
*[日期]2011-12-08
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/
BOOL CUDPCommunicatio::InitUDPCom()
{
	int iInitialError;								// 函数返回值
	WSADATA data;       							// 初始化套接字
	iInitialError=WSAStartup(MAKEWORD(2,2),&data);	// 2.2版本套接字

	//  initiates use of WS2_32.DLL
	if (0 != iInitialError)
	{
		CCsccLog::DebugPrint("套接字版本加载失败！\n");
		return FALSE;;
	}

	// 判断加载的套接字版本是否正确
	if ( LOBYTE( data.wVersion ) != 2 || HIBYTE( data.wVersion ) != 2 )
	{
		CCsccLog::DebugPrint("套接字版本加载错误！\n");

		// release use of WS2_32.DLL
		WSACleanup();
		return FALSE;
	}

	return TRUE;

}
/***************************************************************************
* 函数名称：[UnInitUDPCom]
* 摘 要：UDP通信类反初始化函数，在其中卸载windows套接字版本库
* 全局影响：
* 参数：void
* 返回值：BOOL 函数执行成功返回TRUE;否则，返回FALSE
*
* 修改记录：
*[日期]2011-12-08
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/
BOOL CUDPCommunicatio::UnInitUDPCom()
{
	int iUnInitialErr = 0;					// 函数返回值

	// 释放套接字库
	iUnInitialErr = WSACleanup();

	// 判断是否成功释放
	if (0 != iUnInitialErr)
	{
		// 判断错误代码是否是WSANOTINITIALISED
		if (WSANOTINITIALISED == WSAGetLastError())
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
#pragma endregion Initialize

/***************************************************************************
* 函数名称：SetThreadObj
* 摘 要：服务器数量增加时，需创建额外的线程，相应的增加一个线程对象
* 全局影响：
* 参数：param1:
* 返回值：PST_THD_OBJ 新增的线程对象结构体
*
* 修改记录：
*[日期]2011-12-08
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/
PST_THD_OBJ CUDPCommunicatio::SetThreadObj()
{
	PST_THD_OBJ pstThreadInfoObj = new ST_THD_OBJ; //TODO：记得释放，已经在FreeThreadObj中释放
	if (pstThreadInfoObj != NULL)
	{
		//初始化访问服务器端对象链表关键代码段
		InitializeCriticalSection(&(pstThreadInfoObj->crt_Server));

		// 一个事件对象，用于标识该线程对应的事件句柄数组是否需要重建
		// 件默认无信号，手动复位，用于在删除服务器标识Event数组的重构
		pstThreadInfoObj->hSelectEvents[0] = WSACreateEvent();

		//线程对象初始套接字个数
		pstThreadInfoObj->uiSocketNum = 0;
		pstThreadInfoObj->uiThreadID = m_uiCurrentThdNum;
		pstThreadInfoObj->lpClassObj = this;

		// 启动线程
		pstThreadInfoObj->pThread = AfxBeginThread(Thread_Server_Proc, (LPVOID)pstThreadInfoObj);

		Sleep(100);

		m_lstThreadList.AddTail(pstThreadInfoObj);
		m_uiCurrentThdNum++; //UNDONE:这里需不需要互斥？
	}

	 return pstThreadInfoObj;
}

/***************************************************************************
* 函数名称：FreeThreadObj
* 摘 要：当处理IO的线程关闭后，释放线程对象
* 全局影响：
* 参数：PST_THD_OBJ pThreadObj 待释放的线程对象
* 返回值：BOOL 如果在线程对象链表中找到所要释放节点，则将其释放并返回True；
*
* 修改记录：
*[日期]2011-12-08
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/
BOOL CUDPCommunicatio::FreeThreadObj(PST_THD_OBJ pstThreadObj)
{
	// 进入临界区
	EnterCriticalSection(&m_crt_Thread);

	PST_THD_OBJ pstListNode = NULL;								// 线程对象，初始化为空

	// 在线程对象链表中遍历查到所要删除的线程对象
	for (POSITION posThdObj = m_lstThreadList.GetHeadPosition();// 取得链表头部节点，通常为最后建立的线程对象
		 posThdObj != NULL;                                     // 链表节点指针已经到达链表末尾
		 m_lstThreadList.GetNext(posThdObj))                    // 将链表节点指针向后移动
	{
		pstListNode = m_lstThreadList.GetAt(posThdObj);         // 取出链表中的节点值
		if (pstListNode->uiThreadID == pstThreadObj->uiThreadID)// 在线程对象链表中找到与待删除线程对象一致的节点
		{
			// 将节点从链表中移除
			m_lstThreadList.RemoveAt(posThdObj);

			// 关闭重构线程对象数组事件
			WSACloseEvent(pstListNode->hSelectEvents[0]);  

			// 释放临界区资源
			DeleteCriticalSection(&pstListNode->crt_Server);
			
			// 释放节点占用的内存
			delete pstListNode;                                 
			pstListNode = NULL;
			break;
		}
	}

	// 离开临界区
	LeaveCriticalSection(&m_crt_Thread);

    return TRUE;
}

/***************************************************************************
* 函数名称：SetServerObj
* 摘 要：创建一个新的服务器端对象
* 全局影响：
* 参数：SOCKET ClinetSocket 服务器端通信套接字
* 返回值： PST_SERVER_OBJ 服务器端对象实例
*
* 修改记录：
*[日期]2011-12-08
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/
PST_SERVER_OBJ CUDPCommunicatio::SetServerObj(SOCKET sktServer, BYTE abyServerIp[IP_BYTE_LENGTH], UINT uiServerPort)
{
	PST_SERVER_OBJ pstServerInfo = new ST_SERVER_OBJ;//TODO：记得释放,已经在FreeServerObj进行释放

	// 服务器端对象指针不为空
	if (pstServerInfo!=NULL)
	{
		//初始化访问服务器链表临界区
		InitializeCriticalSection(&(pstServerInfo->crt_RcvPacket));
		
		//创建服务器端通信套接字
		pstServerInfo->sktServerSocket = sktServer;

		//服务器端地址族
		memcpy(pstServerInfo->abyServerIp, abyServerIp, IP_BYTE_LENGTH);
		pstServerInfo->uiServerPort = uiServerPort;

		// 创建与套接字相关的事件，初始无信号，手动复位
		pstServerInfo->hServerEvent = WSACreateEvent();

		//为套接字注册网络事件
		int iretVal = WSAEventSelect(sktServer, pstServerInfo->hServerEvent, FD_READ | FD_WRITE);
		if (iretVal == SOCKET_ERROR)
		{
			CCsccLog::DebugPrint("注册网络事件出错！\n");
		}

		AssignToFreeThreadObj(pstServerInfo);
	}

	return pstServerInfo;
}

/***************************************************************************
* 函数名称：FreeServerObj
* 摘 要：释放一个服务器端对象
* 全局影响：
* 参数：PST_SERVER_OBJ pServerObj  服务器端对象实例
* 返回值：void
*
* 修改记录：
*[日期]2011-12-08
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/
void CUDPCommunicatio::FreeServerObj(PST_SERVER_OBJ pstServerObj)
{
	//关闭与服务器端套接字相关的事件
	if (pstServerObj->hServerEvent)
	{
		WSACloseEvent(pstServerObj->hServerEvent);
	}

	//关闭套接字
	if (pstServerObj->sktServerSocket!=INVALID_SOCKET)
	{
		closesocket(pstServerObj->sktServerSocket);
	}

	//DeleteCriticalSection(&(pstServerObj->crt_packet));
	DeleteCriticalSection(&(pstServerObj->crt_RcvPacket));

	//删除服务器端对象
	delete pstServerObj;

	return;

}


/***************************************************************************
* 函数名称：[AddClinetObj]
* 摘 要：为新增的服务器端创建一个套接字，并将它加入到一个服务器端对象中
* 全局影响：
* 参数：CString strServerIP:[out] 服务器端IP
*       UINT uiServerPort:[in] 服务器端Port
* 返回值：BOOL
*
* 修改记录：
*[日期]2011-12-08
*[作者/修改者] 陈洞滨
*[修改原因]
***************************************************************************/
BOOL CUDPCommunicatio::AddServerObj(BYTE abyServerIp[IP_BYTE_LENGTH], UINT uiServerPort)
{
	USES_CONVERSION;

	CString strServerIp = _T("");
	strServerIp.Format(_T("%d.%d.%d.%d"), abyServerIp[0], abyServerIp[1], abyServerIp[2], abyServerIp[3]);
	char* pcServerIp = "";
	pcServerIp = W2A(strServerIp);

	//创建UDP通信套接字
	SOCKET sktServer = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET==sktServer)
	{
		CCsccLog::DebugPrint("创建服务器端UDP通信套接字错误！\n");
		return FALSE;
	}

	//定义错误代码
	int iErrCode;

	//待设置的套接字缓冲区大小
	UINT uiBuflen;

	//缓冲区大小
	UINT uiOptlen = sizeof(uiBuflen);

	//获取发送缓冲区参数
	iErrCode = getsockopt(sktServer,		//套接字
						SOL_SOCKET,			//套接字级别，此处选择套接字所在的应用层
						SO_SNDBUF,			//将要设置的套接字选项名称，此处为缓冲区大小
						(char *)&uiBuflen,	//设置的套接字选项值
						(int*)&uiOptlen);	//缓冲区大小

	//判断获取缓冲区参数是否异常
	if (SOCKET_ERROR == iErrCode)                                              
	{
		CCsccLog::DebugPrint("获取发送缓冲区大小失败！\n");
		return INVALID_SOCKET;
	}

	//将原先的缓冲区大小扩大十倍，系统默认为8192byte(8k)
	uiBuflen *= 10;

	//设定缓冲区大小                                                           
	iErrCode = setsockopt(sktServer, SOL_SOCKET, SO_RCVBUF, (char *)&uiBuflen, uiOptlen);

	//判断设置缓冲区大小是否异常
	if (SOCKET_ERROR == iErrCode)
	{
		CCsccLog::DebugPrint("设置发送缓冲区大小失败！\n");
		return INVALID_SOCKET;
	}

	// 检查缓冲区大小是否修改成功
	unsigned int uiNewRcvBuf;                                               
	iErrCode = getsockopt(sktServer, SOL_SOCKET, SO_SNDBUF, (char*)&uiNewRcvBuf, (int*)&uiOptlen);
	if (SOCKET_ERROR == iErrCode || uiNewRcvBuf == uiBuflen)
	{
		CCsccLog::DebugPrint("修改系统发送数据缓冲区失败！\n");
		return INVALID_SOCKET;
	}
	
	// 绑定服务器IP地址
	sockaddr_in sdServerAddr;									// 服务器端地址簇
	sdServerAddr.sin_addr.S_un.S_addr = inet_addr(pcServerIp);	// 发送本机IP
	sdServerAddr.sin_family = AF_INET;
	sdServerAddr.sin_port = htons(uiServerPort);				// 端口号,主机字节序三转换为网络字节序

	//绑定套接字
	int iBindErr = bind(sktServer, (LPSOCKADDR)&sdServerAddr, sizeof(sdServerAddr));
	if (iBindErr == SOCKET_ERROR)
	{
		CCsccLog::DebugPrint("绑定接收套接字出错！\n");
		return FALSE;
	}

	SetServerObj(sktServer, abyServerIp, uiServerPort);			// 将UDP通信套接字加入到服务器端对象中

	return TRUE;

}
/**************************************************************************
* 函数名称：[myFunction]
* 摘 要：简要描述本函数功能
* 全局影响：
* 参数：PST_THD_OBJ pstThreadObj:[in] 待移除服务器端所在的线程对象
*       ST_SERVER_OBJ &stServerObj:[in] 待移除服务器端对象
* 返回值：BOOL 
*
* 修改记录：
*[日期]2011-12-08
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/
BOOL CUDPCommunicatio::RemoveServerObj(PST_THD_OBJ pstThreadObj,ST_SERVER_OBJ &stServerObj)
{
	ST_LOG stUdpLog;
	stUdpLog.iType=LOG_INFO;
	EnterCriticalSection(&(pstThreadObj->crt_Server));
	//m_clsCsccLog.MyNSprintf(stUdpLog.swzMsg,MAX_LOG_LENGTH,"RemoveServerObj EnterCriticalSection -----pstThreadObj->crt_Server");
	//m_clsCsccLog.WriteLog(stUdpLog.iType,stUdpLog.swzMsg);



	PST_SERVER_OBJ pstServerObj=NULL;
	for (POSITION posServerObj=pstThreadObj->lstServer.GetHeadPosition(); //取得服务器端链表头节点
		 posServerObj!=NULL;                                               //链表节点指针未到尾部
		 pstThreadObj->lstServer.GetNext(posServerObj))                    //向后移动链表节点指针
	{
		//取出服务器端链表节点
		pstServerObj=pstThreadObj->lstServer.GetAt(posServerObj);

		//如果找到服务器端则将它从链表中移除
		if (pstServerObj->sktServerSocket==stServerObj.sktServerSocket)
		{
			pstThreadObj->lstServer.RemoveAt(posServerObj);

			//线程对象管理的套接字个数减一
			pstThreadObj->uiSocketNum--;    
			break;
		}
	}

	LeaveCriticalSection(&(pstThreadObj->crt_Server));
	//m_clsCsccLog.MyNSprintf(stUdpLog.swzMsg,MAX_LOG_LENGTH,"RemoveServerObj LeaveCriticalSection -----pstThreadObj->crt_Server");
	//m_clsCsccLog.WriteLog(stUdpLog.iType,stUdpLog.swzMsg);

	//指示服务器端对象更新事件句柄数组
	WSASetEvent(pstThreadObj->hSelectEvents[0]);

	return TRUE;

}
/***************************************************************************
* 函数名称：[ReBuildEventArry]
* 摘 要：当线程对象中服务器端数目发生变化时，更新事件句柄数组
* 全局影响：
* 参数：PST_THD_OBJ pstThreadObj:[in] 需要重构事件数组的线程对象
* 返回值：void
*
* 修改记录：
*[日期]2011-12-08
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/
void CUDPCommunicatio::ReBuildEventArry(PST_THD_OBJ pstThreadObj)
{
	EnterCriticalSection(&(pstThreadObj->crt_Server));

	int iNum = 1;
	PST_SERVER_OBJ pstServerObj = NULL;

	for (POSITION posServerObj = pstThreadObj->lstServer.GetHeadPosition();	//线程对象中服务器端对象链表头
		posServerObj != NULL;												//未到服务器端链表尾
		pstThreadObj->lstServer.GetNext(posServerObj))						//服务器端链表指针后移
	{
		// 取出每个服务器端对应的事件句柄
		pstServerObj=pstThreadObj->lstServer.GetAt(posServerObj);

		// 重构事件句柄数组
		pstThreadObj->hSelectEvents[iNum] = pstServerObj->hServerEvent; 
		iNum++;
	}

	LeaveCriticalSection(&(pstThreadObj->crt_Server));
	return;
}
/***************************************************************************
* 函数名称：[InsertServerObj]
* 摘 要：简要描述本函数功能
* 全局影响：
* 参数：PST_THD_OBJ pstThreadObj:[in] 欲添加到的线程对象指针
*       PST_SERVER_OBJ pstServerObj:[in] 待添加的服务器端对象指针
* 返回值：BOOL 
*
* 修改记录：
*[日期]2011-12-08
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/
BOOL CUDPCommunicatio::InsertServerObj(PST_THD_OBJ pstThreadObj,PST_SERVER_OBJ pstServerObj)
{
	BOOL bRet=FALSE;

	// 进入临界区
	EnterCriticalSection(&(pstThreadObj->crt_Server));

	if (pstThreadObj->uiSocketNum < WSA_MAXIMUM_WAIT_EVENTS-1)
	{
		//线程对象管理的套接字个数加一
		pstThreadObj->uiSocketNum++;

        //为服务器端对象分配ID索引号，该索引号从1开始
		pstServerObj->uiServerID=pstThreadObj->uiSocketNum;

		//新节点加入到线程对象链表中
		pstThreadObj->lstServer.AddHead(pstServerObj);
		
		//插入成功
		bRet=TRUE;
	}

	LeaveCriticalSection(&(pstThreadObj->crt_Server));
	
	//更新线程对象链表
	WSASetEvent(pstThreadObj->hSelectEvents[0]);

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
void CUDPCommunicatio::AssignToFreeThreadObj(PST_SERVER_OBJ pstServerObj)
{
	ST_LOG stUdpLog;
	stUdpLog.iType = LOG_INFO;

	// 进入临界区
	EnterCriticalSection(&(m_crt_Thread));

	PST_THD_OBJ pstThdObj = NULL;

	POSITION posThdObj = NULL;
	if (!m_lstThreadList.IsEmpty()) //链表不为空时，在链表所有节点中查找空闲节点
	{
		for (posThdObj = m_lstThreadList.GetHeadPosition();
		     posThdObj != NULL;
		     m_lstThreadList.GetNext(posThdObj))
		{
			pstThdObj = m_lstThreadList.GetAt(posThdObj);
			
			//试图在当前线程对象中插入新的服务器端节点
			if (InsertServerObj(pstThdObj, pstServerObj))
			{
				//插入成功则跳出
				break;
			}
		}
	} 

	//如果链表中每个节点均没有空闲或者链表本身为空，则创建新线程
	if (posThdObj == NULL || m_lstThreadList.IsEmpty())
	{
		pstThdObj = SetThreadObj();
		InsertServerObj(pstThdObj, pstServerObj);
	}

	// 离开临界区
	LeaveCriticalSection(&(m_crt_Thread));
	return;
}
/***************************************************************************
* 函数名称：[FindServerObj]
* 摘 要：该函数依据事件的绝对索引号，从指定的线程对象中查找服务器端对象
* 全局影响：
* 参数：PST_THD_OBJ pstThdObj:[in]  事件所在的线程对象
*       int iEventIndex:[in] 事件在数组中的绝对索引号
* 返回值：PST_SERVER_OBJ 查找成功，返回查找到的服务器端对象；否则，返回NULL
*
* 修改记录：
*[日期]2011-12-08
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/
PST_SERVER_OBJ CUDPCommunicatio::FindServerObj(PST_THD_OBJ pstThdObj, int iEventIndex)
{
	PST_SERVER_OBJ pstServerObj=NULL;

	//遍历链表，依据事件ID号查找对应的服务器端对象
	for (POSITION posServer=pstThdObj->lstServer.GetHeadPosition();
		 posServer!=NULL;
		 pstThdObj->lstServer.GetNext(posServer))
	{
		//取得链表节点
		pstServerObj=pstThdObj->lstServer.GetAt(posServer);
		
		//如果查找到对应的服务器端则将该服务器端返回
		if (pstServerObj->uiServerID == iEventIndex)
		{
			return pstServerObj;
		}
	}

	return NULL;
}
/***************************************************************************
* 函数名称：[BufferSave]
* 摘 要：将接收的数据包，添加到链表中
* 全局影响：
* 参数：PST_SERVER_OBJ pstServerObj:[in] 数据包对应的服务器端对象
*       PST_PACKATE pstPackage:[in] 接收到的数据包
* 返回值：BOOL 成功将数据包插入链表中，返回TRUE;否则，返回FALSE
*
* 修改记录：
*[日期]2011-12-08
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/
//BOOL CUDPCommunicatio::BufferSave(PST_SERVER_OBJ pstServerObj,PST_UDP_PACKATE pstPackage)							  
//{
//	if (pstPackage==NULL)
//	{
//		return FALSE;
//	}
//
//    //链表不为空
//	if (!(pstServerObj->lstPackage.IsEmpty()))
//    {
//		POSITION posNodeFrame=pstServerObj->lstPackage.GetHeadPosition();
//		POSITION posTail=pstServerObj->lstPackage.GetTailPosition();
//		PST_UDP_PACKATE pstNodePackage=pstServerObj->lstPackage.GetHead();
//		//比较数据包帧号
//		while(1)
//		{
//			//待插入包帧号小于参考包号，直接将包插入参考包前
//			if (pstNodePackage->uiFrameNum > pstPackage->uiFrameNum)
//			{
//				PST_UDP_PACKATE pstTempPackage=new ST_UDP_PACKATE;              // 临时数据包，用于将接收到的数据加入到链表中
//				
//				pstTempPackage->uiFrameNum=pstPackage->uiFrameNum;      // 拷贝帧号
//				pstTempPackage->uiPackageNum=pstPackage->uiPackageNum;  // 拷贝包号
//				pstTempPackage->uiDataLength=pstPackage->uiDataLength;  // 数据包中数据的长度
//				pstTempPackage->uiTotalDataLength=pstPackage->uiDataLength; //数据包中数据总长度
//				memcpy(pstTempPackage->abyData,pstPackage->abyData,DATA_BUF_LENGTH); // 拷贝数据
//
//				pstServerObj->lstPackage.InsertBefore(posNodeFrame,pstTempPackage);
//
//				break;
//				
//			} 
//			else if (pstNodePackage->uiFrameNum==pstPackage->uiFrameNum) //待插入包帧号与参考包帧号相同，则比较包号
//			{
//				POSITION posNodePacket=posNodeFrame;
//
//				//循环链表节点比较包号
//			    while(1)
//			    {
//					
//					if (pstNodePackage->uiFrameNum==pstPackage->uiFrameNum)
//					{
//						if (pstNodePackage->uiPackageNum > pstPackage->uiPackageNum) // 待插入数据包的包号大于节点数据包包号
//						{
//							PST_UDP_PACKATE pstTempPackage=new ST_UDP_PACKATE;               // 临时数据包，用于将接收到的数据加入到链表中
//
//							pstTempPackage->uiFrameNum=pstPackage->uiFrameNum;       // 拷贝帧号
//							pstTempPackage->uiPackageNum=pstPackage->uiPackageNum;   // 拷贝包号
//							pstTempPackage->uiDataLength=pstPackage->uiDataLength;   // 数据包中数据的长度
//							pstTempPackage->uiTotalDataLength=pstPackage->uiDataLength; //数据包中数据总长度
//							memcpy(pstTempPackage->abyData,pstPackage->abyData,DATA_BUF_LENGTH); // 拷贝数据
//
//							pstServerObj->lstPackage.InsertBefore(posNodeFrame,pstTempPackage);
//
//							break;
//						}
//
//					} 
//					else
//					{
//						    PST_UDP_PACKATE pstTempPackage=new ST_UDP_PACKATE;                  // 临时数据包，用于将接收到的数据加入到链表中
//
//							pstTempPackage->uiFrameNum=pstPackage->uiFrameNum;          // 拷贝帧号
//							pstTempPackage->uiPackageNum=pstPackage->uiPackageNum;      // 拷贝包号
//							pstTempPackage->uiDataLength=pstPackage->uiDataLength;      // 数据包中数据的长度
//							pstTempPackage->uiTotalDataLength=pstPackage->uiDataLength; //数据包中数据总长度
//							memcpy(pstTempPackage->abyData,pstPackage->abyData,DATA_BUF_LENGTH); // 拷贝数据
//
//							pstServerObj->lstPackage.InsertBefore(posNodeFrame,pstTempPackage);
//
//							break;
//					}
//
//					//链表尾节点，在链表末尾插入数据包
//					if (posNodePacket==posTail)
//					{
//						PST_UDP_PACKATE pstTempPackage=new ST_UDP_PACKATE;                  // 临时数据包，用于将接收到的数据加入到链表中
//
//						pstTempPackage->uiFrameNum=pstPackage->uiFrameNum;          // 拷贝帧号
//						pstTempPackage->uiPackageNum=pstPackage->uiPackageNum;      // 拷贝包号
//						pstTempPackage->uiDataLength=pstPackage->uiDataLength;      // 数据包中数据的长度
//						pstTempPackage->uiTotalDataLength=pstPackage->uiDataLength; //数据包中数据总长度
//						memcpy(pstTempPackage->abyData,pstPackage->abyData,DATA_BUF_LENGTH); // 拷贝数据
//
//						pstServerObj->lstPackage.InsertAfter(posNodeFrame,pstTempPackage);
//
//						break;
//
//					}
//
//					//更改链表节点位置
//					pstServerObj->lstPackage.GetNext(posNodePacket);
//					//取得新节点位置数据包
//					pstNodePackage=pstServerObj->lstPackage.GetAt(posNodePacket);
//		
//			    }
//
//				break;
//			}
//
//			//链表尾节点，在链表末尾插入数据包
//			if (posNodeFrame==posTail)
//			{
//				PST_UDP_PACKATE pstTempPackage=new ST_UDP_PACKATE;                  // 临时数据包，用于将接收到的数据加入到链表中
//
//				pstTempPackage->uiFrameNum=pstPackage->uiFrameNum;          // 拷贝帧号
//				pstTempPackage->uiPackageNum=pstPackage->uiPackageNum;      // 拷贝包号
//				pstTempPackage->uiDataLength=pstPackage->uiDataLength;      // 数据包中数据的长度
//				pstTempPackage->uiTotalDataLength=pstPackage->uiDataLength; //数据包中数据总长度
//				memcpy(pstTempPackage->abyData,pstPackage->abyData,DATA_BUF_LENGTH); // 拷贝数据
//
//				pstServerObj->lstPackage.InsertAfter(posNodeFrame,pstTempPackage);
//
//				break;
//
//			}
//			//更改链表节点位置
//			pstServerObj->lstPackage.GetNext(posNodeFrame);
//            //取得新节点位置数据包
//			pstNodePackage=pstServerObj->lstPackage.GetAt(posNodeFrame);
//		
//		}
//
//
//    } 
//    else  //链表为空时，将节点为链表增加一个头结点
//    {
//		PST_UDP_PACKATE pstTempPackage=new ST_UDP_PACKATE;              // 临时数据包，用于将接收到的数据加入到链表中
//
//		pstTempPackage->uiFrameNum=pstPackage->uiFrameNum;      // 拷贝帧号
//		pstTempPackage->uiPackageNum=pstPackage->uiPackageNum;  // 拷贝包号
//		pstTempPackage->uiDataLength=pstPackage->uiDataLength;  // 数据包中数据的长度
//		pstTempPackage->uiTotalDataLength=pstPackage->uiDataLength; //数据包中数据总长度
//		memcpy(pstTempPackage->abyData,pstPackage->abyData,DATA_BUF_LENGTH); // 拷贝数据
//
//		pstServerObj->lstPackage.AddHead(pstTempPackage);
//    }
//
//
//	
//	return TRUE;
//}
/***************************************************************************
* 函数名称：[Has_a_nal]
* 摘 要：该函数用于判断接收的数据是否可以组成一帧
* 全局影响：
* 参数：PST_SERVER_OBJ pstServerObj:[in] 服务器端对象
* 返回值：BOOL 链表中有一帧完整数据时返回TRUE;否则，返回FALSE
*
* 修改记录：
*[日期]2011-12-08
*[作者/修改者] 陈洞滨
*[修改原因]
***************************************************************************/
//BOOL CUDPCommunicatio::Has_a_nal(PST_SERVER_OBJ pstServerObj) //判断缓存数据是否是同一帧
//{
//	PST_UDP_PACKATE pstHeadPackage,pstNodePackage;//缓存头结点数据包指针和当前位置数据包指针
//
//	POSITION posNode = pstServerObj->lstPackage.GetHeadPosition();//头结点位置
//	pstHeadPackage = pstServerObj->lstPackage.GetHead();//头节点
//	pstNodePackage=pstServerObj->lstPackage.GetAt(posNode); //取得当前位置节点，并将节点指针后移
//	//一个数据包中数据大小
//	int iDataLength=0;
//	//一帧数据总大小
//	int iTotalDataLength=pstHeadPackage->uiTotalDataLength;
//
//	int iCounter =0;	//计数器
//	while(pstHeadPackage->uiFrameNum ==pstNodePackage->uiFrameNum  &&posNode !=NULL/*pstServerObj->lstPackage.GetTailPosition()*/)   //计数
//	{
//		//计算数据的总大小
//		iDataLength+=pstNodePackage->uiDataLength;
//		//更改节点位置
//		pstNodePackage=pstServerObj->lstPackage.GetNext(posNode);
//		//计数属于同一帧的数据节点个数
//		iCounter++; 
//	}
//    //求包号差
//	int iNodediff = pstNodePackage->uiPackageNum - pstHeadPackage->uiPackageNum+1; //HACK:此处是否需要-1？
//	
//	//判断包号范围和帧数据节点个数是否一致，如果一致则为一帧，如果不一致则不是一帧 
//	if((iCounter == iNodediff)) 
//	{		
//		//查看该帧数据是否完整
//		if (iTotalDataLength==iDataLength)
//		{
//			return TRUE;
//		}
//		else
//		{
//			//数据包丢包或者一帧还没有缓存完毕
//			return FALSE;
//		}
//
//	}
//	else
//	{
//		return FALSE;
//	}
//
//
//}

/***************************************************************************
* 函数名称：[FillBuffer]
* 摘 要：简要描述本函数功能
* 全局影响：
* 参数：param1:[in/out] 参数作用描述
* 返回值：
*
* 修改记录：
*[日期]
*[作者/修改者]
*[修改原因]
***************************************************************************/
//BOOL CUDPCommunicatio::FillBuffer(PST_SERVER_OBJ pstServerObj,SOCKADDR_IN sdClientAddr) //填充缓冲区
//{
//
//	EnterCriticalSection(&(pstServerObj->crt_packet));
//	POSITION posNode;//位置变量
//	UINT uiNodeFrame; //头结点帧号			
//	if(pstServerObj->lstPackage.GetCount() >= 1) //缓冲链表是否超过5个节点   
//	{
//		//头结点位置
//		posNode = pstServerObj->lstPackage.GetHeadPosition();		
//		//头结点
//		PST_UDP_PACKATE pstHeadPackage = pstServerObj->lstPackage.GetHead();
//		PST_UDP_PACKATE pstNodePackage=pstServerObj->lstPackage.GetAt(posNode);
//		//头结点帧号
//		uiNodeFrame=pstHeadPackage->uiFrameNum;      
//		//一帧数据的总大小
//		UINT uiTotalDataLength=pstHeadPackage->uiTotalDataLength;
//
//
//
//		//判断是否是同一帧，如果是则提取数据组成一帧
//		if(Has_a_nal(pstServerObj))
//		{  
//            //取得当前位置节点，并将节点指针后移
//			//pstHeadPackage=pstServerObj->lstPackage.GetNext(posNode); 
//
//			//组包之后的数据结构
//			PST_UDPPACKET pstUDPPackage=new ST_UDPPACKET;
//			pstUDPPackage->pbyRcvData=new byte[uiTotalDataLength];
//			pstUDPPackage->uiRcvDatalength=uiTotalDataLength;
//			pstUDPPackage->sdSource=sdClientAddr;         //UNDONE:源地址信息没有添加进来
//
//			//数据拷贝偏移量
//			UINT uiOffsetLength=0;
//			
//			//从链表头开始遍历，找出属于同一帧的数据包
//			while(pstHeadPackage->uiFrameNum ==pstNodePackage->uiFrameNum  &&posNode !=NULL/*pstServerObj->lstPackage.GetTailPosition()*/)
//			{
//				//拷贝数据
//				memcpy(pstUDPPackage->pbyRcvData+uiOffsetLength,pstNodePackage->abyData,pstNodePackage->uiDataLength);
//				uiOffsetLength+=pstNodePackage->uiDataLength;		
//
//				//拷贝完成后，将原来链表中的节点移除
//				pstServerObj->lstPackage.RemoveAt(posNode);
//				//delete pstNodePackage; //TODO:此处是否需要删除需进一步确认
//				if (!pstServerObj->lstPackage.IsEmpty())
//				{
//					pstNodePackage=pstServerObj->lstPackage.GetNext(posNode);
//
//				}
//				else
//				{
//					posNode=NULL;
//				}
//		
//
//			}
//
//			EnterCriticalSection(&(pstServerObj->crt_RcvPacket));
//			//将组好的数据节点添加到链表中
//			pstServerObj->lstRcvPackage.AddHead(pstUDPPackage);
//			LeaveCriticalSection(&(pstServerObj->crt_RcvPacket));
//
//		}
//		else 
//		{
//			//不够一帧数据则丢掉和头结点属于同一帧的包
//			POSITION posTail= pstServerObj->lstPackage.GetTailPosition();//获取尾节点位置
//			PST_UDP_PACKATE pstNodepacket=pstServerObj->lstPackage.GetAt(posNode); //改变链表指针，指向下一个
//			if(posNode != posTail )
//				while(uiNodeFrame == pstNodepacket->uiFrameNum)//如果属于同一帧数据则丢掉这些数据			
//				{
//					pstNodepacket=pstServerObj->lstPackage.GetNext(posNode);
//					//从包缓存中移除头结点
//					pstServerObj->lstPackage.RemoveHead();
//					//如果到了链表末尾则退出
//					if(posNode == posTail)
//						break;					
//				}
//
//		}
//		
//	}	
//
//	LeaveCriticalSection(&(pstServerObj->crt_packet));
//
//	return TRUE;
//
//} 
/***************************************************************************
* 函数名称：[myFunction]
* 摘 要：简要描述本函数功能
* 全局影响：
* 参数：param1:[in/out] 参数作用描述
* 返回值：
*
* 修改记录：
*[日期]
*[作者/修改者]
*[修改原因]
***************************************************************************/
INT CUDPCommunicatio::UDPRecv(PST_SERVER_OBJ pstServerObj)
{
	int ireVal=0;  //返回值

	SOCKADDR_IN sdClientAddr;
	int iClientAddrLength = sizeof(sdClientAddr);

    PST_UDP_PACKATE pstUdpPackage = new ST_UDP_PACKATE;				//接收数据包结构体，需要在调用GetPacket时释放	

	ireVal = recvfrom(pstServerObj->sktServerSocket,				//接收套接字
		             (char*)pstUdpPackage,							//接收数据缓存
		              sizeof(ST_UDP_PACKATE),						//接收数据缓存大小
		              0,											//标志位，此处表示无特殊行为
		             (SOCKADDR*)&sdClientAddr,						//数据源地址族
		              &iClientAddrLength);							//数据源地址族长度

	if (ireVal == 0)
	{
		return 0;
	} 
	else if (ireVal == SOCKET_ERROR)
	{
		int iErrcode = WSAGetLastError();
		if (WSAEWOULDBLOCK == iErrcode)
		{
			CCsccLog::DebugPrint("接收错误！\n");
		}	
		return -1;
	}

	// 数据源IP和端口信息
	DWORD dwSourceIp = ntohl(sdClientAddr.sin_addr.S_un.S_addr);
	pstUdpPackage->abySourceIp[0] = HIBYTE(HIWORD(dwSourceIp));
	pstUdpPackage->abySourceIp[1] = LOBYTE(HIWORD(dwSourceIp));
	pstUdpPackage->abySourceIp[2] = HIBYTE(LOWORD(dwSourceIp));
	pstUdpPackage->abySourceIp[3] = LOBYTE(LOWORD(dwSourceIp));
	pstUdpPackage->uiSourcePort = ntohs(sdClientAddr.sin_port);

	// 没有对接收到的数据包排序，需要用户在调用GetPacket函数之后依据包里的帧信息自己组包
	EnterCriticalSection(&pstServerObj->crt_RcvPacket);
	pstServerObj->lstRcvPackage.AddHead(pstUdpPackage);
	LeaveCriticalSection(&pstServerObj->crt_RcvPacket);

	return ireVal;
}

/***************************************************************************
* 函数名称：[HandleIO]
* 摘 要：
* 全局影响：
* 参数：PST_THD_OBJ pstThreadObj:[in] 
*       PST_SERVER_OBJ pstServerObj:[in]
* 返回值：BOOL
*
* 修改记录：
*[日期]2011-12-08
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/
BOOL CUDPCommunicatio::HandleIO(PST_THD_OBJ pstThreadObj, PST_SERVER_OBJ pstServerObj)
{ 
	BOOL bRetVal = FALSE;

	WSANETWORKEVENTS  evntsNetWork; //网络事件结构
	if (WSAEnumNetworkEvents(pstServerObj->sktServerSocket, pstServerObj->hServerEvent, &evntsNetWork) == SOCKET_ERROR)
	{
		CCsccLog::DebugPrint("获取具体网络事件错误！\n");
	}

	do 
	{ 
		//套接字有数据可读
		if (evntsNetWork.lNetworkEvents & FD_READ)     
		{
			if (evntsNetWork.iErrorCode[FD_READ_BIT] != 0)
			{
				CCsccLog::DebugPrint("套接字可读发生网络错误！\n");
				break;
			} 
			else
			{
				//套接字有数据可读
				UDPRecv(pstServerObj);
				bRetVal = TRUE;			
			}
		} 
	} while (FALSE);

	return bRetVal;
}
/***************************************************************************
* 函数名称：[myFunction]
* 摘 要：简要描述本函数功能
* 全局影响：
* 参数：param1:[in/out] 参数作用描述
* 返回值：unsigned int 线程退出后返回TRUE
*
* 修改记录：
*[日期]
*[作者/修改者]
*[修改原因]
***************************************************************************/
unsigned int Thread_Server_Proc(LPVOID lpvoid)
{
	//线程参数
	PST_THD_OBJ pstThdObj = (PST_THD_OBJ)lpvoid;

	//取得当前线程函数所在类的对象指针
	CUDPCommunicatio *pclsUDPComObj = (CUDPCommunicatio*)(pstThdObj->lpClassObj);

	while(TRUE)
	{
		//返回事件句柄数组中第一个授信的事件相对索引号
		int iEventIndex = WSAWaitForMultipleEvents(pstThdObj->uiSocketNum+1,	//等待的网络事件句柄个数
													pstThdObj->hSelectEvents,	//等待的网络事件句柄数组
													FALSE,						//不需要所有的事件均授信
													WSA_INFINITE,				//无限等待
													FALSE);						//该模型中，此处为固定值
		//取得授信事件的绝对索引号
		iEventIndex = iEventIndex - WSA_WAIT_EVENT_0;
 
		//逐个查看是哪个事件对象授信
		for (UINT i = iEventIndex; i < (pstThdObj->uiSocketNum + 1); i++)
		{
			iEventIndex = WSAWaitForMultipleEvents(1,                           //每次查看一个网络事件
				                                   &pstThdObj->hSelectEvents[i],//网络事件对应的事件
				                                   TRUE,                        //必须等待事件全部授信
				                                   SERVER_WAIT_TIME,            //等待的时间
												   FALSE);                      //该模型中，此处为固定
		    //函数调用失败或者等待超时
			if (iEventIndex == WSA_WAIT_FAILED || iEventIndex == WSA_WAIT_TIMEOUT) 
			{
				Sleep(SERVER_WAIT_TIME);
				continue;
			} 
			else //有感兴趣的网络事件发生
			{
				if (i == 0)   //重构事件对象授信
				{
					//重构事件句柄数组，如果没有服务器I/O处理了，则退出线程
					pclsUDPComObj->ReBuildEventArry(pstThdObj);	

					if (pstThdObj->uiSocketNum == 0)
					{
						pclsUDPComObj->FreeThreadObj(pstThdObj);
						return 0;
					}

					// 将事件对象设置为无信号状态
					WSAResetEvent(pstThdObj->hSelectEvents[0]);
				} 
				else // 其它网络事件
				{
	                // 查找网络事件对应的服务器端对象
					PST_SERVER_OBJ pstServerObj = (PST_SERVER_OBJ)pclsUDPComObj->FindServerObj(pstThdObj, i);
					
					// 如果找到发生网络事件的对象，则对该事件进行处理
					if (pstServerObj != NULL)
					{
						if (!pclsUDPComObj->HandleIO(pstThdObj, pstServerObj))
						{
							pclsUDPComObj->ReBuildEventArry(pstThdObj);
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
/**************************************************************************
* 函数名称：[GetPacket]
* 摘 要：简要描述本函数功能
* 全局影响：
* 参数：param1:[in] 参数作用描述
* 返回值：
*
* 修改记录：
*[日期]
*[作者/修改者]
*[修改原因]
***************************************************************************/
BOOL CUDPCommunicatio::GetPacket(BYTE abyServerIP[IP_BYTE_LENGTH],UINT uiServerPort,ST_UDP_PACKATE &stUDPpacket)
{
	BOOL bRetVal = FALSE;

	USES_CONVERSION;

	CString strServerIp = _T("");
	strServerIp.Format(_T("%d.%d.%d.%d"), abyServerIP[0], abyServerIP[1], abyServerIP[2], abyServerIP[3]);

	ST_LOG stUdpLog;
	stUdpLog.iType = LOG_INFO;

	// 进入临界区域
	EnterCriticalSection(&m_crt_Thread);

	//遍历线程对象链表，查找服务端对象所在的线程对象
	for (POSITION posThread = m_lstThreadList.GetHeadPosition();
		 posThread != NULL;
		 m_lstThreadList.GetNext(posThread))
	{
		// 判断线程链表是否为空
		if (m_lstThreadList.IsEmpty())
		{
			// 为空就跳出循环
			break;
		}

		// 取出线程对象结构体
		PST_THD_OBJ pstThdObj = m_lstThreadList.GetAt(posThread);

		// 进入临界区域
		EnterCriticalSection(&(pstThdObj->crt_Server));

		// 遍历服务端对象链表，查找指定的服务器节点
		for (POSITION posServer = pstThdObj->lstServer.GetHeadPosition(); posServer != NULL; )
		{
			// 取出服务端对象
			PST_SERVER_OBJ pstServerObj = pstThdObj->lstServer.GetNext(posServer);

			// 服务端临时IP
			CString strTempServerIp = _T("");
			strTempServerIp.Format(_T("%d.%d.%d.%d"), pstServerObj->abyServerIp[0], pstServerObj->abyServerIp[1], pstServerObj->abyServerIp[2], pstServerObj->abyServerIp[3]);

			//如果找到了，则将其从链表中移除，并释放内存
			if ((strTempServerIp == strServerIp) && (pstServerObj->uiServerPort == uiServerPort))
			{
				// 进入临界区
				EnterCriticalSection(&(pstServerObj->crt_RcvPacket));

				// 套接字关闭。误发生网络错误时，清除服务器端对象
			    if (!(pstServerObj->lstRcvPackage.IsEmpty()))
			    {
					// 取出数据链表最后一个节点
					PST_UDP_PACKATE pstUDPPacket = pstServerObj->lstRcvPackage.GetTail();

					// 拷贝数据
					memcpy(stUDPpacket.abyData, pstUDPPacket->abyData, pstUDPPacket->uiDataLength);

					// 数据源信息
					memcpy(&stUDPpacket.abySourceIp, pstUDPPacket->abySourceIp, IP_BYTE_LENGTH);

					// 端口号
					stUDPpacket.uiSourcePort = pstUDPPacket->uiSourcePort;

					// 数据包中有效数据长度
					stUDPpacket.uiDataLength = pstUDPPacket->uiDataLength;

					// 数据包所在帧的总长度
					stUDPpacket.uiTotalDataLength = pstUDPPacket->uiTotalDataLength;

					// 数据包帧号
					stUDPpacket.uiFrameNum = pstUDPPacket->uiFrameNum;

					// 数据包包号
					stUDPpacket.uiPackageNum = pstUDPPacket->uiPackageNum;

					// 取出数据后，清除当前节点
					pstServerObj->lstRcvPackage.RemoveTail();

					// 清除数据包
					delete pstUDPPacket;
					pstUDPPacket=NULL;

					bRetVal=TRUE;
			    }
				// 离开临界区
				LeaveCriticalSection(&pstServerObj->crt_RcvPacket);	
			}
		}
		// 离开临界区
		LeaveCriticalSection(&(pstThdObj->crt_Server));
	}
	// 离开临界区
	LeaveCriticalSection(&m_crt_Thread);

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
*[作者/修改者]
*[修改原因]
***************************************************************************/
BOOL CUDPCommunicatio::AddToAcceptList(BYTE abyServerIP[IP_BYTE_LENGTH], UINT uiServerPort)
{
	if (AddServerObj(abyServerIP, uiServerPort))
	{
		return TRUE;
	}
	else
	{
		CCsccLog::DebugPrint("添加服务器对象出错！\n");
		return FALSE;
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
*[日期]
*[作者/修改者]
*[修改原因]
***************************************************************************/
BOOL CUDPCommunicatio::DeleteFromAcceptList(BYTE abyServerIP[IP_BYTE_LENGTH],UINT uiServerPort)
{
	BOOL bRetVal=FALSE;
	USES_CONVERSION;

	CString strServerIp=_T("");
	strServerIp.Format(_T("%d.%d.%d.%d"),abyServerIP[0],abyServerIP[1],abyServerIP[2],abyServerIP[3]);

	EnterCriticalSection(&m_crt_Thread);

	//遍历线程对象链表，查找服务端对象所在的线程对象
	for (POSITION posThread=m_lstThreadList.GetHeadPosition();posThread!=NULL;)
	{
		PST_THD_OBJ pstThdObj=m_lstThreadList.GetNext(posThread);
		
		EnterCriticalSection(&(pstThdObj->crt_Server));

		//遍历服务端对象链表，查找指定的服务器节点
		for (POSITION posServer=pstThdObj->lstServer.GetHeadPosition();posServer!=NULL;)
		{
			PST_SERVER_OBJ pstServerObj=pstThdObj->lstServer.GetNext(posServer);

			CString strTempServerIp=_T("");
			strTempServerIp.Format(_T("%d.%d.%d.%d"),pstServerObj->abyServerIp[0],pstServerObj->abyServerIp[1],pstServerObj->abyServerIp[2],pstServerObj->abyServerIp[3]);

			//如果找到了，则将其从链表中移除，并释放内存
			if ((strTempServerIp==strServerIp)&&(uiServerPort==pstServerObj->uiServerPort))
			{
				EnterCriticalSection(&(pstServerObj->crt_RcvPacket));

				if (!(pstServerObj->lstRcvPackage.IsEmpty()))
				{
					for (POSITION posRcvPacket=pstServerObj->lstRcvPackage.GetHeadPosition();posRcvPacket!=NULL;)
					{
						PST_UDP_PACKATE pstUDPPacket=pstServerObj->lstRcvPackage.GetNext(posRcvPacket);
						delete pstUDPPacket;
					}
					pstServerObj->lstRcvPackage.RemoveAll();
				}
				LeaveCriticalSection(&(pstServerObj->crt_RcvPacket));

				//套接字关闭、发生网络错误时，清除服务器端对象
				RemoveServerObj(pstThdObj,*pstServerObj);
				FreeServerObj(pstServerObj);

				bRetVal=TRUE;
			}
		}
		LeaveCriticalSection(&(pstThdObj->crt_Server));
	}
    LeaveCriticalSection(&m_crt_Thread);
	return bRetVal;

}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma region UDPSendFuntion

/***************************************************************************
* 函数名称：[StartUDPSender]
* 摘 要：启动UDP发送程序，内部实现套接字的创建等内容
* 全局影响：
* 参数：CString strDesIp:[in] 发送目的地IP地址
*       UINT uiDesPort:[in] 发送目的地Port
* 返回值：BOOL 启动成功，返回TRUE；否则，返回FALSE
*
* 修改记录：
*[日期]2011-12-08
*[作者/修改者] 陈洞滨
*[修改原因]
***************************************************************************/
BOOL  CUDPCommunicatio::StartUDPSender(SOCKET &sktUDPSender, BYTE abyLocalIp[IP_BYTE_LENGTH], UINT uiLocalPort)
{
	USES_CONVERSION;

	// 创建UDP发送套接字
	sktUDPSender = socket(AF_INET,      // 协议地址家族，windows下一般为AF_INET
                          SOCK_DGRAM,   // 协议套接字类型，此处UDP采用数据报型
					      IPPROTO_UDP); // 协议，此处采用UDP协议

	// 判断发送套接字是否创建成功
	if (INVALID_SOCKET == sktUDPSender)
	{
		// 得到错误代码
		int iSocketErr = WSAGetLastError();

		// 判断是否是套接字版本不正确
		if (iSocketErr == WSANOTINITIALISED)
		{  
			CCsccLog::DebugPrint("请正确加载套接字版本库！\n");
		}

		// 创建套接字失败，返回不可用的套接字
		return WSA_CREATE_SOCKET_ERROR;
	}

	int iErrCode;						// 定义错误代码
	UINT uiBuflen;						// 待设置的套接字缓冲区大小  
	UINT uiOptlen = sizeof(uiBuflen);	// 缓冲区大小

	// 获取发送缓冲区参数
	iErrCode = getsockopt(sktUDPSender,			// 套接字
		                  SOL_SOCKET,			// 套接字级别，此处选择套接字所在的应用层
		                  SO_SNDBUF,			// 将要设置的套接字选项名称，此处为缓冲区大小
		                  (char *)&uiBuflen,	// 设置的套接字选项值
						  (int*)&uiOptlen);		// 缓冲区大小

	// 判断获取缓冲区参数是否异常
	if (SOCKET_ERROR == iErrCode)                                              
	{
		CCsccLog::DebugPrint("获取发送缓冲区大小失败！\n");
		return WSA_GET_SOCKET_OPT_ERROR;
	}

	// 将原先的缓冲区大小扩大十倍，系统默认为8192byte
	uiBuflen *= 10;

	//设定缓冲区大小                                                           
	iErrCode = setsockopt(sktUDPSender, SOL_SOCKET, SO_RCVBUF, (char*)&uiBuflen, uiOptlen);

	//判断设置缓冲区大小是否异常
	if (SOCKET_ERROR == iErrCode)                                              
	{
		CCsccLog::DebugPrint("设置发送缓冲区大小失败！\n");
		return WSA_SET_SOCKET_OPT_ERROR;
	}

	//检查缓冲区大小是否修改成功
	unsigned int uiNewRcvBuf;                                               
	iErrCode = getsockopt(sktUDPSender, SOL_SOCKET, SO_SNDBUF, (char*)&uiNewRcvBuf, (int*)&uiOptlen);
	if (SOCKET_ERROR == iErrCode || uiNewRcvBuf == uiBuflen)
	{
		CCsccLog::DebugPrint("修改系统发送数据缓冲区失败！\n");
		return WSA_GET_SOCKET_OPT_ERROR;
	}

	//设置发送端端口重用
	BOOL bIsReusePort;                                                  
	int iReusePortLen = sizeof(bIsReusePort);
	iErrCode = setsockopt(sktUDPSender, SOL_SOCKET, SO_REUSEADDR, (char*)&bIsReusePort, iReusePortLen);
	if (SOCKET_ERROR == iErrCode)
	{
		CCsccLog::DebugPrint("设置端口重用失败！\n");
		return WSA_SET_SOCKET_OPT_ERROR;
	}

	// 本地IP
	CString strLocalIp = _T("");
	strLocalIp.Format(_T("%d.%d.%d.%d"), abyLocalIp[0], abyLocalIp[1], abyLocalIp[2], abyLocalIp[3]);
	char* pcLocalIp = "";

	// 编码转换(CString向LPCWSTR转换)。
	pcLocalIp = W2A(strLocalIp);

	//绑定本地IP地址
	sockaddr_in sdClientAddr;									

    //发送本机IP
	sdClientAddr.sin_addr.S_un.S_addr = inet_addr(pcLocalIp);	//TODO:此处是否需要在DLL内部自动获取本机的IP地址
	sdClientAddr.sin_family = AF_INET;							//本地地址族
	sdClientAddr.sin_port = htons(uiLocalPort);					//端口号,主机字节序转换为网络字节序，等待发送

	//绑定套接字d
	int iBindErr = bind(sktUDPSender, (LPSOCKADDR)&sdClientAddr, sizeof(sdClientAddr));
	if (iBindErr == SOCKET_ERROR)
	{
		CCsccLog::DebugPrint("发送套接字绑定本地地址失败!\n");
		return WSA_BIND_ERROR;
	}
	return TRUE;
}

/***************************************************************************
* 函数名称：[UDPSend]
* 摘 要：客户端通过UDP套接字发送数据
* 全局影响：
* 参数：byte *pbyBuffer:[in] 待发送的数据
*       UINT uiDataLength:[in] 待发送的数据长度
* 返回值：BOOL 发送成功，返回TRUE；否则，返回FALSE
*
* 修改记录：
*[日期]2011-12-08
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/
BOOL CUDPCommunicatio::UDPSend(SOCKET sktUDPSender, BYTE abyDestIp[IP_BYTE_LENGTH], UINT uiDesPort,
							   byte *pbyBuffer, UINT uiDataLength, UINT uiFrameNum)
{
	USES_CONVERSION;

	CString strDestIp =_T("");
	strDestIp.Format(_T("%d.%d.%d.%d"), abyDestIp[0], abyDestIp[1], abyDestIp[2], abyDestIp[3]);
	char* pcDesIp=W2A(strDestIp);

	sockaddr_in sdServeraddr; //目的服务器端地址簇
	sdServeraddr.sin_family = AF_INET;
	sdServeraddr.sin_addr.S_un.S_addr = inet_addr(pcDesIp);
	sdServeraddr.sin_port = htons(uiDesPort); //端口号,主机字节序转换为网络字节序，等待发送

	//开辟发送缓存区大小为MAXLENGTH字节
	PST_UDP_PACKATE pstSendPacket = new ST_UDP_PACKATE;

	//初始化发送缓存
	ZeroMemory(pstSendPacket->abyData, DATA_BUF_LENGTH);	// 数据缓存
	pstSendPacket->uiDataLength = -1;						// 数据包中数据长度
	pstSendPacket->uiFrameNum = uiFrameNum;					// 数据包帧号
	pstSendPacket->uiPackageNum = -1;						// 数据包包号
	pstSendPacket->uiTotalDataLength = uiDataLength;		// 待发送数据的总长度（由于UDP本身的限制，在发送过程中将会发生切包）
	memset(pstSendPacket->abySourceIp, 0, IP_BYTE_LENGTH);
	pstSendPacket->uiSourcePort = 0;



	//如果数据未发送完，则循环发送数据
	while (uiDataLength != 0) 
	{
		UINT uipacknum = 0;                       //用于计数数据包的个数，以填充发送的数据包包号

		//数据包总长度大于一个数据包中数据的容量
		while(uiDataLength > DATA_BUF_LENGTH ) 
		{

			// 填充数据包包号
			pstSendPacket->uiPackageNum = uipacknum;

			// 当前数据包中数据长度
			pstSendPacket->uiDataLength = DATA_BUF_LENGTH;

			// UDP包数据
			memcpy(pstSendPacket->abyData, pbyBuffer + DATA_BUF_LENGTH * uipacknum, DATA_BUF_LENGTH); //循环复制缓冲区数据到数据包结构体


			// 每次发送DATA_BUF个字节
			int reti = sendto(sktUDPSender, (const char*)pstSendPacket, sizeof(ST_UDP_PACKATE),
								0, (SOCKADDR*)&sdServeraddr, sizeof(SOCKADDR)); //发送数据包

			// 判断是否发送成功
			if (SOCKET_ERROR == reti)
			{
				CCsccLog::DebugPrint("数据发送出错！\n");
				return FALSE;
			}

			// 一帧数据剩余的数据量
			uiDataLength -= DATA_BUF_LENGTH;

			// 数据包包号加一
			uipacknum++;
		}

		if (uiDataLength>0 )//剩余字节发送
		{
			//最后一个数据包会比DATA_BUF小，故需要将内存全部格式化为0
			ZeroMemory(pstSendPacket->abyData, DATA_BUF_LENGTH); //清空缓存,使未填满的数据也初始化为0

			//填充数据包包号
			pstSendPacket->uiPackageNum = uipacknum;

			// 代码写错了
			//当前数据包中数据长度
			//pstSendPacket->uiDataLength = uiDataLength - uipacknum * DATA_BUF_LENGTH;
			pstSendPacket->uiDataLength = uiDataLength;

			// 代码写错了，总长度在开头已经被赋值
			// 数据包中数据总长度
			// pstSendPacket->uiTotalDataLength = uiDataLength;

	        //拷贝发送数据至数据包
			memcpy(pstSendPacket->abyData, pbyBuffer + uipacknum * DATA_BUF_LENGTH, uiDataLength); //将剩余数据拷贝到数据包中准备发送

			//发送最后一个数据包，数据包中的数据长度为剩余所有字节，且不大于DATA_BUF
			int reti = sendto(sktUDPSender, (const char*)pstSendPacket, sizeof(ST_UDP_PACKATE),
								0, (sockaddr*)&sdServeraddr, sizeof(sockaddr));

			//判断是否发送成功
			if (SOCKET_ERROR == reti)
			{
				CCsccLog::DebugPrint("最后一个数据包发送失败！\n");
				return FALSE;
			}

			uiDataLength = 0; //表示数据已经发送完成
			uipacknum ++;     //数据包包号加一
		}
	}

	delete pstSendPacket;  //删除数据缓存指针
	pstSendPacket =NULL;   //置空数据缓存指针

	return TRUE;
}

/***************************************************************************
* 函数名称：[EndUDPSender]
* 摘 要：本函数终止UDP程序的发送
* 全局影响：
* 参数：CString strDesIp:[in] 发送目的地IP地址
*       UINT uiDesPort:[in] 发送目的地Port
* 返回值：BOOL 启动成功，返回TRUE；否则，返回FALSE
*
* 修改记录：
*[日期]2011-12-08
*[作者/修改者] 陈洞滨
*[修改原因]
***************************************************************************/
BOOL  CUDPCommunicatio::EndUDPSender(SOCKET skSocket)
{
	//关闭套接字
	closesocket(skSocket);
	return TRUE;
}

/***************************************************************************
* 函数名称：[ReleseUDPCom]
* 摘 要：该函数删除导出函数UDPComExports中new出来的对象
* 全局影响：
* 参数：CUDPComBase*  pclsUDPComBase:[in] 基类对象
* 返回值：BOOL 清除对象成功返回TRUE,否则，返回FALSE
*
* 修改记录：
*[日期]2011-12-08
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/
BOOL CUDPCommunicatio::ReleseUDPCom(CUDPComBase*  pclsUDPComBase)
{
	//对象指针不为空，则删除；否则，直接退出
	if (pclsUDPComBase!=NULL)
	{
		delete pclsUDPComBase;	
		pclsUDPComBase=NULL;
		return TRUE;
	}
	else
	{
		return FALSE;
	}

}

#pragma endregion UDPSendFuntion

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
void CUDPCommunicatio::GetIpInfo(PST_IP_INFO pstIpInfo)
{

	int iAdapterIdx=0;					// 本机上各个网卡的索引号
	PIP_ADAPTER_INFO pAdapterInfo;		// 网卡信息结构体
	PIP_ADAPTER_INFO pAdapter = NULL;
	ULONG ulOutBufLen;					// 接收网卡信息缓冲区的大小
	pAdapterInfo= new IP_ADAPTER_INFO;  // 为网卡信息结构体分配内存
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
			IPStringtoByteArray(pAdapter->IpAddressList.IpAddress.String, pstIpInfo[iAdapterIdx].abyIPAddr);

			// 获取子网掩码信息,并存入结构体
			IPStringtoByteArray(pAdapter->IpAddressList.IpMask.String, pstIpInfo[iAdapterIdx].abySubNetMask);

			// 获取IP地址信息,并存入结构体
			IPStringtoByteArray(pAdapter->GatewayList.IpAddress.String, pstIpInfo[iAdapterIdx].abyGateWay);    

			// 计算广播地址，由位运算BroadCastAddr=IPAddr+ !(子网掩码)求得。其中+表示或，!表示非（即，取反）
			for (int i = 0; i < 4; i++)
			{
				pstIpInfo[iAdapterIdx].abyBroadCastAddr[i] =
					(pstIpInfo[iAdapterIdx].abyIPAddr[i]) | ~(pstIpInfo[iAdapterIdx].abySubNetMask[i]);
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
void CUDPCommunicatio::IPStringtoByteArray(char szIp[16], byte abyByteArray[4])
{
	CString cstrTemp = _T("");
	byte byIp;
	int iIpByteNum = 0; // IP字段号

	for(int i=0;i<16;i++)
	{
		if (szIp[i] == '.'|| szIp[i] == '\0')
		{
			if(iIpByteNum == 4)
				return;

			// 将IP的一个字段有CSting转化成为整型
			byIp = _wtoi(cstrTemp);
			
			// 保存IP当前字段
			abyByteArray[iIpByteNum] = byIp;

			// 拷贝下一个字段
			cstrTemp = _T("");

			iIpByteNum++;
		}
		else
			// 拷贝IP字段
			cstrTemp = cstrTemp + szIp[i];
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
void CUDPCommunicatio::ByteIpToStringIp(byte abyByteArray[4],char* pcStringIp)
{
	USES_CONVERSION;

	CString cstrTempIp = _T("");

	// 将BYTE型IP转化成为CString型
	cstrTempIp.Format(_T("%d.%d.%d.%d"), abyByteArray[0], abyByteArray[1], abyByteArray[2], abyByteArray[3]);

	// 将CSting型IP转化为char*型
	pcStringIp = W2A(cstrTempIp);
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
void CUDPCommunicatio::GetIpAddr(PST_IP_INFO pstIpInfo,UINT &uiIpNum)
{
	char szHostName[HOST_NAME_LENGTH]; // 主机名字

	// 获取本地主机地址
	if (gethostname(szHostName, HOST_NAME_LENGTH) == 0)
	{
		// 接收主机地址信息
		struct hostent* pstHostAddrInfo;

		// 依据主机名字获取其地址信息
		pstHostAddrInfo = gethostbyname(szHostName);

		// 针对有多个网卡的主机，依次获取它的各个IP地址
		for (UINT i = 0; pstHostAddrInfo != NULL && pstHostAddrInfo->h_addr_list[i] != NULL; i++)
		{
			BYTE szTempIpBuf[IP_BYTE_LENGTH];		// 存放IP的缓存

			memset(szTempIpBuf,0,IP_BYTE_LENGTH);	// 初始化IP缓存

			// 存放IP的四个字段
			for (int j = 0; j < pstHostAddrInfo->h_length; j++)
			{
				szTempIpBuf[j] = (BYTE)((unsigned char*)pstHostAddrInfo->h_addr_list[i])[j];
			}

			// 保存有效IP
			if (szTempIpBuf[0] != 127 && szTempIpBuf[0] != 0 && szTempIpBuf[0] < 224)
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
* 函数名称：[UDPComExports]
* 摘 要：该函数实现UDP通信类的导出，用户通过该函数取得UDP通信类的指针，进而
*        可以调用类内部的函数
* 全局影响：
* 参数：LPVOID lpvoid:[in] 
* 返回值：CUDPComBase* 基类对象指针
*
* 修改记录：
*[日期]2011-12-08
*[作者/修改者] 陈洞滨
*[修改原因]
***************************************************************************/
CUDPComBase* UDPComExports(LPVOID lpvoid)
{
	return (CUDPComBase*)new CUDPCommunicatio();
}
