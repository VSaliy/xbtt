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
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>

#define closesocket close
#define ioctlsocket ioctl
#define WSAGetLastError() errno

#define WSAEACCES EACCES
#define WSAEADDRINUSE EADDRINUSE
#define WSAEADDRNOTAVAIL EADDRNOTAVAIL
#define WSAEAFNOSUPPORT EAFNOSUPPORT
#define WSAEALREADY EALREADY
#define WSAEBADF EBADF
#define WSAECONNABORTED ECONNABORTED
#define WSAECONNREFUSED ECONNREFUSED
#define WSAECONNRESET ECONNRESET
#define WSAEDESTADDRREQ EDESTADDRREQ
#define WSAEDQUOT EDQUOT
#define WSAEFAULT EFAULT
#define WSAEHOSTDOWN EHOSTDOWN
#define WSAEHOSTUNREACH EHOSTUNREACH
#define WSAEINPROGRESS EINPROGRESS
#define WSAEINTR EINTR
#define WSAEINVAL EINVAL
#define WSAEISCONN EISCONN
#define WSAELOOP ELOOP
#define WSAEMFILE EMFILE
#define WSAEMSGSIZE EMSGSIZE
#define WSAENAMETOOLONG ENAMETOOLONG
#define WSAENETDOWN ENETDOWN
#define WSAENETRESET ENETRESET
#define WSAENETUNREACH ENETUNREACH
#define WSAENOBUFS ENOBUFS
#define WSAENOPROTOOPT ENOPROTOOPT
#define WSAENOTCONN ENOTCONN
#define WSAENOTEMPTY ENOTEMPTY
#define WSAENOTSOCK ENOTSOCK
#define WSAEOPNOTSUPP EOPNOTSUPP
#define WSAEPFNOSUPPORT EPFNOSUPPORT
#define WSAEPROTONOSUPPORT EPROTONOSUPPORT
#define WSAEPROTOTYPE EPROTOTYPE
#define WSAEREMOTE EREMOTE
#define WSAESHUTDOWN ESHUTDOWN
#define WSAESOCKTNOSUPPORT ESOCKTNOSUPPORT
#define WSAESTALE ESTALE
#define WSAETIMEDOUT ETIMEDOUT
#define WSAETOOMANYREFS ETOOMANYREFS
#define WSAEUSERS EUSERS
#define WSAEWOULDBLOCK EWOULDBLOCK

typedef int SOCKET;

const int INVALID_SOCKET = -1;
const int SOCKET_ERROR = -1;
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
	static string error2a(int v);
	static int get_host(const string& name);
	static string inet_ntoa(int h);
	static int start_up();
	int accept(int& h, int& p);
	int bind(int h, int p);
	int blocking(bool v);
	void close();
	int connect(int h, int p);
	int getsockopt(int level, int name, void* v, socklen_t& cb_v);
	int getsockopt(int level, int name, int& v);
	int listen();
	const Csocket& open(int t, bool blocking = false);
	int recv(void*, int) const;
	int recvfrom(void* d, int cb_d, sockaddr* a, socklen_t* cb_a) const;
	int send(const void*, int) const;
	int sendto(const void*, int, const sockaddr* a, socklen_t cb_a) const;
	int setsockopt(int level, int name, const void* v, int cb_v);
	int setsockopt(int level, int name, int v);
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
