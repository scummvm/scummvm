/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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

#include "common/stdafx.h"

#include "agos/agos.h"
#include "agos/intern.h"

namespace AGOS {

uint AGOSEngine::getWindowNum(WindowBlock *window) {
	uint i;

	for (i = 0; i != ARRAYSIZE(_windowArray); i++)
		if (_windowArray[i] == window)
			return i;

	error("getWindowNum: not found");
	return 0;
}

WindowBlock *AGOSEngine::openWindow(uint x, uint y, uint w, uint h, uint flags, uint fillColor, uint textColor) {
	WindowBlock *window;

	window = _windowList;
	while (window->mode != 0)
		window++;

	if (getGameType() == GType_ELVIRA1 && y >= 133)
		textColor += 16;

	window->mode = 2;
	window->x = x;
	window->y = y;
	window->width = w;
	window->height = h;
	window->flags = flags;
	window->fill_color = fillColor;
	window->text_color = textColor;
	window->textColumn = 0;
	window->textRow = 0;
	window->textColumnOffset = 0;
	window->textMaxLength = window->width * 8 / 6; // characters are 6 pixels
	window->scrollY = 0;

	if (getGameType() == GType_ELVIRA1 || getGameType() == GType_ELVIRA2 || getGameType() == GType_WW)
		clearWindow(window);

	return window;
}

void AGOSEngine::changeWindow(uint a) {
	a &= 7;

	if (_windowArray[a] == NULL || _curWindow == a)
		return;

	_curWindow = a;
	justifyOutPut(0);
	_textWindow = _windowArray[a];
	justifyStart();
}

void AGOSEngine::closeWindow(uint a) {
	if (_windowArray[a] == NULL)
		return;
	removeIconArray(a);
	resetWindow(_windowArray[a]);
	_windowArray[a] = NULL;
	if (_curWindow == a) {
		_textWindow = NULL;
		changeWindow(0);
	}
}

void AGOSEngine::clearWindow(WindowBlock *window) {
	if (window->flags & 0x10)
		restoreWindow(window);
	else
		colorWindow(window);

	window->textColumn = 0;
	window->textRow = 0;
	window->textColumnOffset = 0;
	window->textLength = 0;
	window->scrollY = 0;
}

void AGOSEngine::colorWindow(WindowBlock *window) {
	byte *dst;
	uint h, w;

	_lockWord |= 0x8000;

	if (getGameType() == GType_FF || getGameType() == GType_PP) {
		dst = getFrontBuf() + _dxSurfacePitch * window->y + window->x;

		for (h = 0; h < window->height; h++) {
			for (w = 0; w < window->width; w++) {
				if (dst[w] == 113  || dst[w] == 116 || dst[w] == 252)
					dst[w] = window->fill_color;
			}
			dst += _screenWidth;
		}
	} else {
		dst = getFrontBuf() + _dxSurfacePitch * window->y + window->x * 8;
		h = window->height * 8;
		w = window->width * 8;

		uint8 color = window->fill_color;
		if (getGameType() == GType_ELVIRA2 || getGameType() == GType_WW)
			color += dst[0] & 0xF0;

		do {
			memset(dst, color, w);
			dst += _dxSurfacePitch;
		} while (--h);
	}

	_lockWord &= ~0x8000;
}

void AGOSEngine::resetWindow(WindowBlock *window) {
	if (window->flags & 8)
		restoreWindow(window);
	window->mode = 0;
}

void AGOSEngine::restoreWindow(WindowBlock *window) {
	_lockWord |= 0x8000;

	if (getGameType() == GType_FF || getGameType() == GType_PP) {
		restoreBlock(window->y + window->height, window->x + window->width, window->y, window->x);
	} else if (getGameType() == GType_SIMON2) {
		if (_restoreWindow6 && _windowArray[2] == window) {
			window = _windowArray[6];
			_restoreWindow6 = 0;
		}

		restoreBlock(window->y + window->height * 8, (window->x + window->width) * 8, window->y, window->x * 8);
	} else {
		restoreBlock(window->y + window->height * 8 + ((window == _windowArray[2]) ? 1 : 0), (window->x + window->width) * 8, window->y, window->x * 8);
	}

	_lockWord &= ~0x8000;
}

void AGOSEngine::restoreBlock(uint h, uint w, uint y, uint x) {
	byte *dst, *src;
	uint i;

	dst = getFrontBuf();
	src = _backGroundBuf;

	dst += y * _dxSurfacePitch;
	src += y * _dxSurfacePitch;

	while (y < h) {
		for (i = x; i < w; i++)
			dst[i] = src[i];
		y++;
		dst += _dxSurfacePitch;
		src += _dxSurfacePitch;
	}
}

void AGOSEngine::setTextColor(uint color) {
	WindowBlock *window = _windowArray[_curWindow];

	if ((getFeatures() & GF_32COLOR) && color != 0) {
		if (window->fill_color == 17)
			color = 25;
		else
			color = 12;
	}

	window->text_color = color;
}

void AGOSEngine::windowPutChar(uint a) {
	if (_textWindow != _windowArray[0])
		windowPutChar(_textWindow, a);
}

void AGOSEngine::waitWindow(WindowBlock *window) {
	HitArea *ha;
	const char *message;

	window->textColumn = (window->width / 2) - 3;
	window->textRow = window->height - 1;
	window->textLength = 0;

	message = "[ OK ]";
	for (; *message; message++)
		windowPutChar(window, *message);

	ha = findEmptyHitArea();
	ha->x = 96;
	ha->y = 62;
	ha->width = 48;
	ha->height = 8;
	ha->flags = kBFBoxInUse;
	ha->id = 0x7FFF;
	ha->priority = 999;

	for (;;) {
		_lastHitArea = NULL;
		_lastHitArea3 = NULL;

		for (;;) {
			if (_lastHitArea3 != 0)
				break;
			delay(1);
		}

		ha = _lastHitArea;
		if (ha == NULL) {
		} else if (ha->id == 0x7FFF) {
			break;
		}
	}

	undefineBox(0x7FFF);
}

} // End of namespace AGOS
