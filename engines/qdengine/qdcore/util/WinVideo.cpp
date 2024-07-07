/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/file.h"
#include "video/mpegps_decoder.h"

#include "qdengine/qd_precomp.h"
#include "qdengine/qdengine.h"
#include "qdengine/qdcore/util/WinVideo.h"

#include "qdengine/system/graphics/gr_dispatcher.h"


namespace QDEngine {

bool winVideo::is_initialized = false;

#ifdef __WINVIDEO_LOG__
void *winVideo::log_file_handle_ = NULL;
#endif

winVideo::winVideo() : graph_builder_(NULL),
	media_control_(NULL),
	video_window_(NULL),
	media_event_(NULL),
	basic_video_(NULL),
	basic_audio_(NULL),
	hwnd_(NULL) {
	_decoder = new Video::MPEGPSDecoder();
}

winVideo::~winVideo() {
	close_file();
	delete _decoder;
}

bool winVideo::init() {
	return true;
}

bool winVideo::done() {
	if (!is_initialized) return false;

#ifdef __WINVIDEO_LOG__
	CloseHandle((HANDLE)log_file_handle_);
#endif

	CoUninitialize();
	is_initialized = false;

	return true;
}

bool winVideo::set_volume(int volume_db) {
	warning("STUB: winVideo::set_volume");
#if 0
	if (basic_audio_) {
		basic_audio_->put_Volume(volume_db);
		return true;
	}
#endif
	return false;
}

void winVideo::set_window(void *hwnd, int x, int y, int xsize, int ysize) {
	warning("STUB: winVideo::set_window");

	winVideo::hwnd_ = hwnd;
	if (video_window_ && hwnd) { // Set the video window
#if 0
		video_window_->put_Owner((OAHWND)hwnd);
		video_window_->put_WindowStyle(WS_CHILD | WS_CLIPSIBLINGS);
//		video_window_->put_WindowStyleEx(WS_EX_TOPMOST);

		RECT grc = { x, y, xsize, ysize };
		if (!grc.right || !grc.bottom) GetClientRect((HWND)hwnd, &grc);
		video_window_->SetWindowPosition(grc.left, grc.top, grc.right, grc.bottom);
#endif
		toggle_cursor(false);
	}
}

bool winVideo::open_file(const char *fname) {
	Common::File *videoFile = new Common::File();

	if (!videoFile->open(fname)) {
		warning("WinVideo::open: Failed to open file %s", fname);
		delete videoFile;
		return false;
	}

	_videostream = videoFile;

	if (!_decoder->loadStream(_videostream)) {
		warning("WinVideo::play: Failed to Load Stream");
		delete videoFile;
		return false;
	}

	return true;
}

bool winVideo::play() {
	if (!_videostream) {
		warning("WinVideo::play: No video stream loaded");
		return false;
	}

	// Calculating the coordinates to center the video
	int32 x = 0, y = 0;
	int32 videoWidth = _decoder->getWidth(), videoHeight = _decoder->getHeight();

	int screenWidth = grDispatcher::instance()->Get_SizeX();
	int screenHeight = grDispatcher::instance()->Get_SizeY();

	x = (screenWidth - videoWidth) / 2;
	y = (screenHeight - videoHeight) / 2;

	if (x < 0) {
		x = 0;
		videoWidth = screenWidth;
	}

	if (y < 0) {
		y = 0;
		videoHeight = screenHeight;
	}

	// Video Playback loop
	debugC(9, kDebugGraphics, "WinVideo::play: Video Playback loop");

	_decoder->start();
	if (_decoder->needsUpdate()) {
		const Graphics::Surface *frame = _decoder->decodeNextFrame();
		if (frame) {
			g_system->copyRectToScreen(frame->getPixels(), frame->pitch, x, y, videoWidth, videoHeight);
		}
		g_system->updateScreen();
	}


	return true;
}

bool winVideo::stop() {
	warning("STUB: winVideo::stop()");
	return true;
}

winVideo::PlaybackStatus winVideo::playback_status() {
	if (graph_builder_ && video_window_ && media_control_ && hwnd_) {
		warning("STUB: winVideo::playback_status");
#if 0
		OAFilterState pfs;

		if (media_control_->GetState(INFINITE, &pfs) != S_OK)
			return VID_STOPPED;
		if (pfs == State_Running) return VID_RUNNING;
		if (pfs == State_Stopped) return VID_STOPPED;
		if (pfs == State_Paused) return VID_PAUSED;
#endif
	}

	return VID_STOPPED;
}

bool winVideo::wait_end() {
	warning("STUB: winVideo::wait_end()");

	if (media_event_) {
#if 0
		long evCode;
		media_event_->WaitForCompletion(INFINITE, &evCode);
#endif
		return true;
	}
	return false;
}

bool winVideo::is_playback_finished() {
	warning("STUB: winVideo::is_playback_finished %d", _decoder->endOfVideo());
	return _decoder->endOfVideo();
}

bool winVideo::toggle_fullscreen(bool fullscr) {
	warning("STUB: winVideo::toggle_fullscreen()");

	if (video_window_) {
#if 0
		video_window_->put_FullScreenMode(fullscr);
#endif
		return true;
	}

	return false;
}

bool winVideo::toggle_cursor(bool visible) {
	warning("STUB: winVideo::toggle_cursor()");

	if (!video_window_)
		return false;
#if 0
	video_window_->HideCursor(visible);
#endif
	return true;
}

bool winVideo::get_movie_size(int &sx, int &sy) {
	/*  if(!video_window_) return false;

	    long sx_,sy_;
	    video_window_->get_Width(&sx_);
	    video_window_->get_Height(&sy_);

	    sx = sx_;
	    sy = sy_;
	*/
	warning("STUB: winVideo::get_movie_size()");

	if (!basic_video_) return false;

#if 0
	long width, height;
	if (basic_video_->GetVideoSize(&width, &height) == E_NOINTERFACE)
		return false;
	sx = width;
	sy = height;

#endif

	return true;
}

bool winVideo::set_window_size(int sx, int sy) {
	warning("STUB: winVideo::set_window_size()");

	if (!video_window_) return false;
#if 0
	video_window_->put_Width(sx);
	video_window_->put_Height(sy);
#endif
	return true;
}

void winVideo::close_file() {
	warning("STUB: winVideo::close_file()");
	return;
}

} // namespace QDEngine

