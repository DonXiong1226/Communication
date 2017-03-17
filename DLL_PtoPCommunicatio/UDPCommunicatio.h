/***************************************************************************
* Copyright (c) 2011, AEC, All rights reserved.
*
* �ļ����ƣ�UDPCommunicatio.h
* ժ Ҫ�����ļ�������һ��UDPͨ�Žӿ��࣬�������WSAEventSelectģ�͹����׽���
* �� �ߣ��¶���
*
* �޸ļ�¼��
*[����]2011-12-07
*[����/�޸���]�¶���
*[�޸�ԭ��]
***************************************************************************/

#pragma once

//#define  PtoPCommunicatio_API _declspec(dllexport)

#include "UDPComBase.h"
#include "DataDef.h"
#include "CSCCLog.h"
class  CUDPCommunicatio:public CUDPComBase
{
	// UDPͨ���ർ������
	friend CUDPComBase* UDPComExports(LPVOID lpvoid);
	
	// �������̴߳���������Ԫ����
	friend unsigned int Thread_Server_Proc(LPVOID lpvoid);
protected:
	// �޲ι��캯��
	CUDPCommunicatio(void);
public:
	// �鹹����
	~CUDPCommunicatio(void);
private:
	CRITICAL_SECTION m_crt_Thread;         //�����߳�����ؼ������
	CThreadObjList m_lstThreadList;        //�̶߳�������
	UINT m_uiCurrentThdNum;                //��ǰ�̶߳����������̶߳������
	CCsccLog m_clsCsccLog;
private:
	// ����һ���̶߳��󣬲�����������Ϣ
	PST_THD_OBJ SetThreadObj();
	
	// �ͷ��̶߳��󣬲��������̶߳����б����Ƴ�
	BOOL FreeThreadObj(PST_THD_OBJ pstThreadObj);

	// ����µĽ��ն�
	BOOL AddServerObj(BYTE  abyServerIp[IP_BYTE_LENGTH],UINT uiServerPort);
	
	// �����µķ������˶���
	PST_SERVER_OBJ SetServerObj(SOCKET sktServer,BYTE abyServerIp[IP_BYTE_LENGTH],UINT uiServerPort);
	
	// �ͷŷ���������Ϣ
	void FreeServerObj(PST_SERVER_OBJ pstServerObj);
	
	// �ع��̹߳����¼��������
	void ReBuildEventArry(PST_THD_OBJ pstThreadObj);
	
	// ɾ�����ն�
	BOOL RemoveServerObj(PST_THD_OBJ pstThreadInfo,ST_SERVER_OBJ &stServerObj);
	
	// �������ķ������˶�����������������
	BOOL InsertServerObj(PST_THD_OBJ pstThreadObj,PST_SERVER_OBJ pstServerObj);
	
	// ���µķ������˰��Ÿ������̴߳���
	void AssignToFreeThreadObj(PST_SERVER_OBJ pstServerObj);
	
	// �����¼��������¼������������е�����������Ӧ���׽��ֶ���
	PST_SERVER_OBJ FindServerObj(PST_THD_OBJ pstThdObj,int iEventIndex);

	// ������巢���������¼�
	BOOL HandleIO(PST_THD_OBJ pstThreadInfo,PST_SERVER_OBJ pstServerObj);

	// UDP��������
	INT UDPRecv(PST_SERVER_OBJ pstServerObj);
	
	// �����յ������ݰ�����������
	BOOL BufferSave(PST_SERVER_OBJ pstServerObj,PST_UDP_PACKATE pstPackage);
	
	// �жϽ��յ��������Ƿ�������һ֡
    BOOL Has_a_nal(PST_SERVER_OBJ pstServerObj);
	
	// �����յ�������ȡ���ϲ���һ�����������ݰ�                             		 
	BOOL FillBuffer(PST_SERVER_OBJ pstServerObj,SOCKADDR_IN sdServerAddr);    		 

public:
	// ��ʼ��UDPͨ����
	virtual BOOL InitUDPCom();
	
	// ����ʼ��UDPͨ����
	virtual BOOL UnInitUDPCom();

	// ȡ������Ϣ
	virtual void GetIpInfo(PST_IP_INFO pstIpInfo);
	
	// ���ַ�����IP��ַ�ָ��Ϊ�ĸ��ֶΣ��������Ǵ����ֽ�����
	virtual void IPStringtoByteArray(char szIp[16],BYTE abyByteArray[IP_BYTE_LENGTH]);

	// ��BYTE�͵�IP��ַת��ΪString���͵�IP
	virtual void ByteIpToStringIp(byte abyByteArray[IP_BYTE_LENGTH],char* pcStringIp);
	
	// ��ȡ����IP��ַ
	virtual void GetIpAddr(PST_IP_INFO pstIpInfo,UINT &uiIpNum);

	// ����µĽ��ն�
	virtual BOOL AddToAcceptList(BYTE abyServerIP[IP_BYTE_LENGTH],UINT uiServerPort);
	
	//ȡ������
    virtual BOOL GetPacket(BYTE abyServerIP[IP_BYTE_LENGTH],UINT uiServerPort,ST_UDP_PACKATE &stUDPpacket);
	
	// ɾ����������
	virtual BOOL DeleteFromAcceptList(BYTE abyServerIP[IP_BYTE_LENGTH],UINT uiServerPort);

	// ����UDP���ͳ���
	virtual BOOL StartUDPSender(SOCKET &sktUDPSender,BYTE abyLocalIp[IP_BYTE_LENGTH],UINT uiLocalPort);

	// ��������
	virtual BOOL UDPSend(SOCKET sktUDPSender,BYTE abyDestIp[IP_BYTE_LENGTH],UINT uiDesPort,
		                 byte *pbyBuffer,UINT uiDataLength,UINT uiFrameNum);
	
	// ֹͣUDP����
	virtual BOOL EndUDPSender(SOCKET skSocket);

	// �ͷ�UDP������ָ��
	virtual BOOL ReleseUDPCom(CUDPComBase* pclsUDPCom);
};
