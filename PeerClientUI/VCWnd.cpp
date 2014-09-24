#include "VCWnd.h"
#include <assert.h>
QPoint render::VCWnd::wnd_location_ = QPoint(20, 20);
render::VCWnd::VCWnd(UIcallbackInterface* UI, bool islocal)
	:UI_(UI),
	is_local_(islocal)
{
	if (is_local_)
		setWindowTitle("Local Camera");
	else
		setWindowTitle("Remote Camera");
	assert(UI_);
	resize(600, 400);
	setMinimumSize(QSize(600, 400));
	setMaximumSize(QSize(600, 400));
	move(wnd_location_);
	wnd_location_ += QPoint(20,20);
}


render::VCWnd::~VCWnd()
{
	if (!is_local_)
		UI_->StopRemoteRenderer();
	else
		UI_->StopLocalRenderer();
	if (this == UI_->own_wnd_&&UI_->peer_wnd_!=NULL)
	{
		UI_->own_wnd_ = NULL;
		delete UI_->peer_wnd_;
		UI_->peer_wnd_ = NULL;
		
		
	}
	if (this == UI_->peer_wnd_&& UI_->own_wnd_ != NULL)
	{
		UI_->peer_wnd_ = NULL;
		delete UI_->own_wnd_;
		UI_->own_wnd_ = NULL;
		
	}
	wnd_location_ = QPoint(20, 20);
}

void render::VCWnd::closeEvent(QCloseEvent* event)
{
}

void render::VCWnd::paintEvent(QPaintEvent* event)
{
	if (!is_local_)
		RemoteFrameRenderer();
	else
		LocalFrameRenderer();
}

void render::VCWnd::LocalFrameRenderer()
{
	VideoRenderer* local_renderer = UI_->local_renderer_.get();
	if (local_renderer)
	{
		AutoLock<VideoRenderer> local_lock(local_renderer);
		const BITMAPINFO& bmi = local_renderer->bmi();
		int height = abs(bmi.bmiHeader.biHeight);
		int width = bmi.bmiHeader.biWidth;

		const uint8* image = local_renderer->image();
		if (image != NULL)
		{
			QImage *local_video = new QImage(image, bmi.bmiHeader.biWidth, abs(bmi.bmiHeader.biHeight), QImage::Format_RGB32);
			QPainter painter(this);
			QRect rect = this->rect();
			painter.drawImage(rect, *local_video);
		}
		else
		{
			QImage img(600, 400, QImage::Format_RGB32);
			img.fill(QColor(0, 0, 0, 255));
			QPainter painter(this);
			painter.drawImage(this->rect(), img);
		}
	}
}

void render::VCWnd::RemoteFrameRenderer()
{
	VideoRenderer* remote_renderer = UI_->remote_renderer_.get();
	if (remote_renderer)
	{
		AutoLock<VideoRenderer> remote_lock(remote_renderer);
		const BITMAPINFO& bmi = remote_renderer->bmi();
		int height = abs(bmi.bmiHeader.biHeight);
		int width = bmi.bmiHeader.biWidth;

		const uint8* image = remote_renderer->image();
		if (image != NULL)
		{
			QImage *remote_video = new QImage(image, bmi.bmiHeader.biWidth, abs(bmi.bmiHeader.biHeight), QImage::Format_RGB32);
			QPainter painter(this);
			QRect rect = this->rect();
			painter.drawImage(rect, *remote_video);
		}
		else
		{
			QImage img(600, 400, QImage::Format_RGB32);
			img.fill(QColor(0, 0, 0, 255));
			QPainter painter(this);
			painter.drawImage(this->rect(), img);
		}
	}
}