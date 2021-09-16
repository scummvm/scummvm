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

#include "chamber/chamber.h"
#include "chamber/common.h"
#include "chamber/resdata.h"
#include "chamber/cga.h"

namespace Chamber {

extern unsigned char backbuffer[0x4000];
byte CGA_SCREENBUFFER[0x4000];
byte scrbuffer[320*200];

unsigned char carpc_data[RES_CARPC_MAX];

extern unsigned char *scratch_mem2;

unsigned char char_draw_coords_x;
unsigned char char_draw_coords_y;
unsigned char *char_xlat_table;
unsigned char string_ended;
unsigned char char_draw_max_width;
unsigned char char_draw_max_height;

/*pixels order reverse in a byte*/
unsigned char cga_pixel_flip[256] = {
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

/*
  Switch to CGA 320x200x2bpp mode
*/
void SwitchToGraphicsMode(void) {
	g_system->getPaletteManager()->setPalette(PALETTE_CGA, 0, 4);
}

/*
  Switch to text mode
*/
void SwitchToTextMode(void) {
	warning("STUB: SwitchToTextMode()");
}

void WaitVBlank(void) {
}

void CGA_ColorSelect(unsigned char csel) {
	warning("STUB: CGA_ColorSelect(%d)", csel);
	//outportb(0x3D9, csel);
}

void CGA_BackBufferToRealFull(void) {
	memcpy(CGA_SCREENBUFFER, backbuffer, sizeof(backbuffer));

	byte *dst = scrbuffer;

	for (int y = 0; y < 200; y++) {
		byte *ptr = CGA_SCREENBUFFER + (y >> 1) * 80;
		if (y & 1)
			ptr += 0x2000;

		for (int x = 0; x < 320 / 4; x++) {
			byte colors = *ptr++;

			for (int c = 0; c < 4; c++) {
				byte color = (colors & 0xC0) >> 6;
				colors <<= 2;

				*dst++ = color;
			}
		}
	}

	g_system->copyRectToScreen(scrbuffer, 320, 0, 0, 320, 200);
	g_system->updateScreen();
}

void CGA_RealBufferToBackFull(void) {
	memcpy(backbuffer, CGA_SCREENBUFFER, sizeof(backbuffer));
}

/*Copy interlaced screen data to another screen*/
/*NB! w is in bytes*/
void CGA_CopyScreenBlock(unsigned char *source, unsigned int w, unsigned int h, unsigned char *target, unsigned int ofs) {
	while (h--) {
		memcpy(target + ofs, source + ofs, w);
		ofs ^= CGA_ODD_LINES_OFS;
		if ((ofs & CGA_ODD_LINES_OFS) == 0)
			ofs += CGA_BYTES_PER_LINE;
	}
}

/*
Flip screen and backbuffer
*/
void CGA_SwapRealBackBuffer(void) {
	unsigned int i;
	unsigned short *s, *d;
	WaitVBlank();
	s = (unsigned short *)CGA_SCREENBUFFER;
	d = (unsigned short *)backbuffer;
	for (i = 0; i < sizeof(backbuffer) / 2; i++) {
		unsigned short t = *s;
		*s++ = *d;
		*d++ = t;
	}
}


/*
Copy current screen's pixels to scratch mem, put new pixels to screen
*/
void CGA_SwapScreenRect(unsigned char *pixels, unsigned int w, unsigned int h, unsigned char *screen, unsigned int ofs) {
	unsigned char *old = scratch_mem2;
	while (h--) {
		unsigned int i;
		for (i = 0; i < w; i++) {
			*old++ = screen[ofs + i];
			screen[ofs + i] = *pixels++;
		}
		ofs ^= CGA_ODD_LINES_OFS;
		if ((ofs & CGA_ODD_LINES_OFS) == 0)
			ofs += CGA_BYTES_PER_LINE;
	}
}

/*
Calc screen offset from normal pixel coordinates
Out:
  screen offset
*/
unsigned int CGA_CalcXY(unsigned int x, unsigned int y) {
	return CGA_CalcXY_p(x / 4, y);
}

/*
Calc screen offset from packed pixel coordinates
Out:
  screen offset
*/
unsigned int CGA_CalcXY_p(unsigned int x, unsigned int y) {
	unsigned int ofs = 0;
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
unsigned char *CGA_BackupImage(unsigned char *screen, unsigned int ofs, unsigned int w, unsigned int h, unsigned char *buffer) {
	*(unsigned char *)(buffer + 0) = h;
	*(unsigned char *)(buffer + 1) = w;
	*(unsigned int *)(buffer + 2) = ofs;
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

unsigned char *CGA_BackupImageReal(unsigned int ofs, unsigned int w, unsigned int h) {
	warning("STUB: CGA_BackupImageReal()");

	return NULL;
	//return CGA_BackupImage(MK_FP(CGA_BASE_SEG, 0), ofs, w, h, scratch_mem2);
}

/*
Blit progressive image to interlaced screen buffer
NB! width and pixelswidth specify a number of bytes, not count of pixels
*/
void CGA_Blit(unsigned char *pixels, unsigned int pw, unsigned int w, unsigned int h, unsigned char *screen, unsigned int ofs) {
	while (h--) {
		memcpy(screen + ofs, pixels, w);
		pixels += pw;
		ofs ^= CGA_ODD_LINES_OFS;
		if ((ofs & CGA_ODD_LINES_OFS) == 0)
			ofs += CGA_BYTES_PER_LINE;
	}
}

/*
Blit progressive image to interlaced screen buffer, then wait for VBlank
NB! width and pixelswidth specify a number of bytes, not count of pixels
*/
void CGA_BlitAndWait(unsigned char *pixels, unsigned int pw, unsigned int w, unsigned int h, unsigned char *screen, unsigned int ofs) {
	CGA_Blit(pixels, pw, w, h, screen, ofs);
	WaitVBlank();
}

void CGA_Fill(unsigned char pixel, unsigned int w, unsigned int h, unsigned char *screen, unsigned int ofs) {
	while (h--) {
		memset(screen + ofs, pixel, w);
		ofs ^= CGA_ODD_LINES_OFS;
		if ((ofs & CGA_ODD_LINES_OFS) == 0)
			ofs += CGA_BYTES_PER_LINE;
	}
}

void CGA_FillAndWait(unsigned char pixel, unsigned int w, unsigned int h, unsigned char *screen, unsigned int ofs) {
	CGA_Fill(pixel, w, h, screen, ofs);
	WaitVBlank();
}

/*
Restore saved image to target screen buffer
*/
void CGA_RestoreImage(unsigned char *buffer, unsigned char *target) {
	unsigned int w, h;
	unsigned int ofs;

	if (!buffer)
		return;

	h = *(unsigned char *)(buffer + 0);
	w = *(unsigned char *)(buffer + 1);
	ofs = *(unsigned int *)(buffer + 2);
	buffer += 4;    /*TODO: fix me for large int*/

	CGA_Blit(buffer, w, w, h, target, ofs);
}

/*
Restore saved image from scratch mem to target screen buffer
*/
void CGA_RestoreBackupImage(unsigned char *target) {
	CGA_RestoreImage(scratch_mem2, target);
}

/*
Copy image's real screen data to backbuffer
*/
void CGA_RefreshImageData(unsigned char *buffer) {
	unsigned int w, h;
	unsigned int ofs;

	if (!buffer)
		return;

	h = *(unsigned char *)(buffer + 0);
	w = *(unsigned char *)(buffer + 1);
	ofs = *(unsigned int *)(buffer + 2);

	CGA_CopyScreenBlock(CGA_SCREENBUFFER, w, h, backbuffer, ofs);
}

/*
Draw a vertical line with origin x:y and length l, using color
NB! Line must not wrap around the edge
*/
void CGA_DrawVLine(unsigned int x, unsigned int y, unsigned int l, unsigned char color, unsigned char *target) {
	unsigned int ofs;
	/*pixels are starting from top bits of byte*/
	unsigned int mask = ~(3 << ((CGA_PIXELS_PER_BYTE - 1) * CGA_BITS_PER_PIXEL));
	unsigned char pixel = color << ((CGA_PIXELS_PER_BYTE - 1) * CGA_BITS_PER_PIXEL);

	mask >>= (x % CGA_PIXELS_PER_BYTE) * CGA_BITS_PER_PIXEL;
	pixel >>= (x % CGA_PIXELS_PER_BYTE) * CGA_BITS_PER_PIXEL;

	ofs = CGA_CalcXY_p(x / CGA_PIXELS_PER_BYTE, y);

	while (l--) {
		target[ofs] = (target[ofs] & mask) | pixel;
		ofs ^= CGA_ODD_LINES_OFS;
		if ((ofs & CGA_ODD_LINES_OFS) == 0)
			ofs += CGA_BYTES_PER_LINE;
	}
}

/*
Draw a horizontal line with origin x:y and length l, using color
NB! Line must not wrap around the edge
*/
void CGA_DrawHLine(unsigned int x, unsigned int y, unsigned int l, unsigned char color, unsigned char *target) {
	unsigned int ofs;
	/*pixels are starting from top bits of byte*/
	unsigned int mask = ~(3 << ((CGA_PIXELS_PER_BYTE - 1) * CGA_BITS_PER_PIXEL));
	unsigned char pixel = color << ((CGA_PIXELS_PER_BYTE - 1) * CGA_BITS_PER_PIXEL);

	mask >>= (x % CGA_PIXELS_PER_BYTE) * CGA_BITS_PER_PIXEL;
	pixel >>= (x % CGA_PIXELS_PER_BYTE) * CGA_BITS_PER_PIXEL;

	ofs = CGA_CalcXY_p(x / CGA_PIXELS_PER_BYTE, y);
	while (l--) {
		target[ofs] = (target[ofs] & mask) | pixel;
		mask >>= CGA_BITS_PER_PIXEL;
		pixel >>= CGA_BITS_PER_PIXEL;
		if (mask == 0xFF) {
			ofs++;
			mask = ~(3 << ((CGA_PIXELS_PER_BYTE - 1) * CGA_BITS_PER_PIXEL));
			pixel = color << ((CGA_PIXELS_PER_BYTE - 1) * CGA_BITS_PER_PIXEL);
		}
	}
}

/*
Draw horizontal line of length l with color, add surrounding pixels (bmask, bpix, left in high byte, right in low)
Return next line screen offset
NB! Length specifies byte lenght of inner segment, not amount of pixels
 */
unsigned int CGA_DrawHLineWithEnds(unsigned int bmask, unsigned int bpix, unsigned char color, unsigned int l, unsigned char *target, unsigned int ofs) {
	target[ofs] = (target[ofs] & (bmask >> 8)) | (bpix >> 8);
	memset(target + ofs + 1, color, l);
	target[ofs + 1 + l] = (target[ofs + 1 + l] & (bmask & 255)) | (bpix & 255);
	ofs ^= CGA_ODD_LINES_OFS;
	if ((ofs & CGA_ODD_LINES_OFS) == 0)
		ofs += CGA_BYTES_PER_LINE;
	return ofs;
}

/*
Print a character at current cursor pos, then advance
*/
void CGA_PrintChar(unsigned char c, unsigned char *target) {
	unsigned int i;
	unsigned char *font = carpc_data + c * CGA_FONT_HEIGHT;
	unsigned int ofs = CGA_CalcXY_p(char_draw_coords_x++, char_draw_coords_y);
	for (i = 0; i < CGA_FONT_HEIGHT; i++) {
		c = *font++;
		c = char_xlat_table[c];
		target[ofs] = c;
		ofs ^= CGA_ODD_LINES_OFS;
		if ((ofs & CGA_ODD_LINES_OFS) == 0)
			ofs += CGA_BYTES_PER_LINE;
	}
}


/*
Blit progressive sprite (mask+pixel) from scratch buffer to interlaced screen buffer, using backbuffer pixels for transparency
NB! width specify a number of bytes, not count of pixels
TODO: generalize/merge me with BlitSprite
*/
void CGA_BlitScratchBackSprite(unsigned int sprofs, unsigned int w, unsigned int h, unsigned char *screen, unsigned int ofs) {
	unsigned char x;
	unsigned char *pixels = scratch_mem2 + 2 + sprofs;
	while (h--) {
		for (x = 0; x < w; x++)
			screen[ofs + x] = (backbuffer[ofs + x] & pixels[x * 2]) | pixels[x * 2 + 1];
		pixels += w * 2;
		ofs ^= CGA_ODD_LINES_OFS;
		if ((ofs & CGA_ODD_LINES_OFS) == 0)
			ofs += CGA_BYTES_PER_LINE;
	}
}

void CGA_BlitFromBackBuffer(unsigned char w, unsigned char h, unsigned char *screen, unsigned int ofs) {
	CGA_CopyScreenBlock(backbuffer, w, h, screen, ofs);
}

/*
Blit progressive sprite (mask+pixel) to interlaced screen buffer
NB! width and pixelswidth specify a number of bytes, not count of pixels
*/
void CGA_BlitSprite(unsigned char *pixels, signed int pw, unsigned int w, unsigned int h, unsigned char *screen, unsigned int ofs) {
	unsigned char x;
	while (h--) {
		for (x = 0; x < w; x++)
			screen[ofs + x] = (screen[ofs + x] & pixels[x * 2]) | pixels[x * 2 + 1];
		pixels += pw;
		ofs ^= CGA_ODD_LINES_OFS;
		if ((ofs & CGA_ODD_LINES_OFS) == 0)
			ofs += CGA_BYTES_PER_LINE;
	}
}

/*
Blit progressive sprite (mask+pixel) to interlaced screen buffer. Flip the sprite horizontally
NB! width and pixelswidth specify a number of bytes, not count of pixels
*/
void CGA_BlitSpriteFlip(unsigned char *pixels, signed int pw, unsigned int w, unsigned int h, unsigned char *screen, unsigned int ofs) {
	unsigned char x;
	while (h--) {
		for (x = 0; x < w; x++)
			screen[ofs - x] = (screen[ofs - x] & cga_pixel_flip[pixels[x * 2]]) | cga_pixel_flip[pixels[x * 2 + 1]];
		pixels += pw;
		ofs ^= CGA_ODD_LINES_OFS;
		if ((ofs & CGA_ODD_LINES_OFS) == 0)
			ofs += CGA_BYTES_PER_LINE;
	}
}

/*
Blit progressive sprite (pixel+mask) to interlaced screen buffer
Backup original screen data to buffer
Apply extra mask to source pixels when drawing
Used to draw mouse cursor and backup what's under it
NB! width and pixelswidth specify a number of bytes, not count of pixels
NB! pixel+mask comes in reversed order, compared to regular BlitSprite
*/
void CGA_BlitSpriteBak(unsigned char *pixels, signed int pw, unsigned int w, unsigned int h, unsigned char *screen, unsigned int ofs, unsigned char *backup, unsigned char mask) {
	unsigned char x;
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
}


/*
Blit progressive sprite (w+h+mask+pixel) to interlaced screen buffer
*/
void DrawSprite(unsigned char *sprite, unsigned char *screen, unsigned int ofs) {
	unsigned char w, h;
	w = *sprite++;
	h = *sprite++;
	CGA_BlitSprite(sprite, w * 2, w, h, screen, ofs);
}

/*
Blit progressive sprite (w+h+mask+pixel) to interlaced screen buffer, horizontally flipped
*/
void DrawSpriteFlip(unsigned char *sprite, unsigned char *screen, unsigned int ofs) {
	unsigned char w, h;
	w = *sprite++;
	h = *sprite++;
	CGA_BlitSpriteFlip(sprite, w * 2, w, h, screen, ofs);
}

/*
Load and uncompress 2-bit sprite
Return next ptr after the loaded sprite
*/
unsigned char *LoadSprite(unsigned char index, unsigned char *bank, unsigned char *buffer, unsigned char header_only) {
	unsigned char w, h;
	unsigned int rsize;
	unsigned char *sprite, *sprite_end;
	unsigned char *bitmask;
	sprite = SeekToEntryW(bank, index, &sprite_end);
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
			unsigned char bi = 1;
			while (rsize--) {
				unsigned char pixels = *sprite++;
				unsigned char mask = 0;

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

extern unsigned char sprit_data[RES_SPRIT_MAX];

unsigned char sprit_load_buffer[1290];

unsigned char *LoadSprit(unsigned char index) {
	LoadSprite(index, sprit_data + 4, sprit_load_buffer, 0);
	return sprit_load_buffer;
}

unsigned char *LoadPersSprit(unsigned char index) {
#if 1
	/*Use separate memory for pers1/pers2*/
	if (index < 61)
		LoadSprite(index, pers1_data + 4, scratch_mem2, 0);
	else
		LoadSprite(index - 61, pers2_data + 4, scratch_mem2, 0);
#else
	/*Use single large chunk for pers1+pers2*/
	LoadSprite(index, pers1_data + 4, scratch_mem2, 0);
#endif

	return scratch_mem2;
}


void DrawSpriteN(unsigned char index, unsigned int x, unsigned int y, unsigned char *target) {
	unsigned int ofs;
	unsigned char *sprite;
	sprite = LoadSprit(index);
	ofs = CGA_CalcXY_p(x, y);
	DrawSprite(sprite, target, ofs);
}

void DrawSpriteNFlip(unsigned char index, unsigned int x, unsigned int y, unsigned char *target) {
	unsigned int ofs;
	unsigned char *sprite;
	sprite = LoadSprit(index);
	ofs = CGA_CalcXY_p(x, y);
	DrawSpriteFlip(sprite, target, ofs);
}

void BackupAndShowSprite(unsigned char index, unsigned char x, unsigned char y) {
	unsigned char w, h;
	unsigned int ofs;
	unsigned char *sprite = LoadSprit(index);
	ofs = CGA_CalcXY_p(x, y);
	w = sprite[0];
	h = sprite[1];
	CGA_BackupImageReal(ofs, w, h);
	DrawSprite(sprite, CGA_SCREENBUFFER, ofs);  /*DrawSpriteN(index, x, y, CGA_SCREENBUFFER);*/
}

/*
Blit progressive image to interlaced screen buffer, then wait for VBlank
Push image from the top to down
NB! width and pixelswidth specify a number of bytes, not count of pixels
*/
void CGA_AnimLiftToDown(unsigned char *pixels, unsigned int pw, unsigned int w, unsigned int h, unsigned char *screen, unsigned int ofs) {
	unsigned int i;
	pixels += pw * (h - 1);
	for (i = 1; i <= h; i++) {
		CGA_BlitAndWait(pixels, pw, w, i, screen, ofs);
		pixels -= pw;
	}
}

/*
Blit progressive image to interlaced screen buffer, then wait for VBlank
Pull and expand image from the right to left
NB! width and pixelswidth specify a number of bytes, not count of pixels
NB! ofs specifies top-right corner of the image
*/
void CGA_AnimLiftToLeft(unsigned int n, unsigned char *pixels, unsigned int pw, unsigned int w, unsigned int h, unsigned char *screen, unsigned int ofs) {
	unsigned int i;
	for (i = 0; i < n; i++) {
		CGA_BlitAndWait(pixels, pw, w + i, h, screen, ofs);
		ofs -= 1;
	}
}

/*
Blit progressive image to interlaced screen buffer, then wait for VBlank
Push image from the left to right
NB! width and pixelswidth specify a number of bytes, not count of pixels
*/
void CGA_AnimLiftToRight(unsigned int n, unsigned char *pixels, unsigned int pw, unsigned int w, unsigned int h, unsigned char *screen, unsigned int ofs) {
	unsigned int i;
	for (i = 0; i < n; i++) {
		CGA_BlitAndWait(pixels, pw, w + i, h, screen, ofs);
		pixels -= 1;
	}
}

/*
Blit progressive image to interlaced screen buffer, then wait for VBlank
Push image from the down to up
NB! width and pixelswidth specify a number of bytes, not count of pixels
NB! x:y specifies left-bottom coords
*/
void CGA_AnimLiftToUp(unsigned char *pixels, unsigned int pw, unsigned int w, unsigned int h, unsigned char *screen, unsigned int x, unsigned int y) {
	unsigned int i;
	for (i = 1; i <= h; i++) {
		CGA_BlitAndWait(pixels, pw, w, i, screen, CGA_CalcXY_p(x, y));
		y -= 1;
	}
}

/*Copy interlaced screen data n lines down (with clipping)*/
/*Fill gap with source screen data*/
/*offs points to block's bottom most line, data will be shifted to next line*/
/*NB! w is in bytes*/
void CGA_HideScreenBlockLiftToDown(unsigned int n, unsigned char *screen, unsigned char *source, unsigned int w, unsigned int h, unsigned char *target, unsigned int ofs) {
	while (n--) {
		int i;
		unsigned int sofs, tofs;

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
		WaitVBlank();

		ofs ^= CGA_ODD_LINES_OFS;
		if ((ofs & CGA_ODD_LINES_OFS) == 0)
			ofs += CGA_BYTES_PER_LINE;
	}
}

/*Copy interlaced screen data n lines up (with clipping)*/
/*Fill gap with source screen data*/
/*offs points to block's top most line, data will be shifted to previous line*/
/*NB! w is in bytes*/
void CGA_HideScreenBlockLiftToUp(unsigned int n, unsigned char *screen, unsigned char *source, unsigned int w, unsigned int h, unsigned char *target, unsigned int ofs) {
	while (n--) {
		int i;
		unsigned int sofs, tofs;

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
		WaitVBlank();

		ofs ^= CGA_ODD_LINES_OFS;
		if ((ofs & CGA_ODD_LINES_OFS) != 0)
			ofs -= CGA_BYTES_PER_LINE;
	}
}

/*Copy interlaced screen data n lines left (with clipping)*/
/*Fill gap with source screen data*/
/*offs points to block's left most column, data will be shifted to previous column*/
/*NB! w is in bytes*/
void CGA_HideScreenBlockLiftToLeft(unsigned int n, unsigned char *screen, unsigned char *source, unsigned int w, unsigned int h, unsigned char *target, unsigned int ofs) {
	while (n--) {
		int i;
		unsigned int sofs, tofs;

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

		WaitVBlank();

		ofs--;
	}
}

/*Copy interlaced screen data n lines right (with clipping)*/
/*Fill gap with source screen data*/
/*offs points to block's right most column, data will be shifted to next column*/
/*NB! w is in bytes*/
void CGA_HideScreenBlockLiftToRight(unsigned int n, unsigned char *screen, unsigned char *source, unsigned int w, unsigned int h, unsigned char *target, unsigned int ofs) {
	while (n--) {
		int i;
		unsigned int sofs, tofs;

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

		WaitVBlank();

		ofs++;
	}
}


typedef struct scrpiece_t {
	unsigned int    offs;
	unsigned char   delay;  /*speed in bits 1..0, delay in bits 7..2*/
	unsigned char   pix0;
	unsigned char   pix2;
	unsigned char   pix1;
	unsigned char   pix3;
} scrpiece_t;

static const unsigned char piecedelays[] = {
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
static void ScreenToPieces(unsigned char width, unsigned char height, unsigned char *screen, unsigned int offs, scrpiece_t *pieces) {
	const unsigned char *delays = piecedelays;
	height = (height + 3) / 4;
	while (height--) {
		unsigned int x;
		for (x = 0; x < width; x++) {
			unsigned int bofs = offs + x;
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
		}
		offs += CGA_BYTES_PER_LINE * 2; /*4 lines down*/
	}
	pieces->offs = 0;   /*end of list*/
}

static void FallPieces(scrpiece_t *pieces, unsigned char *source, unsigned char *target) {
	unsigned char t = 1;
	unsigned char again = 0;
	do {
		scrpiece_t *piece;
		for (piece = pieces, again = 0; piece->offs; piece++) {
			if ((piece->delay >> 2) < t) {
				unsigned int offs = piece->offs;
				unsigned int bofs = offs;
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
		WaitVBlank();
		t++;
	} while (again);
}

void CGA_HideShatterFall(unsigned char *screen, unsigned char *source, unsigned int w, unsigned int h, unsigned char *target, unsigned int ofs) {
	scrpiece_t *pieces = (scrpiece_t *)scratch_mem2;
	ScreenToPieces(w, h, screen, ofs, pieces);
	FallPieces(pieces, source, target);
}

void CGA_TraceLine(unsigned int sx, unsigned int ex, unsigned int sy, unsigned int ey, unsigned char *source, unsigned char *target) {
	unsigned char b0 = 0;
	unsigned char b1 = 0;
	unsigned char mask;
	unsigned int ofs;
	unsigned int count;
	int w, h, dx, dy, val;

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

	ofs = CGA_CalcXY_p(sx / 4, sy);
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
}

} // End of namespace Chamber
