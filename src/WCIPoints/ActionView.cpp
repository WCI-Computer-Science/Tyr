
// ActionView.cpp : implementation of the CActionView class

#include "pch.h"
#include "framework.h"

#include "WCIPoints.h"
#include "WCIPointsDoc.h"
#include "CEditDlg.h"

#include "afxdialogex.h"

#include <string>
#include <iostream>

#include "mysql_connection.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include "Action.h"

#include "ActionView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



IMPLEMENT_DYNCREATE(CActionView, CFormView)

BEGIN_MESSAGE_MAP(CActionView, CFormView)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_BN_CLICKED(IDC_ACTION_TYPE_CHANGE, &CActionView::OnBnClickedActionTypeChange)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_ACTION_LIST, &CActionView::OnLvnColumnclickActionList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_ACTION_LIST, &CActionView::OnLvnItemchangedActionList)
	ON_BN_CLICKED(IDC_REMOVE_ACTION, &CActionView::OnBnClickedRemoveAction)
END_MESSAGE_MAP()



CActionView::CActionView() noexcept
	: CFormView(IDD_ACTION)
	, m_type(0)
	, selectionMark(-1)
{
	// TODO: add construction code here

}

CActionView::~CActionView()
{
}

void CActionView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TITLE, m_title);
	DDX_Control(pDX, IDC_ACTION_TYPE_STATIC, m_type_static);
	DDX_Control(pDX, IDC_ACTION_LIST, m_action_list);
}

BOOL CActionView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

void CActionView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	// Default created with WS_EX_CLIENTEDGE for some reason
	ModifyStyleEx(WS_EX_CLIENTEDGE, 0);

	m_action_list.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	// Insert dummy column so LVCFMT_FIXED_WIDTH works properly (see Remarks of LVCOLUMN struct)
	m_action_list.InsertColumn(0, &LVCOLUMN());

	m_col_name.mask = m_col_points.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;

	m_col_name.fmt = m_col_points.fmt = LVCFMT_LEFT | LVCFMT_FIXED_WIDTH;

	m_col_name.cx = 450;
	m_col_points.cx = 150;

	m_col_name.pszText = _T("Action Name");
	m_col_points.pszText = _T("Points");

	m_action_list.InsertColumn(1, &m_col_name);
	m_action_list.InsertColumn(2, &m_col_points);

	m_action_list.DeleteColumn(0);


	loadTypeData();

	m_titleFont.CreateFontW(
		27,
		0,
		0,
		0,
		FW_NORMAL,
		FALSE,
		FALSE,
		0,
		ANSI_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_SWISS,
		_T("Arial")
	);
	m_title.SetFont(&m_titleFont);

	m_type_staticFont.CreateFontW(
		50,
		0,
		0,
		0,
		FW_NORMAL,
		FALSE,
		FALSE,
		0,
		ANSI_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_SWISS,
		_T("Arial")
	);
	m_type_static.SetFont(&m_type_staticFont);

	GetParentFrame()->RecalcLayout();
	ResizeParentToFit();
}

void CActionView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CActionView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
}


// CActionView diagnostics

#ifdef _DEBUG
void CActionView::AssertValid() const
{
	CView::AssertValid();
}

void CActionView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CWCIPointsDoc* CActionView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CWCIPointsDoc)));
	return (CWCIPointsDoc*)m_pDocument;
}
#endif //_DEBUG


// Load data for each type of action

void CActionView::loadTypeData() {

	// Title
	if (m_type == 0)
		m_type_static.SetWindowTextW(_T("Athletics"));
	else if (m_type == 1)
		m_type_static.SetWindowTextW(_T("Academics"));
	else if (m_type == 2)
		m_type_static.SetWindowTextW(_T("Activities"));

	m_action_list.DeleteAllItems();

	// Fetch data from MySQL
	try {
		sql::Driver* driver = get_driver_instance();
		std::auto_ptr<sql::Connection> con(driver->connect("localhost", "points", "points"));
		con->setSchema("points");
		
		std::auto_ptr<sql::ResultSet> res = Action::get(con.get(), m_type);
		int i = 0;
		while (res->next()) {
			std::string name = res->getString("name");
			m_action_list.InsertItem(i, CString(name.c_str()));

			std::string points = res->getString("points");
			m_action_list.SetItemText(i, 1, CString(points.c_str()));

			m_action_list.SetItemData(i, res->getInt("id"));

			++i;
		}
	}
	catch (sql::SQLException& e) {
		// Exception occured
		std::string err = "Something went wrong...\nError: " + (std::string)e.what();
		AfxMessageBox(CString(err.c_str()));
	}
}


// CActionView message handlers

// Change Action type (click on change type button)
void CActionView::OnBnClickedActionTypeChange()
{
	CActionChangeTypeDlg actionChangeTypeDlg;
	if (actionChangeTypeDlg.DoModal() == IDOK) {
		m_type = actionChangeTypeDlg.m_type;
		loadTypeData();
		Invalidate();
	}
}

// Edit Action attribute (click on list control column)
void CActionView::OnLvnColumnclickActionList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	*pResult = 0;

	if (m_action_list.GetSelectionMark() == -1)
		return;

	try {
		sql::Driver* driver = get_driver_instance();
		std::auto_ptr<sql::Connection> con(driver->connect("localhost", "points", "points"));
		con->setSchema("points");

		CString title = _T("Edit Action");

		if (pNMLV->iSubItem == 0) {
			CString caption = _T("Edit Name");
			CString value = m_action_list.GetItemText(m_action_list.GetSelectionMark(), 0);

			CEditStringDlg editStringDlg(title, caption, value);
			if (editStringDlg.DoModal() == IDOK) {
				// Update database
				Action::edit_name(con.get(), m_action_list.GetItemData(m_action_list.GetSelectionMark()), editStringDlg.m_value);
				// Update UI
				m_action_list.SetItemText(m_action_list.GetSelectionMark(), 0, editStringDlg.m_value);
				Invalidate();
			}
		}
		else {
			CString caption = _T("Edit Points");
			int value = _ttoi(m_action_list.GetItemText(m_action_list.GetSelectionMark(), 1));

			CEditIntDlg editIntDlg(title, caption, value);
			if (editIntDlg.DoModal() == IDOK) {
				// Update database
				Action::edit_points(con.get(), m_action_list.GetItemData(m_action_list.GetSelectionMark()), editIntDlg.m_value);
				// Update UI
				CString update;
				update.Format(_T("%d"), editIntDlg.m_value);
				m_action_list.SetItemText(m_action_list.GetSelectionMark(), 1, update);
				Invalidate();
			}
		}
	}
	catch (sql::SQLException& e) {
		// Exception occured
		std::string err = "Something went wrong...\nError: " + (std::string)e.what();
		AfxMessageBox(CString(err.c_str()));
	}
}

// Keep track of current selection for other functions (select row in list control)
void CActionView::OnLvnItemchangedActionList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	if (pNMLV->uChanged & LVIF_STATE) {
		if (pNMLV->uNewState & LVIS_SELECTED)
			selectionMark = pNMLV->iItem;
		else
			selectionMark = -1;
	}

	*pResult = 0;
}

// Remove an action, or archive it if unable
void CActionView::OnBnClickedRemoveAction() //TODO: fix this, see if i can directly selection mark instead of storing it
{
	if (selectionMark == -1)
		return;

	try {
		sql::Driver* driver = get_driver_instance();
		std::auto_ptr<sql::Connection> con(driver->connect("localhost", "points", "points"));
		con->setSchema("points");

		if (Action::autoremove(con.get(), m_action_list.GetItemData(selectionMark))) {
			loadTypeData();
			AfxMessageBox(_T("Action deleted."));
			m_action_list.Invalidate();
		}
		else {
			loadTypeData();
			AfxMessageBox(_T("Action moved to archive."));
			m_action_list.Invalidate();
		}
	}
	catch (sql::SQLException& e) {
		// Exception occured
		std::string err = "Something went wrong...\nError: " + (std::string)e.what();
		AfxMessageBox(CString(err.c_str()));
	}
}




// ActionChangeTypeDlg dialog

IMPLEMENT_DYNAMIC(CActionChangeTypeDlg, CDialogEx)

CActionChangeTypeDlg::CActionChangeTypeDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ACTION_CHANGE_TYPE, pParent)
	, m_type(0)
{

}

CActionChangeTypeDlg::~CActionChangeTypeDlg()
{
}

BOOL CActionChangeTypeDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_type_list.AddString(_T("Athletics"));
	m_type_list.AddString(_T("Academics"));
	m_type_list.AddString(_T("Activities"));
	m_type_list.SetCurSel(0);

	return TRUE;
}

void CActionChangeTypeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ACTION_TYPE_LIST, m_type_list);
}


BEGIN_MESSAGE_MAP(CActionChangeTypeDlg, CDialogEx)
	ON_LBN_SELCHANGE(IDC_ACTION_TYPE_LIST, &CActionChangeTypeDlg::OnLbnSelchangeActionTypeList)
END_MESSAGE_MAP()


// ActionChangeTypeDlg message handlers

void CActionChangeTypeDlg::OnLbnSelchangeActionTypeList()
{
	m_type = m_type_list.GetCurSel() == LB_ERR ? 0 : m_type_list.GetCurSel();
}
