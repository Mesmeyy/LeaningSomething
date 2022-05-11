
// MFCApplication20220510_2Dlg.cpp : 实现文件
//

#include "stdafx.h"
#include "MFCApplication20220510_2.h"
#include "MFCApplication20220510_2Dlg.h"
#include "afxdialogex.h"
#include"StudentDTO.h"
#include"CLogicImpl.h"
#include"AddStudengDlg.h"
#include"ShowStudentDlg.h"
#include"FindStudentDlg.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMFCApplication20220510_2Dlg 对话框



CMFCApplication20220510_2Dlg::CMFCApplication20220510_2Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_MFCAPPLICATION20220510_2_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	alogic = new  CLogicImpl();
}

void CMFCApplication20220510_2Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMFCApplication20220510_2Dlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CMFCApplication20220510_2Dlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CMFCApplication20220510_2Dlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CMFCApplication20220510_2Dlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CMFCApplication20220510_2Dlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &CMFCApplication20220510_2Dlg::OnBnClickedButton5)
END_MESSAGE_MAP()


// CMFCApplication20220510_2Dlg 消息处理程序

BOOL CMFCApplication20220510_2Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMFCApplication20220510_2Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMFCApplication20220510_2Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMFCApplication20220510_2Dlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码

	CAddStudengDlg dlg(alogic);
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDCANCEL || nResponse == IDOK) {
		;
	}
}


void CMFCApplication20220510_2Dlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	CShowStudentDlg dlg(alogic);
	INT_PTR nResponse = dlg.DoModal();
}


void CMFCApplication20220510_2Dlg::OnBnClickedButton3()
{
	CFindStudentDlg dlg(alogic);
	INT_PTR nResponse = dlg.DoModal();
	// TODO: 在此添加控件通知处理程序代码
}


void CMFCApplication20220510_2Dlg::OnBnClickedButton4()
{
	// TODO: 在此添加控件通知处理程序代码
	CFileDialog adlg(TRUE);
	if (adlg.DoModal() == IDOK) {
		CString name = adlg.GetPathName();
		alogic->ReadStudentsFromFiletoList(name);
		MessageBox("文件读取成功");
	}
}




void CMFCApplication20220510_2Dlg::OnBnClickedButton5()
{
	// TODO: 在此添加控件通知处理程序代码
	CFileDialog adlg(FALSE);
	if (adlg.DoModal() == IDOK) {
		CString name = adlg.GetPathName();
		alogic->WriteStudentstoFromListtoFile(name);
		MessageBox("文件写入成功");
	}
}
