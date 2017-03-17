#pragma once


#include "UDPComBase.h"
#include "TCPComBase.h"

#define MAX_FILE_PATH	 256
#define MAX_LOG_LENGTH	 512
#define SERVER_WAIT_TIME 100   // �������ȴ�ʱ��
#define RECV_BUF 2048           // ���ջ����С
#define WSA_IS_MEMBER 2           // �������
#define WSA_CREATE_SOCKET_ERROR  3 // �����׽��ִ�����
#define WSA_CONNECT_ERROR 4    //�ͻ������ӷ�����������
#define WSA_BIND_ERROR 5 // ���׽��ִ�����
#define WSA_SET_SOCKET_OPT_ERROR 6
#define WSA_GET_SOCKET_OPT_ERROR 7
#define WSA_RCV_DATA_ERROR 8


//UDP��������
typedef CTypedPtrList <CPtrList,PST_UDP_PACKATE> CRecvPackageList;


//��������ָ��������
//typedef CTypedPtrList<CPtrList,PST_UDP_PACKATE> CPackageList;

//���ն˶˶���
typedef struct _NCHU_STRUCT_SERVER_OBJ
{
	UINT    uiServerID;					// ������������
	SOCKET  sktServerSocket;			// ���������׽���
	BYTE abyServerIp[IP_BYTE_LENGTH];	// ���ն�IP��ַ
	UINT uiServerPort;					// ���ն˶˿�
	WSAEVENT  hServerEvent;				// ����������׽��ֹ������¼�������
	CRITICAL_SECTION crt_RcvPacket;		// ���ʽ������ݰ�����ؼ������
    CRecvPackageList lstRcvPackage;		// ��������������������δ���

}ST_SERVER_OBJ,*PST_SERVER_OBJ;

//���ն�������
typedef CTypedPtrList<CPtrList,PST_SERVER_OBJ> CServerObjList;


//�̶߳���ṹ�壬ÿ���̶߳����а���һ���̣߳����߳������Թ���64���׽���
//����UDP���ԣ�����64�����նˣ�����TCP���Ծ����뵱ǰ���������ӵĿͻ��˸���
typedef struct _NCHU_STRUCT_THD_OBJ
{
	UINT    uiThreadID;                                  // �߳�����
	CWinThread* pThread;                                 // �̶߳�����
	HANDLE	hEventToExit;	                             // Ҫ���˳��¼�
	LPVOID  lpClassObj;                                  // �̺߳���������Ķ���ָ��
	WSAEVENT  hSelectEvents[WSA_MAXIMUM_WAIT_EVENTS];    // ��ǰ�̹߳�����¼�����������
	UINT uiSocketNum;                                    // ��ǰ�̹߳�����׽��ָ���
	CRITICAL_SECTION crt_Server;                         // ���ʷ�����������ؼ���
	CServerObjList lstServer;                            // ���̶߳�Ӧ�ķ������˶�������

}ST_THD_OBJ,*PST_THD_OBJ;


//�̶߳�������
typedef CTypedPtrList<CPtrList, PST_THD_OBJ> CThreadObjList;

//_________________________________TCPͨ�Ų��õĽṹ��____________________________________________

//TCP���ݰ�ͷ
typedef struct _NCHU_STRUCT_TCP_PACKET_HEAD
{
	UINT uiTotalDataLength;  // �����ܳ���
	UINT uiDataLength;       // ��ǰ�������ݳ���

}ST_TCP_PACKET_HEAD,*PST_TCP_PACKET_HEAD;

//TCP������������ÿ���ڵ�ʱû�����������
typedef CTypedPtrList<CPtrList,PST_TCP_SEG_PACKET> CSegPackageList;

//�������˶���
typedef struct _NCHU_STRUCT_TCP_SERVER_OBJ
{
	UINT    uiServerIdx;                  // ������������
	SOCKET  sktAccept;                    // ��������ͻ��������׽���
	BYTE    abyClientIp[IP_BYTE_LENGTH];  // ���������ӵĿͻ���IP��ַ
	UINT    uiClientPort;                 // ���������ӵĿͻ��˶˿�
	WSAEVENT  hAcceptEvent;               // ����������׽��ֹ������¼�������
	CRITICAL_SECTION crt_RcvPacket;       // TCP�������������ٽ���
	CSegPackageList lstSegPacket;         // TCP���ݽ�������
}ST_TCP_SERVER_OBJ,*PST_TCP_SERVER_OBJ;

// TCP������������
typedef CTypedPtrList<CPtrList,PST_TCP_SERVER_OBJ> CTcpServerObjList;

typedef struct _NCHU_STRUCT_TCP_SERVER_THD_OBJ
{
	UINT    uiThreadIdx;                                 // �߳�����
	CWinThread* pThread;                                 // �̶߳�����
	HANDLE hThreadExit;                                  // �߳��˳��¼�
	LPVOID  lpClassObj;                                  // �̺߳���������Ķ���ָ��
	LPVOID  lpListenObj;                                 // ��Ӧ�ļ����׽���
	WSAEVENT  hSelectEvents[WSA_MAXIMUM_WAIT_EVENTS];    // ��ǰ�̹߳�����¼�����������
	UINT uiSocketNum;                                    // ��ǰ�̹߳�����׽��ָ���
	CRITICAL_SECTION crt_Tcp_Server;                     // ���ʷ�����������ؼ���
	CTcpServerObjList lstTcpServer;                      // ���̶߳�Ӧ�ķ������˶�������

}ST_TCP_SERVER_THD_OBJ,*PST_TCP_SERVER_THD_OBJ;
typedef CTypedPtrList<CPtrList,PST_TCP_SERVER_THD_OBJ>CTcpServerThdObjList;

typedef struct _NCHU_STRUCT_TCP_CLIENT_OBJ
{
	UINT uiClientIdx;                 // �ͻ������̶߳����е�����
	SOCKET sktClient;                 // �ͻ���ͨ���׽���
	BYTE abyClientIp[IP_BYTE_LENGTH]; // �ͻ���IP��ַ
	UINT uiClientPort;                // �ͻ��˶˿ں�
	WSAEVENT hClientEvent;            // �ͻ����׽��ֹ������¼�����
	CRITICAL_SECTION crt_RcvPacket;   // �ͻ��˽��������ٽ���
	CSegPackageList lstSegPacket;     // �ͻ��˽��յ���������
}ST_TCP_CLIENT_OBJ,*PST_TCP_CLIENT_OBJ;
// TCP�ͻ�������
typedef CTypedPtrList<CPtrList,PST_TCP_CLIENT_OBJ> CTcpClientObjList;

typedef struct _NCHU_STRUCT_TCP_CLIENT_THD_OBJ
{
	UINT    uiThreadIdx;                                 // �߳�����
	CWinThread* pThread;                                 // �̶߳�����
	LPVOID  lpClassObj;                                  // �̺߳���������Ķ���ָ��
	WSAEVENT  hSelectEvents[WSA_MAXIMUM_WAIT_EVENTS];    // ��ǰ�̹߳�����¼�����������
	UINT uiSocketNum;                                    // ��ǰ�̹߳�����׽��ָ���
	CRITICAL_SECTION crt_Tcp_Client;                     // ���ʷ�����������ؼ���
	CTcpClientObjList lstTcpClient;                      // ���̶߳�Ӧ�ķ������˶�������

}ST_TCP_CLIENT_THD_OBJ,*PST_TCP_CLIENT_THD_OBJ;
typedef CTypedPtrList<CPtrList,PST_TCP_CLIENT_THD_OBJ> CTcpClientThdObjList;

//�����׽��ֶ���ṹ��
typedef struct _NCHU_STRUCT_LISTEN_OBJ
{
	UINT  uiCurrentThdNum;						// ��ǰ�̶߳����������̶߳������
	CRITICAL_SECTION crt_ThreadObj;				// �����̶߳�������ؼ������
	CTcpServerThdObjList   lstTcpServerThdObj;  // �̶߳�������
	SOCKET sktListen;							// �����׽���
	BYTE abyServerIp[IP_BYTE_LENGTH];			// ������������ַ
	UINT uiServerPort;							// �����������˿�
	WSAEVENT hListenEvent;						// �����׽��ְ󶨵��¼����
	CWinThread* pclsThdListen;					// ��������׽��������¼��߳�
	LPVOID lpClsObj;							// �����ָ��
	HANDLE hServerExit;							// �������˳������¼�
	HANDLE hServerHasExit;						// �����������߳��Ѿ��˳�
}ST_LISTEN_OBJ,*PST_LISTEN_OBJ;

//�����׽�������
typedef CTypedPtrList<CPtrList,PST_LISTEN_OBJ> CListenObjList;















