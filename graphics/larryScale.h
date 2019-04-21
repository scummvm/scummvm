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

#ifndef GRAPHICS_LARRYSCALE_H
#define GRAPHICS_LARRYSCALE_H

#include "common/scummsys.h"

namespace Graphics {

/*

LarryScale is a bitmap scaler for cartoon images.

Features:

* Supports arbitrary scaling factors along both axes (but looks best between 50% and 200%).
* When downscaling: preserves fine details.
* When upscaling: keeps lines smooth, introducing very few block artifacts.
* Does not introduce additional colors.

Limitations:

* Lines must not be anti-aliased and should be only one pixel wide.
* Fills should be flat without gradients.

*/

typedef byte LarryScaleColor;

class RowReader {
public:
	virtual const LarryScaleColor *readRow(int y) = 0;
	virtual ~RowReader() {}
};

class RowWriter {
public:
	virtual void writeRow(int y, const LarryScaleColor *row) = 0;
	virtual ~RowWriter() {}
};

/**
 * @param srcWidth			The width, in pixels, of the original image
 * @param srcHeight			The height, in pixels, of the original image
 * @param transparentColor	The transparent color. Used for outline detection.
 * @param rowReader			An object with a callback method for reading the lines of the original
 *							image
 * @param dstWidth			The width, in pixels, of the scaled target image
 * @param dstHeight			The height, in pixels, of the scaled target image
 * @param rowWriter			An object with a callback method accepting the lines of the target image
 */
void larryScale(
	int srcWidth, int srcHeight,
	LarryScaleColor transparentColor,
	RowReader &rowReader,
	int dstWidth, int dstHeight,
	RowWriter &rowWriter
);

}

#endif

