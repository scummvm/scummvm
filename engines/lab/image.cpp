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

#include "lab/lab.h"
#include "lab/image.h"

namespace Lab {

/*****************************************************************************/
/* Reads in an image from disk.                                              */
/*****************************************************************************/
Image::Image(Common::File *s) {
	_width = s->readUint16LE();
	_height = s->readUint16LE();
	s->skip(4);

	uint32 size = _width * _height;
	if (size & 1)
		size++;

	_imageData = new byte[size];	// FIXME: Memory leak!
	s->read(_imageData, size);
}

/*****************************************************************************/
/* Draws an image to the screen.                                             */
/*****************************************************************************/
void Image::drawImage(uint16 x, uint16 y) {
	int sx = 0, sy = 0;
	int dx = x, dy = y;
	int w = _width;
	int h = _height;

	if (dx < 0) {
		sx -= dx;
		w += dx;
		dx = 0;
	}

	if (dy < 0) {
		sy -= dy;
		w += dy;
		dy = 0;
	}

	if (dx + w > g_lab->_screenWidth)
		w = g_lab->_screenWidth - dx;

	if (dy + h > g_lab->_screenHeight)
		h = g_lab->_screenHeight - dy;

	if ((w > 0) && (h > 0)) {
		byte *s = _imageData + sy * _width + sx;
		byte *d = g_lab->getCurrentDrawingBuffer() + dy * g_lab->_screenWidth + dx;

		while (h-- > 0) {
			memcpy(d, s, w);
			s += _width;
			d += g_lab->_screenWidth;
		}
	}
}

/*****************************************************************************/
/* Draws an image to the screen.                                             */
/*****************************************************************************/
void Image::drawMaskImage(uint16 x, uint16 y) {
	int sx = 0, sy = 0;
	int dx = x, dy = y;
	int w = _width;
	int h = _height;

	if (dx < 0) {
		sx -= dx;
		w += dx;
		dx = 0;
	}

	if (dy < 0) {
		sy -= dy;
		w += dy;
		dy = 0;
	}

	if (dx + w > g_lab->_screenWidth)
		w = g_lab->_screenWidth - dx;

	if (dy + h > g_lab->_screenHeight)
		h = g_lab->_screenHeight - dy;

	if ((w > 0) && (h > 0)) {
		byte *s = _imageData + sy * _width + sx;
		byte *d = g_lab->getCurrentDrawingBuffer() + dy * g_lab->_screenWidth + dx;

		while (h-- > 0) {
			byte *ss = s;
			byte *dd = d;
			int ww = w;

			while (ww-- > 0) {
				byte c = *ss++;

				if (c) *dd++ = c - 1;
				else dd++;
			}

			s += _width;
			d += g_lab->_screenWidth;
		}
	}
}

/*****************************************************************************/
/* Reads an image from the screen.                                           */
/*****************************************************************************/
void Image::readScreenImage(uint16 x, uint16 y) {
	int sx = 0, sy = 0;
	int	dx = x, dy = y;
	int w = _width;
	int h = _height;

 	if (dx < 0) {
		sx -= dx;
		w += dx;
		dx = 0;
	}

	if (dy < 0) {
		sy -= dy;
		w += dy;
		dy = 0;
	}

	if (dx + w > g_lab->_screenWidth)
		w = g_lab->_screenWidth - dx;

	if (dy + h > g_lab->_screenHeight)
		h = g_lab->_screenHeight - dy;

	if ((w > 0) && (h > 0)) {
		byte *s = _imageData + sy * _width + sx;
		byte *d = g_lab->getCurrentDrawingBuffer() + dy * g_lab->_screenWidth + dx;

		while (h-- > 0) {
			memcpy(s, d, w);
			s += _width;
			d += g_lab->_screenWidth;
		}
	}
}

/*****************************************************************************/
/* Blits a piece of one image to another.                                    */
/* NOTE: for our purposes, assumes that ImDest is to be in VGA memory.       */
/*****************************************************************************/
void Image::bltBitMap(uint16 xs, uint16 ys, Image *imDest,
					uint16 xd, uint16 yd, uint16 width, uint16 height) {
	// I think the old code assumed that the source image data was valid for the given box.
	// I will proceed on that assumption.
	int sx = xs;
	int sy = ys;
	int dx = xd;
	int dy = yd;
	int w = width;
	int h = height;

	if (dx < 0) {
		sx -= dx;
		w += dx;
		dx = 0;
	}

	if (dy < 0) {
		sy -= dy;
		w += dy;
		dy = 0;
	}

	if (dx + w > imDest->_width)
		w = imDest->_width - dx;

	if (dy + h > imDest->_height)
		h = imDest->_height - dy;

	if (w > 0 && h > 0) {
		byte *s = _imageData + sy * _width + sx;
		byte *d = imDest->_imageData + dy * imDest->_width + dx;

		while (h-- > 0) {
			memcpy(d, s, w);
			s += _width;
			d += imDest->_width;
		}
	}
}

} // End of namespace Lab
