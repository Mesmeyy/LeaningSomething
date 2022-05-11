// FindStudentDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "MFCApplication20220510_2.h"
#include "FindStudentDlg.h"
#include "afxdialogex.h"
#include"ILogic.h"


// CFindStudentDlg 对话框

IMPLEMENT_DYNAMIC(CFindStudentDlg, CDialog)

CFindStudentDlg::CFindStudentDlg(ILogic*a,CWnd* pParent /*=NULL*/)
	: CDialog(IDD_FINDSTUDENTDLG, pParent)
	,alogic(a)
	, M_FindStuId(1)
	, M_StuId(1)
	, M_StuName(_T(""))
	, M_StuScore(0)
{

}

CFindStudentDlg::~CFindStudentDlg()
{
}

void CFindStudentDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, M_FindStuId);
	DDV_MinMaxInt(pDX, M_FindStuId, 1, 10000);
	DDX_Text(pDX, IDC_EDIT2, M_StuId);
	DDV_MinMaxInt(pDX, M_StuId, 1, 10000);
	DDX_Text(pDX, IDC_EDIT3, M_StuName);
	DDX_Text(pDX, IDC_EDIT4, M_StuScore);
	DDV_MinMaxInt(pDX, M_StuScore, 0, 100);
}


BEGIN_MESSAGE_MAP(CFindStudentDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &CFindStudentDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CFindStudentDlg::OnBnClickedButton2)
END_MESSAGE_MAP()


// CFindStudentDlg 消息处理程序


void CFindStudentDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	this->UpdateData(true);
	int stuID = this->M_FindStuId;

	CStudentDTO astu = alogic->FindByID(stuID);
	this->M_StuId = astu.getStuID();
	this->M_StuName = astu.getStuName();
	this->M_StuScore = astu.getStuScore();
	this->UpdateData(false);
}


void CFindStudentDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	this->UpdateData(true);
	CStudentDTO astu(this->M_StuId, this->M_StuName, this->M_StuScore);
	alogic->EditeStudentByID(astu);
	MessageBox("修改成功");
}
