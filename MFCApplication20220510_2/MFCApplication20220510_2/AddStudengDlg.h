#pragma once
#include"ILogic.h"

// CAddStudengDlg 对话框

class CAddStudengDlg : public CDialog
{
	DECLARE_DYNAMIC(CAddStudengDlg)

public:
	CAddStudengDlg(ILogic* a,CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CAddStudengDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ADDSTUDENGDLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	int M_StuId;
	CString M_StuName;
	int M_StuScore;
	afx_msg void OnBnClickedOk();
	ILogic* alogic;
};
