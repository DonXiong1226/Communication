// DLL_PtoPCommunicatio.h : DLL_PtoPCommunicatio DLL ����ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CDLL_PtoPCommunicatioApp
// �йش���ʵ�ֵ���Ϣ������� DLL_PtoPCommunicatio.cpp
//

class CDLL_PtoPCommunicatioApp : public CWinApp
{
public:
	CDLL_PtoPCommunicatioApp();

// ��д
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
