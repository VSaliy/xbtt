#if !defined(AFX_DLG_OPTIONS_H__F88A336D_3B46_4580_8ACF_F796B1E0ED0F__INCLUDED_)
#define AFX_DLG_OPTIONS_H__F88A336D_3B46_4580_8ACF_F796B1E0ED0F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// dlg_options.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// Cdlg_options dialog

class Cdlg_options : public CDialog
{
// Construction
public:
	struct t_data
	{
		int admin_port;
		int peer_port;
		string public_ipa;
		int upload_rate;
		int upload_slots;
	};

	t_data get() const;
	void set(const t_data&);
	Cdlg_options(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(Cdlg_options)
	enum { IDD = IDD_OPTIONS };
	int		m_peer_port;
	int		m_admin_port;
	int		m_upload_rate;
	CString	m_public_ipa;
	int		m_upload_slots;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(Cdlg_options)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(Cdlg_options)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLG_OPTIONS_H__F88A336D_3B46_4580_8ACF_F796B1E0ED0F__INCLUDED_)
