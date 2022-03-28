
// StudentView.h : interface of the CStudentView class

#pragma once

#include <vector>

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

	afx_msg void OnBnClickedCreateStudent();
	afx_msg void OnBnClickedRemoveStudent();
	afx_msg void OnBnClickedAssignActions();
	afx_msg void OnBnClickedEditActions();
	afx_msg void OnBnClickedSeeStudentAwards();

	afx_msg void OnBnClickedEvaluateAwards();
	afx_msg void OnBnClickedHandOutAwards();
	afx_msg void OnBnClickedSeeAwards();
};

#ifndef _DEBUG  // debug version in WCIPointsView.cpp
inline CWCIPointsDoc* CStudentView::GetDocument() const
   { return reinterpret_cast<CWCIPointsDoc*>(m_pDocument); }
#endif



// Create student dialog

class CStudentCreateDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CStudentCreateDlg)

public:
	CStudentCreateDlg(int start_year, CWnd* pParent = nullptr);   // standard constructor
	virtual ~CStudentCreateDlg();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_STUDENT_CREATE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_oen;
	CString m_last_name;
	CString m_first_name;
	CString m_pref_name;
	int m_start_year;
	int m_grad_year;
};



// Assign awards to student dialog

class CStudentAssignActionDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CStudentAssignActionDlg)

public:
	CStudentAssignActionDlg(int year, CWnd* pParent = nullptr);   // standard constructor
	virtual ~CStudentAssignActionDlg();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_STUDENT_ASSIGN_ACTION };
#endif

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	virtual void OnOK(); // Add all ids into the vector on OK

	DECLARE_MESSAGE_MAP()

private:
	CFont m_titleFont;
	CStatic m_title;

	int m_action_type;
	CListCtrl m_action_list;
	CListBox m_action_type_list;

	void loadTypeData();

public:
	std::vector<int> m_action_ids;
	int m_year;

	afx_msg void OnLbnSelchangeActionTypeList();
};



// See and manage student actions dialog

class CStudentEditActionsDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CStudentEditActionsDlg)

public:
	CStudentEditActionsDlg(CString header, CString title, int student_id, CWnd* pParent = nullptr);   // no standard constructor needed
	virtual ~CStudentEditActionsDlg();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_STUDENT_MANAGE_LIST };
#endif

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CString m_header;

	CString m_title_text;
	CFont m_titleFont;
	CStatic m_title;

	int m_student_id;

	LVCOLUMN m_col_name, m_col_type, m_col_points, m_col_year;
	CListCtrl m_action_list;
	int selectionMark;

	CButton m_remove_action;

	void loadTypeData();
public:
	afx_msg void OnBnClickedStudentDeleteAction();
	afx_msg void OnLvnItemchangedStudentList(NMHDR* pNMHDR, LRESULT* pResult);
};



// See and manage student awards dialog

class CStudentEditAwardsDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CStudentEditAwardsDlg)

public:
	CStudentEditAwardsDlg(CString header, CString title, int student_id, CWnd* pParent = nullptr);   // no standard constructor needed
	virtual ~CStudentEditAwardsDlg();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_STUDENT_MANAGE_LIST };
#endif

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CString m_header;

	CString m_title_text;
	CFont m_titleFont;
	CStatic m_title;

	int m_student_id;

	LVCOLUMN m_col_name, m_col_desc, m_col_confirmed;
	CListCtrl m_award_list;
	int selectionMark;

	CButton m_toggle_handout_award;

	void loadConstraintList();
public:
	afx_msg void OnBnClickedStudentToggleAward();
	afx_msg void OnLvnItemchangedAwardList(NMHDR* pNMHDR, LRESULT* pResult);
};



// Hand out all awards for the year dialog

class CHandOutAwardsDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CHandOutAwardsDlg)

public:
	CHandOutAwardsDlg(CString header, CString title, int year, CWnd* pParent = nullptr);   // standard constructor
	virtual ~CHandOutAwardsDlg();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_STUDENT_AWARD_LIST };
#endif

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CString m_header;

	CString m_title_text;
	CFont m_titleFont;
	CStatic m_title;

	int m_year;

	CButton m_see_confirmed;

	LVCOLUMN m_col_stdt_first_name, m_col_stdt_last_name, m_col_award_name, m_col_award_desc, m_col_award_confirmed;
	CListCtrl m_award_list;
	std::vector<std::pair<int, int>> m_ids; // Store (stdt_id, cnst_id) for every assigned award
	int selectionMark;

	CButton m_toggle_handout_award;

	void loadConstraintList();
public:
	afx_msg void OnBnClickedSeeUnconfirmed();
	afx_msg void OnBnClickedStudentToggleAward();
	afx_msg void OnLvnItemchangedAwardList(NMHDR* pNMHDR, LRESULT* pResult);
};



// See and manage all awards (possibly from all years) dialog

class CManageAllAwardsDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CManageAllAwardsDlg)

public:
	CManageAllAwardsDlg(CString header, CString title, int year, CWnd* pParent = nullptr);   // standard constructor
	virtual ~CManageAllAwardsDlg();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_STUDENT_AWARD_LIST };
#endif

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CString m_header;

	CString m_title_text;
	CFont m_titleFont;
	CStatic m_title;

	CButton m_see_all_years;

	int m_year;

	LVCOLUMN m_col_stdt_first_name, m_col_stdt_last_name, m_col_award_name, m_col_award_desc, m_col_grad_year;
	CListCtrl m_award_list;
	std::vector<std::pair<int, int>> m_ids; // Store (stdt_id, cnst_id) for every assigned award
	int selectionMark;

	CButton m_remove_award;

	void loadConstraintList();
public:
	afx_msg void OnBnClickedSeeAllYears();
	afx_msg void OnBnClickedStudentRemoveAward();
	afx_msg void OnLvnItemchangedAwardList(NMHDR* pNMHDR, LRESULT* pResult);
};