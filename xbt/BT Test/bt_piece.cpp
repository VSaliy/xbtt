// bt_piece.cpp: implementation of the Cbt_piece class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bt_piece.h"

#define for if (0) {} else for

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void Cbt_sub_piece::erase_peers(Cbt_piece* piece)
{
	for (t_peers::const_iterator i = m_peers.begin(); i != m_peers.end(); i++)
		(*i)->m_pieces.erase(piece);
}

Cbt_piece::Cbt_piece()
{
	mc_peers = 0;
	m_priority = 0;
	m_valid = false;
}

int Cbt_piece::resize(int v)
{
	m_size = v;
	mc_sub_pieces_left = mc_unrequested_sub_pieces = c_sub_pieces();
	return size();
}

int Cbt_piece::next_invalid_sub_piece(Cbt_peer_link* peer)
{
	if (!mc_unrequested_sub_pieces)
		return -1;
	m_sub_pieces.resize(c_sub_pieces());
	for (t_sub_pieces::iterator i = m_sub_pieces.begin(); i != m_sub_pieces.end(); i++)
	{
		if (i->valid() || !i->m_peers.empty())
			continue;
		i->m_peers.insert(peer);
		mc_unrequested_sub_pieces--;
		return &*i - &m_sub_pieces.front();
	}
	assert(false);
	return -1;
}

void Cbt_piece::erase_peer(Cbt_peer_link* peer)
{
	for (t_sub_pieces::iterator i = m_sub_pieces.begin(); i != m_sub_pieces.end(); i++)
	{
		if (i->m_peers.find(peer) == i->m_peers.end())
			continue;
		i->m_peers.erase(peer);
		assert(!i->valid());
		mc_unrequested_sub_pieces++;
	}
}

void Cbt_piece::write(int offset, const char* s, int cb_s)
{
	int b = offset / cb_sub_piece();
	assert(b < m_sub_pieces.size());
	if (m_valid || offset < 0 || offset >= size() || offset % cb_sub_piece() || cb_s != cb_sub_piece(b)
		|| b >= m_sub_pieces.size() || m_sub_pieces[b].valid())
		return;
	if (m_sub_pieces[b].m_peers.empty())
		mc_unrequested_sub_pieces--;
	m_sub_pieces[b].valid(true);
	if (!--mc_sub_pieces_left)
	{
		for (t_sub_pieces::iterator i = m_sub_pieces.begin(); i != m_sub_pieces.end(); i++)
			i->erase_peers(this);
		m_sub_pieces.clear();
	}
}

int Cbt_piece::c_sub_pieces() const
{
	return (size() + cb_sub_piece() - 1) / cb_sub_piece();
}

int Cbt_piece::cb_sub_piece(int b)
{
	return min(cb_sub_piece() * (b + 1), size()) - cb_sub_piece() * b;
}

int Cbt_piece::rank() const
{
	return 20000 * max(0, min(9 - m_priority, 19)) 
		+ 2000 * min(mc_peers, 9)
		+ 1000 * m_sub_pieces.empty()
		+ min(mc_peers, 999);
}

int Cbt_piece::pre_dump() const
{
	return 12;
}

void Cbt_piece::dump(Cstream_writer& w) const
{
	if (m_sub_pieces.empty())
	{
		w.write_int(1, 0);
		w.write_int(1, 0);
	}
	else
	{
		w.write_int(1, mc_sub_pieces_left);
		w.write_int(1, c_sub_pieces() - mc_sub_pieces_left);
	}
	w.write_int(4, mc_peers);
	w.write_int(1, m_priority);
	w.write_int(4, rank());
	w.write_int(1, m_valid);
}

void Cbt_piece::load_state(Cstream_reader& r)
{
	switch (r.read_int(1))
	{
	case 0:
		m_valid = false;
		break;
	case 1:
		m_valid = false;
		m_sub_pieces.resize(mc_sub_pieces_left = mc_unrequested_sub_pieces = c_sub_pieces());
		for (t_sub_pieces::iterator i = m_sub_pieces.begin(); i != m_sub_pieces.end(); i++)
		{
			i->valid(r.read_int(1));
			mc_sub_pieces_left -= i->valid();
			mc_unrequested_sub_pieces -= i->valid();
		}
		break;
	case 2:
		m_valid = true;
		break;
	}
}

int Cbt_piece::pre_save_state() const
{
	return m_valid || m_sub_pieces.empty() ? 1 : m_sub_pieces.size() + 1;
}

void Cbt_piece::save_state(Cstream_writer& w) const
{
	if (m_valid)
		w.write_int(1, 2);
	else if (m_sub_pieces.empty())
		w.write_int(1, 0);
	else
	{
		w.write_int(1, 1);
		for (t_sub_pieces::const_iterator i = m_sub_pieces.begin(); i != m_sub_pieces.end(); i++)
			w.write_int(1, i->valid());
	}
}
