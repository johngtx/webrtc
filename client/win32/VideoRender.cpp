#include "stdafx.h"
#include "VideoRender.h"

#include <math.h>
#include "webrtc/base/arraysize.h"
#include "webrtc/base/common.h"
#include "webrtc/base/logging.h"
#include "webrtc/base/basictypes.h"

LONG_PTR OldProcess;
typedef INT_PTR __stdcall CBPTR(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    RenderProcess(HWND, UINT, WPARAM, LPARAM);

CVideoRenderer::CVideoRenderer(HWND hwnd, int width, int height,
	webrtc::VideoTrackInterface* track_to_render)
	: hwnd_(hwnd)
	, rendered_track_(track_to_render)
{
	::InitializeCriticalSection(&buffer_lock_);
	ZeroMemory(&bmi_, sizeof(bmi_));

	bmi_.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi_.bmiHeader.biPlanes = 1;
	bmi_.bmiHeader.biBitCount = 32;
	bmi_.bmiHeader.biCompression = BI_RGB;
	bmi_.bmiHeader.biWidth = width;
	bmi_.bmiHeader.biHeight = -height;
	bmi_.bmiHeader.biSizeImage = width * height *
		(bmi_.bmiHeader.biBitCount >> 3);
	rendered_track_->AddOrUpdateSink(this, rtc::VideoSinkWants());

	//
	old_process_ = ::SetWindowLongPtr(hwnd, GWL_WNDPROC, (LONG_PTR)(&RenderProcess));
	old_userdata_ = ::SetWindowLongPtr(hwnd, GWL_USERDATA, (LONG_PTR)(this));
}

CVideoRenderer::~CVideoRenderer()
{
	rendered_track_->RemoveSink(this);
	::SetWindowLongPtr(hwnd_, GWL_WNDPROC, (LONG_PTR)old_process_);
	::SetWindowLongPtr(hwnd_, GWL_USERDATA, (LONG_PTR)old_userdata_);
	::DeleteCriticalSection(&buffer_lock_);
}

void CVideoRenderer::SetSize(int width, int height) 
{
	AutoLock<CVideoRenderer> lock(this);

	if (width == bmi_.bmiHeader.biWidth 
		&& height == bmi_.bmiHeader.biHeight) 
	{
		return;
	}

	bmi_.bmiHeader.biWidth = width;
	bmi_.bmiHeader.biHeight = -height;
	bmi_.bmiHeader.biSizeImage = width * height *
		(bmi_.bmiHeader.biBitCount >> 3);
	image_.reset(new uint8_t[bmi_.bmiHeader.biSizeImage]);
}

void CVideoRenderer::OnFrame( const cricket::VideoFrame& video_frame)
{
	{
		AutoLock<CVideoRenderer> lock(this);

		const cricket::VideoFrame* frame =
			video_frame.GetCopyWithRotationApplied();

		SetSize(frame->width(), frame->height());

		ASSERT(image_.get() != NULL);
		frame->ConvertToRgbBuffer(
			cricket::FOURCC_ARGB,
			image_.get(),
			bmi_.bmiHeader.biSizeImage,
			bmi_.bmiHeader.biWidth *
			bmi_.bmiHeader.biBitCount / 8);
	}
	::InvalidateRect(hwnd_, NULL, TRUE);
}

INT_PTR CALLBACK RenderProcess(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	LONG_PTR lp = ::GetWindowLongPtr(hwnd, GWL_USERDATA);
	void* ptr = (void*)lp;
	CVideoRenderer* render = static_cast<CVideoRenderer*>(ptr);

	switch (message)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		RECT rc;
		::GetClientRect(hwnd, &rc);
		::BeginPaint(hwnd, &ps);

		if (render != nullptr)
		{
			AutoLock<CVideoRenderer> local_lock(render);

			const BITMAPINFO& bmi = render->bmi();
			int height = abs(bmi.bmiHeader.biHeight);
			int width = bmi.bmiHeader.biWidth;

			const uint8_t* image = render->image();
			if (image != NULL)
			{
				HDC dc_mem = ::CreateCompatibleDC(ps.hdc);
				::SetStretchBltMode(dc_mem, HALFTONE);

				// Set the map mode so that the ratio will be maintained for us.
				HDC all_dc[] = { ps.hdc, dc_mem };
				for (int i = 0; i < arraysize(all_dc); ++i) {
					SetMapMode(all_dc[i], MM_ISOTROPIC);
					SetWindowExtEx(all_dc[i], width, height, NULL);
					SetViewportExtEx(all_dc[i], rc.right, rc.bottom, NULL);
				}

				HBITMAP bmp_mem = ::CreateCompatibleBitmap(ps.hdc, rc.right, rc.bottom);
				HGDIOBJ bmp_old = ::SelectObject(dc_mem, bmp_mem);

				POINT logical_area = { rc.right, rc.bottom };
				DPtoLP(ps.hdc, &logical_area, 1);

				HBRUSH brush = ::CreateSolidBrush(RGB(0, 0, 0));
				RECT logical_rect = { 0, 0, logical_area.x, logical_area.y };
				::FillRect(dc_mem, &logical_rect, brush);
				::DeleteObject(brush);

				int x = (logical_area.x / 2) - (width / 2);
				int y = (logical_area.y / 2) - (height / 2);

				StretchDIBits(dc_mem, x, y, width, height,
					0, 0, width, height, image, &bmi, DIB_RGB_COLORS, SRCCOPY);

				BitBlt(ps.hdc, 0, 0, logical_area.x, logical_area.y,
					dc_mem, 0, 0, SRCCOPY);

				// Cleanup.
				::SelectObject(dc_mem, bmp_old);
				::DeleteObject(bmp_mem);
				::DeleteDC(dc_mem);
			}
			else
			{
				HBRUSH brush = ::CreateSolidBrush(RGB(0, 0, 0));
				::FillRect(ps.hdc, &rc, brush);
				::DeleteObject(brush);
			}
		}
		::EndPaint(hwnd, &ps);
		return (INT_PTR)TRUE;
	}
	default:
		return ((CBPTR*)render->old_process_)(hwnd, message, wParam, lParam);
	}
}