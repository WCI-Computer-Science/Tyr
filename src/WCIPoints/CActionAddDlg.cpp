// CActionAdd.cpp : implementation file
//

#include "pch.h"
#include "WCIPoints.h"
#include "CActionAddDlg.h"
#include "afxdialogex.h"


// CActionAdd dialog

IMPLEMENT_DYNAMIC(CActionAddDlg, CDialogEx)

CActionAddDlg::CActionAddDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ACTION_ADD, pParent)
	, m_name(_T(""))
	, m_points(0)
{
}

CActionAddDlg::~CActionAddDlg()
{
}

BOOL CActionAddDlg::OnInitDialog() {
	CDialog::OnInitDialog();

	m_type_list.AddString(_T("Athletics"));
	m_type_list.AddString(_T("Academics"));
	m_type_list.AddString(_T("Activities"));

	return TRUE;
}

void CActionAddDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_TYPE, m_type_list);
	DDX_Text(pDX, IDC_EDIT_NAME, m_name);
	DDV_MaxChars(pDX, m_name, 64);
	DDX_Text(pDX, IDC_EDIT_POINTS, m_points);
	DDV_MinMaxInt(pDX, m_points, 0, 100);
	DDX_Control(pDX, IDC_SPIN_POINTS, m_points_spin);
}


BEGIN_MESSAGE_MAP(CActionAddDlg, CDialogEx)
END_MESSAGE_MAP()


// CActionAdd message handlers
