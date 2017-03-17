
/***************************************************************************
* Copyright (c) 2011, AEC, All rights reserved.
*
* 文件名称：CSCCLog.h
* 摘 要：调试日志类声明
* 作 者：陈洞滨
*
* 修改记录：
*[日期]2011-12-08
*[作者/修改者]
*[修改原因]
***************************************************************************/
#pragma once
#include <string>
#include <vector>
using namespace std;

#define MAX_MSG_LENGTH	 10000
#define MAX_FILE_PATH	 256
#define MAX_LOG_LENGTH	 512
#define DIRECTORY_NAME_LOG	   (_T("Log\\"))

// LOG类别
#define LOG_INFO		  0   // 信息内容
#define LOG_WARNING		  1   // 警告
#define LOG_ERROR		  2   // 错误

// LOG信息
typedef struct _NCHU_LOG
{
	INT	iType;                    //LOG类型
	CHAR swzMsg[MAX_MSG_LENGTH];  //LOG内容

}ST_LOG, *PST_LOG;

class CCsccLog
{
public:
	CCsccLog(void);
public:
	~CCsccLog(void);
public:	
	static VOID DebugPrint(const char* format , ...);       // 在输出标签上写LOG
	BOOL CreateLogFile();
	BOOL FolderExist(CString strPath);  // 查看LOG文件夹是否存在
	BOOL CreateFolder(CString strPath); // 创建一个LOG文件夹
	BOOL OpenLogFile(CFile &clsFile, CString strDirectory); // 打开LOG文件
	INT MyNSprintf(char *pchLog, int iMaxLen, char *pchMsg, ...);  // 要在文件中写的LOG内容
	VOID WriteLog(int iType, const string strMsg);         // 写文件LOG并附加上时间戳
private:
	//写LOG
	CFile m_fileLog;
	CFile *m_pfileLog; //文件指针
	TCHAR m_swzWorkPath[MAX_PATH];
};
