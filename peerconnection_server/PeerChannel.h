#pragma once
#ifndef PEERCHANNEL_H
#define PEERCHANNEL_H
#include <time.h>

#include <queue>
#include <string>
#include <vector>
#include <memory>
#include "ChannelMember.h"

class PeerChannel
{
public:
	typedef std::vector<ChannelMember*> Members;
	PeerChannel(){}
	~PeerChannel(){ DeleteAll(); }

	// Finds a connected peer that's associated with the |ds| socket.
	ChannelMember* Lookup(DataSocket* ds) const;
	const Members& members() const { return members_; }
	Json::Value BuildList();
	// Adds a new ChannelMember instance to the list of connected peers and
	// associates it with the socket.
	int AddMember(DataSocket* ds);

	bool OnMessageToPeer(int
		peer_id, int
		to_peer_id, std::string
		message);

	bool OnPeerListChange();
	// Closes all connections and sends a "shutting down" message to all
	// connected peers.
	void DeleteAll();

	// Called to clear a member when a socket was determined to be closing by the peer (or if the
	// connection went dead).
	void OnClosing(DataSocket* ds);

	void CheckForTimeout();
protected:
	

	Members members_;
};

#endif