#ifndef PEERCONDUCTOR_H
#define PEERCONDUCTOR_H
#pragma once
#include <deque>
#include <map>
#include <set>
#include <string>
#include "talk\base\sigslot.h"
#include "talk/base/scoped_ptr.h"
#include "talk/app/webrtc/mediastreaminterface.h"
#include "talk/app/webrtc/peerconnectioninterface.h"
#include "talk/app/webrtc/videosourceinterface.h"
#include "talk/media/base/mediachannel.h"
#include "talk/media/base/videocommon.h"
#include "talk/media/base/videoframe.h"
#include "talk/media/base/videorenderer.h"
#include "talk\media\devices\devicemanager.h"
#include "PeerConnectionClientObserver.h"
#include "PeerconnectionClient.h"
#include "UIcallbackInterface.h"
#include "defaults.h"

namespace webrtc 
{
	class VideoCaptureModule;
}  // namespace webrtc

namespace cricket 
{
	class VideoRenderer;
}  // namespace cricket

namespace render
{
	class UIcallbackInterface;
}


class DummySetSessionDescriptionObserver
	: public webrtc::SetSessionDescriptionObserver 
{
public:
	static DummySetSessionDescriptionObserver* Create() 
	{
		return
			new talk_base::RefCountedObject<DummySetSessionDescriptionObserver>();
	}
	virtual void OnSuccess() {}
	virtual void OnFailure(const std::string& error) {}

protected:
	DummySetSessionDescriptionObserver() {}
	~DummySetSessionDescriptionObserver() {}
};



class PeerConductor:
	public sigslot::has_slots<>,
	public PeerConnectionClientObserver,
	public webrtc::PeerConnectionObserver,
	public webrtc::CreateSessionDescriptionObserver

{
public:
	PeerConductor(PeerConnectionClient* client, render::UIcallbackInterface* UIinterface);
	~PeerConductor();
	bool InitializePeerConnection();
	void DeletePeerConnection();
	void AddStreams();
	void HangUp();
	cricket::VideoCapturer* OpenVideoCaptureDevice();

	//UI signal slot
	virtual void OnStartLogin(const std::string server, int port);
	virtual void OnDisconnectFromServer(){ client_->SignOut(); }
	virtual void OnConnectToPeer(int peer_id);
	virtual void OnDisconnectFromCurrentPeer();//should always call it to disconnect from peer

	//
	// PeerConnectionObserver implementation.
	//
	virtual void OnError(){}
	virtual void OnStateChange(
		webrtc::PeerConnectionObserver::StateType state_changed) {}
	virtual void OnAddStream(webrtc::MediaStreamInterface* stream);
	virtual void OnRemoveStream(webrtc::MediaStreamInterface* stream){}
	virtual void OnRenegotiationNeeded() {}
	virtual void OnIceChange() {}
	virtual void OnIceCandidate(const webrtc::IceCandidateInterface* candidate);

	//
	// PeerConnectionClientObserver implementation.
	//
	virtual void OnSignedIn();

	virtual void OnDisconnected();

	virtual void OnPeerConnected(int id, const Json::Value name){}

	virtual void OnPeerDisconnected(){ OnDisconnectFromCurrentPeer(); }

	virtual void OnMessageFromPeer(int peer_id, const Json::Value message);

	virtual void OnMessageSent(int peer_id, const Json::Value message){ client_->SendMessageToPeer(peer_id, message); }

	virtual void OnServerConnectionFailure(){ OnDisconnectFromServer(); }
	
	virtual void OnListChange(Json::Value list){ UI_->RefreshPeerList(list); }

	// CreateSessionDescriptionObserver implementation.
	virtual void OnSuccess(webrtc::SessionDescriptionInterface* desc);
	virtual void OnFailure(const std::string& error){}

private:
	int peer_id_;
	render::UIcallbackInterface *UI_;
	talk_base::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
	talk_base::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
		peer_connection_factory_;
	PeerConnectionClient *client_;
	std::map<std::string, talk_base::scoped_refptr<webrtc::MediaStreamInterface> >
		active_streams_;
	std::string server_;
	CRITICAL_SECTION lock;
};

#endif