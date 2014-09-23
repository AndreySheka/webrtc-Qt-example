
#include "ChannelMember.h"
int ChannelMember::s_member_id_ = 0;

ChannelMember::ChannelMember(DataSocket* socket)
	:member_socket_(socket),
	timestamp_(time(NULL)),
	id_(++s_member_id_),
	connected_(true)
{
	assert(socket);
	assert(socket->data_["action"].asString() == std::string("sign_in"));
	name_ = socket->data_["peer_name"].asString();
}

void ChannelMember::OnClosing(DataSocket* ds) 
{
	//we need do nothing.
	//may be used in future.
	if (ds == member_socket_)
	{
		member_socket_ = NULL;
		connected_ = false;
	}
}