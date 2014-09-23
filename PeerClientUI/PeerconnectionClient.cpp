#include "PeerConnectionClient.h"
#include "defaults.h"
#include <thread>
#include "C:\Qt\Qt5.3.1\5.3\Src\qtbase\src\winmain\qtmain_win.cpp"
WinsockInitializer winsocketinit;
using talk_base::sprintfn;
char stdHead[] = "POST / HTTP/1.0\r\n\r\n";
PeerConnectionClient::PeerConnectionClient(render::UIcallbackInterface* ui_interface)
	:my_id_(INVALID_ID),
	resolver_(NULL),
	state_(NOT_SIGN_IN),
	client_name_(GetPeerName()),
	control_socket_(new QAsySocket(this)),
	UI_(ui_interface)
	
{
	InitializeCriticalSection(&lock);
	talk_base::InitializeSSL();
}


PeerConnectionClient::~PeerConnectionClient()
{
	talk_base::CleanupSSL();
}


talk_base::AsyncSocket* CreateClientSocket(int family) 
{
	talk_base::Win32Socket* sock = new talk_base::Win32Socket();
	sock->CreateT(family, SOCK_STREAM);
	return sock;
}

bool PeerConnectionClient::InitSocketSignals()
{

	return true;
}

bool PeerConnectionClient::ConnectToServer(const std::string& server, int port)
{
	ASSERT(!server.empty());
	ASSERT(!client_name_.empty());
	if (state_ != NOT_SIGN_IN) 
	{
		UI_->log(UI_->WARNING, new QString("already logged in."));
		client_observer_->OnServerConnectionFailure();
		return false;
	}

	if (server.empty() || client_name_.empty()) 
	{
		UI_->log(UI_->ERRORS, new QString("server ip or client name is empty."));
		client_observer_->OnServerConnectionFailure();
		return false;
	}

	if (port <= 0)
		port = kDefaultServerPort;
	server_=server;
	port_=port;
	DoConnect();
	return true;
}

bool PeerConnectionClient::ConnectControlSocket()
{
	control_socket_->tcp_socket_ = new QTcpSocket();
	control_socket_->RegisterObserver();
	ASSERT(control_socket_->GetState() == QAbstractSocket::UnconnectedState);
	int err = control_socket_->Connect(server_, port_);
	if (err == QAbstractSocket::UnconnectedState)
	{
		close();
		return false;
	}
	return true;
}

void PeerConnectionClient::DoConnect()
{
	Json::Value msg;
	Json::FastWriter writer;
	msg["action"] = "sign_in";
	msg["peer_name"] = client_name_;
	onconnect_data_ = stdHead;
	onconnect_data_ += writer.write(msg);
	bool ret = ConnectControlSocket();
	if (ret)
		state_ = SIGNING_IN;
	if (!ret) {
		UI_->log(UI_->ERRORS, new QString("cannot connect to server."));
		client_observer_->OnServerConnectionFailure();
	}
}

void PeerConnectionClient::OnConnect(QAsySocket* socket)
{
	ASSERT(!onconnect_data_.empty());
	size_t sent = socket->Send(onconnect_data_.c_str(), onconnect_data_.length());
	ASSERT(sent == onconnect_data_.length());
	UNUSED(sent);
	onconnect_data_.clear();
}


void PeerConnectionClient::OnRead(QAsySocket* socket)
{
	EnterCriticalSection(&lock);
	size_t content_length = 0;
	Json::Value control_data_;
	if (ReadIntoBuffer(socket, control_data_, content_length))
	{
		std::string action = control_data_["action"].asString();
		Json::FastWriter writer;
		std::string msg = writer.write(control_data_);
		if (action == "sign_in")
		{
			my_id_ = control_data_["peer_id"].asInt();
			state_ = SIGNED_IN;
			//std::thread sendmessage(&PeerConnectionClient::HeartBeatCheck, this);
			//sendmessage.detach();
			client_observer_->OnSignedIn();
		}
		else if (action == "heartbeat_check")
		{
			//need do nothing
		}
		else if (action=="message_to_peer")
		{
			Json::Value msg_root = control_data_["message"];
			int peer_id = control_data_["peer_id"].asInt();
			UI_->peer_id_ = control_data_["peer_id"].asInt();
			client_observer_->OnMessageFromPeer(peer_id, msg_root);
		}
		else if (action=="list_change")
		{
			Json::Value list_root = control_data_["list"];
			client_observer_->OnListChange(list_root);
			UI_->log(UI_->NORMAL, new QString("peer list changed."));
		}
		else if (action=="sign_out")
		{
			my_id_ = INVALID_ID;
			client_observer_->OnDisconnected();
		}
		else if (action=="hangup")
		{
			client_observer_->OnPeerDisconnected();

		}
		else
		{
			UI_->log(UI_->WARNING, new QString("unknown message received."));
			client_observer_->OnServerConnectionFailure();
		}
	}
	LeaveCriticalSection(&lock);
}

bool PeerConnectionClient::ReadIntoBuffer(QAsySocket* socket,
	Json::Value& data,
	size_t& content_length)
{
	EnterCriticalSection(&lock);
	char buffer[0xffff];
	std::string data_t;
	do {
		int bytes = socket->Recv(buffer, sizeof(buffer));
		if (bytes <= 0)
			break;
		data_t.append(buffer, bytes);
	} while (true);
	std::regex regexp("\\r\\n\\r\\n(.*)");
	std::smatch sm;
	if (std::regex_search(data_t, sm, regexp, std::regex_constants::match_any))
	{
		Json::Reader reader;
		Json::Value msg_data_;
		Json::FastWriter writer;
		data.clear();
		if (reader.parse(sm[1], data)) return true;//sm[1] always means backreference
	}
	UI_->log(UI_->WARNING, new QString("cannot find any data."));
	LeaveCriticalSection(&lock);
	return false;

}

bool PeerConnectionClient::close()
{
	control_socket_->Close();
	my_id_ = INVALID_ID;
	state_ = State::NOT_SIGN_IN;
	control_socket_->UnRegisterObserver();
	return true;
}

bool PeerConnectionClient::SignOut() 
{
	if (state_ == NOT_SIGN_IN || state_ == SIGNING_OUT)
		return true;
	if (control_socket_->GetState() != QTcpSocket::SocketState::UnconnectedState)
	{
		state_ = SIGNING_OUT;
		if (my_id_ != -1) 
		{
			Json::Value msg;
			Json::FastWriter writer;
			msg["action"] = "sign_out";
			msg["peer_id"] = my_id_;
			onconnect_data_ = stdHead;
			onconnect_data_ += writer.write(msg);
			control_socket_->Send(onconnect_data_.c_str(), onconnect_data_.length());
			return true;
		}
		else 
		{
			// Can occur if the app is closed before we finish connecting.
			return true;
		}
	}
	else 
	{
		my_id_ = INVALID_ID;
		state_ = SIGNING_OUT;
		client_observer_->OnDisconnected();
	}

	return true;
}

void PeerConnectionClient::OnResolveResult(
	talk_base::AsyncResolverInterface* resolver) 
{
	//if (resolver_->GetError() != 0) 
	//{
	//	client_observer_->OnServerConnectionFailure();
	//	resolver_->Destroy(false);
	//	resolver_ = NULL;
	//	state_ = NOT_SIGN_IN;
	//}
	//else 
	//{
	//	server_address_ = resolver_->address();
	//	DoConnect();
	//}
}


void PeerConnectionClient::OnClose(QAsySocket* socket, int err)
{
		close();
		client_observer_->OnDisconnected();
}

bool PeerConnectionClient::SendMessageToPeer(int peer_id, const Json::Value message)
{
	if (state_ != SIGNED_IN)
	{
		UI_->log(UI_->ERRORS, new QString("can't send message to peer before having logged in."));
		return false;
	}
		
	ASSERT(is_connected());
	ASSERT(control_socket_->GetState() == QTcpSocket::SocketState::ConnectedState);
	if (!is_connected() || peer_id == -1)
		return false;
	std::string msg = stdHead;
	Json::FastWriter writer;
	Json::Value jmsg;
	jmsg["action"] = "message_to_peer";
	jmsg["peer_id"] = my_id_;
	jmsg["to_peer_id"] = peer_id;
	jmsg["message"] = message;
	msg += writer.write(jmsg);
	control_socket_->Send(msg.c_str(), msg.length());
	return true;
}

void PeerConnectionClient::HeartBeatCheck()
{
	Json::Value heartbeat;
	Json::FastWriter writer;
	std::string msg;
	heartbeat["action"] = "heartbeat_check";
	heartbeat["peer_id"] = my_id_;
	msg = stdHead;
	msg += writer.write(heartbeat);
	while (state_ != NOT_SIGN_IN
		&&control_socket_->GetState() != control_socket_->tcp_socket_->UnconnectedState
		&&is_connected())
	{
		control_socket_->Send(msg.c_str(), msg.length());
		Sleep(1000 * 10);
	}
	return;
}