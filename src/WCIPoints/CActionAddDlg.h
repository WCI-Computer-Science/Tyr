#pragma once


// CActionAdd dialog

class CActionAddDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CActionAddDlg)

public:
	CActionAddDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CActionAddDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ACTION_ADD };
#endif

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CListBox m_type_list;
	CString m_name;
	int m_points;
	CSpinButtonCtrl m_points_spin;
};
