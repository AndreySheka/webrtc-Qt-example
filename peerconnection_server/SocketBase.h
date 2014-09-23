#pragma once
#ifndef SOCKETBASE_H
#define SOCKETBASE_H
#include <winsock2.h>
typedef int socklen_t;
#include <string>
#include <assert.h>

#ifndef SOCKET_ERROR
#define SOCKET_ERROR (-1)
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET  static_cast<int>(~0)
#endif

#if defined(WIN32)
class WinsockInitializer 
{
public:
	WinsockInitializer() 
	{
		WORD wVersionRequested;
		WSADATA wsaData;
		int err;

		/* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
		wVersionRequested = MAKEWORD(1, 0);

		err = WSAStartup(wVersionRequested, &wsaData);
		if (err != 0) 
		{
			/* Tell the user that we could not find a usable */
			/* Winsock DLL.                                  */
			printf("WSAStartup failed with error: %d\n", err);
		}
	}

public:
	~WinsockInitializer() { WSACleanup(); }
};
#endif

class SocketBase
{
public:
	SocketBase() : socket_(INVALID_SOCKET) { }
	explicit SocketBase(int socket) : socket_(socket) { }
	~SocketBase() { close(); }

	int socket() const { return socket_; }
	bool valid() const { return socket_ != INVALID_SOCKET; }

	bool create();
	void close();

protected:
	int socket_;

};

#endif