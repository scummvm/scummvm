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

#include "glk/scott/scott.h"
#include "glk/scott/globals.h"
#include "glk/scott/ring_buffer.h"
#include "glk/scott/saga_draw.h"
#include "glk/scott/line_drawing.h"

namespace Glk {
namespace Scott {

void scottLinegraphicsPlotClip(int x, int y, int colour) {
	/*
	 * Clip the plot if the value is outside the context.  Otherwise, plot the
	 * pixel as colour1 if it is currently colour2.
	 */
	if (x >= 0 && x <= _G(_scottGraphicsWidth) && y >= 0 && y < _G(_scottGraphicsHeight)) {
		_G(_pictureBitmap)[y * 255 + x] = colour;
		PixelToDraw *toDraw = new PixelToDraw;
		toDraw->_x = x;
		toDraw->_y = y;
		toDraw->_colour = colour;
		_G(_pixelsToDraw)[_G(_totalDrawInstructions)++] = toDraw;
	}
}

void scottLinegraphicsDrawLine(int x1, int y1, int x2, int y2, int colour) {
	int x, y, dx, dy, incx, incy, balance;

	/* Normalize the line into deltas and increments. */
	if (x2 >= x1) {
		dx = x2 - x1;
		incx = 1;
	} else {
		dx = x1 - x2;
		incx = -1;
	}

	if (y2 >= y1) {
		dy = y2 - y1;
		incy = 1;
	} else {
		dy = y1 - y2;
		incy = -1;
	}

	/* Start at x1,y1. */
	x = x1;
	y = y1;

	/* Decide on a direction to progress in. */
	if (dx >= dy) {
		dy <<= 1;
		balance = dy - dx;
		dx <<= 1;

		/* Loop until we reach the end point of the line. */
		while (x != x2) {
			scottLinegraphicsPlotClip(x, y, colour);
			if (balance >= 0) {
				y += incy;
				balance -= dx;
			}
			balance += dy;
			x += incx;
		}
		scottLinegraphicsPlotClip(x, y, colour);
	} else {
		dx <<= 1;
		balance = dx - dy;
		dy <<= 1;

		/* Loop until we reach the end point of the line. */
		while (y != y2) {
			scottLinegraphicsPlotClip(x, y, colour);
			if (balance >= 0) {
				x += incx;
				balance -= dy;
			}
			balance += dx;
			y += incy;
		}
		scottLinegraphicsPlotClip(x, y, colour);
	}
}

void freePixels() {
	for (int i = 0; i < _G(_totalDrawInstructions); i++)
		if (_G(_pixelsToDraw)[i] != nullptr)
			delete _G(_pixelsToDraw)[i];
	delete[] _G(_pixelsToDraw);
}

int linegraphicsGetPixel(int x, int y) {
	return _G(_pictureBitmap)[y * 255 + x];
}

void diamondFill(int x, int y, int colour) {
	uint8_t buffer[2048];
	cbuf_handle_t ringbuf = circularBufInit(buffer, 2048);
	circularBufPut(ringbuf, x, y);
	while (!circularBufEmpty(ringbuf)) {
		circularBufGet(ringbuf, &x, &y);
		if (x >= 0 && x < _G(_scottGraphicsWidth) && y >= 0 &&
			y < _G(_scottGraphicsHeight) &&
			linegraphicsGetPixel(x, y) == _G(_bgColour)) {
			scottLinegraphicsPlotClip(x, y, colour);
			circularBufPut(ringbuf, x, y + 1);
			circularBufPut(ringbuf, x, y - 1);
			circularBufPut(ringbuf, x + 1, y);
			circularBufPut(ringbuf, x - 1, y);
		}
	}
}

void drawVectorPicture(int image) {
	if (image < 0) {
		return;
	}

	if (_G(_vectorImageShown) == image) {
		if (_G(_vectorState) == SHOWING_VECTOR_IMAGE) {
			return;
		} else {
			if (_G(_gliSlowDraw))
				g_scott->glk_request_timer_events(20);
			drawSomeVectorPixels(1);
			return;
		}
	}

	g_scott->glk_request_timer_events(0);
	_G(_vectorImageShown) = image;
	if (_G(_pixelsToDraw) != nullptr)
		freePixels();
	_G(_pixelsToDraw) = new PixelToDraw *[255 * 97];
	_G(_totalDrawInstructions) = 0;
	_G(_currentDrawInstruction) = 0;

	if (_G(_palChosen) == NO_PALETTE) {
		_G(_palChosen) = _G(_game)->_palette;
		definePalette();
	}
	_G(_pictureBitmap) = new uint8_t[255 * 97];
	_G(_bgColour) = _G(_lineImages)[image]._bgColour;
	memset(_G(_pictureBitmap), _G(_bgColour), 255 * 97);
	if (_G(_bgColour) == 0)
		_G(_lineColour) = 7;
	else
		_G(_lineColour) = 0;
	int x = 0, y = 0, y2 = 0;
	int arg1, arg2, arg3;
	uint8_t *p = _G(_lineImages)[image]._data;
	uint8_t opcode = 0;
	while (((p < _G(_lineImages)[image]._data) || static_cast<size_t>(p - _G(_lineImages)[image]._data) < _G(_lineImages)[image]._size) && opcode != 0xff) {
		if (p > _G(_entireFile) + _G(_fileLength)) {
			error("drawVectorPicture: Out of range! Opcode: %x. Image: %d. LineImages[%d].size: %llu", opcode, image, image, _G(_lineImages)[image]._size);
			break;
		}
		opcode = *(p++);
		switch (opcode) {
		case 0xc0:
			y = 190 - *(p++);
			x = *(p++);
			break;
		case 0xc1:
			arg1 = *(p++);
			arg2 = *(p++);
			arg3 = *(p++);
			diamondFill(arg3, 190 - arg2, arg1);
			break;
		case 0xff:
			break;
		default:
			arg1 = *(p++);
			y2 = 190 - opcode;
			scottLinegraphicsDrawLine(x, y, arg1, y2, _G(_lineColour));
			x = arg1;
			y = y2;
			break;
		}
	}
	if (_G(_pictureBitmap) != nullptr) {
		delete[] _G(_pictureBitmap);
		_G(_pictureBitmap) = nullptr;
	}
	if (_G(_gliSlowDraw))
		g_scott->glk_request_timer_events(20);
	else
		drawSomeVectorPixels(1);
}

void drawSomeVectorPixels(int fromStart) {
	_G(_vectorState) = DRAWING_VECTOR_IMAGE;
	int i = _G(_currentDrawInstruction);
	if (fromStart)
		i = 0;
	if (i == 0)
		rectFill(0, 0, _G(_scottGraphicsWidth), _G(_scottGraphicsHeight), remap(_G(_bgColour)));
	for (; i < _G(_totalDrawInstructions) && (!_G(_gliSlowDraw) || i < _G(_currentDrawInstruction) + 50); i++) {
		PixelToDraw toDraw = *_G(_pixelsToDraw)[i];
		putPixel(toDraw._x, toDraw._y, remap(toDraw._colour));
	}
	_G(_currentDrawInstruction) = i;
	if (_G(_currentDrawInstruction) >= _G(_totalDrawInstructions)) {
		g_scott->glk_request_timer_events(0);
		_G(_vectorState) = SHOWING_VECTOR_IMAGE;
	}
}

int drawingVector() {
	return (_G(_totalDrawInstructions) > _G(_currentDrawInstruction));
}

} // End of namespace Scott
} // End of namespace Glk
