
// AwardView.cpp : implementation of the CAwardView class

#include "pch.h"
#include "framework.h"

#include "WCIPoints.h"

#include "WCIPointsDoc.h"
#include "AwardView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



IMPLEMENT_DYNCREATE(CAwardView, CFormView)

BEGIN_MESSAGE_MAP(CAwardView, CFormView)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()



CAwardView::CAwardView() noexcept
	: CFormView(IDD_AWARD)
{
	// TODO: add construction code here

}

CAwardView::~CAwardView()
{
}

void CAwardView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BOOL CAwardView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

void CAwardView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	// Default created with WS_EX_CLIENTEDGE for some reason
	ModifyStyleEx(WS_EX_CLIENTEDGE, 0);
	GetParentFrame()->RecalcLayout();
	ResizeParentToFit();

}

void CAwardView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CAwardView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
}


// CActionView diagnostics

#ifdef _DEBUG
void CAwardView::AssertValid() const
{
	CView::AssertValid();
}

void CAwardView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CWCIPointsDoc* CAwardView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CWCIPointsDoc)));
	return (CWCIPointsDoc*)m_pDocument;
}
#endif //_DEBUG


// CActionView message handlers
