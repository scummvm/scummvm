#ifndef __WINVIDEO_H__
#define __WINVIDEO_H__

//#define __WINVIDEO_LOG__

class winVideo
{
public:
	enum PlaybackStatus {
		VID_RUNNING,
		VID_STOPPED,
		VID_PAUSED
	};

	winVideo();
	~winVideo();

	static bool init();	// initialize DirectShow Lib
	static bool done();	// uninitialize DirectShow Lib

	//! Установка громкости звука, параметр - в децибелах [-10000, 0].
	bool set_volume(int volume_db);

	void set_window(void* hwnd,int x = 0,int y = 0,int xsize = 0,int ysize = 0);
	bool open_file(const char* fname);
	void close_file();

	bool play();
	bool stop();
	bool wait_end();

	PlaybackStatus playback_status();
	bool is_playback_finished();

	bool toggle_cursor(bool visible = false);

	bool get_movie_size(int& sx,int& sy);

	bool set_window_size(int sx,int sy);
	bool toggle_fullscreen(bool fullscr = true);

private:
	struct IGraphBuilder*	graph_builder_;
	struct IMediaControl*	media_control_;
	struct IVideoWindow*	video_window_;
	struct IMediaEvent*	media_event_;
	struct IBasicAudio*	basic_audio_;
	struct IBasicVideo*	basic_video_;

	void* hwnd_;

#ifdef __WINVIDEO_LOG__
	static void* log_file_handle_;
#endif

	static bool is_initialized;
};

#endif /* __WINVIDEO_H__ */