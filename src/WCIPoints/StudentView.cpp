
// StudentView.cpp : implementation of the CStudentView class

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

#include "StudentView.h"
#include "Student.h"
#include "Constraint.h"
#include "Action.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNCREATE(CStudentView, CFormView)

BEGIN_MESSAGE_MAP(CStudentView, CFormView)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_BN_CLICKED(IDC_STUDENT_YEAR_CHANGE, &CStudentView::OnBnClickedStudentYearChange)
	ON_BN_CLICKED(IDC_TOGGLE_SEE_ALL, &CStudentView::OnBnClickedToggleSeeAll)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_STUDENT_LIST, &CStudentView::OnLvnItemchangedStudentList)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_STUDENT_LIST, &CStudentView::OnLvnColumnclickStudentList)
	ON_BN_CLICKED(IDC_CREATE_STUDENT, &CStudentView::OnBnClickedCreateStudent)
	ON_BN_CLICKED(IDC_SEE_STUDENT_AWARDS, &CStudentView::OnBnClickedSeeStudentAwards)
	ON_BN_CLICKED(IDC_ASSIGN_ACTIONS, &CStudentView::OnBnClickedAssignActions)
	ON_BN_CLICKED(IDC_REMOVE_STUDENT, &CStudentView::OnBnClickedRemoveStudent)
	ON_BN_CLICKED(IDC_EVALUATE_AWARDS, &CStudentView::OnBnClickedEvaluateAwards)
	ON_BN_CLICKED(IDC_HAND_OUT_AWARDS, &CStudentView::OnBnClickedHandOutAwards)
	ON_BN_CLICKED(IDC_SEE_AWARDS, &CStudentView::OnBnClickedSeeAwards)
	ON_BN_CLICKED(IDC_EDIT_ACTIONS, &CStudentView::OnBnClickedEditActions)
END_MESSAGE_MAP()

// CStudentView construction/destruction

CStudentView::CStudentView() noexcept
	: CFormView(IDD_STUDENT)
	, selectionMark(-1)
	, m_year(-1)
{
	// TODO: add construction code here

}

CStudentView::~CStudentView()
{
}

void CStudentView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TITLE, m_title);
	DDX_Control(pDX, IDC_DESCRIPTION, m_year_static);
	DDX_Control(pDX, IDC_STUDENT_LIST, m_student_list);
	DDX_Control(pDX, IDC_TOGGLE_SEE_ALL, m_see_all_students_check);
}

BOOL CStudentView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

void CStudentView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	// Default created with WS_EX_CLIENTEDGE for some reason
	ModifyStyleEx(WS_EX_CLIENTEDGE, 0);

	// Set current year
	CTime current_time = CTime::GetCurrentTime();
	m_year = current_time.GetYear();
	m_year_static.SetWindowTextW(_T("Year: ") + current_time.Format("%Y"));

	// Set student list
	m_student_list.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	// Insert dummy column so LVCFMT_FIXED_WIDTH works properly (see Remarks of LVCOLUMN struct)
	m_student_list.InsertColumn(0, &LVCOLUMN());

	m_col_oen.mask = m_col_last_name.mask = m_col_first_name.mask = m_col_pref_name.mask = m_col_start_year.mask = m_col_grad_year.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;

	m_col_oen.fmt = m_col_last_name.fmt = m_col_first_name.fmt = m_col_pref_name.fmt = m_col_start_year.fmt = m_col_grad_year.fmt = LVCFMT_LEFT | LVCFMT_FIXED_WIDTH;

	m_col_oen.cx = 85;
	m_col_last_name.cx = 130;
	m_col_first_name.cx = 130;
	m_col_pref_name.cx = 130;
	m_col_start_year.cx = 60;
	m_col_grad_year.cx = 60;

	m_col_oen.pszText = _T("OEN");
	m_col_last_name.pszText = _T("Last name");
	m_col_first_name.pszText = _T("First name");
	m_col_pref_name.pszText = _T("Preferred name");
	m_col_start_year.pszText = _T("Start");
	m_col_grad_year.pszText = _T("Grad");

	m_student_list.InsertColumn(1, &m_col_oen);
	m_student_list.InsertColumn(2, &m_col_first_name); // Note the order of names is First, Preferred, Last
	m_student_list.InsertColumn(3, &m_col_pref_name);
	m_student_list.InsertColumn(4, &m_col_last_name);
	m_student_list.InsertColumn(5, &m_col_start_year);
	m_student_list.InsertColumn(6, &m_col_grad_year);

	m_student_list.DeleteColumn(0);


	loadStudentList();

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

	m_year_staticFont.CreateFontW(
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
	m_year_static.SetFont(&m_year_staticFont);

	GetParentFrame()->RecalcLayout();
	ResizeParentToFit();
}

void CStudentView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CStudentView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
}


// CStudentView diagnostics

#ifdef _DEBUG
void CStudentView::AssertValid() const
{
	CView::AssertValid();
}

void CStudentView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CWCIPointsDoc* CStudentView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CWCIPointsDoc)));
	return (CWCIPointsDoc*)m_pDocument;
}
#endif //_DEBUG


// Load students into list
void CStudentView::loadStudentList() {

	m_student_list.DeleteAllItems();
	selectionMark = -1;

	// Fetch data from MySQL
	try {
		sql::Driver* driver = get_driver_instance();
		std::auto_ptr<sql::Connection> con(driver->connect("localhost", "points", "points"));
		con->setSchema("points");

		std::auto_ptr<sql::ResultSet> res;

		if (m_see_all_students_check.GetCheck() == BST_CHECKED)
			res = Student::get(con.get());
		else
			res = Student::get(con.get(), m_year);

		int i = 0;
		while (res->next()) {
			std::string oen = res->getString("oen");
			m_student_list.InsertItem(i, CString(oen.c_str()));

			std::string first_name = res->getString("first_name");
			m_student_list.SetItemText(i, 1, CString(first_name.c_str()));

			std::string pref_name = res->getString("pref_name");
			m_student_list.SetItemText(i, 2, CString(pref_name.c_str()));

			std::string last_name = res->getString("last_name");
			m_student_list.SetItemText(i, 3, CString(last_name.c_str()));

			std::string start_year = res->getString("start_year");
			m_student_list.SetItemText(i, 4, CString(start_year.c_str()));

			std::string grad_year = res->getString("grad_year");
			m_student_list.SetItemText(i, 5, CString(grad_year.c_str()));

			m_student_list.SetItemData(i, res->getInt("id"));

			++i;
		}
	}
	catch (sql::SQLException& e) {
		// Exception occured
		std::string err = "Something went wrong...\nError: " + (std::string)e.what();
		AfxMessageBox(CString(err.c_str()));
	}
}


// CStudentView message handlers

// Change the year manually
void CStudentView::OnBnClickedStudentYearChange()
{
	CString title = _T("Change year");
	CString caption = _T("Change year");
	CEditIntDlg editIntDlg(title, caption, m_year);
	if (editIntDlg.DoModal() == IDOK) {
		// Check to see if value is invalid (year should be from 1970 to around current year)
		if (editIntDlg.m_value < 1970 || editIntDlg.m_value > CTime::GetCurrentTime().GetYear()+10) {
			AfxMessageBox(_T("Please enter a valid year. The year cannot be too far in the past or future."));
			return;
		}
		// Set year value
		m_year = editIntDlg.m_value;

		// Set year text and reload
		CString yearString;
		yearString.Format(_T("%d"), m_year);
		m_year_static.SetWindowTextW(_T("Year: ") + yearString);
		loadStudentList();
	}
}

// Toggling whether all students from every year should be seen
void CStudentView::OnBnClickedToggleSeeAll()
{
	loadStudentList();
}

// Keep track of current selection for other functions (select row in list control)
// Selection mark stored as a field because the list control doesn't reset the selection mark when user deselects row
void CStudentView::OnLvnItemchangedStudentList(NMHDR* pNMHDR, LRESULT* pResult)
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

// Edit Student attributes (a column header was clicked; edit the corresponding column data)
void CStudentView::OnLvnColumnclickStudentList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	*pResult = 0;

	if (selectionMark == -1)
		return;

	try {
		sql::Driver* driver = get_driver_instance();
		std::auto_ptr<sql::Connection> con(driver->connect("localhost", "points", "points"));
		con->setSchema("points");

		CString title = _T("Edit Student");

		if (pNMLV->iSubItem == 0) {
			CString caption = _T("Edit OEN");
			CString OEN = m_student_list.GetItemText(selectionMark, 0);

			CEditStringDlg editStringDlg(title, caption, OEN);
			if (editStringDlg.DoModal() == IDOK) {
				CString new_OEN = editStringDlg.m_value;

				// Check to see if value is invalid (OEN should be exactly 9 characters)
				if (new_OEN.GetLength() != 9) {
					AfxMessageBox(_T("OEN should be exactly 9 digits long"));
					return;
				}
				
				// Check to see if OEN contains non-digit characters
				for (int i = 0; i < new_OEN.GetLength(); ++i)
					if (!std::isdigit(new_OEN[i])) {
						AfxMessageBox(_T("OEN must contain only digits (0-9)"));
						return;
					}

				// Update database
				Student::edit_OEN(con.get(), m_student_list.GetItemData(selectionMark), new_OEN);
				// Update UI
				m_student_list.SetItemText(selectionMark, 0, new_OEN);
				Invalidate();
			}
		}
		else if (pNMLV->iSubItem == 1) {
			CString caption = _T("Edit First Name");
			CString first_name = m_student_list.GetItemText(selectionMark, 1);

			CEditStringDlg editStringDlg(title, caption, first_name);
			if (editStringDlg.DoModal() == IDOK) {
				CString new_first_name = editStringDlg.m_value;

				// Check to see if name is invalid (should be from 1 to 64 characters)
				if (new_first_name.GetLength() < 1 || new_first_name.GetLength() > 64) {
					AfxMessageBox(_T("Name should be from 1 to 64 characters long"));
					return;
				}

				// Update database
				Student::edit_first_name(con.get(), m_student_list.GetItemData(selectionMark), new_first_name);
				// Update UI
				m_student_list.SetItemText(selectionMark, 1, new_first_name);
				Invalidate();
			}
		}
		else if (pNMLV->iSubItem == 2) {
			CString caption = _T("Edit Preferred Name");
			CString pref_name = m_student_list.GetItemText(selectionMark, 2);

			CEditStringDlg editStringDlg(title, caption, pref_name);
			if (editStringDlg.DoModal() == IDOK) {
				CString new_pref_name = editStringDlg.m_value;

				// Check to see if name is invalid (should be from 1 to 64 characters)
				if (new_pref_name.GetLength() > 64) {
					AfxMessageBox(_T("Name should be no more than 64 characters long"));
					return;
				}

				// Update database
				Student::edit_pref_name(con.get(), m_student_list.GetItemData(selectionMark), new_pref_name);
				// Update UI
				m_student_list.SetItemText(selectionMark, 2, new_pref_name);
				Invalidate();
			}
		}
		else if (pNMLV->iSubItem == 3) {
			CString caption = _T("Edit Last Name");
			CString last_name = m_student_list.GetItemText(selectionMark, 3);

			CEditStringDlg editStringDlg(title, caption, last_name);
			if (editStringDlg.DoModal() == IDOK) {
				CString new_last_name = editStringDlg.m_value;

				// Check to see if name is invalid (should be from 1 to 64 characters)
				if (new_last_name.GetLength() < 1 || new_last_name.GetLength() > 64) {
					AfxMessageBox(_T("Name should be from 1 to 64 characters long"));
					return;
				}

				// Update database
				Student::edit_last_name(con.get(), m_student_list.GetItemData(selectionMark), new_last_name);
				// Update UI
				m_student_list.SetItemText(selectionMark, 3, new_last_name);
				Invalidate();
			}
		}
		else if (pNMLV->iSubItem == 4) {
			CString caption = _T("Edit Start Year");
			int start_year = _ttoi(m_student_list.GetItemText(selectionMark, 4));
			int grad_year = _ttoi(m_student_list.GetItemText(selectionMark, 5));

			CEditIntDlg editIntDlg(title, caption, start_year);
			if (editIntDlg.DoModal() == IDOK) {
				int new_start_year = editIntDlg.m_value;

				// Check to see if year is invalid (should be from 1970 to around present)
				// Ensure can't be 10 years in the future
				if (new_start_year < 1970 || new_start_year > CTime::GetCurrentTime().GetYear()+10) {
					AfxMessageBox(_T("Year cannot be too far in the past or future"));
					return;
				}

				// Check to ensure start year <= grad year
				if (new_start_year > grad_year) {
					AfxMessageBox(_T("Start year cannot be after graduation year"));
					return;
				}

				// Update database
				Student::edit_start_year(con.get(), m_student_list.GetItemData(selectionMark), new_start_year);
				// Update UI
				CString yearString;
				yearString.Format(_T("%d"), new_start_year);
				m_student_list.SetItemText(selectionMark, 4, yearString);
				Invalidate();
			}
		}
		else if (pNMLV->iSubItem == 5) {
			CString caption = _T("Edit Grad Year");
			int start_year = _ttoi(m_student_list.GetItemText(selectionMark, 4));
			int grad_year = _ttoi(m_student_list.GetItemText(selectionMark, 5));

			CEditIntDlg editIntDlg(title, caption, grad_year);
			if (editIntDlg.DoModal() == IDOK) {
				int new_grad_year = editIntDlg.m_value;

				// Check to see if year is invalid (should be from 1970 to around present)
				if (new_grad_year < 1970 || new_grad_year > CTime::GetCurrentTime().GetYear()+10) {
					AfxMessageBox(_T("Year cannot be too far in the past or future"));
					return;
				}

				// Check to ensure grad year >= grad year
				if (new_grad_year < start_year) {
					AfxMessageBox(_T("Grad year cannot be before start year"));
					return;
				}

				// Update database
				Student::edit_grad_year(con.get(), m_student_list.GetItemData(selectionMark), new_grad_year);
				// Update UI
				CString yearString;
				yearString.Format(_T("%d"), new_grad_year);
				m_student_list.SetItemText(selectionMark, 5, yearString);
				Invalidate();
			}
		}
	}
	catch (sql::SQLException& e) {
		// Exception occured
		std::string err;
		err = "Something went wrong...\nError: " + (std::string)e.what();
		AfxMessageBox(CString(err.c_str()));
	}
}


// Create new student interface
void CStudentView::OnBnClickedCreateStudent()
{
	CStudentCreateDlg studentCreateDlg(CTime::GetCurrentTime().GetYear());
	if (studentCreateDlg.DoModal() == IDOK) {
		// Check to see if year is invalid (should be around present)
		if (studentCreateDlg.m_grad_year > CTime::GetCurrentTime().GetYear() + 10) {
			AfxMessageBox(_T("Year cannot be too far in the future"));
			return;
		}

		// Check to ensure start year <= grad year
		if (studentCreateDlg.m_start_year > studentCreateDlg.m_grad_year) {
			AfxMessageBox(_T("Grad year cannot be before start year"));
			return;
		}

		try {
			sql::Driver* driver = get_driver_instance();
			std::auto_ptr<sql::Connection> con(driver->connect("localhost", "points", "points"));
			con->setSchema("points");

			Student::add(
				con.get(),
				studentCreateDlg.m_oen,
				studentCreateDlg.m_last_name,
				studentCreateDlg.m_first_name,
				studentCreateDlg.m_pref_name,
				studentCreateDlg.m_start_year,
				studentCreateDlg.m_grad_year
			);
			loadStudentList();
		}
		catch (sql::SQLException& e) {
			// Exception occured
			std::string err;
			if (e.getErrorCode() == 1062) err = "An action with that OEN already exists!"; // If duplicate key name
			else err = "Something went wrong...\nError: " + (std::string)e.what();
			AfxMessageBox(CString(err.c_str()));
		}
	}
}

// Remove currently selected student
void CStudentView::OnBnClickedRemoveStudent()
{
	if (selectionMark == -1)
		return;

	try {
		sql::Driver* driver = get_driver_instance();
		std::auto_ptr<sql::Connection> con(driver->connect("localhost", "points", "points"));
		con->setSchema("points");

		int res = MessageBox(
			_T(
				"Deleting a student is a permanent action! It is suggested you keep all students as reference for future years.\n\n"
				"Are you sure you want to delete this student?"
			),
			_T("Confirm deletion"),
			MB_OKCANCEL | MB_ICONSTOP | MB_DEFBUTTON2
		);

		if (res == IDOK) {
			Student::remove(con.get(), m_student_list.GetItemData(selectionMark));
			loadStudentList();
			AfxMessageBox(_T("Student deleted."));
			selectionMark = -1;
		}
	}
	catch (sql::SQLException& e) {
		// Exception occured
		std::string err = "Something went wrong...\nError: " + (std::string)e.what();
		AfxMessageBox(CString(err.c_str()));
	}
}

// Assign actions to currently selected student
// TODO: make sure year cannot be less than start year, or greater than grad year
void CStudentView::OnBnClickedAssignActions()
{
	if (selectionMark == -1)
		return;

	CStudentAssignActionDlg studentAssignActionDlg(CTime::GetCurrentTime().GetYear());
	if (studentAssignActionDlg.DoModal() == IDOK) {
		try {
			sql::Driver* driver = get_driver_instance();
			std::auto_ptr<sql::Connection> con(driver->connect("localhost", "points", "points"));
			con->setSchema("points");

			for (int action_id : studentAssignActionDlg.m_action_ids) {
				Student::assign_action(con.get(), m_student_list.GetItemData(selectionMark), action_id, studentAssignActionDlg.m_year);
			}
		}
		catch (sql::SQLException& e) {
			// Exception occured
			std::string err = "Something went wrong...\nError: " + (std::string)e.what();
			AfxMessageBox(CString(err.c_str()));
		}
	}
}

// See actions of currently selected student
void CStudentView::OnBnClickedEditActions()
{
	if (selectionMark == -1)
		return;

	CString student_name = m_student_list.GetItemText(selectionMark, 1) + _T(" ");
	if (m_student_list.GetItemText(selectionMark, 2).GetLength() > 0)
		student_name += _T("(") + m_student_list.GetItemText(selectionMark, 2) + _T(") ");
	student_name += m_student_list.GetItemText(selectionMark, 3);

	CStudentEditActionsDlg studentEditActionsDlg(_T("Manage actions"), _T("Manage actions of ") + student_name, m_student_list.GetItemData(selectionMark));
	studentEditActionsDlg.DoModal();
}

// See and manage all awards given to the currently selected student
void CStudentView::OnBnClickedSeeStudentAwards()
{
	if (selectionMark == -1)
		return;

	CString student_name = m_student_list.GetItemText(selectionMark, 1) + _T(" ");
	if (m_student_list.GetItemText(selectionMark, 2).GetLength() > 0)
		student_name += _T("(") + m_student_list.GetItemText(selectionMark, 2) + _T(") ");
	student_name += m_student_list.GetItemText(selectionMark, 3);

	CStudentEditAwardsDlg studentEditAwardsDlg(_T("See awards"), _T("See awards of ") + student_name, m_student_list.GetItemData(selectionMark));
	studentEditAwardsDlg.DoModal();
}


// Perform calculation of awards for all students who graduate the current year
void CStudentView::OnBnClickedEvaluateAwards()
{
	CString yearString;
	yearString.Format(_T("%d"), m_year);
	int res = MessageBox(
		_T("Please confirm: evaluate eligible awards for all students who graduate in the year ") + yearString,
		_T("Confirm evaluate"),
		MB_OKCANCEL | MB_ICONQUESTION | MB_DEFBUTTON2
	);
	if (res != IDOK)
		return;

	try {
		sql::Driver* driver = get_driver_instance();
		std::auto_ptr<sql::Connection> con(driver->connect("localhost", "points", "points"));
		con->setSchema("points");

		std::auto_ptr<sql::ResultSet> res = Student::get_grad(con.get(), m_year);
		while (res->next()) {
			int student_id = res->getInt("id");
			Constraint::assign_awards(con.get(), student_id);
		}
		AfxMessageBox(_T("All awards assigned out.\nTo confirm these awards (mark as handed out), see \"Hand out awards\"."));
	}
	catch (sql::SQLException& e) {
		// Exception occured
		std::string err = "Something went wrong...\nError: " + (std::string)e.what();
		AfxMessageBox(CString(err.c_str()));
	}
}

// Open up interface to select awards to hand out or directly hand out all awards, and see all awards that haven't been handed out
void CStudentView::OnBnClickedHandOutAwards()
{
	CString yearString;
	yearString.Format(_T("%d"), m_year);

	CHandOutAwardsDlg handOutAwardsDlg(_T("Hand out awards"), _T("Hand out awards for graduating class of ") + yearString, m_year);
	handOutAwardsDlg.DoModal();
}

// Open up interface to see all awards from all years
// Allow for deletion of awards as well
void CStudentView::OnBnClickedSeeAwards()
{
	CManageAllAwardsDlg manageAllAwardsDlg(_T("Manage all awards"), _T("Manage all awards"), m_year);
	manageAllAwardsDlg.DoModal();
}






// CStudentCreateDlg dialog

IMPLEMENT_DYNAMIC(CStudentCreateDlg, CDialogEx)

CStudentCreateDlg::CStudentCreateDlg(int start_year, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_STUDENT_CREATE, pParent)
	, m_oen(_T(""))
	, m_last_name(_T(""))
	, m_first_name(_T(""))
	, m_pref_name(_T(""))
	, m_start_year(start_year)
	, m_grad_year(start_year+4)
{

}

CStudentCreateDlg::~CStudentCreateDlg()
{
}

void CStudentCreateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STUDENT_OEN, m_oen);
	DDV_MaxChars(pDX, m_oen, 9);
	DDV_MinChars(pDX, m_oen, 9);
	DDX_Text(pDX, IDC_STUDENT_LASTNAME, m_last_name);
	DDV_MaxChars(pDX, m_last_name, 64);
	DDV_MinChars(pDX, m_last_name, 1);
	DDX_Text(pDX, IDC_STUDENT_FIRSTNAME, m_first_name);
	DDV_MaxChars(pDX, m_first_name, 64);
	DDV_MinChars(pDX, m_first_name, 1);
	DDX_Text(pDX, IDC_STUDENT_PREFNAME, m_pref_name);
	DDV_MaxChars(pDX, m_pref_name, 64);
	DDX_Text(pDX, IDC_STUDENT_STARTYEAR, m_start_year);
	DDV_MinMaxInt(pDX, m_start_year, 1970, 2999);
	DDX_Text(pDX, IDC_STUDENT_GRADYEAR, m_grad_year);
	DDV_MinMaxInt(pDX, m_grad_year, 1970, 2999);
}


BEGIN_MESSAGE_MAP(CStudentCreateDlg, CDialogEx)
END_MESSAGE_MAP()


// CStudentCreateDlg message handlers






// CStudentAssignActionDlg dialog

IMPLEMENT_DYNAMIC(CStudentAssignActionDlg, CDialogEx)

CStudentAssignActionDlg::CStudentAssignActionDlg(int year, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_STUDENT_ASSIGN_ACTION, pParent)
	, m_action_type(-1)
	, m_year(year)
{

}

CStudentAssignActionDlg::~CStudentAssignActionDlg()
{
}

BOOL CStudentAssignActionDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_action_list.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_action_list.InsertColumn(0, _T("Actions:"), LVCFMT_LEFT, 450, 0);

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

	m_action_type_list.AddString(_T("Athletics"));
	m_action_type_list.AddString(_T("Academics"));
	m_action_type_list.AddString(_T("Activities"));

	loadTypeData();

	return TRUE;
}

void CStudentAssignActionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TITLE, m_title);
	DDX_Control(pDX, IDC_ACTION_LIST, m_action_list);
	DDX_Control(pDX, IDC_ACTION_TYPE_LIST, m_action_type_list);
	DDX_Text(pDX, IDC_STUDENT_STARTYEAR, m_year);
	DDV_MinMaxInt(pDX, m_year, 1970, 2999);
}

void CStudentAssignActionDlg::OnOK()
{

	POSITION pos = m_action_list.GetFirstSelectedItemPosition();
	if (pos == NULL) {
		AfxMessageBox(_T("Select at least one action!"));
		return;
	}

	m_action_ids.clear();
	while (pos) {
		int nItem = m_action_list.GetNextSelectedItem(pos);
		m_action_ids.push_back(m_action_list.GetItemData(nItem));
	}

	CDialog::OnOK();
}

// Load data for type of action
void CStudentAssignActionDlg::loadTypeData() {

	// Reset items
	m_action_list.DeleteAllItems();

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



BEGIN_MESSAGE_MAP(CStudentAssignActionDlg, CDialogEx)
	ON_LBN_SELCHANGE(IDC_ACTION_TYPE_LIST, &CStudentAssignActionDlg::OnLbnSelchangeActionTypeList)
END_MESSAGE_MAP()


// CStudentAssignActionDlg message handlers


void CStudentAssignActionDlg::OnLbnSelchangeActionTypeList()
{
	if (m_action_type_list.GetCurSel() == LB_ERR)
		m_action_type = -1;
	else
		m_action_type = m_action_type_list.GetCurSel();

	loadTypeData();
}






// CStudentEditActionsDlg dialog

IMPLEMENT_DYNAMIC(CStudentEditActionsDlg, CDialogEx)

CStudentEditActionsDlg::CStudentEditActionsDlg(CString header, CString title, int student_id, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_STUDENT_MANAGE_LIST, pParent)
	, m_header(header)
	, m_title_text(title)
	, m_student_id(student_id)
	, selectionMark(-1)
{

}

CStudentEditActionsDlg::~CStudentEditActionsDlg()
{
}

void CStudentEditActionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STUDENT_LIST, m_action_list);
	DDX_Control(pDX, IDC_TITLE, m_title);
	DDX_Control(pDX, IDC_STUDENT_MANAGE, m_remove_action);
}

BOOL CStudentEditActionsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Initialize action list
	m_action_list.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	// Insert dummy column so LVCFMT_FIXED_WIDTH works properly (see Remarks of LVCOLUMN struct)
	m_action_list.InsertColumn(0, &LVCOLUMN());

	m_col_name.mask = m_col_type.mask = m_col_points.mask = m_col_year.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;

	m_col_name.fmt = m_col_type.fmt = m_col_points.fmt = m_col_year.fmt = LVCFMT_LEFT | LVCFMT_FIXED_WIDTH;

	m_col_name.cx = 145;
	m_col_type.cx = 125;
	m_col_points.cx = 55;
	m_col_year.cx = 75;

	m_col_name.pszText = _T("Name");
	m_col_type.pszText = _T("Type");
	m_col_points.pszText = _T("Points");
	m_col_year.pszText = _T("Year");

	m_action_list.InsertColumn(1, &m_col_name);
	m_action_list.InsertColumn(2, &m_col_type);
	m_action_list.InsertColumn(3, &m_col_points);
	m_action_list.InsertColumn(4, &m_col_year);

	m_action_list.DeleteColumn(0);

	// Initialize dialog text
	SetWindowText(m_header);
	m_title.SetWindowTextW(m_title_text);

	m_titleFont.CreateFontW(
		16,
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

	m_remove_action.SetWindowTextW(_T("Delete"));

	loadTypeData();

	return TRUE;
}

// Load data for type of action
void CStudentEditActionsDlg::loadTypeData() {

	m_action_list.DeleteAllItems();

	// Fetch data from MySQL
	try {
		sql::Driver* driver = get_driver_instance();
		std::auto_ptr<sql::Connection> con(driver->connect("localhost", "points", "points"));
		con->setSchema("points");

		std::auto_ptr<sql::ResultSet> res = Student::get_actions(con.get(), m_student_id);
		int i = 0;
		while (res->next()) {
			std::string type;
			if (res->getInt("type") == 0)
				type = "Athletics";
			else if (res->getInt("type") == 1)
				type = "Academics";
			else if (res->getInt("type") == 2)
				type = "Activities";

			std::string name = (std::string)res->getString("name");
			m_action_list.InsertItem(i, CString(name.c_str()));

			m_action_list.SetItemText(i, 1, CString(type.c_str()));

			std::string points = (std::string)res->getString("points");
			m_action_list.SetItemText(i, 2, CString(points.c_str()));

			std::string year = (std::string)res->getString("year");
			m_action_list.SetItemText(i, 3, CString(year.c_str()));

			m_action_list.SetItemData(i, res->getInt("actn_id"));

			++i;
		}
	}
	catch (sql::SQLException& e) {
		// Exception occured
		std::string err = "Something went wrong...\nError: " + (std::string)e.what();
		AfxMessageBox(CString(err.c_str()));
	}
}


BEGIN_MESSAGE_MAP(CStudentEditActionsDlg, CDialogEx)
	ON_BN_CLICKED(IDC_STUDENT_MANAGE, &CStudentEditActionsDlg::OnBnClickedStudentDeleteAction)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_STUDENT_LIST, &CStudentEditActionsDlg::OnLvnItemchangedStudentList)
END_MESSAGE_MAP()


// CStudentEditActionsDlg message handlers

// Keep track of current selection for other functions (select row in list control)
// Selection mark stored as a field because the list control doesn't reset the selection mark when user deselects row
void CStudentEditActionsDlg::OnLvnItemchangedStudentList(NMHDR* pNMHDR, LRESULT* pResult)
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

// Delete selected student action
void CStudentEditActionsDlg::OnBnClickedStudentDeleteAction()
{
	if (selectionMark == -1)
		return;

	try {
		sql::Driver* driver = get_driver_instance();
		std::auto_ptr<sql::Connection> con(driver->connect("localhost", "points", "points"));
		con->setSchema("points");

		int res = MessageBox(
			_T("Are you sure you want to delete this action?"),
			_T("Confirm deletion"),
			MB_OKCANCEL | MB_ICONEXCLAMATION | MB_DEFBUTTON2
		);

		if (res == IDOK) {
			// args in order: student id, action id, year
			Student::remove_action(con.get(), m_student_id, m_action_list.GetItemData(selectionMark), _ttoi(m_action_list.GetItemText(selectionMark, 3)));
			loadTypeData();
			AfxMessageBox(_T("Action deleted."));
			selectionMark = -1;
		}
	}
	catch (sql::SQLException& e) {
		// Exception occured
		std::string err = "Something went wrong...\nError: " + (std::string)e.what();
		AfxMessageBox(CString(err.c_str()));
	}
}






// CStudentEditAwardsDlg dialog

IMPLEMENT_DYNAMIC(CStudentEditAwardsDlg, CDialogEx)

CStudentEditAwardsDlg::CStudentEditAwardsDlg(CString header, CString title, int student_id, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_STUDENT_MANAGE_LIST, pParent)
	, m_header(header)
	, m_title_text(title)
	, m_student_id(student_id)
	, selectionMark(-1)
{

}

CStudentEditAwardsDlg::~CStudentEditAwardsDlg()
{
}

void CStudentEditAwardsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STUDENT_LIST, m_award_list);
	DDX_Control(pDX, IDC_TITLE, m_title);
	DDX_Control(pDX, IDC_STUDENT_MANAGE, m_toggle_handout_award);
}

BOOL CStudentEditAwardsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Initialize award list
	m_award_list.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	// Insert dummy column so LVCFMT_FIXED_WIDTH works properly (see Remarks of LVCOLUMN struct)
	m_award_list.InsertColumn(0, &LVCOLUMN());

	m_col_name.mask = m_col_desc.mask = m_col_confirmed.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;

	m_col_name.fmt = m_col_desc.fmt = m_col_confirmed.fmt = LVCFMT_LEFT | LVCFMT_FIXED_WIDTH;

	m_col_name.cx = 150;
	m_col_desc.cx = 210;
	m_col_confirmed.cx = 75;

	m_col_name.pszText = _T("Name");
	m_col_desc.pszText = _T("Description");
	m_col_confirmed.pszText = _T("Confirmed");

	m_award_list.InsertColumn(1, &m_col_name);
	m_award_list.InsertColumn(2, &m_col_desc);
	m_award_list.InsertColumn(3, &m_col_confirmed);

	m_award_list.DeleteColumn(0);

	// Initialize dialog text
	SetWindowText(m_header);
	m_title.SetWindowTextW(m_title_text);

	m_titleFont.CreateFontW(
		16,
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

	m_toggle_handout_award.SetWindowTextW(_T("Toggle Confirm"));

	loadConstraintList();

	return TRUE;
}

// Load awards for student
void CStudentEditAwardsDlg::loadConstraintList() {

	m_award_list.DeleteAllItems();

	// Fetch data from MySQL
	try {
		sql::Driver* driver = get_driver_instance();
		std::auto_ptr<sql::Connection> con(driver->connect("localhost", "points", "points"));
		con->setSchema("points");

		std::auto_ptr<sql::ResultSet> res = Student::get_awards(con.get(), m_student_id);
		int i = 0;
		while (res->next()) {
			std::string name = (std::string)res->getString("name");
			m_award_list.InsertItem(i, CString(name.c_str()));

			std::string desc = (std::string)res->getString("description");
			m_award_list.SetItemText(i, 1, CString(desc.c_str()));

			std::string confirmed = res->getBoolean("confirmed") ? "yes" : "no";
			m_award_list.SetItemText(i, 2, CString(confirmed.c_str()));

			m_award_list.SetItemData(i, res->getInt("cnst_id"));

			++i;
		}
	}
	catch (sql::SQLException& e) {
		// Exception occured
		std::string err = "Something went wrong...\nError: " + (std::string)e.what();
		AfxMessageBox(CString(err.c_str()));
	}
}


BEGIN_MESSAGE_MAP(CStudentEditAwardsDlg, CDialogEx)
	ON_BN_CLICKED(IDC_STUDENT_MANAGE, &CStudentEditAwardsDlg::OnBnClickedStudentToggleAward)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_STUDENT_LIST, &CStudentEditAwardsDlg::OnLvnItemchangedAwardList)
END_MESSAGE_MAP()


// CStudentEditAwardsDlg message handlers

// Keep track of current selection for other functions (select row in list control)
// Selection mark stored as a field because the list control doesn't reset the selection mark when user deselects row
void CStudentEditAwardsDlg::OnLvnItemchangedAwardList(NMHDR* pNMHDR, LRESULT* pResult)
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

// Toggle whether the student's award has been confirmed
void CStudentEditAwardsDlg::OnBnClickedStudentToggleAward()
{
	if (selectionMark == -1)
		return;

	try {
		sql::Driver* driver = get_driver_instance();
		std::auto_ptr<sql::Connection> con(driver->connect("localhost", "points", "points"));
		con->setSchema("points");

		// args in order: student_id, cnst_id
		if (m_award_list.GetItemText(selectionMark, 2) == _T("no")) {
			Student::confirm_student_award(con.get(), m_student_id, m_award_list.GetItemData(selectionMark));
			loadConstraintList();
			AfxMessageBox(_T("Award handed out."));
		}
		else if (m_award_list.GetItemText(selectionMark, 2) == _T("yes")) {
			Student::unconfirm_student_award(con.get(), m_student_id, m_award_list.GetItemData(selectionMark));
			loadConstraintList();
			AfxMessageBox(_T("Award unhanded out."));
		}
		selectionMark = -1;
	}
	catch (sql::SQLException& e) {
		// Exception occured
		std::string err = "Something went wrong...\nError: " + (std::string)e.what();
		AfxMessageBox(CString(err.c_str()));
	}
}






// CHandOutAwardsDlg dialog

IMPLEMENT_DYNAMIC(CHandOutAwardsDlg, CDialogEx)

CHandOutAwardsDlg::CHandOutAwardsDlg(CString header, CString title, int year, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_STUDENT_AWARD_LIST, pParent)
	, m_header(header)
	, m_title_text(title)
	, m_year(year)
	, selectionMark(-1)
{

}

CHandOutAwardsDlg::~CHandOutAwardsDlg()
{
}

void CHandOutAwardsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STUDENT_AWARD_LIST, m_award_list);
	DDX_Control(pDX, IDC_TITLE, m_title);
	DDX_Control(pDX, IDC_STUDENT_AWARD_CHECK, m_see_confirmed);
	DDX_Control(pDX, IDC_STUDENT_MANAGE, m_toggle_handout_award);
}

BOOL CHandOutAwardsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Initialize action list
	m_award_list.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	// Insert dummy column so LVCFMT_FIXED_WIDTH works properly (see Remarks of LVCOLUMN struct)
	m_award_list.InsertColumn(0, &LVCOLUMN());

	m_col_stdt_first_name.mask = m_col_stdt_last_name.mask =
		m_col_award_name.mask = m_col_award_desc.mask = m_col_award_confirmed.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;

	m_col_stdt_first_name.fmt = m_col_stdt_last_name.fmt =
		m_col_award_name.fmt = m_col_award_desc.fmt = m_col_award_confirmed.fmt = LVCFMT_LEFT | LVCFMT_FIXED_WIDTH;

	m_col_stdt_first_name.cx = 130;
	m_col_stdt_last_name.cx = 100;
	m_col_award_name.cx = 150;
	m_col_award_desc.cx = 210;
	m_col_award_confirmed.cx = 75;

	m_col_stdt_first_name.pszText = _T("First Name");
	m_col_stdt_last_name.pszText = _T("Last Name");
	m_col_award_name.pszText = _T("Award Name");
	m_col_award_desc.pszText = _T("Award Description");
	m_col_award_confirmed.pszText = _T("Confirmed");

	m_award_list.InsertColumn(1, &m_col_stdt_first_name);
	m_award_list.InsertColumn(2, &m_col_stdt_last_name);
	m_award_list.InsertColumn(3, &m_col_award_name);
	m_award_list.InsertColumn(4, &m_col_award_desc);
	m_award_list.InsertColumn(5, &m_col_award_confirmed);

	m_award_list.DeleteColumn(0);

	// Initialize dialog text
	SetWindowText(m_header);
	m_title.SetWindowTextW(m_title_text);

	m_titleFont.CreateFontW(
		20,
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

	m_see_confirmed.SetWindowTextW(_T("See already confirmed awards"));
	m_toggle_handout_award.SetWindowTextW(_T("Toggle Confirm"));

	loadConstraintList();

	return TRUE;
}

// Load awards for this year
void CHandOutAwardsDlg::loadConstraintList() {

	m_award_list.DeleteAllItems();
	m_ids.clear();

	// Fetch data from MySQL
	try {
		sql::Driver* driver = get_driver_instance();
		std::auto_ptr<sql::Connection> con(driver->connect("localhost", "points", "points"));
		con->setSchema("points");

		std::auto_ptr<sql::ResultSet> res = Student::get_awards_grad(con.get(), m_year, m_see_confirmed.GetCheck() == BST_CHECKED);
		int i = 0;
		while (res->next()) {
			int award_id = res->getInt("cnst_id"), student_id = res->getInt("stdt_id");

			std::string first_name = (std::string)res->getString("first_name");
			std::string pref_name = (std::string)res->getString("pref_name");
			if (pref_name.size() > 0)
				first_name += " (" + pref_name + ")";
			m_award_list.InsertItem(i, CString(first_name.c_str()));

			std::string last_name = (std::string)res->getString("last_name");
			m_award_list.SetItemText(i, 1, CString(last_name.c_str()));

			std::string name = (std::string)res->getString("name");
			m_award_list.SetItemText(i, 2, CString(name.c_str()));

			std::string desc = (std::string)res->getString("description");
			m_award_list.SetItemText(i, 3, CString(desc.c_str()));

			std::string confirmed = res->getBoolean("confirmed") ? "yes" : "no";
			m_award_list.SetItemText(i, 4, CString(confirmed.c_str()));

			// Set item data to point to index of id vector, if the order of m_award_list changes (unsure if this is possible, but setting just to be safe)
			m_award_list.SetItemData(i, i);

			m_ids.push_back({ student_id, award_id });

			++i;
		}
	}
	catch (sql::SQLException& e) {
		// Exception occured
		std::string err = "Something went wrong...\nError: " + (std::string)e.what();
		AfxMessageBox(CString(err.c_str()));
	}
}

BEGIN_MESSAGE_MAP(CHandOutAwardsDlg, CDialogEx)
	ON_BN_CLICKED(IDC_STUDENT_AWARD_CHECK, &CHandOutAwardsDlg::OnBnClickedSeeUnconfirmed)
	ON_BN_CLICKED(IDC_STUDENT_MANAGE, &CHandOutAwardsDlg::OnBnClickedStudentToggleAward)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_STUDENT_AWARD_LIST, &CHandOutAwardsDlg::OnLvnItemchangedAwardList)
END_MESSAGE_MAP()


// CStudentHandOutAwardsDlg message handlers

// Keep track of current selection for other functions (select row in list control)
// Selection mark stored as a field because the list control doesn't reset the selection mark when user deselects row
void CHandOutAwardsDlg::OnLvnItemchangedAwardList(NMHDR* pNMHDR, LRESULT* pResult)
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

// Changed whether confirmed awards are seen
void CHandOutAwardsDlg::OnBnClickedSeeUnconfirmed()
{
	loadConstraintList();
}

// Toggle whether the student's award has been confirmed
void CHandOutAwardsDlg::OnBnClickedStudentToggleAward()
{
	if (selectionMark == -1)
		return;

	try {
		sql::Driver* driver = get_driver_instance();
		std::auto_ptr<sql::Connection> con(driver->connect("localhost", "points", "points"));
		con->setSchema("points");

		std::pair<int, int> id_pair = m_ids[m_award_list.GetItemData(selectionMark)];
		int student_id = id_pair.first, award_id = id_pair.second;

		// args in order: student_id, cnst_id
		if (m_award_list.GetItemText(selectionMark, 5) == _T("no")) {
			Student::confirm_student_award(con.get(), student_id, award_id);
			loadConstraintList();
			AfxMessageBox(_T("Award handed out."));
		}
		else if (m_award_list.GetItemText(selectionMark, 5) == _T("yes")) {
			Student::unconfirm_student_award(con.get(), student_id, award_id);
			loadConstraintList();
			AfxMessageBox(_T("Award unhanded out."));
		}
		selectionMark = -1;
	}
	catch (sql::SQLException& e) {
		// Exception occured
		std::string err = "Something went wrong...\nError: " + (std::string)e.what();
		AfxMessageBox(CString(err.c_str()));
	}
}






// CManageAllAwardsDlg dialog

IMPLEMENT_DYNAMIC(CManageAllAwardsDlg, CDialogEx)

CManageAllAwardsDlg::CManageAllAwardsDlg(CString header, CString title, int year, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_STUDENT_AWARD_LIST, pParent)
	, m_header(header)
	, m_title_text(title)
	, m_year(year)
	, selectionMark(-1)
{

}

CManageAllAwardsDlg::~CManageAllAwardsDlg()
{
}

void CManageAllAwardsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STUDENT_AWARD_LIST, m_award_list);
	DDX_Control(pDX, IDC_TITLE, m_title);
	DDX_Control(pDX, IDC_STUDENT_AWARD_CHECK, m_see_all_years);
	DDX_Control(pDX, IDC_STUDENT_MANAGE, m_remove_award);
}

BOOL CManageAllAwardsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Initialize award list
	m_award_list.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	// Insert dummy column so LVCFMT_FIXED_WIDTH works properly (see Remarks of LVCOLUMN struct)
	m_award_list.InsertColumn(0, &LVCOLUMN());

	m_col_stdt_first_name.mask = m_col_stdt_last_name.mask =
		m_col_award_name.mask = m_col_award_desc.mask = m_col_grad_year.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;

	m_col_stdt_first_name.fmt = m_col_stdt_last_name.fmt =
		m_col_award_name.fmt = m_col_award_desc.fmt = m_col_grad_year.fmt = LVCFMT_LEFT | LVCFMT_FIXED_WIDTH;

	m_col_stdt_first_name.cx = 130;
	m_col_stdt_last_name.cx = 100;
	m_col_award_name.cx = 150;
	m_col_award_desc.cx = 210;
	m_col_grad_year.cx = 75;

	m_col_stdt_first_name.pszText = _T("First Name");
	m_col_stdt_last_name.pszText = _T("Last Name");
	m_col_award_name.pszText = _T("Award Name");
	m_col_award_desc.pszText = _T("Award Description");
	m_col_grad_year.pszText = _T("Grad Year");

	m_award_list.InsertColumn(1, &m_col_stdt_first_name);
	m_award_list.InsertColumn(2, &m_col_stdt_last_name);
	m_award_list.InsertColumn(3, &m_col_award_name);
	m_award_list.InsertColumn(4, &m_col_award_desc);
	m_award_list.InsertColumn(5, &m_col_grad_year);

	m_award_list.DeleteColumn(0);

	// Initialize dialog text
	SetWindowText(m_header);
	m_title.SetWindowTextW(m_title_text);

	m_titleFont.CreateFontW(
		20,
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

	m_see_all_years.SetWindowTextW(_T("See awards from all years"));
	m_remove_award.SetWindowTextW(_T("Delete"));

	loadConstraintList();

	return TRUE;
}

// Load awards for this year
void CManageAllAwardsDlg::loadConstraintList() {

	m_award_list.DeleteAllItems();
	m_ids.clear();

	// Fetch data from MySQL
	try {
		sql::Driver* driver = get_driver_instance();
		std::auto_ptr<sql::Connection> con(driver->connect("localhost", "points", "points"));
		con->setSchema("points");

		std::auto_ptr<sql::ResultSet> res;

		if (m_see_all_years.GetCheck() == BST_CHECKED) {
			res = Student::get_awards_all(con.get());
		}
		else {
			res = Student::get_awards_grad(con.get(), m_year, true);
		}

		int i = 0;
		while (res->next()) {
			int award_id = res->getInt("cnst_id"), student_id = res->getInt("stdt_id");

			std::string first_name = (std::string)res->getString("first_name");
			std::string pref_name = (std::string)res->getString("pref_name");
			if (pref_name.size() > 0)
				first_name += " (" + pref_name + ")";
			m_award_list.InsertItem(i, CString(first_name.c_str()));

			std::string last_name = (std::string)res->getString("last_name");
			m_award_list.SetItemText(i, 1, CString(last_name.c_str()));

			std::string name = (std::string)res->getString("name");
			m_award_list.SetItemText(i, 2, CString(name.c_str()));

			std::string desc = (std::string)res->getString("description");
			m_award_list.SetItemText(i, 3, CString(desc.c_str()));

			std::string year = res->getString("grad_year");
			m_award_list.SetItemText(i, 4, CString(year.c_str()));

			// Set item data to point to index of id vector, if the order of m_award_list changes (unsure if this is possible, but setting just to be safe)
			m_award_list.SetItemData(i, i);

			m_ids.push_back({ student_id, award_id });

			++i;
		}
	}
	catch (sql::SQLException& e) {
		// Exception occured
		std::string err = "Something went wrong...\nError: " + (std::string)e.what();
		AfxMessageBox(CString(err.c_str()));
	}
}

BEGIN_MESSAGE_MAP(CManageAllAwardsDlg, CDialogEx)
	ON_BN_CLICKED(IDC_STUDENT_AWARD_CHECK, &CManageAllAwardsDlg::OnBnClickedSeeAllYears)
	ON_BN_CLICKED(IDC_STUDENT_MANAGE, &CManageAllAwardsDlg::OnBnClickedStudentRemoveAward)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_STUDENT_AWARD_LIST, &CManageAllAwardsDlg::OnLvnItemchangedAwardList)
END_MESSAGE_MAP()


// CManageAllAwardsDlg message handlers

// Keep track of current selection for other functions (select row in list control)
// Selection mark stored as a field because the list control doesn't reset the selection mark when user deselects row
void CManageAllAwardsDlg::OnLvnItemchangedAwardList(NMHDR* pNMHDR, LRESULT* pResult)
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

// Changed whether confirmed awards are seen
void CManageAllAwardsDlg::OnBnClickedSeeAllYears()
{
	loadConstraintList();
}

// Toggle whether the student's award has been confirmed
void CManageAllAwardsDlg::OnBnClickedStudentRemoveAward()
{
	if (selectionMark == -1)
		return;

	try {
		sql::Driver* driver = get_driver_instance();
		std::auto_ptr<sql::Connection> con(driver->connect("localhost", "points", "points"));
		con->setSchema("points");

		int res = MessageBox(
			_T(
				"Removing a student's award may be permanent! It is suggested you keep all awards as reference for future years.\n\n"
				"Are you sure you want to delete this student's award?"
			),
			_T("Confirm deletion"),
			MB_OKCANCEL | MB_ICONSTOP | MB_DEFBUTTON2
		);

		if (res == IDOK) {
			std::pair<int, int> id_pair = m_ids[m_award_list.GetItemData(selectionMark)];
			int student_id = id_pair.first, award_id = id_pair.second;

			// args in order: cnst_id, stdt_id
			Constraint::remove_student_award(con.get(), award_id, student_id);
			loadConstraintList();
			AfxMessageBox(_T("Student award deleted."));
			selectionMark = -1;
		}
	}
	catch (sql::SQLException& e) {
		// Exception occured
		std::string err = "Something went wrong...\nError: " + (std::string)e.what();
		AfxMessageBox(CString(err.c_str()));
	}
}