
#include "PeerChannel.h"
#include "DataSocket.h"
#include "utils.h"
const char* METHOD[] = { "GET", "POST", "OPTION" };
const char* REQUESTACTIONS[] = { "heartbeat_check", "sign_out", "message_to_peer" };
ChannelMember* PeerChannel::Lookup(DataSocket* ds) const 
{
	assert(ds);
	std::string action = ds->data_["action"].asString();
	if (ds->method() != DataSocket::GET && ds->method() != DataSocket::POST)
		return NULL;

	size_t i = 0;
	for (; i < 3; i++)
	{
		if (action==REQUESTACTIONS[i])
			break;
	}

	if (i == 3)
		return NULL;

	if(ds->data_["peer_id"].isNull()) 
		return NULL;
	int id = ds->data_["peer_id"].asInt();
	Members::const_iterator iter = members_.begin();
	for (; iter != members_.end(); ++iter) 
	{
		if (id == (*iter)->id())
		{
			return *iter;
		}
	}

	return NULL;
}

int PeerChannel::AddMember(DataSocket* ds)
{
	ChannelMember* new_guy = new ChannelMember(ds);
	members_.push_back(new_guy);
	printf("New member added (total=%s): %s\n",
		size_t2str(members_.size()).c_str(), new_guy->name().c_str());
	return new_guy->id_;
}

void PeerChannel::OnClosing(DataSocket* ds) 
{
	for (Members::iterator i = members_.begin(); i != members_.end(); ++i) 
	{
		ChannelMember* m = (*i);
		m->OnClosing(ds);
		if (!m->connected()) 
		{
			i = members_.erase(i);
			printf("Member disconnected: %s\n", m->name().c_str());
			delete m;
			OnPeerListChange();
			if (i == members_.end())
				break;
		}
	}
	printf("Total connected: %s\n", size_t2str(members_.size()).c_str());
}

void PeerChannel::CheckForTimeout() 
{
	for (Members::iterator i = members_.begin(); i != members_.end(); ++i) 
	{
		ChannelMember* m = (*i);
		if (m->IsTimedOut()) 
		{
			printf("Timeout: %s\n", m->name().c_str());
			m->SetDisconnected();
			m->member_socket_ = NULL;
			i = members_.erase(i);
			printf("Member disconnected: %s\n", m->name().c_str());
			delete m;
			if (i == members_.end())
			{
				Sleep(500);
				OnPeerListChange();
				break;
			}
				
		}
	}
	
}

void PeerChannel::DeleteAll() 
{
	for (Members::iterator i = members_.begin(); i != members_.end(); ++i)
		delete (*i);
	members_.clear();
}

bool PeerChannel::OnMessageToPeer(int
	peer_id, int
	to_peer_id, std::string
	message)
{
	char buffer[0xffff];
	for (Members::iterator i = members_.begin(); i != members_.end(); ++i)
	{
		ChannelMember* m = *i;
		if (m->id() == to_peer_id && m->connected())
		{
			Json::Value msg;
			Json::FastWriter writer;
			Json::Reader reader;
			msg["peer_id"] = peer_id;
			msg["to_peer_id"] = to_peer_id;
			msg["action"] = "message_to_peer";
			if (!reader.parse(message, msg["message"]))
			{
				return false;
			}
			sprintf(buffer, "POST / HTTP/1.1\r\nContent-Length:%d\r\nContent-Type:text/plain\r\n\r\n", message.size());
			std::string data_to_send = buffer;
			data_to_send+=writer.write(msg);
			return m->member_socket_->Send(data_to_send);
		}
	}
	return false;
}

Json::Value PeerChannel::BuildList()
{
	Json::Value list;
	Json::Value tmp;
	for (Members::iterator i = members_.begin(); i != members_.end(); ++i)
	{
		ChannelMember* m = *i;
		tmp["peer_name"] = m->name();
		tmp["peer_id"] = m->id();
		list.append(tmp);
	}
	return list;
}

bool PeerChannel::OnPeerListChange()
{
	char buffer[0xffff];
	Json::Value message;
	Json::FastWriter writer;
	message["list"] = BuildList();
	message["action"] = "list_change";
	for (Members::iterator i = members_.begin(); i != members_.end(); ++i)
	{
		ChannelMember* m = *i;
		sprintf(buffer, "POST / HTTP/1.1\r\nContent-Length:%d\r\nContent-Type:text/plain\r\n\r\n", writer.write(message).length());
		std::string data_to_send = buffer;
		data_to_send += writer.write(message);
		m->member_socket_->Send(data_to_send);
	}
	return true;
}