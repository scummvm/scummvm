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

static void uncompressplane(const byte *plane, byte *outptr, uint16 length) {
	char x;
	byte y, z;
	while (length) {
		x = *plane++;
		if (x >= 0) {
			x += 1;
			y = *plane++;
			z = *plane++;
			while (x) {
				*outptr++ = y;
				*outptr++ = z;
				length--;
				if (length == 0)
					break;
				x--;
			}
		} else {
			while (x) {
				*outptr++ = *plane++;
				*outptr++ = *plane++;
				length--;
				if (length == 0)
					break;
				x++;
			}
		}
	}
}

static void convertcompressedclip(const byte *src, byte *dst, uint16 height, uint16 width) {
	const byte *plane0, *plane1, *plane2, *plane3;
	byte *uncbuffer;
	byte *uncptr0, *uncptr1, *uncptr2, *uncptr3;
	byte *uncbfrout;
	byte *uncbfroutptr;
	uint16 length, i, j, k, word1, word2, word3, word4, cliplength;
	byte outbyte, outbyte1, x, y;
	char n;
	uncbuffer = (byte *)malloc(height * width * 4);
	uncbfrout = (byte *)malloc(height * width * 4);
	
	byte *free_uncbuffer = uncbuffer;
	byte *free_uncbfrout = uncbfrout;
	
	length = width / 16;
	length *= height;
	plane0 = READ_BE_UINT16(src) + READ_BE_UINT16(src + 2) + src; src += 4; plane0 += 4;
	plane1 = READ_BE_UINT16(src) + READ_BE_UINT16(src + 2) + src; src += 4; plane1 += 4;
	plane2 = READ_BE_UINT16(src) + READ_BE_UINT16(src + 2) + src; src += 4; plane2 += 4;
	plane3 = READ_BE_UINT16(src) + READ_BE_UINT16(src + 2) + src; src += 4; plane3 += 4;
	plane0 -= 4;
	plane1 -= 8;
	plane2 -= 12;
	plane3 -= 16;
	uncptr0 = uncbuffer;
	uncptr1 = uncptr0+(length * 2);
	uncptr2 = uncptr1+(length * 2);
	uncptr3 = uncptr2+(length * 2);
	uncompressplane(plane0, uncptr0, length);
	uncompressplane(plane1, uncptr1, length);
	uncompressplane(plane2, uncptr2, length);
	uncompressplane(plane3, uncptr3, length);
	uncbfroutptr = uncbfrout;
	for (i = 0; i < length; i++) {
		word1=READ_BE_UINT16(uncptr0); uncptr0 += 2;
		word2=READ_BE_UINT16(uncptr1); uncptr1 += 2;
		word3=READ_BE_UINT16(uncptr2); uncptr2 += 2;
		word4=READ_BE_UINT16(uncptr3); uncptr3 += 2;
		for (j = 0; j < 8; j++) {
			outbyte = ((word1 / 32768) + ((word2 / 32768) * 2) + ((word3 / 32768) * 4) + ((word4 / 32768) * 8));
			word1 <<= 1;
			word2 <<= 1;
			word3 <<= 1;
			word4 <<= 1;
			outbyte1 = ((word1 / 32768) + ((word2 / 32768) * 2) + ((word3 / 32768) * 4) + ((word4 / 32768) * 8));
			word1 <<= 1;
			word2 <<= 1;
			word3 <<= 1;
			word4 <<= 1;
			*uncbfroutptr++ = (outbyte * 16 + outbyte1);
		}
	}
	uncptr0 = uncbuffer;
	uncptr1 = uncbfrout;
	uncptr2 = uncbfrout;
	uncptr3 = uncbfrout;
	for (i = 0; i < (width / 16); i++) {
		for (k = 0; k < 8; k++) {
			for (j = 0; j < height; j++) {
				*uncptr0++ = *uncptr1;
				uncptr1 += 8;
			}
			uncptr2++;
			uncptr1 = uncptr2;
		}
		uncptr3 += (height * 8);
		uncptr2 = uncptr3;
		uncptr1 = uncptr2;
	}
	length *= 8;
	cliplength = 0;
	while(1) {
		if (length == 1) {
			*dst++ = 0xFF;
			*dst++ = *uncbuffer;
			cliplength += 2;
			break;
		}
		x = *uncbuffer++;
		y = *uncbuffer++;
		length -= 2;
		if (x == y) {
			n = 1;
			y = *uncbuffer++;
			if (length == 0) {
				*dst++ = n;
				*dst++ = x;
				cliplength += 2;
				break;
			}
			length--;
			while (x == y) 	{
				n++;
				y = *uncbuffer++;
				if (length == 0)
					break;
				length--;
				if(n == 127)
					break;
			}
			*dst++ = n;
			*dst++ = x;
			cliplength += 2;
			uncbuffer--;
			if (length == 0)
				break;
			length++;
		} else {
			n =- 1;
			uncptr0 = dst;
			dst++;
			*dst++ = x;
			cliplength += 2;
			x = y;
			y = *uncbuffer++;
			if (length == 0) {
				*uncptr0 = n;
				break;
			}
			length--;
			while (x != y) {
				if (n == -127)
					break;
				n--;
				*dst++ = x;
				cliplength++;
				x = y;
				y = *uncbuffer++;
				if (length == 0)
					break;
				length--;
			}
			*uncptr0 = n;
			if (length == 0)
				break;
			uncbuffer -= 2;
			length += 2;
		}
	}

	free(free_uncbuffer);
	free(free_uncbfrout);
}

byte *AGOSEngine::convertclip(const byte *src, uint height, uint width, byte flags) {
	uint32 length, i, j;
	uint16 word1, word2, word3, word4;
	byte outbyte, outbyte1;

	free(_planarBuf);
	_planarBuf = (byte *)malloc(width * height);
	byte *dst = _planarBuf;

	if (flags & 0x80) {
		convertcompressedclip(src, dst, height, width);
	} else {
		width /= 16;
		length = height * width;
		for (i = 0; i < length; i++) {
			word1 = READ_BE_UINT16(src); src += 2;
			word2 = READ_BE_UINT16(src); src += 2;
			word3 = READ_BE_UINT16(src); src += 2;
			word4 = READ_BE_UINT16(src); src += 2;
			for (j = 0; j < 8; j++) {
				outbyte = ((word1 / 32768) + ((word2 / 32768) * 2) + ((word3 / 32768) * 4) + ((word4 / 32768) * 8));
				word1 <<= 1;
				word2 <<= 1;
				word3 <<= 1;
				word4 <<= 1;
				outbyte1 = ((word1 / 32768) + ((word2 / 32768) * 2) + ((word3 / 32768) * 4) + ((word4 / 32768) * 8));
				word1 <<= 1;
				word2 <<= 1;
				word3 <<= 1;
				word4 <<= 1;
				*dst++ = (outbyte * 16 + outbyte1);
			}
		}
	}

	return _planarBuf;
}

} // End of namespace AGOS
