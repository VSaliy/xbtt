// server.cpp: implementation of the Cserver class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "server.h"

#include "bt_strings.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Cserver::Cserver(Cdatabase& database):
	m_database(database)
{
	m_fid_end = 0;

	for (int i = 0; i < 8; i++)
		m_secret = m_secret << 8 ^ rand();
}

void Cserver::run()
{
	read_config();
	t_sockets lt, lu;
	for (t_listen_ports::const_iterator i = m_listen_ports.begin(); i != m_listen_ports.end(); i++)
	{
		Csocket l0, l1;
		if (l0.open(SOCK_STREAM) == INVALID_SOCKET)			
			cerr << "socket failed: " << WSAGetLastError() << endl;
		else if (l0.bind(htonl(INADDR_ANY), htons(*i)))			
			cerr << "bind failed: " << WSAGetLastError() << endl;
		else if (l0.listen())
			cerr << "listen failed: " << WSAGetLastError() << endl;
		else
			lt.push_back(l0);
		if (l1.open(SOCK_DGRAM) == INVALID_SOCKET)
			cerr << "socket failed: " << WSAGetLastError() << endl;
		else if (l1.bind(htonl(INADDR_ANY), htons(*i)))
			cerr << "bind failed: " << WSAGetLastError() << endl;
		else
			lu.push_back(l1);
	}
	if (lt.empty() && lu.empty())
		return;
#ifndef WIN32
	if (m_daemon && daemon(true, false))
		cerr << "daemon failed" << endl;
	ofstream("xbt_tracker.pid") << getpid() << endl;
#endif
	clean_up();
	read_db();
	write_db();
	fd_set fd_read_set;
	fd_set fd_write_set;
	fd_set fd_except_set;
	while (1)
	{
		FD_ZERO(&fd_read_set);
		FD_ZERO(&fd_write_set);
		FD_ZERO(&fd_except_set);
		int n = 0;
		{
			for (t_connections::iterator i = m_connections.begin(); i != m_connections.end(); i++)
			{
				int z = i->pre_select(&fd_read_set, &fd_write_set);
				n = max(n, z);
			}
		}
		{
			for (t_peer_links::iterator i = m_peer_links.begin(); i != m_peer_links.end(); i++)
			{
				int z = i->pre_select(&fd_write_set, &fd_except_set);
				n = max(n, z);
			}
		}		
		for (t_sockets::iterator i = lt.begin(); i != lt.end(); i++)
		{
			FD_SET(*i, &fd_read_set);
			n = max(n, *i);
		}
		for (t_sockets::iterator i = lu.begin(); i != lu.end(); i++)
		{
			FD_SET(*i, &fd_read_set);
			n = max(n, *i);
		}
		if (select(n + 1, &fd_read_set, &fd_write_set, &fd_except_set, NULL) == SOCKET_ERROR)
			cerr << "select failed: " << WSAGetLastError() << endl;
		else 
		{
			for (t_sockets::iterator i = lt.begin(); i != lt.end(); i++)
			{
				if (!FD_ISSET(*i, &fd_read_set))
					continue;
				sockaddr_in a;
				socklen_t cb_a = sizeof(sockaddr_in);
				Csocket s = accept(*i, reinterpret_cast<sockaddr*>(&a), &cb_a);
				if (s == SOCKET_ERROR)
					cerr << "accept failed: " << WSAGetLastError() << endl;
				else
				{
					if (s.blocking(false))
						cerr << "ioctlsocket failed: " << WSAGetLastError() << endl;
					m_connections.push_front(Cconnection(this, s, a));
				}
			}
			for (t_sockets::iterator i = lu.begin(); i != lu.end(); i++)
			{
				if (FD_ISSET(*i, &fd_read_set))
					Ctransaction(*this, *i).recv();
			}
			{
				for (t_connections::iterator i = m_connections.begin(); i != m_connections.end(); )
				{
					i->post_select(&fd_read_set, &fd_write_set);
					if (*i)
						i++;
					else
						i = m_connections.erase(i);
				}
			}
			{
				for (t_peer_links::iterator i = m_peer_links.begin(); i != m_peer_links.end(); )
				{
					i->post_select(&fd_write_set, &fd_except_set);
					if (*i)
						i++;
					else
						i = m_peer_links.erase(i);
				}
			}
		}
		if (time(NULL) - m_read_config_time > m_read_config_interval)
			read_config();
		if (time(NULL) - m_clean_up_time > m_clean_up_interval)
			clean_up();
		if (time(NULL) - m_write_db_time > m_write_db_interval)
			write_db();
	}
}

void Cserver::insert_peer(const Ctracker_input& v)
{
	if (m_log)
	{
		Csql_query q(m_database);
		q.write("(%d, %d, %d, %s, %s, %d, %d, %d, %d)");
		q.p(ntohl(v.m_ipa));
		q.p(ntohs(v.m_port));
		q.p(v.m_event);
		q.pe(v.m_info_hash);
		q.pe(v.m_peer_id);
		q.p(v.m_downloaded);
		q.p(v.m_left);
		q.p(v.m_uploaded);
		q.p(time(NULL));
		if (!m_announce_log_buffer.empty())
			m_announce_log_buffer += ", ";
		m_announce_log_buffer += q.read();
	}
	if (!m_auto_register && m_files.find(v.m_info_hash) == m_files.end())
	{
		if (time(NULL) - m_read_db_time > m_read_db_interval)
			read_db();
		if (m_files.find(v.m_info_hash) == m_files.end())
			return;
	}
	t_file& file = m_files[v.m_info_hash];
	t_peers::iterator i = file.peers.find(v.m_ipa);
	if (i != file.peers.end())
		(i->second.left ? file.leechers : file.seeders)--;
	if (v.m_event == Ctracker_input::e_stopped)
		file.peers.erase(v.m_ipa);
	else
	{
		t_peer& peer = file.peers[v.m_ipa];
		// peer.downloaded = v.m_downloaded;
		peer.left = v.m_left;
		peer.peer_id = v.m_peer_id;
		peer.port = v.m_port;
		// peer.uploaded = v.m_uploaded;
		(peer.left ? file.leechers : file.seeders)++;

		if (!m_listen_check)
			peer.listening = true;
		else if (!peer.listening && time(NULL) - peer.mtime > 900)
		{
			Cpeer_link peer_link(v.m_ipa, v.m_port, this, v.m_info_hash, v.m_ipa);
			if (peer_link)
				m_peer_links.push_front(peer_link);
		}
		peer.mtime = time(NULL);
	}
	switch (v.m_event)
	{
	case Ctracker_input::e_completed:
		file.completed++;
		break;
	case Ctracker_input::e_started:
		file.started++;
		break;
	case Ctracker_input::e_stopped:
		file.stopped++;
		break;
	}
	file.announced++;
	file.dirty = true;
}

void Cserver::update_peer(const string& file_id, int peer_id, bool listening)
{
	t_files::iterator i = m_files.find(file_id);
	if (i == m_files.end())
		return;
	t_peers::iterator j = i->second.peers.find(peer_id);
	if (j == i->second.peers.end())
		return;
	j->second.listening = listening;
}

Cbvalue Cserver::t_file::select_peers(const Ctracker_input& ti) const
{
	typedef vector<t_peers::const_iterator> t_candidates;

	t_candidates candidates;
	{
		for (t_peers::const_iterator i = peers.begin(); i != peers.end(); i++)
		{
			if ((ti.m_left || i->second.left) && i->second.listening)
				candidates.push_back(i);
		}		
	}
	int c = ti.m_num_want < 0 ? 50 : min(ti.m_num_want, 50);
	if (ti.m_compact)
	{
		string peers;
		for (t_candidates::const_iterator i = candidates.begin(); c-- && i != candidates.end(); i++)
			peers += string(reinterpret_cast<const char*>(&(*i)->first), 4)
				+ string(reinterpret_cast<const char*>(&(*i)->second.port), 2);
		return peers;
	}
	Cbvalue peers(Cbvalue::vt_list);
	for (t_candidates::const_iterator i = candidates.begin(); c-- && i != candidates.end(); i++)
	{
		Cbvalue peer;
		if (!ti.m_no_peer_id)
			peer.d(bts_peer_id, (*i)->second.peer_id);
		in_addr a;
		a.s_addr = (*i)->first;
		peer.d(bts_ipa, static_cast<string>(inet_ntoa(a)));
		peer.d(bts_port, ntohs((*i)->second.port));
		peers.l(peer);
	}	
	return peers;
}

Cbvalue Cserver::select_peers(const Ctracker_input& ti)
{
	t_files::const_iterator i = m_files.find(ti.m_info_hash);
	if (i == m_files.end())
		return Cbvalue();
	Cbvalue v;
	v.d(bts_interval, m_announce_interval);
	v.d(bts_peers, i->second.select_peers(ti));	
	return v;
}

void Cserver::t_file::clean_up(int t)
{
	for (t_peers::iterator i = peers.begin(); i != peers.end(); )
	{
		if (i->second.mtime < t)
		{
			(i->second.left ? leechers : seeders)--;
			peers.erase(i++);
			dirty = true;
		}
		else
			i++;
	}
}

void Cserver::clean_up()
{
	for (t_files::iterator i = m_files.begin(); i != m_files.end(); i++)
		i->second.clean_up(time(NULL) - static_cast<int>(1.5 * m_announce_interval));
	m_clean_up_time = time(NULL);
}

Cbvalue Cserver::t_file::scrape() const
{
	Cbvalue v;
	v.d(bts_complete, seeders);
	v.d(bts_downloaded, completed);
	v.d(bts_incomplete, leechers);
	return v;
}

Cbvalue Cserver::scrape(const Ctracker_input& ti)
{
	if (m_log)
	{
		Csql_query q(m_database);
		q.write("(%d, %s, %d)");
		q.p(ntohl(ti.m_ipa));
		if (ti.m_info_hash.empty())
			q.p("NULL");
		else
			q.pe(ti.m_info_hash);
		q.p(time(NULL));
		if (!m_scrape_log_buffer.empty())
			m_scrape_log_buffer += ", ";
		m_scrape_log_buffer += q.read();
	}
	Cbvalue v;
	Cbvalue files(Cbvalue::vt_dictionary);
	if (ti.m_info_hash.empty())
	{
		for (t_files::const_iterator i = m_files.begin(); i != m_files.end(); i++)
		{
			if (i->second.leechers > 1 || i->second.seeders)
				files.d(i->first, i->second.scrape());
		}
	}
	else
	{
		t_files::iterator i = m_files.find(ti.m_info_hash);
		if (i != m_files.end())
		{
			i->second.scraped++;
			i->second.dirty = true;
			files.d(i->first, i->second.scrape());
		}
	}
	v.d(bts_files, files);
	return v;
}

void Cserver::read_db()
{
	try
	{
		Csql_query q(m_database);
		q.write("select info_hash, completed, fid, started, stopped, announced, scraped from xbt_files where fid >= %s");
		q.p(m_fid_end);
		Csql_result result = q.execute();
		Csql_row row;
		while (row = result.fetch_row())
		{
			if (row.size(0) != 20)
				continue;
			t_file& file = m_files[string(row.f(0), 20)];
			file.completed = row.f_int(1, 0);
			file.dirty = true;
			file.fid = row.f_int(2, 0);
			file.started = row.f_int(3, 0);
			file.stopped = row.f_int(4, 0);
			file.announced = row.f_int(5, 0);
			file.scraped = row.f_int(6, 0);
			m_fid_end = max(m_fid_end, file.fid + 1);
		}
	}
	catch (Cxcc_error error)
	{
	}
	m_read_db_time = time(NULL);
}

void Cserver::write_db()
{
	try
	{
		for (t_files::iterator i = m_files.begin(); i != m_files.end(); i++)
		{
			t_file& file = i->second;
			if (!file.dirty)
				continue;
			Csql_query q(m_database);
			if (!file.fid)
			{
				q.write("insert into xbt_files (info_hash, ctime) values (%s, NULL)");
				q.pe(i->first);
				q.execute();
				file.fid = m_database.insert_id();
			}
			q.write("update xbt_files set leechers = %s, seeders = %s, completed = %s, started = %s, stopped = %s, announced = %s, scraped = %s where fid = %s");
			q.p(file.leechers);
			q.p(file.seeders);
			q.p(file.completed);
			q.p(file.started);
			q.p(file.stopped);
			q.p(file.announced);
			q.p(file.scraped);
			q.p(file.fid);
			q.execute();
			file.dirty = false;
		}
		if (!m_announce_log_buffer.empty())
		{
			m_database.query("insert delayed into xbt_announce_log (ipa, port, event, info_hash, peer_id, downloaded, left0, uploaded, mtime) values " + m_announce_log_buffer);
			m_announce_log_buffer.erase();
		}
		if (!m_scrape_log_buffer.empty())
		{
			m_database.query("insert delayed into xbt_scrape_log (ipa, info_hash, mtime) values " + m_scrape_log_buffer);
			m_scrape_log_buffer.erase();
		}
	}
	catch (Cxcc_error error)
	{
	}
	m_write_db_time = time(NULL);
}

void Cserver::read_config()
{
	m_announce_interval = 1800;
	m_auto_register = true;
	m_clean_up_interval = 60;
	m_daemon = true;
	m_gzip_announce = true;
	m_gzip_debug = true;
	m_gzip_scrape = true;
	m_listen_check = true;
	m_listen_ports.clear();
	m_log = false;
	m_read_config_interval = 300;
	m_read_db_interval = 60;
	m_write_db_interval = 60;
	try
	{
		Csql_result result = m_database.query("select name, value from xbt_config where value is not null");
		Csql_row row;
		while (row = result.fetch_row())
		{
			if (!strcmp(row.f(0), "announce_interval"))
				m_announce_interval = row.f_int(1);
			else if (!strcmp(row.f(0), "auto_register"))
				m_auto_register = row.f_int(1);
			else if (!strcmp(row.f(0), "daemon"))
				m_daemon = row.f_int(1);
			else if (!strcmp(row.f(0), "gzip_announce"))
				m_gzip_announce = row.f_int(1);
			else if (!strcmp(row.f(0), "gzip_debug"))
				m_gzip_debug = row.f_int(1);
			else if (!strcmp(row.f(0), "gzip_scrape"))
				m_gzip_scrape = row.f_int(1);
			else if (!strcmp(row.f(0), "listen_check"))
				m_listen_check = row.f_int(1);
			else if (!strcmp(row.f(0), "listen_port"))
				m_listen_ports.insert(row.f_int(1));
			else if (!strcmp(row.f(0), "log"))
				m_log = row.f_int(1);
			else if (!strcmp(row.f(0), "read_config_interval"))
				m_read_config_interval = row.f_int(1);
			else if (!strcmp(row.f(0), "read_db_interval"))
				m_read_db_interval = row.f_int(1);
			else if (!strcmp(row.f(0), "write_db_interval"))
				m_write_db_interval = row.f_int(1);
		}
	}
	catch (Cxcc_error error)
	{
	}
	if (m_listen_ports.empty())
		m_listen_ports.insert(2710);
	m_read_config_time = time(NULL);
}

string Cserver::t_file::debug() const
{
	string page;
	for (t_peers::const_iterator i = peers.begin(); i != peers.end(); i++)
	{
		in_addr a;
		a.s_addr = i->first;
		page += "<tr><td>" + static_cast<string>(inet_ntoa(a))
			+ "<td align=right>" + n(ntohs(i->second.port))
			+ "<td>" + (i->second.listening ? '*' : ' ')
			+ "<td align=right>" + n(i->second.left)
			+ "<td align=right>" + n(time(NULL) - i->second.mtime)
			+ "<td>" + hex_encode(i->second.peer_id);
	}
	return page;
}

string Cserver::debug(const Ctracker_input& ti) const
{
	string page;
	page += "<meta http-equiv=refresh content=60><title>XBT Tracker</title><table>";
	int leechers = 0;
	int seeders = 0;
	int torrents = 0;
	if (ti.m_info_hash.empty())
	{
		for (t_files::const_iterator i = m_files.begin(); i != m_files.end(); i++)
		{
			if (!i->second.leechers && !i->second.seeders)
				continue;
			leechers += i->second.leechers;
			seeders += i->second.seeders;
			torrents++;
			page += "<tr><td align=right>" + n(i->second.fid) 
				+ "<td><a href=\"?info_hash=" + uri_encode(i->first) + "\">" + hex_encode(i->first) + "</a>"
				+ "<td>" + (i->second.dirty ? '*' : ' ')
				+ "<td align=right>" + n(i->second.leechers) 
				+ "<td align=right>" + n(i->second.seeders) 
				+ "<td align=right>" + n(i->second.announced) 
				+ "<td align=right>" + n(i->second.scraped) 
				+ "<td align=right>" + n(i->second.completed) 
				+ "<td align=right>" + n(i->second.started) 
				+ "<td align=right>" + n(i->second.stopped);
		}
	}
	else
	{
		t_files::const_iterator i = m_files.find(ti.m_info_hash);
		if (i != m_files.end())
			page += i->second.debug();
	}
	page += "</table><hr><table><tr><td>leechers<td>" + n(leechers)
		+ "<tr><td>seeders<td>" + n(seeders)
		+ "<tr><td>torrents<td>" + n(torrents)
		+ "<tr>"
		+ "<tr><td>read config time<td>" + n(m_read_config_time) 
		+ "<tr><td>clean up time<td>" + n(m_clean_up_time) 
		+ "<tr><td>listen check<td>" + n(m_listen_check) 
		+ "<tr><td>read db time<td>" + n(m_read_db_time) 
		+ "<tr><td>write db time<td>" + n(m_write_db_time) 
		+ "<tr><td>time<td>" + n(time(NULL)) + "</table>";
	return page;
}

