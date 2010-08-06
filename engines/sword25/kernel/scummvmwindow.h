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
	BS_ScummVMWindow
	----------------
	Implementation of the BS_Window Interfaces for ScummVM
*/

#ifndef SWORD25_SCUMMVMWINDOW_H
#define SWORD25_SCUMMVMWINDOW_H

// Includes
#include "sword25/kernel/common.h"
#include "sword25/kernel/window.h"

namespace Sword25 {

// Class definition
class BS_ScummVMWindow : public BS_Window {
public:
	BS_ScummVMWindow(int X, int Y, int Width, int Height, bool Visible);
	virtual ~BS_ScummVMWindow();

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
	Common::String GetTitle();
	void SetWindowAlive(bool v);
	void SetTitle(const Common::String &Title);
	bool HasFocus();
	uint GetWindowHandle();
	bool WaitForFocus();
	bool ProcessMessages();

private:
	static bool _ClassRegistered;
	bool _WindowAlive;	
	int	 _ClientXDelta;
	int	 _ClientYDelta;
};

} // End of namespace Sword25

#endif
