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
	if (_enabled) {
		char tempBuf[256];
		size_t size;

		size = Common::vsprintf_s(tempBuf, format, argptr);

		if (size) {
			if ((size < sizeof(tempBuf) - 2) &&
				tempBuf[size - 1] != '\n') {
				tempBuf[size++] = '\n';
				tempBuf[size] = '\0';
			}
			return dumpit(tempBuf, size);
		}
	}
	return 0;
}

size_t Messager::operator()(const char *format, ...) {
	if (_enabled) {
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

	r.x = _atX;
	r.y = _atY;
	r.width = _atW;
	r.height = heightStatusF;

	_textPort->setColor(blackStatusF);
	_textPort->fillRect(r);
	_textPort->setColor(_atColor);
	_textPort->setStyle(0);
	_textPort->drawTextInBox(s, size, r, textPosLeft, Point16(2, 1));

	return 0;
}

StatusLineMessager::StatusLineMessager(const char *entry, int lineno, gDisplayPort *mp, int32 x, int32 y, int32 w, int16 color)
	: Messager(entry) {
	_line = lineno;
	_textPort = mp;
	_atX = (x >= 0 ? x : defaultStatusFX);
	_atY = (y >= 0 ? y : defaultStatusFY + _line * heightStatusF);
	_atW = (w >= 0 ? w : 640 - (defaultStatusFX - 16) - 20);
	_atColor = (color >= 0 ? color : _line * 16 + 12);
	operator()("Status Line %d", _line);
}

StatusLineMessager::StatusLineMessager(int lineno, gDisplayPort *mp, int32 x, int32 y, int32 w, int16 color) {
	_line = lineno;
	_textPort = mp;
	_atX = (x >= 0 ? x : defaultStatusFX);
	_atY = (y >= 0 ? y : defaultStatusFY + _line * heightStatusF);
	_atW = (w >= 0 ? w : 640 - (defaultStatusFX - 16) - 20);
	_atColor = (color >= 0 ? color : _line * 16 + 12);
	operator()("Status Line %d", _line);
}

StatusLineMessager::~StatusLineMessager() {
	operator()("");
}

} // end of namespace Saga2
