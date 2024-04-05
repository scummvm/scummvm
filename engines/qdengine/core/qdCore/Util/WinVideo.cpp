/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include <amstream.h>
#include <control.h>
#include <uuids.h>
#include <evcode.h>

#include "WinVideo.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

bool winVideo::is_initialized = false;

#ifdef __WINVIDEO_LOG__
void* winVideo::log_file_handle_ = NULL;
#endif

winVideo::winVideo() : graph_builder_(NULL),
	media_control_(NULL),
	video_window_(NULL),
	media_event_(NULL),
	basic_video_(NULL),
	basic_audio_(NULL),
	hwnd_(NULL)
{ 
}

winVideo::~winVideo()
{
	close_file();
}

bool winVideo::init()
{
	if(is_initialized) return false;

	CoInitialize(NULL);
	is_initialized = true;

#ifdef __WINVIDEO_LOG__
	log_file_handle_ = CreateFile("qd_video.log",GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,NULL);
#endif

	return true;
}

bool winVideo::done()
{
	if(!is_initialized) return false;

#ifdef __WINVIDEO_LOG__
	CloseHandle((HANDLE)log_file_handle_);
#endif

	CoUninitialize();
	is_initialized = false;

	return true;
}

bool winVideo::set_volume(int volume_db)
{
	if(basic_audio_){
		basic_audio_ -> put_Volume(volume_db);
		return true;
	}

	return false;
}

void winVideo::set_window(void* hwnd,int x,int y,int xsize,int ysize)
{
	winVideo::hwnd_ = hwnd;
	if(video_window_ && hwnd){ // Set the video window
		video_window_ -> put_Owner((OAHWND)hwnd);
		video_window_ -> put_WindowStyle(WS_CHILD | WS_CLIPSIBLINGS);
//		video_window_ -> put_WindowStyleEx(WS_EX_TOPMOST);

		RECT grc = { x, y, xsize, ysize };
		if(!grc.right || !grc.bottom) GetClientRect((HWND)hwnd, &grc);

		video_window_ -> SetWindowPosition(grc.left,grc.top,grc.right,grc.bottom);
		toggle_cursor(false);
	}
}

bool winVideo::open_file(const char *fname)
{
	close_file();

	CoCreateInstance(CLSID_FilterGraph,NULL,CLSCTX_INPROC,IID_IGraphBuilder,(void**)&graph_builder_);
	if(!graph_builder_){
		close_file();
		return false;
	}

#ifdef __WINVIDEO_LOG__
	graph_builder_ -> SetLogFile((DWORD)log_file_handle_);
#endif

	graph_builder_ -> QueryInterface(IID_IMediaControl,(void**)&media_control_);
	if(!media_control_){
		close_file();
		return false;
	}

	graph_builder_-> QueryInterface(IID_IVideoWindow,(void**)&video_window_);
	if(!video_window_){
		close_file();
		return false;
	}

	graph_builder_-> QueryInterface(IID_IMediaEvent,(void**)&media_event_);
	if(!media_event_){
		close_file();
		return false;
	}

	graph_builder_-> QueryInterface(IID_IBasicVideo,(void**)&basic_video_);
	if(!basic_video_){ 
		close_file();
		return false;
	}

	graph_builder_-> QueryInterface(IID_IBasicAudio,(void**)&basic_audio_);
	if(!basic_audio_){ 
		close_file();
		return false;
	}

	WCHAR wPath[MAX_PATH];
	MultiByteToWideChar(CP_ACP,0,fname,-1,wPath,MAX_PATH);

	if(graph_builder_ -> RenderFile(wPath,NULL)) { close_file(); return false; }
	if(hwnd_) set_window(hwnd_);

	return true;
}

bool winVideo::play()
{
	if(graph_builder_ && video_window_ && media_control_ && hwnd_){
		video_window_ -> put_AutoShow(-1);
		video_window_ -> put_Visible(-1);
		video_window_ -> SetWindowForeground(-1);
		media_control_ -> Run();

		return true;
	}
	return false;
}

bool winVideo::stop()
{
	if(graph_builder_ && video_window_ && media_control_ && hwnd_){
		media_control_ -> Stop();
		return true;
	}

	return false;
}

winVideo::PlaybackStatus winVideo::playback_status()
{
	if(graph_builder_ && video_window_ && media_control_ && hwnd_){
		OAFilterState pfs;

		if(media_control_ -> GetState(INFINITE,&pfs) != S_OK) 
			return VID_STOPPED;

		if(pfs == State_Running) return VID_RUNNING;
		if(pfs == State_Stopped) return VID_STOPPED;
		if(pfs == State_Paused) return VID_PAUSED;
	}

	return VID_STOPPED;
}

bool winVideo::wait_end()
{
	if(media_event_){
		long evCode;
		media_event_ -> WaitForCompletion(INFINITE,&evCode);
		return true;
	}
	return false;
}

bool winVideo::is_playback_finished()
{
	if(!media_event_) return true;

	long evCode,param1,param2;
	while(media_event_ -> GetEvent(&evCode,&param1,&param2,0) == S_OK){
		media_event_ -> FreeEventParams(evCode,param1,param2);
		if((EC_COMPLETE == evCode) || (EC_USERABORT == evCode))
			return true;
	}

	return false;
}

bool winVideo::toggle_fullscreen(bool fullscr)
{
	if(video_window_){
		video_window_ -> put_FullScreenMode(fullscr);
		return true;
	}

	return false;
}

bool winVideo::toggle_cursor(bool visible)
{
	if(!video_window_) return false;
	video_window_ -> HideCursor(visible);

	return true;
}

bool winVideo::get_movie_size(int& sx,int& sy)
{
/*	if(!video_window_) return false;

	long sx_,sy_;
	video_window_ -> get_Width(&sx_); 
	video_window_ -> get_Height(&sy_);

	sx = sx_;
	sy = sy_;
*/
	if(!basic_video_) return false;

	long width,height;
	if(basic_video_ -> GetVideoSize(&width,&height) == E_NOINTERFACE)
		return false;

	sx = width;
	sy = height;

	return true;
}

bool winVideo::set_window_size(int sx,int sy)
{
	if(!video_window_) return false;

	video_window_ -> put_Width(sx); 
	video_window_ -> put_Height(sy);

	return true;
}

void winVideo::close_file()
{
	if(video_window_){
		video_window_ -> put_Visible(0);
//		video_window_ -> put_Owner(NULL);
	}

	if(media_event_){
		media_event_ -> Release();
		media_event_ = NULL;
	}

	if(media_control_){
		media_control_ -> Release(); 
		media_control_ = NULL;
	}

	if(basic_video_){
		basic_video_ -> Release();
		basic_video_ = NULL;
	}

	if(basic_audio_){
		basic_audio_ -> Release();
		basic_audio_ = NULL;
	}

	if(video_window_){
		video_window_ -> Release(); 
		video_window_ = NULL;
	}

	if(graph_builder_){
		graph_builder_ -> Release(); 
		graph_builder_ = NULL;
	}

	hwnd_ = NULL;
}
