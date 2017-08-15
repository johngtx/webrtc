#ifndef __CVIDEORENDER_H
#define __CVIDEORENDER_H

#include <map>
#include <memory>
#include <string>

#include "webrtc/api/mediastreaminterface.h"
#include "webrtc/base/win32.h"
#include "webrtc/media/base/mediachannel.h"
#include "webrtc/media/base/videoframe.h"
#include "webrtc/media/base/videocommon.h"

class CVideoRenderer 
	: public rtc::VideoSinkInterface<cricket::VideoFrame>
{
public:
	CVideoRenderer(HWND hwnd, int width, int height, webrtc::VideoTrackInterface* track_to_render);
	virtual ~CVideoRenderer();

	inline void Lock() { ::EnterCriticalSection(&buffer_lock_); }
	inline void UnLock() { ::LeaveCriticalSection(&buffer_lock_); }

	//on frame event
	void OnFrame(const cricket::VideoFrame& frame) override;

	inline const BITMAPINFO& bmi() const { return bmi_; }
	inline uint8_t* image() const { return image_.get(); }

	long	old_process_;
	long	old_userdata_;

protected:
	void SetSize(int width, int height);
	enum {
		SET_SIZE,
		RENDER_FRAME
	};

	HWND hwnd_;
	BITMAPINFO bmi_;
	std::unique_ptr<uint8_t[]> image_;
	CRITICAL_SECTION buffer_lock_;
	rtc::scoped_refptr<webrtc::VideoTrackInterface> rendered_track_;

};

// A little helper class to make sure we always to proper locking and
// unlocking when working with VideoRenderer buffers.
template <typename T>
class AutoLock {
public:
	explicit AutoLock(T* obj) : obj_(obj) { obj_->Lock(); }
	~AutoLock() { obj_->UnLock(); }
protected:
	T* obj_;
};

#endif //__CVIDEORENDER_H