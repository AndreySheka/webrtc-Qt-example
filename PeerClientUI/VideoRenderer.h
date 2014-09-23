#ifndef VIDEORENDERER_H
#define VIDEORENDERER_H
#include "talk/base/scoped_ptr.h"
#include "talk/app/webrtc/mediastreaminterface.h"
#include "talk/app/webrtc/peerconnectioninterface.h"
#include "talk/app/webrtc/videosourceinterface.h"
#include "talk/media/base/mediachannel.h"
#include "talk/media/base/videocommon.h"
#include "talk/media/base/videoframe.h"
#include "talk/media/base/videorenderer.h"
#include "talk\media\devices\devicemanager.h"
#include "VCWnd.h"


// A little helper class to make sure we always to proper locking and
// unlocking when working with VideoRenderer buffers.
template <typename T>
class AutoLock {
public:
	explicit AutoLock(T* obj) : obj_(obj) { obj_->Lock(); }
	~AutoLock() { obj_->Unlock(); }
protected:
	T* obj_;
};

namespace render
{
	class VCWnd;
}

class VideoRenderer
	: public webrtc::VideoRendererInterface
{
public:
	explicit VideoRenderer(render::VCWnd *wnd, int width, int height,
		webrtc::VideoTrackInterface* track_to_render);
	virtual ~VideoRenderer();

	void Lock() { ::EnterCriticalSection(&buffer_lock_); }

	void Unlock() { ::LeaveCriticalSection(&buffer_lock_); }

	// VideoRendererInterface implementation
	virtual void SetSize(int width, int height);
	virtual void RenderFrame(const cricket::VideoFrame* frame);

	const BITMAPINFO& bmi() const { return bmi_; }
	const uint8* image() const { return image_.get(); }

protected:
	enum
	{
		SET_SIZE,
		RENDER_FRAME,
	};

	render::VCWnd *wnd_;
	BITMAPINFO bmi_;
	talk_base::scoped_ptr<uint8[]> image_;
	CRITICAL_SECTION buffer_lock_;
	talk_base::scoped_refptr<webrtc::VideoTrackInterface> rendered_track_;
};
#endif