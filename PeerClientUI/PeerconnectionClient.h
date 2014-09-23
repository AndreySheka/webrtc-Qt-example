#ifndef PEERCONNECTIONCLIENT_H 
#define PEERCONNECTIONCLIENT_H 
#pragma once
#include <map>
#include <string>
#include <regex>
#include "PeerConnectionClientObserver.h"
#include "talk/base/nethelpers.h"
#include "talk/base/signalthread.h"
#include "talk/base/sigslot.h"
#include "talk/base/physicalsocketserver.h"
#include "talk/base/scoped_ptr.h"
#include "talk\base\socketaddress.h"
#include "talk/base/ssladapter.h"
#include "talk/base/win32socketinit.h"
#include "stringutils.h"
#include "QtNetwork\qtcpsocket.h"
#include "qasysocket.h"
#include "UIcallbackInterface.h"
#ifdef WIN32
#include "talk/base/win32socketserver.h"
#endif
#include "json/json.h"
namespace render
{
	class UIcallbackInterface;
}
typedef std::map<int, std::string> Peers;
#define INVALID_ID -1
#define INVALID_PORT INVALID_ID

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


//client signaling implementation
class PeerConnectionClient :
	public talk_base::MessageHandler,
	public SocketCallback
{
public:
	enum State
	{
		NOT_SIGN_IN = 0,
		SIGNING_IN,
		SIGNED_IN,
		CONNECTED,
		SIGNING_OUT,
		RESOLVING
	};
	explicit PeerConnectionClient(render::UIcallbackInterface*);
	~PeerConnectionClient();

	int id(){ return my_id_; }
	bool is_connected(){ return my_id_ != INVALID_ID; }
	void PeerListener();
	bool InitSocketSignals();
	bool ConnectToServer(const std::string& server, int port);
	void DoConnect();
	bool ConnectControlSocket();
	bool close();
	void HeartBeatCheck();
	bool ReadIntoBuffer(QAsySocket* socket,
		Json::Value& data,
		size_t& content_length);
	bool SignOut();
	// implements the MessageHandler interface
	void OnMessage(talk_base::Message* msg){ DoConnect(); }

	//ensure socket address is resolved
	void OnResolveResult(talk_base::AsyncResolverInterface* resolver);

	//asynchronous socket callbacks
	void OnConnect(QAsySocket* socket);
	//check whether server is down here.
	void OnRead(QAsySocket* socket);
	void OnClose(QAsySocket* socket, int err);

	//communicate with peer via server
	bool SendMessageToPeer(int peer_id, const Json::Value message);

	// PeerConnectionClientObserver signal defines

	//talk_base::SocketAddress server_address_;
	std::string server_;
	int port_;
	talk_base::AsyncResolver* resolver_;

	//signaling control socket
	//talk_base::scoped_ptr<talk_base::AsyncSocket> control_socket_;
	QAsySocket* control_socket_;
	std::string onconnect_data_;
	std::string client_name_;
	State state_;
	int my_id_;
	PeerConnectionClientObserver *client_observer_;
	render::UIcallbackInterface* UI_;
	CRITICAL_SECTION lock;
};

#endif