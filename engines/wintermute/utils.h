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

#include "wintypes.h"

namespace WinterMute {

class CBGame;

class CBUtils {
public:
	static void Clip(int *DestX, int *DestY, RECT *SrcRect, RECT *DestRect);
	static void Swap(int *a, int *b);
	static bool StrBeginsI(const char *String, const char *Fragment);
	static float NormalizeAngle(float Angle);

	static void CreatePath(const char *Path, bool PathOnly = false);

	static void DebugMessage(HWND hWnd, const char *Text);
	static char *SetString(char **String, const char *Value);

	static int StrNumEntries(const char *Str, const char Delim = ',');
	static char *StrEntry(int Entry, const char *Str, const char Delim = ',');

	static int RandomInt(int From, int To);
	static float RandomFloat(float From, float To);
	static float RandomAngle(float From, float To);

	static bool MatchesPattern(const char *pattern, const char *string);

	static char *GetPath(const char *Filename);
	static char *GetFilename(const char *Filename);

	static void RGBtoHSL(uint32 RGBColor, byte *OutH, byte *OutS, byte *OutL);
	static uint32 HSLtoRGB(byte  H, byte S, byte L);

private:
	static float Hue2RGB(float v1, float v2, float vH);
};

} // end of namespace WinterMute

#endif
