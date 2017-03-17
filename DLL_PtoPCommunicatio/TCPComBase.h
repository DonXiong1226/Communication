#pragma once

#define  NIC_NUM 1 // 机器的网卡个数
#define HOST_NAME_LENGTH 256    // 主机名字长度
#define MAC_BYTE_LENGTH	6       // MAC地址字节数
#define IP_BYTE_LENGTH	4       // IP地址字节数


#ifndef IP_INFO
#define IP_INFO 1
#else
#define  IP_INFO 0
#endif

#if IP_INFO
typedef struct _NCHU_STRUCT_IP_INFO//IP地址信息
{
	BYTE abyIPAddr[IP_BYTE_LENGTH];    // IP地址
	BYTE abySubNetMask[IP_BYTE_LENGTH];// 子网掩码
	BYTE abyGateWay[IP_BYTE_LENGTH];   // 网关
	BYTE abyMacAddr[MAC_BYTE_LENGTH];  // MAC地址
	BYTE abyBroadCastAddr[IP_BYTE_LENGTH]; // 广播地址
}ST_IP_INFO, *PST_IP_INFO;
#endif

//TCP数据包体结构体
typedef struct _NCHU_STRUCT_TCP_SEG_PACKET
{
	UINT uiTotalDataLength;  // 数据总长度
	UINT uiDataLength;       // 当前包中数据长度
	BYTE* pbyRcvData;        // 接收的数据
	BYTE abySrcIp[IP_BYTE_LENGTH];           // 数据源IP
	UINT  uiSrcPort;         // 数据源端口
}ST_TCP_SEG_PACKET,*PST_TCP_SEG_PACKET;



class CTCPComBase
{
protected: 
	CTCPComBase(void);
public:
	virtual ~CTCPComBase(void);

public:
	virtual	BOOL InitTCPCom()=0;                                                        		// 初始化TCP通信类
	virtual BOOL UnInitTCPCom()=0;                                                      		// 反初始化TCP通信类
public:
	virtual void GetIpInfo(PST_IP_INFO pstIpInfo)=0;     // 取网卡信息
	virtual void IPStringtoByteArray(char szIp[16],byte abyByteArray[IP_BYTE_LENGTH])=0; // 将字符串型IP地址分割成为四个字段，并将它们存入字节数组
	virtual void ByteIpToStringIp(byte abyByteArray[IP_BYTE_LENGTH],char* pcStringIp)=0;
	virtual void GetIpAddr(PST_IP_INFO pstIpInfo,UINT &uiIpNum)=0;  // 获取主机IP地址
public:
	virtual BOOL BeginListen(BYTE abyServerIP[IP_BYTE_LENGTH],UINT uiServerPort)=0;               	    	    // 添加新的接收端
	virtual BOOL GetPacketFromServer(BYTE abyClientIP[IP_BYTE_LENGTH],UINT uiClientPort,ST_TCP_SEG_PACKET &stTcpSegPacket)=0;     // 取出数据
	virtual BOOL EndListen(BYTE abyServerIP[IP_BYTE_LENGTH],UINT uiServerPort)=0;          	    	            // 删除服务器端
public:
	virtual SOCKET ConnetServer(BYTE abyServerIp[IP_BYTE_LENGTH],unsigned int uiServerPort,BYTE abyClientIp[IP_BYTE_LENGTH],UINT uiClientPort)=0;                    // 客户端连接服务器
	virtual BOOL TCPSend(SOCKET skClient,BYTE *pbyBuffer,UINT uiDataLength)=0;    // TCP发送数据
	virtual BOOL GetPacketFromClient(BYTE abyClientIP[IP_BYTE_LENGTH],UINT uiClientPort,ST_TCP_SEG_PACKET &stTcpSegPacket)=0;     // 取出数据
	virtual BOOL DisConnetServer(BYTE abyClientIp[IP_BYTE_LENGTH],unsigned int uiClientPort)=0;                 // 停止TCP发送

public:
	virtual BOOL ReleseTCPCom(CTCPComBase*  pclsTCPComBase)=0;
};


CTCPComBase* TCPComExports(LPVOID lpvoid);                                                     //TCP通信类导出函数