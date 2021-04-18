
// WCIPoints.h : main header file for the WCIPoints application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CWCIPointsApp:
// See WCIPoints.cpp for the implementation of this class
//

class CWCIPointsApp : public CWinAppEx
{
public:
	CWCIPointsApp() noexcept;


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CWCIPointsApp theApp;
