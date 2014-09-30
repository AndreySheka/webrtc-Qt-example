#ifndef VCWND_H
#define VCWND_H
#pragma once
#include "QtWidgets\qmainwindow.h"
#include "QtGui\qpainter.h"
#include "UiObserver.h"
#include "QtCore\qpoint.h"
namespace render
{
	class UiObserver;
}
namespace render
{
	class VCWnd :
		public QMainWindow
	{
	public:
		explicit VCWnd(render::UiObserver* UI, bool islocal);
		~VCWnd();
	protected:
		void paintEvent(QPaintEvent* event);
		void closeEvent(QCloseEvent* event);
	private:
		void LocalFrameRenderer();
		void RemoteFrameRenderer();
		bool is_local_;
		static QPoint wnd_location_;
		render::UiObserver* UI_;
	};
}

#endif

