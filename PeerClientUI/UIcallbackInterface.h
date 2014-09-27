#ifndef UICALLBACKINTERFACE_H
#define UICALLBACKINTERFACE_H
#pragma once
namespace render
{
	class VCWnd;
}

#include <QtWidgets/QMainWindow>
#include <QtCore\qobject.h>
#include "ui_peerclientui.h"
#include "json/json.h"
#include "VideoRenderer.h"
#include <QtCore\qelapsedtimer.h>
class VideoRenderer;
namespace render
{
	class UIcallbackInterface
	{
	public:
		enum PeerStatus
		{
			NOT_CONNECTED,
			CONNECTED
		};
		enum LogType
		{
			NORMAL,
			WARNING,
			ERRORS
		};
		virtual ~UIcallbackInterface(){}
		virtual void StartLocalRenderer(webrtc::VideoTrackInterface* local_video) = 0;
		virtual void StopLocalRenderer() = 0;
		virtual void StartRemoteRenderer(webrtc::VideoTrackInterface* remote_video) = 0;
		virtual void StopRemoteRenderer() = 0;
		virtual void RefreshPeerList(Json::Value) = 0;
		virtual void SetUIstatus(PeerStatus status) = 0;
		virtual void log(LogType type, QString* log) = 0;
		virtual void OnTalk() = 0;
		virtual void timerEvent(QTimerEvent *event) = 0;
		virtual void msgbox(LogType type, QString* log) = 0;
		webrtc::VideoTrackInterface* remote_video;
		QString peer_name_;
		bool on_addstream_;
		render::VCWnd* own_wnd_;
		render::VCWnd* peer_wnd_;
		Ui::PeerClientUIClass ui;
		std::deque<Json::Value> pending_messages_;
		PeerStatus peer_state_;
		int peer_id_;
		bool is_pending_messages_;
		talk_base::scoped_ptr<VideoRenderer> local_renderer_;
		talk_base::scoped_ptr<VideoRenderer> remote_renderer_;
	};
}

#endif