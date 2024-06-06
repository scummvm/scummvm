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

/* ---------------------------- INCLUDE SECTION ----------------------------- */
#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/rect.h"
#include "qdengine/xlibs/xutil/xglobal.h"


namespace QDEngine {

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */

extern void* hXConOutput;
extern void* hXConInput;

/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

#define _DEF_RADIX  10
#define _DEF_TAB    8
#define _DEF_DIGITS 8

//char _ConvertBuffer[_CONV_BUFFER_LEN + 1];

char* XConTitleString = "XConsole";
#define STD_OUTPUT_HANDLE ((DWORD)-11)
#define STD_INPUT_HANDLE ((DWORD)-10)

void XConsole::initialize(int mode) {
	warning("XConsole::initialize()");
#if 0
	AllocConsole();
	SetConsoleTitle(XConTitleString);
	hXConOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	hXConInput = GetStdHandle(STD_INPUT_HANDLE);
#endif
}

void XConsole::UpdateBuffer(const char* p) {
	DWORD cCharsWritten;
	warning("XConsole::UpdateBuffer()");
#if 0
	WriteConsole(hXConOutput, p, strlen(p), &cCharsWritten, NULL);
#endif
}

void XConsole::clear() {
	int numChars;
	warning("XConsole::clear()");
#if 0
	CONSOLE_SCREEN_BUFFER_INFO conInfo;
	Common::Point coordScreen = {0, 0};
	DWORD cCharsWritten;

	GetConsoleScreenBufferInfo(hXConOutput, &conInfo);
	numChars = (conInfo.dwSize.X * conInfo.dwSize.Y) - conInfo.dwSize.X;

	FillConsoleOutputCharacter(hXConOutput, 0x20, numChars, coordScreen, &cCharsWritten);
#endif
	setpos(0, 0);
}

void XConsole::getpos(int &x, int &y) {
	warning("XConsole::getpos()");
#if 0
	CONSOLE_SCREEN_BUFFER_INFO conInfo;
	GetConsoleScreenBufferInfo(hXConOutput, &conInfo);

	x = conInfo.dwCursorPosition.X;
	y = conInfo.dwCursorPosition.Y;
#endif
}

void XConsole::setpos(int x, int y) {
	Common::Point pos;
	pos.x = x;
	pos.y = y;
	warning("XConsole::setpos()");
#if 0
	SetConsoleCursorPosition(hXConOutput, pos);
#endif
}

XConsole::XConsole(void) {
	radix   = _DEF_RADIX;
	digits  = _DEF_DIGITS;
	tab = _DEF_TAB;
}

XConsole::~XConsole(void) {
}
} // namespace QDEngine
