/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include <locale.h>

#include "resource.h"

#include "gdi_gr_dispatcher.h"
#include "ddraw_gr_dispatcher.h"
#include "app_core.h"
#include "app_error_handler.h"
#include "qd_game_dispatcher.h"
#include "qd_game_scene.h"
#include "qd_trigger_chain.h"
#include "qd_dialogs.h"
#include "qd_setup.h"
#include "ds_snd_dispatcher.h"
#include "WinVideo.h"
#include "qd_trigger_profiler.h"
#include "qd_file_manager.h"
#include "plaympp_api.h"

#include "input_wndproc.h"
#include "input_recorder.h"
#include "mouse_input.h"
#include "keyboard_input.h"

#include "qd_dialogs.h"
#include "splash_screen.h"

#include "ResourceDispatcher.h"

#include "comline_parser.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */

namespace qdrt {

void init_graphics();
bool init_graphics_dispatcher();
bool is_graphics_reinit_needed();
void restore_graphics();

void toggle_fullscreen(bool force_fullscreen = false);
void maximize_window(){ toggle_fullscreen(true); }

void qd_show_load_progress(int percents_loaded,void* p);

void restore();

bool request_CD_handler(int cd_id);

/* --------------------------- DEFINITION SECTION --------------------------- */

//! Командная строка
enum {
	COMLINE_SCENE_NAME,
	COMLINE_ENABLE_LOG,
	COMLINE_SETTINGS,
	COMLINE_RECORDER_WRITE,
	COMLINE_RECORDER_PLAY,
	COMLINE_TRIGGERS_DEBUG,
	COMLINE_TRIGGERS_PROFILER
};

GDI_grDispatcher* gdi_grD = NULL;
DDraw_grDispatcher* dd_grD = NULL;

HWND hmainWnd;

qdGameDispatcher* qd_gameD = NULL;

}; // namespace qdrt

using namespace qdrt;

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpszCmdLine, int nCmdShow)
{
	const char* const event_name = "QD Engine Game";
	if(HANDLE event = OpenEvent(EVENT_ALL_ACCESS,FALSE,event_name)){
		if(HWND hwnd = FindWindow(grDispatcher::wnd_class_name(),NULL)){
			ShowWindow(hwnd,SW_RESTORE);
			BringWindowToTop(hwnd);
			SetForegroundWindow(hwnd);
		}
		return 0;
	}
	else
		event = CreateEvent(0,TRUE,TRUE,event_name);

	ErrH.SetRestore(restore);

	comlineParser comline_parser;
	comline_parser.register_option("s",COMLINE_SCENE_NAME);
	comline_parser.register_option("log",COMLINE_ENABLE_LOG);
	comline_parser.register_option("settings",COMLINE_SETTINGS);

	comline_parser.register_option(inputRecorder::write_comline(),COMLINE_RECORDER_WRITE);
	comline_parser.register_option(inputRecorder::play_comline(),COMLINE_RECORDER_PLAY);

#ifdef __QD_DEBUG_ENABLE__
	comline_parser.register_option(qdTriggerChain::debug_comline(),COMLINE_TRIGGERS_DEBUG);
#endif

#ifdef __QD_TRIGGER_PROFILER__
	comline_parser.register_option(qdTriggerProfiler::activation_comline(),COMLINE_TRIGGERS_PROFILER);
#endif

	comline_parser.parse_comline(__argc,__argv);

	std::string script_name;
	if(comline_parser.has_argument(-1))
		script_name = comline_parser.argument_string(-1);

	if(script_name.empty()){
		if(char* p = XFindFirst("*.qml"))
			script_name = p;

		if(script_name.empty()) return 0;
	}

	app_io::set_current_directory(script_name.c_str());

	grDispatcher::sys_init();

	gdi_grD = new GDI_grDispatcher;
	dd_grD = new DDraw_grDispatcher;

	if(comline_parser.has_argument(COMLINE_SETTINGS)){
		qdlg::settings_dialog();

		delete gdi_grD;
		delete dd_grD;

		grDispatcher::sys_finit();
		return 0;
	}

#ifdef __QD_DEBUG_ENABLE__
	if(comline_parser.has_argument(COMLINE_TRIGGERS_DEBUG))
		qdGameConfig::get_config().toggle_triggers_debug(true);
#endif

#ifdef __QD_TRIGGER_PROFILER__
	if(comline_parser.has_argument(COMLINE_TRIGGERS_PROFILER))
		qdTriggerProfiler::instance().enable();
#endif
	
	if(const char* rec_name = comline_parser.argument_string(COMLINE_RECORDER_WRITE))
		inputRecorder::instance().open(rec_name,inputRecorder::RECORDER_WRITE);
	else if(rec_name = comline_parser.argument_string(COMLINE_RECORDER_PLAY))
		inputRecorder::instance().open(rec_name,inputRecorder::RECORDER_PLAY);

	gdi_grD -> set_maximize_handler(maximize_window);
	dd_grD -> set_maximize_handler(maximize_window);

	gdi_grD -> HideMouse();
	dd_grD -> HideMouse();

	qdGameConfig::get_config().load();
	setlocale(LC_CTYPE, qdGameConfig::get_config().locale());
									 
#ifdef __QD_TRIGGER_PROFILER__
	if(qdGameConfig::get_config().is_profiler_enabled())
		qdTriggerProfiler::instance().enable();
	if (0 != strlen(qdGameConfig::get_config().profiler_file()))
		qdTriggerProfiler::instance().set_work_file(qdGameConfig::get_config().profiler_file());
#endif	

	if(qdGameConfig::get_config().driver_ID()){
		if(!qdGameConfig::get_config().fullscreen()){
			int sx,sy;
			grPixelFormat pixel_format;
			dd_grD -> get_current_mode(sx,sy,pixel_format);

			qdGameConfig::get_config().set_pixel_format(pixel_format);
		}
		else 
			qdGameConfig::get_config().set_pixel_format(dd_grD -> adjust_mode((grPixelFormat)qdGameConfig::get_config().pixel_format()));
	}

	SplashScreen sp;
	if(qdGameConfig::get_config().is_splash_enabled()){
		sp.create(IDB_SPLASH);
		sp.set_mask(IDB_SPLASH_MASK);
		sp.show();
	}

	SetErrorMode(SEM_FAILCRITICALERRORS);

//	qdFileManager::instance().check_drive('E');
	qdFileManager::instance().set_request_CD_handler(request_CD_handler);

	for(int i = 1; i < __argc; i ++)
		appLog::default_log() << " \"" << __argv[i] << "\"";
	appLog::default_log() << "\r\n";

	grDispatcher::set_default_font(qdGameDispatcher::create_font(0));

	qd_gameD = new qdGameDispatcher;
	qd_gameD -> load_script(app_io::strip_path(script_name.c_str()));

	qd_gameD -> set_scene_loading_progress_callback(qd_show_load_progress);

	if(qdGameConfig::get_config().is_splash_enabled()){
		sp.wait(qdGameConfig::get_config().splash_time());
		sp.destroy();
	}

	init_graphics();

	ds_sndDispatcher* sndD = new ds_sndDispatcher;

	qdGameConfig::get_config().update_sound_settings();
	qdGameConfig::get_config().update_music_settings();

	winVideo::init();

	qd_gameD -> load_resources();
	
	if(const char* scene_name = comline_parser.argument_string(COMLINE_SCENE_NAME)){
		if(!qd_gameD -> select_scene(scene_name)) app_errH.show_error("Стартовая сцена не найдена",appErrorHandler::ERR_OTHER);
	}
	else {
		bool music_enabled = mpegPlayer::instance().is_enabled();
		mpegPlayer::instance().disable();

		qd_gameD -> toggle_main_menu(true);
		if(!qd_gameD -> start_intro_videos()){
			if(music_enabled)
				mpegPlayer::instance().enable(true);
		}
		else {
			if(music_enabled)
				mpegPlayer::instance().enable(false);
		}
	}

	qd_gameD -> update_time();
	qd_gameD -> quant();

	ResourceDispatcher resD;
	resD.setTimer(qdGameConfig::get_config().logic_synchro_by_clock(),qdGameConfig::get_config().logic_period(),300);
	resD.attach(new MemberFunctionCallResourceUser<qdGameDispatcher>(*qd_gameD,qdGameDispatcher::quant,qdGameConfig::get_config().logic_period()));
	sndD -> set_frequency_coeff(qdGameConfig::get_config().game_speed());
	resD.set_speed(qdGameConfig::get_config().game_speed());
	resD.start();

	MSG msg;
	bool exit_flag = false;
	bool was_inactive = false;

	while(!exit_flag && !qd_gameD -> need_exit()){
		while(PeekMessage(&msg,NULL,0,0,PM_REMOVE)){
			switch(msg.message){
				case WM_QUIT:
					if(!grDispatcher::instance() -> is_in_reinit_mode())
						exit_flag = true;
					break;
				case WM_SYSCOMMAND:
					if(msg.wParam == SC_MAXIMIZE)
						toggle_fullscreen(true);
					break;
				case WM_SYSKEYDOWN:
					if(msg.wParam == VK_RETURN && msg.lParam & (1 << 29))
						toggle_fullscreen();
					break;
				case WM_KEYDOWN:
					if(msg.wParam == 'F')
						qdGameConfig::get_config().toggle_fps();
#ifdef __QD_DEBUG_ENABLE__
					else if(msg.wParam == VK_NEXT){
						float speed = qdGameConfig::get_config().game_speed() * 0.9f;
						if(speed < 0.1f) speed = 0.1f;
						qdGameConfig::get_config().set_game_speed(speed);
						sndD -> set_frequency_coeff(speed);
						resD.set_speed(qdGameConfig::get_config().game_speed());
					}
					else if(msg.wParam == VK_PRIOR){
						float speed = qdGameConfig::get_config().game_speed() * 1.1f;
						if(speed > 10.0f) speed = 10.0f;
						qdGameConfig::get_config().set_game_speed(speed);
						sndD -> set_frequency_coeff(speed);
						resD.set_speed(qdGameConfig::get_config().game_speed());
					}
					else if(msg.wParam == VK_HOME){
						qdGameConfig::get_config().set_game_speed(1.0f);
						sndD -> set_frequency_coeff(1.0f);
						resD.set_speed(qdGameConfig::get_config().game_speed());
					}
					else if(msg.wParam == 'G')
						qdGameConfig::get_config().toggle_show_grid();
#endif

					break;
			}

			input::keyboard_wndproc(msg,keyboardDispatcher::instance());
			input::mouse_wndproc(msg,mouseDispatcher::instance());

			TranslateMessage(&msg);
			DispatchMessage(&msg); 
		}

		if(grDispatcher::is_active()){
			if (was_inactive)
			{
				was_inactive = false;
				// При активации ждем, чтобы звукововая система успела настроиться
				// на наше приложение (предположение)
				Sleep(500); 
			}
			resD.quant();
			qd_gameD -> redraw();
		
		}
		else {
			was_inactive = true;
			Sleep(100);
			resD.skip_time();
		}
	}

	delete qd_gameD;

	grDispatcher::instance() -> Finit();

	ShowWindow(hmainWnd,SW_HIDE);
	CloseWindow(hmainWnd);
	DestroyWindow(hmainWnd);

	delete sndD;
	delete gdi_grD;
	delete dd_grD;

	winVideo::done();

	grDispatcher::sys_finit();

	return 0;
}

namespace qdrt {

void init_graphics()
{
	grDispatcher::set_restore_handler(restore_graphics);
	grDispatcher::instance() -> Finit();

	if(!qdGameConfig::get_config().driver_ID())
		grDispatcher::set_instance(gdi_grD);
	else
		grDispatcher::set_instance(dd_grD);


	if(!init_graphics_dispatcher())
		app_errH.show_error("Ошибка инициализации графики",appErrorHandler::ERR_OTHER);

	grDispatcher::instance() -> resize_window();

	hmainWnd = (HWND)grDispatcher::instance() -> Get_hWnd();
	qdGameConfig::get_config().set_pixel_format(grDispatcher::instance() -> pixel_format());

	qdGameConfig::get_config().save();

	qdlg::set_icon(hmainWnd);
	appSetHandle(hmainWnd);

	SendMessage(hmainWnd,WM_SETTEXT,(WPARAM)0,(LPARAM)qd_gameD -> game_title());

	grDispatcher::instance() -> SetClip();
	grDispatcher::instance() -> SetClipMode(1);
	grDispatcher::instance() -> Fill(0);

	if(grDispatcher::instance() -> is_in_fullscreen_mode())
		ShowWindow(hmainWnd,SW_SHOWMAXIMIZED);
	else
		ShowWindow(hmainWnd,SW_SHOWNORMAL);

	UpdateWindow(hmainWnd);
	
	grDispatcher::instance() -> Flush();
}

bool init_graphics_dispatcher()
{
	if(grDispatcher::instance() -> init(qdGameConfig::get_config().screen_sx(),qdGameConfig::get_config().screen_sy(),(grPixelFormat)qdGameConfig::get_config().pixel_format(),hmainWnd,qdGameConfig::get_config().fullscreen()))
		return true;

	for(int i = 0; i <= GR_ARGB8888; i++){
		if(grDispatcher::instance() -> init(qdGameConfig::get_config().screen_sx(),qdGameConfig::get_config().screen_sy(),(grPixelFormat)i,hmainWnd,qdGameConfig::get_config().fullscreen()))
			return true;
	}

	qdGameConfig::get_config().toggle_fullscreen();

	for(int i = 0; i <= GR_ARGB8888; i++){
		if(grDispatcher::instance() -> init(qdGameConfig::get_config().screen_sx(),qdGameConfig::get_config().screen_sy(),(grPixelFormat)i,hmainWnd,qdGameConfig::get_config().fullscreen()))
			return true;
	}

	grDispatcher::set_instance(dd_grD);
	if(grDispatcher::instance() -> init(qdGameConfig::get_config().screen_sx(),qdGameConfig::get_config().screen_sy(),(grPixelFormat)qdGameConfig::get_config().pixel_format(),hmainWnd,qdGameConfig::get_config().fullscreen()))
		return true;

	return false;
}

void qd_show_load_progress(int percents_loaded,void* p)
{
	const int rect_sx = 200;
	const int rect_sy = 10;

	int sx = rect_sx * percents_loaded / 100;
	if(sx < 0) sx = 0;
	if(sx > rect_sx) sx = rect_sx;

	int x = 10;
	int y = grDispatcher::instance() -> Get_SizeY() - rect_sy - 10;

	grDispatcher::instance() -> Rectangle(x,y,rect_sx,rect_sy,0xFFFFFF,0,GR_OUTLINED);
	grDispatcher::instance() -> Rectangle(x,y,sx,rect_sy,0xFFFFFF,0xFFFFFF,GR_FILLED);
	grDispatcher::instance() -> Flush(x,y,rect_sx,rect_sy);
}

bool is_graphics_reinit_needed()
{
	if(qdGameConfig::get_config().pixel_format() != (int)grDispatcher::instance() -> pixel_format() || qdGameConfig::get_config().fullscreen() != grDispatcher::instance() -> is_in_fullscreen_mode())
		return true;

	if(qdGameConfig::get_config().driver_ID() && grDispatcher::instance() == gdi_grD)
		return true;

	if(!qdGameConfig::get_config().driver_ID() && grDispatcher::instance() == dd_grD)
		return true;

	return false;
}

void restore_graphics()
{
	if(sndDispatcher* dp = sndDispatcher::get_dispatcher())
		dp -> set_volume(dp -> volume());

	if(qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher())
		dp -> set_flag(qdGameDispatcher::FULLSCREEN_REDRAW_FLAG);
}

void restore()
{
	int dummy = 0;
}

void toggle_fullscreen(bool force_fullscreen)
{
	if(force_fullscreen && qdGameConfig::get_config().fullscreen()) return;

	qdGameConfig::get_config().toggle_fullscreen();
	qdGameConfig::get_config().set_driver_ID(1);

	grDispatcher::instance() -> toggle_reinit();
	grDispatcher::instance() -> Finit();
	grDispatcher::instance() -> destroy_window();

	hmainWnd = NULL;

	init_graphics();

	qdGameDispatcher::get_dispatcher() -> toggle_full_redraw();
//	qdGameDispatcher::get_dispatcher() -> convert_graphics();
}

bool request_CD_handler(int cd_id)
{
	if(qdGameDispatcher* p = qdGameDispatcher::get_dispatcher())
		p -> request_CD(cd_id);

	return true;
}

}; // namespace main
