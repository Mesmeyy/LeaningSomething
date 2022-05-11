// AddStudengDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "MFCApplication20220510_2.h"
#include "AddStudengDlg.h"
#include "afxdialogex.h"
#include"ILogic.h"

// CAddStudengDlg �Ի���

IMPLEMENT_DYNAMIC(CAddStudengDlg, CDialog)

CAddStudengDlg::CAddStudengDlg(ILogic* a,CWnd* pParent /*=NULL*/)
	: CDialog(IDD_ADDSTUDENGDLG, pParent)
	, M_StuId(0)
	, M_StuName(_T(""))
	, M_StuScore(0)
	,alogic(a)
{

}

CAddStudengDlg::~CAddStudengDlg()
{
}

void CAddStudengDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, M_StuId);
	DDV_MinMaxInt(pDX, M_StuId, 1, 1000);
	DDX_Text(pDX, IDC_EDIT2, M_StuName);
	DDX_Text(pDX, IDC_EDIT3, M_StuScore);
	DDV_MinMaxInt(pDX, M_StuScore, 0, 100);
}


BEGIN_MESSAGE_MAP(CAddStudengDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CAddStudengDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CAddStudengDlg ��Ϣ�������


void CAddStudengDlg::OnBnClickedOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	this->UpdateData(true);
	CStudentDTO s1(this->M_StuId, this->M_StuName.GetString(), this->M_StuScore);
	alogic->AddStudentInfo(s1);
	CDialog::OnOK();
}
