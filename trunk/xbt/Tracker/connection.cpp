#include "stdafx.h"
#include "connection.h"

#include "bt_misc.h"
#include "bt_strings.h"
#include "server.h"
#include "xcc_z.h"

Cconnection::Cconnection()
{
}

Cconnection::Cconnection(Cserver* server, const Csocket& s, const sockaddr_in& a, bool log_access)
{
	m_server = server;
	m_s = s;
	m_a = a;
	m_ctime = server->time();

	m_state = 0;
	m_w = 0;
	m_log_access = log_access;
}

int Cconnection::pre_select(fd_set* fd_read_set, fd_set* fd_write_set)
{
	FD_SET(m_s, fd_read_set);
	if (!m_write_b.empty())
		FD_SET(m_s, fd_write_set);
	return m_s;
}

int Cconnection::post_select(fd_set* fd_read_set, fd_set* fd_write_set)
{
	return FD_ISSET(m_s, fd_read_set) && recv()
		|| FD_ISSET(m_s, fd_write_set) && send()
		|| m_server->time() - m_ctime > 15
		|| m_state == 5 && m_write_b.empty();
}

int Cconnection::recv()
{
	if (!m_read_b.size())
		m_read_b.resize(4 << 10);
	for (int r; r = m_s.recv(&m_read_b.front() + m_w, m_read_b.size() - m_w); )
	{
		if (r == SOCKET_ERROR)
		{
			int e = WSAGetLastError();
			switch (e)
			{
			case WSAECONNABORTED:
			case WSAECONNRESET:
				return 1;
			case WSAEWOULDBLOCK:
				return 0;
			}
			cerr << "recv failed: " << Csocket::error2a(e) << endl;
			return 1;
		}
		if (m_state == 5)
			return 0;
		char* a = &m_read_b.front() + m_w;
		m_w += r;
		int state;
		do
		{
			state = m_state;
			while (a < &m_read_b.front() + m_w && *a != '\n' && *a != '\r')
			{
				a++;
				if (m_state)
					m_state = 1;

			}
			if (a < &m_read_b.front() + m_w)
			{
				switch (m_state)
				{
				case 0:
					read(string(&m_read_b.front(), a));
					m_state = 1;
				case 1:
				case 3:
					m_state += *a == '\n' ? 2 : 1;
					break;
				case 2:
				case 4:
					m_state++;
					break;
				}
				a++;
			}
		}
		while (state != m_state);
		if (m_state == 5)
			return 0;
	}
	m_state = 5;
	return 0;
}

int Cconnection::send()
{
	for (int r; !m_write_b.empty() && (r = m_s.send(&m_write_b.front() + m_r, m_write_b.size() - m_r)); )
	{
		if (r == SOCKET_ERROR)
		{
			int e = WSAGetLastError();
			switch (e)
			{
			case WSAECONNABORTED:
			case WSAECONNRESET:
				return 1;
			case WSAEWOULDBLOCK:
				return 0;
			}
			cerr << "send failed: " << Csocket::error2a(e) << endl;
			return 0;
		}
		m_r += r;
		if (m_r == m_write_b.size())
		{
			m_write_b.clear();
			break;
		}
	}
	return 0;
}

static string calculate_torrent_pass1(const string& info_hash, long long torrent_pass_secret)
{
	Csha1 sha1;
	sha1.write(info_hash.c_str(), info_hash.size());
	torrent_pass_secret = htonll(torrent_pass_secret);
	sha1.write(&torrent_pass_secret, sizeof(torrent_pass_secret));
	return sha1.read();
}

void Cconnection::read(const string& v)
{
#ifndef NDEBUG
	cout << v << endl;
#endif
	if (m_log_access)
	{
		static ofstream f("xbt_tracker_raw.log");
		f << m_server->time() << '\t' << inet_ntoa(m_a.sin_addr) << '\t' << ntohs(m_a.sin_port) << '\t' << v << endl;
	}
	Ctracker_input ti;
	size_t a = v.find('?');
	if (a++ != string::npos)
	{
		size_t b = v.find(' ', a);
		if (b == string::npos)
			return;
		while (a < b)
		{
			size_t c = v.find('=', a);
			if (c++ == string::npos)
				break;
			size_t d = v.find_first_of(" &", c);
			if (d == string::npos)
				break;
			ti.set(v.substr(a, c - a - 1), uri_decode(v.substr(c, d - c)));
			a = d + 1;
		}
	}
	if (!ti.m_ipa || !is_private_ipa(m_a.sin_addr.s_addr))
		ti.m_ipa = m_a.sin_addr.s_addr;
	string torrent_pass0;
	string torrent_pass1;
	a = 4;
	if (a < v.size() && v[a] == '/')
	{
		a++;
		if (a + 1 < v.size() && v[a + 1] == '/')
			a += 2;
		if (a + 32 < v.size() && v[a + 32] == '/')
		{
			torrent_pass0 = v.substr(a, 32);
			a += 33;
			if (a + 40 < v.size() && v[a + 40] == '/')
			{
				torrent_pass1 = v.substr(a, 40);
				a += 41;
			}
		}
	}
	string h = "HTTP/1.0 200 OK\r\n";
	Cvirtual_binary s;
	bool gzip = true;
	switch (a < v.size() ? v[a] : 0)
	{
	case 'a':
		gzip = m_server->gzip_announce() && !ti.m_compact;
		if (ti.valid())
		{
			if (!ti.m_compact && !ti.m_no_peer_id && ti.m_event != Ctracker_input::e_stopped && ti.m_num_want && !ti.m_info_hash.empty())
				s = Cbvalue().d(bts_failure_reason, bts_unsupported_tracker_protocol).read();
			else
			{
				Cserver::t_user* user = m_server->find_user_by_torrent_pass(torrent_pass0);
				if (!m_server->anonymous_announce() && !user)
					s = Cbvalue().d(bts_failure_reason, bts_unregistered_torrent_pass).read();
				else if (user && user->torrent_pass_secret && calculate_torrent_pass1(ti.m_info_hash, user->torrent_pass_secret) != hex_decode(torrent_pass1))
					s = Cbvalue().d(bts_failure_reason, bts_unregistered_torrent_pass).read();
				else
				{
					string error = m_server->insert_peer(ti, ti.m_ipa == m_a.sin_addr.s_addr, false, user);
					s = (error.empty() ? m_server->select_peers(ti, user) : Cbvalue().d(bts_failure_reason, error)).read();
				}
			}
		}
		break;
	case 'd':
		if (m_server->debug())
		{
			gzip = m_server->gzip_debug();
			string v = m_server->debug(ti);
			h += "Content-Type: text/html; charset=us-ascii\r\n";
			s = Cvirtual_binary(v.c_str(), v.length());
		}
		break;
	case 's':
		if (v.length() >= 7 && v[6] == 't')
		{
			gzip = m_server->gzip_debug();
			string v = m_server->statistics();
			h += "Content-Type: text/html; charset=us-ascii\r\n";
			s = Cvirtual_binary(v.c_str(), v.length());
		}
		else
		{
			gzip = m_server->gzip_scrape() && ti.m_info_hash.empty();
			s = m_server->scrape(ti).read();
		}
		break;
	}
	if (!s.size())
	{
		if (m_server->redirect_url().empty())
			h = "HTTP/1.0 404 Not Found\r\n";
		else
		{
			h = "HTTP/1.0 302 Found\r\n"
				"Location: " + m_server->redirect_url() + (ti.m_info_hash.empty() ? "" : "?info_hash=" + uri_encode(ti.m_info_hash)) + "\r\n";
		}
	}
	else if (gzip)
	{
		Cvirtual_binary s2 = xcc_z::gzip(s);
#ifndef NDEBUG
		static ofstream f("xbt_tracker_gzip.log");
		f << m_server->time() << '\t' << v[5] << '\t' << s.size() << '\t' << s2.size() << '\t' << ti.m_compact << '\t' << (!ti.m_compact && ti.m_no_peer_id) << endl;
#endif
		if (s2.size() + 24 < s.size())
		{
			h += "Content-Encoding: gzip\r\n";
			s = s2;
		}
	}
	h += "\r\n";
	Cvirtual_binary d;
	memcpy(d.write_start(h.size() + s.size()), h.c_str(), h.size());
	s.read(d.data_edit() + h.size());
	int r = m_s.send(d, d.size());
	if (r == SOCKET_ERROR)
		cerr << "send failed: " << Csocket::error2a(WSAGetLastError()) << endl;
	else if (r != d.size())
	{
		m_write_b.resize(d.size() - r);
		memcpy(&m_write_b.front(), d + r, d.size() - r);
		m_r = 0;
	}
}

void Cconnection::process_events(int events)
{
	if (events & (EPOLLIN | EPOLLPRI | EPOLLERR | EPOLLHUP) && recv()
		|| events & EPOLLOUT && send()
		|| m_state == 5 && m_write_b.empty())
		m_s.close();
}

int Cconnection::run()
{
	return s() == INVALID_SOCKET || m_server->time() - m_ctime > 15;
}
