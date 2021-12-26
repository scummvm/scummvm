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
 * aint32 with this program; if not, write to the Free Software
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "saga2/saga2.h"
#include "saga2/messager.h"
#include "saga2/vdraw.h"

namespace Saga2 {

size_t Messager::va(const char *format, va_list argptr) {
	if (enabled) {
		char tempBuf[256];
		size_t size;

		size = vsprintf((char *) tempBuf, format, argptr);

		if (size) {
			if (tempBuf[size - 1] != '\n') {
				tempBuf[size++] = '\n';
				tempBuf[size] = '\0';
			}
			return dumpit(tempBuf, size);
		}
	}
	return 0;
}

size_t Messager::operator()(const char *format, ...) {
	if (enabled) {
		size_t size;
		va_list argptr;

		va_start(argptr, format);
		size = va(format, argptr);
		va_end(argptr);
		return size;
	}
	return 0;
}


uint16 defaultStatusFX = 468;
uint16 defaultStatusFY = 354;
uint16 blackStatusF = 24;
uint16 heightStatusF = 11;

int StatusLineMessager::dumpit(char *s, size_t size) {
	Rect16          r;

	r.x = atX;
	r.y = atY;
	r.width = atW;
	r.height = heightStatusF;

	textPort->setColor(blackStatusF);
	textPort->fillRect(r);
	textPort->setColor(atColor);
	textPort->setStyle(0);
	textPort->drawTextInBox(s, size, r, textPosLeft, Point16(2, 1));

	return 0;
}

StatusLineMessager::StatusLineMessager(const char *entry, int lineno, gDisplayPort *mp, int32 x, int32 y, int32 w, int16 color)
	: Messager(entry) {
	line = lineno;
	textPort = mp;
	atX = (x >= 0 ? x : defaultStatusFX);
	atY = (y >= 0 ? y : defaultStatusFY + line * heightStatusF);
	atW = (w >= 0 ? w : 640 - (defaultStatusFX - 16) - 20);
	atColor = (color >= 0 ? color : line * 16 + 12);
	operator()("Status Line %d", line);
}

StatusLineMessager::StatusLineMessager(int lineno, gDisplayPort *mp, int32 x, int32 y, int32 w, int16 color) {
	line = lineno;
	textPort = mp;
	atX = (x >= 0 ? x : defaultStatusFX);
	atY = (y >= 0 ? y : defaultStatusFY + line * heightStatusF);
	atW = (w >= 0 ? w : 640 - (defaultStatusFX - 16) - 20);
	atColor = (color >= 0 ? color : line * 16 + 12);
	operator()("Status Line %d", line);
}

StatusLineMessager::~StatusLineMessager() {
	operator()("");
}

} // end of namespace Saga2
