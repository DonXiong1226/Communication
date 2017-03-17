// DLL_PtoPCommunicatio.h : DLL_PtoPCommunicatio DLL 的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"		// 主符号


// CDLL_PtoPCommunicatioApp
// 有关此类实现的信息，请参阅 DLL_PtoPCommunicatio.cpp
//

class CDLL_PtoPCommunicatioApp : public CWinApp
{
public:
	CDLL_PtoPCommunicatioApp();

// 重写
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
