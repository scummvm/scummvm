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

#ifndef GLS_GLSPRITES_H
#define GLS_GLSPRITES_H

#ifdef __cplusplus
	extern "C" {
#endif

/* Typedefs */
typedef unsigned __int32						GLS_UInt32;
typedef signed __int32							GLS_SInt32;
typedef unsigned __int8							GLS_UInt8;
typedef float									GLS_Float;
typedef enum { GLS_True = 1, GLS_False = 0 }	GLS_Bool;

/* Errors */
typedef enum 
{
	GLS_OK,
	GLS_OUT_OF_MEMORY,
	GLS_WINDOW_CREATION_FAILED,
	GLS_WINDOW_PREPARATION_FAILED,
	GLS_DISPLAY_MODE_CHANGE_FAILED,
	GLS_WINDOW_WAS_CLOSED,
	GLS_OPENGL_CONTEXT_CREATION_FAILED,
	GLS_COULD_NOT_FLIP,
	GLS_TEXTURE_CREATION_FAILED,
	GLS_NO_WINDOW,
	GLS_INVALID_SPRITE_OBJECT,
	GLS_INVALID_SPRITE_DIMENSIONS,
	GLS_NO_DRIVER_SUPPORT_FOR_VSYNC,
	GLS_OPENGL_ERROR,
	GLS_INVALID_DATA_DIMENSIONS,
	GLS_INVALID_DATA_POINTER,
	GLS_INVALID_SUB_IMAGE
} GLS_Result;

/* GLS_Rect */
typedef struct
{
	GLS_UInt32 x1;
	GLS_UInt32 y1;
	GLS_UInt32 x2;
	GLS_UInt32 y2;
} GLS_Rect;

/* GLS_Color */
typedef struct
{
	GLS_UInt8 r;
	GLS_UInt8 g;
	GLS_UInt8 b;
	GLS_UInt8 a;
} GLS_Color;

/* GLS_Sprite */
typedef void * GLS_Sprite;

/* Functions */
GLS_Result GLS_Init(GLS_UInt32 width, GLS_UInt32 height, GLS_Bool fullscreen, const char * windowTitle, GLS_Bool allowCloseWindow);
GLS_Result GLS_InitExternalWindow(GLS_UInt32 width, GLS_UInt32 height, GLS_Bool fullscreen, void * windowHandle);
GLS_Result GLS_Quit();

GLS_Result GLS_SetVSync(GLS_Bool status);
GLS_Result GLS_IsVsync(GLS_Bool * pStatus);

GLS_Result GLS_StartFrame();
GLS_Result GLS_EndFrame();

GLS_Result GLS_NewSprite(GLS_UInt32 width, GLS_UInt32 height, GLS_Bool useAlphachannel, const void * data, GLS_Sprite * pSprite);
GLS_Result GLS_DeleteSprite(GLS_Sprite sprite);

GLS_Result GLS_SetSpriteData(GLS_Sprite sprite, GLS_UInt32 width, GLS_UInt32 height, const void * data, GLS_UInt32 stride);

GLS_Result GLS_Blit(GLS_Sprite sprite,
					GLS_SInt32 x, GLS_SInt32 y,
					const GLS_Rect * subImage,
					const GLS_Color * color,
					GLS_Bool flipH, GLS_Bool flipV,
					GLS_Float scaleX, GLS_Float scaleY);

GLS_Result GLS_GetWindowHandle(void ** pWindowHandle);

const char * GLS_ResultString(GLS_Result ResultCode);

#ifdef __cplusplus
	}
#endif

#endif
