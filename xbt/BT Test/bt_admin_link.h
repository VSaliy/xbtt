// bt_admin_link.h: interface for the Cbt_admin_link class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BT_ADMIN_LINK_H__C5AA1CA0_8023_42E9_A747_00CF206F5833__INCLUDED_)
#define AFX_BT_ADMIN_LINK_H__C5AA1CA0_8023_42E9_A747_00CF206F5833__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ring_buffer.h"
#include "socket.h"

class Cbt_admin_link  
{
public:
	void close();
	int pre_select(fd_set* fd_read_set, fd_set* fd_write_set, fd_set* fd_except_set);
	void post_select(fd_set* fd_read_set, fd_set* fd_write_set, fd_set* fd_except_set);
	void recv();
	void send();
	Cbt_admin_link();
	Cbt_admin_link(const sockaddr_in& a, const Csocket& s);
	~Cbt_admin_link();

	Csocket s()
	{
		return m_s;
	}
private:	
	Cring_buffer m_read_b;
	Cring_buffer m_write_b;
	sockaddr_in m_a;
	Csocket m_s;
	int m_ctime;
	int m_mtime;
};

#endif // !defined(AFX_BT_ADMIN_LINK_H__C5AA1CA0_8023_42E9_A747_00CF206F5833__INCLUDED_)
