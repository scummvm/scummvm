/******************************************************************************/
/* This file is part of Broken Sword 2.5                                      */
/* Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdörfer     */
/*                                                                            */
/* Broken Sword 2.5 is free software; you can redistribute it and/or modify   */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* Broken Sword 2.5 is distributed in the hope that it will be useful,        */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of             */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the               */
/* GNU General Public License for more details.                               */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with Broken Sword 2.5; if not, write to the Free Software            */
/* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA   */
/******************************************************************************/

/* --------------------------------------------------------------------------
   INCLUDES
   -------------------------------------------------------------------------- */


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <malloc.h>

#include "glswindow.h"


/* --------------------------------------------------------------------------
   CONSTANTS
   -------------------------------------------------------------------------- */

static const char *		WINDOW_CLASSNAME = "GLsprites class";
static const GLS_UInt32	BITS_PER_PIXEL = 32;


/* -------------------------------------------------------------------------- */

static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_SYSCOMMAND:
		{
			switch (wParam)
			{
			case SC_SCREENSAVE:
			case SC_MONITORPOWER:
				return 0;
			}
			break;
		}

	case WM_CLOSE:
		{
			if (GetWindowLong(hWnd, GWL_USERDATA)) PostQuitMessage(0);
			return 0;
		}
	}

	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

/* -------------------------------------------------------------------------- */

static GLS_Bool RegisterWindowClass()
{
	WNDCLASS wc;

	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc		= (WNDPROC) WndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= GetModuleHandle(NULL);
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground	= NULL;
	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= "OpenGL";

	if (RegisterClass(&wc))
		return GLS_True;
	else
		return GLS_False;
}

/* -------------------------------------------------------------------------- */

static void CenterWindow(RECT * pWR, int * pX, int * pY)
{
	int screenX, screenY;

	screenX = GetSystemMetrics(SM_CXSCREEN);
	screenY = GetSystemMetrics(SM_CYSCREEN);

	if (screenX == 0 || screenY == 0)
	{
		*pX = 0;
		*pY = 0;
	}
	else
	{
		*pX = (screenX - (pWR->right - pWR->left)) / 2;
		*pY = (screenY - (pWR->bottom - pWR->top)) / 2;
	}
}

/* -------------------------------------------------------------------------- */

static GLS_Bool CreateTheWindow(GLS_UInt32 width, GLS_UInt32 height, GLS_Bool fullscreen, const char * windowTitle, GLS_Bool allowCloseWindow, HWND * pHwnd)
{
	DWORD style;
	DWORD exStyle;
	RECT windowRect;
	int x, y;

	if (fullscreen)
	{
		exStyle = WS_EX_APPWINDOW;
		style = WS_POPUP;
	}
	else
	{
		exStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		style = WS_CAPTION;
		if (allowCloseWindow) style |= WS_SYSMENU;
	}

	windowRect.left = 0;
	windowRect.right = width;
	windowRect.top= 0;
	windowRect.bottom = height;
	if (!AdjustWindowRectEx(&windowRect, style, FALSE, exStyle)) return GLS_False;

	if (fullscreen)
	{
		x = 0;
		y = 0;
	}
	else
		CenterWindow(&windowRect, &x, &y);

	*pHwnd = CreateWindowEx(
		exStyle,
		"OpenGL",
		windowTitle,
		style | WS_CLIPSIBLINGS |	WS_CLIPCHILDREN | WS_VISIBLE,
		x, y,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		GetModuleHandle(NULL),
		NULL);

	if (!*pHwnd) return GLS_False;

	SetWindowLong(*pHwnd, GWL_USERDATA, (allowCloseWindow == GLS_True) ? 1 : 0);

	return GLS_True;
}

/* -------------------------------------------------------------------------- */

static void SetWindowClientSize(HWND hwnd, int cx, int cy)
{
	HMENU hmenu = GetMenu(hwnd);
	RECT rcWindow = { 0, 0, cx, cy };

	/*
	*  First convert the client rectangle to a window rectangle the
	*  menu-wrap-agnostic way.
	*/
	AdjustWindowRectEx(&rcWindow, GetWindowLong(hwnd, GWL_STYLE), hmenu != NULL, GetWindowLong(hwnd, GWL_EXSTYLE));

	/*
	*  If there is a menu, then check how much wrapping occurs
	*  when we set a window to the width specified by AdjustWindowRect
	*  and an infinite amount of height.  An infinite height allows
	*  us to see every single menu wrap.
	*/
	if (hmenu) {
		RECT rcTemp = rcWindow;
		rcTemp.bottom = 0x7FFF;     /* "Infinite" height */
		SendMessage(hwnd, WM_NCCALCSIZE, FALSE, (LPARAM)&rcTemp);

		/*
		*  Adjust our previous calculation to compensate for menu
		*  wrapping.
		*/
		rcWindow.bottom += rcTemp.top;
	}

	SetWindowPos(hwnd, NULL, 0, 0, rcWindow.right - rcWindow.left,
		rcWindow.bottom - rcWindow.top, SWP_NOMOVE | SWP_NOZORDER);

}

/* ---------------------------------------------------------------------------- */

static GLS_Bool PrepareWindow(GLS_UInt32 width, GLS_UInt32 height, GLS_Bool fullscreen, HWND hwnd)
{
	if (fullscreen)
	{
		SetWindowLong(hwnd, GWL_STYLE, WS_POPUP);
		SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_TOPMOST);
		SetWindowPos(hwnd, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	}

	SetWindowClientSize(hwnd, width, height);
	ShowWindow(hwnd, SW_SHOW);

	return GLS_True;
}

/* -------------------------------------------------------------------------- */

static GLS_Bool ChangeDisplayMode(GLS_UInt32 width, GLS_UInt32 height)
{
	DEVMODE dmScreenSettings;
	memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
	dmScreenSettings.dmSize = sizeof(dmScreenSettings);
	dmScreenSettings.dmPelsWidth	= width;
	dmScreenSettings.dmPelsHeight	= height;
	dmScreenSettings.dmBitsPerPel	= BITS_PER_PIXEL;
	dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

	if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL)
		return GLS_True;
	else
		return GLS_False;
}

/* -------------------------------------------------------------------------- */

static GLS_Bool CreateOpenGLContext(HWND hwnd, HGLRC * pHrc)
{
	HDC hdc;
	int pixelformat;
	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		BITS_PER_PIXEL,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		0,
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	hdc = GetDC(hwnd);
	if (!hdc) return GLS_False;

	pixelformat = ChoosePixelFormat(hdc, &pfd);
	if (!pixelformat)
	{
		ReleaseDC(hwnd, hdc);
		return GLS_False;
	}

	if (!SetPixelFormat(hdc, pixelformat, &pfd))
	{
		ReleaseDC(hwnd, hdc);
		return GLS_False;
	}

	*pHrc = wglCreateContext(hdc);
	if (!*pHrc)
	{
		ReleaseDC(hwnd, hdc);
		return GLS_False;
	}

	if (!wglMakeCurrent(hdc, *pHrc))
	{
		wglDeleteContext(*pHrc);
		*pHrc = 0;
		ReleaseDC(hwnd, hdc);
		return GLS_False;
	}

	ReleaseDC(hwnd, hdc);
	return GLS_True;
}

/* -------------------------------------------------------------------------- */

GLS_Result GLS_CreateGLWindow(GLS_UInt32 width, GLS_UInt32 height, GLS_Bool fullscreen, const char * windowTitle, GLS_Bool allowCloseWindow, GLS_Window ** pWindow)
{
	*pWindow = (GLS_Window *) malloc(sizeof(GLS_Window));
	if (!*pWindow) return GLS_OUT_OF_MEMORY;

	(*pWindow)->hrc = 0;
	(*pWindow)->hwnd = 0;
	(*pWindow)->fullscreen = GLS_False;
	(*pWindow)->ownHwnd = GLS_True;

	if (!RegisterWindowClass())
	{
		GLS_CloseGLWindow(*pWindow);
		return GLS_WINDOW_CREATION_FAILED;
	}

	if (!CreateTheWindow(width, height, fullscreen, windowTitle, allowCloseWindow, &((*pWindow)->hwnd)))
	{
		GLS_CloseGLWindow(*pWindow);
		return GLS_WINDOW_CREATION_FAILED;
	}

	if (fullscreen)
	{
		if (!ChangeDisplayMode(width, height))
		{
			return GLS_DISPLAY_MODE_CHANGE_FAILED;
		}
		(*pWindow)->fullscreen = GLS_True;
	}

	if (!CreateOpenGLContext((*pWindow)->hwnd, &((*pWindow)->hrc)))
	{
		GLS_CloseGLWindow(*pWindow);
		return GLS_OPENGL_CONTEXT_CREATION_FAILED;
	}

	return GLS_OK;
}

/* -------------------------------------------------------------------------- */

GLS_Result GLS_CreateGLExternalWindow(GLS_UInt32 width, GLS_UInt32 height, GLS_Bool fullscreen, void * windowHandle, GLS_Window ** pWindow)
{
	*pWindow = (GLS_Window *) malloc(sizeof(GLS_Window));
	if (!*pWindow) return GLS_OUT_OF_MEMORY;

	(*pWindow)->hrc = 0;
	(*pWindow)->hwnd = windowHandle;
	(*pWindow)->fullscreen = GLS_False;
	(*pWindow)->ownHwnd = GLS_False;

	if (!PrepareWindow(width, height, fullscreen, (*pWindow)->hwnd))
	{
		GLS_CloseGLWindow(*pWindow);
		return GLS_WINDOW_PREPARATION_FAILED;
	}

	if (fullscreen)
	{
		if (!ChangeDisplayMode(width, height))
		{
			return GLS_DISPLAY_MODE_CHANGE_FAILED;
		}
		(*pWindow)->fullscreen = GLS_True;
	}

	if (!CreateOpenGLContext((*pWindow)->hwnd, &((*pWindow)->hrc)))
	{
		GLS_CloseGLWindow(*pWindow);
		return GLS_OPENGL_CONTEXT_CREATION_FAILED;
	}

	return GLS_OK;
}


/* -------------------------------------------------------------------------- */

GLS_Result GLS_CloseGLWindow(GLS_Window * window)
{
	if (window)
	{
		if (window->hrc)
		{
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(window->hrc);
		}

		if (window->fullscreen) ChangeDisplaySettings(NULL, 0);
		if (window->ownHwnd)
		{
			if (window->hwnd) DestroyWindow(window->hwnd);
			UnregisterClass(WINDOW_CLASSNAME, GetModuleHandle(NULL));
		}

		free(window);
	}

	return GLS_True;
}


/* -------------------------------------------------------------------------- */

GLS_Result GLS_ProcessWindowMessages(GLS_Window * window)
{
	MSG msg;
	GLS_Result Result = GLS_OK;

	if (window->ownHwnd)
	{
		while (PeekMessage(&msg, window->hwnd, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				Result = GLS_WINDOW_WAS_CLOSED;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return Result;
}


/* -------------------------------------------------------------------------- */

GLS_Result GLS_WindowFlip(GLS_Window * window)
{
	HDC hdc;

	hdc = GetDC(window->hwnd);
	if (!hdc) return GLS_False;

	if (SwapBuffers(hdc))
	{
		ReleaseDC(window->hwnd, hdc);
		return GLS_True;
	}
	else
	{
		ReleaseDC(window->hwnd, hdc);
		return GLS_False;
	}
}
