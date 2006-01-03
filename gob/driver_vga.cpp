/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2005 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */
#include "gob/driver_vga.h"
#include "graphics/primitives.h"

#if defined  (_MSC_VER) || defined (__WINS__)
#define STUB_FUNC	printf("STUB:")
#else
#define STUB_FUNC	printf("STUB: %s\n", __PRETTY_FUNCTION__)
#endif

namespace Gob {

void VGAVideoDriver::drawSprite(Video::SurfaceDesc *source, Video::SurfaceDesc *dest, int16 left, int16 top, int16 right, int16 bottom, int16 x, int16 y, int16 transp) {
	if (x >= 0 && x < dest->width && y >= 0 && y < dest->height) {
		int16 width = (right - left) + 1;
		int16 height = (bottom - top) + 1;

		byte *srcPos = source->vidPtr + (top * source->width) + left;
		byte *destPos = dest->vidPtr + (y * dest->width) + x;
		while (height--) {
			if (transp) {
				for (int16 i = 0; i < width; ++i) {
					if (srcPos[i])
						destPos[i] = srcPos[i];
				}
			} else {
				for (int16 i = 0; i < width; ++i)
					destPos[i] = srcPos[i];
			}

			srcPos += source->width; //width ?
			destPos += dest->width;
		}
	}
}

void VGAVideoDriver::fillRect(Video::SurfaceDesc *dest, int16 left, int16 top, int16 right, int16 bottom, byte color) {
	if (left < dest->width && right < dest->width && top < dest->height && bottom < dest->height) {
		byte *pos = dest->vidPtr + (top * dest->width) + left;
		int16 width = (right - left) + 1;
		int16 height = (bottom - top) + 1;
		while (height--) {
			for (int16 i = 0; i < width; ++i) {
				pos[i] = color;
			}

			pos += dest->width;
		}
	}
}

void VGAVideoDriver::putPixel(int16 x, int16 y, byte color, Video::SurfaceDesc *dest) {
	if (x >= 0 && x < dest->width && y >= 0 && y < dest->height)
		dest->vidPtr[(y * dest->width) + x] = color;
}

void VGAVideoDriver::drawLetter(unsigned char item, int16 x, int16 y, Video::FontDesc *fontDesc, byte color1, byte color2, byte transp, Video::SurfaceDesc *dest) {
	byte *src, *dst;
	uint16 data;
	int i, j;

	src = (byte *)fontDesc->dataPtr + (item - fontDesc->startItem) * (fontDesc->itemSize & 0xff);
	dst = dest->vidPtr + x + dest->width * y;

	for (i = 0; i < fontDesc->itemHeight; i++) {
		data = READ_BE_UINT16(src);
		src += 2;
		if (fontDesc->itemSize <= 8)
			src--;

		for (j = 0; j < fontDesc->itemWidth; j++) {
			if (data & 0x8000) {
				*dst = color2;
			} else {
				if (color1 == 0)
					*dst = transp;
			}
			dst++;
			data <<= 1;
		}
		dst += dest->width - fontDesc->itemWidth;
	}
}

static void plotPixel(int x, int y, int color, void *data) {
	Video::SurfaceDesc *dest = (Video::SurfaceDesc *)data;
	if (x >= 0 && x < dest->width && y >= 0 && y < dest->height)
		dest->vidPtr[(y * dest->width) + x] = color;
}

void VGAVideoDriver::drawLine(Video::SurfaceDesc *dest, int16 x0, int16 y0, int16 x1, int16 y1, byte color) {
	Graphics::drawLine(x0, y0, x1, y1, color, &plotPixel, dest);
}

void VGAVideoDriver::drawPackedSprite(byte *sprBuf, int16 width, int16 height, int16 x, int16 y, byte transp, Video::SurfaceDesc *dest) {
	int destRight = x + width;
	int destBottom = y + height;

	byte* dst = dest->vidPtr + x + dest->width * y;

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
			if (curx < dest->width && cury < dest->height)
				if (!transp || val)
					*dst = val;

			dst++;
			curx++;
			if (curx == destRight) {
				dst += dest->width + x - curx;
				curx = x;
				cury++;
				if (cury == destBottom)
					return;
			}
		}
	}

}

}

