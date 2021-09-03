// All edit dialog variants

#include "pch.h"
#include "WCIPoints.h"
#include "CEditDlg.h"
#include "afxdialogex.h"



IMPLEMENT_DYNAMIC(CEditStringDlg, CDialogEx)

CEditStringDlg::CEditStringDlg(CString title, CString caption, CString value, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_EDIT_VALUE, pParent)
	, m_title(title)
	, m_caption_text(caption)
	, m_value(value)
{

}

CEditStringDlg::~CEditStringDlg()
{
}

BOOL CEditStringDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetWindowText(m_title);
	m_caption.SetWindowTextW(m_caption_text);

	return TRUE;
}

void CEditStringDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ACTION_ATTR_STATIC, m_caption);
	DDX_Text(pDX, IDC_EDIT_ACTION_ATTR, m_value);
}


BEGIN_MESSAGE_MAP(CEditStringDlg, CDialogEx)
END_MESSAGE_MAP()




IMPLEMENT_DYNAMIC(CEditIntDlg, CDialogEx)

CEditIntDlg::CEditIntDlg(CString title, CString caption, int value, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_EDIT_VALUE, pParent)
	, m_title(title)
	, m_caption_text(caption)
	, m_value(value)
{

}

CEditIntDlg::~CEditIntDlg()
{
}

BOOL CEditIntDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetWindowText(m_title);
	m_caption.SetWindowTextW(m_caption_text);

	return TRUE;
}

void CEditIntDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ACTION_ATTR_STATIC, m_caption);
	DDX_Text(pDX, IDC_EDIT_ACTION_ATTR, m_value);
	DDV_MinMaxInt(pDX, m_value, 1, 10); // Only allow action to be up to 10 points
}


BEGIN_MESSAGE_MAP(CEditIntDlg, CDialogEx)
END_MESSAGE_MAP()