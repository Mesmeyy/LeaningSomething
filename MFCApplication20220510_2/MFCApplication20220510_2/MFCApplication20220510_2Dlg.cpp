
// MFCApplication20220510_2Dlg.cpp : ʵ���ļ�
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


// CMFCApplication20220510_2Dlg �Ի���



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


// CMFCApplication20220510_2Dlg ��Ϣ�������

BOOL CMFCApplication20220510_2Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CMFCApplication20220510_2Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CMFCApplication20220510_2Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMFCApplication20220510_2Dlg::OnBnClickedButton1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	CAddStudengDlg dlg(alogic);
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDCANCEL || nResponse == IDOK) {
		;
	}
}


void CMFCApplication20220510_2Dlg::OnBnClickedButton2()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CShowStudentDlg dlg(alogic);
	INT_PTR nResponse = dlg.DoModal();
}


void CMFCApplication20220510_2Dlg::OnBnClickedButton3()
{
	CFindStudentDlg dlg(alogic);
	INT_PTR nResponse = dlg.DoModal();
	// TODO: �ڴ���ӿؼ�֪ͨ����������
}


void CMFCApplication20220510_2Dlg::OnBnClickedButton4()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CFileDialog adlg(TRUE);
	if (adlg.DoModal() == IDOK) {
		CString name = adlg.GetPathName();
		alogic->ReadStudentsFromFiletoList(name);
		MessageBox("�ļ���ȡ�ɹ�");
	}
}




void CMFCApplication20220510_2Dlg::OnBnClickedButton5()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CFileDialog adlg(FALSE);
	if (adlg.DoModal() == IDOK) {
		CString name = adlg.GetPathName();
		alogic->WriteStudentstoFromListtoFile(name);
		MessageBox("�ļ�д��ɹ�");
	}
}
