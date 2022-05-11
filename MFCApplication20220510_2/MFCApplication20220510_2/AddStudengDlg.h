#pragma once
#include"ILogic.h"

// CAddStudengDlg �Ի���

class CAddStudengDlg : public CDialog
{
	DECLARE_DYNAMIC(CAddStudengDlg)

public:
	CAddStudengDlg(ILogic* a,CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CAddStudengDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ADDSTUDENGDLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	int M_StuId;
	CString M_StuName;
	int M_StuScore;
	afx_msg void OnBnClickedOk();
	ILogic* alogic;
};
