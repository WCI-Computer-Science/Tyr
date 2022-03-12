
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
void CAwardView::loadConstraintList(bool only_awards) {

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
			m_constraint_vector.push_back({ id, type, res->getBoolean("award") });

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

	int root_constraint_id = m_constraint_vector[m_constraint_list.GetItemData(selectionMark)].id;

	// Add sub-constraints to constraint tree
	try {
		sql::Driver* driver = get_driver_instance();
		std::auto_ptr<sql::Connection> con(driver->connect("localhost", "points", "points"));
		con->setSchema("points");

		std::auto_ptr<sql::ResultSet> res = Constraint::get_compound(con.get(), root_constraint_id);
		while (res->next()) {
			std::string name = res->getString("name");
			std::string desc = res->getString("description");
			CString child_name = CString(name.c_str()) + _T(": ") + CString(desc.c_str());

			m_constraint_tree.InsertItem(child_name, root_handle);
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
	// TODO: Add your control notification handler code here
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
		else
			selectionMark = -1;
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
	// TODO: Add your control notification handler code here
}

// Access the archive menu
void CAwardView::OnBnClickedAccessArchive()
{
	// TODO: Add your control notification handler code here
}
