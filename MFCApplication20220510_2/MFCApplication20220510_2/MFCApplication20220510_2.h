
// MFCApplication20220510_2.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CMFCApplication20220510_2App: 
// �йش����ʵ�֣������ MFCApplication20220510_2.cpp
//

class CMFCApplication20220510_2App : public CWinApp
{
public:
	CMFCApplication20220510_2App();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CMFCApplication20220510_2App theApp;