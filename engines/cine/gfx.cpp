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
#include "cine/various.h"

#include "common/system.h"

namespace Cine {

byte *screenBuffer;

uint16 c_palette[256];

byte *page0;
byte *page1;
byte *page2;
byte *page3;

byte page1Raw[320 * 200];
byte page2Raw[320 * 200];
byte page3Raw[320 * 200];

static const byte mouseCursorNormal[] = {
	0x00, 0x00, 0x40, 0x00, 0x60, 0x00, 0x70, 0x00,
	0x78, 0x00, 0x7C, 0x00, 0x7E, 0x00, 0x7F, 0x00,
	0x7F, 0x80, 0x7C, 0x00, 0x6C, 0x00, 0x46, 0x00,
	0x06, 0x00, 0x03, 0x00, 0x03, 0x00, 0x00, 0x00,
	0xC0, 0x00, 0xE0, 0x00, 0xF0, 0x00, 0xF8, 0x00,
	0xFC, 0x00, 0xFE, 0x00, 0xFF, 0x00, 0xFF, 0x80,
	0xFF, 0xC0, 0xFF, 0xC0, 0xFE, 0x00, 0xFF, 0x00,
	0xCF, 0x00, 0x07, 0x80, 0x07, 0x80, 0x03, 0x80
};

static const byte mouseCursorDisk[] = {
	0x7F, 0xFC, 0x9F, 0x12, 0x9F, 0x12, 0x9F, 0x12,
	0x9F, 0x12, 0x9F, 0xE2, 0x80, 0x02, 0x9F, 0xF2,
	0xA0, 0x0A, 0xA0, 0x0A, 0xA0, 0x0A, 0xA0, 0x0A,
	0xA0, 0x0A, 0xA0, 0x0A, 0x7F, 0xFC, 0x00, 0x00,
	0x7F, 0xFC, 0xFF, 0xFE, 0xFF, 0xFE, 0xFF, 0xFE,
	0xFF, 0xFE, 0xFF, 0xFE, 0xFF, 0xFE, 0xFF, 0xFE,
	0xFF, 0xFE, 0xFF, 0xFE, 0xFF, 0xFE, 0xFF, 0xFE,
	0xFF, 0xFE, 0xFF, 0xFE, 0x7F, 0xFC, 0x00, 0x00
};

static const byte mouseCursorCross[] = {
	0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00,
	0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x7C, 0x7C,
	0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00,
	0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x03, 0x80, 0x03, 0x80, 0x03, 0x80, 0x03, 0x80,
	0x03, 0x80, 0x03, 0x80, 0xFF, 0xFE, 0xFE, 0xFE,
	0xFF, 0xFE, 0x03, 0x80, 0x03, 0x80, 0x03, 0x80,
	0x03, 0x80, 0x03, 0x80, 0x03, 0x80, 0x00, 0x00
};

static const struct MouseCursor {
	int hotspotX;
	int hotspotY;
	const byte *bitmap;
} mouseCursors[] = {
	{ 1, 1, mouseCursorNormal },
	{ 0, 0, mouseCursorDisk },
	{ 7, 7, mouseCursorCross }
};

void init_video() {
	screenBuffer = (byte *)malloc(320 * 200 * 3);
	assert(screenBuffer);

	page0 = (byte *)malloc(0x8000);
	page1 = (byte *)malloc(0x8000);
	page2 = (byte *)malloc(0x8000);
	page3 = (byte *)malloc(0x8000);
}

void setMouseCursor(int cursor) {
	static int currentMouseCursor = -1;
	assert(cursor >= 0 && cursor < 3);
	if (currentMouseCursor != cursor) {
		byte mouseCursor[16 * 16];
		const MouseCursor *mc = &mouseCursors[cursor];
		const byte *src = mc->bitmap;
		for (int i = 0; i < 32; ++i) {
			int offs = i * 8;
			for (byte mask = 0x80; mask != 0; mask >>= 1) {
				if (src[0] & mask) {
					mouseCursor[offs] = 2;
				} else if (src[32] & mask) {
					mouseCursor[offs] = 0;
				} else {
					mouseCursor[offs] = 0xFF;
				}
				++offs;
			}
			++src;
		}
		g_system->setMouseCursor(mouseCursor, 16, 16, mc->hotspotX, mc->hotspotY);
		currentMouseCursor = cursor;
	}
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
	else if (oriR > 7)
		oriR = 7;

	if (oriG < 0)
		oriG = 0;
	else if (oriG > 7)
		oriG = 7;

	if (oriB < 0)
		oriB = 0;
	else if (oriB > 7)
		oriB = 7;

	return (oriR | (oriG << 4) | (oriB << 8));
}

void transformPaletteRange(byte startColor, byte stopColor, int8 r, int8 g, int8 b) {
	byte i;

	for (i = startColor; i <= stopColor; i++) {
		c_palette[i] = transformColor(tempPalette[i], b, g, r);
	}
	//gfxFlipPage(page2);
}

void gfxFillSprite(byte *spritePtr, uint16 width, uint16 height, byte *page, int16 x, int16 y) {
	int16 i;
	int16 j;

	for (i = 0; i < height; i++) {
		byte *destPtr = page + x + y * 320;
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

void gfxDrawLine(int16 x1, int16 y1, int16 x2, int16 y2, byte color, byte *page) {
	if (x1 == x2) {
		if (y1 > y2) {
			SWAP(y1, y2);
		}
		while (y1 <= y2) {
			*(page + (y1 * 320 + x1)) = color;
			y1++;
		}
	} else {
		if (x1 > x2) {
			SWAP(x1, x2);
		}
		while (x1 <= x2) {
			*(page + (y1 * 320 + x1)) = color;
			x1++;
		}
	}

}

void gfxDrawPlainBoxRaw(int16 x1, int16 y1, int16 x2, int16 y2, byte color, byte *page) {
	int16 t;

	if (x1 > x2) {
		SWAP(x1, x2);
	}

	if (y1 > y2) {
		SWAP(y1, y2);
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

int16 gfxGetBit(int16 x, int16 y, byte *ptr, int16 width) {
	byte *ptrToData = (ptr) + y * width + x;

	if (x > width) {
		return 0;
	}

	if (*ptrToData) {
		return 0;
	}

	return (1);
}

void gfxResetRawPage(byte *pageRaw) {
	memset(pageRaw, 0, 320 * 200);
}

void gfxConvertSpriteToRaw(byte *dst, byte *src, uint16 width, uint16 height) {
	int x, y, d, bit, plane;

	width >>= 3;
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			byte data[2][4];
			data[0][0] = *src++;
			data[1][0] = *src++;
			data[0][1] = *src++;
			data[1][1] = *src++;
			data[0][2] = *src++;
			data[1][2] = *src++;
			data[0][3] = *src++;
			data[1][3] = *src++;
			for (d = 0; d < 2; ++d) {
				for (bit = 0; bit < 8; ++bit) {
					byte color = 0;
					for (plane = 0; plane < 4; ++plane) {
						if (data[d][plane] & (1 << (7 - bit))) {
							color |= 1 << plane;
						}
					}
					*dst++ = color;
				}
			}
		}
	}
}

void gfxCopyRawPage(byte *source, byte *dest) {
	memcpy(dest, source, 320 * 200);
}

void gfxFlipRawPage(byte *frontBuffer) {
	byte *page = frontBuffer;
	int x, y;
	byte *pixels = (byte *) screenBuffer;
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
			// This seems to match the output from DOSbox.
			pal[i * 4 + 2] = ((c_palette[i] & 0x00f) >> 0) * 32;
			pal[i * 4 + 1] = ((c_palette[i] & 0x0f0) >> 4) * 32;
			pal[i * 4 + 0] = ((c_palette[i] & 0xf00) >> 8) * 32;
			pal[i * 4 + 3] = 0;
		}
		g_system->setPalette(pal, 0, 16);
	}

	g_system->copyRectToScreen(screenBuffer, 320, 0, 0, 320, 200);
}

void drawSpriteRaw(byte *spritePtr, byte *maskPtr, int16 width, int16 height,
				   byte *page, int16 x, int16 y) {
	int16 i;
	int16 j;

	for (i = 0; i < height; i++) {
		byte *destPtr = page + x + y * 320;
		destPtr += i * 320;

		for (j = 0; j < width * 8; j++) {
			if (((gameType == Cine::GID_FW && !(*maskPtr)) || (gameType == Cine::GID_OS)) && (x + j >= 0
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

void drawSpriteRaw2(byte *spritePtr, byte transColor, int16 width, int16 height,
					byte *page, int16 x, int16 y) {
	int16 i;
	int16 j;

	for (i = 0; i < height; i++) {
		byte *destPtr = page + x + y * 320;
		destPtr += i * 320;

		for (j = 0; j < width * 8; j++) {
			if ((*(spritePtr) != transColor) && (x + j >= 0 && x + j < 320 && i + y >= 0 && i + y < 200)) {
				*(destPtr++) = *(spritePtr++);
			} else {
				destPtr++;
				spritePtr++;
			}
		}
	}
}

} // End of namespace Cine
