// bt_peer_link.h: interface for the Cbt_peer_link class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BT_PEER_LINK_H__50DC0701_DE7E_4A9D_B266_8C3188C24072__INCLUDED_)
#define AFX_BT_PEER_LINK_H__50DC0701_DE7E_4A9D_B266_8C3188C24072__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "alerts.h"
#include "bt_logger.h"
#include "bt_pl_write_data.h"
#include "data_counter.h"
#include "ring_buffer.h"
#include "socket.h"
#include "stream_writer.h"

class Cbt_file;
class Cbt_piece;

class Cbt_peer_link  
{
public:
	int c_max_requests_pending() const;
	void check_pieces();
	Cbt_logger& logger();
	void clear_local_requests();
	int cb_write_buffer() const;
	void alert(Calert::t_level, const string&);
	int pre_dump() const;
	void dump(Cstream_writer&) const;
	void write_piece(int, int, int, const void*);
	void write_merkle_piece(__int64 offset, int size, const void* s, const string& hashes);
	void queue_have(int);
	void write_have(int);
	void write_haves();
	int read_handshake(const char* r);
	void read_message(const char* s, const char* s_end);
	void read_info(const char* r, const char* r_end);
	void write_keepalive();
	void read_piece(int, int, int, const char*);
	void read_merkle_piece(__int64 offset, int size, const char* s, const string& hashes);
	void write_handshake();
	void write_request(int, int, int);
	void write_merkle_cancel(__int64 offset);
	void write_merkle_request(__int64 offset, int c_hashes);
	void write_cancel(int, int, int);
	void write_get_info(int);
	void write_info(int);
	void write_get_peers();
	void write_peers();
	void choked(bool);
	void interested(bool);
	static byte* write16(byte* w, int v);
	static byte* write(byte* w, int v);
	void write_bitfield();
	void remote_requests(int, int, int);
	void remote_has(int);
	void remote_cancels(int, int, int);
	void remote_merkle_cancels(__int64 offset);
	void remote_merkle_requests(__int64 offset, int c_hashes);
	int send();
	int recv();
	void write(const Cvirtual_binary&);
	void write(const void* s, int cb_s);
	void close();
	int pre_select(fd_set* fd_read_set, fd_set* fd_write_set, fd_set* fd_except_set);
	int post_select(fd_set* fd_read_set, fd_set* fd_write_set, fd_set* fd_except_set);
	Cbt_peer_link();
	~Cbt_peer_link();

	void send_quota(int v)
	{
		m_send_quota = v;
	}

	operator bool() const
	{
		return m_s != INVALID_SOCKET;
	}

	struct t_local_request
	{
		__int64 offset;
		int size;
		int stime;

		t_local_request()
		{
		}

		t_local_request(__int64 _offset, int _size)
		{
			offset = _offset;
			size = _size;
			stime = time(NULL);
		}
	};

	struct t_remote_request
	{
		int c_hashes;
		__int64 offset;
		int size;

		t_remote_request()
		{
		}

		t_remote_request(__int64 _offset, int _size, int _c_hashes)
		{
			c_hashes = _c_hashes;
			offset = _offset;
			size = _size;
		}
	};

	typedef list<t_local_request> t_local_requests;
	typedef set<int> t_have_queue;
	typedef set<Cbt_piece*> t_pieces;
	typedef vector<bool> t_remote_pieces;
	typedef list<t_remote_request> t_remote_requests;
	typedef list<Cbt_pl_write_data> t_write_buffer;
	
	sockaddr_in m_a;
	Cbt_file* m_f;
	Csocket m_s;
	int m_state;
	Cring_buffer m_read_b;
	t_write_buffer m_write_b;
	t_have_queue m_have_queue;
	t_pieces m_pieces;
	int m_rtime;
	int m_stime;
	int m_check_pieces_time;
	int m_send_quota;

	bool m_local_link;
	bool m_local_choked;
	bool m_local_interested;
	t_local_requests m_local_requests;
	int mc_local_requests_pending;
	bool m_remote_choked;
	bool m_remote_interested;
	string m_remote_peer_id;
	t_remote_pieces m_remote_pieces;
	t_remote_requests m_remote_requests;
	__int64 m_downloaded;
	__int64 m_left;
	__int64 m_uploaded;
	Cdata_counter m_down_counter;
	Cdata_counter m_up_counter;
	int m_get_peers_stime;
	int m_peers_stime;
	bool m_get_info_extension;
	bool m_get_peers_extension;
};

#endif // !defined(AFX_BT_PEER_LINK_H__50DC0701_DE7E_4A9D_B266_8C3188C24072__INCLUDED_)
