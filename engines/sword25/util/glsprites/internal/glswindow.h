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

#ifndef GLS_WINDOW_H
#define GLS_WINDOW_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "../glsprites.h"

typedef struct
{
	HWND		hwnd;
	HGLRC		hrc;
	GLS_Bool	fullscreen;
	GLS_Bool	ownHwnd;
} GLS_Window;

GLS_Result GLS_CreateGLWindow(GLS_UInt32 width, GLS_UInt32 height, GLS_Bool fullscreen, const char * windowTitle, GLS_Bool allowCloseWindow, GLS_Window ** pWindow);
GLS_Result GLS_CreateGLExternalWindow(GLS_UInt32 width, GLS_UInt32 height, GLS_Bool fullscreen, void * windowHandle, GLS_Window ** pWindow);
GLS_Result GLS_CloseGLWindow(GLS_Window * window);
GLS_Result GLS_ProcessWindowMessages(GLS_Window * window);
GLS_Result GLS_WindowFlip(GLS_Window * window);

#endif
