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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#include "common/file.h"

#include "lab/lab.h"

#include "lab/dispman.h"
#include "lab/image.h"

namespace Lab {

/**
 * Reads in an image from disk.
 */
Image::Image(Common::File *s, LabEngine *vm) : _vm(vm) {
	_width = s->readUint16LE();
	_height = s->readUint16LE();
	s->skip(4);

	uint32 size = _width * _height;
	if (size & 1)
		size++;

	_imageData = new byte[size];	// FIXME: Memory leak!
	s->read(_imageData, size);
}

/**
 * Blits a piece of one image to another.
 */
void Image::blitBitmap(uint16 xs, uint16 ys, Image *imDest,
	uint16 xd, uint16 yd, uint16 width, uint16 height, byte masked) {
	int w = width;
	int h = height;
	int destWidth = (imDest) ? imDest->_width : _vm->_graphics->_screenWidth;
	int destHeight = (imDest) ? imDest->_height : _vm->_graphics->_screenHeight;
	byte *destBuffer = (imDest) ? imDest->_imageData : _vm->_graphics->getCurrentDrawingBuffer();

	if (xd + w > destWidth)
		w = destWidth - xd;

	if (yd + h > destHeight)
		h = destHeight - yd;

	if (w > 0 && h > 0) {
		byte *s = _imageData + ys * _width + xs;
		byte *d = destBuffer + yd * destWidth + xd;

		if (!masked) {
			while (h-- > 0) {
				memcpy(d, s, w);
				s += _width;
				d += destWidth;
			}
		} else {
			while (h-- > 0) {
				byte *ss = s;
				byte *dd = d;
				int ww = w;

				while (ww-- > 0) {
					byte c = *ss++;

					if (c)
						*dd++ = c - 1;
					else
						dd++;
				}

				s += _width;
				d += destWidth;
			}
		}
	}
}

/**
 * Draws an image to the screen.
 */
void Image::drawImage(uint16 x, uint16 y) {
	blitBitmap(0, 0, NULL, x, y, _width, _height, false);
}

/**
 * Draws an image to the screen with transparency.
 */
void Image::drawMaskImage(uint16 x, uint16 y) {
	blitBitmap(0, 0, NULL, x, y, _width, _height, true);
}

/**
 * Reads an image from the screen.
 */
void Image::readScreenImage(uint16 x, uint16 y) {
	int w = _width;
	int h = _height;

	if (x + w > _vm->_graphics->_screenWidth)
		w = _vm->_graphics->_screenWidth - x;

	if (y + h > _vm->_graphics->_screenHeight)
		h = _vm->_graphics->_screenHeight - y;

	if ((w > 0) && (h > 0)) {
		byte *s = _imageData;
		byte *d = _vm->_graphics->getCurrentDrawingBuffer() + y * _vm->_graphics->_screenWidth + x;

		while (h-- > 0) {
			memcpy(s, d, w);
			s += _width;
			d += _vm->_graphics->_screenWidth;
		}
	}
}

} // End of namespace Lab
