
// AwardView.h : interface of the CAwardView class

#pragma once

#include <vector>
#include "Constraint.h"


class CAwardView : public CFormView
{
public: // create from serialization only
	CAwardView() noexcept;
	DECLARE_DYNCREATE(CAwardView)
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_AWARD };
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
	virtual ~CAwardView();
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

	CButton m_see_awards_only_check;

	CListCtrl m_constraint_list;
	std::vector<Constraint::C> m_constraint_vector;
	int selectionMark; // used to id constraint when editing, etc

	CTreeCtrl m_constraint_tree;

	void loadConstraintList();
	void loadConstraintTree();
public:
	afx_msg void OnBnClickedToggleOnlyAward();
	afx_msg void OnLvnItemchangedConstraintList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMCustomdrawConstraintTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTvnItemChangingConstraintTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedCreateConstraint();
	afx_msg void OnBnClickedRemoveConstraint();
	afx_msg void OnBnClickedEditConstraint();
	afx_msg void OnBnClickedAccessArchive();
};

#ifndef _DEBUG  // debug version in WCIPointsView.cpp
inline CWCIPointsDoc* CAwardView::GetDocument() const
{
	return reinterpret_cast<CWCIPointsDoc*>(m_pDocument);
}
#endif



// Award Archive dialog

class CConstraintArchiveDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CConstraintArchiveDlg)

public:
	CConstraintArchiveDlg(CWnd* pParent = nullptr);
	virtual ~CConstraintArchiveDlg();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ARCHIVE };
#endif

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	CFont m_titleFont;
	CStatic m_title;

	int m_type;

	CListCtrl m_constraint_list;
	int selectionMark;

	void loadConstraintList();
public:
	afx_msg void OnLvnItemchangedConstraintList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedUnarchiveConstraint();
	afx_msg void OnBnClickedRemoveConstraint();
};