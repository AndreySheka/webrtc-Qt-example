
#include "DataSocket.h"
#include <regex>
static const char kHeaderTerminator[] = "\r\n\r\n";
static const int kHeaderTerminatorLength = sizeof(kHeaderTerminator) - 1;
const char DataSocket::kCrossOriginAllowHeaders[] =
"Access-Control-Allow-Origin: *\r\n"
"Access-Control-Allow-Credentials: true\r\n"
"Access-Control-Allow-Methods: POST, GET, OPTIONS\r\n"
"Access-Control-Allow-Headers: Content-Type, "
"Content-Length, Connection, Cache-Control\r\n"
"Access-Control-Expose-Headers: Content-Length, X-Peer-Id\r\n";
bool DataSocket::OnDataAvailable(bool* close_socket) 
{
	assert(valid());
	char buffer[0xfff] = { 0 };
	int bytes = recv(socket_, buffer, sizeof(buffer), 0);
	if (bytes == SOCKET_ERROR || bytes == 0) 
	{
		*close_socket = true;
		return false;
	}

	*close_socket = false;
	std::string data;
	bool ret = true;
	if (HeaderReceived()) 
	{
		if (method_ != POST) 
		{
			// unexpectedly received data.
			ret = false;
		}
		else 
		{
			data.append(buffer, bytes);
			ret = ParseHeaders(data);
		}
	}
	else 
	{
		data.append(buffer, bytes);
		//size_t found = data.find(kHeaderTerminator);
		//if (found != std::string::npos) 
		//{
		//	Json::Reader reader;
		//	std::string msg = data.substr(found + kHeaderTerminatorLength);
		//	ret=reader.parse(msg, data_);
		//	data.resize(found + kHeaderTerminatorLength);
		//}
		ret = ParseHeaders(data);
	}
	return ret;
}

bool DataSocket::ParseHeaders(std::string& header)
{
	if (ParseData(header) &&
		ParseMethod(header)/* &&
		ParseContentLength(header)*/) return true;
	return false;
}

bool DataSocket::ParseData(std::string& header)
{
	std::regex regex_name("\\r\\n\\r\\n(.*)");
	std::smatch sm;
	if (std::regex_search(header, sm, regex_name, std::regex_constants::match_any))
	{
		Json::Reader reader;
		if (reader.parse(sm[1], data_))
		{
			std::string content = sm[1];
			content_length_ =content.length() ;
			return true;
		}//sm[1] always means backreference
	}
	return false;
}

bool DataSocket::ParseMethod(std::string& header)
{
	const char* METHOD[] = { "GET", "POST", "OPTION" };
	std::regex regex_name("([A-Z]+)");
	std::smatch sm;
	if (std::regex_search(header, sm, regex_name, std::regex_constants::match_any))
	{
		std::string method = sm[1];
		for (int i = 0; i <= 2; i++)
		{
			if (method==METHOD[i])
			{
				method_ = (RequestMethod)(i + 1);
				return true;
			}
		}
		method_ = RequestMethod::INVALID;
		return true;
	}
	return false;
}

bool DataSocket::ParseContentLength(std::string& header)
{
	std::regex regex_name("Content-Length:[\\s]*([0-9]+)\\r\\n");
	std::smatch sm;
	if(std::regex_search(header, sm, regex_name, std::regex_constants::match_any))
	{
		content_length_ = atoi(sm[1].str().c_str());
		return true;
	}
	return false;
}

bool DataSocket::Send(const std::string& status, bool connection_close,
	const std::string& content_type,
	const std::string& extra_headers,
	const std::string& data) const 
{
	assert(valid());
	assert(!status.empty());
	std::string buffer("HTTP/1.1 " + status + "\r\n");

	buffer += "Server: PeerConnectionTestServer/0.1\r\n"
		"Cache-Control: no-cache\r\n";

	if (connection_close)
		buffer += "Connection: close\r\n";

	if (!content_type.empty())
		buffer += "Content-Type: " + content_type + "\r\n";

	buffer += "Content-Length: " + int2str(static_cast<int>(data.size())) +
		"\r\n";

	if (!extra_headers.empty()) {
		buffer += extra_headers;
		// Extra headers are assumed to have a separator per header.
	}

	buffer += kCrossOriginAllowHeaders;

	buffer += "\r\n";
	buffer += data;

	return Send(buffer);
}

bool DataSocket::Send(const std::string& data) const 
{
	return send(socket_, data.data(), static_cast<int>(data.length()), 0) !=
		SOCKET_ERROR;
}