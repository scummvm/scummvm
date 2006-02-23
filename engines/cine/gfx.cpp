/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * cinE Engine is (C) 2004-2005 by CinE Team
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

#include "cine/cine.h"

byte *screenBuffer;

uint16 c_palette[256];

unsigned char *page0;
unsigned char *page1;
unsigned char *page2;
unsigned char *page3;

uint8 page1Raw[320 * 200];
uint8 page2Raw[320 * 200];
uint8 page3Raw[320 * 200];

void init_video() {
	screenBuffer = (byte *) malloc(320 * 200 * 3);
	assert(screenBuffer);

	page0 = (unsigned char *)malloc(0x8000);
	page1 = (unsigned char *)malloc(0x8000);
	page2 = (unsigned char *)malloc(0x8000);
	page3 = (unsigned char *)malloc(0x8000);
}

uint16 transformColor(uint16 baseColor, int8 r, int8 g, int8 b) {
	int8 oriR = (baseColor & 0x7);
	int8 oriG = (baseColor & 0x70) >> 4;
	int8 oriB = (baseColor & 0x700) >> 8;

	oriR += r;
	oriG += g;
	oriB += b;

	if (oriR < 0)
		oriR = 0;

	if (oriR > 7)
		oriR = 7;

	if (oriG < 0)
		oriG = 0;

	if (oriG > 7)
		oriG = 7;

	if (oriB < 0)
		oriB = 0;

	if (oriB > 7)
		oriB = 7;

	return (oriR | (oriG << 4) | (oriB << 8));
}

void transformPaletteRange(uint8 startColor, uint8 stopColor, int8 r, int8 g, int8 b) {
	uint8 i;

	for (i = startColor; i <= stopColor; i++) {
		c_palette[i] = transformColor(tempPalette[i], b, g, r);
	}
	//gfxFlipPage(page2);
}

void gfxFillSprite(uint8 *spritePtr, uint16 width, uint16 height, uint8 *page, int16 x, int16 y) {
	int16 i;
	int16 j;

	for (i = 0; i < height; i++) {
		uint8 *destPtr = page + x + y * 320;
		destPtr += i * 320;

		for (j = 0; j < width * 8; j++) {
			if (x + j >= 0 && x + j < 320 && i + y >= 0
			    && i + y < 200) {
				if (!*(spritePtr++)) {
					*(destPtr++) = 0;
				} else {
					destPtr++;
				}
			} else {
				destPtr++;
				spritePtr++;
			}
		}
	}
}

void gfxDrawLine(int16 x1, int16 y1, int16 x2, int16 y2, uint8 color, uint8 *page) {
	int16 t;
	if (x1 == x2) {
		if (y1 > y2) {
			t = y1;
			y1 = y2;
			y2 = t;
		}
		while (y1 <= y2) {
			*(page + (y1 * 320 + x1)) = color;
			y1++;
		}
	} else {
		if (x1 > x2) {
			t = x1;
			x1 = x2;
			x2 = t;
		}
		while (x1 <= x2) {
			*(page + (y1 * 320 + x1)) = color;
			x1++;
		}
	}

}

void gfxDrawPlainBoxRaw(int16 x1, int16 y1, int16 x2, int16 y2, uint8 color, uint8 *page) {
	int16 t;

	if (x1 > x2) {
		t = x1;
		x1 = x2;
		x2 = t;
	}

	if (y1 > y2) {
		t = y1;
		y1 = y2;
		y2 = t;
	}

	t = x1;
	while (y1 <= y2) {
		x1 = t;
		while (x1 <= x2) {
			*(page + y1 * 320 + x1) = color;
			x1++;
		}
		y1++;
	}
}

int16 gfxGetBit(int16 x, int16 y, uint8 *ptr, int16 width) {
	uint8 *ptrToData = (ptr) + y * width + x;

	if (x > width) {
		return 0;
	}

	if (*ptrToData) {
		return 0;
	}

	return (1);
}

void gfxResetRawPage(uint8 *pageRaw) {
	memset(pageRaw, 0, 320 * 200);
}

void gfxConvertSpriteToRaw(uint8 *dest, uint8 *source, uint16 width, uint16 height) {
	int x, y;
	uint8 b1, b2, b3, b4, b5, b6, b7, b8, d1a, d1b, d2a, d2b, d3a, d3b, d4a,
	    d4b;

	for (y = 0; y < height; y++) {
		for (x = 0; x < (width >> 3); x++) {
			b4 = *(source++);
			b8 = *(source++);
			b3 = *(source++);
			b7 = *(source++);
			b2 = *(source++);
			b6 = *(source++);
			b1 = *(source++);
			b5 = *(source++);

			d1a = d1b = d2a = d2b = d3a = d3b = d4a = d4b = 0;

			d1a |=
			    (((b4 & 1) >> 0) | ((b3 & 1) << 1) | ((b2 & 1) <<
				2) | ((b1 & 1) << 3)) << 0;
			d1b |=
			    (((b4 & 2) >> 1) | ((b3 & 2) >> 0) | ((b2 & 2) <<
				1) | ((b1 & 2) << 2)) << 0;
			d2a |=
			    (((b4 & 4) >> 2) | ((b3 & 4) >> 1) | ((b2 & 4) >>
				0) | ((b1 & 4) << 1)) << 0;
			d2b |=
			    (((b4 & 8) >> 3) | ((b3 & 8) >> 2) | ((b2 & 8) >>
				1) | ((b1 & 8) >> 0)) << 0;

			b1 >>= 4;
			b2 >>= 4;
			b3 >>= 4;
			b4 >>= 4;

			d3a |=
			    (((b4 & 1) >> 0) | ((b3 & 1) << 1) | ((b2 & 1) <<
				2) | ((b1 & 1) << 3)) << 0;
			d3b |=
			    (((b4 & 2) >> 1) | ((b3 & 2) >> 0) | ((b2 & 2) <<
				1) | ((b1 & 2) << 2)) << 0;
			d4a |=
			    (((b4 & 4) >> 2) | ((b3 & 4) >> 1) | ((b2 & 4) >>
				0) | ((b1 & 4) << 1)) << 0;
			d4b |=
			    (((b4 & 8) >> 3) | ((b3 & 8) >> 2) | ((b2 & 8) >>
				1) | ((b1 & 8) >> 0)) << 0;

			*(dest++) = d4b;
			*(dest++) = d4a;
			*(dest++) = d3b;
			*(dest++) = d3a;
			*(dest++) = d2b;
			*(dest++) = d2a;
			*(dest++) = d1b;
			*(dest++) = d1a;

			b1 = b5;
			b2 = b6;
			b3 = b7;
			b4 = b8;

			d1a = d1b = d2a = d2b = d3a = d3b = d4a = d4b = 0;

			d1a |=
			    (((b4 & 1) >> 0) | ((b3 & 1) << 1) | ((b2 & 1) <<
				2) | ((b1 & 1) << 3)) << 0;
			d1b |=
			    (((b4 & 2) >> 1) | ((b3 & 2) >> 0) | ((b2 & 2) <<
				1) | ((b1 & 2) << 2)) << 0;
			d2a |=
			    (((b4 & 4) >> 2) | ((b3 & 4) >> 1) | ((b2 & 4) >>
				0) | ((b1 & 4) << 1)) << 0;
			d2b |=
			    (((b4 & 8) >> 3) | ((b3 & 8) >> 2) | ((b2 & 8) >>
				1) | ((b1 & 8) >> 0)) << 0;

			b1 >>= 4;
			b2 >>= 4;
			b3 >>= 4;
			b4 >>= 4;

			d3a |=
			    (((b4 & 1) >> 0) | ((b3 & 1) << 1) | ((b2 & 1) <<
				2) | ((b1 & 1) << 3)) << 0;
			d3b |=
			    (((b4 & 2) >> 1) | ((b3 & 2) >> 0) | ((b2 & 2) <<
				1) | ((b1 & 2) << 2)) << 0;
			d4a |=
			    (((b4 & 4) >> 2) | ((b3 & 4) >> 1) | ((b2 & 4) >>
				0) | ((b1 & 4) << 1)) << 0;
			d4b |=
			    (((b4 & 8) >> 3) | ((b3 & 8) >> 2) | ((b2 & 8) >>
				1) | ((b1 & 8) >> 0)) << 0;

			*(dest++) = d4b;
			*(dest++) = d4a;
			*(dest++) = d3b;
			*(dest++) = d3a;
			*(dest++) = d2b;
			*(dest++) = d2a;
			*(dest++) = d1b;
			*(dest++) = d1a;

		}
	}
}

void gfxCopyRawPage(uint8 *source, uint8 *dest) {
	memcpy(dest, source, 320 * 200);
}

void gfxFlipRawPage(uint8 *frontBuffer) {
	uint8 *page = frontBuffer;
	int x, y;
	uint8 *pixels = (uint8 *) screenBuffer;
	byte c;

	for (y = 0; y < 200; y++) {
		for (x = 0; x < 320; x++) {
			c = *(page++);

			if (!colorMode256) {
				c = c & 15;
			}

			pixels[x + 0 + y * 320] = c;
		}
	}

	byte pal[256 * 4];
	int i;

	if (colorMode256) {
		for (i = 0; i < 256; i++) {
			pal[i * 4 + 0] = palette256[i * 3 + 0];
			pal[i * 4 + 1] = palette256[i * 3 + 1];
			pal[i * 4 + 2] = palette256[i * 3 + 2];
			pal[i * 4 + 3] = 0;
		}
		g_system->setPalette(pal, 0, 256);
	} else {
		for (i = 0; i < 16; i++) {
			pal[i * 4 + 2] =
			    ((c_palette[i] & 0x00f) >> 0) * 255 / 7;
			pal[i * 4 + 1] =
			    ((c_palette[i] & 0x0f0) >> 4) * 255 / 7;
			pal[i * 4 + 0] =
			    ((c_palette[i] & 0xf00) >> 8) * 255 / 7;
			pal[i * 4 + 3] = 0;
		}
		g_system->setPalette(pal, 0, 16);
	}

	g_system->copyRectToScreen(screenBuffer, 320, 0, 0, 320, 200);
	g_system->updateScreen();
	g_system->delayMillis(100);
}

void drawSpriteRaw(uint8 *spritePtr, uint8 *maskPtr, int16 width, int16 height,
				   uint8 *page, int16 x, int16 y) {
	int16 i;
	int16 j;

	for (i = 0; i < height; i++) {
		uint8 *destPtr = page + x + y * 320;
		destPtr += i * 320;

		for (j = 0; j < width * 8; j++) {
			if (((gameType == Cine::GID_FW && !(*maskPtr))
				|| (gameType == Cine::GID_OS)) && (x + j >= 0
				&& x + j < 320 && i + y >= 0 && i + y < 200)) {
				*(destPtr++) = *(spritePtr++);
			} else {
				destPtr++;
				spritePtr++;
			}

			maskPtr++;
		}
	}
}

void drawSpriteRaw2(uint8 *spritePtr, uint8 transColor, int16 width, int16 height,
					uint8 *page, int16 x, int16 y) {
	int16 i;
	int16 j;

	for (i = 0; i < height; i++) {
		uint8 *destPtr = page + x + y * 320;
		destPtr += i * 320;

		for (j = 0; j < width * 8; j++) {
			if ((*(spritePtr) != transColor) && (x + j >= 0
				&& x + j < 320 && i + y >= 0 && i + y < 200)) {
				*(destPtr++) = *(spritePtr++);
			} else {
				destPtr++;
				spritePtr++;
			}
		}
	}
}
