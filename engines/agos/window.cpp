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
 * $URL$
 * $Id$
 *
 */



#include "common/system.h"

#include "graphics/surface.h"

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
	window->textColumnOffset = 0;
	window->textRow = 0;
	window->scrollY = 0;

	// Characters are 6 pixels
	if (getGameType() == GType_ELVIRA2)
		window->textMaxLength = (window->width * 8 - 4) / 6;
	else
		window->textMaxLength = window->width * 8 / 6;

	if (getGameType() == GType_ELVIRA1 || getGameType() == GType_ELVIRA2 || getGameType() == GType_WW)
		clearWindow(window);

	if (getGameType() == GType_SIMON1 && getPlatform() == Common::kPlatformAmiga && window->fill_color == 225)
		window->fill_color = (getFeatures() & GF_32COLOR) ? 17 : 241;

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
	window->textColumnOffset = (getGameType() == GType_ELVIRA2) ? 4 : 0;
	window->textLength = 0;
	window->scrollY = 0;
}

void AGOSEngine_Feeble::colorWindow(WindowBlock *window) {
	byte *dst;
	uint16 h, w;

	_lockWord |= 0x8000;

	dst = getBackGround() + _dxSurfacePitch * window->y + window->x;

	for (h = 0; h < window->height; h++) {
		for (w = 0; w < window->width; w++) {
			if (dst[w] == 113  || dst[w] == 116 || dst[w] == 252)
				dst[w] = window->fill_color;
		}
		dst += _screenWidth;
	}

	_lockWord &= ~0x8000;
}

void AGOSEngine::colorWindow(WindowBlock *window) {
	uint16 y, h;

	y = window->y;
	h = window->height * 8;

	if (getGameType() == GType_ELVIRA2 && window->y == 146) {
		if (window->fill_color == 1) {
			_displayPalette[33 * 4 + 0] = 48 * 4;
			_displayPalette[33 * 4 + 1] = 40 * 4;
			_displayPalette[33 * 4 + 2] = 32 * 4;
		} else {
			_displayPalette[33 * 4 + 0] = 56 * 4;
			_displayPalette[33 * 4 + 1] = 56 * 4;
			_displayPalette[33 * 4 + 2] = 40 * 4;
		}

		y--;
		h += 2;

		_paletteFlag = 1;
	}

	colorBlock(window, window->x * 8, y, window->width * 8, h);
}

void AGOSEngine::colorBlock(WindowBlock *window, uint16 x, uint16 y, uint16 w, uint16 h) {
	_lockWord |= 0x8000;

	Graphics::Surface *screen = _system->lockScreen();
	byte *dst = (byte *)screen->pixels + y * _screenWidth + x;

	uint8 color = window->fill_color;
	if (getGameType() == GType_ELVIRA2 || getGameType() == GType_WW)
		color += dst[0] & 0xF0;

	do {
		memset(dst, color, w);
		dst += _screenWidth;
	} while (--h);

	_system->unlockScreen();

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
	} else if (getGameType() == GType_SIMON1) {
		restoreBlock(window->y + window->height * 8 + ((window == _windowArray[2]) ? 1 : 0), (window->x + window->width) * 8, window->y, window->x * 8);
	} else {
		uint16 x = window->x;
		uint16 w = window->width;

		if (getGameType() == GType_ELVIRA1) {
			// Adjustments to remove inventory arrows
			if (x & 1) {
				x--;
				w++;
			}
			if (w & 1) {
				w++;
			}
		}

		restoreBlock(window->y + window->height * 8, (x + w) * 8, window->y, x * 8);
	}

	_lockWord &= ~0x8000;
}

void AGOSEngine::restoreBlock(uint16 h, uint16 w, uint16 y, uint16 x) {
	byte *dst, *src;
	uint i;

	Graphics::Surface *screen = _system->lockScreen();
	dst = (byte *)screen->pixels;
	src = getBackGround();

	dst += y * _dxSurfacePitch;
	src += y * _dxSurfacePitch;

	uint8 paletteMod = 0;
	if (getGameType() == GType_ELVIRA1 && !(getFeatures() & GF_DEMO) && y >= 133)
		paletteMod = 16;

	while (y < h) {
		for (i = x; i < w; i++)
			dst[i] = src[i] + paletteMod;
		y++;
		dst += _dxSurfacePitch;
		src += _dxSurfacePitch;
	}

	_system->unlockScreen();
}

void AGOSEngine::setTextColor(uint color) {
	WindowBlock *window = _windowArray[_curWindow];

	if ((getFeatures() & GF_32COLOR) && color != 0) {
		if (window->fill_color == 17)
			color = 25;
		else
			color = 220;
	}

	window->text_color = color;
}

void AGOSEngine::sendWindow(uint a) {
	if (_textWindow != _windowArray[0]) {
		if (getGameType() == GType_ELVIRA1 || getGameType() == GType_ELVIRA2 || getGameType() == GType_WW) {
			if (!(_textWindow->flags & 1)) {
				haltAnimation();
			}
		}

		windowPutChar(_textWindow, a);
	}
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
	ha->x = (window->width / 2 + window->x - 3) * 8;
	ha->y = window->height * 8 + window->y - 8;
	ha->width = 48;
	ha->height = 8;
	ha->flags = kBFBoxInUse;
	ha->id = 0x7FFF;
	ha->priority = 999;

	while (!shouldQuit()) {
		_lastHitArea = NULL;
		_lastHitArea3 = NULL;

		while (!shouldQuit()) {
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

void AGOSEngine::writeChar(WindowBlock *window, int x, int y, int offs, int val) {
	int chr;

	// Clear background of first digit
	window->textColumnOffset = offs;
	window->text_color = 0;
	windowDrawChar(window, x * 8, y, 129);

	if (val != -1) {
		// Print first digit
		chr = val / 10 + 48;
		window->text_color = 15;
		windowDrawChar(window, x * 8, y, chr);
	}

	offs += 6;
	if (offs >= 7) {
		offs -= 8;
		x++;
	}

	// Clear background of second digit
	window->textColumnOffset = offs;
	window->text_color = 0;
	windowDrawChar(window, x * 8, y, 129);

	if (val != -1) {
		// Print second digit
		chr = val % 10 + 48;
		window->text_color = 15;
		windowDrawChar(window, x * 8, y, chr);
	}
}

} // End of namespace AGOS
