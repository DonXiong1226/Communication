
/***************************************************************************
* Copyright (c) 2011, AEC, All rights reserved.
*
* �ļ����ƣ�CSCCLog.h
* ժ Ҫ��������־������
* �� �ߣ��¶���
*
* �޸ļ�¼��
*[����]2011-12-08
*[����/�޸���]
*[�޸�ԭ��]
***************************************************************************/
#pragma once
#include <string>
#include <vector>
using namespace std;

#define MAX_MSG_LENGTH	 10000
#define MAX_FILE_PATH	 256
#define MAX_LOG_LENGTH	 512
#define DIRECTORY_NAME_LOG	   (_T("Log\\"))

// LOG���
#define LOG_INFO		  0   // ��Ϣ����
#define LOG_WARNING		  1   // ����
#define LOG_ERROR		  2   // ����

// LOG��Ϣ
typedef struct _NCHU_LOG
{
	INT	iType;                    //LOG����
	CHAR swzMsg[MAX_MSG_LENGTH];  //LOG����

}ST_LOG, *PST_LOG;

class CCsccLog
{
public:
	CCsccLog(void);
public:
	~CCsccLog(void);
public:	
	static VOID DebugPrint(const char* format , ...);       // �������ǩ��дLOG
	BOOL CreateLogFile();
	BOOL FolderExist(CString strPath);  // �鿴LOG�ļ����Ƿ����
	BOOL CreateFolder(CString strPath); // ����һ��LOG�ļ���
	BOOL OpenLogFile(CFile &clsFile, CString strDirectory); // ��LOG�ļ�
	INT MyNSprintf(char *pchLog, int iMaxLen, char *pchMsg, ...);  // Ҫ���ļ���д��LOG����
	VOID WriteLog(int iType, const string strMsg);         // д�ļ�LOG��������ʱ���
private:
	//дLOG
	CFile m_fileLog;
	CFile *m_pfileLog; //�ļ�ָ��
	TCHAR m_swzWorkPath[MAX_PATH];
};
