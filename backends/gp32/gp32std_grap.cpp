/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2005 The ScummVM project
 * Copyright (C) 2005 Won Star - GP32 Backend
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
 * $Header$
 *
 */

#include "stdafx.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "backends/intern.h"

#include "backends/gp32/gp32std.h"
#include "backends/gp32/gp32std_grap.h"

#include "backends/gp32/globals.h"

GPDRAWSURFACE lcdSurface[2];

uint8 flipIndex = 1;

uint16 *frameBuffer1;
uint16 *frameBuffer2;

uint8 gammaLUT[256];
uint8 gammaLUTInv[256];

extern const unsigned char fontresEng1[];
extern const unsigned char fontresKor1[];

void gp_putBitmap8x16(uint16 *frameBuffer, int x, int y, byte *lpBitmap, uint16 wColor) {
	byte *pBitmap  = lpBitmap;
	for (int nRow = 0; nRow < 12; nRow ++) {
		byte data = *pBitmap++;
		for (int nCol = 0; nCol < 7; nCol ++) {
			if (data & 0x80)
				if (x + nCol >= 0 && y + nRow >= 0 && x + nCol < 320 && y + nRow < 240)
					gpd_drawPixel16(frameBuffer, x + nCol, y + nRow, wColor);

			data <<= 1;
		}
	}
}

void gp_putEngFont(uint16 *frameBuffer, int x, int y, char c, uint16 wColor) {
	byte *pBitmap = (byte *) &fontresEng1[c * 16];
	gp_putBitmap8x16(frameBuffer, x, y, pBitmap, wColor);
}

void gp_textOut(uint16 *frameBuffer, int x, int y, char* lpszText, uint16 wColor) {
	// TODO: Handle korean font
	int nPos = x;
	char* pszText = lpszText;

	while (*pszText != '\0') {
		if (*pszText == '\n') {
			nPos = x;
			y += 8;
		} else {
			gp_putEngFont(frameBuffer, nPos, y, *pszText, wColor);
			nPos += 7;
		}

		pszText++;
	}
}

void gp_fillRect(uint16 *frameBuffer, int16 x, int16 y, int16 w, int16 h, uint16 color) {
	uint16 *buffer = &frameBuffer[(240 - (y + h)) + (240 * x)];
	for (int i = 0; i < w; i++) {
		for (int j = 0; j < h; j++) {
			*buffer++ = color;
		}
		buffer += 240 - h;
	}
}

void gp_initGammaTable(float value)
{
	for (int i = 0; i < 256; i++) {
		if (value == 1.0f) {
			gammaLUT[i] = i;
			gammaLUTInv[i] = i;
		} else {
			gammaLUT[i] = (uint8)(pow((double)i / 256, 1 / (double)value) * 256);
			gammaLUTInv[i] = (uint8)(pow((double)i / 256, (double)value) * 256);
		}
	}
}

uint16 gp_RGBTo16(uint16 r, uint16 g, uint16 b) {
	// GP32 16bit color 5551
	if (g_vars.gammaRamp != 10000) {
		r = gammaLUT[r];
		g = gammaLUT[g];
		b = gammaLUT[b];
	}
	return (((r >> 3) & 0x1F) << 11) | (((g >> 3) & 0x1F) << 6) | ((b >> 3) & 0x1F) << 1;
}

void gp_16ToRGB(uint16 color, uint8 *r, uint8 *g, uint8 *b) {
	*r = ((((color) >> 11) & 0x1F) << 3);
	*g = ((((color) >> 6) & 0x1F) << 3);	//(((color>>5)&0x3F) << 2);
	*b = ((((color) >> 1) & 0x1F) << 3);	//((color&0x1F) << 3);

	if (g_vars.gammaRamp != 10000) {
		*r = gammaLUTInv[*r];
		*g = gammaLUTInv[*g];
		*b = gammaLUTInv[*b];
	}
}

void gp_flipScreen() {
	uint16 *frameBuffer1_old = frameBuffer1;
	uint16 *frameBuffer2_old = frameBuffer2;

	GpSurfaceFlip(&lcdSurface[flipIndex]);
	flipIndex = 1 - flipIndex;
	frameBuffer1 = frameBuffer2_old;
	frameBuffer2 = frameBuffer1_old;
}

void gp_initFrameBuffer() {
	GpLcdSurfaceGet(&lcdSurface[0], 0);
	GpLcdSurfaceGet(&lcdSurface[1], 1);
	GpSurfaceSet(&lcdSurface[0]);
	frameBuffer1 = (uint16 *)lcdSurface[0].ptbuffer;
	frameBuffer2 = (uint16 *)lcdSurface[1].ptbuffer;

	memset(frameBuffer1, 0xFF, LCD_WIDTH * LCD_HEIGHT * sizeof(uint16));
	memset(frameBuffer2, 0xFF, LCD_WIDTH * LCD_HEIGHT * sizeof(uint16));
}
