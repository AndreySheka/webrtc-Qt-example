#pragma once
#include "DataSocket.h"
#include "SocketBase.h"
class ListenerSocket :
	public SocketBase
{
public:
	ListenerSocket(){}
	bool listen(unsigned short port);
	DataSocket* accept() const;
};

