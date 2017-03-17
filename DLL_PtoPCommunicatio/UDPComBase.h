#pragma once

#define  IP_BYTE_LENGTH 4
#define  NIC_NUM 1 // ��������������
#define  HOST_NAME_LENGTH 256    // �������ֳ���
#define  MAC_BYTE_LENGTH	6       // MAC��ַ�ֽ���
#define  IP_BYTE_LENGTH	4       // IP��ַ�ֽ���
#define  DATA_BUF_LENGTH 4000  // ���ݰ���ŵ����ݴ�С


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


//���ݰ��ṹ��
typedef struct _NCHU_STRUCT_UDP_PACKATE
{
	BYTE abySourceIp[IP_BYTE_LENGTH];	// ����Դ��ַ
	UINT uiSourcePort;					// ����Դ�˿�
	UINT uiFrameNum;					// ֡��
	UINT uiPackageNum;					// ����
	byte abyData[DATA_BUF_LENGTH];		// ���ݻ���
	UINT uiDataLength;					// �������ݳ���
	UINT uiTotalDataLength;				// ���������ݵ��ܳ���

}ST_UDP_PACKATE,*PST_UDP_PACKATE;


class CUDPComBase
{
protected:
	CUDPComBase(void);
public:
	virtual ~CUDPComBase(void);

public:
	virtual	BOOL InitUDPCom()=0;                                                        		// ��ʼ��UDPͨ����
	virtual BOOL UnInitUDPCom()=0;                                                      		// ����ʼ��UDPͨ����
public:
	virtual void GetIpInfo(PST_IP_INFO pstIpInfo)=0;     // ȡ������Ϣ
	virtual void IPStringtoByteArray(char szIp[16],byte abyByteArray[IP_BYTE_LENGTH])=0; // ���ַ�����IP��ַ�ָ��Ϊ�ĸ��ֶΣ��������Ǵ����ֽ�����
	virtual void ByteIpToStringIp(byte abyByteArray[IP_BYTE_LENGTH],char* pcStringIp)=0;
	virtual void GetIpAddr(PST_IP_INFO pstIpInfo,UINT &uiIpNum)=0;  // ��ȡ����IP��ַ
public:
	virtual BOOL AddToAcceptList(BYTE abyServerIP[IP_BYTE_LENGTH],UINT uiServerPort)=0;               	    	// ����µĽ��ն�
	virtual BOOL GetPacket(BYTE abyServerIP[IP_BYTE_LENGTH],UINT uiServerPort,ST_UDP_PACKATE &stUDPpacket)=0;     // ȡ������
	virtual BOOL DeleteFromAcceptList(BYTE abyServerIP[IP_BYTE_LENGTH],UINT uiServerPort)=0;          	    	// ɾ����������
public:
	virtual BOOL StartUDPSender(SOCKET &sktUDPSender,BYTE abyLocalIp[IP_BYTE_LENGTH],UINT uiLocalPort)=0;          // ����UDP���ͳ���
	virtual BOOL UDPSend(SOCKET sktUDPSender,BYTE abyDestIp[IP_BYTE_LENGTH],UINT uiDesPort,
		                 byte *pbyBuffer,UINT uiDataLength,UINT uiFrameNum)=0;           		// UDP��������
	virtual BOOL EndUDPSender(SOCKET skSocket)=0;                                        		// ֹͣUDP����
public:
	virtual BOOL ReleseUDPCom(CUDPComBase*  pclsUDPComBase)=0;
};


CUDPComBase* UDPComExports(LPVOID lpvoid);                                                     //UDPͨ���ർ������
