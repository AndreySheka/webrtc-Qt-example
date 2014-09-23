#ifndef PEERCONNECTIONCLIENTOBSERVER_H
#define PEERCONNECTIONCLIENTOBSERVER_H
#pragma once
#include <string>
#include "json/json.h"
class PeerConnectionClientObserver {
public:
	virtual void OnSignedIn() = 0;  // Called when we're logged on.
	virtual void OnDisconnected() = 0;
	virtual void OnPeerConnected(int id, const Json::Value name) = 0;
	virtual void OnPeerDisconnected() = 0;
	virtual void OnMessageFromPeer(int peer_id, const Json::Value message) = 0;
	virtual void OnMessageSent(int peer_id, const Json::Value message) = 0;
	virtual void OnServerConnectionFailure() = 0;
	virtual void OnListChange(Json::Value list) = 0;

protected:
	virtual ~PeerConnectionClientObserver() {}
};

#endif