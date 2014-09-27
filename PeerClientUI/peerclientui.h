#ifndef PEERCLIENTUI_H
#define PEERCLIENTUI_H

#include <QtWidgets/QMainWindow>
#include <QtCore\qstringlistmodel.h>
#include <QtCore\qelapsedtimer.h>
#include <QtCore\qstring.h>
#include <qtimezone.h>
#include <thread>
#include <regex>
#include <string>
#include <deque>
#include <thread>
#include "ui_peerclientui.h"
#include  "PeerConductor.h"
#include "VCWnd.h"

namespace render
{
	class VCWnd;
	class UIcallbackInterface;
}


class PeerClientUI : 
	public QMainWindow,
	public render::UIcallbackInterface

{
	Q_OBJECT

public:

	PeerClientUI(QWidget *parent = 0);
	~PeerClientUI();
	
	void SetUIstatus(PeerStatus status);
	void InitializeClient();
	virtual void StartLocalRenderer(webrtc::VideoTrackInterface* local_video);
	virtual void StopLocalRenderer();
	virtual void StartRemoteRenderer(webrtc::VideoTrackInterface* remote_video);
	virtual void StopRemoteRenderer();
	virtual void RefreshPeerList(Json::Value peers);
	virtual void closeEvent(QCloseEvent* event);
	virtual void timerEvent(QTimerEvent *event);
	virtual void log(LogType type, QString* log);
	virtual void msgbox(LogType type, QString* log);
private:
	PeerConnectionClient* client_;
	talk_base::scoped_refptr<PeerConductor> conductor_;
	std::string server_;
	int port_;
	bool is_connected;
	QStringList peers_;
	QStringListModel* model_;
	QElapsedTimer timer;
	int timer_id_;
	int pending_timer_;
	
private slots:
	void OnConnect();
	void OnDisconnect();
	void OnTalk();
	void OnClear();
	void OnListClicked(const QModelIndex &index);
};

#endif // PEERCLIENTUI_H
