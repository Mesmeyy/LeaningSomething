#pragma once
#include"ILogic.h"

// CFindStudentDlg �Ի���

class CFindStudentDlg : public CDialog
{
	DECLARE_DYNAMIC(CFindStudentDlg)

public:
	CFindStudentDlg(ILogic* a,CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CFindStudentDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FINDSTUDENTDLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	ILogic* alogic;
	
	int M_FindStuId;
	int M_StuId;
	CString M_StuName;
	int M_StuScore;
};
