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

#ifndef GRAPHICS_TRANSFORM_ENUMS_H
#define GRAPHICS_TRANSFORM_ENUMS_H

#include "common/scummsys.h"

// WARNING: DO NOT PUT ANY FUNCTION DEFINITIONS (I.E. CODE) IN THIS FILE.
// DO NOT INCLUDE ANYTHING THAT CONTAINS FUNCTION DEFINITIONS EITHER.
// SEE COMMENTS IN "blit-nocode.h" FOR WHY.

namespace Graphics {

enum TSpriteBlendMode {
	BLEND_UNKNOWN = -1,
	BLEND_NORMAL = 0,
	BLEND_ADDITIVE = 1,
	BLEND_SUBTRACTIVE = 2,
	BLEND_MULTIPLY = 3,
	NUM_BLEND_MODES
};

enum AlphaType {
	ALPHA_OPAQUE = 0,
	ALPHA_BINARY = 1,
	ALPHA_FULL = 2
};

/**
 @brief The possible flipping parameters for the blit method.
 */
enum FLIP_FLAGS {
	/// The image will not be flipped.
	FLIP_NONE = 0,
	/// The image will be flipped at the horizontal axis.
	FLIP_H = 1,
	/// The image will be flipped at the vertical axis.
	FLIP_V = 2,
	/// The image will be flipped at the horizontal and vertical axis.
	FLIP_HV = FLIP_H | FLIP_V,
	/// The image will be flipped at the horizontal and vertical axis.
	FLIP_VH = FLIP_H | FLIP_V
};

/**
 * Contains all the required information that define a transform.
 * Same source sprite + same TransformStruct = Same resulting sprite.
 * Has a number of overloaded constructors to accommodate various argument lists.
 */

const int32 kDefaultZoomX = 100;
const int32 kDefaultZoomY = 100;
const uint32 kDefaultRgbaMod = 0xFFFFFFFF;
const int32 kDefaultHotspotX = 0;
const int32 kDefaultHotspotY = 0;
const int32 kDefaultOffsetX = 0;
const int32 kDefaultOffsetY = 0;
const int32 kDefaultAngle = 0;

} // End of namespace Graphics

// WARNING: DO NOT PUT ANY FUNCTION DEFINITIONS (I.E. CODE) IN THIS FILE.
// DO NOT INCLUDE ANYTHING THAT CONTAINS FUNCTION DEFINITIONS EITHER.
// SEE COMMENTS IN "blit-nocode.h" FOR WHY.

#endif
