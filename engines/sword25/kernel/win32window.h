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
 * $URL$
 * $Id$
 *
 */

/* 
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

/*
	BS_Win32Window
	-------------
	Implementation des BS_Window Interfaces für Win32.
	Zu den einzelnen Methoden bitte "window.h" konsultieren.

	Autor: Malte Thiesen
*/

#ifndef SWORD25_WIN32WINDOW_H
#define SWORD25_WIN32WINDOW_H

// Includes
#include "sword25/kernel/memlog_off.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "sword25/kernel/memlog_on.h"

#include "sword25/kernel/common.h"
#include "sword25/kernel/window.h"

// Klassendefinition
class BS_Win32Window : public BS_Window
{
public:
	BS_Win32Window(int X, int Y, int Width, int Height, bool Visible);
	virtual ~BS_Win32Window();

	bool IsVisible();
	void SetVisible(bool Visible);
	int GetX();
	void SetX(int X);
	int GetY();
	void SetY(int X);
	int GetClientX();
	int GetClientY();
	int GetWidth();
	void SetWidth(int Width);
	int GetHeight();
	void SetHeight(int Height);
	std::string GetTitle();
	void SetTitle(std::string Title);
	bool HasFocus();
	UINT GetWindowHandle();
	bool WaitForFocus();
	bool ProcessMessages();

private:
	static bool _ClassRegistered;
	bool _WindowAlive;	
	HWND _Window;
	int	 _ClientXDelta;
	int	 _ClientYDelta;

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif