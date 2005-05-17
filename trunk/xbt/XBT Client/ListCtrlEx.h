#if !defined(AFX_LISTCTRLEX_H__1A7B4E56_9CB8_4BA4_B5AC_AF5DA38C4285__INCLUDED_)
#define AFX_LISTCTRLEX_H__1A7B4E56_9CB8_4BA4_B5AC_AF5DA38C4285__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>

class CListCtrlEx: public CListCtrl
{
public:
	//{{AFX_VIRTUAL(CListCtrlEx)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL
public:
	std::string& get_buffer();
	std::string get_selected_rows_tsv();
	void select_all();
	void DeleteAllColumns();
	DWORD GetItemData(int nItem) const;
	int InsertItemData(int nItem, DWORD dwData);
	int InsertItemData(DWORD dwData);
	void auto_size();
protected:
	//{{AFX_MSG(CListCtrlEx)
	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	std::string m_buffer[4];
	int m_buffer_w;
};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_LISTCTRLEX_H__1A7B4E56_9CB8_4BA4_B5AC_AF5DA38C4285__INCLUDED_)
