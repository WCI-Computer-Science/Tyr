
// ActionView.cpp : implementation of the CActionView class

#include "pch.h"
#include "framework.h"

#include "WCIPoints.h"

#include "WCIPointsDoc.h"
#include "ActionView.h"

#include "afxdialogex.h"

#include <string>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



IMPLEMENT_DYNCREATE(CActionView, CFormView)

BEGIN_MESSAGE_MAP(CActionView, CFormView)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_BN_CLICKED(IDC_ACTION_TYPE_CHANGE, &CActionView::OnBnClickedActionTypeChange)
END_MESSAGE_MAP()



CActionView::CActionView() noexcept
	: CFormView(IDD_ACTION)
	, m_type(0)
{
	// TODO: add construction code here

}

CActionView::~CActionView()
{
}

void CActionView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TITLE, m_title);
	DDX_Control(pDX, IDC_ACTION_TYPE_STATIC, m_type_static);
}

BOOL CActionView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

void CActionView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	m_titleFont.CreateFontW(
		20,
		0,
		0,
		0,
		FW_NORMAL,
		FALSE,
		FALSE,
		0,
		ANSI_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_SWISS,
		_T("Arial")
	);
	m_title.SetFont(&m_titleFont);

	if (m_type == 0)
		m_type_static.SetWindowTextW(_T("Athletics"));
	else if (m_type == 1)
		m_type_static.SetWindowTextW(_T("Academics"));
	else if (m_type == 2)
		m_type_static.SetWindowTextW(_T("Activities"));

	m_type_staticFont.CreateFontW(
		50,
		0,
		0,
		0,
		FW_NORMAL,
		FALSE,
		FALSE,
		0,
		ANSI_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_SWISS,
		_T("Arial")
	);
	m_type_static.SetFont(&m_type_staticFont);

	GetParentFrame()->RecalcLayout();
	ResizeParentToFit();

}

void CActionView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CActionView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
}


// CActionView diagnostics

#ifdef _DEBUG
void CActionView::AssertValid() const
{
	CView::AssertValid();
}

void CActionView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CWCIPointsDoc* CActionView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CWCIPointsDoc)));
	return (CWCIPointsDoc*)m_pDocument;
}
#endif //_DEBUG


// CActionView message handlers


void CActionView::OnBnClickedActionTypeChange()
{
	CActionChangeTypeDlg actionChangeTypeDlg;
	if (actionChangeTypeDlg.DoModal() == IDOK) {
		m_type = actionChangeTypeDlg.m_type;

		if (m_type == 0)
			m_type_static.SetWindowTextW(_T("Athletics"));
		else if (m_type == 1)
			m_type_static.SetWindowTextW(_T("Academics"));
		else if (m_type == 2)
			m_type_static.SetWindowTextW(_T("Activities"));

		m_title.Invalidate();
	}
}




// ActionChangeTypeDlg dialog

IMPLEMENT_DYNAMIC(CActionChangeTypeDlg, CDialogEx)

CActionChangeTypeDlg::CActionChangeTypeDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ACTION_CHANGE_TYPE, pParent)
{

}

CActionChangeTypeDlg::~CActionChangeTypeDlg()
{
}

BOOL CActionChangeTypeDlg::OnInitDialog() {
	CDialog::OnInitDialog();

	m_type_list.AddString(_T("Athletics"));
	m_type_list.AddString(_T("Academics"));
	m_type_list.AddString(_T("Activities"));
	m_type_list.SetCurSel(0);

	return TRUE;
}

void CActionChangeTypeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ACTION_TYPE_LIST, m_type_list);
}


BEGIN_MESSAGE_MAP(CActionChangeTypeDlg, CDialogEx)
	ON_LBN_SELCHANGE(IDC_ACTION_TYPE_LIST, &CActionChangeTypeDlg::OnLbnSelchangeActionTypeList)
END_MESSAGE_MAP()


// ActionChangeTypeDlg message handlers

void CActionChangeTypeDlg::OnLbnSelchangeActionTypeList()
{
	m_type = m_type_list.GetCurSel() == LB_ERR ? 0 : m_type_list.GetCurSel();
}
