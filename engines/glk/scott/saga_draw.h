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

/*
 * Based on ScottFree interpreter version 1.14 developed by Swansea
 * University Computer Society without disassembly of any other game
 * drivers, only of game databases as permitted by EEC law (for purposes
 * of compatibility).
 *
 * Licensed under GPLv2
 *
 * https://github.com/angstsmurf/spatterlight/tree/master/terps/scott
 */

#ifndef GLK_SCOTT_SAGADRAW_H
#define GLK_SCOTT_SAGADRAW_H

#include "glk/glk_types.h"
#include "glk/scott/types.h"

namespace Glk {
namespace Scott {

struct Image {
	uint8_t *_imageData;
	uint8_t _xOff;
	uint8_t _yOff;
	uint8_t _width;
	uint8_t _height;
};

typedef uint8_t RGB[3];
typedef RGB PALETTE[16];

uint8_t *drawSagaPictureFromData(uint8_t *dataptr, int xSize, int ySize, int xOff, int yOff);
void drawSagaPictureNumber(int pictureNumber);
void drawSagaPictureAtPos(int pictureNumber, int x, int y);
void drawSagaPictureFromBuffer();
void flip(uint8_t character[]);

void sagaSetup(size_t imgOffset);

void putPixel(glsi32 x, glsi32 y, int32_t color);
void rectFill(int32_t x, int32_t y, int32_t width, int32_t height, int32_t color);
void switchPalettes(int pal1, int pal2);
void definePalette();

int32_t remap(int32_t color);

} // End of namespace Scott
} // End of namespace Glk

#endif
