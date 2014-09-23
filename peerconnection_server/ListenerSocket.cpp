
#include "ListenerSocket.h"

bool ListenerSocket::listen(unsigned short port) 
{
	assert(valid());
	int enabled = 1;
	setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR,
		reinterpret_cast<const char*>(&enabled), sizeof(enabled));
	struct sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);
	if (bind(socket_, reinterpret_cast<const sockaddr*>(&addr),
		sizeof(addr)) == SOCKET_ERROR) 
	{
		printf("bind failed\n");
		return false;
	}
	return ::listen(socket_, 5) != SOCKET_ERROR;
}

DataSocket* ListenerSocket::accept() const {
	assert(valid());
	struct sockaddr_in addr = { 0 };
	socklen_t size = sizeof(addr);
	int client = ::accept(socket_, reinterpret_cast<sockaddr*>(&addr), &size);
	if (client == INVALID_SOCKET)
		return NULL;

	return new DataSocket(client);
}