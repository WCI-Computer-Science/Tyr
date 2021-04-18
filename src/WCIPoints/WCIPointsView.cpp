
// WCIPointsView.cpp : implementation of the CWCIPointsView class
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "WCIPoints.h"
#endif

#include "WCIPointsDoc.h"
#include "WCIPointsView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CWCIPointsView

IMPLEMENT_DYNCREATE(CWCIPointsView, CView)

BEGIN_MESSAGE_MAP(CWCIPointsView, CView)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CWCIPointsView construction/destruction

CWCIPointsView::CWCIPointsView() noexcept
{
	// TODO: add construction code here

}

CWCIPointsView::~CWCIPointsView()
{
}

BOOL CWCIPointsView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CWCIPointsView drawing

void CWCIPointsView::OnDraw(CDC* /*pDC*/)
{
	CWCIPointsDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}

void CWCIPointsView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CWCIPointsView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CWCIPointsView diagnostics

#ifdef _DEBUG
void CWCIPointsView::AssertValid() const
{
	CView::AssertValid();
}

void CWCIPointsView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CWCIPointsDoc* CWCIPointsView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CWCIPointsDoc)));
	return (CWCIPointsDoc*)m_pDocument;
}
#endif //_DEBUG


// CWCIPointsView message handlers