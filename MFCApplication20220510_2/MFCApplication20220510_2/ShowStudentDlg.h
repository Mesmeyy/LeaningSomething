#pragma once
#include "afxcmn.h"
#include"ILogic.h"

// CShowStudentDlg �Ի���

class CShowStudentDlg : public CDialog
{
	DECLARE_DYNAMIC(CShowStudentDlg)

public:
	CShowStudentDlg(ILogic* a,CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CShowStudentDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SHOWSTUDENTDLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
	ILogic* alogic;
	virtual BOOL OnInitDialog();
	
public:
	CListCtrl M_ListCTRL;
	afx_msg void OnBnClickedButton1();
};
