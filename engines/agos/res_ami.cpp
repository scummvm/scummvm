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

// Conversion routines for planar graphics in Amiga verisions
#include "common/stdafx.h"

#include "agos/agos.h"
#include "agos/intern.h"

namespace AGOS {

enum {
	kMaxColorDepth = 5
};

static void uncompressplane(const byte *plane, byte *outptr, uint16 length) {
	while (length != 0) {
		char x = *plane++;
		if (x >= 0) {
			x++;
			byte y = *plane++;
			byte z = *plane++;
			while (x != 0) {
				*outptr++ = y;
				*outptr++ = z;
				if (--length == 0)
					break;
				x--;
			}
		} else {
			while (x != 0) {
				*outptr++ = *plane++;
				*outptr++ = *plane++;
				if (--length == 0)
					break;
				x++;
			}
		}
	}
}

static void bitplanetochunky(uint16 *w, uint8 colorDepth, uint8 *&dst) {
	for (int j = 0; j < 8; j++) {
		byte color1 = 0;
		byte color2 = 0;
		for (int p = 0; p < 5; ++p) {
			if (w[p] & 0x8000) {
				color1 |= 1 << p;
			}
			if (w[p] & 0x4000) {
				color2 |= 1 << p;
			}
			w[p] <<= 2;
		}
		if (colorDepth > 4) {
			*dst++ = color1;
			*dst++ = color2;
		} else {
			*dst++ = (color1 << 4) | color2;
		}
	}
}

static void convertcompressedclip(const byte *src, byte *dst, uint8 colorDepth, uint16 height, uint16 width) {
	const byte *plane[kMaxColorDepth];
	byte *uncptr[kMaxColorDepth];
	uint32 length, i, j;
	uint16 w[kMaxColorDepth];

	byte *uncbfrout = (byte *)malloc(width * height);

	length = (width + 15) / 16 * height;

	for (i = 0; i < colorDepth; ++i) {
		plane[i] = src + READ_BE_UINT16(src + i * 4) + READ_BE_UINT16(src + i * 4 + 2);
		uncptr[i] = (uint8 *)malloc(length * 2);
		uncompressplane(plane[i], uncptr[i], length);
		plane[i] = uncptr[i];
	}

	byte *uncbfroutptr = uncbfrout;
	for (i = 0; i < length; ++i) {
		for (j = 0; j < colorDepth; ++j) {
			w[j] = READ_BE_UINT16(plane[j]); plane[j] += 2;
		}
		bitplanetochunky(w, colorDepth, uncbfroutptr);
	}

	uncbfroutptr = uncbfrout;
	const int chunkSize = colorDepth > 4 ? 16 : 8;
	for (i = 0; i < width / 16; ++i) {
		for (j = 0; j < height; ++j) {
			memcpy(dst + width * chunkSize / 16 * j + chunkSize * i, uncbfroutptr, chunkSize);
			uncbfroutptr += chunkSize;
		}
	}

	free(uncbfrout);
	for (i = 0; i < colorDepth; ++i) {
		free(uncptr[i]);
  	}
}

byte *AGOSEngine::convertclip(const byte *src, bool is32Colors, uint height, uint width, byte flags) {
	uint8 colorDepth = is32Colors ? 5 : 4;

	uint32 length, i, j;

	free(_planarBuf);
	_planarBuf = (byte *)malloc(width * height);
	byte *dst = _planarBuf;

	if (flags & 0x80) {
		convertcompressedclip(src, dst, colorDepth, height, width);
	} else {
		length = (width + 15) / 16 * height;
		for (i = 0; i < length; i++) {
			uint16 w[kMaxColorDepth];
			for (j = 0; j < colorDepth; ++j) {
				w[j] = READ_BE_UINT16(src); src += 2;
			}
			bitplanetochunky(w, colorDepth, dst);
		}
	}

	return _planarBuf;
}

} // End of namespace AGOS
