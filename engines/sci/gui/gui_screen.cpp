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

#include "common/timer.h"
#include "common/util.h"

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/tools.h"
#include "sci/gui/gui_screen.h"

namespace Sci {

SciGUIscreen::SciGUIscreen(OSystem *system, EngineState *state)
	: _system(system), _s(state) {
	init();
}

SciGUIscreen::~SciGUIscreen() {
}

void SciGUIscreen::init() {
	int i;
	uint16 base = 0;

	_width  = 320;
	_height = 200;
	_pixels = _width * _height;

	// if you want to do scaling, adjust Put_Pixel() accordingly
	_displayWidth = 320;
	_displayHeight = 200;
	_displayPixels = _displayWidth * _displayHeight;
	_bytesPerDisplayPixel = 1;

	_visualScreen = initScreen(_pixels);
	_priorityScreen = initScreen(_pixels);
	_controlScreen = initScreen(_pixels);
	_displayScreen = initScreen(_displayPixels);

	for (i = 0; i < _height; i++) {
		_baseTable[i] = base; _baseDisplayTable[i] = base;
		base += _width;
	}
}

byte *SciGUIscreen::initScreen(uint16 pixelCount) {
	byte *screen = (byte *)malloc(pixelCount);
	memset(screen, 0, pixelCount);
	return screen;
}

void SciGUIscreen::UpdateWhole() {
	_system->copyRectToScreen(_displayScreen, _displayWidth, 0, 0, _displayWidth, _displayHeight);
	_system->updateScreen();
}

byte SciGUIscreen::GetDrawingMask(byte color, byte prio, byte control) {
	byte flag = 0;
	if (color != 255)
		flag |= SCI_SCREEN_MASK_VISUAL;
	if (prio != 255)
		flag |= SCI_SCREEN_MASK_PRIORITY;
	if (control != 255)
		flag |= SCI_SCREEN_MASK_CONTROL;
	return flag;
}

void SciGUIscreen::Put_Pixel(int x, int y, byte drawMask, byte color, byte priority, byte control) {
	int offset = _baseTable[y] + x;

	if (drawMask & SCI_SCREEN_MASK_VISUAL) {
		if (!_s->resMan->isVGA()) {
			// EGA output (16 colors, dithered)
			color = ((x^y) & 1) ? color >> 4 : color & 0x0F;
		}
		*(_visualScreen + offset) = color;
		_displayScreen[_baseDisplayTable[y] + x] = color;
	}
	if (drawMask & SCI_SCREEN_MASK_PRIORITY)
		*(_priorityScreen + offset) = priority;
	if (drawMask & SCI_SCREEN_MASK_CONTROL)
		*(_controlScreen + offset) = control;
}

byte SciGUIscreen::Get_Visual(int x, int y) {
	return _visualScreen[_baseTable[y] + x];
}

byte SciGUIscreen::Get_Priority(int x, int y) {
	return _priorityScreen[_baseTable[y] + x];
}

byte SciGUIscreen::Get_Control(int x, int y) {
	return _controlScreen[_baseTable[y] + x];
}

byte SciGUIscreen::IsFillMatch(int16 x, int16 y, byte flag, byte t_color, byte t_pri, byte t_con) {
	int offset = _baseTable[y] + x;
	byte match = 0;

	if (flag & SCI_SCREEN_MASK_VISUAL && *(_visualScreen + offset) == t_color)
		match |= SCI_SCREEN_MASK_VISUAL;
	if (flag & SCI_SCREEN_MASK_PRIORITY && *(_priorityScreen + offset) == t_pri)
		match |= SCI_SCREEN_MASK_PRIORITY;
	if (flag & SCI_SCREEN_MASK_CONTROL && *(_controlScreen + offset) == t_con)
		match |= SCI_SCREEN_MASK_CONTROL;
	return match;
}

int SciGUIscreen::BitsGetDataSize(Common::Rect rect, byte mask) {
	int byteCount = sizeof(rect) + sizeof(mask);
	int pixels = rect.width() * rect.height();
	if (mask & SCI_SCREEN_MASK_VISUAL) {
		byteCount += pixels + (pixels * _bytesPerDisplayPixel);
	}
	if (mask & SCI_SCREEN_MASK_PRIORITY) {
		byteCount += pixels;
	}
	if (mask & SCI_SCREEN_MASK_CONTROL) {
		byteCount += pixels;
	}
	return byteCount;
}

void SciGUIscreen::BitsSave(Common::Rect rect, byte mask, byte *memoryPtr) {
	memcpy(memoryPtr, (void *)&rect, sizeof(rect)); memoryPtr += sizeof(rect);
	memcpy(memoryPtr, (void *)&mask, sizeof(mask)); memoryPtr += sizeof(mask);

	if (mask & SCI_SCREEN_MASK_VISUAL) {
		BitsSaveScreen(rect, _visualScreen, memoryPtr);
		BitsSaveScreen(rect, _displayScreen, memoryPtr);
	}
	if (mask & SCI_SCREEN_MASK_PRIORITY) {
		BitsSaveScreen(rect, _priorityScreen, memoryPtr);
	}
	if (mask & SCI_SCREEN_MASK_CONTROL) {
		BitsSaveScreen(rect, _controlScreen, memoryPtr);
	}
}

void SciGUIscreen::BitsSaveScreen(Common::Rect rect, byte *screen, byte *&memoryPtr) {
	int width = rect.width();
	int height = rect.height();
	int y;

	screen += (rect.top * _width) + rect.left;

	for (y=rect.top; y<rect.bottom; y++) {
		memcpy(memoryPtr, (void*)screen, width); memoryPtr += width;
		screen += _width;
	}
}

void SciGUIscreen::BitsRestore(byte *memoryPtr) {
	Common::Rect rect;
	byte mask;

	memcpy((void *)&rect, memoryPtr, sizeof(rect)); memoryPtr += sizeof(rect);
	memcpy((void *)&mask, memoryPtr, sizeof(mask)); memoryPtr += sizeof(mask);

	if (mask & SCI_SCREEN_MASK_VISUAL) {
		BitsRestoreScreen(rect, memoryPtr, _visualScreen);
		BitsRestoreScreen(rect, memoryPtr, _displayScreen);
	}
	if (mask & SCI_SCREEN_MASK_PRIORITY) {
		BitsRestoreScreen(rect, memoryPtr, _priorityScreen);
	}
	if (mask & SCI_SCREEN_MASK_CONTROL) {
		BitsRestoreScreen(rect, memoryPtr, _controlScreen);
	}
}

void SciGUIscreen::BitsRestoreScreen(Common::Rect rect, byte *&memoryPtr, byte *screen) {
	int width = rect.width();
	int height = rect.height();
	int y;

	screen += (rect.top * _width) + rect.left;

	for (y=rect.top; y<rect.bottom; y++) {
		memcpy((void*) screen, memoryPtr, width); memoryPtr += width;
		screen += _width;
	}
}

} // end of namespace Sci
