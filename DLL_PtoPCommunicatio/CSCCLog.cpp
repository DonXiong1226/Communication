#include "stdafx.h"

#include "CSCCLog.h"

CCsccLog::CCsccLog(void)
{
	GetModuleFileName( NULL, m_swzWorkPath, sizeof(m_swzWorkPath) );
	CString strBuf = m_swzWorkPath;
	INT iPos = strBuf.ReverseFind('\\');
	m_swzWorkPath[iPos+1] = '\0';
	CreateLogFile();
}

CCsccLog::~CCsccLog(void)
{
}

// �������ǩ��дLOG
VOID CCsccLog::DebugPrint(const CHAR* format , ...)
{
	va_list		marker ;
	CHAR		szMsg[MAX_LOG_LENGTH];
	CHAR		szBuf[MAX_LOG_LENGTH];

	sprintf_s(szBuf, ">>CSCC:\t");
	va_start( marker, format );
	memset(szMsg , 0 , sizeof(szMsg) ) ;
	vsprintf_s(szMsg , format , marker) ;
	va_end(marker);
	strcat_s(szBuf, szMsg);
	OutputDebugStringA(szBuf) ;
}

// д�ļ�LOG
VOID CCsccLog::WriteLog(int iType, const string strMsg)
{
	USES_CONVERSION;

	// ���ʱ���
	SYSTEMTIME   timenow;
	GetLocalTime(&timenow);

	ST_LOG	lmsg;
	lmsg.iType = iType;
	// ƴ��ʱ���
	MyNSprintf(lmsg.swzMsg, MAX_MSG_LENGTH,
		"[%02d-%02d %02d:%02d:%02d:%03d] : %s \r\n",
		timenow.wMonth, timenow.wDay,timenow.wHour,
		timenow.wMinute,timenow.wSecond,timenow.wMilliseconds,
		strMsg.c_str());

	// ���ļ������LOG
	m_pfileLog->Write(lmsg.swzMsg,(UINT)strlen(lmsg.swzMsg));
	m_pfileLog->Flush();
}

INT CCsccLog::MyNSprintf(char *pchLog, int iMaxLen, char *pchMsg, ...)
{
	va_list args;
	va_start(args, pchMsg);
	int len = _vsnprintf_s(pchLog,iMaxLen,iMaxLen,pchMsg,args);
	va_end(args);
	return len;
}

BOOL CCsccLog::CreateLogFile()
{
	// ����LOGĿ¼
	CString strLogDirectory;
	strLogDirectory.Format(_T("%s%s"), m_swzWorkPath, DIRECTORY_NAME_LOG);
	if (!FolderExist(strLogDirectory))
	{
		CreateFolder(strLogDirectory);
	}
	// ��LOG�ļ�
	if (!OpenLogFile(m_fileLog, strLogDirectory))
	{
		return FALSE;
	}
	m_pfileLog=&m_fileLog;

	return TRUE;
}

BOOL CCsccLog::FolderExist(CString strPath)
{
	WIN32_FIND_DATA wfd;
	BOOL rValue = FALSE;
	HANDLE hFind = FindFirstFile(strPath, &wfd);
	if ((hFind!=INVALID_HANDLE_VALUE) &&
		(wfd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
	{
		rValue = TRUE;
	}
	FindClose(hFind);
	return rValue;
}

BOOL CCsccLog::CreateFolder(CString strPath)
{
	SECURITY_ATTRIBUTES attrib;
	attrib.bInheritHandle = FALSE;
	attrib.lpSecurityDescriptor = NULL;
	attrib.nLength = sizeof(SECURITY_ATTRIBUTES);
	//���涨������Կ���ʡ��
	//ֱ��ʹ��return ::CreateDirectory(path, NULL);����
	return ::CreateDirectory(strPath, &attrib);
}

BOOL CCsccLog::OpenLogFile(CFile &clsFile, CString strDirectory)
{
	SYSTEMTIME   timenow;
	GetLocalTime(&timenow);

	CString strFileName;
	strFileName.Format(_T("UdpAndTcp%4d-%02d-%02d.log"),
		timenow.wYear,timenow.wMonth,timenow.wDay);
	CString strINIFullPath = strDirectory + strFileName;
	//if(clsFile.m_hFile != CFile::hFileNull)
	//	return FALSE;

	if(!clsFile.Open(strINIFullPath,CFile::modeReadWrite|CFile::shareDenyWrite))
	{
		if(!clsFile.Open(strINIFullPath, CFile::modeCreate | CFile::modeReadWrite | CFile::shareDenyWrite ))
		{
			clsFile.m_hFile = NULL;
			CString strMsg;
			CString csInfo;
			csInfo=_T("����־�ļ�ʧ�ܡ�\n�ļ�:");
			strMsg.Format( csInfo + _T("%s "), strFileName);
			return FALSE;
		}
	}

	clsFile.SeekToEnd();
	return TRUE;
}
