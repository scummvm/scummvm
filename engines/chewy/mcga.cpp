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

#include "common/system.h"
#include "graphics/palette.h"
#include "chewy/chewy.h"
#include "chewy/mcga.h"
#include "chewy/mcga_grafik.h"

namespace Chewy {

static byte saved_palette[PALETTE_SIZE];
static byte *screenP;
static bool screenHasDefault;
static byte *screenDefaultP;
static int spriteWidth;
static byte *fontAddr;
static size_t fontWidth, fontHeight;
static int fontFirst, fontLast;
static int fontX, fontY;

void init_mcga() {
	screenP = (byte *)g_engine->_screen->getPixels();
	screenHasDefault = false;
	screenDefaultP = nullptr;
	spriteWidth = 0;
	fontX = fontY = 0;
}

void old_mode() {
	// No implementation needed
}

void vflyback_start() {
	// No implementation needed
}

void vflyback_end() {
	// No implementation needed
}

void hflyback_start() {
	// No implementation needed
}

void hflyback_end() {
	// No implementation needed
}

void set_pointer(byte *ptr) {
	if (ptr) {
		screenP = ptr;
	} else if (screenHasDefault) {
		screenP = screenDefaultP;
	} else {
		screenP = (byte *)g_engine->_screen->getPixels();
	}
}

byte *get_dispoff() {
	return screenP;
}

void setpalette(byte *palette) {
	g_system->getPaletteManager()->setPalette(palette, 0, PALETTE_COUNT);
}

void save_palette(byte *palette) {
	if (!palette)
		palette = saved_palette;

	g_system->getPaletteManager()->grabPalette(palette, 0, PALETTE_COUNT);
}

void restore_palette() {
	setpalette(saved_palette);
}

void rastercol(int16 color, int16 r, int16 g, int16 b) {
	byte rgb[3];
	rgb[0] = r;
	rgb[1] = g;
	rgb[2] = b;

	g_system->getPaletteManager()->setPalette(&rgb[0], color, 1);
}

void set_palpart(byte *palette, int16 startcol, int16 anz) {
	g_system->getPaletteManager()->setPalette(palette, startcol, anz);
}

void clear_mcga() {
	if (screenP == (byte *)g_engine->_screen->getPixels())
		g_engine->_screen->clear();
	else
		Common::fill(screenP, screenP + SCREEN_WIDTH * SCREEN_HEIGHT, 0);
}

void setpixel_mcga(int16 x, int16 y, int16 farbe) {
	line_mcga(x, y, x, y, farbe);
}

uint8 getpix(int16 x, int16 y) {
	byte *pixel = (byte *)g_engine->_screen->getBasePtr(x, y);
	return *pixel;
}

void line_mcga(int16 x1, int16 y1, int16 x2, int16 y2, int16 farbe) {
	g_engine->_screen->drawLine(x1, y1, x2, y2, farbe);
}

void mem2mcga(const byte *ptr) {
	byte *destP = (byte *)g_engine->_screen->getPixels();
	Common::copy(ptr + 4, ptr + 4 + (SCREEN_WIDTH * SCREEN_HEIGHT), destP);
}

void mem2mcga_masked(const byte *ptr, int16 maske) {
	byte *destP = screenP;
	byte pixel;

	for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; ++i, ++ptr, ++destP) {
		pixel = *ptr;
		if (pixel != maske)
			*destP = pixel;
	}
}

void mcga2mem(byte *ptr) {
	const byte *srcP = screenP;
	*((uint16 *)ptr) = SCREEN_WIDTH;
	*((uint16 *)(ptr + 2)) = SCREEN_HEIGHT;

	Common::copy(srcP, srcP + (SCREEN_WIDTH * SCREEN_HEIGHT), ptr + 4);
}

void mem2mem(const byte *ptr1, byte *ptr2) {
	Common::copy(ptr1, ptr1 + (SCREEN_WIDTH * SCREEN_HEIGHT), ptr2);
}

void mem2mem_masked(const byte *ptr1, byte *ptr2, int16 maske) {
	byte pixel;

	for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; ++i, ++ptr1, ++ptr2) {
		pixel = *ptr1;
		if (pixel != maske)
			*ptr2 = pixel;
	}
}

void map_spr_2screen(const byte *sptr, int16 x, int16 y) {
	int width = *((const int16 *)sptr);
	sptr += y * width + x;
	byte *destP = screenP;

	for (int row = 0; row < SCREEN_HEIGHT;
			++row, sptr += width, destP += SCREEN_WIDTH) {
		Common::copy(sptr, sptr + SCREEN_WIDTH, destP);
	}
}

void spr_save_mcga(byte *sptr, int16 x, int16 y, int16 width,
		int16 height, int16 scrWidth) {
	int pitch;
	byte *scrP;
	*((int16 *)sptr) = width;
	sptr += 2;
	*((int16 *)sptr) = height;
	sptr += 2;

	if (scrWidth == 0) {
		scrP = screenP + y * SCREEN_WIDTH + x;
		pitch = SCREEN_WIDTH;
	} else {
		scrP = screenP + y * scrWidth + x;
		pitch = scrWidth;
	}

	if (width >= 1 && height >= 1) {
		for (int row = 0; row < height; ++row) {
			Common::copy(scrP, scrP + width, sptr);
			scrP += pitch;
		}
	}
}

void spr_set_mcga(const byte *sptr, int16 x, int16 y, int16 scrWidth) {
	int pitch;
	byte *scrP;
	int width = *((const int16 *)sptr);
	sptr += 2;
	int height = *((const int16 *)sptr);
	sptr += 2;

	if (width >= 1 && height >= 1) {
		if (scrWidth == 0) {
			scrP = screenP + y * SCREEN_WIDTH + x;
			pitch = SCREEN_WIDTH;
		} else {
			scrP = screenP + y * scrWidth + x;
			pitch = scrWidth;
		}

		for (int row = 0; row < height; ++row) {
			Common::copy(sptr, sptr + width, scrP);
			scrP += pitch;
		}
	}
}

static bool mspr_set_mcga_clip(int x, int y, int pitch, int &width, int &height, const byte *&srcP, byte *&destP) {
	if (y < clipy1) {
		int yDiff = ABS(clipy1 - y);
		height -= yDiff;
		srcP += yDiff * width;
		y = clipy1;
	}
	if (height < 1)
		return false;

	if (x < clipx1) {
		int xDiff = ABS(clipx1 - x);
		width -= xDiff;
		srcP += xDiff;
		x = clipx1;
	}
	if (width < 1)
		return false;

	int x2 = x + width;
	if (x2 > clipx2) {
		int xDiff = x2 - clipx2;
		width -= xDiff;
	}
	if (width <= 1)
		return false;

	int y2 = y + height;
	if (y2 > clipy2) {
		int yDiff = y2 - clipy2;
		height -= yDiff;
	}
	if (height < 1)
		return false;

	destP = screenP + pitch * y + x;
	return true;
}

void mspr_set_mcga(byte *sptr, int16 x, int16 y, int16 scrWidth) {
	if (!sptr)
		return;

	byte *destP;
	int width = *((const int16 *)sptr);
	sptr += 2;
	int height = *((const int16 *)sptr);
	sptr += 2;
	const byte *srcP = sptr;
	spriteWidth = width;

	if (!(height >= 1 && width >= 4))
		return;

	int pitch = scrWidth ? scrWidth : SCREEN_WIDTH;
	if (!mspr_set_mcga_clip(x, y, pitch, width, height, srcP, destP))
		return;
	int destPitchRemainder = pitch - width;
	int srcPitchRemainder = spriteWidth - width;

	for (int row = 0; row < height; ++row,
			srcP += srcPitchRemainder, destP += destPitchRemainder) {
		for (int col = 0; col < width; ++col, ++srcP, ++destP) {
			if (*srcP != 0)
				*destP = *srcP;
		}
	}
}

void setfont(byte *addr, int16 width, int16 height, int16 first, int16 last) {
	fontAddr = addr;
	fontWidth = width;
	fontHeight = height;
	fontFirst = first;
	fontLast = last;
}

void upd_scr() {
	g_engine->_screen->markAllDirty();
	g_engine->_screen->update();
}

void vors() {
	fontX = fvorx;
	fontY = fvory;
}

void zoom_img(byte *source, byte *dest, int16 xdiff_, int16 ydiff_) {
	warning("STUB - zoom_img");
}

void zoom_set(byte *source, int16 x, int16 y, int16 xdiff_, int16 ydiff_, int16 scrWidth) {
	warning("STUB - zoom_set");
}

void putcxy(int16 x, int16 y, char zeichen, int16 forcol, int16 backcol, int16 scrWidth) {
	warning("STUB - putcxy");
}

void putz(char c, int16 fgCol, int16 bgCol, int16 scrWidth) {
	warning("STUB - putz");
}

void init_svga(VesaInfo *vi_, byte *virt_screen) {
	warning("STUB - init_svga");
}

bool kbhit() {
	warning("STUB: kbhit()");

	return false;
}

char getch() {
	warning("STUB: getch()");

	return ' ';
}

void putch(char c) {
	warning("STUB: putch()");
}

} // namespace Chewy
