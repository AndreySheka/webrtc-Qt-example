#ifndef DATASOCKET_H
#define DATASOCKET_H
#pragma once
#include "SocketBase.h"
#include "F:\webrtc\trunk\third_party\jsoncpp\source\include\json\json.h"
#include "utils.h"


//repesents a single connected client socket.
class DataSocket :
	public SocketBase
{
public:
	enum RequestMethod {
		INVALID=0,
		GET,
		POST, //only use POST at present.
		OPTIONS,
	};
	explicit DataSocket(int socket)
		: SocketBase(socket),
		method_(INVALID),
		content_length_(0){}
	~DataSocket(){}

	RequestMethod method() const { return method_; }

	//receiving check
	bool HeaderReceived() const { return method_ != INVALID; }
	bool RequestReceived() const {return HeaderReceived() && (method_ == POST || DataReceived());}
	bool DataReceived() const {return method_ == POST || !data_.empty();}

	// Called when we have received some data from clients.
	// Returns false if an error occurred.
	bool OnDataAvailable(bool* close_socket);

	// Send a raw buffer of bytes.
	bool Send(const std::string& data) const;

	// Send an HTTP response.  The |status| should start with a valid HTTP
	// response code, followed by a string.  E.g. "200 OK".
	// If |connection_close| is set to true, an extra "Connection: close" HTTP
	// header will be included.  |content_type| is the mime content type, not
	// including the "Content-Type: " string.
	// |extra_headers| should be either empty or a list of headers where each
	// header terminates with "\r\n".
	// |data| is the body of the message.  It's length will be specified via
	// a "Content-Length" header.
	bool Send(const std::string& status, bool connection_close,
		const std::string& content_type,
		const std::string& extra_headers, const std::string& data) const;

	//all these parses use regular express.
	bool ParseHeaders(std::string&);
	bool ParseData(std::string& data);
	bool ParseMethod(std::string&method);
	bool ParseContentLength(std::string&content_length);

	static const char kCrossOriginAllowHeaders[];
	RequestMethod method_;//always be POST
	size_t content_length_;

	//json is convenient to parse.
	//so we use POST to transfer datas in json form.
	Json::Value data_;       
};

#endif