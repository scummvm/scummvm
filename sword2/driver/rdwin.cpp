/* Copyright (C) 1994-2003 Revolution Software Ltd
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "common/stdafx.h"
#include "base/engine.h"
#include "common/timer.h"

#include "driver96.h"

#include "_mouse.h"
#include "keyboard.h"
#include "rdwin.h"
#include "d_draw.h"
#include "palette.h"
#include "render.h"
#include "menu.h"
#include "d_sound.h"
#include "bs2/sword2.h"


#define MENUDEEP 40		// Temporary, until menu.h is written!



//static BOOL		bMouseVisible = FALSE;
//static BOOL		controlKey = FALSE;
//static BOOL		altKey = FALSE;
//static BOOL		wasScreenSaverActive = FALSE;
//static BOOL		myAppClosed = FALSE;
static BOOL		controlQDisabled = FALSE;
//static uint8	gameName[80];


//BOOL			gotTheFocus = FALSE;
//assume we always have focus for the time being - khalek
BOOL			gotTheFocus = TRUE; 
//HWND			hwnd;
//RECT			rcWindow;



//-----------------------------------------------------------------------------

void Zdebug(const char *format,...) {
#ifdef __PALM_OS__
	char buf[256]; // 1024 is too big overflow the stack
#else
	char buf[1024];
#endif
	va_list va;

	va_start(va, format);
	vsprintf(buf, format, va);
	va_end(va);

#ifdef __GP32__ //ph0x FIXME: implement fprint?
	printf("ZDEBUG: %s\n", buf);
#else
	fprintf(stderr, "ZDEBUG: %s!\n", buf);
#endif
#if defined( USE_WINDBG )
	strcat(buf, "\n");
#if defined( _WIN32_WCE )
	TCHAR buf_unicode[1024];
	MultiByteToWideChar(CP_ACP, 0, buf, strlen(buf) + 1, buf_unicode, sizeof(buf_unicode));
	OutputDebugString(buf_unicode);
#else
	OutputDebugString(buf);
#endif
#endif
}

/*
void Zdebug(char *format,...)	//Tony's special debug logging file March96
{
	warning("stub Zdebug");
//	Write a printf type string to a debug file

	va_list		arg_ptr;			// Variable argument pointer
	FILE *		debug_filep=0;			// Debug file pointer
	static int	first_debug = 1;		// Flag for first time this is used

	va_start(arg_ptr,format);

	if (first_debug)					//First time round delete any previous debug file
	{
		unlink("debug.txt");
		first_debug = 0;
	}

	debug_filep = fopen("debug.txt","a+t");

	if (debug_filep != NULL)	// if it could be opened
	{
		vfprintf(debug_filep, format, arg_ptr);
		fprintf(debug_filep,"\n");

		fclose(debug_filep);
	}
}
*/
//-----------------------------------------------------------------------------

/*
void Message(LPSTR fmt, ...)
{
    char	buff[256];
    va_list	va;

    
	va_start(va, fmt);

    //
    // format message with header
    //
    lstrcpy( buff, "DRIVER96:" );
    wvsprintf( &buff[lstrlen(buff)], fmt, va );
    lstrcat( buff, "\r\n" );

    //
    // To the debugger
    //
    OutputDebugString( buff );

}
*/

//-----------------------------------------------------------------------------
//	OSystem Event Handler. Full of cross platform goodness and 99% fat free!
//-----------------------------------------------------------------------------
void Sword2State::parseEvents() {
	OSystem::Event event;
	
	while (_system->poll_event(&event)) {
		switch(event.event_code) {
		
		case OSystem::EVENT_KEYDOWN:
			if (event.kbd.flags==OSystem::KBD_CTRL) {
				if (event.kbd.keycode == 'w')
					GrabScreenShot();
			}
			WriteKey(event.kbd.ascii, event.kbd.keycode, event.kbd.flags);
			break;
		case OSystem::EVENT_MOUSEMOVE:
			mousex = event.mouse.x;
			mousey = event.mouse.y - MENUDEEP;
			break;
		case OSystem::EVENT_LBUTTONDOWN:
			LogMouseEvent(RD_LEFTBUTTONDOWN);
			break;
		case OSystem::EVENT_RBUTTONDOWN:
			LogMouseEvent(RD_RIGHTBUTTONDOWN);
			break;
		case OSystem::EVENT_LBUTTONUP:
			LogMouseEvent(RD_LEFTBUTTONUP);
			break;
		case OSystem::EVENT_RBUTTONUP:
			LogMouseEvent(RD_RIGHTBUTTONUP);
			break;
		case OSystem::EVENT_QUIT:
			Close_game();
			CloseAppWindow();
			break;
		default:
			break;
		}
	}
		
}


//-----------------------------------------------------------------------------
//	Windows Message Handler.  All keyboard and mouse input is handled here.
//-----------------------------------------------------------------------------
/*
long FAR PASCAL WindowsMessageHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{

	switch( message )
	{


	case WM_TIMER:
		switch (wParam)
		{
		case 25:
			FadeServer();
			return(0);
		case 1:
			FxServer();
			return(0);
		}
		break;
		

    case WM_CLOSE:
		Zdebug("WM_CLOSE");
		break;

    case WM_SIZE:
    case WM_MOVE:
        if (IsIconic(hwnd))
        {
            Message("minimising");
//            PauseGame();
        }

        if (bFullScreen)
        {
            SetRect(&rcWindow, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
        }
        else
        {
            GetClientRect(hwnd, &rcWindow);
            ClientToScreen(hwnd, (LPPOINT)&rcWindow);
            ClientToScreen(hwnd, (LPPOINT)&rcWindow+1);
        }
        Message("WINDOW RECT: [%d,%d,%d,%d]", rcWindow.left, rcWindow.top, rcWindow.right, rcWindow.bottom);
		if (bFullScreen)
		{
			SetCapture(hwnd);
		}
		else
		{
			ReleaseCapture();
		}
//		SetCursor(NULL);
//		ShowCursor(FALSE);
        break;



	case WM_ACTIVATEAPP:
		gotTheFocus = wParam;
		if (gotTheFocus)
		{
			Message("Got the focus");
			bMouseVisible = FALSE;
			Message("Mouse invisible");
			ShowCursor(FALSE);
		}
		else
		{
			if (bMouseVisible == FALSE)
				ShowCursor(TRUE);
			Message("Lost the focus");
			bMouseVisible = TRUE;
			Message("Mouse visible");
		}
		break;


    case WM_SYSKEYUP:
        switch( wParam )
        {

//			int32 rv;

        // handle ALT+ENTER (fullscreen)
        case VK_RETURN:
			break;
        }
        break;
 

	case WM_DISPLAYCHANGE:
		break;

	case WM_CREATE:
		SystemParametersInfo(SPI_GETSCREENSAVEACTIVE , 0 , &wasScreenSaverActive , 0);
		if (wasScreenSaverActive)
		{
			SystemParametersInfo(SPI_SETSCREENSAVEACTIVE , FALSE , 0 , 0 );
		}
    	break;


    case WM_QUERYNEWPALETTE:
        //
        //  we are getting the palette focus, select our palette
        //
        if (!bFullScreen && lpPalette && lpPrimarySurface)
        {
			int32 hr;

            hr = IDirectDrawSurface_BS2_SetPalette(lpPrimarySurface, lpPalette);
            if (hr == DDERR_SURFACELOST)
            {
                IDirectDrawSurface_Restore(lpPrimarySurface);

                hr= IDirectDrawSurface_BS2_SetPalette(lpPrimarySurface, lpPalette);
                if(hr == DDERR_SURFACELOST)
                {
                   Message("Failed to restore palette after second try");
                }
            }

            //
            // Restore normal title if palette is ours
            //
            if(hr == DD_OK)
            {
                SetWindowText(hwnd, gameName);
            }
        }
        break;

    case WM_PALETTECHANGED:
        //
        //  if another app changed the palette we dont have full control
        //  of the palette. NOTE this only applies for FoxBear in a window
        //  when we are fullscreen we get all the palette all of the time.
        //
        if ((HWND)wParam != hwnd)
        {
            if( !bFullScreen )
            {
				Message("Lost palette but continuing");
            }
        }
        break;


//    case WM_SETCURSOR:
//		if (bMouseVisible)
//			SetCursor(LoadCursor(NULL, IDC_ARROW));
//		else
//			SetCursor(NULL);
//	    return TRUE;
//		break;

	case WM_CHAR:
		if (lParam & (1 << 30))
			return(0);
		WriteKey((char) (wParam & 0xff));
		return(0);

    case WM_KEYDOWN:

		Zdebug("key %d", wParam);

		switch( wParam )
		{
			case VK_CONTROL:
				controlKey = TRUE;
				break;

//			case VK_ALT:
//				altKey = TRUE;
//				break;

			case 'W':
				if (controlKey)
					GrabScreenShot();
				return 0;

			case 'Q':
				if (controlKey && !controlQDisabled)
					DestroyWindow( hWnd );
  				return 0;

			case 'F4':
				DestroyWindow( hWnd );
  				return 0;

        }
      	break;


	case WM_KEYUP:
		switch(wParam)
		{
			case VK_CONTROL:
				controlKey = FALSE;
				break;

//			case VK_ALT:
//				altKey = FALSE;
//				break;

		}
	    break;


    case WM_DESTROY:
		Zdebug("*destroy*");

		if (wasScreenSaverActive)
			SystemParametersInfo(SPI_SETSCREENSAVEACTIVE , TRUE , 0 , 0 );
        PostQuitMessage( 0 );
        break;


	case WM_MOUSEMOVE:
		mousex = lParam & 0xffff;

		if (bFullScreen)
		{
			mousey = (uint16) (lParam >> 16) - MENUDEEP;
		}
		else
		{
			mousey = (uint16) (lParam >> 16) - MENUDEEP;

			if (mousex < 0)
				mousex = 0;
			if (mousex >= RENDERWIDE)
				mousex = RENDERWIDE-1;
		}

		if (mousey < -MENUDEEP)
			mousey = -MENUDEEP;
		if (mousey >= RENDERDEEP + MENUDEEP)
			mousey = RENDERDEEP + MENUDEEP - 1;

		return(0);

	case WM_LBUTTONDOWN:
		LogMouseEvent(RD_LEFTBUTTONDOWN);
		return(0);

	case WM_LBUTTONUP:
		LogMouseEvent(RD_LEFTBUTTONUP);
		return(0);

	case WM_RBUTTONDOWN:
		LogMouseEvent(RD_RIGHTBUTTONDOWN);
		return(0);

	case WM_RBUTTONUP:
		LogMouseEvent(RD_RIGHTBUTTONUP);
		return(0);

	case WM_LBUTTONDBLCLK:
		LogMouseEvent(RD_LEFTBUTTONDOWN);
		return(0);
		
	case WM_RBUTTONDBLCLK:
		LogMouseEvent(RD_RIGHTBUTTONDOWN);


	case WM_SYSCOMMAND:
		if (gotTheFocus)
			return(0);




	}

	return DefWindowProc(hWnd, message, wParam, lParam);

}
*/


/*
int32 InitialiseWindow(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow, char *gameName)

{
	
	WNDCLASS    wc;
//	uint32		err;


//	hPrevInstance = hPrevInstance;
	wc.style = CS_DBLCLKS;
	wc.lpfnWndProc = WindowsMessageHandler;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon( hInstance, "resourc1");  //IDI_APPLICATION );
	wc.hCursor = LoadCursor( NULL, IDC_ARROW );
	wc.hbrBackground = GetStockObject(BLACK_BRUSH);

	wc.lpszMenuName = gameName;
	wc.lpszClassName = gameName;

	RegisterClass( &wc );

	// Create a window
	hwnd = CreateWindowEx(WS_EX_APPWINDOW, gameName, gameName, WS_VISIBLE | WS_SYSMENU | WS_POPUP, 0, 0,
			GetSystemMetrics( SM_CXSCREEN ), GetSystemMetrics( SM_CYSCREEN ),
			NULL, NULL, hInstance, NULL );

	if(!hwnd)
	{
	    MessageBox(hwnd, "Failed to create window", gameName, MB_OK );
	    DestroyWindow(hwnd);
		return(RDERR_CREATEWINDOW);
	}

//	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);
	SetFocus(hwnd);
	
	SetTimer(hwnd, 25, 1000 / 25, NULL);
	SetTimer(hwnd, 1, 100, NULL);

	return(RD_OK);

}
*/


int32 CloseAppWindow(void)

{
	warning("stub CloseAppWindow");
/*
	DestroyWindow(hwnd);
*/
	// just quit for now
	g_engine->_timer->releaseProcedure(sword2_sound_handler);
	g_system->quit();
	return(RD_OK);

}

static bool _needRedraw = false;

void SetNeedRedraw() {
	_needRedraw = true;
}

int32 ServiceWindows(void)

{
	g_sword2->parseEvents();
	FadeServer();

	// FIXME: We re-render the entire picture area of the screen for each
	// frame, which is pretty horrible.

	if (_needRedraw) {
		g_system->copy_rect(lpBackBuffer + MENUDEEP * screenWide, screenWide, 0, MENUDEEP, screenWide, screenDeep - 2 * MENUDEEP);
		_needRedraw = false;
	}

	// We still need to update because of fades, menu animations, etc.
	g_system->update_screen();

//	warning("stub ServiceWindows");  // too noisy
/*
	MSG msg;

	if (myAppClosed)
		return(RDERR_APPCLOSED);

	while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		if (!GetMessage(&msg, NULL, 0, 0))
		{
			myAppClosed = TRUE;
			return(RDERR_APPCLOSED);
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
  	}
*/
	return(RD_OK);

}




int32 _ReportDriverError(int32 error, const uint8 *filename, uint32 line)

{
	warning("stub _ReportDriverError 0x%.8x file: %s, line: %d ", error, (const char *) filename, line);
/*

	char errorText[128];
	char name[80];


	GetGameName(name);
	sprintf(errorText, "Fatal error in %s, line %u!  Code 0x%.8x", filename, line, error);
    MessageBox(hwnd, errorText, name, MB_SYSTEMMODAL | MB_ICONHAND | MB_ABORTRETRYIGNORE);
*/	
	return(RD_OK);

}



int32 _ReportFatalError(const uint8 *error, const uint8 *filename, uint32 line)

{

	warning("stub _ReportFatalError");
	char errorText[500];
	char name[80];


	GetGameName((uint8 *)name);
	sprintf(errorText, "FATAL ERROR - GAME TERMINATED\n%s", error);
	//MessageBox(hwnd, errorText, name, MB_SYSTEMMODAL | MB_ICONHAND | MB_ABORTRETRYIGNORE);
	warning("%s", errorText);

	return(RD_OK);

}


int32 DisableQuitKey(void)
{
	controlQDisabled = TRUE;
	return(RD_OK);
}

void SetWindowName(const char *windowName)
{
	warning("stub SetWindowName( %s )", windowName);
//	SetWindowText(hwnd,windowName);
//	strcpy(gameName,windowName);
}
