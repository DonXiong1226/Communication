#pragma once
#include "TCPComBase.h"
#include "DataDef.h"

class CTCPCommunicatio :public CTCPComBase
{

	friend CTCPComBase* TCPComExports(LPVOID lpvoid);									// TCP通信类导出函数
	friend unsigned int Thread_Listen_Proc(LPVOID lpvoid);								// 监听线程函数
	friend unsigned int Thread_Tcp_Client_Proc(LPVOID lpvoid);                    		// 客户端线程处理函数，友元函数
	friend unsigned int Thread_Tcp_Server_Proc(LPVOID lpvoid);                    		// 服务器线程处理函数，友元函数
public:
	CTCPCommunicatio(void);

	~CTCPCommunicatio(void);
private:
	CTcpClientThdObjList m_TcpClientThdList;  // 客户端线程对象链表

	CRITICAL_SECTION m_Crt_Thd_ClientObj;     // 访问客户端通信套接字管理线程对象链表关键段

	UINT m_uiCurrentThdNum;                   // 当前客户端线程对象链表中线程对象个数

	CListenObjList m_lstListenObj;            // 监听套接字链表

	CRITICAL_SECTION m_Crt_ListenObj;         // 访问监听套接字对象链表关键段

	CCsccLog m_clsCsccLog;      // 日志类对象

private:
	// 初始化套接字版本库
	BOOL InitialSocket();
	
	// 卸载套接字版本库
	BOOL UnInitialSocket();

	// 创建监听套接字对象
	INT SetListenObj(PST_LISTEN_OBJ &pstListenObj,BYTE abyServerIP[IP_BYTE_LENGTH],UINT uiServerPort,UINT uiConnectNum = SOMAXCONN);
	
	// 判断主机是否已经处于监听状态
	BOOL IsListen(BYTE abyServerIP[IP_BYTE_LENGTH],UINT uiServerPort);
	
	// 处理监听套接字上发送的网络事件
	BOOL HandleListenIO(PST_LISTEN_OBJ pstListenObj);
	
	// 释放监听套接字对象
	BOOL FreeListenObj(PST_LISTEN_OBJ pstListenObj);

	// 创建一个线程对象，并设置它的信息
	INT SetServerThreadObj(PST_LISTEN_OBJ pstListenObj,PST_TCP_SERVER_THD_OBJ &pstTcpServerThdObj);
	
	// 释放线程对象，并将它从线程对象列表中移除
	BOOL FreeServerThreadObj(PST_LISTEN_OBJ pstListenObj,PST_TCP_SERVER_THD_OBJ pstTcpServerThdObj);

	// 创建新的服务器端对象
	BOOL SetServerObj(PST_TCP_SERVER_OBJ &pstTcpServerObj,SOCKET sktAccept,BYTE abyClientIp[IP_BYTE_LENGTH],
					UINT uiClientPort,PST_LISTEN_OBJ pstListenObj);

	// 将新的服务器端安排给空闲线程处理 
	BOOL AssignToFreeServerThread(PST_LISTEN_OBJ pstListenObj,PST_TCP_SERVER_OBJ pstTcpServerObj);
	
	// 将新增的连接套接字分配至一个空闲线程
	BOOL InsertTcpServerObj(PST_TCP_SERVER_THD_OBJ pstTcpServerThread,PST_TCP_SERVER_OBJ pstTcpServerObj);

	// 释放服务器对象
	BOOL FreeTcpServerObj(PST_TCP_SERVER_OBJ pstTcpServerObj);
	
	// 依据事件对象在事件对象句柄数组中的索引查找响应的套接字对象
    BOOL FindTcpServerObj(PST_TCP_SERVER_OBJ &pstTcpServerObj,PST_TCP_SERVER_THD_OBJ pstTcpServerThdObj,int iEventIndex);
	
	// 删除接收端
	BOOL RemoveTcpServerObj(PST_TCP_SERVER_THD_OBJ pstTcpThreadInfo,PST_TCP_SERVER_OBJ pstTcpServerObj);
	
	// 重构线程管理事件句柄数组
	BOOL ReBuildServerEventArry(PST_TCP_SERVER_THD_OBJ pstTcpServerThdObj);
	
	// 处理具体发生的网络事件
	BOOL HandleServerIO(PST_TCP_SERVER_THD_OBJ pstTcpServerThdObj,PST_TCP_SERVER_OBJ pstTcpServerObj);
	
	// TCP接收数据
	BOOL TcpServerRcv(PST_TCP_SERVER_OBJ pstTcpServerObj);

	BOOL SetClientThreadObj(PST_TCP_CLIENT_THD_OBJ &pstTcpClientThdObj);

	BOOL FreeClientThreadObj(PST_TCP_CLIENT_THD_OBJ pstTcpClientThdObj);

	// 添加新的接收端  
	INT  AddClientObj(SOCKET &sktConnectServer,BYTE abyServerIp[IP_BYTE_LENGTH],UINT uiServerPort,
					BYTE abyClientIp[IP_BYTE_LENGTH],UINT uiClientPort);
	
	//创建新的服务器端对象
	BOOL SetClientObj(PST_TCP_CLIENT_OBJ &pstTcpClientObj,SOCKET sktClinet,BYTE abyClientIp[IP_BYTE_LENGTH],UINT uiClientPort);
	
	//将新的服务器端安排给空闲线程处理  
	BOOL AssignToFreeClientThreadObj(PST_TCP_CLIENT_OBJ pstTcpClientObj);
	
	// 将新增的服务器端对象加入服务器端链表
	BOOL InsertClientObj(PST_TCP_CLIENT_THD_OBJ pstTcpClientThdObj,PST_TCP_CLIENT_OBJ pstTcpClientObj);
	
	// 重构线程管理事件句柄数组
	BOOL ReBuildClientEventArry(PST_TCP_CLIENT_THD_OBJ pstTcpClientThdObj);
	
	// 删除接收端
	BOOL RemoveTcpClientObj(PST_TCP_CLIENT_THD_OBJ pstTcpClientThdObj,PST_TCP_CLIENT_OBJ pstTcpClientObj);
	
	// 释放服务器端信息
	BOOL FreeClientObj(PST_TCP_CLIENT_OBJ pstTcpClientObj);
	
	// 依据事件对象在事件对象句柄数组中的索引查找响应的套接字对象
	BOOL FindClientObj(PST_TCP_CLIENT_OBJ &pstTcpClientObj,PST_TCP_CLIENT_THD_OBJ pstTcpClientThdObj,int iEventIndex);

	BOOL HandleClientIO(PST_TCP_CLIENT_THD_OBJ pstTcpClientThdObj,PST_TCP_CLIENT_OBJ pstTcpClientObj);
	
	// TCP接收数据
	INT TcpClientRcv(PST_TCP_CLIENT_OBJ pstTcpClientObj);
	
	
public:// 导出的函数
	// 初始化TCP通信类
	virtual	BOOL InitTCPCom();

	// 反初始化TCP通信类
	virtual BOOL UnInitTCPCom();

	// 取网卡信息
	virtual void GetIpInfo(PST_IP_INFO pstIpInfo);
	
	// 将字符串型IP地址分割成为四个字段，并将它们存入字节数组
	virtual void IPStringtoByteArray(char szIp[16],BYTE abyByteArray[IP_BYTE_LENGTH]);

	virtual void ByteIpToStringIp(byte abyByteArray[IP_BYTE_LENGTH],char* pcStringIp);

	// 获取主机IP地址
	virtual void GetIpAddr(PST_IP_INFO pstIpInfo,UINT &uiIpNum);

	// 添加新的接收端
	virtual BOOL BeginListen(BYTE abyServerIP[IP_BYTE_LENGTH],UINT uiServerPort);
	
	// 取出数据
	virtual BOOL GetPacketFromServer(BYTE abyClientIP[IP_BYTE_LENGTH],UINT uiClientPort,ST_TCP_SEG_PACKET &stTcpSegPacket);
	
	// 删除服务器端
	virtual BOOL EndListen(BYTE abyServerIP[IP_BYTE_LENGTH],UINT uiServerPort);

	// 客户端连接服务器
	virtual SOCKET ConnetServer(BYTE abyServerIp[IP_BYTE_LENGTH],unsigned int uiServerPort,
								BYTE abyClientIp[IP_BYTE_LENGTH],UINT uiClientPort);

	// TCP发送数据
	virtual BOOL TCPSend(SOCKET skClient,BYTE *pbyBuffer,UINT uiDataLength);
	
	// 取出数据
	virtual BOOL GetPacketFromClient(BYTE abyClientIP[IP_BYTE_LENGTH],UINT uiClientPort,ST_TCP_SEG_PACKET &stTcpSegPacket);
	
	// 停止TCP发送
	virtual BOOL DisConnetServer(BYTE abyClientIp[IP_BYTE_LENGTH],unsigned int uiClientPorts);

	//TCP通信导出类函数
	virtual BOOL ReleseTCPCom(CTCPComBase*  pclsTCPComBase);

};
