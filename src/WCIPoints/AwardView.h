
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
	afx_msg void OnBnClickedCreateBasicConstraint();
	afx_msg void OnBnClickedCreateCompoundConstraint();
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

	CListCtrl m_constraint_list;
	int selectionMark;

	void loadConstraintList();
public:
	afx_msg void OnLvnItemchangedConstraintList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedUnarchiveConstraint();
	afx_msg void OnBnClickedRemoveConstraint();
};



// Create basic award/constraint dialog

class CConstraintCreateBasicDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CConstraintCreateBasicDlg)

public:
	CConstraintCreateBasicDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CConstraintCreateBasicDlg();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_AWARD_BASIC_CREATE };
#endif

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	virtual void OnOK(); // Override OK button to perform input validation

	DECLARE_MESSAGE_MAP()

private:
	CFont m_titleFont;
	CStatic m_title;

	CButton m_is_award_check;

	CListBox m_award_type_list;
	int selectionMarkAwardType;
	CStatic m_award_type_description;

	CListBox m_action_type_list;
	CListCtrl m_action_list;

	void loadTypeData();
	void disableActionList();
	void enableActionList();

public:
	bool m_is_award;

	CString m_award_name;
	CString m_award_description;

	int m_award_type;

	int m_action_type;
	int m_action_id;

	int m_award_mx;
	int m_award_x;
	int m_award_y;	

public:
	afx_msg void OnLbnSelchangeAwardTypeList();
	afx_msg void OnLbnSelchangeActionTypeList();
	afx_msg void OnLvnItemchangedActionList(NMHDR* pNMHDR, LRESULT* pResult);
};



// Create compound award/constraint dialog

class CConstraintCreateCompoundDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CConstraintCreateCompoundDlg)

public:
	CConstraintCreateCompoundDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CConstraintCreateCompoundDlg();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_AWARD_COMPOUND_CREATE };
#endif

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	virtual void OnOK(); // Override OK button to perform input validation

	DECLARE_MESSAGE_MAP()

private:
	CFont m_titleFont;
	CStatic m_title;

	CButton m_is_award_check;

	CListBox m_award_type_list;
	CStatic m_award_type_description;

	CListCtrl m_constraint_list;
	void loadConstraintList();

public:
	bool m_is_award;

	CString m_award_name;
	CString m_award_description;

	std::vector<int> m_constraint_ids;

	int m_award_type;

public:
	afx_msg void OnLbnSelchangeAwardTypeList();
};



// Edit basic award/constraint dialog

class CConstraintEditBasicDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CConstraintEditBasicDlg)

public:
	// requires current award info to construct
	CConstraintEditBasicDlg(int award_type, CString name, CString description, CString action_type, CString action_id, int mx, int x, int y, CWnd* pParent = nullptr);
	virtual ~CConstraintEditBasicDlg();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_AWARD_BASIC_EDIT };
#endif

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	virtual void OnOK(); // Override OK button to perform input validation

	DECLARE_MESSAGE_MAP()

private:
	CFont m_titleFont;
	CStatic m_title;

	int m_award_type;
	CStatic m_award_type_description;

	CFont m_action_descriptionFont;
	CStatic m_action_description;

	CString m_action_type; // For simplicity, action_type is non-mutable, info is displayed instead
	CString m_action_name; // Same with action_id, name is displayed instead

public:
	CString m_award_name;
	CString m_award_description;

	int m_award_mx;
	int m_award_x;
	int m_award_y;
};
