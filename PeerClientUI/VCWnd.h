#ifndef VCWND_H
#define VCWND_H
#pragma once
#include "QtWidgets\qmainwindow.h"
#include "QtGui\qpainter.h"
#include "UIcallbackInterface.h"
#include "QtCore\qpoint.h"
namespace render
{
	class UIcallbackInterface;
}
namespace render
{
	class VCWnd :
		public QMainWindow
	{
	public:
		explicit VCWnd(render::UIcallbackInterface* UI,bool islocal);
		~VCWnd();
	protected:
		void paintEvent(QPaintEvent* event);
		void closeEvent(QCloseEvent* event);
	private:
		void LocalFrameRenderer();
		void RemoteFrameRenderer();
		bool is_local_;
		static QPoint wnd_location_;
		render::UIcallbackInterface* UI_;
	};
}

#endif

