#include "PeerConductor.h"


PeerConductor::PeerConductor(PeerConnectionClient *client, render::UIcallbackInterface* UIinterface)
	:client_(client), 
	peer_id_(INVALID_ID),
	UI_(UIinterface)
{
	ASSERT(client);
	ASSERT(UIinterface);
	InitializeCriticalSection(&lock);
}


PeerConductor::~PeerConductor()
{
}

void PeerConductor::OnStartLogin(const std::string server, int port)
{
	if (client_->is_connected())
	{
		UI_->log(render::UIcallbackInterface::WARNING, new QString("already logged in."));
		return;
	}
		
	server_ = server;
	client_->ConnectToServer(server,port);
}

void PeerConductor::OnSignedIn()
{
	UI_->ui.stackedWidget->setCurrentIndex(1);
	client_->state_ = PeerConnectionClient::State::SIGNED_IN;
	UI_->log(render::UIcallbackInterface::NORMAL, new QString("suecceed to sign in."));
}

void PeerConductor::OnDisconnected()
{
	client_->close();
	server_.clear();
	UI_->log(render::UIcallbackInterface::NORMAL, new QString("suecceed to sign out."));
}

void PeerConductor::OnMessageFromPeer(int peer_id, const Json::Value message)
{
	EnterCriticalSection(&lock);
	ASSERT(peer_id_ == peer_id || peer_id_ == -1);
	ASSERT(!message.empty());

	if (!peer_connection_.get()) 
	{
		ASSERT(peer_id_ == -1);
		peer_id_ = peer_id;
		UI_->peer_id_ = peer_id;
		if (!InitializePeerConnection()) {
			client_->SignOut();
			return;
		}
	}
	else if (peer_id != peer_id_) 
	{
		ASSERT(peer_id_ != -1);
		return;
	}
	std::string type;
	Json::FastWriter writer;
	std::string json_object;
	json_object = writer.write(message);
	type = message["action"].asString();
	if (type == "hangup")
	{
		OnDisconnectFromCurrentPeer();
		return;
	}
	type = message["type"].asString();
	
	if (!type.empty()) 
	{
		std::string sdp;
		UI_->SetUIstatus(UI_->NOT_CONNECTED);
		UI_->peer_state_ = UI_->CONNECTED;
		sdp = message["sdp"].asString();
		if (sdp.empty()) 
		{
			UI_->log(render::UIcallbackInterface::WARNING, new QString("cannot read sdp info from peer."));
			return;
		}
		webrtc::SessionDescriptionInterface* session_description(
			webrtc::CreateSessionDescription(type, sdp));
		UI_->log(render::UIcallbackInterface::NORMAL, new QString("sdp received from peer."));
		if (!session_description) 
		{
			UI_->log(render::UIcallbackInterface::ERRORS, new QString("create sdp faild."));
			return;
		}
		peer_connection_->SetRemoteDescription(
			DummySetSessionDescriptionObserver::Create(), session_description);
		if (session_description->type() ==
			webrtc::SessionDescriptionInterface::kOffer) 
		{
			peer_connection_->CreateAnswer(this, NULL);
		}
		return;
	}
	else 
	{
		std::string sdp_mid;
		int sdp_mlineindex = -1;
		std::string sdpCandidate;
		sdp_mid = message["sdpMid"].asString();
		sdp_mlineindex = message["sdpMLineIndex"].asInt();
		sdpCandidate = message["candidate"].asString();
		if (sdp_mid.empty()||
			sdp_mlineindex<0 ||
			sdpCandidate.empty())
		{
			UI_->log(render::UIcallbackInterface::ERRORS, new QString("cannot get ice candidate info from peer."));
			return;
		}
		talk_base::scoped_ptr<webrtc::IceCandidateInterface> candidate(
			webrtc::CreateIceCandidate(sdp_mid, sdp_mlineindex, sdpCandidate));

		if (!candidate.get()) 
		{
			UI_->log(render::UIcallbackInterface::ERRORS, new QString("cannot get ice candidate info from peer."));
			return;
		}
		UI_->log(render::UIcallbackInterface::NORMAL, new QString("ice candidate info received."));
		if (!peer_connection_->AddIceCandidate(candidate.get())) 
		{
			UI_->log(render::UIcallbackInterface::ERRORS, new QString("failed to add candidate."));
			return;
		}
		return;
	}
	LeaveCriticalSection(&lock);
}

void PeerConductor::OnConnectToPeer(int peer_id)
{
	ASSERT(peer_id_ == -1);
	ASSERT(peer_id != -1);

	if (peer_connection_.get()) 
	{
		//err
		UI_->log(render::UIcallbackInterface::ERRORS, new QString("peer connection creation failed."));
		return;
	}

	if (InitializePeerConnection()) 
	{
		peer_id_ = peer_id;
		peer_connection_->CreateOffer(this, NULL);
	}
	else 
	{
		UI_->log(render::UIcallbackInterface::ERRORS, new QString("peer connection initialization failed."));
		//err
	}
}

bool PeerConductor::InitializePeerConnection()
{
	ASSERT(peer_connection_factory_.get() == NULL);
	ASSERT(peer_connection_.get() == NULL);

	peer_connection_factory_ = webrtc::CreatePeerConnectionFactory();

	if (!peer_connection_factory_.get()) 
	{
		UI_->log(render::UIcallbackInterface::ERRORS, new QString("peer connection factory initialization failed."));
		DeletePeerConnection();
		return false;
	}

	webrtc::PeerConnectionInterface::IceServers servers;
	webrtc::PeerConnectionInterface::IceServer server;
	server.uri = GetPeerConnectionString();
	servers.push_back(server);
	peer_connection_ = peer_connection_factory_->CreatePeerConnection(servers,
		NULL,
		NULL,
		NULL,
		this);
	if (!peer_connection_.get()) {
		//err
		UI_->log(render::UIcallbackInterface::ERRORS, new QString("peer connection creation failed."));
		DeletePeerConnection();
	}
	AddStreams();
	UI_->log(UI_->NORMAL, new QString("Initialization of PeerConnection finished."));
	return peer_connection_.get() != NULL;
}

void PeerConductor::DeletePeerConnection() 
{
	UI_->StopLocalRenderer();
	UI_->StopRemoteRenderer();
	peer_connection_.release();
	active_streams_.clear();

	peer_connection_factory_ = NULL;
	peer_id_ = -1;
	UI_->log(render::UIcallbackInterface::NORMAL, new QString("succeed to delete peer."));
}

void PeerConductor::AddStreams() {
	if (active_streams_.find(kStreamLabel) != active_streams_.end())
	{
		UI_->log(render::UIcallbackInterface::NORMAL, new QString("streams already added."));
		return;  // Already added.
	}
		

	talk_base::scoped_refptr<webrtc::AudioTrackInterface> audio_track(
		peer_connection_factory_->CreateAudioTrack(
		kAudioLabel, peer_connection_factory_->CreateAudioSource(NULL)));

	talk_base::scoped_refptr<webrtc::VideoTrackInterface> video_track(
		peer_connection_factory_->CreateVideoTrack(
		kVideoLabel,
		peer_connection_factory_->CreateVideoSource(OpenVideoCaptureDevice(),
		NULL)));
	UI_->StartLocalRenderer(video_track);

	talk_base::scoped_refptr<webrtc::MediaStreamInterface> stream =
		peer_connection_factory_->CreateLocalMediaStream(kStreamLabel);

	stream->AddTrack(audio_track);
	stream->AddTrack(video_track);
	UI_->log(UI_->NORMAL, new QString("audio and video streams added."));
	if (!peer_connection_->AddStream(stream, NULL)) {
		//err
		UI_->log(render::UIcallbackInterface::ERRORS, new QString("streams addition failed."));
	}
	typedef std::pair<std::string,
		talk_base::scoped_refptr<webrtc::MediaStreamInterface> >
		MediaStreamPair;
	active_streams_.insert(MediaStreamPair(stream->label(), stream));
}

void PeerConductor::OnSuccess(webrtc::SessionDescriptionInterface* desc) 
{
	peer_connection_->SetLocalDescription(
		DummySetSessionDescriptionObserver::Create(), desc);
	Json::FastWriter writer;
	Json::Value jmessage;
	jmessage["type"] = desc->type();
	std::string sdp;
	desc->ToString(&sdp);
	jmessage["sdp"] = sdp;
	std::string msg = writer.write(jmessage);
	ASSERT(peer_id_!=-1);
	UI_->log(UI_->NORMAL, new QString("sdp created successfully."));
	UI_->pending_messages_.push_back(jmessage);
}

// Called when a remote stream is added
void PeerConductor::OnAddStream(webrtc::MediaStreamInterface* stream)
{
	UI_->log(render::UIcallbackInterface::NORMAL, new QString("remote stream received."));
	webrtc::VideoTrackVector tracks = stream->GetVideoTracks();
	// Only render the first track.
	if (!tracks.empty()) 
	{
		UI_->remote_video = tracks[0];
		UI_->on_addstream_ = true;
		Sleep(500);
	}
	stream->Release();
}

void PeerConductor::OnIceCandidate(const webrtc::IceCandidateInterface* candidate)
{
	Json::StyledWriter writer;
	Json::Value jmessage;
	jmessage["sdpMid"] = candidate->sdp_mid();
	jmessage["sdpMLineIndex"] = candidate->sdp_mline_index();
	std::string sdpCandidate;
	if (!candidate->ToString(&sdpCandidate))
	{
		//err
		UI_->log(render::UIcallbackInterface::ERRORS, new QString("cannot get candidate info."));
		return;
	}
	jmessage["candidate"] = sdpCandidate;
	ASSERT(peer_id_ != -1);
	UI_->pending_messages_.push_back(jmessage);
}

void PeerConductor::OnDisconnectFromCurrentPeer()
{
	DeletePeerConnection();
	UI_->peer_state_ = UI_->NOT_CONNECTED;
	UI_->SetUIstatus(render::UIcallbackInterface::PeerStatus::CONNECTED);
	if (UI_->own_wnd_)
	{
		delete UI_->own_wnd_;
		UI_->own_wnd_ = NULL;
	}
	if (UI_->peer_wnd_)
	{
		delete UI_->peer_wnd_;
		UI_->peer_wnd_ = NULL;
	}
	
}

cricket::VideoCapturer* PeerConductor::OpenVideoCaptureDevice() 
{
	talk_base::scoped_ptr<cricket::DeviceManagerInterface> dev_manager(
		cricket::DeviceManagerFactory::Create());
	if (!dev_manager->Init()) {
		//err
		UI_->log(render::UIcallbackInterface::ERRORS, new QString("device manager initialization failed"));
		return NULL;
	}
	std::vector<cricket::Device> devs;
	if (!dev_manager->GetVideoCaptureDevices(&devs)) {
		//err
		UI_->log(render::UIcallbackInterface::ERRORS, new QString("device manager accession to video captrue device failed."));
		return NULL;
	}
	std::vector<cricket::Device>::iterator dev_it = devs.begin();
	cricket::VideoCapturer* capturer = NULL;
	for (; dev_it != devs.end(); ++dev_it) {
		capturer = dev_manager->CreateVideoCapturer(*dev_it);
		if (capturer != NULL)
			break;
	}
	return capturer;
}

void PeerConductor::HangUp()
{
	Json::Value msg;
	msg["action"] = "hangup";
	msg["peer_id"] = client_->my_id_;
	msg["to_peer_id"] = peer_id_;
	UI_->pending_messages_.push_back(msg);
}