// Custom MFC stuff

#include "pch.h"
#include "framework.h"
#include "afxdialogex.h"

#include "custommfc.h"

void AFXAPI DDV_MinChars(CDataExchange* pDX, CString const& value, int nChars) {
	if (pDX->m_bSaveAndValidate && value.GetLength() < nChars) {
		CString msg;
		msg.Format(_T("Enter at least %d character(s)."), nChars);
		AfxMessageBox(msg);
		pDX->Fail();
	}
}