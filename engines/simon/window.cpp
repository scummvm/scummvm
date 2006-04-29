/* ScummVM - Scumm Interpreter
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

#include "simon/simon.h"
#include "simon/intern.h"

namespace Simon {

uint SimonEngine::getWindowNum(WindowBlock *window) {
	uint i;

	for (i = 0; i != ARRAYSIZE(_windowArray); i++)
		if (_windowArray[i] == window)
			return i;

	error("getWindowNum: not found");
	return 0;
}

WindowBlock *SimonEngine::openWindow(uint x, uint y, uint w, uint h, uint flags, uint fill_color, uint text_color) {
	WindowBlock *window;

	window = _windowList;
	while (window->mode != 0)
		window++;

	window->mode = 2;
	window->x = x;
	window->y = y;
	window->width = w;
	window->height = h;
	window->flags = flags;
	window->fill_color = fill_color;
	window->text_color = text_color;
	window->textColumn = 0;
	window->textRow = 0;
	window->textColumnOffset = 0;
	window->textMaxLength = window->width * 8 / 6; // characters are 6 pixels
	window->scrollY = 0;
	return window;
}

void SimonEngine::changeWindow(uint a) {
	a &= 7;

	if (_windowArray[a] == NULL || _curWindow == a)
		return;

	_curWindow = a;
	showmessage_print_char(0);
	_textWindow = _windowArray[a];

	if (getGameType() == GType_FF)
		showmessage_helper_3(_textWindow->textColumn, _textWindow->width);
	else
		showmessage_helper_3(_textWindow->textLength, _textWindow->textMaxLength);
}

void SimonEngine::closeWindow(uint a) {
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

void SimonEngine::clearWindow(WindowBlock *window) {
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

void SimonEngine::colorWindow(WindowBlock *window) {
	byte *dst;
	uint h, w;

	_lockWord |= 0x8000;

	if (getGameType() == GType_FF) {
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

		do {
			memset(dst, window->fill_color, w);
			dst += _dxSurfacePitch;
		} while (--h);
	}

	_lockWord &= ~0x8000;
}

void SimonEngine::resetWindow(WindowBlock *window) {
	if (window->flags & 8)
		restoreWindow(window);
	window->mode = 0;
}

void SimonEngine::restoreWindow(WindowBlock *window) {
	_lockWord |= 0x8000;

	if (getGameType() == GType_FF) {
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

void SimonEngine::restoreBlock(uint h, uint w, uint y, uint x) {
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

void SimonEngine::windowPutChar(uint a) {
	if (_textWindow != _windowArray[0])
		windowPutChar(_textWindow, a);
}

} // End of namespace Simon
