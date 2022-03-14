
// AwardView.cpp : implementation of the CAwardView class

#include "pch.h"
#include "framework.h"
#include "custommfc.h"

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

#include "AwardView.h"
#include "Action.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



IMPLEMENT_DYNCREATE(CAwardView, CFormView)

BEGIN_MESSAGE_MAP(CAwardView, CFormView)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_BN_CLICKED(IDC_CREATE_CONSTRAINT, &CAwardView::OnBnClickedCreateConstraint)
	ON_BN_CLICKED(IDC_REMOVE_CONSTRAINT, &CAwardView::OnBnClickedRemoveConstraint)
	ON_BN_CLICKED(IDC_ACCESS_ARCHIVE, &CAwardView::OnBnClickedAccessArchive)
	ON_BN_CLICKED(IDC_TOGGLE_ONLY_AWARD, &CAwardView::OnBnClickedToggleOnlyAward)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_CONSTRAINT_LIST, &CAwardView::OnLvnItemchangedConstraintList)
	ON_NOTIFY(TVN_ITEMCHANGING, IDC_CONSTRAINT_TREE, &CAwardView::OnTvnItemChangingConstraintTree)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_CONSTRAINT_TREE, &CAwardView::OnNMCustomdrawConstraintTree)
	ON_BN_CLICKED(IDC_EDIT_CONSTRAINT, &CAwardView::OnBnClickedEditConstraint)
END_MESSAGE_MAP()



CAwardView::CAwardView() noexcept
	: CFormView(IDD_AWARD)
	, selectionMark(-1)
{
	// TODO: add construction code here

}

CAwardView::~CAwardView()
{
}

void CAwardView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TITLE, m_title);
	DDX_Control(pDX, IDC_CONSTRAINT_LIST, m_constraint_list);
	DDX_Control(pDX, IDC_CONSTRAINT_TREE, m_constraint_tree);
	DDX_Control(pDX, IDC_TOGGLE_ONLY_AWARD, m_see_awards_only_check);
}

BOOL CAwardView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

void CAwardView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	// Default created with WS_EX_CLIENTEDGE for some reason
	ModifyStyleEx(WS_EX_CLIENTEDGE, 0);

	m_constraint_list.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	m_constraint_list.InsertColumn(0, _T("Name"), LVCFMT_LEFT, 180, 0);
	m_constraint_list.InsertColumn(1, _T("Description"), LVCFMT_LEFT, 180, 0);

	loadConstraintList();

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


	GetParentFrame()->RecalcLayout();
	ResizeParentToFit();
}

void CAwardView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CAwardView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
}


// CActionView diagnostics

#ifdef _DEBUG
void CAwardView::AssertValid() const
{
	CView::AssertValid();
}

void CAwardView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CWCIPointsDoc* CAwardView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CWCIPointsDoc)));
	return (CWCIPointsDoc*)m_pDocument;
}
#endif //_DEBUG

// Load constraints into list control and vector
// Set only_awards = true if list should only load awards
void CAwardView::loadConstraintList() {

	bool only_awards = m_see_awards_only_check.GetCheck() == BST_CHECKED;

	m_constraint_tree.DeleteAllItems();
	m_constraint_list.DeleteAllItems();
	m_constraint_vector.clear();
	selectionMark = -1;

	// Fetch data from MySQL
	try {
		sql::Driver* driver = get_driver_instance();
		std::auto_ptr<sql::Connection> con(driver->connect("localhost", "points", "points"));
		con->setSchema("points");

		std::auto_ptr<sql::ResultSet> res = Constraint::get(con.get(), only_awards);
		int i = 0;
		while (res->next()) {
			std::string name = res->getString("name");
			m_constraint_list.InsertItem(i, CString(name.c_str()));

			std::string desc = res->getString("description");
			m_constraint_list.SetItemText(i, 1, CString(desc.c_str()));

			// Set item data to be index of corresponding C object in m_constraint_vector
			// Theoretically, the indices of both should be the same, but this prevents the case where order of m_constraint_list somehow changes
			m_constraint_list.SetItemData(i, i);

			int id = res->getInt("id");

			int type = res->getInt("type");

			// Since we need to keep track of both id and type, only setting item data isn't enough, so we also have a constraint vector
			m_constraint_vector.push_back({ id, type, only_awards ? true : res->getBoolean("award") });

			++i;
		}
	}
	catch (sql::SQLException& e) {
		// Exception occured
		std::string err = "Something went wrong...\nError: " + (std::string)e.what();
		AfxMessageBox(CString(err.c_str()));
	}
}

// Load current constraint into tree control
void CAwardView::loadConstraintTree() {
	if (selectionMark == -1)
		return;

	m_constraint_tree.DeleteAllItems();

	CString root_name = m_constraint_list.GetItemText(selectionMark, 0) + _T(": ") + m_constraint_list.GetItemText(selectionMark, 1);

	// TODO: Figure out why the entire row is empty here but not in the one below
	/*
	HTREEITEM cnst = m_constraint_tree.InsertItem(
		TVIF_TEXT & TVIF_STATE,
		name,
		0,
		0,
		TVIS_EXPANDED,
		TVIS_EXPANDED,
		0,
		NULL,
		NULL
	);
	*/

	// Use root_handle to add sub-constraints as children in the constraint tree
	HTREEITEM root_handle = m_constraint_tree.InsertItem(root_name);

	Constraint::C root_constraint = m_constraint_vector[m_constraint_list.GetItemData(selectionMark)];

	// Add sub-constraints to constraint tree if root_constraint is a compound constraint
	// Otherwise, add additonal information if root_constraint is a basic constraint
	try {
		sql::Driver* driver = get_driver_instance();
		std::auto_ptr<sql::Connection> con(driver->connect("localhost", "points", "points"));
		con->setSchema("points");

		std::auto_ptr<sql::ResultSet> res;
		if (root_constraint.type == 0 || root_constraint.type == 1) { // Compound constraint: add subconstraints
			res = Constraint::get_compound(con.get(), root_constraint.id);

			if (root_constraint.type == 0)
				m_constraint_tree.InsertItem(CString(_T("Info: Must satisfy at least one of the criteria below: ")), root_handle);
			else
				m_constraint_tree.InsertItem(CString(_T("Info: Must satisfy all of the criteria below: ")), root_handle);
			m_constraint_tree.InsertItem(CString(_T("------------------------------------------------------------------------------")), root_handle);

			bool first_row = true;
			while (res->next()) {
				std::string name = res->getString("name");
				std::string pref = "             ";
				if (!first_row) {
					if (root_constraint.type == 0)
						pref = "OR      ";
					else if (root_constraint.type == 1)
						pref = "AND   ";
				}

				CString child_name = CString(pref.c_str()) + CString(name.c_str());

				m_constraint_tree.InsertItem(child_name, root_handle);

				first_row = false;
			}
		}
		else { // Basic constraint: add extra info
			res = Constraint::get_basic(con.get(), root_constraint.id);
			int action_id = -1, action_type = -1;
			std::string mx = "", x = "", y = "";
			if (res->next()) { // Should always be true
				action_id = res->getInt("actn_id");
				if (res->wasNull())
					action_id = -1;

				action_type = res->getInt("actn_type");
				if (res->wasNull())
					action_type = -1;

				mx = res->getString("mx");
				if (res->wasNull())
					mx = -1;

				x = res->getString("x");

				y = res->getString("y");
			}

			std::string action_name, action_type_name;
			if (action_id > 0) {
				res = Action::info(con.get(), action_id);
				action_name = res->getString("name");
				// Set action_type_name anyway, since we need to display the information later
				switch (res->getInt("type")) {
				case 0:
					action_type_name = "Athletics";
					break;
				case 1:
					action_type_name = "Academics";
					break;
				case 2:
					action_type_name = "Activities";
					break;
				}
			}
			if (action_type >= 0) {
				switch (action_type) {
				case 0:
					action_type_name = "Athletics";
					break;
				case 1:
					action_type_name = "Academics";
					break;
				case 2:
					action_type_name = "Activities";
					break;
				}
			}

			if (root_constraint.type == 2) { // Action sum
				m_constraint_tree.InsertItem(CString(_T("Info: Find out sum of ") + CString(action_type_name.c_str()) + _T(" points")), root_handle); // Description
				m_constraint_tree.InsertItem(CString(_T("Max points per year: ") + CString(mx.c_str())), root_handle);
				m_constraint_tree.InsertItem(CString(_T("Points: at least ") + CString(x.c_str()) + _T(", at most ") + CString(y.c_str())), root_handle);
			}
			else if (root_constraint.type == 3) { // Grad years
				m_constraint_tree.InsertItem(CString(_T("Info: Find years taken to graduate")), root_handle); // Description
				m_constraint_tree.InsertItem(CString(_T("Years in school: at least ") + CString(x.c_str()) + _T(", at most ") + CString(y.c_str())), root_handle);
			}
			else if (root_constraint.type == 4) { // Frequency
				if (action_id > 0 && action_type < 0) { // Frequency of a specific action
					m_constraint_tree.InsertItem(CString(_T("Info: Find out frequency of a specific ") + CString(action_type_name.c_str())), root_handle); // Description
					m_constraint_tree.InsertItem(CString(
						_T("The student must have participated in the ") +
						CString(action_type_name.c_str()) +
						_T(" given below, the specified number of times")
					), root_handle);
					m_constraint_tree.InsertItem(CString(_T("Name: ") + CString(action_name.c_str())), root_handle); // Description
					m_constraint_tree.InsertItem(CString(_T("Times participated: at least ") + CString(x.c_str()) + _T(", at most ") + CString(y.c_str())), root_handle);
				}
				else if (action_id > 0 && action_type >= 0) { // Frequency of some specific action of the given type 
					m_constraint_tree.InsertItem(CString(_T("Info: Find out frequency of some specific ") + CString(action_type_name.c_str())), root_handle); // Description
					m_constraint_tree.InsertItem(CString(
						_T("As long as the student participates in the same ") +
						CString(action_type_name.c_str()) +
						_T(" the specified number of times, this is satisfied")
					), root_handle);
					m_constraint_tree.InsertItem(CString(_T("Times participated: at least ") + CString(x.c_str()) + _T(", at most ") + CString(y.c_str())), root_handle);
				}
				else if (action_id < 0 && action_type >= 0) { // Frequency of any action of the given type
					m_constraint_tree.InsertItem(CString(_T("Info: Find out frequency of any ") + CString(action_type_name.c_str())), root_handle); // Description
					m_constraint_tree.InsertItem(CString(
						_T("As long as the student participates in any ") +
						CString(action_type_name.c_str()) +
						_T(" the specified number of times, this is satisfied")
					), root_handle);
					m_constraint_tree.InsertItem(CString(_T("Times participated: at least ") + CString(x.c_str()) + _T(", at most ") + CString(y.c_str())), root_handle);
				}
			}
			else if (root_constraint.type == 5) { // Consecutive
				if (action_id > 0 && action_type < 0) { // Consecutive occurrences of a specific action
					m_constraint_tree.InsertItem(CString(_T("Info: Find out consecutive occurrences of a specific ") + CString(action_type_name.c_str())), root_handle); // Description
					m_constraint_tree.InsertItem(CString(
						_T("The student must participate in the ") +
						CString(action_type_name.c_str()) +
						_T(" given below, the specified number of years in a row")
					), root_handle);
					m_constraint_tree.InsertItem(CString(_T("Must participate in: ") + CString(action_name.c_str())), root_handle); // Description
					m_constraint_tree.InsertItem(CString(_T("Consecutive occurrences: at least ") + CString(x.c_str()) + _T(", at most ") + CString(y.c_str())), root_handle);
				}
				else if (action_id > 0 && action_type >= 0) { // Consecutive occurrences of some specific action of the given type 
					m_constraint_tree.InsertItem(CString(_T("Info: Find out consecutive occurrences of some specific ") + CString(action_type_name.c_str())), root_handle); // Description
					m_constraint_tree.InsertItem(CString(
						_T("As long as the student participates in the same ") +
						CString(action_type_name.c_str()) +
						_T(" the specified number of years in a row, this is satisfied")
					), root_handle);
					m_constraint_tree.InsertItem(CString(_T("Consecutive occurrences: at least ") + CString(x.c_str()) + _T(", at most ") + CString(y.c_str())), root_handle);
				}
				else if (action_id < 0 && action_type >= 0) { // Consecutive occurrences of any action of the given type
					m_constraint_tree.InsertItem(CString(_T("Info: Find out consecutive occurrences of any ") + CString(action_type_name.c_str())), root_handle); // Description
					m_constraint_tree.InsertItem(CString(
						_T("As long as the student participates in any ") +
						CString(action_type_name.c_str()) +
						_T(" the specified number of years in a row, this is satisfied")
					), root_handle);
					m_constraint_tree.InsertItem(CString(_T("Consecutive occurrences: at least ") + CString(x.c_str()) + _T(", at most ") + CString(y.c_str())), root_handle);
				}
			}
		}
	}
	catch (sql::SQLException& e) {
		// Exception occured
		std::string err = "Something went wrong...\nError: " + (std::string)e.what();
		AfxMessageBox(CString(err.c_str()));
	}

	m_constraint_tree.Expand(root_handle, TVE_EXPAND);

	Invalidate();
}


// CActionView message handlers

// Toggle whether the constraint list control only shows awards
void CAwardView::OnBnClickedToggleOnlyAward()
{
	loadConstraintList();
}

// Keep track of constraint list selection mark, and change the constraint displayed in the tree control to be the selected constraint
// Selection mark stored as a field because the list control doesn't reset the selection mark when user deselects row
void CAwardView::OnLvnItemchangedConstraintList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	if (pNMLV->uChanged & LVIF_STATE) {
		if (pNMLV->uNewState & LVIS_SELECTED) {
			selectionMark = pNMLV->iItem;
			loadConstraintTree();
		}
		else {
			selectionMark = -1;
			m_constraint_tree.DeleteAllItems();
		}
	}

	*pResult = 0;
}

// Handle custom draw of the tree control to support word wrap (since description should be a new line)
// TODO: finish later, for now, simply do default
void CAwardView::OnNMCustomdrawConstraintTree(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	
	*pResult = CDRF_DODEFAULT;
	/*
	switch (pNMCD->dwDrawStage) {
	case CDDS_PREPAINT:
		*pResult = CDRF_NOTIFYITEMDRAW;
		break;
	case CDDS_ITEMPREPAINT:
		*pResult = CDRF_NOTIFYPOSTPAINT;
		break;
	case CDDS_ITEMPOSTPAINT:
		{
			CDC* pDC = CDC::FromHandle(pNMCD->hdc);
			HTREEITEM item = (HTREEITEM)pNMCD->dwItemSpec;

			LPRECT rc = nullptr; // is rc copied or is it the original rect? if it's copied, I don't need temprc
			RECT temprc;
			m_constraint_tree.GetItemRect(item, rc, 0); // TODO: why isn't rc added into here?
			temprc = *rc;
			CString txt = m_constraint_tree.GetItemText(item);

			int curpos = 0;
			CString tok;
			for (tok = txt.Tokenize(_T(":"), curpos); tok != _T(""); tok = txt.Tokenize(_T(":"), curpos)) {
				pDC->DrawText(tok, &temprc, DT_LEFT);
				temprc.bottom += temprc.bottom - temprc.top;
				temprc.top = (temprc.top + temprc.bottom) / 2;
				rc->bottom = temprc.bottom;
			}
		//TODO: FINISH THIS
		}
		*pResult = CDRF_NEWFONT;
		break;
	default:
		*pResult = CDRF_DODEFAULT;
	}
	*/
}

// Make it so you can't select items on the tree control (should be used only as an information reference)
void CAwardView::OnTvnItemChangingConstraintTree(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMTVITEMCHANGE* pNMTVItemChange = reinterpret_cast<NMTVITEMCHANGE*>(pNMHDR);
	*pResult = 1;
}

// Create new Constraint
void CAwardView::OnBnClickedCreateConstraint()
{
	// TODO: Add your control notification handler code here
}

// Remove constraint, or archive if it's an award that's been assigned before
void CAwardView::OnBnClickedRemoveConstraint()
{
	if (selectionMark == -1)
		return;

	try {
		sql::Driver* driver = get_driver_instance();
		std::auto_ptr<sql::Connection> con(driver->connect("localhost", "points", "points"));
		con->setSchema("points");

		Constraint::C current_cnst = m_constraint_vector[m_constraint_list.GetItemData(selectionMark)];

		if (Constraint::autoremove(con.get(), current_cnst.id)) {
			loadConstraintList();
			AfxMessageBox(_T("Award/Constraint deleted."));
		}
		else {
			loadConstraintList();
			AfxMessageBox(_T("Students have received this award in the past!\nAward moved to archive instead."));
		}

		selectionMark = -1;
	}
	catch (sql::SQLException& e) {
		// Exception occured
		std::string err = "Something went wrong...\nError: " + (std::string)e.what();
		AfxMessageBox(CString(err.c_str()));
	}
}

void CAwardView::OnBnClickedEditConstraint()
{
	// TODO: Add your control notification handler code here
}

// Access the archive menu
void CAwardView::OnBnClickedAccessArchive()
{
	CConstraintArchiveDlg constraintArchiveDlg;
	constraintArchiveDlg.DoModal();
	loadConstraintList();
}






// Action Archive dialog

IMPLEMENT_DYNAMIC(CConstraintArchiveDlg, CDialogEx)

CConstraintArchiveDlg::CConstraintArchiveDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ARCHIVE, pParent)
	, selectionMark(-1)
{

}

CConstraintArchiveDlg::~CConstraintArchiveDlg()
{
}

BOOL CConstraintArchiveDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_constraint_list.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_constraint_list.InsertColumn(0, _T("Constraints"), LVCFMT_LEFT, 450, 0);

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

	loadConstraintList();

	m_title.SetFont(&m_titleFont);
	m_title.SetWindowTextW(_T("Award Archive"));

	return TRUE;
}

// Load all archived awards
// Note that archived constraints are always awards, non-award constraints are directly deleted
void CConstraintArchiveDlg::loadConstraintList() {

	m_constraint_list.DeleteAllItems();

	// Fetch data from MySQL
	try {
		sql::Driver* driver = get_driver_instance();
		std::auto_ptr<sql::Connection> con(driver->connect("localhost", "points", "points"));
		con->setSchema("points");

		std::auto_ptr<sql::ResultSet> res = Constraint::get(con.get(), false, true);
		int i = 0;
		while (res->next()) {
			std::string name = res->getString("name");
			m_constraint_list.InsertItem(i, CString(name.c_str()));

			std::string desc = res->getString("description");
			m_constraint_list.SetItemText(i, 1, CString(desc.c_str()));

			// Set item data to be id
			int id = res->getInt("id");
			m_constraint_list.SetItemData(i, id);

			++i;
		}
	}
	catch (sql::SQLException& e) {
		// Exception occured
		std::string err = "Something went wrong...\nError: " + (std::string)e.what();
		AfxMessageBox(CString(err.c_str()));
	}
}

void CConstraintArchiveDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TITLE, m_title);
	DDX_Control(pDX, IDC_ARCHIVE_LIST, m_constraint_list);
}


BEGIN_MESSAGE_MAP(CConstraintArchiveDlg, CDialogEx)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_ARCHIVE_LIST, &CConstraintArchiveDlg::OnLvnItemchangedConstraintList)
	ON_BN_CLICKED(IDC_ARCHIVE_UNARCHIVE, &CConstraintArchiveDlg::OnBnClickedUnarchiveConstraint)
	ON_BN_CLICKED(IDC_ARCHIVE_REMOVE, &CConstraintArchiveDlg::OnBnClickedRemoveConstraint)
END_MESSAGE_MAP()


// CConstraintArchiveDlg message handlers

// Keep track of current selection for other functions (select row in list control)
// Selection mark stored as a field because the list control doesn't reset the selection mark when user deselects row
void CConstraintArchiveDlg::OnLvnItemchangedConstraintList(NMHDR* pNMHDR, LRESULT* pResult)
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

// Unarchive an award
void CConstraintArchiveDlg::OnBnClickedUnarchiveConstraint()
{
	if (selectionMark == -1)
		return;

	try {
		sql::Driver* driver = get_driver_instance();
		std::auto_ptr<sql::Connection> con(driver->connect("localhost", "points", "points"));
		con->setSchema("points");

		Constraint::unarchive(con.get(), m_constraint_list.GetItemData(selectionMark));
		loadConstraintList();
		selectionMark = -1;
	}
	catch (sql::SQLException& e) {
		// Exception occured
		std::string err = "Something went wrong...\nError: " + (std::string)e.what();
		AfxMessageBox(CString(err.c_str()));
	}
}

// Delete award from archive, or notify user if not possible
void CConstraintArchiveDlg::OnBnClickedRemoveConstraint()
{
	if (selectionMark == -1)
		return;

	try {
		sql::Driver* driver = get_driver_instance();
		std::auto_ptr<sql::Connection> con(driver->connect("localhost", "points", "points"));
		con->setSchema("points");

		if (Constraint::autoremove(con.get(), m_constraint_list.GetItemData(selectionMark))) {
			loadConstraintList();
			AfxMessageBox(_T("Award successfully deleted."));
			selectionMark = -1;
		}
		else {
			AfxMessageBox(_T(
				"Students have received this award in the past!\n"
				"You cannot delete this award unless you delete those students.\n"
				"It's recommended to just leave this award here."
			));
		}
	}
	catch (sql::SQLException& e) {
		// Exception occured
		std::string err = "Something went wrong...\nError: " + (std::string)e.what();
		AfxMessageBox(CString(err.c_str()));
	}
}
