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

#include "common/endian.h"
#include "graphics/primitives.h"

#include "gob/driver_vga.h"

namespace Gob {

static void plotPixel(int x, int y, int color, void *data) {
	SurfaceDesc *dest = (SurfaceDesc *)data;

	if ((x >= 0) && (x < dest->getWidth()) &&
	    (y >= 0) && (y < dest->getHeight()))
		dest->getVidMem()[(y * dest->getWidth()) + x] = color;
}

void VGAVideoDriver::putPixel(int16 x, int16 y, byte color, SurfaceDesc *dest) {
	if ((x >= 0) && (x < dest->getWidth()) &&
	    (y >= 0) && (y < dest->getHeight()))
		dest->getVidMem()[(y * dest->getWidth()) + x] = color;
}

void VGAVideoDriver::drawLine(SurfaceDesc *dest, int16 x0, int16 y0, int16 x1,
		int16 y1, byte color) {

	Graphics::drawLine(x0, y0, x1, y1, color, &plotPixel, dest);
}

void VGAVideoDriver::fillRect(SurfaceDesc *dest, int16 left, int16 top,
		int16 right, int16 bottom, byte color) {

	if ((left >= dest->getWidth()) || (right >= dest->getWidth()) ||
	    (top >= dest->getHeight()) || (bottom >= dest->getHeight()))
		return;

	byte *pos = dest->getVidMem() + (top * dest->getWidth()) + left;
	int16 width = (right - left) + 1;
	int16 height = (bottom - top) + 1;

	while (height--) {
		for (int16 i = 0; i < width; ++i)
			pos[i] = color;

		pos += dest->getWidth();
	}
}

void VGAVideoDriver::drawLetter(unsigned char item, int16 x, int16 y,
		Video::FontDesc *fontDesc, byte color1, byte color2,
		byte transp, SurfaceDesc *dest) {
	byte *src, *dst;
	uint16 data;

	src = fontDesc->dataPtr +
		(item - fontDesc->startItem) * (fontDesc->itemSize & 0xFF);
	dst = dest->getVidMem() + x + dest->getWidth() * y;

	for (int i = 0; i < fontDesc->itemHeight; i++) {
		data = READ_BE_UINT16(src);
		src += 2;
		if (fontDesc->itemWidth <= 8)
			src--;

		for (int j = 0; j < fontDesc->itemWidth; j++) {
			if (data & 0x8000)
				*dst = color2;
			else if (color1 == 0)
				*dst = transp;

			dst++;
			data <<= 1;
		}
		dst += dest->getWidth() - fontDesc->itemWidth;
	}
}

void VGAVideoDriver::drawSprite(SurfaceDesc *source, SurfaceDesc *dest,
		int16 left, int16 top, int16 right, int16 bottom,
		int16 x, int16 y, int16 transp) {

	if ((x >= dest->getWidth()) || (x < 0) ||
	    (y >= dest->getHeight()) || (y < 0))
		return;

	int16 width = MIN((right - left) + 1, (int) dest->getWidth());
	int16 height = MIN((bottom - top) + 1, (int) dest->getHeight());

	if ((width < 1) || (height < 1))
		return;

	const byte *srcPos = source->getVidMem() + (top * source->getWidth()) + left;
	byte *destPos = dest->getVidMem() + (y * dest->getWidth()) + x;

	uint32 size = width * height;
	
	if (transp) {

		while (height--) {
			for (int16 i = 0; i < width; ++i) {
				if (srcPos[i])
					destPos[i] = srcPos[i];
			}

			srcPos += source->getWidth();
			destPos += dest->getWidth();
		}

	} else if (((srcPos  >= destPos) && (srcPos  <= (destPos + size))) ||
	           ((destPos >= srcPos)  && (destPos <= (srcPos  + size)))) {

		while (height--) {
			memmove(destPos, srcPos, width);

			srcPos += source->getWidth();
			destPos += dest->getWidth();
		}

	} else {

		while (height--) {
			memcpy(destPos, srcPos, width);

			srcPos += source->getWidth();
			destPos += dest->getWidth();
		}

	}
}

void VGAVideoDriver::drawPackedSprite(byte *sprBuf, int16 width, int16 height,
		int16 x, int16 y, byte transp, SurfaceDesc *dest) {
	int destRight = x + width;
	int destBottom = y + height;

	byte *dst = dest->getVidMem() + x + dest->getWidth() * y;

	int curx = x;
	int cury = y;

	while (1) {
		uint8 val = *sprBuf++;
		unsigned int repeat = val & 7;
		val &= 0xF8;

		if (!(val & 8)) {
			repeat <<= 8;
			repeat |= *sprBuf++;
		}
		repeat++;
		val >>= 4;

		for (unsigned int i = 0; i < repeat; ++i) {
			if (curx < dest->getWidth() && cury < dest->getHeight())
				if (!transp || val)
					*dst = val;

			dst++;
			curx++;
			if (curx == destRight) {
				dst += dest->getWidth() + x - curx;
				curx = x;
				cury++;
				if (cury == destBottom)
					return;
			}
		}

	}

}

}

