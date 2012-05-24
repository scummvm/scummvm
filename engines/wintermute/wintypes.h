/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_WINTYPES_H
#define WINTERMUTE_WINTYPES_H

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/scummsys.h"

//namespace WinterMute {
#ifndef __WIN32__

#define WINAPI
#define CALLBACK

#define PI ((float) 3.141592653589793f)
#define DRGBA(r,g,b,a) ((uint32)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))

#define D3DCOLGetB(rgb)  ((byte )(rgb))
#define D3DCOLGetG(rgb)  ((byte )(((uint16)(rgb)) >> 8))
#define D3DCOLGetR(rgb)  ((byte )((rgb)>>16))
#define D3DCOLGetA(rgb)  ((byte )((rgb)>>24))

#define MAX_PATH 512

typedef char *NPSTR, *LPSTR, *PSTR;
typedef PSTR *PZPSTR;
typedef const PSTR *PCZPSTR;
typedef const char *LPCSTR, *PCSTR;
typedef PCSTR *PZPCSTR;

typedef struct tagRECT {
	int32	left;
	int32	top;
	int32  right;
	int32	bottom;
} RECT, *LPRECT;


typedef struct tagPOINT {
	int32  x;
	int32  y;
} POINT, *LPPOINT;


typedef uint32 HINSTANCE;
typedef uint32 HMODULE;
typedef uint32 HWND;

typedef int32 HRESULT;

#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#define FAILED(hr) (((HRESULT)(hr)) < 0)

#define S_OK      ((HRESULT)0)
//#define S_FALSE   ((HRESULT)1)
#define E_FAIL   ((HRESULT)-1)


#endif // !__WIN32__

//} // end of namespace WinterMute

#endif // WINTERMUTE_WINTYPES_H
