// dlg_profiles.cpp : implementation file
//

#include "stdafx.h"
#include "dlg_profiles.h"

#include "dlg_profile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Cdlg_profiles dialog


Cdlg_profiles::Cdlg_profiles(CWnd* pParent /*=NULL*/): 
	ETSLayoutDialog(Cdlg_profiles::IDD, pParent, "Cdlg_profiles")
{
	//{{AFX_DATA_INIT(Cdlg_profiles)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void Cdlg_profiles::DoDataExchange(CDataExchange* pDX)
{
	ETSLayoutDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(Cdlg_profiles)
	DDX_Control(pDX, IDC_LIST, m_list);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(Cdlg_profiles, ETSLayoutDialog)
	//{{AFX_MSG_MAP(Cdlg_profiles)
	ON_BN_CLICKED(IDC_INSERT, OnInsert)
	ON_BN_CLICKED(IDC_EDIT, OnEdit)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST, OnGetdispinfoList)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST, OnDblclkList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Cdlg_profiles message handlers

BOOL Cdlg_profiles::OnInitDialog() 
{
	ETSLayoutDialog::OnInitDialog();
	CreateRoot(VERTICAL)
		<< item(IDC_LIST, GREEDY)
		<< (pane(HORIZONTAL, ABSOLUTE_VERT)
			<< itemGrowing(HORIZONTAL)
			<< item(IDC_INSERT, NORESIZE)
			<< item(IDC_EDIT, NORESIZE)
			<< item(IDC_DELETE, NORESIZE)
			<< item(IDOK, NORESIZE)
			<< item(IDCANCEL, NORESIZE)
			)
		;
	UpdateLayout();

	m_list.InsertColumn(0, "Name");
	m_list.InsertColumn(1, "Seeding Ratio", LVCFMT_RIGHT);
	m_list.InsertColumn(2, "Upload Rate", LVCFMT_RIGHT);
	m_list.InsertColumn(3, "Upload Slots", LVCFMT_RIGHT);
	m_list.InsertColumn(4, "Peer Limit", LVCFMT_RIGHT);
	m_list.InsertColumn(5, "Torrent Limit", LVCFMT_RIGHT);
	m_list.InsertColumn(6, "");
	m_list.auto_size();
	return true;
}

void Cdlg_profiles::OnInsert() 
{
	Cdlg_profile dlg(this);
	if (IDOK != dlg.DoModal())
		return;
	t_entry e;
	e.name = dlg.m_name;
	e.peer_limit = dlg.m_peer_limit_value;
	e.peer_limit_enable = dlg.m_peer_limit_enable;
	e.seeding_ratio = dlg.m_seeding_ratio_value;
	e.seeding_ratio_enable = dlg.m_seeding_ratio_enable;
	e.torrent_limit = dlg.m_torrent_limit_value;
	e.torrent_limit_enable = dlg.m_torrent_limit_enable;
	e.upload_rate = dlg.m_upload_rate_value;
	e.upload_rate_enable = dlg.m_upload_rate_enable;
	e.upload_slots = dlg.m_upload_slots_value;
	e.upload_slots_enable = dlg.m_upload_slots_enable;
	insert(e);
	m_list.auto_size();
}

void Cdlg_profiles::OnEdit() 
{
	int index = m_list.GetNextItem(-1, LVNI_FOCUSED);
	if (index == -1)
		return;
	int id = m_list.GetItemData(index);
	t_entry& e = m_entries.find(id)->second;
	Cdlg_profile dlg(this);
	dlg.m_name = e.name.c_str();
	dlg.m_peer_limit_value = e.peer_limit;
	dlg.m_peer_limit_enable = e.peer_limit_enable;
	dlg.m_seeding_ratio_value = e.seeding_ratio;
	dlg.m_seeding_ratio_enable = e.seeding_ratio_enable;
	dlg.m_torrent_limit_value = e.torrent_limit;
	dlg.m_torrent_limit_enable = e.torrent_limit_enable;
	dlg.m_upload_rate_value = e.upload_rate;
	dlg.m_upload_rate_enable = e.upload_rate_enable;
	dlg.m_upload_slots_value = e.upload_slots;
	dlg.m_upload_slots_enable = e.upload_slots_enable;
	if (IDOK != dlg.DoModal())
		return;
	e.name = dlg.m_name;
	e.peer_limit = dlg.m_peer_limit_value;
	e.peer_limit_enable = dlg.m_peer_limit_enable;
	e.seeding_ratio = dlg.m_seeding_ratio_value;
	e.seeding_ratio_enable = dlg.m_seeding_ratio_enable;
	e.torrent_limit = dlg.m_torrent_limit_value;
	e.torrent_limit_enable = dlg.m_torrent_limit_enable;
	e.upload_rate = dlg.m_upload_rate_value;
	e.upload_rate_enable = dlg.m_upload_rate_enable;
	e.upload_slots = dlg.m_upload_slots_value;
	e.upload_slots_enable = dlg.m_upload_slots_enable;
	m_list.Update(index);
}

void Cdlg_profiles::OnDelete() 
{
	int index = m_list.GetNextItem(-1, LVNI_FOCUSED);
	if (index == -1)
		return;
	m_entries.erase(m_list.GetItemData(index));
	m_list.DeleteItem(index);
}

void Cdlg_profiles::insert(const t_entry& e)
{
	int id = m_entries.empty() ? 0 : m_entries.rbegin()->first + 1;
	m_entries[id] = e;
	if (m_list.GetSafeHwnd())
		m_list.SetItemData(m_list.InsertItem(m_list.GetItemCount(), LPSTR_TEXTCALLBACK), id);
}

void Cdlg_profiles::OnGetdispinfoList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = reinterpret_cast<LV_DISPINFO*>(pNMHDR);
	m_buffer[++m_buffer_w &= 3].erase();
	const t_entry& e = m_entries.find(pDispInfo->item.lParam)->second;
	switch (pDispInfo->item.iSubItem)
	{
	case 0:
		m_buffer[m_buffer_w] = e.name;
		break;
	case 1:
		if (e.seeding_ratio_enable)
			m_buffer[m_buffer_w] = n(e.seeding_ratio);
		break;
	case 2:
		if (e.upload_rate_enable)
			m_buffer[m_buffer_w] = n(e.upload_rate);
		break;
	case 3:
		if (e.upload_slots_enable)
			m_buffer[m_buffer_w] = n(e.upload_slots);
		break;
	case 4:
		if (e.peer_limit_enable)
			m_buffer[m_buffer_w] = n(e.peer_limit);
		break;
	case 5:
		if (e.torrent_limit_enable)
			m_buffer[m_buffer_w] = n(e.torrent_limit);
		break;
	}
	pDispInfo->item.pszText = const_cast<char*>(m_buffer[m_buffer_w].c_str());
	*pResult = 0;
}

void Cdlg_profiles::OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	EndDialog(IDOK);
	*pResult = 0;
}
