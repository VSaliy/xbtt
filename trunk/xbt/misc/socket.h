// socket.h: interface for the Csocket class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SOCKET_H__7FCC2721_54CD_4609_8737_92478B4090EA__INCLUDED_)
#define AFX_SOCKET_H__7FCC2721_54CD_4609_8737_92478B4090EA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>

using namespace std;

#ifdef WIN32
#include <windows.h>

typedef int socklen_t;

const int MSG_NOSIGNAL = 0;
#endif

class Csocket_source
{
public:
	Csocket_source* attach();
	void detach();
	Csocket_source(SOCKET s);

	operator SOCKET() const
	{
		return m_s;
	}
private:	
	SOCKET m_s;
	int mc_references;
};

class Csocket  
{
public:
	static int get_host(const string& name);
	int bind(int h, int p);
	int blocking(bool v);
	void close();
	int connect(int h, int p);
	const Csocket& open(int t, bool blocking = false);
	int recv(void*, int);
	int send(const void*, int);
	Csocket(SOCKET = INVALID_SOCKET);
	Csocket(const Csocket&);
	const Csocket& operator=(const Csocket&);
	~Csocket();

	operator SOCKET() const
	{
		return m_source ? static_cast<SOCKET>(*m_source) : INVALID_SOCKET;
	}
private:
	Csocket_source* m_source;
};

#endif // !defined(AFX_SOCKET_H__7FCC2721_54CD_4609_8737_92478B4090EA__INCLUDED_)
