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

#ifndef QDENGINE_QDCORE_UTIL_WINVIDEO_H
#define QDENGINE_QDCORE_UTIL_WINVIDEO_H

//#define __WINVIDEO_LOG__

namespace Video {
class MPEGPSDecoder;
}

namespace QDEngine {

class winVideo {
public:
	enum PlaybackStatus {
		VID_RUNNING,
		VID_STOPPED,
		VID_PAUSED
	};

	winVideo();
	~winVideo();

	static bool init(); // initialize DirectShow Lib
	static bool done(); // uninitialize DirectShow Lib

	//! Установка громкости звука, параметр - в децибелах [-10000, 0].
	bool set_volume(int volume_db);

	void set_window(void *hwnd, int x = 0, int y = 0, int xsize = 0, int ysize = 0);
	bool open_file(const char *fname);
	void close_file();

	bool play();
	bool stop();
	bool wait_end();

	PlaybackStatus playback_status();
	bool is_playback_finished();

	bool toggle_cursor(bool visible = false);

	bool get_movie_size(int &sx, int &sy);

	bool set_window_size(int sx, int sy);
	bool toggle_fullscreen(bool fullscr = true);

private:
	struct IGraphBuilder   *graph_builder_;
	struct IMediaControl   *media_control_;
	struct IVideoWindow    *video_window_;
	struct IMediaEvent *media_event_;
	struct IBasicAudio *basic_audio_;
	struct IBasicVideo *basic_video_;

	// Video decoder
	Video::MPEGPSDecoder *_decoder;
	Common::SeekableReadStream *_videostream;

	void *hwnd_;

#ifdef __WINVIDEO_LOG__
	static void *log_file_handle_;
#endif

	static bool is_initialized;
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_UTIL_WINVIDEO_H

