// dlg_options.cpp : implementation file
//

#include "stdafx.h"
#include "XBT Client.h"
#include "dlg_options.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Cdlg_options dialog


Cdlg_options::Cdlg_options(CWnd* pParent /*=NULL*/)
	: CDialog(Cdlg_options::IDD, pParent)
{
	//{{AFX_DATA_INIT(Cdlg_options)
	m_peer_port = 0;
	m_admin_port = 0;
	//}}AFX_DATA_INIT
}


void Cdlg_options::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(Cdlg_options)
	DDX_Text(pDX, IDC_PEER_PORT, m_peer_port);
	DDV_MinMaxInt(pDX, m_peer_port, 0, 65535);
	DDX_Text(pDX, IDC_ADMIN_PORT, m_admin_port);
	DDV_MinMaxInt(pDX, m_admin_port, 0, 65535);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(Cdlg_options, CDialog)
	//{{AFX_MSG_MAP(Cdlg_options)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Cdlg_options message handlers

Cdlg_options::t_data Cdlg_options::get() const
{
	t_data v;
	v.admin_port = m_admin_port;
	v.peer_port = m_peer_port;
	return v;
}

void Cdlg_options::set(const t_data& v)
{
	m_admin_port = v.admin_port;
	m_peer_port = v.peer_port;
}
