// bt_piece.cpp: implementation of the Cbt_piece class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bt_piece.h"

#define for if (0) {} else for

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Cbt_piece::Cbt_piece()
{
	mc_peers = 0;
	m_priority = 0;
	m_valid = false;
}

int Cbt_piece::resize(int v)
{
	return m_size = v;
}

void Cbt_piece::write(int offset, const char* s, int cb_s)
{
	int b = offset / cb_sub_piece();
	if (m_valid || offset < 0 || offset >= size() || offset % cb_sub_piece() || cb_s != cb_sub_piece(b))
		return;
	if (m_sub_pieces.empty())
		m_sub_pieces.resize(mc_sub_pieces_left = c_sub_pieces());
	if (m_sub_pieces[b])
		return;
	m_sub_pieces[b] = true;
	if (!--mc_sub_pieces_left)
	{
		for (t_peers::const_iterator i = m_peers.begin(); i != m_peers.end(); i++)
			(*i)->m_pieces.erase(this);
		m_peers.clear();
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

int Cbt_piece::pre_dump() const
{
	return 12;
}

int Cbt_piece::rank() const
{
	return 400000 * min(m_peers.size(), 9)
		+ 20000 * max(0, min(9 - m_priority, 19)) 
		+ 2000 * min(mc_peers, 9)
		+ 1000 * m_sub_pieces.empty()
		+ min(mc_peers, 999);
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
		m_sub_pieces.resize(mc_sub_pieces_left = c_sub_pieces());
		for (t_sub_pieces::iterator i = m_sub_pieces.begin(); i != m_sub_pieces.end(); i++)
		{
			*i = r.read_int(1);
			mc_sub_pieces_left -= *i;
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
			w.write_int(1, *i);
	}
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

