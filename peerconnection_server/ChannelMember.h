#pragma once
#include <time.h>

#include <queue>
#include <string>
#include <vector>
#include <assert.h>
#include "DataSocket.h"


class ChannelMember
{
public:
	explicit ChannelMember(DataSocket* socket);
	~ChannelMember(){}
	bool connected() const { return connected_; }
	int id() const { return id_; }
	const std::string& name() const { return name_; }
	void SetDisconnected() { connected_ = false; }
	void RefreshTimeStamp(){ timestamp_ = time(NULL); }
	void OnClosing(DataSocket* ds);
	bool IsTimedOut(){ return !connected()||(time(NULL) - timestamp_) >= 600; }
	int id_;
	DataSocket* member_socket_;
	bool connected_;
	time_t timestamp_;
	std::string name_;
	static int s_member_id_;
};

