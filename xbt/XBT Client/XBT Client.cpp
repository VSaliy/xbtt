// XBT Client.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "XBT Client.h"
#include "XBT ClientDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CXBTClientApp

BEGIN_MESSAGE_MAP(CXBTClientApp, CWinApp)
	//{{AFX_MSG_MAP(CXBTClientApp)
	//}}AFX_MSG
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// The one and only CXBTClientApp object

CXBTClientApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CXBTClientApp initialization

BOOL CXBTClientApp::InitInstance()
{
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	CreateMutex(NULL, true, "9a6bfda6-7733-4b7d-92b0-3046c9191830");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		HWND hWnd = FindWindow(NULL, "XBT Client");
		ShowWindow(hWnd, SW_SHOWMAXIMIZED);
		SetForegroundWindow(hWnd);
		return false;
	}
	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	// Standard initialization

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif
	_setmaxstdio(2 << 10);
	SetRegistryKey("XBT");

	CXBTClientDlg dlg;
	m_pMainWnd = &dlg;
	if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileOpen && !cmdInfo.m_strFileName.IsEmpty())
		dlg.open(static_cast<string>(cmdInfo.m_strFileName));
	dlg.DoModal();

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

