// transaction.cpp: implementation of the Ctransaction class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "transaction.h"

#include "server.h"
#include "sha1.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Ctransaction::Ctransaction(Cserver& server, const Csocket& s):
	m_server(server)
{
	m_s = s;
}

__int64 Ctransaction::connection_id() const
{
	const int cb_s = 8 + sizeof(int);
	char s[cb_s];
	*reinterpret_cast<__int64*>(s) = m_server.secret();
	*reinterpret_cast<int*>(s + 8) = m_a.sin_addr.s_addr;
	char d[20];
	compute_sha1(&s, cb_s, d);
	return *reinterpret_cast<__int64*>(d);
}

void Ctransaction::recv()
{
	const int cb_b = 2 << 10;
	char b[cb_b];
	socklen_t cb_a = sizeof(sockaddr_in);
	int r = m_s.recvfrom(b, cb_b, reinterpret_cast<sockaddr*>(&m_a), &cb_a);
	if (r == SOCKET_ERROR)
	{
		cerr << "recv failed: " << WSAGetLastError() << endl;
		return;
	}
	if (r < sizeof(t_udp_tracker_input))
		return;
	const t_udp_tracker_input& uti = *reinterpret_cast<t_udp_tracker_input*>(b);
	switch (uti.action())
	{
	case uta_connect:
		if (r >= sizeof(t_udp_tracker_input_connect))
			send_connect(*reinterpret_cast<t_udp_tracker_input_connect*>(b), b + sizeof(t_udp_tracker_input_connect), b + r);
		break;
	case uta_announce:
		if (r >= sizeof(t_udp_tracker_input_announce))
			send_announce(*reinterpret_cast<t_udp_tracker_input_announce*>(b), b + sizeof(t_udp_tracker_input_announce), b + r);
		break;
	case uta_scrape:
		if (r >= sizeof(t_udp_tracker_input_scrape))
			send_scrape(*reinterpret_cast<t_udp_tracker_input_scrape*>(b), b + sizeof(t_udp_tracker_input_scrape), b + r);
		break;
	}
}

void Ctransaction::send_connect(const t_udp_tracker_input_connect& uti, const char* r, const char* r_end)
{
	t_udp_tracker_output_connect uto;
	uto.action(uta_connect);
	uto.transaction_id(uti.transaction_id());
	uto.m_connection_id = connection_id();
	send(&uto, sizeof(t_udp_tracker_output_connect));
}

void Ctransaction::send_announce(const t_udp_tracker_input_announce& uti, const char* r, const char* r_end)
{
	if (uti.m_connection_id != connection_id())
		return;
	const int cb_b = 2 << 10;
	char b[cb_b];
	Ctracker_input ti;
	ti.m_downloaded = uti.downloaded();
	ti.m_event = static_cast<Ctracker_input::t_event>(uti.event());
	ti.m_info_hash = uti.info_hash();
	ti.m_ipa = uti.ipa() && is_private_ipa(m_a.sin_addr.s_addr)
		? uti.ipa()
		: m_a.sin_addr.s_addr;
	ti.m_left = uti.left();
	ti.m_num_want = uti.num_want();
	ti.m_peer_id = uti.peer_id();
	ti.m_port = uti.port();
	ti.m_uploaded = uti.uploaded();
	m_server.insert_peer(ti);
	const Cserver::t_files& files = m_server.files();
	Cserver::t_files::const_iterator i = files.find(ti.m_info_hash);
	if (i == files.end())
	{
		send_error(uti, "invalid info hash");
		return;
	}
	t_udp_tracker_output_announce& uto = *reinterpret_cast<t_udp_tracker_output_announce*>(b);
	uto.action(uta_announce);
	uto.transaction_id(uti.transaction_id());
	uto.interval(m_server.announce_interval());
	uto.leechers(i->second.leechers);
	uto.seeders(i->second.seeders);
	t_udp_tracker_output_peer* peer = reinterpret_cast<t_udp_tracker_output_peer*>(b + sizeof(t_udp_tracker_output_announce));
	int c = ti.m_num_want < 0 ? 100 : min(ti.m_num_want, 100);
	for (Cserver::t_peers::const_iterator j = i->second.peers.begin(); j != i->second.peers.end(); j++)
	{
		if (!ti.m_left && !j->second.left || !j->second.listening)
			continue;
		if (!c--)
			break;
		peer->host(j->first);
		peer->port(j->second.port);
		peer++;
	}
	send(b, reinterpret_cast<char*>(peer) - b);
}

void Ctransaction::send_scrape(const t_udp_tracker_input_scrape& uti, const char* r, const char* r_end)
{
	if (uti.m_connection_id != connection_id())
		return;
	const int cb_b = 2 << 10;
	char b[cb_b];
	t_udp_tracker_output_scrape& uto = *reinterpret_cast<t_udp_tracker_output_scrape*>(b);
	uto.transaction_id(uti.transaction_id());
	uto.action(uta_scrape);
	t_udp_tracker_output_file* file = reinterpret_cast<t_udp_tracker_output_file*>(b + sizeof(t_udp_tracker_output_scrape));
	const Cserver::t_files& files = m_server.files();
	for (; r + 20 <= r_end && reinterpret_cast<char*>(file + 1) <= b + cb_b; r += 20)
	{
		Cserver::t_files::const_iterator i = files.find(string(r, 20));
		if (i == files.end())
		{
			file->complete(0);
			file->downloaded(0);
			file->incomplete(0);
		}
		else
		{
			file->complete(i->second.seeders);
			file->downloaded(i->second.completed);
			file->incomplete(i->second.leechers);
		}
		file++;
	}
	send(b, reinterpret_cast<char*>(file) - b);
}

void Ctransaction::send_error(const t_udp_tracker_input& uti, const string& msg)
{
	const int cb_b = 2 << 10;
	char b[cb_b];
	t_udp_tracker_output_error& uto = *reinterpret_cast<t_udp_tracker_output_error*>(b);
	uto.action(uta_error);
	uto.transaction_id(uti.transaction_id());
	memcpy(b + sizeof(t_udp_tracker_output_error), msg.c_str(), msg.length());
	send(b, sizeof(t_udp_tracker_output_error) + msg.length());
}

void Ctransaction::send(const void* b, int cb_b)
{
	if (m_s.sendto(b, cb_b, reinterpret_cast<const sockaddr*>(&m_a), sizeof(sockaddr_in)) != cb_b)
		cerr << "send failed: " << WSAGetLastError() << endl;
}
