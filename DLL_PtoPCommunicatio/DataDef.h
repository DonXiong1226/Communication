#pragma once


#include "UDPComBase.h"
#include "TCPComBase.h"

#define MAX_FILE_PATH	 256
#define MAX_LOG_LENGTH	 512
#define SERVER_WAIT_TIME 100   // 服务器等待时间
#define RECV_BUF 2048           // 接收缓存大小
#define WSA_IS_MEMBER 2           // 错误代码
#define WSA_CREATE_SOCKET_ERROR  3 // 创建套接字错误码
#define WSA_CONNECT_ERROR 4    //客户端连接服务器错误码
#define WSA_BIND_ERROR 5 // 绑定套接字错误码
#define WSA_SET_SOCKET_OPT_ERROR 6
#define WSA_GET_SOCKET_OPT_ERROR 7
#define WSA_RCV_DATA_ERROR 8


//UDP数据链表
typedef CTypedPtrList <CPtrList,PST_UDP_PACKATE> CRecvPackageList;


//接收数据指针链表类
//typedef CTypedPtrList<CPtrList,PST_UDP_PACKATE> CPackageList;

//接收端端对象
typedef struct _NCHU_STRUCT_SERVER_OBJ
{
	UINT    uiServerID;					// 服务器端索引
	SOCKET  sktServerSocket;			// 服务器端套接字
	BYTE abyServerIp[IP_BYTE_LENGTH];	// 接收端IP地址
	UINT uiServerPort;					// 接收端端口
	WSAEVENT  hServerEvent;				// 与服务器端套接字关联的事件对象句柄
	CRITICAL_SECTION crt_RcvPacket;		// 访问接收数据包链表关键代码段
    CRecvPackageList lstRcvPackage;		// 接收数据链表，表中数据未组包

}ST_SERVER_OBJ,*PST_SERVER_OBJ;

//接收端链表类
typedef CTypedPtrList<CPtrList,PST_SERVER_OBJ> CServerObjList;


//线程对象结构体，每个线程对象中包含一个线程，该线程最多可以管理64个套接字
//对于UDP而言，就是64个接收端；对于TCP而言就是与当前服务器连接的客户端个数
typedef struct _NCHU_STRUCT_THD_OBJ
{
	UINT    uiThreadID;                                  // 线程索引
	CWinThread* pThread;                                 // 线程对象句柄
	HANDLE	hEventToExit;	                             // 要求退出事件
	LPVOID  lpClassObj;                                  // 线程函数所在类的对象指针
	WSAEVENT  hSelectEvents[WSA_MAXIMUM_WAIT_EVENTS];    // 当前线程管理的事件对象句柄数组
	UINT uiSocketNum;                                    // 当前线程管理的套接字个数
	CRITICAL_SECTION crt_Server;                         // 访问服务器端链表关键段
	CServerObjList lstServer;                            // 本线程对应的服务器端对象链表

}ST_THD_OBJ,*PST_THD_OBJ;


//线程对象链表
typedef CTypedPtrList<CPtrList, PST_THD_OBJ> CThreadObjList;

//_________________________________TCP通信采用的结构体____________________________________________

//TCP数据包头
typedef struct _NCHU_STRUCT_TCP_PACKET_HEAD
{
	UINT uiTotalDataLength;  // 数据总长度
	UINT uiDataLength;       // 当前包中数据长度

}ST_TCP_PACKET_HEAD,*PST_TCP_PACKET_HEAD;

//TCP接收数据链表，每个节点时没有组包的数据
typedef CTypedPtrList<CPtrList,PST_TCP_SEG_PACKET> CSegPackageList;

//服务器端对象
typedef struct _NCHU_STRUCT_TCP_SERVER_OBJ
{
	UINT    uiServerIdx;                  // 服务器端索引
	SOCKET  sktAccept;                    // 服务器与客户端连接套接字
	BYTE    abyClientIp[IP_BYTE_LENGTH];  // 服务器连接的客户端IP地址
	UINT    uiClientPort;                 // 服务器连接的客户端端口
	WSAEVENT  hAcceptEvent;               // 与服务器端套接字关联的事件对象句柄
	CRITICAL_SECTION crt_RcvPacket;       // TCP接收数据链表临界区
	CSegPackageList lstSegPacket;         // TCP数据接收链表
}ST_TCP_SERVER_OBJ,*PST_TCP_SERVER_OBJ;

// TCP服务器端链表
typedef CTypedPtrList<CPtrList,PST_TCP_SERVER_OBJ> CTcpServerObjList;

typedef struct _NCHU_STRUCT_TCP_SERVER_THD_OBJ
{
	UINT    uiThreadIdx;                                 // 线程索引
	CWinThread* pThread;                                 // 线程对象句柄
	HANDLE hThreadExit;                                  // 线程退出事件
	LPVOID  lpClassObj;                                  // 线程函数所在类的对象指针
	LPVOID  lpListenObj;                                 // 对应的监听套接字
	WSAEVENT  hSelectEvents[WSA_MAXIMUM_WAIT_EVENTS];    // 当前线程管理的事件对象句柄数组
	UINT uiSocketNum;                                    // 当前线程管理的套接字个数
	CRITICAL_SECTION crt_Tcp_Server;                     // 访问服务器端链表关键段
	CTcpServerObjList lstTcpServer;                      // 本线程对应的服务器端对象链表

}ST_TCP_SERVER_THD_OBJ,*PST_TCP_SERVER_THD_OBJ;
typedef CTypedPtrList<CPtrList,PST_TCP_SERVER_THD_OBJ>CTcpServerThdObjList;

typedef struct _NCHU_STRUCT_TCP_CLIENT_OBJ
{
	UINT uiClientIdx;                 // 客户端在线程对象中的索引
	SOCKET sktClient;                 // 客户端通信套接字
	BYTE abyClientIp[IP_BYTE_LENGTH]; // 客户端IP地址
	UINT uiClientPort;                // 客户端端口号
	WSAEVENT hClientEvent;            // 客户端套接字关联的事件对象
	CRITICAL_SECTION crt_RcvPacket;   // 客户端接收数据临界区
	CSegPackageList lstSegPacket;     // 客户端接收的数据链表
}ST_TCP_CLIENT_OBJ,*PST_TCP_CLIENT_OBJ;
// TCP客户端链表
typedef CTypedPtrList<CPtrList,PST_TCP_CLIENT_OBJ> CTcpClientObjList;

typedef struct _NCHU_STRUCT_TCP_CLIENT_THD_OBJ
{
	UINT    uiThreadIdx;                                 // 线程索引
	CWinThread* pThread;                                 // 线程对象句柄
	LPVOID  lpClassObj;                                  // 线程函数所在类的对象指针
	WSAEVENT  hSelectEvents[WSA_MAXIMUM_WAIT_EVENTS];    // 当前线程管理的事件对象句柄数组
	UINT uiSocketNum;                                    // 当前线程管理的套接字个数
	CRITICAL_SECTION crt_Tcp_Client;                     // 访问服务器端链表关键段
	CTcpClientObjList lstTcpClient;                      // 本线程对应的服务器端对象链表

}ST_TCP_CLIENT_THD_OBJ,*PST_TCP_CLIENT_THD_OBJ;
typedef CTypedPtrList<CPtrList,PST_TCP_CLIENT_THD_OBJ> CTcpClientThdObjList;

//监听套接字对象结构体
typedef struct _NCHU_STRUCT_LISTEN_OBJ
{
	UINT  uiCurrentThdNum;						// 当前线程对象链表中线程对象个数
	CRITICAL_SECTION crt_ThreadObj;				// 访问线程对象链表关键代码段
	CTcpServerThdObjList   lstTcpServerThdObj;  // 线程对象链表
	SOCKET sktListen;							// 监听套接字
	BYTE abyServerIp[IP_BYTE_LENGTH];			// 服务器监听地址
	UINT uiServerPort;							// 服务器监听端口
	WSAEVENT hListenEvent;						// 监听套接字绑定的事件句柄
	CWinThread* pclsThdListen;					// 处理监听套接字网络事件线程
	LPVOID lpClsObj;							// 类对象指针
	HANDLE hServerExit;							// 服务器退出监听事件
	HANDLE hServerHasExit;						// 服务器监听线程已经退出
}ST_LISTEN_OBJ,*PST_LISTEN_OBJ;

//监听套接字链表
typedef CTypedPtrList<CPtrList,PST_LISTEN_OBJ> CListenObjList;















