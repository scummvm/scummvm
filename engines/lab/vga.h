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

/*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#include "lab/stddefines.h"

#ifndef LAB_VGA_H
#define LAB_VGA_H

namespace Lab {

struct Image {
	uint16 Width;
	uint16 Height;
	byte *ImageData;
};

bool createScreen(bool HiRes);
void waitTOF();
void quickWaitTOF();
byte *getVGABaseAddr();
void writeColorReg(byte *buf, uint16 regnum);
void writeColorRegs(byte *buf, uint16 first, uint16 numreg);
void writeColorRegsSmooth(byte *buf, uint16 first, uint16 numreg);
void VGASetPal(void *cmap, uint16 numcolors);

/*---------- Drawing Routines ----------*/

void drawImage(Image *Im, uint16 x, uint16 y);
void drawMaskImage(Image *Im, uint16 x, uint16 y);
void readScreenImage(Image *Im, uint16 x, uint16 y);
void bltBitMap(Image *ImSource, uint16 xs, uint16 ys, Image *ImDest, uint16 xd, uint16 yd, uint16 width, uint16 height);
void scrollDisplayX(int16 dx, uint16 x1, uint16 y1, uint16 x2, uint16 y2);
void scrollDisplayY(int16 dy, uint16 x1, uint16 y1, uint16 x2, uint16 y2);
void setAPen(uint16 pennum);
void drawHLine(uint16 x, uint16 y1, uint16 y2);
void drawVLine(uint16 x1, uint16 y, uint16 x2);
void rectFill(uint16 x1, uint16 y1, uint16 x2, uint16 y2);
void ghoastRect(uint16 pencolor, uint16 x1, uint16 y1, uint16 x2, uint16 y2);
void WSDL_UpdateScreen();
void WSDL_GetMousePos(int *x, int *y);
uint16 WSDL_GetNextChar();
bool WSDL_HasNextChar();
void WSDL_ProcessInput(bool can_delay);

} // End of namespace Lab

#endif /* LAB_VGA_H */
