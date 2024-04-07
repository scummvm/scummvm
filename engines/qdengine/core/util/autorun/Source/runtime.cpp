
/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "gdi_gr_dispatcher.h"
#include "qd_file_manager.h"

#include "ar_button.h"
#include "input_wndproc.h"

#include "..\Resource.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */

bool mouseMove(int x,int y,int flags);
bool mouseClick(int x,int y,int flags);

char* getIniKey(const char* fname,const char* section,const char* key);

/* --------------------------- DEFINITION SECTION --------------------------- */

HWND hmainWnd;

arButtonDispatcher* arbt_D;

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpszCmdLine, int nCmdShow)
{
	const char* const autorun_event_name = "K-D Lab Game Launcher";
	if(HANDLE event = OpenEvent(EVENT_ALL_ACCESS,FALSE,autorun_event_name))
		return 0;
	else
		event = CreateEvent(0,TRUE,TRUE,autorun_event_name);

	const char* event_name = getIniKey("autorun.ini","settings","game_event_name");
	if(strlen(event_name)){
		if(HANDLE event = OpenEvent(EVENT_ALL_ACCESS,FALSE,event_name))
			return 0;
	}

	XStream fh;

	qdFileManager::instance().toggle_silent_update_mode(true);

	arbt_D = new arButtonDispatcher;
	arbt_D -> init("autorun.ini");
	
	grDispatcher::sys_init();

	GDI_grDispatcher* gr_D = new GDI_grDispatcher;

	if(!arbt_D -> background()) return 1;

	int sx = arbt_D -> background() -> size_x();
	int sy = arbt_D -> background() -> size_y();

	int x = (GetSystemMetrics(SM_CXSCREEN) - sx)/2;
	int y = (GetSystemMetrics(SM_CYSCREEN) - sy)/2;

	hmainWnd = CreateWindow(grDispatcher::wnd_class_name(),getIniKey("autorun.ini","settings","wnd_title"),WS_POPUP,x,y,sx,sy,NULL,NULL,hInstance,NULL);

	if(!grDispatcher::instance() -> init(sx,sy,GR_RGB888,hmainWnd,false))
		return 1;

	if(gr_D -> palette_mode()){
		XStream fh(0);
		const char* pal_file = getIniKey("autorun.ini","settings","palette_file");
		if(pal_file[0] && fh.open(pal_file)){
			char pal_buf[768];
			fh.read(pal_buf,768);
			fh.close();
			gr_D -> set_palette(pal_buf,0,256);
		}
	}
	
	mouseDispatcher::instance() -> set_event_handler(mouseDispatcher::EV_LEFT_DOWN,mouseClick);
	mouseDispatcher::instance() -> set_event_handler(mouseDispatcher::EV_MOUSE_MOVE,mouseMove);

	if(!arbt_D -> init_language("autorun.ini"))
		arbt_D -> redraw();

	SetTimer(hmainWnd,1,300,NULL);

	ShowWindow(hmainWnd,SW_SHOWNORMAL);
	UpdateWindow(hmainWnd);

	MSG msg;
	while(GetMessage(&msg,NULL,0,0)){ 
		switch(msg.message){
			case WM_TIMER:
				if(arbt_D -> need_check())
					arbt_D -> check_exec();
				break;
			case WM_KEYDOWN:
				if(msg.wParam == VK_ESCAPE)
					PostQuitMessage(0);
				break;
		}
		
		input::keyboard_wndproc(msg,keyboardDispatcher::instance());
		input::mouse_wndproc(msg,mouseDispatcher::instance());

		DispatchMessage(&msg); 
	} 

	KillTimer(hmainWnd,1);

	delete gr_D;
	grDispatcher::sys_finit();

	return 0;
}

bool mouseMove(int x,int y,int flags)
{
	int dx,dy;
	static RECT rc;
	static int mx = -1;
	static int my = -1;

	if(mx == -1){
		mx = x;
		my = y;

		return true;
	}

	if(flags & MK_LBUTTON){
		dx = x - mx;
		dy = y - my;

		if(dx || dy){
			GetWindowRect(hmainWnd,&rc);
			SetWindowPos(hmainWnd,NULL,rc.left + dx,rc.top + dy,0,0,SWP_NOZORDER | SWP_NOSIZE);
		}
	}
	else {
		arbt_D -> hit(x,y);

		mx = x;
		my = y;
	}

	return true;
}

bool mouseClick(int x,int y,int flags)
{
	arbt_D -> click(x,y);
	return true;
}
