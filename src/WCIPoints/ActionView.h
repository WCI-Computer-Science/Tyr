
// ActionView.h : interface of the CActionView class

#pragma once


class CActionView : public CFormView
{
public: // create from serialization only
	CActionView() noexcept;
	DECLARE_DYNCREATE(CActionView)
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ACTION };
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
	virtual ~CActionView();
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

	int m_type;
	CFont m_type_staticFont;
	CStatic m_type_static;

	LVCOLUMN m_col_name, m_col_points;
	CListCtrl m_action_list;
	int selectionMark;

	void loadTypeData();
public:
	afx_msg void OnBnClickedActionTypeChange();
	afx_msg void OnLvnColumnclickActionList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLvnItemchangedActionList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedCreateAction();
	afx_msg void OnBnClickedRemoveAction();
	afx_msg void OnBnClickedAccessArchive();
};

#ifndef _DEBUG  // debug version in WCIPointsView.cpp
inline CWCIPointsDoc* CActionView::GetDocument() const
{
	return reinterpret_cast<CWCIPointsDoc*>(m_pDocument);
}
#endif



// Action Archive dialog

class CActionArchiveDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CActionArchiveDlg)

public:
	CActionArchiveDlg(int type, CWnd* pParent = nullptr);
	virtual ~CActionArchiveDlg();

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

	CListCtrl m_action_list;
	int selectionMark;

	void loadTypeData();
public:
	afx_msg void OnLvnItemchangedActionList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedUnarchiveAction();
	afx_msg void OnBnClickedRemoveAction();
};






// ActionChangeTypeDlg dialog

class CActionChangeTypeDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CActionChangeTypeDlg)

public:
	CActionChangeTypeDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CActionChangeTypeDlg();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ACTION_CHANGE_TYPE };
#endif

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	CListBox m_type_list;
public:
	int m_type;
	afx_msg void OnLbnSelchangeActionTypeList();
};



// ActionCreateDlg dialog

class CActionCreateDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CActionCreateDlg)

public:
	CActionCreateDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CActionCreateDlg();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ACTION_CREATE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_name;
	int m_points;
};
