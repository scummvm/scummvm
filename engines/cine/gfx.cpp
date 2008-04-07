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

#include "cine/cine.h"
#include "cine/bg.h"
#include "cine/various.h"

#include "common/endian.h"
#include "common/system.h"

#include "graphics/cursorman.h"

namespace Cine {

uint16 c_palette[256];
byte colorMode256 = 0;
byte palette256[256 * 3];
byte newPalette[256 * 3];
byte newColorMode = 0;
byte ctColorMode = 0;
byte bgColorMode = 0;

byte *screenBuffer;
byte *page1Raw;
byte *page2Raw;
byte *page3Raw;

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

static const byte cursorPalette[] = {
	0, 0, 0, 0xff,
	0xff, 0xff, 0xff, 0xff
};

void gfxInit() {
	screenBuffer = (byte *)malloc(320 * 200);
	page1Raw = (byte *)malloc(320 * 200);
	page2Raw = (byte *)malloc(320 * 200);
	page3Raw = (byte *)malloc(320 * 200);
	if (!screenBuffer || !page1Raw || !page2Raw || !page3Raw) {
		error("Unable to allocate offscreen buffers");
	}
	memset(page1Raw, 0, 320 * 200);
	memset(page2Raw, 0, 320 * 200);
	memset(page3Raw, 0, 320 * 200);

	memset(additionalBgTable, 0, sizeof(additionalBgTable));
	additionalBgTable[0] = page2Raw;
	additionalBgTable[8] = page3Raw;
}

void gfxDestroy() {
	free(screenBuffer);
	free(page1Raw);
	free(page2Raw);
	free(page3Raw);
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
					mouseCursor[offs] = 1;
				} else if (src[32] & mask) {
					mouseCursor[offs] = 0;
				} else {
					mouseCursor[offs] = 0xFF;
				}
				++offs;
			}
			++src;
		}
		CursorMan.replaceCursor(mouseCursor, 16, 16, mc->hotspotX, mc->hotspotY);
		CursorMan.replaceCursorPalette(cursorPalette, 0, 2);
		currentMouseCursor = cursor;
	}
}

static uint16 transformColor(uint16 baseColor, int8 r, int8 g, int8 b) {
	int8 oriR = CLIP( (baseColor & 0x007)       + r, 0, 7);
	int8 oriG = CLIP(((baseColor & 0x070) >> 4) + g, 0, 7);
	int8 oriB = CLIP(((baseColor & 0x700) >> 8) + b, 0, 7);

	return oriR | (oriG << 4) | (oriB << 8);
}

void transformPaletteRange(byte startColor, byte stopColor, int8 r, int8 g, int8 b) {
	for (byte i = startColor; i <= stopColor; i++) {
		c_palette[i] = transformColor(tempPalette[i], b, g, r);
	}
	//gfxFlipPage(page2);
}

void gfxFillSprite(const byte *spritePtr, uint16 width, uint16 height, byte *page, int16 x, int16 y, uint8 fillColor) {
	int16 i, j;

	for (i = 0; i < height; i++) {
		byte *destPtr = page + x + y * 320;
		destPtr += i * 320;

		for (j = 0; j < width * 8; j++) {
			if (x + j >= 0 && x + j < 320 && i + y >= 0
			    && i + y < 200) {
				if (!*(spritePtr++)) {
					*(destPtr++) = fillColor;
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

void gfxDrawMaskedSprite(const byte *spritePtr, const byte *maskPtr, uint16 width, uint16 height, byte *page, int16 x, int16 y) {
	int16 i, j;

	for (i = 0; i < height; i++) {
		byte *destPtr = page + x + y * 320;
		destPtr += i * 320;

		for (j = 0; j < width * 8; j++) {
			if (x + j >= 0 && x + j < 320 && i + y >= 0 && i + y < 200 && *maskPtr == 0) {
				*destPtr = *spritePtr;
			}
			++destPtr;
			++spritePtr;
			++maskPtr;
		}
	}
}

void gfxUpdateSpriteMask(const byte *spritePtr, const byte *spriteMskPtr, int16 width, int16 height, const byte *maskPtr,
	int16 maskWidth, int16 maskHeight, byte *bufferSprPtr, byte *bufferMskPtr, int16 xs, int16 ys, int16 xm, int16 ym, byte maskIdx) {
	int16 i, j, d, spritePitch, maskPitch;

	width *= 8;
	maskWidth *= 8;

	spritePitch = width;
	maskPitch = maskWidth;

	if (maskIdx == 0) {
		memcpy(bufferSprPtr, spritePtr, spritePitch * height);
		memcpy(bufferMskPtr, spriteMskPtr, spritePitch * height);
	}

	if (ys > ym) {
		d = ys - ym;
		maskPtr += d * maskPitch;
		maskHeight -= d;
	}
	if (maskHeight <= 0) {
		return;
	}
	if (xs > xm) {
		d = xs - xm;
		maskPtr += d;
		maskWidth -= d;
	}
	if (maskWidth <= 0) {
		return;
	}
	if (ys < ym) {
		d = ym - ys;
		spriteMskPtr += d * spritePitch;
		bufferMskPtr += d * spritePitch;
		height -= d;
	}
	if (height <= 0) {
		return;
	}
	if (xs < xm) {
		d = xm - xs;
		spriteMskPtr += d;
		bufferMskPtr += d;
		width -= d;
	}
	if (width <= 0) {
		return;
	}
	for (j = 0; j < MIN(maskHeight, height); ++j) {
		for (i = 0; i < MIN(maskWidth, width); ++i) {
			bufferMskPtr[i] |= maskPtr[i] ^ 1;
		}
		bufferMskPtr += spritePitch;
		maskPtr += maskPitch;
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

int16 gfxGetBit(int16 x, int16 y, const byte *ptr, int16 width) {
	const byte *ptrToData = (ptr) + y * width + x;

	if (x > width) {
		return 0;
	}

	if (*ptrToData) {
		return 0;
	}

	return 1;
}

void gfxResetRawPage(byte *pageRaw) {
	memset(pageRaw, 0, 320 * 200);
}

void gfxConvertSpriteToRaw(byte *dst, const byte *src, uint16 w, uint16 h) {
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w / 8; ++x) {
			for (int bit = 0; bit < 16; ++bit) {
				uint8 color = 0;
				for (int p = 0; p < 4; ++p) {
					if (READ_BE_UINT16(src + p * 2) & (1 << (15 - bit))) {
						color |= 1 << p;
					}
				}
				*dst++ = color;
			}
			src += 8;
		}
	}
}

void gfxCopyRawPage(byte *source, byte *dest) {
	memcpy(dest, source, 320 * 200);
}

void gfxFlipRawPage(byte *frontBuffer) {
	byte *page = frontBuffer;
	int x, y, i;
	byte *pixels = (byte *) screenBuffer;
	byte c;
	byte pal[256 * 4];

	for (y = 0; y < 200; y++) {
		for (x = 0; x < 320; x++) {
			c = *(page++);

			if (!colorMode256) {
				c = c & 15;
			}

			pixels[x + 0 + y * 320] = c;
		}
	}

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

void drawSpriteRaw(const byte *spritePtr, const byte *maskPtr, int16 width, int16 height,
				   byte *page, int16 x, int16 y) {
	int16 i, j;

	// FIXME: Is it a bug if maskPtr == NULL?
	if (!maskPtr)
		warning("drawSpriteRaw: maskPtr == NULL");

	for (i = 0; i < height; i++) {
		byte *destPtr = page + x + y * 320;
		destPtr += i * 320;

		for (j = 0; j < width * 8; j++) {
			if (((g_cine->getGameType() == Cine::GType_FW && (!maskPtr || !(*maskPtr))) || (g_cine->getGameType() == Cine::GType_OS)) && (x + j >= 0
					&& x + j < 320 && i + y >= 0 && i + y < 200)) {
				*(destPtr++) = *(spritePtr++);
			} else {
				destPtr++;
				spritePtr++;
			}

			if (maskPtr)
				maskPtr++;
		}
	}
}

void drawSpriteRaw2(const byte *spritePtr, byte transColor, int16 width, int16 height,
					byte *page, int16 x, int16 y) {
	int16 i, j;

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

void maskBgOverlay(const byte *bgPtr, const byte *maskPtr, int16 width, int16 height,
				   byte *page, int16 x, int16 y) {
	int16 i, j;

	for (i = 0; i < height; i++) {
		byte *destPtr = page + x + y * 320;
		const byte *srcPtr = bgPtr + x + y * 320;
		destPtr += i * 320;
		srcPtr += i * 320;

		for (j = 0; j < width * 8; j++) {
			if ((!maskPtr || !(*maskPtr)) && (x + j >= 0
					&& x + j < 320 && i + y >= 0 && i + y < 200)) {
				*destPtr = *srcPtr;
			}

			destPtr++;
			srcPtr++;
			maskPtr++;
		}
	}
}

/*! \todo Fix rendering to prevent fadein artifacts
 */
void fadeFromBlack() {
	int i, j;
	int r, g, b, tr, tg, tb;
	if (newColorMode == 2) {
		colorMode256 = 1;
		memset(palette256, 0, 256*3);
	} else if (newColorMode == 1) {
		colorMode256 = 0;
		memset(c_palette, 0, 16 * sizeof(uint16));
	}

	for (i = 0; i < 8; i++ ) {
		gfxFlipRawPage(page1Raw);
		g_system->updateScreen();
		g_system->delayMillis(50);

		if (colorMode256) {
			for (j = 0; j < 256*3; j++) {
				r = palette256[j] + (newPalette[j] + 7) / 8;
				palette256[j] = CLIP(r, 0, (int)newPalette[j]);
			}
		} else {
			for (j = 0; j < 16; j++) {
				r = c_palette[j] & 0xf;
				g = (c_palette[j] & 0xf0) >> 4;
				b = (c_palette[j] & 0xf00) >> 8;

				tr = tempPalette[j] & 0xf;
				tg = (tempPalette[j] & 0xf0) >> 4;
				tb = (tempPalette[j] & 0xf00) >> 8;

				r = CLIP(r + (tr + 7) / 8, 0, tr);
				g = CLIP(g + (tg + 7) / 8, 0, tg);
				b = CLIP(b + (tb + 7) / 8, 0, tb);

				c_palette[j] = r | (g << 4) | (b << 8);
			}

		}
	}

	if (colorMode256) {
		memcpy(palette256, newPalette, 256*3);
	} else {
		memcpy(c_palette, tempPalette, sizeof(uint16) * 16);
	}
}

void fadeToBlack() {
	for (int i = 0; i < 8; i++) {
		if (colorMode256) {
			for (int j = 0; j < 256*3; j++) {
				palette256[j] = CLIP(palette256[j] - 32, 0, 255);
			}
		} else {
			for (int j = 0; j < 16; j++) {
				c_palette[j] = transformColor(c_palette[j], -1, -1, -1);
			}
		}
		gfxFlipRawPage(page1Raw);
		g_system->updateScreen();
		g_system->delayMillis(50);
	}
}

void blitRawScreen(byte *frontBuffer) {
	gfxFlipRawPage(frontBuffer);
}

void flip(void) {
	blitRawScreen(page1Raw);
	if (fadeRequired) {
		if (newColorMode == 3) {
			newColorMode = ctColorMode + 1;
		}

		if (newColorMode == 2) {
			colorMode256 = 1;
			memcpy(palette256, newPalette, 256*3);
		} else {
			colorMode256 = 0;
			memcpy(c_palette, tempPalette, sizeof(uint16) * 16);
		}
		fadeRequired = false;
	}
}

} // End of namespace Cine
