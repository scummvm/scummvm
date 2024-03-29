/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_PLATFORMSDL_H
#define WINTERMUTE_PLATFORMSDL_H

#include "engines/wintermute/dctypes.h"
#include "engines/wintermute/math/rect32.h"
#include "common/events.h"

namespace Wintermute {

class BaseGame;
class WintermuteEngine;
//////////////////////////////////////////////////////////////////////////
class BasePlatform {
public:
	static int initialize(WintermuteEngine *engineRef, BaseGame *inGame, int argc, char *argv[]);
	static void deinit();
	static void handleEvent(Common::Event *event);
	static AnsiString getPlatformName();

	// Win32 API bindings
	static bool getCursorPos(Point32 *lpPoint);
	static bool setCursorPos(int x, int y);

	static bool ptInRect(Rect32 *lprc, Point32 p);
	static bool setRect(Rect32 *lprc, int left, int top, int right, int bottom);
	static bool intersectRect(Rect32 *lprcDst, const Rect32 *lprcSrc1, const Rect32 *lprcSrc2);
	static bool unionRect(Rect32 *lprcDst, Rect32 *lprcSrc1, Rect32 *lprcSrc2);
	static bool copyRect(Rect32 *lprcDst, Rect32 *lprcSrc);

	// string functions
	static char *wintermute_strlwr(char *string);

private:
	// Set by initialize on game-startup, the object referred to is also deleted by deinit in WintermuteEngine
	static BaseGame *_gameRef;
	static WintermuteEngine *_engineRef;
};

} // End of namespace Wintermute

#endif
