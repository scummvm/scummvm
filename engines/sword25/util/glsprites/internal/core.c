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

#include "glinclude.h"
#include "core.h"
#include "glswindow.h"


/* --------------------------------------------------------------------------
   GLOBALS
   -------------------------------------------------------------------------- */

GLS_OGLCaps GLS_TheOGLCaps;
static GLS_Window * window = 0;
static GLS_Bool vsyncActive = GLS_False;
typedef BOOL (APIENTRY *PFNWGLSWAPINTERVALFARPROC)( int );
static PFNWGLSWAPINTERVALFARPROC wglSwapIntervalEXT = 0;


/* -------------------------------------------------------------------------- */

void InitGL(GLS_UInt32 width, GLS_UInt32 height)
{
	if (height == 0) height = 1;

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width, height, 0, 1, -1);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glShadeModel(GL_SMOOTH);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
}

/* -------------------------------------------------------------------------- */

void InitGLCaps(GLS_OGLCaps * OGLCaps)
{
	if (strstr(glGetString(GL_EXTENSIONS), "GL_ARB_texture_rectangle") != 0 ||
		strstr(glGetString(GL_EXTENSIONS), "GL_EXT_texture_rectangle") != 0 ||
		strstr(glGetString(GL_EXTENSIONS), "GL_NV_texture_rectangle") != 0)
	{
		OGLCaps->textureRectanglesSupported = GLS_True;
		glGetIntegerv(GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB, &(OGLCaps->maxTextureRectangleSize));
	}
	else
		OGLCaps->textureRectanglesSupported = GLS_False;

	if (strstr(glGetString(GL_EXTENSIONS), "WGL_EXT_swap_control") != 0)
	{
		wglSwapIntervalEXT = (PFNWGLSWAPINTERVALFARPROC) wglGetProcAddress("wglSwapIntervalEXT");
	}

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &(OGLCaps->maxTextureSize));
}

/* -------------------------------------------------------------------------- */

GLS_Result GLS_Init(GLS_UInt32 width, GLS_UInt32 height, GLS_Bool fullscreen, const char * windowTitle, GLS_Bool allowCloseWindow)
{
	GLS_Result Result;

	Result = GLS_CreateGLWindow(width, height, fullscreen, windowTitle, allowCloseWindow, &window);
	if (Result != GLS_OK) return Result;
	InitGL(width, height);
	InitGLCaps(&GLS_TheOGLCaps);

	return GLS_OK;
}

/* -------------------------------------------------------------------------- */

GLS_Result GLS_InitExternalWindow(GLS_UInt32 width, GLS_UInt32 height, GLS_Bool fullscreen, void * windowHandle)
{
	GLS_Result Result;

	Result = GLS_CreateGLExternalWindow(width, height, fullscreen, windowHandle, &window);
	if (Result != GLS_OK) return Result;
	InitGL(width, height);
	InitGLCaps(&GLS_TheOGLCaps);

	return GLS_OK;
}


/* -------------------------------------------------------------------------- */

GLS_Result GLS_Quit()
{
	if (window) GLS_CloseGLWindow(window);

	return GLS_OK;
}


/* -------------------------------------------------------------------------- */

GLS_Result GLS_StartFrame()
{
	glClear(GL_COLOR_BUFFER_BIT);
	return GLS_ProcessWindowMessages(window);
}


/* -------------------------------------------------------------------------- */

GLS_Result GLS_EndFrame()
{
	if (GLS_WindowFlip(window))
		return GLS_OK;
	else
		return GLS_COULD_NOT_FLIP;
}


/* -------------------------------------------------------------------------- */

GLS_Result GLS_GetWindowHandle(void ** pWindowHandle)
{
	if (window)
	{
		*pWindowHandle = window->hwnd;
		return GLS_OK;
	}
	else
	{
		return GLS_NO_WINDOW;
	}
}


/* -------------------------------------------------------------------------- */

GLS_Result GLS_SetVSync(GLS_Bool status)
{
	if (wglSwapIntervalEXT)
	{
		if (wglSwapIntervalEXT(status ? 1 : 0))
		{
			vsyncActive = status;
			return GLS_OK;
		}
		else
			return GLS_OPENGL_ERROR;
	}
	else
		return GLS_NO_DRIVER_SUPPORT_FOR_VSYNC;
}

GLS_Result GLS_IsVsync(GLS_Bool * pStatus)
{
	if (wglSwapIntervalEXT)
	{
		*pStatus = vsyncActive;
		return GLS_OK;
	}
	else
		return GLS_NO_DRIVER_SUPPORT_FOR_VSYNC;
}


/* -------------------------------------------------------------------------- */

#define GLRS(ERR) case GLS_##ERR: return #ERR;
const char * GLS_ResultString(GLS_Result ResultCode)
{
	switch (ResultCode)
	{
		GLRS(OK)
		GLRS(OUT_OF_MEMORY)
		GLRS(WINDOW_CREATION_FAILED)
		GLRS(WINDOW_PREPARATION_FAILED)
		GLRS(DISPLAY_MODE_CHANGE_FAILED)
		GLRS(WINDOW_WAS_CLOSED)
		GLRS(OPENGL_CONTEXT_CREATION_FAILED)
		GLRS(COULD_NOT_FLIP)
		GLRS(TEXTURE_CREATION_FAILED)
		GLRS(NO_WINDOW)
		GLRS(INVALID_SPRITE_OBJECT)
		GLRS(INVALID_SPRITE_DIMENSIONS)
		GLRS(NO_DRIVER_SUPPORT_FOR_VSYNC)
		GLRS(OPENGL_ERROR)
		GLRS(INVALID_DATA_DIMENSIONS)
		GLRS(INVALID_DATA_POINTER)
		GLRS(INVALID_SUB_IMAGE)
	default:
		return "unknown result code";
	}
}
#undef GLRS
