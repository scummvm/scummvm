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

#include "dcgf.h"
#include "utils.h"
#include "PlatformSDL.h"
#include "wintypes.h"
#include "PathUtil.h"
#include "BGame.h"
#include "common/str.h"
#include "common/textconsole.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////
static inline unsigned Sqr(int x) {
	return (x * x);
}


//////////////////////////////////////////////////////////////////////////////////
void CBUtils::Clip(int *DestX, int *DestY, RECT *SrcRect, RECT *DestRect) {
	// If it's partly off the right side of the screen
	if (*DestX + (SrcRect->right - SrcRect->left) > DestRect->right)
		SrcRect->right -= *DestX + (SrcRect->right - SrcRect->left) - DestRect->right;

	if (SrcRect->right < 0) SrcRect->right = 0;

	// Partly off the left side of the screen
	if (*DestX < DestRect->left) {
		SrcRect->left += DestRect->left - *DestX;
		*DestX = DestRect->left;
	}

	// Partly off the top of the screen
	if (*DestY < DestRect->top) {
		SrcRect->top += DestRect->top - *DestY;
		*DestY = DestRect->top;
	}

	// If it's partly off the bottom side of the screen
	if (*DestY + (SrcRect->bottom - SrcRect->top) > DestRect->bottom)
		SrcRect->bottom -= ((SrcRect->bottom - SrcRect->top) + *DestY) - DestRect->bottom;

	if (SrcRect->bottom < 0) SrcRect->bottom = 0;

	return;
}

//////////////////////////////////////////////////////////////////////////////////
// Swap - swaps two integers
//////////////////////////////////////////////////////////////////////////////////
void CBUtils::Swap(int *a, int *b) {
	int Temp = *a;
	*a = *b;
	*b = Temp;
}

//////////////////////////////////////////////////////////////////////////
bool CBUtils::StrBeginsI(const char *String, const char *Fragment) {
	return (scumm_strnicmp(String, Fragment, strlen(Fragment)) == 0);
}


//////////////////////////////////////////////////////////////////////////
float CBUtils::NormalizeAngle(float Angle) {
	while (Angle > 360) Angle -= 360;
	while (Angle < 0) Angle += 360;

	return Angle;
}


////////////////////////////////////////////////////////////////////////////////
void CBUtils::CreatePath(const char *Path, bool PathOnly) {
	AnsiString path;

	if (!PathOnly) path = PathUtil::GetDirectoryName(Path);
	else path = Path;

//	try {
	warning("CBUtils::CreatePath - not implemented: %s", Path);
//		boost::filesystem::create_directories(path);
//	} catch (...) {
		return;
//	}
}


//////////////////////////////////////////////////////////////////////////
void CBUtils::DebugMessage(HWND hWnd, const char *Text) {
	//MessageBox(hWnd, Text, "WME", MB_OK|MB_ICONINFORMATION);
}


//////////////////////////////////////////////////////////////////////////
char *CBUtils::SetString(char **String, const char *Value) {
	delete[] *String;
	*String = new char[strlen(Value) + 1];
	if (*String) strcpy(*String, Value);
	return *String;
}

//////////////////////////////////////////////////////////////////////////
int CBUtils::StrNumEntries(const char *Str, const char Delim) {
	int NumEntries = 1;
	for (int i = 0; i < strlen(Str); i++) {
		if (Str[i] == Delim) NumEntries++;
	}
	return NumEntries;
}


//////////////////////////////////////////////////////////////////////////
char *CBUtils::StrEntry(int Entry, const char *Str, const char Delim) {
	int NumEntries = 0;

	const char *Start = NULL;
	int Len = 0;

	for (int i = 0; i <= strlen(Str); i++) {
		if (NumEntries == Entry) {
			if (!Start) Start = Str + i;
			else Len++;
		}
		if (Str[i] == Delim || Str[i] == '\0') {
			NumEntries++;
			if (Start) {
				char *Ret = new char[Len + 1];
				memset(Ret, 0, Len + 1);
				strncpy(Ret, Start, Len);
				return Ret;
			}
		}
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
int CBUtils::RandomInt(int From, int To) {
	if (To < From) {
		int i = To;
		To = From;
		From = i;
	}
	return (rand() % (To - From + 1)) + From;
}

//////////////////////////////////////////////////////////////////////////
float CBUtils::RandomFloat(float From, float To) {
	float RandNum = (float)rand() / (float)RAND_MAX;
	return From + (To - From) * RandNum;
}

//////////////////////////////////////////////////////////////////////////
float CBUtils::RandomAngle(float From, float To) {
	while (To < From) {
		To += 360;
	}
	return NormalizeAngle(RandomFloat(From, To));
}

//////////////////////////////////////////////////////////////////////////
bool CBUtils::MatchesPattern(const char *Pattern, const char *String) {
	char stringc, patternc;

	for (;; ++String) {
		stringc = toupper(*String);
		patternc = toupper(*Pattern++);

		switch (patternc) {
		case 0:
			return (stringc == 0);

		case '?':
			if (stringc == 0) return false;
			break;

		case '*':
			if (!*Pattern) return true;

			if (*Pattern == '.') {
				char *dot;
				if (Pattern[1] == '*' && Pattern[2] == 0) return true;
				dot = (char *)strchr(String, '.');
				if (Pattern[1] == 0) return (dot == NULL || dot[1] == 0);
				if (dot != NULL) {
					String = dot;
					if (strpbrk(Pattern, "*?[") == NULL && strchr(String + 1, '.') == NULL)
						return(scumm_stricmp(Pattern + 1, String + 1) == 0);
				}
			}

			while (*String)
				if (CBUtils::MatchesPattern(Pattern, String++))
					return true;
			return false;

		default:
			if (patternc != stringc)
				if (patternc == '.' && stringc == 0)
					return(CBUtils::MatchesPattern(Pattern, String));
				else
					return false;
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
char *CBUtils::GetPath(char *Filename) {
	AnsiString path = PathUtil::GetDirectoryName(Filename);
	//path = boost::filesystem::system_complete(path).string();
	warning("CBUtils::GetPath: (%s), not implemented", Filename);
	return Filename;
	char *ret = new char[path.length() + 1];
	strcpy(ret, path.c_str());

	return ret;
}

//////////////////////////////////////////////////////////////////////////
char *CBUtils::GetFilename(char *Filename) {
	AnsiString path = PathUtil::GetFileName(Filename);
	char *ret = new char[path.length() + 1];
	strcpy(ret, path.c_str());
	return ret;
}

//////////////////////////////////////////////////////////////////////////
void CBUtils::RGBtoHSL(uint32 RGBColor, byte *OutH, byte *OutS, byte *OutL) {
	float var_R = (D3DCOLGetR(RGBColor) / 255.0f);
	float var_G = (D3DCOLGetG(RGBColor) / 255.0f);
	float var_B = (D3DCOLGetB(RGBColor) / 255.0f);

	//Min. value of RGB
	float var_Min = std::min(var_R, var_G);
	var_Min = std::min(var_Min, var_B);

	//Max. value of RGB
	float var_Max = std::max(var_R, var_G);
	var_Max = std::max(var_Max, var_B);

	//Delta RGB value
	float del_Max = var_Max - var_Min;

	float H, S, L;

	L = (var_Max + var_Min) / 2.0f;

	//This is a gray, no chroma...
	if (del_Max == 0) {
		H = 0;
		S = 0;
	}
	//Chromatic data...
	else {
		if (L < 0.5f) S = del_Max / (var_Max + var_Min);
		else S = del_Max / (2.0f - var_Max - var_Min);

		float del_R = (((var_Max - var_R) / 6.0f) + (del_Max / 2.0f)) / del_Max;
		float del_G = (((var_Max - var_G) / 6.0f) + (del_Max / 2.0f)) / del_Max;
		float del_B = (((var_Max - var_B) / 6.0f) + (del_Max / 2.0f)) / del_Max;

		if (var_R == var_Max) H = del_B - del_G;
		else if (var_G == var_Max) H = (1.0f / 3.0f) + del_R - del_B;
		else if (var_B == var_Max) H = (2.0f / 3.0f) + del_G - del_R;

		if (H < 0) H += 1;
		if (H > 1) H -= 1;
	}

	*OutH = H * 255;
	*OutS = S * 255;
	*OutL = L * 255;
}


//////////////////////////////////////////////////////////////////////////
uint32 CBUtils::HSLtoRGB(byte  InH, byte InS, byte InL) {
	float H = InH / 255.0f;
	float S = InS / 255.0f;
	float L = InL / 255.0f;

	byte R, G, B;


	if (S == 0) {
		R = L * 255;
		G = L * 255;
		B = L * 255;
	} else {
		float var_1, var_2;

		if (L < 0.5) var_2 = L * (1.0 + S);
		else var_2 = (L + S) - (S * L);

		var_1 = 2.0f * L - var_2;

		R = 255 * Hue2RGB(var_1, var_2, H + (1.0f / 3.0f));
		G = 255 * Hue2RGB(var_1, var_2, H);
		B = 255 * Hue2RGB(var_1, var_2, H - (1.0f / 3.0f));
	}
	return DRGBA(255, R, G, B);
}


//////////////////////////////////////////////////////////////////////////
float CBUtils::Hue2RGB(float v1, float v2, float vH) {
	if (vH < 0.0f) vH += 1.0f;
	if (vH > 1.0f) vH -= 1.0f;
	if ((6.0f * vH) < 1.0f) return (v1 + (v2 - v1) * 6.0f * vH);
	if ((2.0f * vH) < 1.0f) return (v2);
	if ((3.0f * vH) < 2.0f) return (v1 + (v2 - v1) * ((2.0f / 3.0f) - vH) * 6.0f);
	return (v1);
}

} // end of namespace WinterMute
