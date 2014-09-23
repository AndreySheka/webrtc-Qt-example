// peerconnection_server.cpp : 定义控制台应用程序的入口点。
//

#include <Windows.h>
#include <tchar.h>
#include <memory>
#include <thread>
#include <assert.h>
#include "ListenerSocket.h"
#include "PeerChannel.h"
#ifndef WIN32
#define WIN32
#endif
static const size_t kMaxConnections = (FD_SETSIZE - 2);
WinsockInitializer winsocketinit;
int _tmain(int argc, _TCHAR* argv[])
{
	int FLAG_port = 8888;
	ListenerSocket listener;
	if (!listener.create()) {
		printf("Failed to create server socket\n");
		return -1;
	}
	else if (!listener.listen(FLAG_port)) {
		printf("Failed to listen on server socket\n");
		return -1;
	}
	printf("Server listening on port %i\n", FLAG_port);
	PeerChannel clients;
	typedef std::vector<DataSocket*> SocketArray;
	SocketArray sockets;
	bool quit = false;
	while (!quit)
	{
		fd_set socket_set;
		FD_ZERO(&socket_set);
		if (listener.valid())
			FD_SET(listener.socket(), &socket_set);

		for (SocketArray::iterator i = sockets.begin(); i != sockets.end(); ++i)
			FD_SET((*i)->socket(), &socket_set);

		timeval timeout = { 10, 0 };
		if (select(FD_SETSIZE, &socket_set, NULL, NULL, &timeout) == SOCKET_ERROR)
		{
			printf("select failed\n");
			break;
		}

		for (SocketArray::iterator i = sockets.begin(); i != sockets.end(); ++i)
		{
			DataSocket* s = *i;
			bool socket_done = true;
			Json::FastWriter writer;
			Json::Value response;
			if (FD_ISSET(s->socket(), &socket_set)) 
			{
				if (s->OnDataAvailable(&socket_done) && s->RequestReceived())
				{
					ChannelMember* member = clients.Lookup(s);
					if (!member)
					{
						std::string action = s->data_["action"].asString();
						if (action == std::string("sign_in"))
						{
							int peer_id=clients.AddMember(s);
							response["action"] = action;
							response["peer_id"] = peer_id;
							s->Send("200 OK", true, "text/plain", "", writer.write(response));
							Sleep(300);
							clients.OnPeerListChange();
						}
					}
					else
					{
						std::string action = s->data_["action"].asString();
						if (action == std::string("heartbeat_check"))
						{
							member->RefreshTimeStamp();
							socket_done = false;
							response["action"] = action;
							s->Send("200 OK", true, "text/plain", "", writer.write(response));
						}
						else if (action == std::string("message_to_peer"))
						{
							Json::FastWriter writer;
							std::string msg = writer.write(s->data_);
							int peer_id=s->data_["peer_id"].asInt();
							int to_peer_id=s->data_["to_peer_id"].asInt();
							clients.OnMessageToPeer(peer_id, 
								to_peer_id, 
								writer.write(s->data_["message"]));
							member->RefreshTimeStamp();
							socket_done = false;
						}
						else if (action == std::string("sign_out"))
						{
							response["action"] = action;
							s->Send("200 OK", true, "text/plain", "", writer.write(response));
							Sleep(200);
							socket_done = true;
							clients.OnPeerListChange();
						}
					}
				}
			}
			else
			{
				socket_done = false;
			}

			if (socket_done)
			{
				printf("Disconnecting socket\n");
				clients.OnClosing(s);
				assert(s->valid());  // Close must not have been called yet.
				FD_CLR(s->socket(), &socket_set);
				delete (*i);
				i = sockets.erase(i);
				if (i == sockets.end())
					break;
			}

			
		}




		if (FD_ISSET(listener.socket(), &socket_set))
		{
			DataSocket* s = listener.accept();
			if (sockets.size() >= kMaxConnections)
			{
				delete s;  // sorry, that's all we can take.
				printf("Connection limit reached\n");
			}
			else
			{
				sockets.push_back(s);
				printf("New connection...\n");
			}
		}
	}

	for (SocketArray::iterator i = sockets.begin(); i != sockets.end(); ++i)
		delete (*i);
	sockets.clear();

	return 0;
}

