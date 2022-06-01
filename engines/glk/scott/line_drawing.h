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

#ifndef GLK_SCOTT_LINEDRAWING_H
#define GLK_SCOTT_LINEDRAWING_H

#include "glk/scott/types.h"

namespace Glk {
namespace Scott {

struct LineImage {
	uint8_t *_data;
	int _bgColour;
	size_t _size;
};

struct PixelToDraw {
	uint8_t _x = 0;
	uint8_t _y = 0;
	uint8_t _colour = 0;
};

void drawVectorPicture(int image);
void drawSomeVectorPixels(int fromStart);
int drawingVector();
void freePixels();

enum VectorStateType : int {
	NO_VECTOR_IMAGE,
	DRAWING_VECTOR_IMAGE,
	SHOWING_VECTOR_IMAGE
};

} // End of namespace Scott
} // End of namespace Glk

#endif
