/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2006 The ScummVM project
 * Copyright (C) 2002 Ph0x - GP32 Backend
 * Copyright (C) 2003/2004 DJWillis - GP32 Backend
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

#ifndef __GP32STD_GRAP_H
#define __GP32STD_GRAP_H

extern GPDRAWSURFACE lcdSurface[2];
extern uint8 flipIndex;

extern uint16 *frameBuffer1;
extern uint16 *frameBuffer2;

extern void gp_fillRect(uint16 *frameBuffer, int16 x, int16 y, int16 w, int16 h, uint16 color);
extern uint16 gp_RGBTo16(uint16 r, uint16 g, uint16 b);
extern void gp_16ToRGB(uint16 color, uint8 *r, uint8 *g, uint8 *b);

extern void gp_textOut(uint16 *frameBuffer, int x, int y, char* lpszText, uint16 wColor);

extern void gp_initGammaTable(float value);
extern void gp_initFrameBuffer();
extern void gp_flipScreen();

#define gpd_drawPixel16(dst,x,y,color) (dst)[(239 - (y)) + (240 * (x))] = (color)

#endif
