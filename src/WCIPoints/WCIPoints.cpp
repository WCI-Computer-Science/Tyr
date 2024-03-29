
// WCIPoints.cpp : Defines the class behaviors for the application.
//

#include "pch.h"
#include "framework.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "WCIPoints.h"
#include "MainFrm.h"

#include "WCIPointsDoc.h"
#include "StudentView.h"
#include "ActionView.h"
#include "AwardView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CWCIPointsApp

BEGIN_MESSAGE_MAP(CWCIPointsApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CWCIPointsApp::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_OPEN, &CWCIPointsApp::OnFileOpen)
	// Change view
	ON_COMMAND(ID_STUDENTS_MANAGE, &CWCIPointsApp::OnStudentsManage)
	ON_COMMAND(ID_ACTIONS_MANAGE, &CWCIPointsApp::OnActionsManage)
	ON_COMMAND(ID_AWARDS_MANAGE, &CWCIPointsApp::OnAwardsManage)
END_MESSAGE_MAP()


// CWCIPointsApp construction

CWCIPointsApp::CWCIPointsApp() noexcept
{
	m_bHiColorIcons = TRUE;


	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;
#ifdef _MANAGED
	// If the application is built using Common Language Runtime support (/clr):
	//     1) This additional setting is needed for Restart Manager support to work properly.
	//     2) In your project, you must add a reference to System.Windows.Forms in order to build.
	System::Windows::Forms::Application::SetUnhandledExceptionMode(System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif

	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("WCIPoints.AppID.NoVersion"));

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

// The one and only CWCIPointsApp object

CWCIPointsApp theApp;


// CWCIPointsApp initialization

BOOL CWCIPointsApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();


	EnableTaskbarInteraction(FALSE);

	// AfxInitRichEdit2() is required to use RichEdit control
	// AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("WCI Computer Science"));
	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)


	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CWCIPointsDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CStudentView));    // StudentView is default view
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);


	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);



	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	m_pViews[0] = ((CFrameWnd*)m_pMainWnd)->GetActiveView();

	CDocument* pCurrentDoc = ((CFrameWnd*)m_pMainWnd)->GetActiveDocument();

	CCreateContext newContext;
	newContext.m_pNewViewClass = NULL;
	newContext.m_pNewDocTemplate = NULL;
	newContext.m_pLastView = NULL;
	newContext.m_pCurrentFrame = NULL;
	newContext.m_pCurrentDoc = pCurrentDoc;


	m_pViews[1] = (CView*) new CActionView;
	UINT viewID = AFX_IDW_PANE_FIRST + 1;
	CRect actionRect(0, 0, 0, 0);
	m_pViews[1]->Create(NULL, _T("AnyWindowName"), WS_CHILD, actionRect, m_pMainWnd, viewID, &newContext);
	m_pViews[1]->SendMessage(WM_INITIALUPDATE, 0, 0);

	m_pViews[2] = (CView*) new CAwardView;
	viewID += 1;
	CRect awardRect(0, 0, 0, 0);
	m_pViews[2]->Create(NULL, _T("AnyWindowName"), WS_CHILD, awardRect, m_pMainWnd, viewID, &newContext);
	m_pViews[2]->SendMessage(WM_INITIALUPDATE, 0, 0);


	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	return TRUE;
}

CView* CWCIPointsApp::SwitchView(int i) {
	CView* pActiveView = ((CFrameWnd*)m_pMainWnd)->GetActiveView();
	CView* pNewView = m_pViews[i];

	if (pActiveView == pNewView)
		return pActiveView;

	UINT temp = ::GetWindowLong(pActiveView->m_hWnd, GWL_ID);
	::SetWindowLong(pActiveView->m_hWnd, GWL_ID, ::GetWindowLong(pNewView->m_hWnd, GWL_ID));
	::SetWindowLong(pNewView->m_hWnd, GWL_ID, temp);

	pActiveView->ShowWindow(SW_HIDE);
	pNewView->ShowWindow(SW_SHOW);
	((CFrameWnd*)m_pMainWnd)->SetActiveView(pNewView);
	((CFrameWnd*)m_pMainWnd)->RecalcLayout();
	pNewView->Invalidate();
	return pActiveView;
}

// CWCIPointsApp message handlers

// CWCIPointsApp customization load/save methods

void CWCIPointsApp::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	// No context menu needed
	//GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
}

void CWCIPointsApp::LoadCustomState()
{
}

void CWCIPointsApp::SaveCustomState()
{
}


// CWCIPointsApp message handlers

void CWCIPointsApp::OnFileOpen()
{
	AfxMessageBox(_T("This feature has not been added yet"));
}


void CWCIPointsApp::OnStudentsManage()
{
	SwitchView(0);
}


void CWCIPointsApp::OnActionsManage()
{
	SwitchView(1);
}


void CWCIPointsApp::OnAwardsManage()
{
	SwitchView(2);
}






// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg() noexcept;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() noexcept : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()




// App command to run the dialog
// Placed after CAboutDlg declaration so that it is declared
void CWCIPointsApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}