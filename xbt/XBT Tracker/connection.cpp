// connection.cpp: implementation of the Cconnection class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "connection.h"

#include "server.h"
#include "xcc_z.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Cconnection::Cconnection()
{
}

Cconnection::Cconnection(Cserver* server, const Csocket& s, const sockaddr_in& a)
{
	m_server = server;
	m_s = s;
	m_a = a;
	m_ctime = time(NULL);
	
	m_read_b.resize(4 << 10);
	m_w = 0;
}

int Cconnection::pre_select(fd_set* fd_read_set)
{
	FD_SET(m_s, fd_read_set);
	return m_s;
}

void Cconnection::post_select(fd_set* fd_read_set)
{
	if (FD_ISSET(m_s, fd_read_set))
		recv();
	if (time(NULL) - m_ctime > 15)
		close();
}

void Cconnection::recv()
{
	for (int r; r = m_s.recv(&m_read_b.front() + m_w, m_read_b.size() - m_w); )
	{
		if (r == SOCKET_ERROR)
		{
			int e = WSAGetLastError();
			switch (e)
			{
			case WSAECONNABORTED:
			case WSAECONNRESET:
				close();
			case WSAEWOULDBLOCK:
				break;
			default:
				cerr << "recv failed: " << e << endl;
				close();
			}
			return;
		}
		char* a = &m_read_b.front() + m_w;
		m_w += r;
		while (a < &m_read_b.front() + m_w && *a != '\n' && *a != '\r')
			a++;
		if (a < &m_read_b.front() + m_w)
		{
			read(string(&m_read_b.front(), a));
			break;
		}
	}
	close();
}

void Cconnection::close()
{
	m_s.close();
}

void Cconnection::read(const string& v)
{
	cout << v << endl;
	{
		static ofstream f("xbt_tracker_raw.log");
		f << time(NULL) << '\t' << inet_ntoa(m_a.sin_addr) << '\t' << ntohs(m_a.sin_port) << '\t' << v << endl;
	}
	Ctracker_input ti;
	int a = v.find('?');
	if (a++ != string::npos)
	{
		int b = v.find(' ', a);
		if (b == string::npos)
			return;
		while (a < b)
		{
			int c = v.find('=', a);
			if (c++ == string::npos)
				break;
			int d = v.find_first_of(" &", c);
			assert(d != string::npos);
			ti.set(v.substr(a, c - a - 1), uri_decode(v.substr(c, d - c)));
			a = d + 1;
		}
	}
	if (!ti.m_ipa || !is_private_ipa(m_a.sin_addr.s_addr))
		ti.m_ipa = m_a.sin_addr.s_addr;
	Cvirtual_binary s;
	bool gzip = true;
	switch (v.length() >= 5 ? v[5] : 0) 
	{
	case 'a':
		gzip = m_server->gzip_announce();
		if (ti.valid())
		{
			m_server->insert_peer(ti);
			s = m_server->select_peers(ti).read();
		}
		break;
	case 'd':
		gzip = m_server->gzip_debug();
		{
			string v = m_server->debug(ti);
			s = Cvirtual_binary(v.c_str(), v.length());
		}
		break;
	case 's':
		gzip = m_server->gzip_scrape();
		s = m_server->scrape(ti).read();
		break;
	}
	if (!s)
		gzip = false;
	else if (gzip)
		s = xcc_z::gzip(s);
	const char* h = gzip
		? "HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\nContent-Encoding: gzip\r\n\r\n"
		: "HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\n\r\n";
	Cvirtual_binary d;
	memcpy(d.write_start(strlen(h) + s.size()), h, strlen(h));
	s.read(d.data_edit() + strlen(h));
	if (m_s.send(d, d.size()) != d.size())
		cerr << "send failed" << endl;
}
