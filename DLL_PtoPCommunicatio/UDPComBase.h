#pragma once

#define  IP_BYTE_LENGTH 4
#define  NIC_NUM 1 // 机器的网卡个数
#define  HOST_NAME_LENGTH 256    // 主机名字长度
#define  MAC_BYTE_LENGTH	6       // MAC地址字节数
#define  IP_BYTE_LENGTH	4       // IP地址字节数
#define  DATA_BUF_LENGTH 4000  // 数据包存放的数据大小


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


//数据包结构体
typedef struct _NCHU_STRUCT_UDP_PACKATE
{
	BYTE abySourceIp[IP_BYTE_LENGTH];	// 数据源地址
	UINT uiSourcePort;					// 数据源端口
	UINT uiFrameNum;					// 帧号
	UINT uiPackageNum;					// 包号
	byte abyData[DATA_BUF_LENGTH];		// 数据缓存
	UINT uiDataLength;					// 包中数据长度
	UINT uiTotalDataLength;				// 待发送数据的总长度

}ST_UDP_PACKATE,*PST_UDP_PACKATE;


class CUDPComBase
{
protected:
	CUDPComBase(void);
public:
	virtual ~CUDPComBase(void);

public:
	virtual	BOOL InitUDPCom()=0;                                                        		// 初始化UDP通信类
	virtual BOOL UnInitUDPCom()=0;                                                      		// 反初始化UDP通信类
public:
	virtual void GetIpInfo(PST_IP_INFO pstIpInfo)=0;     // 取网卡信息
	virtual void IPStringtoByteArray(char szIp[16],byte abyByteArray[IP_BYTE_LENGTH])=0; // 将字符串型IP地址分割成为四个字段，并将它们存入字节数组
	virtual void ByteIpToStringIp(byte abyByteArray[IP_BYTE_LENGTH],char* pcStringIp)=0;
	virtual void GetIpAddr(PST_IP_INFO pstIpInfo,UINT &uiIpNum)=0;  // 获取主机IP地址
public:
	virtual BOOL AddToAcceptList(BYTE abyServerIP[IP_BYTE_LENGTH],UINT uiServerPort)=0;               	    	// 添加新的接收端
	virtual BOOL GetPacket(BYTE abyServerIP[IP_BYTE_LENGTH],UINT uiServerPort,ST_UDP_PACKATE &stUDPpacket)=0;     // 取出数据
	virtual BOOL DeleteFromAcceptList(BYTE abyServerIP[IP_BYTE_LENGTH],UINT uiServerPort)=0;          	    	// 删除服务器端
public:
	virtual BOOL StartUDPSender(SOCKET &sktUDPSender,BYTE abyLocalIp[IP_BYTE_LENGTH],UINT uiLocalPort)=0;          // 启动UDP发送程序
	virtual BOOL UDPSend(SOCKET sktUDPSender,BYTE abyDestIp[IP_BYTE_LENGTH],UINT uiDesPort,
		                 byte *pbyBuffer,UINT uiDataLength,UINT uiFrameNum)=0;           		// UDP发送数据
	virtual BOOL EndUDPSender(SOCKET skSocket)=0;                                        		// 停止UDP发送
public:
	virtual BOOL ReleseUDPCom(CUDPComBase*  pclsUDPComBase)=0;
};


CUDPComBase* UDPComExports(LPVOID lpvoid);                                                     //UDP通信类导出函数
