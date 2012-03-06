/*
This file is part of WME Lite.
http://dead-code.org/redir.php?target=wmelite

Copyright (c) 2011 Jan Nedoma

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef __WmeUtils_H__
#define __WmeUtils_H__

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

	static char *GetPath(char *Filename);
	static char *GetFilename(char *Filename);

	static void RGBtoHSL(uint32 RGBColor, byte *OutH, byte *OutS, byte *OutL);
	static uint32 HSLtoRGB(byte  H, byte S, byte L);

private:
	static float Hue2RGB(float v1, float v2, float vH);
};

} // end of namespace WinterMute

#endif
