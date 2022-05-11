#pragma once
#include "afxcmn.h"
#include"ILogic.h"

// CShowStudentDlg 对话框

class CShowStudentDlg : public CDialog
{
	DECLARE_DYNAMIC(CShowStudentDlg)

public:
	CShowStudentDlg(ILogic* a,CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CShowStudentDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SHOWSTUDENTDLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
	ILogic* alogic;
	virtual BOOL OnInitDialog();
	
public:
	CListCtrl M_ListCTRL;
	afx_msg void OnBnClickedButton1();
};
