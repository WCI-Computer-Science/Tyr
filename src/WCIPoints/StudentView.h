
// StudentView.h : interface of the CStudentView class

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
private:
	CFont m_titleFont;
	CStatic m_title;

	int m_year;
	CFont m_year_staticFont;
	CStatic m_year_static;

	CButton m_see_all_students_check;

	LVCOLUMN m_col_oen, m_col_last_name, m_col_first_name, m_col_pref_name, m_col_start_year, m_col_grad_year;
	CListCtrl m_student_list;
	int selectionMark; // used to id student when removing, etc

	void loadStudentList();

public:
	afx_msg void OnBnClickedStudentYearChange();
	afx_msg void OnBnClickedToggleSeeAll();
	afx_msg void OnLvnItemchangedStudentList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLvnColumnclickStudentList(NMHDR* pNMHDR, LRESULT* pResult);
};

#ifndef _DEBUG  // debug version in WCIPointsView.cpp
inline CWCIPointsDoc* CStudentView::GetDocument() const
   { return reinterpret_cast<CWCIPointsDoc*>(m_pDocument); }
#endif

