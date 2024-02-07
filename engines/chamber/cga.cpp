/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/system.h"
#include "graphics/paletteman.h"

#include "chamber/chamber.h"
#include "chamber/common.h"
#include "chamber/input.h"
#include "chamber/resdata.h"
#include "chamber/cga.h"

namespace Chamber {

#if 0
#define HGA_WIDTH 720
#define HGA_HEIGHT 348
#define HGA_BASE_SEG 0xB000
#define HGA_PAGE2_SEG 0xB800
#define HGA_NEXT_LINES_OFS 0x2000
#define HGA_BITS_PER_PIXEL 1
#define HGA_PIXELS_PER_BYTE (8 / HGA_BITS_PER_PIXEL)
#define HGA_BYTES_PER_LINE (HGA_WIDTH / HGA_PIXELS_PER_BYTE)
#define HGA_CALCXY_RAW(x, y) ( ((y) % 4) * HGA_NEXT_LINES_OFS + ((y) / 4) * HGA_BYTES_PER_LINE + (x) / HGA_PIXELS_PER_BYTE )
#define HGA_CENTERED_BASE_OFS HGA_CALCXY_RAW(32, 76)
#ifdef __386__
#define HGA_SCREENBUFFER ((byte*)(HGA_BASE_SEG * 16))
#define HGA_BACKBUFFER ((byte*)(HGA_PAGE2_SEG * 16))
#else
#define HGA_SCREENBUFFER ((byte*)MK_FP(HGA_BASE_SEG, 0))
#define HGA_BACKBUFFER ((byte*)MK_FP(HGA_PAGE2_SEG, 0))
#endif
#define HGA_FONT_HEIGHT 6
#define frontbuffer HGA_SCREENBUFFER
#define backbuffer HGA_BACKBUFFER
/* Calc screen offset from normal pixel coordinates
Out:
  screen offset
*/
uint16 HGA_CalcXY(uint16 x, uint16 y) {
	return HGA_CalcXY_p(x / 4, y);
}

/* Calc screen offset from packed pixel coordinates
Out:
  screen offset
*/
uint16 HGA_CalcXY_p(uint16 x, uint16 y) {
	 uint16 ofs = HGA_CENTERED_BASE_OFS; /*initial offset for centering*/
#if 0
	 ofs += HGA_CALCXY_RAW(x * 4 * 2, y);
#else
	/*optimized version of the above*/
	if (y & 1)
		ofs |= HGA_NEXT_LINES_OFS;
	if (y & 2)
		ofs |= HGA_NEXT_LINES_OFS * 2;
	y /= 4;
	ofs += y * HGA_BYTES_PER_LINE;
	ofs += x; /*one unit of x represent 4 pixels, but one unit of ofs is 8 pixels, so x is implicitly multiplied by 2 here*/
#endif
	 return ofs;
}
#endif

extern byte backbuffer[0x4000];
byte CGA_SCREENBUFFER[0x4000];
byte scrbuffer[320*200];

extern byte *scratch_mem2;

byte char_draw_coords_x;
byte char_draw_coords_y;
byte *char_xlat_table;
byte string_ended;
byte char_draw_max_width;
byte char_draw_max_height;

/*pixels order reverse in a byte*/
byte cga_pixel_flip[256] = {
	0, 64, 128, 192, 16, 80, 144, 208, 32, 96, 160, 224,
	48, 112, 176, 240, 4, 68, 132, 196, 20, 84, 148, 212,
	36, 100, 164, 228, 52, 116, 180, 244, 8, 72, 136, 200,
	24, 88, 152, 216, 40, 104, 168, 232, 56, 120, 184, 248,
	12, 76, 140, 204, 28, 92, 156, 220, 44, 108, 172, 236,
	60, 124, 188, 252, 1, 65, 129, 193, 17, 81, 145, 209,
	33, 97, 161, 225, 49, 113, 177, 241, 5, 69, 133, 197,
	21, 85, 149, 213, 37, 101, 165, 229, 53, 117, 181, 245,
	9, 73, 137, 201, 25, 89, 153, 217, 41, 105, 169, 233,
	57, 121, 185, 249, 13, 77, 141, 205, 29, 93, 157, 221,
	45, 109, 173, 237, 61, 125, 189, 253, 2, 66, 130, 194,
	18, 82, 146, 210, 34, 98, 162, 226, 50, 114, 178, 242,
	6, 70, 134, 198, 22, 86, 150, 214, 38, 102, 166, 230,
	54, 118, 182, 246, 10, 74, 138, 202, 26, 90, 154, 218,
	42, 106, 170, 234, 58, 122, 186, 250, 14, 78, 142, 206,
	30, 94, 158, 222, 46, 110, 174, 238, 62, 126, 190, 254,
	3, 67, 131, 195, 19, 83, 147, 211, 35, 99, 163, 227,
	51, 115, 179, 243, 7, 71, 135, 199, 23, 87, 151, 215,
	39, 103, 167, 231, 55, 119, 183, 247, 11, 75, 139, 203,
	27, 91, 155, 219, 43, 107, 171, 235, 59, 123, 187, 251,
	15, 79, 143, 207, 31, 95, 159, 223, 47, 111, 175, 239,
	63, 127, 191, 255
};

static const uint8 PALETTE_CGA[4 * 3] = {
	0x00, 0x00, 0x00, // black
	0x55, 0xff, 0xff, // cyan
	0xff, 0x55, 0xff, // magenta
	0xff, 0xff, 0xff
};

static const uint8 PALETTE_CGA2[4 * 3] = {
	0x00, 0x00, 0x00, // black
	0x55, 0xff, 0x55, // green
	0xff, 0x55, 0x55, // red
	0xff, 0xff, 0x55  // yellow
};

/*
  Switch to CGA 320x200x2bpp mode
*/
void switchToGraphicsMode(void) {
	g_system->getPaletteManager()->setPalette(PALETTE_CGA, 0, 4);
}

/*
  Switch to text mode
*/
void switchToTextMode(void) {
	warning("STUB: switchToTextMode()");
}

void waitVBlank(void) {
	pollInput();
	g_system->delayMillis(10);
}

void cga_ColorSelect(byte csel) {
	const byte *pal;
	if (csel & 0x10)
		pal = PALETTE_CGA;
	else
		pal = PALETTE_CGA2;

	g_system->getPaletteManager()->setPalette(pal, 0, 4);
	g_system->setCursorPalette(pal, 0, 4);
}

void cga_blitToScreen(int16 dx, int16 dy, int16 w, int16 h) {
	dx = dy = 0;
	w = 320; h = 200;
	// Align x by 4
	int16 align = dx & 0x3;

	dx -= align;
	w += align;

	if (dy + h >= 200)
		h = 200 - dy;

	if (dx + w >= 320)
		w = 320 - dx;

	w = (w + 3) / 4;

	for (int16 y = 0; y < h; y++) {
		byte *src = CGA_SCREENBUFFER + cga_CalcXY(dx, dy + y);
		byte *dst = scrbuffer + (y + dy) * 320 + dx;

		for (int16 x = 0; x < w; x++) {
			byte colors = *src++;

			for (int16 c = 0; c < 4; c++) {
				byte color = (colors & 0xC0) >> 6;
				colors <<= 2;

				*dst++ = color;
			}
		}
	}

	g_system->copyRectToScreen(scrbuffer + dy * 320 + dx, 320, dx, dy, w * 4, h);
	g_system->updateScreen();
}

void cga_blitToScreen(int16 ofs, int16 w, int16 h) {
	int16 dy = ofs / 80;
	int16 dx = (ofs % 80) * 4;

	cga_blitToScreen(dx, dy, w, h);
}

void cga_BackBufferToRealFull(void) {
	memcpy(CGA_SCREENBUFFER, backbuffer, sizeof(backbuffer));

	cga_blitToScreen(0, 0, 320, 200);
}

void cga_RealBufferToBackFull(void) {
	memcpy(backbuffer, CGA_SCREENBUFFER, sizeof(backbuffer));
}

/*Copy interlaced screen data to another screen*/
/*NB! w is in bytes*/
void cga_CopyScreenBlock(byte *source, uint16 w, uint16 h, byte *target, uint16 ofs) {
	uint16 oh = h;
	uint16 oofs = ofs;
	while (h--) {
		memcpy(target + ofs, source + ofs, w);
		ofs ^= CGA_ODD_LINES_OFS;
		if ((ofs & CGA_ODD_LINES_OFS) == 0)
			ofs += CGA_BYTES_PER_LINE;
	}

	if (target == CGA_SCREENBUFFER)
		cga_blitToScreen(oofs, w * 4, oh);
}

/*
Flip screen and backbuffer
*/
void cga_SwapRealBackBuffer(void) {
	uint16 i;
	uint16 *s, *d;
	waitVBlank();
	s = (uint16 *)CGA_SCREENBUFFER;
	d = (uint16 *)backbuffer;
	for (i = 0; i < sizeof(backbuffer) / 2; i++) {
		uint16 t = *s;
		*s++ = *d;
		*d++ = t;
	}

	cga_blitToScreen(0, 0, 320, 200);
}


/*
Copy current screen's pixels to scratch mem, put new pixels to screen
*/
void cga_SwapScreenRect(byte *pixels, uint16 w, uint16 h, byte *screen, uint16 ofs) {
	byte *old = scratch_mem2;
	uint16 oh = h;
	uint16 oofs = ofs;
	while (h--) {
		uint16 i;
		for (i = 0; i < w; i++) {
			*old++ = screen[ofs + i];
			screen[ofs + i] = *pixels++;
		}
		ofs ^= CGA_ODD_LINES_OFS;
		if ((ofs & CGA_ODD_LINES_OFS) == 0)
			ofs += CGA_BYTES_PER_LINE;
	}

	if (screen == CGA_SCREENBUFFER)
		cga_blitToScreen(oofs, w * 4, oh);
}

/*
Calc screen offset from normal pixel coordinates
Out:
  screen offset
*/
uint16 cga_CalcXY(uint16 x, uint16 y) {
	return cga_CalcXY_p(x / 4, y);
}

/*
Calc screen offset from packed pixel coordinates
Out:
  screen offset
*/
uint16 cga_CalcXY_p(uint16 x, uint16 y) {
	uint16 ofs = 0;
	if (y & 1)
		ofs += CGA_ODD_LINES_OFS;
	ofs += CGA_BYTES_PER_LINE * (y / 2);
	ofs += x;
	return ofs;
}

/*
backup screen rect to a buffer
Out:
  next buffer ptr
*/
byte *cga_BackupImage(byte *screen, uint16 ofs, uint16 w, uint16 h, byte *buffer) {
	*(byte *)(buffer + 0) = h;
	*(byte *)(buffer + 1) = w;
	*(uint16 *)(buffer + 2) = ofs;
	buffer += 4;
	while (h--) {
		memcpy(buffer, screen + ofs, w);
		buffer += w;
		ofs ^= CGA_ODD_LINES_OFS;
		if ((ofs & CGA_ODD_LINES_OFS) == 0)
			ofs += CGA_BYTES_PER_LINE;
	}
	return buffer;
}

byte *cga_BackupImageReal(uint16 ofs, uint16 w, uint16 h) {
	return cga_BackupImage(CGA_SCREENBUFFER, ofs, w, h, scratch_mem2);
}

/*
Blit progressive image to interlaced screen buffer
NB! width and pixelswidth specify a number of bytes, not count of pixels
*/
void cga_Blit(byte *pixels, uint16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs) {
	byte *src = pixels;
	uint16 oofs = ofs;
	for (int16 y = 0; y < h; y++) {
		memcpy(screen + ofs, src, w);
		src += pw;
		ofs ^= CGA_ODD_LINES_OFS;
		if ((ofs & CGA_ODD_LINES_OFS) == 0)
			ofs += CGA_BYTES_PER_LINE;
	}

	if (screen == CGA_SCREENBUFFER)
		cga_blitToScreen(oofs, w * 4, h);
}

/*
Blit progressive image to interlaced screen buffer, then wait for VBlank
NB! width and pixelswidth specify a number of bytes, not count of pixels
*/
void cga_BlitAndWait(byte *pixels, uint16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs) {
	cga_Blit(pixels, pw, w, h, screen, ofs);
	waitVBlank();
}

void cga_Fill(byte pixel, uint16 w, uint16 h, byte *screen, uint16 ofs) {
	uint16 oofs = ofs;
	for (int16 y = 0; y < h; y++) {
		memset(screen + ofs, pixel, w);
		ofs ^= CGA_ODD_LINES_OFS;
		if ((ofs & CGA_ODD_LINES_OFS) == 0)
			ofs += CGA_BYTES_PER_LINE;
	}

	if (screen == CGA_SCREENBUFFER)
		cga_blitToScreen(oofs, w * 4, h);
}

void cga_FillAndWait(byte pixel, uint16 w, uint16 h, byte *screen, uint16 ofs) {
	cga_Fill(pixel, w, h, screen, ofs);
	waitVBlank();
}

/*
Restore saved image to target screen buffer
*/
void cga_RestoreImage(byte *buffer, byte *target) {
	uint16 w, h;
	uint16 ofs;

	if (!buffer)
		return;

	h = *(byte *)(buffer + 0);
	w = *(byte *)(buffer + 1);
	ofs = *(uint16 *)(buffer + 2);
	buffer += 4;    /*TODO: fix me for large int*/

	cga_Blit(buffer, w, w, h, target, ofs);
}

/*
Restore saved image from scratch mem to target screen buffer
*/
void cga_RestoreBackupImage(byte *target) {
	cga_RestoreImage(scratch_mem2, target);
}

/*
Copy image's real screen data to backbuffer
*/
void cga_RefreshImageData(byte *buffer) {
	uint16 w, h;
	uint16 ofs;

	if (!buffer)
		return;

	h = *(byte *)(buffer + 0);
	w = *(byte *)(buffer + 1);
	ofs = *(uint16 *)(buffer + 2);

	cga_CopyScreenBlock(CGA_SCREENBUFFER, w, h, backbuffer, ofs);
}

/*
Draw a vertical line with origin x:y and length l, using color
NB! Line must not wrap around the edge
*/
void cga_DrawVLine(uint16 x, uint16 y, uint16 l, byte color, byte *target) {
	uint16 ofs;
	/*pixels are starting from top bits of byte*/
	uint16 mask = static_cast<uint16>((~(3 << ((CGA_PIXELS_PER_BYTE - 1) * CGA_BITS_PER_PIXEL))) & 0xffff);
	byte pixel = color << ((CGA_PIXELS_PER_BYTE - 1) * CGA_BITS_PER_PIXEL);

	mask >>= (x % CGA_PIXELS_PER_BYTE) * CGA_BITS_PER_PIXEL;
	pixel >>= (x % CGA_PIXELS_PER_BYTE) * CGA_BITS_PER_PIXEL;

	ofs = cga_CalcXY_p(x / CGA_PIXELS_PER_BYTE, y);

	uint16 ol = l;
	while (l--) {
		target[ofs] = (target[ofs] & mask) | pixel;
		ofs ^= CGA_ODD_LINES_OFS;
		if ((ofs & CGA_ODD_LINES_OFS) == 0)
			ofs += CGA_BYTES_PER_LINE;
	}

	if (target == CGA_SCREENBUFFER)
		cga_blitToScreen(x, y, 1, ol);
}

/*
Draw a horizontal line with origin x:y and length l, using color
NB! Line must not wrap around the edge
*/
void cga_DrawHLine(uint16 x, uint16 y, uint16 l, byte color, byte *target) {
	uint16 ofs;
	/*pixels are starting from top bits of byte*/
	uint16 mask = static_cast<uint16>((~(3 << ((CGA_PIXELS_PER_BYTE - 1) * CGA_BITS_PER_PIXEL))) & 0xffff);
	byte pixel = color << ((CGA_PIXELS_PER_BYTE - 1) * CGA_BITS_PER_PIXEL);

	mask >>= (x % CGA_PIXELS_PER_BYTE) * CGA_BITS_PER_PIXEL;
	pixel >>= (x % CGA_PIXELS_PER_BYTE) * CGA_BITS_PER_PIXEL;

	ofs = cga_CalcXY_p(x / CGA_PIXELS_PER_BYTE, y);
	uint16 ol = l;
	while (l--) {
		target[ofs] = (target[ofs] & mask) | pixel;
		mask >>= CGA_BITS_PER_PIXEL;
		pixel >>= CGA_BITS_PER_PIXEL;
		if (mask == 0xFF) {
			ofs++;
			mask = static_cast<uint16>((~(3 << ((CGA_PIXELS_PER_BYTE - 1) * CGA_BITS_PER_PIXEL))) & 0xffff);
			pixel = color << ((CGA_PIXELS_PER_BYTE - 1) * CGA_BITS_PER_PIXEL);
		}
	}
	if (target == CGA_SCREENBUFFER)
		cga_blitToScreen(x, y, ol, 1);
}

/*
Draw horizontal line of length l with color, add surrounding pixels (bmask, bpix, left in high byte, right in low)
Return next line screen offset
NB! Length specifies byte lenght of inner segment, not amount of pixels
 */
uint16 cga_DrawHLineWithEnds(uint16 bmask, uint16 bpix, byte color, uint16 l, byte *target, uint16 ofs) {
	target[ofs] = (target[ofs] & (bmask >> 8)) | (bpix >> 8);
	memset(target + ofs + 1, color, l);
	target[ofs + 1 + l] = (target[ofs + 1 + l] & (bmask & 255)) | (bpix & 255);
	uint16 oofs = ofs;
	ofs ^= CGA_ODD_LINES_OFS;
	if ((ofs & CGA_ODD_LINES_OFS) == 0)
		ofs += CGA_BYTES_PER_LINE;

	if (target == CGA_SCREENBUFFER)
		cga_blitToScreen(oofs, l * 4 + 2, 1);

	return ofs;
}

/*
Print a character at current cursor pos, then advance
*/
void cga_PrintChar(byte c, byte *target) {
	uint16 i;
	byte *font = carpc_data + c * CGA_FONT_HEIGHT;
	uint16 ofs = cga_CalcXY_p(char_draw_coords_x++, char_draw_coords_y);
	for (i = 0; i < CGA_FONT_HEIGHT; i++) {
		c = *font++;
		c = char_xlat_table[c];
		target[ofs] = c;
		ofs ^= CGA_ODD_LINES_OFS;
		if ((ofs & CGA_ODD_LINES_OFS) == 0)
			ofs += CGA_BYTES_PER_LINE;
	}

	if (target == CGA_SCREENBUFFER)
		cga_blitToScreen((char_draw_coords_x - 1) * 4, char_draw_coords_y, 4, CGA_FONT_HEIGHT);
}


/*
Blit progressive sprite (mask+pixel) from scratch buffer to interlaced screen buffer, using backbuffer pixels for transparency
NB! width specify a number of bytes, not count of pixels
TODO: generalize/merge me with BlitSprite
*/
void cga_BlitScratchBackSprite(uint16 sprofs, uint16 w, uint16 h, byte *screen, uint16 ofs) {
	byte x;
	byte *pixels = scratch_mem2 + 2 + sprofs;
	uint16 oh = h;
	uint16 oofs = ofs;
	while (h--) {
		for (x = 0; x < w; x++)
			screen[ofs + x] = (backbuffer[ofs + x] & pixels[x * 2]) | pixels[x * 2 + 1];
		pixels += w * 2;
		ofs ^= CGA_ODD_LINES_OFS;
		if ((ofs & CGA_ODD_LINES_OFS) == 0)
			ofs += CGA_BYTES_PER_LINE;
	}

	if (screen == CGA_SCREENBUFFER)
		cga_blitToScreen(oofs, w * 4, oh);
}

void cga_BlitFromBackBuffer(byte w, byte h, byte *screen, uint16 ofs) {
	cga_CopyScreenBlock(backbuffer, w, h, screen, ofs);
}

/*
Blit progressive sprite (mask+pixel) to interlaced screen buffer
NB! width and pixelswidth specify a number of bytes, not count of pixels
*/
void cga_BlitSprite(byte *pixels, int16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs) {
	byte x;
	uint16 oh = h;
	uint16 oofs = ofs;
	while (h--) {
		for (x = 0; x < w; x++)
			screen[ofs + x] = (screen[ofs + x] & pixels[x * 2]) | pixels[x * 2 + 1];
		pixels += pw;
		ofs ^= CGA_ODD_LINES_OFS;
		if ((ofs & CGA_ODD_LINES_OFS) == 0)
			ofs += CGA_BYTES_PER_LINE;
	}

	if (screen == CGA_SCREENBUFFER)
		cga_blitToScreen(oofs, w * 4, oh);
}

/*
Blit progressive sprite (mask+pixel) to interlaced screen buffer. Flip the sprite horizontally
NB! width and pixelswidth specify a number of bytes, not count of pixels
*/
void cga_BlitSpriteFlip(byte *pixels, int16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs) {
	byte x;
	uint16 oh = h;
	uint16 oofs = ofs;
	while (h--) {
		for (x = 0; x < w; x++)
			screen[ofs - x] = (screen[ofs - x] & cga_pixel_flip[pixels[x * 2]]) | cga_pixel_flip[pixels[x * 2 + 1]];
		pixels += pw;
		ofs ^= CGA_ODD_LINES_OFS;
		if ((ofs & CGA_ODD_LINES_OFS) == 0)
			ofs += CGA_BYTES_PER_LINE;
	}

	if (screen == CGA_SCREENBUFFER)
		cga_blitToScreen(oofs, w * 4, oh);
}

/*
Blit progressive sprite (pixel+mask) to interlaced screen buffer
Backup original screen data to buffer
Apply extra mask to source pixels when drawing
Used to draw mouse cursor and backup what's under it
NB! width and pixelswidth specify a number of bytes, not count of pixels
NB! pixel+mask comes in reversed order, compared to regular BlitSprite
*/
void cga_BlitSpriteBak(byte *pixels, int16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs, byte *backup, byte mask) {
	byte x;
	uint16 oh = h;
	uint16 oofs = ofs;
	while (h--) {
		for (x = 0; x < w; x++) {
			*backup++ = screen[ofs + x];
			screen[ofs + x] = (screen[ofs + x] & pixels[x * 2 + 1]) | (pixels[x * 2] & mask);
		}
		pixels += pw * 2;
		ofs ^= CGA_ODD_LINES_OFS;
		if ((ofs & CGA_ODD_LINES_OFS) == 0)
			ofs += CGA_BYTES_PER_LINE;
	}

	if (screen == CGA_SCREENBUFFER)
		cga_blitToScreen(oofs, w * 4, oh);
}


/*
Blit progressive sprite (w+h+mask+pixel) to interlaced screen buffer
*/
void drawSprite(byte *sprite, byte *screen, uint16 ofs) {
	byte w, h;
	w = *sprite++;
	h = *sprite++;
	cga_BlitSprite(sprite, w * 2, w, h, screen, ofs);
}

/*
Blit progressive sprite (w+h+mask+pixel) to interlaced screen buffer, horizontally flipped
*/
void drawSpriteFlip(byte *sprite, byte *screen, uint16 ofs) {
	byte w, h;
	w = *sprite++;
	h = *sprite++;
	cga_BlitSpriteFlip(sprite, w * 2, w, h, screen, ofs);
}

/*
Load and uncompress 2-bit sprite
Return next ptr after the loaded sprite
*/
byte *loadSprite(byte index, byte *bank, byte *buffer, byte header_only) {
	byte w, h;
	uint16 rsize;
	byte *sprite, *sprite_end;
	byte *bitmask;
	sprite = seekToEntryW(bank, index, &sprite_end);
	w = *sprite++;
	h = *sprite++;
	rsize = w * h;  /*raster part size*/
	bitmask = sprite + rsize;

	*buffer++ = w;
	*buffer++ = h;
	if (header_only) {
		memset(buffer, 0, rsize * 2);   /*wipe masks+pixels*/
		buffer += rsize * 2;
	} else {
		if (*bitmask == 0) {
			/*solid sprite*/
			while (rsize--) {
				*buffer++ = 0;
				*buffer++ = *sprite++;
			}
		} else {
			/*with transparency*/
			byte bi = 1;
			while (rsize--) {
				byte pixels = *sprite++;
				byte mask = 0;

				if ((pixels & 0xC0) == 0) {
					bi >>= 1;
					if (bi == 0) {
						bi = 0x80;
						bitmask++;
					}
					if (*bitmask & bi)
						mask |= 0xC0;
				}

				if ((pixels & 0x30) == 0) {
					bi >>= 1;
					if (bi == 0) {
						bi = 0x80;
						bitmask++;
					}
					if (*bitmask & bi)
						mask |= 0x30;
				}

				if ((pixels & 0xC) == 0) {
					bi >>= 1;
					if (bi == 0) {
						bi = 0x80;
						bitmask++;
					}
					if (*bitmask & bi)
						mask |= 0xC;
				}

				if ((pixels & 0x3) == 0) {
					bi >>= 1;
					if (bi == 0) {
						bi = 0x80;
						bitmask++;
					}
					if (*bitmask & bi)
						mask |= 0x3;
				}
				*buffer++ = mask;
				*buffer++ = pixels;
			}
		}
	}
	return buffer;
}

extern byte sprit_data[RES_SPRIT_MAX];

byte sprit_load_buffer[1290];

byte *loadSprit(byte index) {
	loadSprite(index, sprit_data + 4, sprit_load_buffer, 0);
	return sprit_load_buffer;
}

byte *loadPersSprit(byte index) {
#if 1
	/*Use separate memory for pers1/pers2*/
	if (index < 61)
		loadSprite(index, pers1_data + 4, scratch_mem2, 0);
	else
		loadSprite(index - 61, pers2_data + 4, scratch_mem2, 0);
#else
	/*Use single large chunk for pers1+pers2*/
	loadSprite(index, pers1_data + 4, scratch_mem2, 0);
#endif

	return scratch_mem2;
}


void drawSpriteN(byte index, uint16 x, uint16 y, byte *target) {
	uint16 ofs;
	byte *sprite;
	sprite = loadSprit(index);
	ofs = cga_CalcXY_p(x, y);
	drawSprite(sprite, target, ofs);
}

void drawSpriteNFlip(byte index, uint16 x, uint16 y, byte *target) {
	uint16 ofs;
	byte *sprite;
	sprite = loadSprit(index);
	ofs = cga_CalcXY_p(x, y);
	drawSpriteFlip(sprite, target, ofs);
}

void backupAndShowSprite(byte index, byte x, byte y) {
	byte w, h;
	uint16 ofs;
	byte *sprite = loadSprit(index);
	ofs = cga_CalcXY_p(x, y);
	w = sprite[0];
	h = sprite[1];
	cga_BackupImageReal(ofs, w, h);
	drawSprite(sprite, CGA_SCREENBUFFER, ofs);  /*DrawSpriteN(index, x, y, CGA_SCREENBUFFER);*/
}

/*
Blit progressive image to interlaced screen buffer, then wait for VBlank
Push image from the top to down
NB! width and pixelswidth specify a number of bytes, not count of pixels
*/
void cga_AnimLiftToDown(byte *pixels, uint16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs) {
	uint16 i;
	pixels += pw * (h - 1);
	for (i = 1; i <= h; i++) {
		cga_BlitAndWait(pixels, pw, w, i, screen, ofs);
		pixels -= pw;
	}
}

/*
Blit progressive image to interlaced screen buffer, then wait for VBlank
Pull and expand image from the right to left
NB! width and pixelswidth specify a number of bytes, not count of pixels
NB! ofs specifies top-right corner of the image
*/
void cga_AnimLiftToLeft(uint16 n, byte *pixels, uint16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs) {
	uint16 i;
	for (i = 0; i < n; i++) {
		cga_BlitAndWait(pixels, pw, w + i, h, screen, ofs);
		ofs -= 1;
	}
}

/*
Blit progressive image to interlaced screen buffer, then wait for VBlank
Push image from the left to right
NB! width and pixelswidth specify a number of bytes, not count of pixels
*/
void cga_AnimLiftToRight(uint16 n, byte *pixels, uint16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs) {
	uint16 i;
	for (i = 0; i < n; i++) {
		cga_BlitAndWait(pixels, pw, w + i, h, screen, ofs);
		pixels -= 1;
	}
}

/*
Blit progressive image to interlaced screen buffer, then wait for VBlank
Push image from the down to up
NB! width and pixelswidth specify a number of bytes, not count of pixels
NB! x:y specifies left-bottom coords
*/
void cga_AnimLiftToUp(byte *pixels, uint16 pw, uint16 w, uint16 h, byte *screen, uint16 x, uint16 y) {
	uint16 i;
	for (i = 1; i <= h; i++) {
		cga_BlitAndWait(pixels, pw, w, i, screen, cga_CalcXY_p(x, y));
		y -= 1;
	}
}

/*Copy interlaced screen data n lines down (with clipping)*/
/*Fill gap with source screen data*/
/*offs points to block's bottom most line, data will be shifted to next line*/
/*NB! w is in bytes*/
void cga_HideScreenBlockLiftToDown(uint16 n, byte *screen, byte *source, uint16 w, uint16 h, byte *target, uint16 ofs) {
	while (n--) {
		int16 i;
		uint16 sofs, tofs;

		sofs = ofs;
		tofs = ofs;

		tofs ^= CGA_ODD_LINES_OFS;
		if ((tofs & CGA_ODD_LINES_OFS) == 0)
			tofs += CGA_BYTES_PER_LINE;

		/*shift whole block 1 line*/
		for (i = 0; i < h; i++) {
			/*copy current line to next*/
			memcpy(target + tofs, screen + sofs, w);

			/*next = current*/
			tofs = sofs;

			/*go 1 line up*/
			sofs ^= CGA_ODD_LINES_OFS;
			if ((sofs & CGA_ODD_LINES_OFS) != 0)
				sofs -= CGA_BYTES_PER_LINE;
		}

		/*fill just freed line with new pixels*/
		memcpy(target + tofs, source + tofs, w);

		if (screen == CGA_SCREENBUFFER) {
			cga_blitToScreen(0, 0, 320, 200);
		}

		waitVBlank();

		ofs ^= CGA_ODD_LINES_OFS;
		if ((ofs & CGA_ODD_LINES_OFS) == 0)
			ofs += CGA_BYTES_PER_LINE;
	}
}

/*Copy interlaced screen data n lines up (with clipping)*/
/*Fill gap with source screen data*/
/*offs points to block's top most line, data will be shifted to previous line*/
/*NB! w is in bytes*/
void cga_HideScreenBlockLiftToUp(uint16 n, byte *screen, byte *source, uint16 w, uint16 h, byte *target, uint16 ofs) {
	while (n--) {
		int16 i;
		uint16 sofs, tofs;

		sofs = ofs;
		tofs = ofs;

		tofs ^= CGA_ODD_LINES_OFS;
		if ((tofs & CGA_ODD_LINES_OFS) != 0)
			tofs -= CGA_BYTES_PER_LINE;

		/*shift whole block 1 line*/
		for (i = 0; i < h; i++) {
			/*copy current line to next*/
			memcpy(target + tofs, screen + sofs, w);

			/*next = current*/
			tofs = sofs;

			/*go 1 line down*/
			sofs ^= CGA_ODD_LINES_OFS;
			if ((sofs & CGA_ODD_LINES_OFS) == 0)
				sofs += CGA_BYTES_PER_LINE;
		}

		/*fill just freed line with new pixels*/
		memcpy(target + tofs, source + tofs, w);

		if (screen == CGA_SCREENBUFFER) {
			cga_blitToScreen(0, 0, 320, 200);
		}

		waitVBlank();

		ofs ^= CGA_ODD_LINES_OFS;
		if ((ofs & CGA_ODD_LINES_OFS) != 0)
			ofs -= CGA_BYTES_PER_LINE;
	}
}

/*Copy interlaced screen data n lines left (with clipping)*/
/*Fill gap with source screen data*/
/*offs points to block's left most column, data will be shifted to previous column*/
/*NB! w is in bytes*/
void cga_HideScreenBlockLiftToLeft(uint16 n, byte *screen, byte *source, uint16 w, uint16 h, byte *target, uint16 ofs) {
	while (n--) {
		int16 i;
		uint16 sofs, tofs;

		sofs = ofs;

		/*shift whole block 1 column*/
		for (i = 0; i < h; i++) {
			tofs = sofs - 1;

			/*use memmove since buffers may overlap*/
			memmove(target + tofs, screen + sofs, w);

			/*fill freed column*/
			memcpy(target + tofs + w, source + tofs + w, 1);

			/*go 1 line down*/
			sofs ^= CGA_ODD_LINES_OFS;
			if ((sofs & CGA_ODD_LINES_OFS) == 0)
				sofs += CGA_BYTES_PER_LINE;
		}

		if (screen == CGA_SCREENBUFFER) {
			cga_blitToScreen(0, 0, 320, 200);
		}

		waitVBlank();

		ofs--;
	}
}

/*Copy interlaced screen data n lines right (with clipping)*/
/*Fill gap with source screen data*/
/*offs points to block's right most column, data will be shifted to next column*/
/*NB! w is in bytes*/
void cga_HideScreenBlockLiftToRight(uint16 n, byte *screen, byte *source, uint16 w, uint16 h, byte *target, uint16 ofs) {
	while (n--) {
		int16 i;
		uint16 sofs, tofs;

		sofs = ofs;

		/*shift whole block 1 column*/
		for (i = 0; i < h; i++) {
			tofs = sofs + 1;

			/*use memmove since buffers may overlap*/
			memmove(target + tofs - w, screen + sofs - w, w);

			/*fill freed column*/
			memcpy(target + tofs - w, source + tofs - w, 1);

			/*go 1 line down*/
			sofs ^= CGA_ODD_LINES_OFS;
			if ((sofs & CGA_ODD_LINES_OFS) == 0)
				sofs += CGA_BYTES_PER_LINE;
		}

		if (screen == CGA_SCREENBUFFER) {
			cga_blitToScreen(0, 0, 320, 200);
		}

		waitVBlank();

		ofs++;
	}
}


typedef struct scrpiece_t {
	uint16    offs;
	byte   delay;  /*speed in bits 1..0, delay in bits 7..2*/
	byte   pix0;
	byte   pix2;
	byte   pix1;
	byte   pix3;
} scrpiece_t;

static const byte piecedelays[] = {
	219, 182, 237, 187, 110, 219, 187, 120, 219, 109, 182,
	219, 109, 182, 219, 104, 182, 214, 218, 219,  91, 107,
	107, 104, 213, 107,  90, 214, 181, 182, 214, 216, 213,
	86, 214, 173,  91,  91,  85,  88, 170, 173,  85, 170, 181,
	85, 182, 168, 170, 170, 171,  85,  85,  90, 170, 168, 170,
	170, 170, 170, 170, 170, 170, 168, 213,  82, 170,  85,
	74, 170,  73,  88, 170, 169,  41,  82, 164, 164, 170, 168,
	170, 148, 165,  41,  74,  73,  41,  40, 201,  41,  37,  36,
	164, 148, 148, 152, 164, 146,  73,  36, 146,  73,  36, 152,
	164,  73,  18,  68, 145,  36,  68, 136, 145,  34,  36,  68,
	136, 145,  18,  40, 145,  16, 136, 136,  68,  68,  66,  24,
	136,  66,  16, 132,  33,   8,  66,  24, 132,  16,  65,   8,  32,
	132,  16,  72, 130,   4,   8,   8,  16,  32,  32,  72, 128, 128,
	64,  32,   8,   2,   1,   8, 128,   8,   0,  32,   0, 128,   8, 8, 128,
	0,   4,   0,   0,   4,   0,   8, 128,   0,   0,   0,   0,   0, 0,   8
};

/*break screen area onto 4x4 pix pieces*/
static void screenToPieces(byte width, byte height, byte *screen, uint16 offs, scrpiece_t *pieces) {
	const byte *delays = piecedelays;
	height = (height + 3) / 4;
	while (height--) {
		uint16 x;
		for (x = 0; x < width; x++) {
			uint16 bofs = offs + x;
			pieces->offs = bofs;
			pieces->delay = *delays++;
			if (pieces->delay == 0) /*ensure piece is alive*/
				pieces->delay = 1;
			pieces->pix0 = screen[bofs];
			pieces->pix2 = screen[bofs + CGA_BYTES_PER_LINE];

			bofs ^= CGA_ODD_LINES_OFS;
			if ((bofs & CGA_ODD_LINES_OFS) == 0)
				bofs += CGA_BYTES_PER_LINE;
			pieces->pix1 = screen[bofs];
			pieces->pix3 = screen[bofs + CGA_BYTES_PER_LINE];
			pieces++;

			if (delays >= piecedelays + ARRAYSIZE(piecedelays))
				delays = piecedelays;
		}
		offs += CGA_BYTES_PER_LINE * 2; /*4 lines down*/
	}
	pieces->offs = 0;   /*end of list*/
}

static void fallPieces(scrpiece_t *pieces, byte *source, byte *target) {
	byte t = 1;
	byte again = 0;
	do {
		scrpiece_t *piece;
		for (piece = pieces, again = 0; piece->offs; piece++) {
			if ((piece->delay >> 2) < t) {
				uint16 offs = piece->offs;
				uint16 bofs = offs;
				if (target[bofs] == piece->pix0)
					target[bofs] = source[bofs];
				if (target[bofs + CGA_BYTES_PER_LINE] == piece->pix2)
					target[bofs + CGA_BYTES_PER_LINE] = source[bofs + CGA_BYTES_PER_LINE];

				bofs ^= CGA_ODD_LINES_OFS;
				if ((bofs & CGA_ODD_LINES_OFS) == 0)
					bofs += CGA_BYTES_PER_LINE;

				if (target[bofs] == piece->pix1)
					target[bofs] = source[bofs];
				if (target[bofs + CGA_BYTES_PER_LINE] == piece->pix3)
					target[bofs + CGA_BYTES_PER_LINE] = source[bofs + CGA_BYTES_PER_LINE];

				/*dead?*/
				if (piece->delay == 0)
					continue;

				/*fall n lines*/
				switch ((piece->delay & 3) + 1) {
				case 1:
					offs ^= CGA_ODD_LINES_OFS;
					if ((offs & CGA_ODD_LINES_OFS) == 0)
						offs += CGA_BYTES_PER_LINE;
					break;
				case 2:
					offs += CGA_BYTES_PER_LINE;
					break;
				case 3:
					offs += CGA_BYTES_PER_LINE;
					offs ^= CGA_ODD_LINES_OFS;
					if ((offs & CGA_ODD_LINES_OFS) == 0)
						offs += CGA_BYTES_PER_LINE;
					break;
				case 4:
					offs += CGA_BYTES_PER_LINE;
					offs += CGA_BYTES_PER_LINE;
					break;
				}

				/*extra 2 lines*/
				offs += CGA_BYTES_PER_LINE;

				piece->offs = offs;

				/*past line 190?*/
				if ((offs | CGA_ODD_LINES_OFS) >= ((190 / 2 * CGA_BYTES_PER_LINE) | CGA_ODD_LINES_OFS)) {
					piece->delay = 0;
					continue;
				}

				bofs = offs;

				target[bofs] = piece->pix0;
				target[bofs + CGA_BYTES_PER_LINE] = piece->pix2;

				bofs ^= CGA_ODD_LINES_OFS;
				if ((bofs & CGA_ODD_LINES_OFS) == 0)
					bofs += CGA_BYTES_PER_LINE;

				target[bofs] = piece->pix1;
				target[bofs + CGA_BYTES_PER_LINE] = piece->pix3;
			}
			again = 1;
		}

		if (target == CGA_SCREENBUFFER) {
			cga_blitToScreen(0, 0, 320, 200);
		}

		waitVBlank();
		t++;
	} while (again);
}

void cga_HideShatterFall(byte *screen, byte *source, uint16 w, uint16 h, byte *target, uint16 ofs) {
	scrpiece_t *pieces = (scrpiece_t *)scratch_mem2;
	screenToPieces(w, h, screen, ofs, pieces);
	fallPieces(pieces, source, target);
}

void cga_TraceLine(uint16 sx, uint16 ex, uint16 sy, uint16 ey, byte *source, byte *target) {
	byte b0 = 0;
	byte b1 = 0;
	byte mask;
	uint16 ofs;
	uint16 count;
	int16 w, h, dx, dy, val;

	if (ex >= sx)
		w = ex - sx;
	else {
		w = sx - ex - 1;
		b0 = 1;
	}
	dx = -(w * 2) - 1;


	if (ey >= sy)
		h = ey - sy;
	else {
		h = sy - ey - 1;
		b1 = 1;
	}
	dy = h * 2;

	ofs = cga_CalcXY_p(sx / 4, sy);
	mask = 0xC0 >> ((sx % 4) * 2);

	val = dy + dx;

	target[ofs] = (target[ofs] & ~mask) | (source[ofs] & mask);

	count = w + h;

	while (count--) {
		if (val > 0) {
			if (b1 == 0) {
				ofs ^= CGA_ODD_LINES_OFS;
				if ((ofs & CGA_ODD_LINES_OFS) == 0)
					ofs += CGA_BYTES_PER_LINE;
			} else {
				ofs ^= CGA_ODD_LINES_OFS;
				if ((ofs & CGA_ODD_LINES_OFS) != 0)
					ofs -= CGA_BYTES_PER_LINE;
			}
			val += dx;
		} else {
			if (b0 == 0) {
				mask >>= 2;
				if (mask == 0) {
					mask = 0xC0;
					ofs += 1;
				}
			} else {
				mask <<= 2;
				if (mask == 0) {
					mask = 0x03;
					ofs -= 1;
				}
			}
			val += dy;
			if (val > 0)
				continue;
		}
		target[ofs] = (target[ofs] & ~mask) | (source[ofs] & mask);
	}

	if (target == CGA_SCREENBUFFER)
		cga_blitToScreen(0, 0, 320, 200);
}

/*TODO: get rid of this structure and pass everything relevant as arguments?*/
typedef struct zoom_t {
	byte ybase;
	byte xbase;    /*IN: original x shift in pixels*/
	byte xval_l;
	byte xval_h;
	byte xstep_l;
	byte xstep_h;
	byte yval_l;
	byte yval_h;
	byte ystep_l;
	byte ystep_h;
	byte ew;   /*IN: original width/height in pixels*/
	byte eh;
	byte scale_x;
	byte scale_y;
	byte oh;   /*IN: original width/height in bytes*/
	byte ow;
	byte *pixels;  /*IN: image pixels*/
	byte fw;
} zoom_t;

/*
Divide two normal values and return 8.8 quotient
*/
static uint16 FPDiv(uint16 a, byte b) {
	byte hi = a / b;
	byte lo = ((a % b) << 8) / b;
	return (hi << 8) + lo;
	/*is it really any better than (uint32)(a << 8) / b ?*/
}

// call commented out in void cga_ZoomImage(byte *pixels, byte w, byte h, byte nw, byte nh, byte *target, uint16 ofs) currently
// thus causes an unused function warning
#if 0
/*
Draw scaled image
NB! tw/th specify target width/height in pixels
*/
static void cga_Zoom(zoom_t *params, byte tw, byte th, byte *source, byte *target, uint16 ofs) {
	byte x, y;
	uint16 finofs = ofs;
	byte *temp = scratch_mem2;

	/*calc old/new ratio*/
	params->scale_x = tw + 1;
	params->xstep_l = params->ew / params->scale_x;
	params->xstep_h = (params->ew % params->scale_x) * 256 / params->scale_x;

	params->scale_y = th + 1;
	params->ystep_l = params->eh / params->scale_y;
	params->ystep_h = (params->eh % params->scale_y) * 256 / params->scale_y;

	params->yval_l = 0;
	params->yval_h = 0;

	for (y = params->scale_y;;) {
		uint16 oofs = ofs;
		byte *pixels = params->pixels + params->yval_l * params->ow;
		byte sc = 4 - params->xbase;
		/*left partial pixel*/
		byte pix = source[ofs] >> (sc * 2);

		params->xval_l = 0;
		params->xval_h = 0;
		params->fw = 0;

		for (x = params->scale_x;;) {
			byte p = pixels[params->xval_l / 4] << ((params->xval_l % 4) * 2);
			pix = (pix << 2) | (p >> 6);
			if (--sc == 0) {
				/*inner full pixel*/
				*temp++ = pix;
				ofs++;
				params->fw++;
				sc = 4;
			}
			params->xval_l += params->xstep_l + ((params->xval_h + params->xstep_h) >> 8);
			params->xval_h += params->xstep_h;

			if (x == 0)
				break;
			if (--x == 0)
				params->xval_l = params->ew;
		}

		/*right partial pixel*/
		*temp++ = (source[ofs] & ~(0xFF << (sc * 2))) | (pix << (sc * 2));
		ofs++;
		params->fw++;

		/*ofs -= params->fw;*/
		ofs = oofs;

		ofs ^= CGA_ODD_LINES_OFS;
		if ((ofs & CGA_ODD_LINES_OFS) == 0)
			ofs += CGA_BYTES_PER_LINE;

		params->yval_l += params->ystep_l + ((params->yval_h + params->ystep_h) >> 8);
		params->yval_h += params->ystep_h;

		if (y == 0)
			break;
		if (--y == 0)
			params->yval_l = params->eh;
	}

	cga_BlitAndWait(scratch_mem2, params->fw, params->fw, th + 2, target, finofs);
}
#endif

/*
Draw scaled image
NB! tw/th specify target width/height in pixels
This is slightly simplified version, but should work identical to the code above
*/
static void cga_ZoomOpt(zoom_t *params, byte tw, byte th, byte *source, byte *target, uint16 ofs) {
	byte x, y;
	uint16 finofs = ofs;
	byte *temp = scratch_mem2;

	/*calc old/new ratio*/
	uint16 target_w = tw + 1;
	uint16 xstep = (params->ew << 8) / target_w;    /*fixed-point 8.8 value*/
	uint16 target_h = th + 1;
	uint16 ystep = (params->eh << 8) / target_h;    /*fixed-point 8.8 value*/

	uint16 yval = 0;

	for (y = target_h;;) {
		byte *pixels = params->pixels + (yval >> 8) * params->ow;
		byte sc = 4 - params->xbase;
		/*left partial pixel*/
		byte pix = source[ofs] >> (sc * 2);

		uint16 xval = 0;
		params->fw = 0;

		for (x = target_w;;) {
			byte p = pixels[(xval >> 8) / 4] << (((xval >> 8) % 4) * 2);
			pix = (pix << 2) | (p >> 6);
			if (--sc == 0) {
				/*inner full pixel*/
				*temp++ = pix;
				ofs++;
				params->fw++;
				sc = 4;
			}
			xval += xstep;

			if (x == 0)
				break;
			if (--x == 0)
				xval = params->ew << 8;
		}

		/*right partial pixel*/
		*temp++ = (source[ofs] & ~(0xFF << (sc * 2))) | (pix << (sc * 2));
		ofs++;
		params->fw++;

		ofs -= params->fw;

		ofs ^= CGA_ODD_LINES_OFS;
		if ((ofs & CGA_ODD_LINES_OFS) == 0)
			ofs += CGA_BYTES_PER_LINE;

		yval += ystep;

		if (y == 0)
			break;
		if (--y == 0)
			yval = params->eh << 8;
	}

	cga_BlitAndWait(scratch_mem2, params->fw, params->fw, th + 2, target, finofs);
}

/*
Draw image zoomed from w:h to nw:nx to target at specified ofs
Use backbuffer pixels to fill sides
NB! w/nw are the number of bytes, not pixels
*/
void cga_ZoomImage(byte *pixels, byte w, byte h, byte nw, byte nh, byte *target, uint16 ofs) {
	zoom_t zoom;

	zoom.pixels = pixels;
	zoom.ow = w;
	zoom.oh = h;
	zoom.ew = (w * 4) - 1;
	zoom.eh = h - 1;

	if (g_vm->getLanguage() == Common::EN_USA)
		zoom.xbase = 0;
	else
		zoom.xbase = ((w * 4 / 2) - 1) % 4;

	/*TODO: why this nw/nh order? maybe bug*/
#if 0
	cga_Zoom(&zoom, nh - 2, nw * 4 - 2, backbuffer, target, ofs);
#else
	cga_ZoomOpt(&zoom, nh - 2, nw * 4 - 2, backbuffer, target, ofs);
#endif
}

/*
Animate image zooming-in from origin ofs to final size w:h in specified number of steps
Use backbuffer pixels to fill sides
Ofs specifies zoom origin
NB! w is the number of bytes, not pixels
*/
void cga_AnimZoomOpt(zoom_t *zoom, uint16 w, uint16 h, byte steps, byte *target, uint16 ofs) {
	uint16 xstep = FPDiv(w, steps); /*fixed-point 8.8 value*/
	uint16 ystep = FPDiv(h, steps); /*fixed-point 8.8 value*/

	uint16 xval = 0x200;
	uint16 yval = 0x200;

	for (steps = steps / 2 - 2; steps; steps--) {
		uint16 prev;

		cga_ZoomOpt(zoom, xval >> 8, yval >> 8, backbuffer, target, ofs);

		prev = yval;
		yval += ystep;
		if (((prev ^ yval) & 0xFF00) || ((yval & 0x100) == 0)) {
			ofs ^= CGA_ODD_LINES_OFS;
			if ((ofs & CGA_ODD_LINES_OFS) != 0)
				ofs -= CGA_BYTES_PER_LINE;
		}

		prev = xval;
		xval += xstep;
		if (((prev ^ xval) & 0xFF00) || ((xval & 0x100) == 0)) {
			if (zoom->xbase-- == 0) {
				ofs -= 1;
				zoom->xbase = 3;
			}
		}
	}
}


/*
Animate image zooming-in from its center to final size w:h to target at specified ofs
Use backbuffer pixels to fill sides
NB! w is the number of bytes, not pixels
NB! ofs is the final image top left corner, not the zoom origin
*/
void cga_AnimZoomIn(byte *pixels, byte w, byte h, byte *target, uint16 ofs) {
	uint16 finofs = ofs;
	byte x, y, maxside;

	zoom_t zoom;
	zoom.pixels = pixels;
	zoom.ow = w;
	zoom.oh = h;
	zoom.ew = (w * 4) - 1;
	zoom.eh = h - 1;
	zoom.xbase = ((w * 4 / 2) - 1) % 4;

	/*set zoom origin*/
	x = (w * 4 / 2) - 1;
	y = (h / 2) - 1;
	ofs += x / 4;
	if (y & 1) {
		ofs ^= CGA_ODD_LINES_OFS;
		if ((ofs & CGA_ODD_LINES_OFS) == 0)
			ofs += CGA_BYTES_PER_LINE;
	}
	ofs += (y / 2) * CGA_BYTES_PER_LINE;

	maxside = w * 4;
	if (maxside < h)
		maxside = h;

	cga_AnimZoomOpt(&zoom, w * 4 * 2, h * 2, maxside, target, ofs);

	cga_BlitAndWait(pixels, w, w, h, target, finofs);
}

/*
Draw scaled image
NB! tw/th specify target width/height in pixels
*/
void cga_ZoomInplace(zoom_t *params, byte tw, byte th, byte *source, byte *target, uint16 ofs) {
	byte x, y;

	/*calc old/new ratio*/
	params->scale_x = tw + 1;
	params->xstep_l = (uint16)params->ew / params->scale_x;
	params->xstep_h = (uint16)(((uint16)params->ew % params->scale_x) << 8) / params->scale_x;

	params->scale_y = th + 1;
	params->ystep_l = (uint16)params->eh / params->scale_y;
	params->ystep_h = (uint16)(((uint16)params->eh % params->scale_y) << 8) / params->scale_y;

	params->yval_l = 0;
	params->yval_h = 0;

	for (y = params->scale_y;;) {
		uint16 oofs = ofs;
		byte *pixels = params->pixels + params->yval_l * params->ow;
		byte sc = 4 - params->xbase;
		/*left partial pixel*/
		byte pix = source[ofs] >> (sc * 2);

		params->xval_l = 0;
		params->xval_h = 0;
		params->fw = 0;

		for (x = params->scale_x;;) {
			byte p = pixels[params->xval_l / 4] << ((params->xval_l % 4) * 2);
			pix = (pix << 2) | (p >> 6);
			if (--sc == 0) {
				/*inner full pixel*/
				target[ofs] = pix;
				ofs++;
				params->fw++;
				sc = 4;
			}
			params->xval_l += params->xstep_l + ((params->xval_h + params->xstep_h) >> 8);
			params->xval_h += params->xstep_h;

			if (x == 0)
				break;
			if (--x == 0)
				params->xval_l = params->ew;
		}

		/*right partial pixel*/
		target[ofs] = (source[ofs] & ~(0xFF << (sc * 2))) | (pix << (sc * 2));
		ofs++;
		params->fw++;

		/*ofs -= params->fw;*/
		ofs = oofs;

		ofs ^= CGA_ODD_LINES_OFS;
		if ((ofs & CGA_ODD_LINES_OFS) == 0)
			ofs += CGA_BYTES_PER_LINE;

		params->yval_l += params->ystep_l + ((params->yval_h + params->ystep_h) >> 8);
		params->yval_h += params->ystep_h;

		if (y == 0)
			break;
		if (--y == 0)
			params->yval_l = params->eh;
	}
}

void cga_ZoomInplaceXY(byte *pixels, byte w, byte h, byte nw, byte nh, uint16 x, uint16 y, byte *target) {
	zoom_t zoom;

	zoom.pixels = pixels;
	zoom.ow = w;
	zoom.oh = h;
	zoom.ew = (w * 4) - 1;
	zoom.eh = h - 1;
	zoom.xbase = x % 4;

	cga_ZoomInplace(&zoom, nw, nh, target, target, cga_CalcXY(x, y));
}

} // End of namespace Chamber
