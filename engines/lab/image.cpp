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

Image::Image(Common::File *s, LabEngine *vm) : _vm(vm) {
	_width = s->readUint16LE();
	_height = s->readUint16LE();
	s->skip(4);

	uint32 size = _width * _height;
	if (size & 1)
		size++;

	_imageData = new byte[size];
	s->read(_imageData, size);
	_autoFree = true;
}

Image::~Image() {
	if (_autoFree)
		delete[] _imageData;
}

void Image::setData(byte *d, bool autoFree) {
	if (_autoFree)
		delete[] _imageData;
	_imageData = d;
	_autoFree = autoFree;
}

void Image::blitBitmap(uint16 srcX, uint16 srcY, Image *imgDest,
	uint16 destX, uint16 destY, uint16 width, uint16 height, byte masked) {
	int clipWidth = width;
	int clipHeight = height;
	int destWidth = (imgDest) ? imgDest->_width : _vm->_graphics->_screenWidth;
	int destHeight = (imgDest) ? imgDest->_height : _vm->_graphics->_screenHeight;
	byte *destBuffer = (imgDest) ? imgDest->_imageData : _vm->_graphics->getCurrentDrawingBuffer();

	if (destX + clipWidth > destWidth)
		clipWidth = destWidth - destX;

	if (destY + clipHeight > destHeight)
		clipHeight = destHeight - destY;

	if ((clipWidth > 0) && (clipHeight > 0)) {
		byte *img = _imageData + srcY * _width + srcX;
		byte *dest = destBuffer + destY * destWidth + destX;

		if (!masked) {
			for (int i = 0; i < clipHeight; i++) {
				memcpy(dest, img, clipWidth);
				img += _width;
				dest += destWidth;
			}
		} else {
			for (int i = 0; i < clipHeight; i++) {
				for (int j = 0; j < clipWidth; j++) {
					byte c = img[j];

					if (c)
						dest[j] = c - 1;
				}

				img += _width;
				dest += destWidth;
			}
		}
	}
}

void Image::drawImage(uint16 x, uint16 y) {
	blitBitmap(0, 0, nullptr, x, y, _width, _height, false);
}

void Image::drawMaskImage(uint16 x, uint16 y) {
	blitBitmap(0, 0, nullptr, x, y, _width, _height, true);
}

void Image::readScreenImage(uint16 x, uint16 y) {
	int clipWidth = _width;
	int clipHeight = _height;

	if (x + clipWidth > _vm->_graphics->_screenWidth)
		clipWidth = _vm->_graphics->_screenWidth - x;

	if (y + clipHeight > _vm->_graphics->_screenHeight)
		clipHeight = _vm->_graphics->_screenHeight - y;

	if ((clipWidth > 0) && (clipHeight > 0)) {
		byte *img = _imageData;
		byte *screen = _vm->_graphics->getCurrentDrawingBuffer() + y * _vm->_graphics->_screenWidth + x;

		while (clipHeight-- > 0) {
			memcpy(img, screen, clipWidth);
			img += _width;
			screen += _vm->_graphics->_screenWidth;
		}
	}
}

} // End of namespace Lab
