
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
