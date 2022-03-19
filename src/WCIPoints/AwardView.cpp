
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
	ON_BN_CLICKED(IDC_CREATE_BASIC_CONSTRAINT, &CAwardView::OnBnClickedCreateBasicConstraint)
	ON_BN_CLICKED(IDC_REMOVE_CONSTRAINT, &CAwardView::OnBnClickedRemoveConstraint)
	ON_BN_CLICKED(IDC_ACCESS_ARCHIVE, &CAwardView::OnBnClickedAccessArchive)
	ON_BN_CLICKED(IDC_TOGGLE_ONLY_AWARD, &CAwardView::OnBnClickedToggleOnlyAward)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_CONSTRAINT_LIST, &CAwardView::OnLvnItemchangedConstraintList)
	ON_NOTIFY(TVN_ITEMCHANGING, IDC_CONSTRAINT_TREE, &CAwardView::OnTvnItemChangingConstraintTree)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_CONSTRAINT_TREE, &CAwardView::OnNMCustomdrawConstraintTree)
	ON_BN_CLICKED(IDC_EDIT_CONSTRAINT, &CAwardView::OnBnClickedEditConstraint)
	ON_BN_CLICKED(IDC_CREATE_COMPOUND_CONSTRAINT, &CAwardView::OnBnClickedCreateCompoundConstraint)
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
				m_constraint_tree.InsertItem(CString(_T("Info: Must satisfy AT LEAST 1 of the criteria below: ")), root_handle);
			else
				m_constraint_tree.InsertItem(CString(_T("Info: Must satisfy ALL of the criteria below: ")), root_handle);
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
					mx = "";

				x = res->getString("x");

				y = res->getString("y");
			}

			std::string action_name, action_type_name;
			if (action_id > 0) {
				res = Action::info(con.get(), action_id);
				if (res->next()) {
					action_name = res->getString("name");
					// Set action_type_name anyway, since we need to display the information later
					switch (res->getInt("type")) {
					case 0:
						action_type_name = "Athletic";
						break;
					case 1:
						action_type_name = "Academic";
						break;
					case 2:
						action_type_name = "Activity";
						break;
					}
				}
			}
			if (action_type >= 0) {
				switch (action_type) {
				case 0:
					action_type_name = "Athletic";
					break;
				case 1:
					action_type_name = "Academic";
					break;
				case 2:
					action_type_name = "Activity";
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

// Create new basic Constraint
void CAwardView::OnBnClickedCreateBasicConstraint()
{
	CConstraintCreateBasicDlg constraintCreateBasicDlg;
	if (constraintCreateBasicDlg.DoModal() == IDOK) {

		// Check for invalid range (must do this check after DoDataExchange is called, thus unable to move this into CConstraintCreateBasicDlg class)
		if (constraintCreateBasicDlg.m_award_x > constraintCreateBasicDlg.m_award_y) {
			AfxMessageBox(_T("Invalid range! Your lower bound was greater than your upper bound."));
			return;
		}

		try {
			sql::Driver* driver = get_driver_instance();
			std::auto_ptr<sql::Connection> con(driver->connect("localhost", "points", "points"));
			con->setSchema("points");

			int cnst_id = Constraint::add(
				con.get(),
				constraintCreateBasicDlg.m_award_type,
				constraintCreateBasicDlg.m_award_name,
				constraintCreateBasicDlg.m_award_description,
				constraintCreateBasicDlg.m_is_award
			);
			if (cnst_id == -1) {
				AfxMessageBox(_T("Something went wrong...\nThe new constraint did not return an ID."));
				return;
			}

			Constraint::add_basic(
				con.get(),
				cnst_id,
				constraintCreateBasicDlg.m_award_type,
				constraintCreateBasicDlg.m_action_id,
				constraintCreateBasicDlg.m_action_type,
				constraintCreateBasicDlg.m_award_mx,
				constraintCreateBasicDlg.m_award_x,
				constraintCreateBasicDlg.m_award_y
			);

			loadConstraintList();
		}
		catch (sql::SQLException& e) {
			// Exception occured
			std::string err;
			if (e.getErrorCode() == 1062) err = "An award/constraint with that name already exists!"; // If duplicate key name
			else err = "Something went wrong...\nError: " + (std::string)e.what();
			AfxMessageBox(CString(err.c_str()));
		}
	}
}

// Create new compound Constraint
void CAwardView::OnBnClickedCreateCompoundConstraint()
{
	CConstraintCreateCompoundDlg constraintCreateCompoundDlg;
	if (constraintCreateCompoundDlg.DoModal() == IDOK) {
		try {
			sql::Driver* driver = get_driver_instance();
			std::auto_ptr<sql::Connection> con(driver->connect("localhost", "points", "points"));
			con->setSchema("points");

			int root_cnst_id = Constraint::add(
				con.get(),
				constraintCreateCompoundDlg.m_award_type,
				constraintCreateCompoundDlg.m_award_name,
				constraintCreateCompoundDlg.m_award_description,
				constraintCreateCompoundDlg.m_is_award
			);
			if (root_cnst_id == -1) {
				AfxMessageBox(_T("Something went wrong...\nThe new constraint did not return an ID."));
				return;
			}

			for (int sub_cnst_id : constraintCreateCompoundDlg.m_constraint_ids) {
				Constraint::add_compound(con.get(), root_cnst_id, sub_cnst_id);
			}

			loadConstraintList();
		}
		catch (sql::SQLException& e) {
			// Exception occured
			std::string err;
			if (e.getErrorCode() == 1062) err = "An award/constraint with that name already exists!"; // If duplicate key name
			else err = "Something went wrong...\nError: " + (std::string)e.what();
			AfxMessageBox(CString(err.c_str()));
		}
	}
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






// Award Archive dialog

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






// Create basic constraint/award dialog

IMPLEMENT_DYNAMIC(CConstraintCreateBasicDlg, CDialogEx)

CConstraintCreateBasicDlg::CConstraintCreateBasicDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_AWARD_BASIC_CREATE, pParent)
	, m_award_name(_T(""))
	, m_award_description(_T(""))
	, m_award_type(-1)
	, selectionMarkAwardType(-1)
	, m_action_type(-1)
	, m_action_id(-1)
	, m_award_mx(0)
	, m_award_x(0)
	, m_award_y(0)
{

}

CConstraintCreateBasicDlg::~CConstraintCreateBasicDlg()
{
}

BOOL CConstraintCreateBasicDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_action_list.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_action_list.InsertColumn(0, _T("Specific actions:"), LVCFMT_LEFT, 450, 0);

	m_titleFont.CreateFontW(
		18,
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

	m_award_type_list.AddString(_T("Action sum"));
	m_award_type_list.AddString(_T("Grad year"));
	m_award_type_list.AddString(_T("Frequency (specific action)"));
	m_award_type_list.AddString(_T("Frequency (specific of a type)"));
	m_award_type_list.AddString(_T("Frequency (any of a type)"));
	m_award_type_list.AddString(_T("Consecutive (specific action)"));
	m_award_type_list.AddString(_T("Consecutive (specific of a type)"));
	m_award_type_list.AddString(_T("Consecutive (any of a type)"));

	m_action_type_list.AddString(_T("Athletics"));
	m_action_type_list.AddString(_T("Academics"));
	m_action_type_list.AddString(_T("Activities"));

	return TRUE;
}

void CConstraintCreateBasicDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IS_AWARD, m_is_award_check);
	DDX_Control(pDX, IDC_AWARD_TYPE_LIST, m_award_type_list);
	DDX_Control(pDX, IDC_DESCRIPTION, m_award_type_description);
	DDX_Text(pDX, IDC_AWARD_NAME, m_award_name);
	DDV_MaxChars(pDX, m_award_name, 64);
	DDV_MinChars(pDX, m_award_name, 1);
	DDX_Text(pDX, IDC_AWARD_DESCRIPTION, m_award_description);
	DDV_MaxChars(pDX, m_award_description, 64);
	DDX_Control(pDX, IDC_ACTION_TYPE_LIST, m_action_type_list);
	DDX_Control(pDX, IDC_ACTION_LIST, m_action_list);
	DDX_Text(pDX, IDC_AWARD_MX, m_award_mx);
	DDV_MinMaxInt(pDX, m_award_mx, 1, 63);
	DDX_Text(pDX, IDC_AWARD_X, m_award_x);
	DDV_MinMaxInt(pDX, m_award_x, 0, 255);
	DDX_Text(pDX, IDC_AWARD_Y, m_award_y);
	DDV_MinMaxInt(pDX, m_award_y, 0, 255);
	DDX_Control(pDX, IDC_TITLE, m_title);
}

// Perform necessary validation that isn't covered by DDV validators
void CConstraintCreateBasicDlg::OnOK()
{
	// Ensure award type is selected from listbox, and type is valid
	if (selectionMarkAwardType == -1) {
		AfxMessageBox(_T("Select an award/constraint type! (e.g, Action sum)"));
		return;
	}

	if (selectionMarkAwardType == 0) { // Action sum
		m_award_type = 2;
		
		if (m_action_type < 0) {
			AfxMessageBox(_T("Select an action type! (e.g, Athletics)"));
			return;
		}
	}
	else if (selectionMarkAwardType == 1) { // Grad years
		m_award_type = 3;
		m_award_mx = -1;
	}
	else if (selectionMarkAwardType <= 4) { // Frequency
		m_award_type = 4;
		m_award_mx = -1;

		if (m_action_type < 0) {
			AfxMessageBox(_T("Select an action type! (e.g, Athletics)"));
			return;
		}

		if (selectionMarkAwardType == 2) { // Specific action
			if (m_action_id < 0) {
				AfxMessageBox(_T("Select a specific action from the bottom left!"));
				return;
			}
			m_action_type_list.SetCurSel(-1);
			m_action_type = -1;
		}
		else if (selectionMarkAwardType == 3) { // Specific of type
			m_action_id = 1; // Set action id to some positive value, since it is ignored but needs to be set
		}
		else { // Any of type
			m_action_id = -1;
		}
	}
	else { // Consecutive
		m_award_type = 5;
		m_award_mx = -1;

		if (m_action_type < 0) {
			AfxMessageBox(_T("Select an action type! (e.g, Athletics)"));
			return;
		}

		if (selectionMarkAwardType == 5) { // Specific action
			if (m_action_id < 0) {
				AfxMessageBox(_T("Select a specific action from the bottom left!"));
				return;
			}
			m_action_type_list.SetCurSel(-1);
			m_action_type = -1;
		}
		else if (selectionMarkAwardType == 6) { // Specific of type
			m_action_id = 1; // Set action id to some positive value, since it is ignored but needs to be set
		}
		else { // Any of type
			m_action_id = -1;
		}
	}

	m_is_award = m_is_award_check.GetCheck() == BST_CHECKED;

	CDialog::OnOK();
}

// Wipe action list when disabling it
void CConstraintCreateBasicDlg::disableActionList() {
	m_action_list.DeleteAllItems();
	m_action_id = -1;
	GetDlgItem(IDC_ACTION_LIST)->EnableWindow(FALSE);
}

// Restore action list when enabling it if action type is selected
void CConstraintCreateBasicDlg::enableActionList() {
	GetDlgItem(IDC_ACTION_LIST)->EnableWindow(TRUE);
	if (m_action_type > -1)
		loadTypeData();
}

// Load data for type of action
void CConstraintCreateBasicDlg::loadTypeData() {

	// Only proceed if window is enabled (i.e, it applies to current type)
	if (!GetDlgItem(IDC_ACTION_LIST)->IsWindowEnabled())
		return;

	// Reset items
	m_action_list.DeleteAllItems();
	m_action_id = -1;

	// Fetch data from MySQL
	try {
		sql::Driver* driver = get_driver_instance();
		std::auto_ptr<sql::Connection> con(driver->connect("localhost", "points", "points"));
		con->setSchema("points");

		std::auto_ptr<sql::ResultSet> res = Action::get(con.get(), m_action_type, false);
		int i = 0;
		while (res->next()) {
			std::string msg = (std::string)res->getString("name") + "    (" + (std::string)res->getString("points") + " points)";
			m_action_list.InsertItem(i, CString(msg.c_str()));

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


BEGIN_MESSAGE_MAP(CConstraintCreateBasicDlg, CDialogEx)
	ON_LBN_SELCHANGE(IDC_AWARD_TYPE_LIST, &CConstraintCreateBasicDlg::OnLbnSelchangeAwardTypeList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_ACTION_LIST, &CConstraintCreateBasicDlg::OnLvnItemchangedActionList)
	ON_LBN_SELCHANGE(IDC_ACTION_TYPE_LIST, &CConstraintCreateBasicDlg::OnLbnSelchangeActionTypeList)
END_MESSAGE_MAP()


// CConstraintCreateBasicDlg message handlers

// Selected award type changes
void CConstraintCreateBasicDlg::OnLbnSelchangeAwardTypeList()
{
	if (m_award_type_list.GetCurSel() == LB_ERR) {
		selectionMarkAwardType = -1;
		m_award_type_description.SetWindowTextW(_T(""));
		GetDlgItem(IDC_ACTION_TYPE_LIST)->EnableWindow(FALSE);
		disableActionList();
		GetDlgItem(IDC_AWARD_MX)->EnableWindow(FALSE);
	}
	else {
		selectionMarkAwardType = m_award_type_list.GetCurSel();
		switch (selectionMarkAwardType) {
		case 0: // Action sum
			m_award_type_description.SetWindowTextW(_T(
				"This award/constraint checks whether the sum of action points (e.g, Athletic points) is in the specified range."
			));

			GetDlgItem(IDC_ACTION_TYPE_LIST)->EnableWindow(TRUE);
			GetDlgItem(IDC_AWARD_MX)->EnableWindow(TRUE);

			disableActionList();
			break;
		case 1: // Grad year
			m_award_type_description.SetWindowTextW(_T(
				"This award/constraint checks whether the years the student takes to graduate is in the specified range."
			));
			GetDlgItem(IDC_ACTION_TYPE_LIST)->EnableWindow(FALSE);
			disableActionList();
			GetDlgItem(IDC_AWARD_MX)->EnableWindow(FALSE);
			GetDlgItem(IDC_AWARD_MX)->SetWindowText(_T("1")); // Set mx to 1 to pass DDV validator
			break;
		case 2: // Frequency (specific action)
			m_award_type_description.SetWindowTextW(_T(
				"This award/constraint checks whether the frequency of a certain action is in the specified range. "
				"Choose a specific action from below (on the left). The student must have participated in it a certain number of times."
			));

			GetDlgItem(IDC_ACTION_TYPE_LIST)->EnableWindow(TRUE);
			enableActionList();

			GetDlgItem(IDC_AWARD_MX)->EnableWindow(FALSE);
			GetDlgItem(IDC_AWARD_MX)->SetWindowText(_T("1")); // Set mx to 1 to pass DDV validator
			break;
		case 3: // Frequency (specific of a type)
			m_award_type_description.SetWindowTextW(_T(
				"This award/constraint checks whether the frequency of a specific action of a certain type in the specified range. "
				"Choose an action type from below (on the right). The student must have participated in the same action of that type a certain number of times."
			));

			GetDlgItem(IDC_ACTION_TYPE_LIST)->EnableWindow(TRUE);

			// Even though the action_id is needed to indicate this type, the user should not be choosing it
			// It is inserted at the end, when validation of all values occurs
			disableActionList();
			GetDlgItem(IDC_AWARD_MX)->EnableWindow(FALSE);
			GetDlgItem(IDC_AWARD_MX)->SetWindowText(_T("1")); // Set mx to 1 to pass DDV validator
			break;
		case 4: // Frequency (any of a type)
			m_award_type_description.SetWindowTextW(_T(
				"This award/constraint checks whether the frequency of any action certain type is in the specified range. "
				"Choose an action type from below (on the right). The student must have participated in actions of that type a certain number of times."
			));

			GetDlgItem(IDC_ACTION_TYPE_LIST)->EnableWindow(TRUE);

			disableActionList();
			GetDlgItem(IDC_AWARD_MX)->EnableWindow(FALSE);
			GetDlgItem(IDC_AWARD_MX)->SetWindowText(_T("1")); // Set mx to 1 to pass DDV validator
			break;
		case 5: // Consecutive (specific action)
			m_award_type_description.SetWindowTextW(_T(
				"This award/constraint checks whether a student has participated in a certain action for some years in a row, where the number of years is in the specified range. "
				"Choose a specific action from below (on the left). The student must have participated in it a certain number of years in a row."
			));

			GetDlgItem(IDC_ACTION_TYPE_LIST)->EnableWindow(TRUE);
			enableActionList();

			GetDlgItem(IDC_AWARD_MX)->EnableWindow(FALSE);
			GetDlgItem(IDC_AWARD_MX)->SetWindowText(_T("1")); // Set mx to 1 to pass DDV validator
			break;
		case 6: // Consecutive (specific of a type)
			m_award_type_description.SetWindowTextW(_T(
				"This award/constraint checks whether a student has participated in the same action of a specific type for some years in a row, where the number of years is in the specified range. "
				"Choose an action type from below (on the right). The student must have participated in the same action of that type a certain number of years in a row."
			));

			GetDlgItem(IDC_ACTION_TYPE_LIST)->EnableWindow(TRUE);

			// Even though the action_id is needed to indicate this type, the user should not be choosing it
			// It is inserted at the end, when validation of all values occurs
			disableActionList();
			GetDlgItem(IDC_AWARD_MX)->EnableWindow(FALSE);
			GetDlgItem(IDC_AWARD_MX)->SetWindowText(_T("1")); // Set mx to 1 to pass DDV validator
			break;
		case 7: // Consecutive (any of a type)
			m_award_type_description.SetWindowTextW(_T(
				"This award/constraint checks whether a student has participated in the any action of a specific type for some years in a row, where the number of years is in the specified range. "
				"Choose an action type from below (on the right). The student must have participated in actions of that type a certain number of years in a row."
			));

			GetDlgItem(IDC_ACTION_TYPE_LIST)->EnableWindow(TRUE);

			disableActionList();
			GetDlgItem(IDC_AWARD_MX)->EnableWindow(FALSE);
			GetDlgItem(IDC_AWARD_MX)->SetWindowText(_T("1")); // Set mx to 1 to pass DDV validator
			break;
		default: // Should not happen
			selectionMarkAwardType = -1;
			m_award_type_description.SetWindowTextW(_T(""));
		}
	}
}

// Selected action type changes
void CConstraintCreateBasicDlg::OnLbnSelchangeActionTypeList()
{
	if (m_action_type_list.GetCurSel() == LB_ERR)
		m_action_type = -1;
	else
		m_action_type = m_action_type_list.GetCurSel();

	loadTypeData();
}

// Selected action changes
void CConstraintCreateBasicDlg::OnLvnItemchangedActionList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	if (pNMLV->uChanged & LVIF_STATE) {
		if (pNMLV->uNewState & LVIS_SELECTED) {
			int selectionMarkAction = pNMLV->iItem;
			m_action_id = m_action_list.GetItemData(selectionMarkAction);
		}
		else {
			m_action_id = -1;
		}
	}

	*pResult = 0;
}






// CConstraintCreateCompoundDlg dialog

IMPLEMENT_DYNAMIC(CConstraintCreateCompoundDlg, CDialogEx)

CConstraintCreateCompoundDlg::CConstraintCreateCompoundDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_AWARD_COMPOUND_CREATE, pParent)
	, m_award_name(_T(""))
	, m_award_description(_T(""))
	, m_award_type(-1)
{

}

CConstraintCreateCompoundDlg::~CConstraintCreateCompoundDlg()
{
}

BOOL CConstraintCreateCompoundDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_constraint_list.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_constraint_list.InsertColumn(0, _T("Awards/Constraints:"), LVCFMT_LEFT, 450, 0);

	m_titleFont.CreateFontW(
		18,
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

	m_award_type_list.AddString(_T("OR (at least 1)"));
	m_award_type_list.AddString(_T("AND (all of)"));

	loadConstraintList();

	return TRUE;
}

void CConstraintCreateCompoundDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IS_AWARD, m_is_award_check);
	DDX_Control(pDX, IDC_TITLE, m_title);
	DDX_Control(pDX, IDC_AWARD_TYPE_LIST, m_award_type_list);
	DDX_Control(pDX, IDC_DESCRIPTION, m_award_type_description);
	DDX_Text(pDX, IDC_AWARD_NAME, m_award_name);
	DDV_MaxChars(pDX, m_award_name, 64);
	DDV_MinChars(pDX, m_award_name, 1);
	DDX_Text(pDX, IDC_AWARD_DESCRIPTION, m_award_description);
	DDV_MaxChars(pDX, m_award_description, 64);
	DDX_Control(pDX, IDC_CONSTRAINT_LIST, m_constraint_list);
}

void CConstraintCreateCompoundDlg::OnOK()
{
	// Ensure award type is selected from listbox, and type is valid
	if (m_award_type == -1) {
		AfxMessageBox(_T("Select an award/constraint type! (e.g, OR: at least one)"));
		return;
	}

	POSITION pos = m_constraint_list.GetFirstSelectedItemPosition();
	if (pos == NULL) {
		AfxMessageBox(_T("Select at least one criterion (award/constraint) from the bottom left!"));
		return;
	}

	m_constraint_ids.clear();
	while (pos) {
		int nItem = m_constraint_list.GetNextSelectedItem(pos);
		m_constraint_ids.push_back(m_constraint_list.GetItemData(nItem));
	}

	m_is_award = m_is_award_check.GetCheck() == BST_CHECKED;

	CDialog::OnOK();
}

// Load all constraints for user to select from
void CConstraintCreateCompoundDlg::loadConstraintList() {

	m_constraint_list.DeleteAllItems();

	// Fetch data from MySQL
	try {
		sql::Driver* driver = get_driver_instance();
		std::auto_ptr<sql::Connection> con(driver->connect("localhost", "points", "points"));
		con->setSchema("points");

		std::auto_ptr<sql::ResultSet> res = Constraint::get(con.get());
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


BEGIN_MESSAGE_MAP(CConstraintCreateCompoundDlg, CDialogEx)
	ON_LBN_SELCHANGE(IDC_AWARD_TYPE_LIST, &CConstraintCreateCompoundDlg::OnLbnSelchangeAwardTypeList)
END_MESSAGE_MAP()


// CConstraintCreateCompoundDlg message handlers

void CConstraintCreateCompoundDlg::OnLbnSelchangeAwardTypeList() {
	if (m_award_type_list.GetCurSel() == LB_ERR) {
		m_award_type = -1;
		m_award_type_description.SetWindowTextW(_T(""));
	}
	else {
		m_award_type = m_award_type_list.GetCurSel(); // 0 for OR constraint, 1 for AND constraint
		if (m_award_type == 0)
			m_award_type_description.SetWindowTextW(_T(
				"Select some constraints or existing awards from the bottom left (hold CTRL while clicking to select multiple). "
				"The student must satisfy AT LEAST ONE of them for this award/constraint to be satisfied."
			));
		else
			m_award_type_description.SetWindowTextW(_T(
				"Select some constraints or existing awards from the bottom left (hold CTRL while clicking to select multiple). "
				"The student must satisfy ALL OF THEM for this award/constraint to be satisfied."
			));
	}
}