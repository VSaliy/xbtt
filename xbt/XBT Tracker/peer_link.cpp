// peer_link.cpp: implementation of the Cpeer_link class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "peer_link.h"

#include "bt_misc.h"
#include "server.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Cpeer_link::Cpeer_link()
{
}

Cpeer_link::Cpeer_link(int h, int p, Cserver* server, const string& file_id, int peer_id)
{
	if (m_s.open(SOCK_STREAM) == INVALID_SOCKET)
		cerr << "socket failed: " << error2a(WSAGetLastError()) << endl;
	else if (m_s.connect(h, p) && WSAGetLastError() != WSAEWOULDBLOCK)
		cerr << "connect failed: " << error2a(WSAGetLastError()) << endl;
	m_ctime = time(NULL);
	m_server = server;
	m_file_id = file_id;
	m_peer_id = peer_id;
}

int Cpeer_link::pre_select(fd_set* fd_write_set, fd_set* fd_except_set)
{
	FD_SET(m_s, fd_write_set);
	FD_SET(m_s, fd_except_set);
	return m_s;
}

int Cpeer_link::post_select(fd_set* fd_write_set, fd_set* fd_except_set)
{
	if (FD_ISSET(m_s, fd_write_set))
	{
		m_server->update_peer(m_file_id, m_peer_id, true);
		return 1;
	}
	return FD_ISSET(m_s, fd_except_set) || time(NULL) - m_ctime > 30;
}
