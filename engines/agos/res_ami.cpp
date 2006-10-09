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

// Resource file routines for Simon1/Simon2
#include "common/stdafx.h"

#include "agos/agos.h"
#include "agos/intern.h"

namespace AGOS {

byte *buffer;
byte *bufptr;
byte *bufferout;
byte *bufptrout;
uint32 bufoutend;
byte *clipptr;
byte *clipoutptr;
int clipnumber;

static void uncompressplane(byte *plane, byte *outptr, uint16 length) {
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

static void convertcompressedclip(uint16 height, uint16 width) {
	byte *plane0;
	byte *plane1;
	byte *plane2;
	byte *plane3;
	byte *uncbuffer;
	byte *uncptr0;
	byte *uncptr1;
	byte *uncptr2;
	byte *uncptr3;
	byte *uncbfrout;
	byte *uncbfroutptr;
	uint16 length, i, j, k, word1, word2, word3, word4, cliplength;
	byte outbyte, outbyte1, x, y;
	char n;
	uncbuffer = (byte *)malloc((long)(height * width / 2));
	uncbfrout = (byte *)malloc((long)(height * width / 2));
	length = width / 16;
	length *= height;
	plane0 = READ_BE_UINT32(clipptr) + clipptr; clipptr += 4; plane0 += 4;
	plane1 = READ_BE_UINT32(clipptr) + clipptr; clipptr += 4; plane1 += 4;
	plane2 = READ_BE_UINT32(clipptr) + clipptr; clipptr += 4; plane2 += 4;
	plane3 = READ_BE_UINT32(clipptr) + clipptr; clipptr += 4; plane3 += 4;
	plane0 -= 4;
	plane1 -= 8;
	plane2 -= 12;
	plane3 -= 16;
	uncptr0 = uncbuffer;
	uncptr1 = uncptr0+(length*2);
	uncptr2 = uncptr1+(length*2);
	uncptr3 = uncptr2+(length*2);
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
			*clipoutptr++ = 0xFF; bufoutend++;
			*clipoutptr++ = *uncbuffer; bufoutend++;
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
				*clipoutptr++ = n; bufoutend++;
				*clipoutptr++ = x; bufoutend++;
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
			*clipoutptr++ = n; bufoutend++;
			*clipoutptr++ = x; bufoutend++;
			cliplength += 2;
			uncbuffer--;
			if (length == 0)
				break;
			length++;
		} else {
			n =- 1;
			uncptr0 = clipoutptr;
			clipoutptr++;
			bufoutend++;
			*clipoutptr++ = x; bufoutend++;
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
				*clipoutptr++ = x; bufoutend++;
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
	if (cliplength > (height * width / 2))
		warning("Negative compression. Clip %d. %d bytes bigger.",clipnumber,(cliplength-(height*width/2)));
	free(uncbuffer);
	free(uncbfrout);
}

static void convertclip(uint32 offset, uint16 height, uint16 width) {
	uint32 length, i, j;
	uint16 word1, word2, word3, word4;
	byte outbyte, outbyte1;
	clipptr = offset + buffer;
	clipoutptr = bufoutend + bufferout;
	WRITE_BE_UINT32(bufptrout, bufoutend); bufptrout += 4;
	WRITE_BE_UINT16(bufptrout, height); bufptrout += 2;
	WRITE_BE_UINT16(bufptrout, width); bufptrout += 2;
	if (height > 32000) {
		convertcompressedclip((uint16)(height - 32768), width);
	} else {
		width /= 16;
		length = height * width;
		for (i = 0; i < length; i++) {
			word1 = READ_BE_UINT16(clipptr); clipptr += 2;
			word2 = READ_BE_UINT16(clipptr); clipptr += 2;
			word3 = READ_BE_UINT16(clipptr); clipptr += 2;
			word4 = READ_BE_UINT16(clipptr); clipptr += 2;
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
				*clipoutptr++ = outbyte * 16 + outbyte1; bufoutend++;
			}
		}
	}
}

void AGOSEngine::convertAmiga(byte *srcBuf, int32 fileSize) {
	uint32 clipoffset, outlength;
	uint16 clipwidth, clipheight;
	byte *clipsend;

	debug(0, "convertAmiga: fizeSize %d", fileSize);

	buffer = (byte *)malloc((long)fileSize);
	memcpy(buffer, srcBuf, fileSize);
	bufptr = buffer;

	bufferout = (byte *)malloc((long)(fileSize * 2));
	bufptr = buffer;
	bufptrout = bufferout;
	clipnumber = 0;
	while(1) {
		clipoffset = READ_BE_UINT32(bufptr); bufptr +=4;
		clipheight = READ_BE_UINT16(bufptr); bufptr +=2;
		clipwidth = READ_BE_UINT16(bufptr); bufptr +=2;
		if (clipoffset != 0)
			break;
		WRITE_BE_UINT32(bufptrout, 0); bufptrout += 4;
		WRITE_BE_UINT32(bufptrout, 0); bufptrout += 4;
		clipnumber++;
	}

	clipsend = (byte *)(clipoffset + (uint32)buffer);
	bufoutend = clipoffset;
	while (bufptr <= clipsend) {
		if (clipoffset != 0) {
			convertclip(clipoffset, clipheight, clipwidth);
		} else {
			WRITE_BE_UINT32(bufptrout, 0); bufptrout += 4;
			WRITE_BE_UINT32(bufptrout, 0); bufptrout += 4;
		}
		clipoffset = READ_BE_UINT32(bufptr); bufptr +=4;
		clipheight = READ_BE_UINT16(bufptr); bufptr +=2;
		clipwidth = READ_BE_UINT16(bufptr); bufptr +=2;
		clipnumber++;
	}
	outlength = bufoutend;
	debug(0, "convertAmiga: outlength %d",outlength);

	byte *dstBuf = allocBlock (outlength);
	memcpy(dstBuf, bufferout, outlength);
	free(buffer);
	free(bufferout);
}

} // End of namespace AGOS
