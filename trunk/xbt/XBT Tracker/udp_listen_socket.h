#if !defined(AFX_UDP_LISTEN_SOCKET_H__D7832887_B5B3_4E8B_9BE2_188B7D463745__INCLUDED_)
#define AFX_UDP_LISTEN_SOCKET_H__D7832887_B5B3_4E8B_9BE2_188B7D463745__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "client.h"

class Cserver;

class Cudp_listen_socket: public Cclient
{
public:
	virtual int process_events(int);

	Csocket m_s;
	Cserver* m_server;
};

#endif // !defined(AFX_UDP_LISTEN_SOCKET_H__D7832887_B5B3_4E8B_9BE2_188B7D463745__INCLUDED_)
