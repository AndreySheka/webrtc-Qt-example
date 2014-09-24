#include "peerclientui.h"
PeerClientUI::PeerClientUI(QWidget *parent)
	: QMainWindow(parent),
	is_connected(false),
	client_(NULL),
	conductor_(NULL)
	
{
	InitializeClient();
	peer_state_ = PeerStatus::NOT_CONNECTED;
	peer_id_ = -1;
	pending_timer_ = startTimer(3000);
	is_pending_messages_ = false;
	own_wnd_ = NULL;
	peer_wnd_ = NULL;
	on_addstream_ = false;
	ui.setupUi(this);
	connect(ui.ConnectButton, SIGNAL(clicked()), 
		this, SLOT(OnConnect()));
	connect(ui.disconnectButton, SIGNAL(clicked()),
		this, SLOT(OnDisconnect()));
	connect(ui.clearlogButton, SIGNAL(clicked()),
		this, SLOT(OnClear()));
	connect(ui.talkButton, SIGNAL(clicked()),
		this, SLOT(OnTalk()));
	connect(ui.peerlistView, SIGNAL(doubleClicked(const QModelIndex &)),
		this, SLOT(OnListClicked(const QModelIndex &)));
	ui.stackedWidget->setCurrentIndex(0);
	QRegExp regex("[1-9][0-9]{0,2}\\.([0-9]{1,3})\\.([0-9]{1,3})\\.([0-9]{1,3})");
	QRegExpValidator *regexv = new QRegExpValidator(regex,ui.page);
	ui.server_lineEdit->setValidator(regexv);
	QIntValidator *intv = new QIntValidator(1, 65535, ui.page);
	ui.port_lineEdit->setValidator(intv);
	model_ = new QStringListModel();
	ui.peerlistView->setModel(model_);
}

PeerClientUI::~PeerClientUI()
{
	delete model_;
}

void PeerClientUI::closeEvent(QCloseEvent* event)
{
	if (conductor_)
		conductor_->DeletePeerConnection();
	if (own_wnd_)
	{
		delete own_wnd_;
		own_wnd_ = NULL;
	}

	if (peer_wnd_)
	{
		delete peer_wnd_;
		peer_wnd_ = NULL;
	}
	peer_state_ = PeerStatus::NOT_CONNECTED;
}
void PeerClientUI::OnConnect()
{
	conductor_->OnStartLogin(ui.server_lineEdit->text().toStdString()
							,ui.port_lineEdit->text().toInt());
	is_connected = true;
}

void PeerClientUI::OnTalk()
{
	
	switch (peer_state_)
	{
	case PeerStatus::NOT_CONNECTED:
		{
			if (!ui.peerlistView->currentIndex().isValid())
			{
				log(ERRORS, new QString("please select a legal peer."));
			}
			else
			{
				std::string peer = peers_.at(ui.peerlistView->currentIndex().row()).toStdString();
				peer_name_ = peer.c_str();
				std::regex regex_peer(".*@.*@([0-9]*)");
				std::smatch sm;
				ASSERT(std::regex_search(peer, sm, regex_peer, std::regex_constants::match_any));
				std::string peer_id = sm[1];
				peer_id_ = atoi(peer_id.c_str());
				SetUIstatus(peer_state_);
				peer_state_ = PeerStatus::CONNECTED;
				pending_timer_ = startTimer(3000);
				conductor_->OnConnectToPeer(peer_id_);
			}
			break;
		}

	case PeerStatus::CONNECTED:
		{
			conductor_->HangUp();
			conductor_->OnDisconnectFromCurrentPeer();
			break;
		}
	}

}

void PeerClientUI::OnDisconnect()
{

	conductor_->OnDisconnectFromServer();
	ui.stackedWidget->setCurrentIndex(0);
	ui.ConnectButton->setEnabled(false);
	is_connected = false;
	timer_id_=startTimer(3000);
}

void PeerClientUI::OnListClicked(const QModelIndex &index)
{
	if (index.isValid())
		OnTalk();
}

void PeerClientUI::SetUIstatus(PeerStatus status)
{
	switch (status)
	{
		case PeerStatus::NOT_CONNECTED:
		{
			ui.disconnectButton->setEnabled(false);
			ui.peerlistView->setEnabled(false);
			ui.talkButton->setText("Hangup");
			break;
		}

		case PeerStatus::CONNECTED:
		{
			ui.disconnectButton->setEnabled(true);
			ui.peerlistView->setEnabled(true);
			ui.talkButton->setText("Talk");
			break;
		}
	}
}

void PeerClientUI::RefreshPeerList(Json::Value peer_list)
{
	peers_.clear();
	Json::Value jpeer;
	std::string peer;
	int peer_id;
	for (Json::Value::iterator i = peer_list.begin(); i != peer_list.end(); i++)
	{
		jpeer = *i;
		peer_id = jpeer["peer_id"].asInt();
		if (client_->my_id_ != peer_id)
		{
			peer.clear();
			peer = jpeer["peer_name"].asString() + "@" + jpeer["peer_id"].asString();
			peers_ += peer.c_str();
		}
	}
	model_->setStringList(peers_);
}

void PeerClientUI::StartLocalRenderer(webrtc::VideoTrackInterface* local_video)
{
	ASSERT(!own_wnd_);
	own_wnd_ = new render::VCWnd(this, true);
	own_wnd_->show();
	local_renderer_.reset(new VideoRenderer(own_wnd_, 1, 1, local_video));
	log(NORMAL, new QString("started local renderer."));
}

void PeerClientUI::StartRemoteRenderer(webrtc::VideoTrackInterface* remote_video)
{
	ASSERT(!peer_wnd_);
	peer_wnd_ = new render::VCWnd(this, false);
	peer_wnd_->show();
	remote_renderer_.reset(new VideoRenderer(peer_wnd_, 1, 1, remote_video));
	log(NORMAL, new QString("started remote renderer."));
}

void PeerClientUI::StopLocalRenderer()
{
	if (local_renderer_.get())
	{
		local_renderer_.reset();
		log(NORMAL, new QString("stopped local renderer."));
	}
	
}


void PeerClientUI::StopRemoteRenderer()
{
	if (remote_renderer_.get())
	{
		remote_renderer_.reset();
		log(NORMAL, new QString("stopped remote renderer."));
	}

}

void PeerClientUI::OnClear()
{
	ui.logtextEdit->clear();
}

void  PeerClientUI::timerEvent(QTimerEvent *event)
{
	if (!is_connected)
	{
		ui.ConnectButton->setEnabled(true);
	}
	if (on_addstream_)
	{
		StartRemoteRenderer(remote_video);
		on_addstream_ = false;
	}
	if (!is_pending_messages_&&!pending_messages_.empty())
	{
		is_pending_messages_ = true;
		Json::Value msg = pending_messages_.front();
		Json::FastWriter writer;
		std::string mm = writer.write(msg);
		pending_messages_.pop_front();
		conductor_->OnMessageSent(peer_id_,msg);
		Sleep(2000);
		is_pending_messages_ = false;
	}
		

}

void PeerClientUI::log(LogType type,QString* log)
{
	auto time=QTime::currentTime();
	*log+=" "+time.toString();
	switch (type)
	{
	case PeerClientUI::NORMAL:
		ui.logtextEdit->setTextColor(QColor(0, 0, 255));
		ui.logtextEdit->append(*log);
		break;
	case PeerClientUI::WARNING:
		ui.logtextEdit->setTextColor(QColor(254,149,7));
		ui.logtextEdit->append(*log);
		break;
	case PeerClientUI::ERRORS:
		ui.logtextEdit->setTextColor(QColor(255, 7, 7));
		ui.logtextEdit->append(*log);
		break;
	default:
		break;
	}
}

void PeerClientUI::InitializeClient() //to init peerclient and peer conductor
{
	ASSERT(!client_);
	ASSERT(!conductor_.get());
	client_ = new PeerConnectionClient(this);
	conductor_ = new talk_base::RefCountedObject<PeerConductor>(client_, this);
	client_->client_observer_ = conductor_.get();
}