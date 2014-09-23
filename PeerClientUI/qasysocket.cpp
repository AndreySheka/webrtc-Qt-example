#include "qasysocket.h"
QAsySocket::QAsySocket(SocketCallback *asycsocket)
	:asyc_socket_(asycsocket),
	tcp_socket_(NULL),
	quit_(false)
{
}

QAsySocket::~QAsySocket()
{

}

void QAsySocket::CheckStatus()
{
	while (GetState()==QTcpSocket::SocketState::ConnectedState&&!quit_)
	{
		tcp_socket_->waitForBytesWritten(300);
		tcp_socket_->waitForReadyRead(300);
		Sleep(100);
		
	}
	std::this_thread::yield();
}

void QAsySocket::RegisterObserver()
{
	connect(tcp_socket_, SIGNAL(readyRead()), this, SLOT(OnreadyRead()));
	connect(tcp_socket_, SIGNAL(connected()), this, SLOT(Onconnected()));
	connect(tcp_socket_, SIGNAL(disconnected()), this, SLOT(Ondisconnected()));
	connect(tcp_socket_, SIGNAL(error()), this, SLOT(Onerror()));
	quit_ = false;
}

void QAsySocket::UnRegisterObserver()
{
	disconnect(tcp_socket_, SIGNAL(readyRead()), this, SLOT(OnreadyRead()));
	disconnect(tcp_socket_, SIGNAL(connected()), this, SLOT(Onconnected()));
	disconnect(tcp_socket_, SIGNAL(disconnected()), this, SLOT(Ondisconnected()));
	disconnect(tcp_socket_, SIGNAL(error()), this, SLOT(Onerror()));
}