// bt_file.cpp: implementation of the Cbt_file class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bt_file.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <io.h>
#include "bt_strings.h"
#include "server.h"

#define for if (0) {} else for

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Cbt_file::Cbt_file()
{
	m_downloaded = 0;
	m_left = -1;
	m_uploaded = 0;
	m_total_downloaded = 0;
	m_total_uploaded = 0;
	mc_leechers_total = 0;
	mc_seeders_total = 0;
	m_run = true;
}

Cbt_file::~Cbt_file()
{
}

int Cbt_file::torrent(const Cbvalue& v)
{
	const Cbvalue::t_list& trackers = v.d(bts_announce_list).l();
	for (Cbvalue::t_list::const_iterator i = trackers.begin(); i != trackers.end(); i++)
	{
		for (Cbvalue::t_list::const_iterator j = i->l().begin(); j != i->l().end(); j++)
			m_trackers.push_back(j->s());		
	}
	if (m_trackers.empty())
		m_trackers.push_back(v.d(bts_announce).s());
	return info(v.d(bts_info));
}

int Cbt_file::info(const Cbvalue& info)
{
	m_name = info.d(bts_name).s();
	if (m_name.empty())
		return 0;
	m_info = info.read();
	m_info_hash = compute_sha1(m_info);
	mcb_piece = info.d(bts_piece_length).i();
	{
		mcb_f = 0;
		const Cbvalue::t_list& files = info.d(bts_files).l();
		for (Cbvalue::t_list::const_iterator i = files.begin(); i != files.end(); i++)
		{
			string name;
			{
				const Cbvalue::t_list& path = i->d(bts_path).l();
				for (Cbvalue::t_list::const_iterator i = path.begin(); i != path.end(); i++)
					name += '/' + i->s();
			}
			__int64 size = i->d(bts_length).i();
			if (name.empty() || size < 1)
				return 1;
			mcb_f += size;
			m_sub_files.push_back(t_sub_file(name, 0, size));
		}
	}
	if (m_sub_files.empty())
		m_sub_files.push_back(t_sub_file("", 0, mcb_f = info.d(bts_length).i()));
	if (mcb_f < 1 
		|| mcb_piece < 16 << 10
		|| mcb_piece > 16 << 20)
		return 1;
	{
		byte* w = m_info_hashes.write_start((m_info.size() + 4095) / 4096 * 20 + 12);
		*reinterpret_cast<__int32*>(w) = htonl(mcb_piece);
		w += 4;
		*reinterpret_cast<__int64*>(w) = htonll(mcb_f);
		w += 8;
		const byte* r = m_info;
		const byte* r_end = m_info.data_end();
		while (r < r_end)
		{
			Csha1(r, min(r_end - r, 4096)).read(w);
			r += 4096;
			w += 20;
		}
		m_info_hashes_hash = compute_sha1(m_info_hashes);
	}
	m_pieces.resize((mcb_f + mcb_piece - 1) / mcb_piece);
	string piece_hashes = info.d(bts_pieces).s();
	if (piece_hashes.length() != 20 * m_pieces.size())
		return 1;
	for (int i = 0; i < m_pieces.size(); i++)
	{
		m_pieces[i].mcb_d = min(mcb_f - mcb_piece * i, mcb_piece);
		memcpy(m_pieces[i].m_hash, piece_hashes.c_str() + 20 * i, 20);
	}
	return 0;
}

void Cbt_file::t_sub_file::close()
{
	if (m_f != -1)
		_close(m_f);
	m_f = -1;
}

void Cbt_file::t_sub_file::dump(Cstream_writer& w) const
{
	w.write_string(name());
	w.write_int32(priority());
	w.write_int64(size());
}

bool Cbt_file::t_sub_file::open(const string& parent_name, int oflag)
{
	m_f = _open((parent_name + m_name).c_str(), oflag | _O_BINARY, _S_IREAD | _S_IWRITE);
	return *this;
}

int Cbt_file::t_sub_file::pre_dump() const
{
	return name().size() + 16;
}

int Cbt_file::t_sub_file::read(__int64 offset, void* s, int cb_s)
{
	return !*this
		|| _lseeki64(m_f, offset, SEEK_SET) != offset
		|| _read(m_f, s, cb_s) != cb_s;
}

int Cbt_file::t_sub_file::write(__int64  offset, const void* s, int cb_s)
{
	return !*this
		|| _lseeki64(m_f, offset, SEEK_SET) != offset
		|| _write(m_f, s, cb_s) != cb_s;
}

int Cbt_file::open(const string& name, bool validate)
{
	m_name = name;
	__int64 offset = 0;
	for (t_sub_files::iterator i = m_sub_files.begin(); i != m_sub_files.end(); i++)
	{
		if (!i->open(m_name, _O_RDWR)
			&& !i->open(m_name, _O_RDONLY))
		{
			int b = (offset + i->size() - 1) / mcb_piece;
			for (int a = offset / mcb_piece; a <= b; a++)
				m_pieces[a].m_valid = false;
		}
		offset += i->size();
	}
	{
		Cvirtual_binary d;
		m_left = 0;
		for (int i = 0; i < m_pieces.size(); i++)
		{
			Cbt_piece& piece = m_pieces[i];
			if (validate)
			{
				piece.m_valid = !read_data(mcb_piece * i, d.write_start(piece.mcb_d), piece.mcb_d)
					&& !memcmp(compute_sha1(d).c_str(), piece.m_hash, 20);
			}
			if (!piece.m_valid)
				m_left += piece.mcb_d;
		}
		alert(Calert(Calert::debug, "Torrent: " + n(c_valid_pieces()) + '/' + n(m_pieces.size())));
		alert(Calert(Calert::debug, "Torrent: " + n(mcb_piece >> 10) + " kb/piece"));
	}
	if (!m_left)
	{
		for (t_sub_files::iterator i = m_sub_files.begin(); i != m_sub_files.end(); i++)
		{
			i->close();
			i->open(m_name, _O_RDONLY);
		}
	}
	return 0;
}

void Cbt_file::close()
{
	for (t_sub_files::iterator i = m_sub_files.begin(); i != m_sub_files.end(); i++)
		i->close();
}

int Cbt_file::pre_select(fd_set* fd_read_set, fd_set* fd_write_set, fd_set* fd_except_set)
{
	if (m_run && !m_left && m_server->seeding_ratio() && m_total_uploaded / m_server->seeding_ratio() > mcb_f)
		m_run = false;
	else if (m_run)
	{
		for (t_new_peers::const_iterator i = m_new_peers.begin(); i != m_new_peers.end(); i++)
		{
			if (find_peer(i->first))
				continue;
			Cbt_peer_link peer;
			peer.m_a.sin_family = AF_INET;
			peer.m_a.sin_addr.s_addr = i->first;
			peer.m_a.sin_port = i->second;
			peer.m_f = this;
			peer.m_local_link = true;
			m_peers.push_back(peer);
		}
		m_new_peers.clear();
	}
	int n = m_tracker.pre_select(*this, fd_read_set, fd_write_set, fd_except_set);
	for (t_peers::iterator i = m_peers.begin(); i != m_peers.end(); i++)
	{
		int z = i->pre_select(fd_read_set, fd_write_set, fd_except_set);
		n = max(n, z);
	}
	return n;
}

void Cbt_file::post_select(fd_set* fd_read_set, fd_set* fd_write_set, fd_set* fd_except_set)
{
	m_tracker.post_select(*this, fd_read_set, fd_write_set, fd_except_set);
	for (t_peers::iterator i = m_peers.begin(); i != m_peers.end(); )
	{
		if (i->post_select(fd_read_set, fd_write_set, fd_except_set) || i->m_state == -1)
		{
			i->close();
			i = m_peers.erase(i);
		}
		else
			i++;
	}
}

void Cbt_file::insert_old_peer(int h, int p)
{
	m_old_peers[h] = p;
}

void Cbt_file::insert_peer(int h, int p)
{
	m_new_peers[h] = p;
}

void Cbt_file::insert_peer(const char* r, const sockaddr_in& a, const Csocket& s)
{
	for (t_peers::const_iterator i = m_peers.begin(); i != m_peers.end(); i++)
	{
		if (i->m_a.sin_addr.s_addr == a.sin_addr.s_addr)
			return;
	}
	Cbt_peer_link peer;
	peer.m_a = a;
	peer.m_f = this;
	peer.m_s = s;
	peer.m_local_link = false;
	peer.m_state = 4;
	if (peer.read_handshake(r))
	{
		assert(false);
		return;
	}
	peer.write_handshake();
	m_peers.push_back(peer);
}

int Cbt_file::c_pieces() const
{
	return m_pieces.size();
}

int Cbt_file::c_invalid_pieces() const
{
	int r = 0;
	for (t_pieces::const_iterator i = m_pieces.begin(); i != m_pieces.end(); i++)
		r += !i->m_valid;
	return r;
}

int Cbt_file::c_valid_pieces() const
{
	int r = 0;
	for (t_pieces::const_iterator i = m_pieces.begin(); i != m_pieces.end(); i++)
		r += i->m_valid;
	return r;
}

void Cbt_file::write_data(__int64 offset, const char* s, int cb_s)
{
	if (offset < 0 || cb_s < 0 || offset + cb_s > mcb_f)
		return;
	int a = offset / mcb_piece;
	if (a < m_pieces.size())
	{
		Cbt_piece& piece = m_pieces[a];
		if (piece.m_valid)
		{
			alert(Calert(Calert::debug, "Piece " + n(a) + ": already valid"));
			return;
		}
		piece.write(offset % mcb_piece, s, cb_s);
		int size = cb_s;
		const char* r = s;
		for (t_sub_files::iterator i = m_sub_files.begin(); i != m_sub_files.end(); i++)
		{
			if (offset < i->size())
			{
				if (!*i && i->size())
				{
					string path = m_name + i->name();
					for (int i = 3; i < path.size(); )
					{
						int a = path.find_first_of("/\\", i);
						if (a == string::npos)
							break;
						CreateDirectory(path.substr(0, a).c_str(), NULL);
						i = a + 1;
					}
					if (i->open(m_name, _O_CREAT | _O_RDWR))
					{
						char b = 0;
						i->write(i->size() - 1, &b, 1);
					}
				}
				int cb_write = min(size, i->size() - offset);
				if (i->write(offset, r, cb_write))
				{
					alert(Calert(Calert::error, "Piece " + n(a) + ": write failed"));
					m_run = false;
				}
				size -= cb_write;
				if (!size)
					break;
				offset = 0;
				r += cb_write;
			}
			else
				offset -= i->size();
		}
		if (piece.mc_sub_pieces_left)
			return;
		Cvirtual_binary d;
		read_data(a * mcb_piece, d.write_start(piece.mcb_d), piece.mcb_d);
		if (memcmp(compute_sha1(d).c_str(), piece.m_hash, 20))
		{
			alert(Calert(Calert::warn, "Piece " + n(a) + ": invalid"));
			return;
		}
		piece.m_valid = true;
		m_left -= piece.mcb_d;
		if (!m_left)
		{
			m_tracker.event(Cbt_tracker_link::t_event::e_completed);
			for (t_sub_files::iterator i = m_sub_files.begin(); i != m_sub_files.end(); i++)
			{
				i->close();
				i->open(m_name, _O_RDONLY);
			}
		}
		{
			for (t_peers::iterator i = m_peers.begin(); i != m_peers.end(); i++)
				i->write_have(a);
		}
		alert(Calert(Calert::debug, "Piece " + n(a) + ": valid"));
	}
}

int Cbt_file::read_data(__int64 offset, byte* d, int cb_d)
{
	int size = cb_d;
	byte* w = d;
	for (t_sub_files::iterator i = m_sub_files.begin(); i != m_sub_files.end(); i++)
	{
		if (offset < i->size())
		{
			int cb_read = min(size, i->size() - offset);
			if (i->read(offset, w, cb_read))
				return 1;
			size -= cb_read;
			if (!size)
				return 0;
			offset = 0;
			w += cb_read;
		}
		else
			offset -= i->size();
	}
	return 1;
}

int Cbt_file::next_invalid_piece(const Cbt_peer_link& peer)
{
	if (!m_left)
		return -1;

	vector<int> invalid_pieces;

	invalid_pieces.reserve(c_invalid_pieces());
	bool begin_mode = c_valid_pieces() < 4;
	bool end_mode = c_invalid_pieces() < 16;
	int rank = INT_MAX;
	for (int i = 0; i < m_pieces.size(); i++)
	{
		if (m_pieces[i].m_valid 
			|| !peer.m_remote_pieces[i] 
			|| peer.m_pieces.find(&m_pieces[i]) != peer.m_pieces.end()
			|| !end_mode && !m_pieces[i].m_peers.empty())
			continue;
		if (begin_mode && m_pieces[i].m_peers.empty() && !m_pieces[i].m_sub_pieces.empty())
			return i;
		int piece_rank = 2560000 * min(m_pieces[i].m_peers.size(), 9)
			+ -256000 * m_pieces[i].m_priority 
			+ 256 * min(m_pieces[i].mc_peers, 999)
			+ m_pieces[i].m_sub_pieces.empty();
		if (piece_rank > rank)
			continue;
		if (piece_rank < rank)
			invalid_pieces.clear();
		rank = piece_rank;
		invalid_pieces.push_back(i);
	}
	return invalid_pieces.empty() ? -1 : invalid_pieces[rand() % invalid_pieces.size()];
}

ostream& Cbt_file::dump(ostream& os) const
{
	os << "<table>"
		<< "<tr><td align=right>invalid pieces:<td align=right>" << c_invalid_pieces() << "<td align=right>" << b2a(mcb_piece * c_invalid_pieces(), "b")
		<< "<tr><td align=right>valid pieces:<td align=right>" << c_valid_pieces() << "<td align=right>" << b2a(mcb_piece * c_valid_pieces(), "b")
		<< "<tr><td align=right>downloaded:<td align=right>" << b2a(m_downloaded, "b") << "<td align=right>" << b2a(m_down_counter.rate(), "b/s") << "<td align=right>";
	int t = time_remaining();
	if (t != -1)
	{
		if (t / 3600)
			os << t / 3600 << " h ";
		os << (t % 3600) / 60 << " m " << t % 60 << " s";
	}
	os << "<tr><td align=right>uploaded:<td align=right>" << b2a(m_uploaded, "b") << "<td align=right>" << b2a(m_up_counter.rate(), "b/s")
		<< "</table>"
		<< "<hr>"
		<< "<table><tr><th><th><th>D<th>U<th><th colspan=2>L<th colspan=2>R";
	for (t_peers::const_iterator i = m_peers.begin(); i != m_peers.end(); i++)
		os << *i;
	os << "</table>";
	if (0)
	{
		os << "<hr><table>";
		for (int i = 0; i < m_pieces.size(); i++)
		{
			if (m_pieces[i].mc_peers)
				os << "<tr><td align=right>" << i << "<td align=right>" << m_pieces[i].mc_peers;
		}
		os << "</table>";
	}
	return os;
}

ostream& operator<<(ostream& os, const Cbt_file& v)
{
	return v.dump(os);
}

int Cbt_file::pre_dump(int flags) const
{
	int size = m_info_hash.length() + m_name.length() + 100;
	if (flags & Cserver::df_trackers)
	{
		for (t_trackers::const_iterator i = m_trackers.begin(); i != m_trackers.end(); i++)
			size += i->size() + 4;
	}
	if (flags & Cserver::df_peers)
	{
		for (t_peers::const_iterator i = m_peers.begin(); i != m_peers.end(); i++)
			size += i->pre_dump();
	}
	if (flags & Cserver::df_alerts)
	{
		for (Calerts::const_iterator i = m_alerts.begin(); i != m_alerts.end(); i++)
			size += i->pre_dump();
	}
	if (flags & Cserver::df_files)
	{
		for (t_sub_files::const_iterator i = m_sub_files.begin(); i != m_sub_files.end(); i++)
			size += i->pre_dump();
	}
	return size;
}

void Cbt_file::dump(Cstream_writer& w, int flags) const
{
	w.write_string(m_info_hash);
	w.write_string(m_name);
	if (flags & Cserver::df_trackers)
	{
		w.write_int32(m_trackers.size());
		for (t_trackers::const_iterator i = m_trackers.begin(); i != m_trackers.end(); i++)
			w.write_string(*i);
	}
	else
		w.write_int32(0);
	w.write_int64(m_downloaded);
	w.write_int64(m_left);
	w.write_int64(size());
	w.write_int64(m_uploaded);
	w.write_int64(m_total_downloaded);
	w.write_int64(m_total_uploaded);
	w.write_int32(m_down_counter.rate());
	w.write_int32(m_up_counter.rate());
	w.write_int32(c_leechers());
	w.write_int32(c_seeders());
	w.write_int32(mc_leechers_total);
	w.write_int32(mc_seeders_total);
	w.write_int32(m_run);
	if (flags & Cserver::df_peers)
	{
		w.write_int32(m_peers.size());
		for (t_peers::const_iterator i = m_peers.begin(); i != m_peers.end(); i++)
			i->dump(w);
	}
	else
		w.write_int32(0);	
	if (flags & Cserver::df_alerts)
	{
		w.write_int32(m_alerts.size());
		for (Calerts::const_iterator i = m_alerts.begin(); i != m_alerts.end(); i++)
			i->dump(w);
	}
	else
		w.write_int32(0);
	if (flags & Cserver::df_files)
	{
		w.write_int32(m_sub_files.size());
		for (t_sub_files::const_iterator i = m_sub_files.begin(); i != m_sub_files.end(); i++)
			i->dump(w);
	}
	else
		w.write_int32(0);
}

int Cbt_file::time_remaining() const
{
	int rate = m_down_counter.rate();
	return rate ? mcb_piece * c_invalid_pieces() / rate : -1;
}

int Cbt_file::c_leechers() const
{
	int c = 0;
	for (t_peers::const_iterator i = m_peers.begin(); i != m_peers.end(); i++)
	{
		if (i->m_left && i->m_state == 3)
			c++;
	}
	return c;
}

int Cbt_file::c_seeders() const
{
	int c = 0;
	for (t_peers::const_iterator i = m_peers.begin(); i != m_peers.end(); i++)
	{
		if (!i->m_left && i->m_state == 3)
			c++;
	}
	return c;
}

__int64 Cbt_file::size() const
{
	return mcb_f;
}

void Cbt_file::load_state(Cstream_reader& r)
{
	for (int c_trackers = r.read_int32(); c_trackers--; )
		m_trackers.push_back(r.read_string());
	info(r.read_data());
	m_name = r.read_string();
	m_total_downloaded = r.read_int64();
	m_total_uploaded = r.read_int64();
	{
		Cvirtual_binary pieces = r.read_data();
		for (int i = 0; i < min(pieces.size(), m_pieces.size()); i++)
			m_pieces[i].m_valid = pieces[i];
	}
	for (t_sub_files::iterator i = m_sub_files.begin(); i != m_sub_files.end(); i++)
		i->priority(r.read_int8());
	update_piece_priorities();
	{
		for (int c_peers = r.read_int32(); c_peers--; )
		{
			int h = r.read_int32();
			m_old_peers[h] = m_new_peers[h] = r.read_int32();
		}
	}
}

int Cbt_file::pre_save_state(bool intermediate) const
{
	int c = m_info.size() + m_name.size() + (!intermediate || !m_left) * m_pieces.size() + m_sub_files.size() + 8 * m_old_peers.size() + 36;
	for (t_trackers::const_iterator i = m_trackers.begin(); i != m_trackers.end(); i++)
		c += i->size() + 4;
	return c;
}

void Cbt_file::save_state(Cstream_writer& w, bool intermediate) const
{
	w.write_int32(m_trackers.size());	
	for (t_trackers::const_iterator i = m_trackers.begin(); i != m_trackers.end(); i++)
		w.write_string(*i);
	w.write_data(m_info);
	w.write_string(m_name);
	w.write_int64(m_total_downloaded);
	w.write_int64(m_total_uploaded);
	if (intermediate && m_left)
		w.write_data(Cvirtual_binary());
	else
	{
		w.write_int32(m_pieces.size());
		for (int j = 0; j < m_pieces.size(); j++)
			w.write_int8(m_pieces[j].m_valid);
	}
	for (t_sub_files::const_iterator i = m_sub_files.begin(); i != m_sub_files.end(); i++)
		w.write_int8(i->priority());
	w.write_int32(m_old_peers.size());
	for (t_old_peers::const_iterator i = m_old_peers.begin(); i != m_old_peers.end(); i++)
	{
		w.write_int32(i->first);
		w.write_int32(i->second);
	}
}

void Cbt_file::alert(const Calert& v)
{
	m_alerts.push_back(v);
}

string Cbt_file::get_url() const
{
	string v = "xbtp://";
	for (t_trackers::const_iterator i = m_trackers.begin(); i != m_trackers.end(); i++)
		v += uri_encode(*i) + ',';
	v += '/' + hex_encode(m_info_hash) + '/' + hex_encode(m_info_hashes_hash) + '/';
	if (local_ipa())
	{
		v += hex_encode(8, ntohl(local_ipa()))
			+ hex_encode(4, local_port());
	}
	for (t_peers::const_iterator i = m_peers.begin(); i != m_peers.end(); i++)
	{
		if (i->m_state != 3)
			continue;
		v += hex_encode(8, ntohl(i->m_a.sin_addr.s_addr))
			+ hex_encode(4, ntohs(i->m_a.sin_port));
	}
	return v;
}

Cbt_peer_link* Cbt_file::find_peer(int h)
{
	for (t_peers::iterator i = m_peers.begin(); i != m_peers.end(); i++)
	{
		if (i->m_a.sin_addr.s_addr == h)
			return &*i;
	}
	return NULL;
}

int Cbt_file::local_ipa() const
{
	return m_server->public_ipa();
}

int Cbt_file::local_port() const
{
	return m_server->peer_port();
}

void Cbt_file::sub_file_priority(const string& id, int priority)
{
	for (t_sub_files::iterator i = m_sub_files.begin(); i != m_sub_files.end(); i++)
	{
		if (i->name() != id)
			continue;
		i->priority(priority);
		update_piece_priorities();
		return;
	}
}

void Cbt_file::update_piece_priorities()
{
	for (int i = 0; i < m_pieces.size(); i++)
		m_pieces[i].m_priority = -128;
	__int64 offset = 0;
	for (t_sub_files::iterator i = m_sub_files.begin(); i != m_sub_files.end(); i++)
	{
		int b = (offset + i->size() - 1) / mcb_piece;
		for (int a = offset / mcb_piece; a <= b; a++)
			m_pieces[a].m_priority = max(m_pieces[a].m_priority, i->priority());
		offset += i->size();
	}
}
