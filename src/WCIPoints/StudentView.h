
// StudentView.h : interface of the CStudentView class
//

#pragma once


class CStudentView : public CFormView
{
public: // create from serialization only
	CStudentView() noexcept;
	DECLARE_DYNCREATE(CStudentView)
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_STUDENT };
#endif

// Attributes
public:
	CWCIPointsDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate(); // called first time after construct

// Implementation
public:
	virtual ~CStudentView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in WCIPointsView.cpp
inline CWCIPointsDoc* CStudentView::GetDocument() const
   { return reinterpret_cast<CWCIPointsDoc*>(m_pDocument); }
#endif

