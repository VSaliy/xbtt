// server.h: interface for the Cserver class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERVER_H__4D905E0B_7206_44A7_B536_848E7E677429__INCLUDED_)
#define AFX_SERVER_H__4D905E0B_7206_44A7_B536_848E7E677429__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "bt_admin_link.h"
#include "bt_file.h"
#include "bt_link.h"
#include "bt_tracker_link.h"
#include "stream_writer.h"

class Cserver  
{
public:
	typedef list<Cbt_admin_link> t_admins;
	typedef list<Cbt_file> t_files;
	typedef list<Cbt_link> t_links;

	int pre_dump() const;
	void dump(Cstream_writer&) const;
	ostream& dump(ostream&) const;
	void insert_peer(const t_bt_handshake& handshake, const sockaddr_in& a, const Csocket& s);
	void run();
	void stop();
	Cserver();
	~Cserver();

	int admin_port() const
	{
		return m_admin_port;
	}

	int peer_port() const
	{
		return m_peer_port;
	}
private:
	t_admins m_admins;
	t_files m_files;
	t_links m_links;

	int m_admin_port;
	int m_peer_port;
	bool m_run;
};

ostream& operator<<(ostream&, const Cserver&);

#endif // !defined(AFX_SERVER_H__4D905E0B_7206_44A7_B536_848E7E677429__INCLUDED_)
