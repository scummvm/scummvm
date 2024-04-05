/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "gr_dispatcher.h"
#include "mouse_input.h"
#include "keyboard_input.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */

LRESULT CALLBACK grWindowProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);

/* --------------------------- DEFINITION SECTION --------------------------- */

void* grDispatcher::default_mouse_cursor_;
char* grDispatcher::wnd_class_name_ = "grWndClass";

LRESULT CALLBACK GR_WindowProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	grDispatcher* p = grDispatcher::instance(hwnd);
	bool hittest = false;

 	switch(uMsg){
		case WM_SYSKEYDOWN:
			if((int)wParam != VK_F4)
				return 0;
			break;
	        case WM_NCHITTEST:
			hittest = true;
			break;
		case WM_CHAR:
			if(grDispatcher::handle_char_input(int(wParam)))
				return 0;
			break;
		case WM_SYSCOMMAND:
			if(wParam == SC_MAXIMIZE){
				if(p && p -> maximize_handler())
					(*p -> maximize_handler())();
				return 0;
			}
			break;
#ifndef _QUEST_EDITOR
	        case WM_CLOSE:
			ShowWindow(hwnd,SW_HIDE);
			DestroyWindow(hwnd);
			return 0;
#endif
	        case WM_DESTROY:
			if(!grDispatcher::instance() || !grDispatcher::instance() -> is_in_reinit_mode())
				PostQuitMessage(0);
			break;
		case WM_ACTIVATEAPP:
			if(wParam)
				grDispatcher::activate(true);
			else
				grDispatcher::activate(false);
			break;
		case WM_SETCURSOR:
			if(p){
#ifndef _QUEST_EDITOR
				SetCursor((HCURSOR)p -> mouse_cursor());
				return 1;
#else
				return 0;
#endif
			}
			break;
		case WM_PAINT:
			if(p){
				PAINTSTRUCT pc;
				HDC hdc = BeginPaint(hwnd,&pc);
				p -> Flush(pc.rcPaint.left,pc.rcPaint.top,pc.rcPaint.right - pc.rcPaint.left,pc.rcPaint.bottom - pc.rcPaint.top);
				EndPaint(hwnd,&pc);
				return 0;
			}
			break;
	}

	LRESULT res = DefWindowProc(hwnd,uMsg,wParam,lParam);

	if(hittest && p){
		if(res == HTCLIENT && p -> is_mouse_hidden())
			p -> set_null_mouse_cursor();
		else
			p -> set_default_mouse_cursor();
	}

	return res;
}

grDispatcher* grDispatcher::instance(void* hwnd)
{
	return reinterpret_cast<grDispatcher*>(GetWindowLong((HWND)hwnd,GWL_USERDATA));
}

bool grDispatcher::sys_init()
{
	WNDCLASS wc;

	wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_OWNDC;
	wc.lpfnWndProc = GR_WindowProc; 
	wc.cbClsExtra = 0; 
	wc.cbWndExtra = 0; 
	wc.hInstance = GetModuleHandle(0); 
	wc.hIcon = NULL; 
	wc.hCursor = NULL; 
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL; 
	wc.lpszClassName = grDispatcher::wnd_class_name();

	if(!RegisterClass(&wc)) return false;

	default_mouse_cursor_ = LoadCursor(NULL,IDC_ARROW);

	return true;
}

bool grDispatcher::sys_finit()
{
	UnregisterClass(grDispatcher::wnd_class_name(),GetModuleHandle(0));
	DestroyCursor((HCURSOR)default_mouse_cursor_);

	return true;
}

