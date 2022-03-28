
// WCIPointsDoc.cpp : implementation of the CWCIPointsDoc class
//

#include "pch.h"
#include "framework.h"

#include "WCIPoints.h"

#include "WCIPointsDoc.h"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CWCIPointsDoc

IMPLEMENT_DYNCREATE(CWCIPointsDoc, CDocument)

BEGIN_MESSAGE_MAP(CWCIPointsDoc, CDocument)
	ON_COMMAND(ID_FILE_SAVE, &CWCIPointsDoc::OnFileSave)
END_MESSAGE_MAP()


// CWCIPointsDoc construction/destruction

CWCIPointsDoc::CWCIPointsDoc() noexcept
{
	// TODO: add one-time construction code here

}

CWCIPointsDoc::~CWCIPointsDoc()
{
}

BOOL CWCIPointsDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CWCIPointsDoc serialization

void CWCIPointsDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}


// CWCIPointsDoc diagnostics

#ifdef _DEBUG
void CWCIPointsDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CWCIPointsDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CWCIPointsDoc commands

void CWCIPointsDoc::OnFileSave() {
	AfxMessageBox(_T("This feature has not been added yet"));
}