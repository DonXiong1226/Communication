#pragma once

#define  NIC_NUM 1 // ��������������
#define HOST_NAME_LENGTH 256    // �������ֳ���
#define MAC_BYTE_LENGTH	6       // MAC��ַ�ֽ���
#define IP_BYTE_LENGTH	4       // IP��ַ�ֽ���


#ifndef IP_INFO
#define IP_INFO 1
#else
#define  IP_INFO 0
#endif

#if IP_INFO
typedef struct _NCHU_STRUCT_IP_INFO//IP��ַ��Ϣ
{
	BYTE abyIPAddr[IP_BYTE_LENGTH];    // IP��ַ
	BYTE abySubNetMask[IP_BYTE_LENGTH];// ��������
	BYTE abyGateWay[IP_BYTE_LENGTH];   // ����
	BYTE abyMacAddr[MAC_BYTE_LENGTH];  // MAC��ַ
	BYTE abyBroadCastAddr[IP_BYTE_LENGTH]; // �㲥��ַ
}ST_IP_INFO, *PST_IP_INFO;
#endif

//TCP���ݰ���ṹ��
typedef struct _NCHU_STRUCT_TCP_SEG_PACKET
{
	UINT uiTotalDataLength;  // �����ܳ���
	UINT uiDataLength;       // ��ǰ�������ݳ���
	BYTE* pbyRcvData;        // ���յ�����
	BYTE abySrcIp[IP_BYTE_LENGTH];           // ����ԴIP
	UINT  uiSrcPort;         // ����Դ�˿�
}ST_TCP_SEG_PACKET,*PST_TCP_SEG_PACKET;



class CTCPComBase
{
protected: 
	CTCPComBase(void);
public:
	virtual ~CTCPComBase(void);

public:
	virtual	BOOL InitTCPCom()=0;                                                        		// ��ʼ��TCPͨ����
	virtual BOOL UnInitTCPCom()=0;                                                      		// ����ʼ��TCPͨ����
public:
	virtual void GetIpInfo(PST_IP_INFO pstIpInfo)=0;     // ȡ������Ϣ
	virtual void IPStringtoByteArray(char szIp[16],byte abyByteArray[IP_BYTE_LENGTH])=0; // ���ַ�����IP��ַ�ָ��Ϊ�ĸ��ֶΣ��������Ǵ����ֽ�����
	virtual void ByteIpToStringIp(byte abyByteArray[IP_BYTE_LENGTH],char* pcStringIp)=0;
	virtual void GetIpAddr(PST_IP_INFO pstIpInfo,UINT &uiIpNum)=0;  // ��ȡ����IP��ַ
public:
	virtual BOOL BeginListen(BYTE abyServerIP[IP_BYTE_LENGTH],UINT uiServerPort)=0;               	    	    // ����µĽ��ն�
	virtual BOOL GetPacketFromServer(BYTE abyClientIP[IP_BYTE_LENGTH],UINT uiClientPort,ST_TCP_SEG_PACKET &stTcpSegPacket)=0;     // ȡ������
	virtual BOOL EndListen(BYTE abyServerIP[IP_BYTE_LENGTH],UINT uiServerPort)=0;          	    	            // ɾ����������
public:
	virtual SOCKET ConnetServer(BYTE abyServerIp[IP_BYTE_LENGTH],unsigned int uiServerPort,BYTE abyClientIp[IP_BYTE_LENGTH],UINT uiClientPort)=0;                    // �ͻ������ӷ�����
	virtual BOOL TCPSend(SOCKET skClient,BYTE *pbyBuffer,UINT uiDataLength)=0;    // TCP��������
	virtual BOOL GetPacketFromClient(BYTE abyClientIP[IP_BYTE_LENGTH],UINT uiClientPort,ST_TCP_SEG_PACKET &stTcpSegPacket)=0;     // ȡ������
	virtual BOOL DisConnetServer(BYTE abyClientIp[IP_BYTE_LENGTH],unsigned int uiClientPort)=0;                 // ֹͣTCP����

public:
	virtual BOOL ReleseTCPCom(CTCPComBase*  pclsTCPComBase)=0;
};


CTCPComBase* TCPComExports(LPVOID lpvoid);                                                     //TCPͨ���ർ������