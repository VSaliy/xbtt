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
	srand(time(NULL));
	CreateMutex(NULL, true, "9a6bfda6-7733-4b7d-92b0-3046c9191830");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		HWND hWnd = FindWindow(NULL, "XBT Client");
		CCommandLineInfo cmdInfo;
		ParseCommandLine(cmdInfo);
		if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileOpen)
		{
			COPYDATASTRUCT cds;
			cds.dwData = 0;
			cds.lpData = const_cast<char*>(static_cast<const char*>(cmdInfo.m_strFileName));
			cds.cbData = cmdInfo.m_strFileName.GetLength();
			SendMessage(hWnd, WM_COPYDATA, NULL, reinterpret_cast<LPARAM>(&cds));
		}
		else
		{
			ShowWindow(hWnd, SW_SHOWMAXIMIZED);
			SetForegroundWindow(hWnd);
		}
		return false;
	}
	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return false;
	}

#ifdef _AFXDLL
	Enable3dControls();
#else
	Enable3dControlsStatic();
#endif
	SetRegistryKey("XBT");

	CXBTClientDlg dlg;
	m_pMainWnd = &dlg;
	dlg.DoModal();

	return false;
}

