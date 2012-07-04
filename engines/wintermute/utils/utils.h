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

#ifndef WINTERMUTE_UTILS_H
#define WINTERMUTE_UTILS_H

#include "engines/wintermute/wintypes.h"

namespace WinterMute {

class CBGame;

class CBUtils {
public:
	static void clip(int *DestX, int *DestY, RECT *SrcRect, RECT *DestRect);
	static void swap(int *a, int *b);
	static bool strBeginsI(const char *String, const char *Fragment);
	static float normalizeAngle(float Angle);

	static void createPath(const char *Path, bool PathOnly = false);

	static void debugMessage(HWND hWnd, const char *Text);
	static char *setString(char **String, const char *Value);

	static int strNumEntries(const char *Str, const char Delim = ',');
	static char *strEntry(int Entry, const char *Str, const char Delim = ',');

	static int randomInt(int From, int To);
	static float randomFloat(float From, float To);
	static float randomAngle(float From, float To);

	static bool matchesPattern(const char *pattern, const char *string);

	static char *getPath(const char *filename);
	static char *getFilename(const char *filename);

	static void RGBtoHSL(uint32 RGBColor, byte *OutH, byte *OutS, byte *OutL);
	static uint32 HSLtoRGB(byte  H, byte S, byte L);

private:
	static float Hue2RGB(float v1, float v2, float vH);
};

} // end of namespace WinterMute

#endif
