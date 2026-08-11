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
#ifndef TOT_ANIMS_H
#define TOT_ANIMS_H

#include "common/scummsys.h"

namespace Tot {

void drawFlc(uint x, uint y, int32 offset, uint loop,
			 byte speed, byte eventNumber, bool fullPalette, bool isSkipAllowed,
			 bool limitPaletteTo200, bool &exitAnim);

/**
 * Header of an Autodesk Animator FLIC file.
 */
struct FliHeader {
	int32 size; // file size
	uint magic, // format id
		frames, // number of animation frames
		width,  // anim width
		height, // anim height
		depth,  // 8 bits per pixel
		flags;
	int32 speed;        // time delay between frames
	uint reserved1;     // all set to zero
	int32 created;      // creation date in DOS format
	byte creator[3];    // serial of creator program
	int32 updated;      // update date in DOS format
	byte updator[3];    // serial of updating program
	uint aspectx,       // aspect ratio
		aspecty;        // aspect ratio
	uint reserved2[19]; // all zeroes
	int32 ofsframe1,    // Offset to frame 1 (FLC only)
		ofsframe2;      // Offset to frame 2 (FLC only)
	uint reserved3[20]; // All zeroes
};

} // End of namespace Tot
#endif
