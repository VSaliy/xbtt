// bt_tracker_link.cpp: implementation of the Cbt_tracker_link class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bt_tracker_link.h"

#include "bt_file.h"
#include "bt_misc.h"
#include "bt_strings.h"
#include "server.h"
#include "xcc_z.h"

template <class T>
static T read(const char* r, const char* r_end)
{
	T v = 0;
	for (int i = 0; i < sizeof(T); i++)
		v = v << 8 | *reinterpret_cast<const unsigned char*>(r++);
	return v;
}

template <class T>
static char* write(char* w, T v)
{
	w += sizeof(T);
	for (int i = 0; i < sizeof(T); i++)
	{
		*--w = v & 0xff;
		v >>= 8;
	}
	return w + sizeof(T);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Cbt_tracker_link::Cbt_tracker_link()
{
	m_announce_time = 0;
	mc_attempts = 0;
	m_current_tracker = 0;
	m_event = e_started;
	m_state = 0;
}

Cbt_tracker_link::~Cbt_tracker_link()
{
}

int Cbt_tracker_link::pre_select(Cbt_file& f, fd_set* fd_read_set, fd_set* fd_write_set, fd_set* fd_except_set)
{
	switch (m_state)
	{
	case 0:
		if (!f.m_run 
			|| m_announce_time > time(NULL) 
			|| m_current_tracker < 0 
			|| m_current_tracker >= f.m_trackers.size() 
			|| !f.m_server->below_peer_limit())
			return 0;
		m_url = f.m_trackers[m_current_tracker];
		if (!m_url.valid())
		{
			mc_attempts++;
			close(f);
			return 0;
		}
		switch (m_url.m_protocol)
		{
		case Cbt_tracker_url::tp_http:
			f.alert(Calert(Calert::info, "Tracker: URL: http://" + m_url.m_host + ':' + n(m_url.m_port) + m_url.m_path + "?info_hash=" + uri_encode(f.m_info_hash)));
			m_announce_time = time(NULL) + (300 << mc_attempts++);
			if (m_s.open(SOCK_STREAM) == INVALID_SOCKET)
				return 0;
			break;
		case Cbt_tracker_url::tp_udp:
			f.alert(Calert(Calert::info, "Tracker: URL: udp://" + m_url.m_host + ':' + n(m_url.m_port) + "?info_hash=" + uri_encode(f.m_info_hash)));
			m_announce_time = time(NULL) + (60 << mc_attempts++);
			if (m_s.open(SOCK_DGRAM) == INVALID_SOCKET)
				return 0;
			break;
		default:
			return 0;
		}
		{
			int h = Csocket::get_host(m_url.m_host);
			if (h == INADDR_NONE)
			{
				f.alert(Calert(Calert::error, "Tracker: gethostbyname failed"));
				close(f);
				return 0;
			}
			if (m_s.connect(h, htons(m_url.m_port)) && WSAGetLastError() != WSAEWOULDBLOCK)
				return 0;
			in_addr a;
			a.s_addr = h;
			f.alert(Calert(Calert::info, "Tracker: IPA: " + static_cast<string>(inet_ntoa(a))));
		}		
		if (m_url.m_protocol == Cbt_tracker_url::tp_udp)
		{
			char d[utic_size];
#ifdef WIN32
			write<__int64>(d + uti_connection_id, 0x41727101980);
#else
			write<__int64>(d + uti_connection_id, 0x41727101980ll);
#endif
			write<__int32>(d + uti_action, uta_connect);
			write<__int32>(d + uti_transaction_id, m_transaction_id = rand());
			if (m_s.send(d, utic_size) != utic_size)
			{
				close(f);
				return 0;
			}
			f.alert(Calert(Calert::debug, "Tracker: UDP: connect send"));
			m_connect_send = time(NULL);
			m_state = 3;
		}
		else
		{
			m_w = m_d.write_start(16 << 10);
			m_state = 1;
		}
	case 1:
		FD_SET(m_s, fd_write_set);
		FD_SET(m_s, fd_except_set);
	case 2:
	case 3:
	case 4:
		FD_SET(m_s, fd_read_set);
		return m_s;
	}
	return 0;
}

void Cbt_tracker_link::post_select(Cbt_file& f, fd_set* fd_read_set, fd_set* fd_write_set, fd_set* fd_except_set)
{
	switch (m_state)
	{
	case 1:
		if (FD_ISSET(m_s, fd_write_set))
		{
			stringstream os;
			os << "GET " << m_url.m_path 
				<< "?info_hash=" << uri_encode(f.m_info_hash) 
				<< "&peer_id=" << uri_encode(f.m_peer_id) 
				<< "&port=" << f.local_port()
				<< "&downloaded=" << n(f.m_downloaded)
				<< "&left=" << n(f.m_left)
				<< "&uploaded=" << n(f.m_uploaded)
				<< "&compact=1"
				<< "&no_peer_id=1";
			if (f.local_ipa())
			{
				in_addr a;
				a.s_addr = f.local_ipa();
				os << "&ip=" << inet_ntoa(a);
			}
			switch (m_event)
			{
			case e_completed:
				os << "&event=completed";
				break;
			case e_started:
				os << "&event=started";
				break;
			case e_stopped:
				os << "&event=stopped";
				break;
			}
			m_event = e_none;			
			os << " HTTP/1.0\r" << endl
				<< "accept-encoding: gzip\r" << endl
				<< "host: " << m_url.m_host;
			if (m_url.m_port != 80)
				os << ':' << m_url.m_port;
			os << '\r' << endl
				<< '\r' << endl;
			if (m_s.send(os.str().c_str(), os.str().size()) != os.str().size())
				close(f);
			else
				m_state = 2;
		}
		else if (FD_ISSET(m_s, fd_except_set))
		{
			int e = 0;
			socklen_t size = sizeof(int);
			getsockopt(m_s, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&e), &size);
			f.alert(Calert(Calert::error, "Tracker: HTTP: connect failed: " + Csocket::error2a(e)));
			close(f);
		}
		break;
	case 2:
		if (FD_ISSET(m_s, fd_read_set))
		{
			for (int r; r = m_s.recv(m_w, m_d.data_end() - m_w); )
			{
				if (r == SOCKET_ERROR)
				{
					int e = WSAGetLastError();
					if (e != WSAEWOULDBLOCK)
					{
						f.alert(Calert(Calert::warn, "Tracker: HTTP: recv failed: " + Csocket::error2a(e)));
						close(f);
					}
					return;
				}
				m_w += r;
			}
			m_d.size(m_w - m_d);
			read(f, m_d);
			close(f);
		}
		break;
	case 3:
		if (FD_ISSET(m_s, fd_read_set))
		{
			const int cb_d = 2 << 10;
			char d[cb_d];
			int r = m_s.recv(d, cb_d);
			if (r != SOCKET_ERROR 
				&& r >= utoc_size
				&& ::read<__int32>(d + uto_transaction_id, d + r) == m_transaction_id
				&& ::read<__int32>(d + uto_action, d + r) == uta_connect)
			{
				m_connection_id = ::read<__int64>(d + utoc_connection_id, d + r);
				const int cb_b = 2 << 10;
				char b[cb_b];
				write<__int64>(b + uti_connection_id, m_connection_id);
				write<__int32>(b + uti_action, uta_announce);
				write<__int32>(b + uti_transaction_id, m_transaction_id = rand());
				memcpy(b + utia_info_hash, f.m_info_hash.c_str(), 20);
				memcpy(b + utia_peer_id, f.m_peer_id.c_str(), 20);
				write<__int64>(b + utia_downloaded, f.m_downloaded);
				write<__int64>(b + utia_left, f.m_left);
				write<__int64>(b + utia_uploaded, f.m_uploaded);
				write<__int32>(b + utia_event, m_event);
				m_event = e_none;
				write<__int32>(b + utia_ipa, ntohl(f.local_ipa()));
				write<__int32>(b + utia_num_want, -1);
				write<__int16>(b + utia_port, f.local_port());
				char* w = b + utia_size;
				const Cbt_tracker_account* account = f.m_server->tracker_accounts().find(m_url.m_host);
				if (account)
				{
					memset(w, 0, 8);
					memcpy(w, account->user().c_str(), min(account->user().size(), 8));
					w += 8;
					Csha1(account->pass().c_str(), account->pass().size()).read(w);
					Csha1(b, w + 20 - b).read(w);
					w += 8;
				}
				if (m_s.send(b, w - b) != w - b)
				{
					close(f);
					return;
				}
				f.alert(Calert(Calert::debug, "Tracker: UDP: announce send"));
				m_announce_send = time(NULL);
				m_state = 4;
			}				
		}
		else if (time(NULL) - m_connect_send > 15)
			close(f);
		break;
	case 4:
		if (FD_ISSET(m_s, fd_read_set))
		{
			const int cb_d = 2 << 10;
			char d[cb_d];
			int r = m_s.recv(d, cb_d);
			if (r != SOCKET_ERROR 
				&& r >= uto_size 
				&& ::read<__int32>(d + uto_transaction_id, d + r) == m_transaction_id)
			{
				if (r >= utoa_size
					&& ::read<__int32>(d + uto_action, d + r) == uta_announce)
				{
					m_announce_time = time(NULL) + max(300, ::read<__int32>(d + utoa_interval, d + r));
					f.mc_leechers_total = ::read<__int32>(d + utoa_leechers, d + r);
					f.mc_seeders_total = ::read<__int32>(d + utoa_seeders, d + r);
					mc_attempts = 0;
					f.alert(Calert(Calert::info, "Tracker: " + n((r - utoa_size) / 6) + " peers (" + n(r) + " bytes)"));
					for (int o = utoa_size; o + 6 <= r; o += 6)
						f.insert_peer(*reinterpret_cast<const __int32*>(d + o), *reinterpret_cast<__int16*>(d + o + 4));
					close(f);
				}
				else if (r >= utoe_size 
					&& ::read<__int32>(d + uto_action, d + r) == uta_error)
				{
					f.alert(Calert(Calert::error, "Tracker: failure reason: " + string(d + utoe_size, r - utoe_size)));
					close(f);
				}
			}
		}
		else if (time(NULL) - m_announce_send > 15)
			close(f);
		break;
	}
}

int Cbt_tracker_link::read(Cbt_file& f, const Cvirtual_binary& d)
{
	for (const byte* r = d; r < d.data_end(); r++)
	{
		if (*r == ' ')
		{
			int http_result = atoi(string(reinterpret_cast<const char*>(r), d.data_end() - r).c_str());
			if (http_result != 200)
			{
				f.alert(Calert(Calert::error, "Tracker: HTTP error: " + n(http_result)));
				return 1;
			}
			for (const byte* r = d; r + 4 <= d.data_end(); r++)
			{
				if (!memcmp(r, "\n\n", 2) || !memcmp(r, "\r\n\r\n", 4))
				{
					r += *r == '\n' ? 2 : 4;
					Cbvalue v;
					if (v.write(r, d.data_end() - r))
					{
						f.alert(Calert(Calert::error, "Tracker: bdecode failed"));
						return 1;
					}
					if (v.d(bts_failure_reason).s().empty())
					{
						m_announce_time = time(NULL) + max(300, v.d(bts_interval).i());
						f.mc_leechers_total = v.d(bts_incomplete).i();
						f.mc_seeders_total = v.d(bts_complete).i();
						mc_attempts = 0;
						if (v.d(bts_peers).s().empty())
						{
							const Cbvalue::t_list& peers = v.d(bts_peers).l();
							f.alert(Calert(Calert::info, "Tracker: " + n(peers.size()) + " peers (" + n(d.size()) + " bytes)"));
							for (Cbvalue::t_list::const_iterator i = peers.begin(); i != peers.end(); i++)
								f.insert_peer(inet_addr(i->d(bts_ipa).s().c_str()), htons(i->d(bts_port).i()));
						}
						else
						{
							string peers = v.d(bts_peers).s();
							f.alert(Calert(Calert::info, "Tracker: " + n(peers.size() / 6) + " peers (" + n(d.size()) + " bytes)"));
							for (const char* r = peers.c_str(); r + 6 <= peers.c_str() + peers.length(); r += 6)
								f.insert_peer(*reinterpret_cast<const __int32*>(r), *reinterpret_cast<const __int16*>(r + 4));
						}
						return 0;
					}
					f.alert(Calert(Calert::error, "Tracker: failure reason: " + v.d(bts_failure_reason).s()));
					return 1;
				}
			}
			break;
		}
	}
	f.alert(Calert(Calert::error, "Tracker: Invalid HTTP output"));
	return 1;
}

void Cbt_tracker_link::close(Cbt_file& f)
{
	m_s.close();
	m_state = 0;
	if (!mc_attempts)
	{
		swap(f.m_trackers[0], f.m_trackers[m_current_tracker]);
		m_current_tracker = 0;
	}
	else if (++m_current_tracker < f.m_trackers.size())
	{
		m_announce_time = 0;
		mc_attempts--;
	}
	else
		m_current_tracker = 0;
}

ostream& Cbt_tracker_link::dump(ostream& os) const
{
	return os;
}

ostream& operator<<(ostream& os, const Cbt_tracker_link& v)
{
	return v.dump(os);
}

int Cbt_tracker_link::pre_dump() const
{
	int size = 0;
	return size;
}

void Cbt_tracker_link::dump(Cstream_writer& w) const
{
}

void Cbt_tracker_link::event(int v)
{
	m_event = v;
	m_announce_time = 0;
}
