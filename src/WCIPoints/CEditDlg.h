#pragma once


// Edit dialog that stores a CString value 
class CEditStringDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CEditStringDlg)

public:
	CEditStringDlg(CString title, CString caption, CString value, CWnd* pParent = nullptr);   // standard constructor
	virtual ~CEditStringDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EDIT_VALUE };
#endif

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	CString m_title;

	CString m_caption_text;
	CStatic m_caption;
public:
	CString m_value;
};