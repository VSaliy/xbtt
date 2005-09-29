#include "stdafx.h"
#include "dlg_block_list.h"

#include "socket.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Cdlg_block_list dialog


Cdlg_block_list::Cdlg_block_list(CWnd* pParent /*=NULL*/)
	: ETSLayoutDialog(Cdlg_block_list::IDD, pParent, "Cdlg_block_list")
{
	//{{AFX_DATA_INIT(Cdlg_block_list)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void Cdlg_block_list::DoDataExchange(CDataExchange* pDX)
{
	ETSLayoutDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(Cdlg_block_list)
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Control(pDX, IDC_DELETE, m_delete);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(Cdlg_block_list, ETSLayoutDialog)
	//{{AFX_MSG_MAP(Cdlg_block_list)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST, OnGetdispinfoList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, OnItemchangedList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Cdlg_block_list message handlers

BOOL Cdlg_block_list::OnInitDialog() 
{
	ETSLayoutDialog::OnInitDialog();
	CreateRoot(VERTICAL)
		<< item(IDC_LIST, GREEDY)
		<< (pane(HORIZONTAL, ABSOLUTE_VERT)
			<< itemGrowing(HORIZONTAL)
			<< item(IDC_DELETE, NORESIZE)
			<< item(IDOK, NORESIZE)
			<< item(IDCANCEL, NORESIZE)
			)
		;
	UpdateLayout();

	m_list.InsertColumn(0, "IP address");
	for (t_entries::const_iterator i = m_entries.begin(); i != m_entries.end(); i++)
		m_list.InsertItemData(*i);
	m_list.auto_size();
	update_controls();
	return true;
}

void Cdlg_block_list::OnDelete() 
{
	int index;
	while ((index = m_list.GetNextItem(-1, LVNI_SELECTED)) != -1)
	{
		m_entries.erase(m_list.GetItemData(index));
		m_list.DeleteItem(index);
	}
}

void Cdlg_block_list::OnGetdispinfoList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = reinterpret_cast<LV_DISPINFO*>(pNMHDR);
	string& buffer = m_list.get_buffer();
	switch (pDispInfo->item.iSubItem)
	{
	case 0:
		buffer = Csocket::inet_ntoa(pDispInfo->item.lParam);
		break;
	}
	pDispInfo->item.pszText = const_cast<char*>(buffer.c_str());
	*pResult = 0;
}

void Cdlg_block_list::update_controls()
{
	m_delete.EnableWindow(m_list.GetSelectedCount());
}

void Cdlg_block_list::OnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	update_controls();
	*pResult = 0;
}
