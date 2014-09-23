#ifndef QASYSOCKET_H
#define QASYSOCKET_H
#include <synchapi.h>
#include <QtNetWork\QTcpSocket>
#include <thread>
class QAsySocket;

class SocketCallback
{
public:
	virtual void OnConnect(QAsySocket* socket) = 0;
	virtual void OnRead(QAsySocket* socket) = 0;
	virtual void OnClose(QAsySocket* socket, int err) = 0;
	~SocketCallback(){}
};

class QAsySocket
	:public QObject
{
	Q_OBJECT

public:
	explicit QAsySocket(SocketCallback *asycsocket);
	~QAsySocket();
	int Recv(char* buf,int cb)
	{
		return tcp_socket_->read(buf,cb); 
	}
	int Connect(std::string server_address, int port)
	{
		Sleep(300);
		tcp_socket_->close();
		tcp_socket_->connectToHost(server_address.c_str(), port);
		tcp_socket_->waitForConnected(300);
		return tcp_socket_->state(); 
	}
	QTcpSocket::SocketState GetState(){ return tcp_socket_->state(); }
	void Close(){ quit_ = true; Sleep(1000); tcp_socket_->disconnectFromHost(); }
	int Send(const char* pv, size_t cb)
	{ 
		int cbw=tcp_socket_->write(pv, cb);
		return cbw;
	}
	void CheckStatus();
	void RegisterObserver();
	void UnRegisterObserver();
	QTcpSocket* tcp_socket_;
	SocketCallback *asyc_socket_;
	bool quit_;
private slots :
	void OnreadyRead(){ asyc_socket_->OnRead(this); }
	void Onerror(QAbstractSocket::SocketError){};
	void Onconnected()
	{
		asyc_socket_->OnConnect(this); 
		std::thread thd(&QAsySocket::CheckStatus, this);
		thd.detach();
	}
	void Ondisconnected(){}
};

#endif // QASYSOCKET_H
