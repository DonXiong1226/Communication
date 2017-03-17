/***************************************************************************
* Copyright (c) 2011, AEC, All rights reserved.
*
* 文件名称：UDPCommunicatio.h
* 摘 要：本文件声明了一个UDP通信接口类，该类采用WSAEventSelect模型管理套接字
* 作 者：陈洞滨
*
* 修改记录：
*[日期]2011-12-07
*[作者/修改者]陈洞滨
*[修改原因]
***************************************************************************/

#pragma once

//#define  PtoPCommunicatio_API _declspec(dllexport)

#include "UDPComBase.h"
#include "DataDef.h"
#include "CSCCLog.h"
class  CUDPCommunicatio:public CUDPComBase
{
	// UDP通信类导出函数
	friend CUDPComBase* UDPComExports(LPVOID lpvoid);
	
	// 服务器线程处理函数，友元函数
	friend unsigned int Thread_Server_Proc(LPVOID lpvoid);
protected:
	// 无参构造函数
	CUDPCommunicatio(void);
public:
	// 虚构函数
	~CUDPCommunicatio(void);
private:
	CRITICAL_SECTION m_crt_Thread;         //访问线程链表关键代码段
	CThreadObjList m_lstThreadList;        //线程对象链表
	UINT m_uiCurrentThdNum;                //当前线程对象链表中线程对象个数
	CCsccLog m_clsCsccLog;
private:
	// 创建一个线程对象，并设置它的信息
	PST_THD_OBJ SetThreadObj();
	
	// 释放线程对象，并将它从线程对象列表中移除
	BOOL FreeThreadObj(PST_THD_OBJ pstThreadObj);

	// 添加新的接收端
	BOOL AddServerObj(BYTE  abyServerIp[IP_BYTE_LENGTH],UINT uiServerPort);
	
	// 创建新的服务器端对象
	PST_SERVER_OBJ SetServerObj(SOCKET sktServer,BYTE abyServerIp[IP_BYTE_LENGTH],UINT uiServerPort);
	
	// 释放服务器端信息
	void FreeServerObj(PST_SERVER_OBJ pstServerObj);
	
	// 重构线程管理事件句柄数组
	void ReBuildEventArry(PST_THD_OBJ pstThreadObj);
	
	// 删除接收端
	BOOL RemoveServerObj(PST_THD_OBJ pstThreadInfo,ST_SERVER_OBJ &stServerObj);
	
	// 将新增的服务器端对象加入服务器端链表
	BOOL InsertServerObj(PST_THD_OBJ pstThreadObj,PST_SERVER_OBJ pstServerObj);
	
	// 将新的服务器端安排给空闲线程处理
	void AssignToFreeThreadObj(PST_SERVER_OBJ pstServerObj);
	
	// 依据事件对象在事件对象句柄数组中的索引查找响应的套接字对象
	PST_SERVER_OBJ FindServerObj(PST_THD_OBJ pstThdObj,int iEventIndex);

	// 处理具体发生的网络事件
	BOOL HandleIO(PST_THD_OBJ pstThreadInfo,PST_SERVER_OBJ pstServerObj);

	// UDP接收数据
	INT UDPRecv(PST_SERVER_OBJ pstServerObj);
	
	// 将接收到的数据包加入链表中
	BOOL BufferSave(PST_SERVER_OBJ pstServerObj,PST_UDP_PACKATE pstPackage);
	
	// 判断接收到的数据是否可以组成一帧
    BOOL Has_a_nal(PST_SERVER_OBJ pstServerObj);
	
	// 将接收到的数据取出合并成一个完整的数据包                             		 
	BOOL FillBuffer(PST_SERVER_OBJ pstServerObj,SOCKADDR_IN sdServerAddr);    		 

public:
	// 初始化UDP通信类
	virtual BOOL InitUDPCom();
	
	// 反初始化UDP通信类
	virtual BOOL UnInitUDPCom();

	// 取网卡信息
	virtual void GetIpInfo(PST_IP_INFO pstIpInfo);
	
	// 将字符串型IP地址分割成为四个字段，并将它们存入字节数组
	virtual void IPStringtoByteArray(char szIp[16],BYTE abyByteArray[IP_BYTE_LENGTH]);

	// 将BYTE型的IP地址转化为String类型的IP
	virtual void ByteIpToStringIp(byte abyByteArray[IP_BYTE_LENGTH],char* pcStringIp);
	
	// 获取主机IP地址
	virtual void GetIpAddr(PST_IP_INFO pstIpInfo,UINT &uiIpNum);

	// 添加新的接收端
	virtual BOOL AddToAcceptList(BYTE abyServerIP[IP_BYTE_LENGTH],UINT uiServerPort);
	
	//取出数据
    virtual BOOL GetPacket(BYTE abyServerIP[IP_BYTE_LENGTH],UINT uiServerPort,ST_UDP_PACKATE &stUDPpacket);
	
	// 删除服务器端
	virtual BOOL DeleteFromAcceptList(BYTE abyServerIP[IP_BYTE_LENGTH],UINT uiServerPort);

	// 启动UDP发送程序
	virtual BOOL StartUDPSender(SOCKET &sktUDPSender,BYTE abyLocalIp[IP_BYTE_LENGTH],UINT uiLocalPort);

	// 发送数据
	virtual BOOL UDPSend(SOCKET sktUDPSender,BYTE abyDestIp[IP_BYTE_LENGTH],UINT uiDesPort,
		                 byte *pbyBuffer,UINT uiDataLength,UINT uiFrameNum);
	
	// 停止UDP发送
	virtual BOOL EndUDPSender(SOCKET skSocket);

	// 释放UDP导出类指针
	virtual BOOL ReleseUDPCom(CUDPComBase* pclsUDPCom);
};
