// XBT ClientDlg.h : header file
//

#if !defined(AFX_XBTCLIENTDLG_H__24B01140_CC8B_4862_B4FD_31A9CF22FAF8__INCLUDED_)
#define AFX_XBTCLIENTDLG_H__24B01140_CC8B_4862_B4FD_31A9CF22FAF8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../bt test/server.h"
#include "resource.h"
#include "stream_reader.h"

/////////////////////////////////////////////////////////////////////////////
// CXBTClientDlg dialog

class CXBTClientDlg : public ETSLayoutDialog
{
// Construction
public:
	void set_priority(int v);
	void set_clipboard(const string&);
	void lower_process_priority(bool);
	void set_dir(const string&);
	void insert_columns();
	void insert_top_columns();
	void insert_bottom_columns();
	void sort_peers();
	void sort_files();
	int files_compare(int id_a, int id_b) const;
	int peers_compare(int id_a, int id_b) const;
	static unsigned int server_thread(void* p);
	void start_server();
	void stop_server();
	void register_tray();
	void unregister_tray();
	void update_tray();
	void update_tray(const char* info_title, const char* info);
	void auto_size_files();
	void auto_size_peers();
	void auto_size();
	void fill_peers();
	void open(const string& name, bool ask_for_location);
	void open_url(const string&);
	CXBTClientDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CXBTClientDlg)
	enum { IDD = IDD_XBTCLIENT_DIALOG };
	CListCtrl	m_peers;
	CListCtrl	m_files;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CXBTClientDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	afx_msg void OnContextMenu(CWnd*, CPoint point);
	afx_msg void OnCustomdrawFiles(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawPeers(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg long OnHotKey(WPARAM, LPARAM);
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CXBTClientDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnGetdispinfoDetails(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGetdispinfoEvents(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGetdispinfoFiles(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGetdispinfoPeers(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGetdispinfoSubFiles(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGetdispinfoTrackers(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnItemchangedFiles(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnPopupOpen();
	afx_msg void OnPopupClose();
	afx_msg void OnUpdatePopupClose(CCmdUI* pCmdUI);
	afx_msg void OnPopupOptions();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnPopupExit();
	afx_msg void OnPopupExplore();
	afx_msg void OnDestroy();
	afx_msg void OnPopupStart();
	afx_msg void OnPopupStop();
	afx_msg void OnWindowPosChanging(WINDOWPOS FAR* lpwndpos);
	afx_msg void OnEndSession(BOOL bEnding);
	afx_msg void OnColumnclickFiles(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnclickPeers(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkFiles(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPopupCopy();
	afx_msg void OnPopupPaste();
	afx_msg void OnPopupFiles();
	afx_msg void OnPopupTrackers();
	afx_msg void OnPopupAnnounce();
	afx_msg void OnPopupExploreTracker();
	afx_msg void OnPopupAbout();
	afx_msg void OnPopupMakeTorrent();
	afx_msg void OnPopupTorrentDelete();
	afx_msg void OnPopupTorrentClipboardCopyAnnounceUrl();
	afx_msg void OnPopupTorrentClipboardCopyHash();
	afx_msg void OnPopupTorrentAlerts();
	afx_msg void OnPopupViewDetails();
	afx_msg void OnPopupViewFiles();
	afx_msg void OnPopupViewPeers();
	afx_msg void OnPopupViewTrackers();
	afx_msg void OnPopupViewEvents();
	afx_msg void OnPopupPriorityExclude();
	afx_msg void OnPopupPriorityHigh();
	afx_msg void OnPopupPriorityLow();
	afx_msg void OnPopupPriorityNormal();
	afx_msg void OnPopupViewAdvancedColumns();
	afx_msg void OnPopupViewTrayIcon();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	struct t_event
	{
		time_t time;
		int level;
		string message;
		string source;
	};

	struct t_peer
	{
		in_addr host;
		int port;
		string peer_id;
		__int64 downloaded;
		__int64 left;
		__int64 uploaded;
		int down_rate;
		int up_rate;
		bool local_link;
		bool local_choked;
		bool local_interested;
		bool remote_choked;
		bool remote_interested;
		bool removed;
	};

	struct t_sub_file
	{
		string hash;
		__int64 left;
		string name;
		int priority;
		__int64 size;
	};

	struct t_tracker
	{
		string url;
	};

	typedef vector<t_event> t_events;
	typedef map<int, t_peer> t_peers;
	typedef vector<t_sub_file> t_sub_files;
	typedef vector<t_tracker> t_trackers;

	struct t_file
	{
		string display_name;
		string info_hash;
		string name;
		t_events events;
		t_trackers trackers;
		t_peers peers;
		t_sub_files sub_files;
		__int64 downloaded;
		__int64 left;
		__int64 size;
		__int64 uploaded;
		__int64 total_downloaded;
		__int64 total_uploaded;
		int down_rate;
		int up_rate;
		int c_leechers;
		int c_seeders;
		int c_leechers_total;
		int c_seeders_total;
		int c_invalid_pieces;
		int c_valid_pieces;
		int cb_piece;
		bool hashing;
		bool running;
		bool removed;
	};

	typedef vector<int> t_columns;
	typedef map<int, t_file> t_files;

	void read_peer_dump(t_file& f, Cstream_reader& sr);
	void read_file_dump(Cstream_reader& sr);
	void read_server_dump(Cstream_reader& sr);

	bool m_ask_for_location;
	bool m_initial_hide;
	string m_buffer[4];
	int m_buffer_w;
	t_columns m_torrents_columns;
	t_columns m_peers_columns;
	t_file* m_file;
	t_files m_files_map;
	CString m_dir;
	CString m_reg_key;
	Cserver m_server;
	CWinThread* m_server_thread;
	int m_bottom_view;
	int m_files_sort_column;
	int m_peers_sort_column;
	bool m_files_sort_reverse;
	bool m_peers_sort_reverse;
	bool m_show_advanced_columns;
	bool m_show_tray_icon;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_XBTCLIENTDLG_H__24B01140_CC8B_4862_B4FD_31A9CF22FAF8__INCLUDED_)
