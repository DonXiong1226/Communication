// DLL_PtoPCommunicatio.cpp : ���� DLL �ĳ�ʼ�����̡�
//

#include "stdafx.h"

#define  PtoPCommunicatio_API _declspec(dllexport)
#include "DLL_PtoPCommunicatio.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//TODO: ����� DLL ����� MFC DLL �Ƕ�̬���ӵģ�
//		��Ӵ� DLL �������κε���
//		MFC �ĺ������뽫 AFX_MANAGE_STATE ����ӵ�
//		�ú�������ǰ�档
//
//		����:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// �˴�Ϊ��ͨ������
//		}
//
//		�˺������κ� MFC ����
//		������ÿ��������ʮ����Ҫ������ζ��
//		��������Ϊ�����еĵ�һ�����
//		���֣������������ж������������
//		������Ϊ���ǵĹ��캯���������� MFC
//		DLL ���á�
//
//		�й�������ϸ��Ϣ��
//		����� MFC ����˵�� 33 �� 58��
//


// CDLL_PtoPCommunicatioApp

BEGIN_MESSAGE_MAP(CDLL_PtoPCommunicatioApp, CWinApp)
END_MESSAGE_MAP()


// CDLL_PtoPCommunicatioApp ����
CDLL_PtoPCommunicatioApp::CDLL_PtoPCommunicatioApp()
{
	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CDLL_PtoPCommunicatioApp ����
CDLL_PtoPCommunicatioApp theApp;


// CDLL_PtoPCommunicatioApp ��ʼ��
BOOL CDLL_PtoPCommunicatioApp::InitInstance()
{
	CWinApp::InitInstance();

	//�����׽��ְ汾�⣬��ʼ���׽��֣�����Ӧ�ó����˳�ʱ����WSACleanup
	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	return TRUE;
}

