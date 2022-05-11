// ShowStudentDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "MFCApplication20220510_2.h"
#include "ShowStudentDlg.h"
#include "afxdialogex.h"
#include"ILogic.h"

// CShowStudentDlg 对话框

IMPLEMENT_DYNAMIC(CShowStudentDlg, CDialog)

CShowStudentDlg::CShowStudentDlg(ILogic* a,CWnd* pParent /*=NULL*/)
	: CDialog(IDD_SHOWSTUDENTDLG, pParent),
	alogic(a)

{

}

CShowStudentDlg::~CShowStudentDlg()
{
}

void CShowStudentDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST2, M_ListCTRL);
}

BOOL CShowStudentDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	DWORD dwStyle = this->M_ListCTRL.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;		//选中某行使其高亮（只适用于report风格的listctrl）
	dwStyle |= LVS_EX_GRIDLINES;			//网格线（只适用于report风格的listctrl）
	dwStyle |= LVS_EX_CHECKBOXES;			//item 前生成checkbox 控件
	M_ListCTRL.SetExtendedStyle(dwStyle);	//设置扩展风格

											//设置表列
	CString scoreString;
	scoreString.Format(_T("%d"), 98);
	this->M_ListCTRL.InsertColumn(0, _T("ID"), 0, 150);
	this->M_ListCTRL.InsertColumn(1, _T("姓名"), 0, 150);
	this->M_ListCTRL.InsertColumn(2, _T("成绩"), 0, 150);
	CObArray *list = alogic->GetAllStudents();
	for (int i = 0; i < list->GetCount(); i++)
	{
		CStudentDTO *temp = (CStudentDTO *)list->GetAt(i);
		CString strT;

		strT.Format("%d", temp->getStuID());
		this->M_ListCTRL.InsertItem(0, strT);
		strT.Format("%s", temp->getStuName());
		this->M_ListCTRL.SetItemText(0, 1, strT);
		strT.Format("%d", temp->getStuScore());
		this->M_ListCTRL.SetItemText(0, 2, strT);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


BEGIN_MESSAGE_MAP(CShowStudentDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &CShowStudentDlg::OnBnClickedButton1)
END_MESSAGE_MAP()

	
// CShowStudentDlg 消息处理程序


void CShowStudentDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	CString str;
	// TODO: 在此添加控件通知处理程序代码
	for (int i = 0; i < this->M_ListCTRL.GetItemCount(); i++)
	{
		if (this->M_ListCTRL.GetCheck(i))
		{
			TCHAR szBuf[1024];
			LVITEM lvi;
			lvi.iItem = i;
			lvi.iSubItem = 0;
			lvi.mask = LVFIF_TEXT;
			lvi.pszText = szBuf;
			lvi.cchTextMax = 1024;
			M_ListCTRL.GetItem(&lvi);
			int stuID = atoi(szBuf);
			alogic->DeleteStudentByID(stuID);

			str.Format(_T("name=%s"), szBuf);
			AfxMessageBox(str);

			this->M_ListCTRL.DeleteItem(i);
			i--;
		}
	}
}
