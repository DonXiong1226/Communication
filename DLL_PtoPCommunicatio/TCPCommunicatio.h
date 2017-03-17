#pragma once
#include "TCPComBase.h"
#include "DataDef.h"

class CTCPCommunicatio :public CTCPComBase
{

	friend CTCPComBase* TCPComExports(LPVOID lpvoid);									// TCPͨ���ർ������
	friend unsigned int Thread_Listen_Proc(LPVOID lpvoid);								// �����̺߳���
	friend unsigned int Thread_Tcp_Client_Proc(LPVOID lpvoid);                    		// �ͻ����̴߳���������Ԫ����
	friend unsigned int Thread_Tcp_Server_Proc(LPVOID lpvoid);                    		// �������̴߳���������Ԫ����
public:
	CTCPCommunicatio(void);

	~CTCPCommunicatio(void);
private:
	CTcpClientThdObjList m_TcpClientThdList;  // �ͻ����̶߳�������

	CRITICAL_SECTION m_Crt_Thd_ClientObj;     // ���ʿͻ���ͨ���׽��ֹ����̶߳�������ؼ���

	UINT m_uiCurrentThdNum;                   // ��ǰ�ͻ����̶߳����������̶߳������

	CListenObjList m_lstListenObj;            // �����׽�������

	CRITICAL_SECTION m_Crt_ListenObj;         // ���ʼ����׽��ֶ�������ؼ���

	CCsccLog m_clsCsccLog;      // ��־�����

private:
	// ��ʼ���׽��ְ汾��
	BOOL InitialSocket();
	
	// ж���׽��ְ汾��
	BOOL UnInitialSocket();

	// ���������׽��ֶ���
	INT SetListenObj(PST_LISTEN_OBJ &pstListenObj,BYTE abyServerIP[IP_BYTE_LENGTH],UINT uiServerPort,UINT uiConnectNum = SOMAXCONN);
	
	// �ж������Ƿ��Ѿ����ڼ���״̬
	BOOL IsListen(BYTE abyServerIP[IP_BYTE_LENGTH],UINT uiServerPort);
	
	// ��������׽����Ϸ��͵������¼�
	BOOL HandleListenIO(PST_LISTEN_OBJ pstListenObj);
	
	// �ͷż����׽��ֶ���
	BOOL FreeListenObj(PST_LISTEN_OBJ pstListenObj);

	// ����һ���̶߳��󣬲�����������Ϣ
	INT SetServerThreadObj(PST_LISTEN_OBJ pstListenObj,PST_TCP_SERVER_THD_OBJ &pstTcpServerThdObj);
	
	// �ͷ��̶߳��󣬲��������̶߳����б����Ƴ�
	BOOL FreeServerThreadObj(PST_LISTEN_OBJ pstListenObj,PST_TCP_SERVER_THD_OBJ pstTcpServerThdObj);

	// �����µķ������˶���
	BOOL SetServerObj(PST_TCP_SERVER_OBJ &pstTcpServerObj,SOCKET sktAccept,BYTE abyClientIp[IP_BYTE_LENGTH],
					UINT uiClientPort,PST_LISTEN_OBJ pstListenObj);

	// ���µķ������˰��Ÿ������̴߳��� 
	BOOL AssignToFreeServerThread(PST_LISTEN_OBJ pstListenObj,PST_TCP_SERVER_OBJ pstTcpServerObj);
	
	// �������������׽��ַ�����һ�������߳�
	BOOL InsertTcpServerObj(PST_TCP_SERVER_THD_OBJ pstTcpServerThread,PST_TCP_SERVER_OBJ pstTcpServerObj);

	// �ͷŷ���������
	BOOL FreeTcpServerObj(PST_TCP_SERVER_OBJ pstTcpServerObj);
	
	// �����¼��������¼������������е�����������Ӧ���׽��ֶ���
    BOOL FindTcpServerObj(PST_TCP_SERVER_OBJ &pstTcpServerObj,PST_TCP_SERVER_THD_OBJ pstTcpServerThdObj,int iEventIndex);
	
	// ɾ�����ն�
	BOOL RemoveTcpServerObj(PST_TCP_SERVER_THD_OBJ pstTcpThreadInfo,PST_TCP_SERVER_OBJ pstTcpServerObj);
	
	// �ع��̹߳����¼��������
	BOOL ReBuildServerEventArry(PST_TCP_SERVER_THD_OBJ pstTcpServerThdObj);
	
	// ������巢���������¼�
	BOOL HandleServerIO(PST_TCP_SERVER_THD_OBJ pstTcpServerThdObj,PST_TCP_SERVER_OBJ pstTcpServerObj);
	
	// TCP��������
	BOOL TcpServerRcv(PST_TCP_SERVER_OBJ pstTcpServerObj);

	BOOL SetClientThreadObj(PST_TCP_CLIENT_THD_OBJ &pstTcpClientThdObj);

	BOOL FreeClientThreadObj(PST_TCP_CLIENT_THD_OBJ pstTcpClientThdObj);

	// ����µĽ��ն�  
	INT  AddClientObj(SOCKET &sktConnectServer,BYTE abyServerIp[IP_BYTE_LENGTH],UINT uiServerPort,
					BYTE abyClientIp[IP_BYTE_LENGTH],UINT uiClientPort);
	
	//�����µķ������˶���
	BOOL SetClientObj(PST_TCP_CLIENT_OBJ &pstTcpClientObj,SOCKET sktClinet,BYTE abyClientIp[IP_BYTE_LENGTH],UINT uiClientPort);
	
	//���µķ������˰��Ÿ������̴߳���  
	BOOL AssignToFreeClientThreadObj(PST_TCP_CLIENT_OBJ pstTcpClientObj);
	
	// �������ķ������˶�����������������
	BOOL InsertClientObj(PST_TCP_CLIENT_THD_OBJ pstTcpClientThdObj,PST_TCP_CLIENT_OBJ pstTcpClientObj);
	
	// �ع��̹߳����¼��������
	BOOL ReBuildClientEventArry(PST_TCP_CLIENT_THD_OBJ pstTcpClientThdObj);
	
	// ɾ�����ն�
	BOOL RemoveTcpClientObj(PST_TCP_CLIENT_THD_OBJ pstTcpClientThdObj,PST_TCP_CLIENT_OBJ pstTcpClientObj);
	
	// �ͷŷ���������Ϣ
	BOOL FreeClientObj(PST_TCP_CLIENT_OBJ pstTcpClientObj);
	
	// �����¼��������¼������������е�����������Ӧ���׽��ֶ���
	BOOL FindClientObj(PST_TCP_CLIENT_OBJ &pstTcpClientObj,PST_TCP_CLIENT_THD_OBJ pstTcpClientThdObj,int iEventIndex);

	BOOL HandleClientIO(PST_TCP_CLIENT_THD_OBJ pstTcpClientThdObj,PST_TCP_CLIENT_OBJ pstTcpClientObj);
	
	// TCP��������
	INT TcpClientRcv(PST_TCP_CLIENT_OBJ pstTcpClientObj);
	
	
public:// �����ĺ���
	// ��ʼ��TCPͨ����
	virtual	BOOL InitTCPCom();

	// ����ʼ��TCPͨ����
	virtual BOOL UnInitTCPCom();

	// ȡ������Ϣ
	virtual void GetIpInfo(PST_IP_INFO pstIpInfo);
	
	// ���ַ�����IP��ַ�ָ��Ϊ�ĸ��ֶΣ��������Ǵ����ֽ�����
	virtual void IPStringtoByteArray(char szIp[16],BYTE abyByteArray[IP_BYTE_LENGTH]);

	virtual void ByteIpToStringIp(byte abyByteArray[IP_BYTE_LENGTH],char* pcStringIp);

	// ��ȡ����IP��ַ
	virtual void GetIpAddr(PST_IP_INFO pstIpInfo,UINT &uiIpNum);

	// ����µĽ��ն�
	virtual BOOL BeginListen(BYTE abyServerIP[IP_BYTE_LENGTH],UINT uiServerPort);
	
	// ȡ������
	virtual BOOL GetPacketFromServer(BYTE abyClientIP[IP_BYTE_LENGTH],UINT uiClientPort,ST_TCP_SEG_PACKET &stTcpSegPacket);
	
	// ɾ����������
	virtual BOOL EndListen(BYTE abyServerIP[IP_BYTE_LENGTH],UINT uiServerPort);

	// �ͻ������ӷ�����
	virtual SOCKET ConnetServer(BYTE abyServerIp[IP_BYTE_LENGTH],unsigned int uiServerPort,
								BYTE abyClientIp[IP_BYTE_LENGTH],UINT uiClientPort);

	// TCP��������
	virtual BOOL TCPSend(SOCKET skClient,BYTE *pbyBuffer,UINT uiDataLength);
	
	// ȡ������
	virtual BOOL GetPacketFromClient(BYTE abyClientIP[IP_BYTE_LENGTH],UINT uiClientPort,ST_TCP_SEG_PACKET &stTcpSegPacket);
	
	// ֹͣTCP����
	virtual BOOL DisConnetServer(BYTE abyClientIp[IP_BYTE_LENGTH],unsigned int uiClientPorts);

	//TCPͨ�ŵ����ຯ��
	virtual BOOL ReleseTCPCom(CTCPComBase*  pclsTCPComBase);

};
