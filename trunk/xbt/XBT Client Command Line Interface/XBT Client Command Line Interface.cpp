#include "stdafx.h"
#include "bt_misc.h"
#include "bt_strings.h"
#include "bvalue.h"

int send(Csocket& s, const string& v)
{
	return s.send(v.c_str(), v.size()) != v.size();
}

int send(Csocket& s, const Cbvalue& v)
{
	char d0[5];
	Cvirtual_binary d1 = v.read();
	*reinterpret_cast<__int32*>(d0) = htonl(d1.size() + 1);
	d0[4] = bti_bvalue;
	if (s.send(d0, 5) != 5)
		return 1;
	return s.send(d1, d1.size()) != d1.size();
}

string strip_name(const string& v)
{
	int i = v.rfind('\\');
	return i == string::npos ? v : v.substr(i + 1);
}

ostream& show_options(ostream& os, const Cbvalue& v)
{
	cout << "admin port: \t" << static_cast<int>(v.d(bts_admin_port).i()) << endl
		<< "peer port: \t" << static_cast<int>(v.d(bts_peer_port).i()) << endl
		<< "tracker port: \t" << static_cast<int>(v.d(bts_tracker_port).i()) << endl
		<< "upload rate: \t" << static_cast<int>(v.d(bts_upload_rate).i()) << endl
		<< "upload slots: \t" << static_cast<int>(v.d(bts_upload_slots).i()) << endl
		<< "seeding ratio: \t" << static_cast<int>(v.d(bts_seeding_ratio).i()) << endl;
	return os;
}

ostream& show_status(ostream& os, const Cbvalue& v)
{
	const Cbvalue& files = v.d(bts_files);
	for (Cbvalue::t_map::const_iterator i = files.d().begin(); i != files.d().end(); i++)
	{
		cout << hex_encode(i->first) 
			<< '\t' << static_cast<int>(i->second.d(bts_incomplete).i()) << " l"
			<< '\t' << static_cast<int>(i->second.d(bts_complete).i()) << " s"
			<< endl
			<< '\t' << strip_name(i->second.d(bts_name).s())
			<< endl
			<< '\t' << b2a(i->second.d(bts_left).i()) 
			<< '\t' << b2a(i->second.d(bts_size).i()) 
			<< '\t' << b2a(i->second.d(bts_total_downloaded).i()) 
			<< '\t' << b2a(i->second.d(bts_total_uploaded).i()) 
			<< '\t' << b2a(i->second.d(bts_down_rate).i()) 
			<< '\t' << b2a(i->second.d(bts_up_rate).i()) 
			<< endl;
	}
	return os;
}

int main(int argc, char* argv[])
{
#ifdef WIN32
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 0), &wsadata))
		return cerr << "Unable to start WSA" << endl, 1;
#endif
	Csocket s;
	if (s.open(SOCK_STREAM, true) == INVALID_SOCKET)
		return cerr << "Csocket::open failed: " << Csocket::error2a(WSAGetLastError()) << endl, 1;
	if (s.connect(htonl(INADDR_LOOPBACK), htons(6879)))
		return cerr << "Csocket::connect failed: " << Csocket::error2a(WSAGetLastError()) << endl, 1;
	if (argc == 3)
	{
		string hash = hex_decode(argv[2]);
		if (!strcmp(argv[1], "close"))
		{
			Cbvalue v;
			v.d(bts_action, bts_close_torrent);
			v.d(bts_hash, hash);
			if (send(s, v))
				return cerr << "Csocket::send failed: " << Csocket::error2a(WSAGetLastError()) << endl, 1;
		}
		else if (!strcmp(argv[1], "open"))
		{
			Cvirtual_binary a;
			if (a.load(argv[2]))
				return cerr << "Unable to load .torrent" << endl, 1;
			Cbvalue b;
			if (b.write(a))
				return cerr << "Unable to parse .torrent" << endl, 1;
			Cbvalue v;
			v.d(bts_action, bts_open_torrent);
			v.d(bts_torrent, b);
			if (send(s, v))
				return cerr << "Csocket::send failed: " << Csocket::error2a(WSAGetLastError()) << endl, 1;
		}
		else if (!strcmp(argv[1], "pause"))
		{
			Cbvalue v;
			v.d(bts_action, bts_pause_torrent);
			v.d(bts_hash, hash);
			if (send(s, v))
				return cerr << "Csocket::send failed: " << Csocket::error2a(WSAGetLastError()) << endl, 1;
		}
		else if (!strcmp(argv[1], "unpause"))
		{
			Cbvalue v;
			v.d(bts_action, bts_unpause_torrent);
			v.d(bts_hash, hash);
			if (send(s, v))
				return cerr << "Csocket::send failed: " << Csocket::error2a(WSAGetLastError()) << endl, 1;
		}
	}
	if (argc == 2 && !strcmp(argv[1], "options"))
	{
		Cbvalue v;
		v.d(bts_action, bts_get_options);
		if (send(s, v))
			return cerr << "Csocket::send failed: " << Csocket::error2a(WSAGetLastError()) << endl, 1;
	}
	else
	{
		Cbvalue v;
		v.d(bts_action, bts_get_status);
		if (send(s, v))
			return cerr << "Csocket::send failed: " << Csocket::error2a(WSAGetLastError()) << endl, 1;
	}
	const int cb_d = 64 << 10;
	char d[cb_d];
	char* w = d;
	int r;
	while (w - d != cb_d && (r = s.recv(w, d + cb_d - w)))
	{
		if (r == SOCKET_ERROR)
			break;
		w += r;
		if (w - d >= 4 && w - d >= ntohl(*reinterpret_cast<__int32*>(d)))
		{
			Cbvalue v;
			if (v.write(d + 5, ntohl(*reinterpret_cast<__int32*>(d)) - 1))
				break;
			v.read().save("/temp/bvalue.txt");
			if (v.d_has(bts_files))
				show_status(cout, v);
			else
				show_options(cout, v);
			break;
		}
	}
#ifdef WIN32
	WSACleanup();
#endif
	return 0;
}
