
#include "SocketBase.h"


bool SocketBase::create() 
{
	assert(!valid());
	socket_ = ::socket(AF_INET, SOCK_STREAM, 0);
	return valid();
}

void SocketBase::close() 
{
	if (socket_ != INVALID_SOCKET) 
	{
		closesocket(socket_);
		socket_ = INVALID_SOCKET;
	}
}