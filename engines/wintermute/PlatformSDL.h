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

#ifndef WINTERMUTE_PLATFORMSDL_H
#define WINTERMUTE_PLATFORMSDL_H

#include "dctypes.h"

#include "wintypes.h"
#include "common/events.h"

namespace WinterMute {

class CBGame;

//////////////////////////////////////////////////////////////////////////
class CBPlatform {
public:
	static int Initialize(CBGame *inGame, int argc, char *argv[]);
	static int MessageLoop();
	static void HandleEvent(Common::Event *event);

	static AnsiString GetSystemFontPath();
	static AnsiString GetPlatformName();

	// Win32 API bindings
	static HINSTANCE ShellExecute(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory, INT nShowCmd);
	static void OutputDebugString(LPCSTR lpOutputString);
	static uint32 GetTime();
	static BOOL GetCursorPos(LPPOINT lpPoint);
	static BOOL SetCursorPos(int X, int Y);
	static BOOL ShowWindow(HWND hWnd, int nCmdShow);
	static bool DeleteFile(const char *lpFileName);
	static bool CopyFile(const char *from, const char *to, bool failIfExists);
	static HWND SetCapture(HWND hWnd);
	static BOOL ReleaseCapture();
	static BOOL SetForegroundWindow(HWND hWnd);

	static BOOL SetRectEmpty(LPRECT lprc);
	static BOOL IsRectEmpty(const LPRECT lprc);
	static BOOL PtInRect(LPRECT lprc, POINT p);
	static BOOL SetRect(LPRECT lprc, int left, int top, int right, int bottom);
	static BOOL IntersectRect(LPRECT lprcDst, const LPRECT lprcSrc1, const LPRECT lprcSrc2);
	static BOOL UnionRect(LPRECT lprcDst, RECT *lprcSrc1, RECT *lprcSrc2);
	static BOOL CopyRect(LPRECT lprcDst, RECT *lprcSrc);
	static BOOL OffsetRect(LPRECT lprc, int dx, int dy);
	static BOOL EqualRect(LPRECT rect1, LPRECT rect2);


	// string functions
//	static int stricmp(const char *str1, const char *str2);
//	static int strnicmp(const char *str1, const char *str2, size_t maxCount);
	static char *strupr(char *string);
	static char *strlwr(char *string);

	// sdl event callback
	static int SDLEventWatcher(void *userdata, Common::Event *event);

private:
	static CBGame *Game;
};

} // end of namespace WinterMute

#endif
