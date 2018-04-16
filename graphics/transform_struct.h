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

#ifndef GRAPHICS_TRANSFORM_STRUCT_H
#define GRAPHICS_TRANSFORM_STRUCT_H

#include "common/rect.h"

namespace Graphics {

enum TSpriteBlendMode {
	BLEND_UNKNOWN       = -1,
	BLEND_NORMAL        = 0,
	BLEND_ADDITIVE      = 1,
	BLEND_SUBTRACTIVE   = 2,
	BLEND_MULTIPLY		= 3,
	NUM_BLEND_MODES
};

/**
 * Contains all the required information that define a transform.
 * Same source sprite + same TransformStruct = Same resulting sprite.
 * Has a number of overloaded constructors to accomodate various argument lists.
 */

const int32 kDefaultZoomX = 100;
const int32 kDefaultZoomY = 100;
const uint32 kDefaultRgbaMod = 0xFFFFFFFF;
const int32 kDefaultHotspotX = 0;
const int32 kDefaultHotspotY = 0;
const int32 kDefaultOffsetX = 0;
const int32 kDefaultOffsetY = 0;
const int32 kDefaultAngle = 0;

struct TransformStruct {
private:
	void init(Common::Point zoom, uint32 angle, Common::Point hotspot, bool alphaDisable, TSpriteBlendMode blendMode, uint32 alpha, bool mirrorX, bool mirrorY, Common::Point offset);

public:
	TransformStruct(int32 zoomX, int32 zoomY, uint32 angle, int32 hotspotX, int32 hotspotY, TSpriteBlendMode blendMode, uint32 alpha, bool mirrorX = false, bool mirrorY = false, int32 offsetX = 0, int32 offsetY = 0);
	TransformStruct(float zoomX, float zoomY, uint32 angle, int32 hotspotX, int32 hotspotY, TSpriteBlendMode blendMode, uint32 alpha, bool mirrorX = false, bool mirrorY = false, int32 offsetX = 0, int32 offsetY = 0);
	TransformStruct(int32 zoomX, int32 zoomY, TSpriteBlendMode blendMode, uint32 alpha, bool mirrorX = false, bool mirrorY = false);
	TransformStruct(int32 zoomX, int32 zoomY, uint32 angle, int32 hotspotX = 0, int32 hotspotY = 0);
	TransformStruct(int32 numTimesX, int32 numTimesY);
	TransformStruct();

	Common::Point _zoom;   ///< Zoom; 100 = no zoom
	Common::Point _hotspot; ///< Position of the hotspot
	int32 _angle;   ///< Rotation angle, in degrees
	byte _flip;      ///< Bitflag: see TransparentSurface::FLIP_XXX
	bool _alphaDisable;
	TSpriteBlendMode _blendMode;
	uint32 _rgbaMod;      ///< RGBa
	Common::Point _offset;
	int32 _numTimesX;
	int32 _numTimesY;

	bool getMirrorX() const;
	bool getMirrorY() const;

	bool operator==(const TransformStruct &compare) const {
		return (compare._angle == _angle &&
				compare._flip == _flip &&
				compare._zoom == _zoom  &&
				compare._offset == _offset &&
				compare._alphaDisable == _alphaDisable  &&
				compare._rgbaMod == _rgbaMod &&
				compare._blendMode == _blendMode &&
				compare._numTimesX == _numTimesX &&
				compare._numTimesY == _numTimesY
			   );
	}

	bool operator!=(const TransformStruct &compare) const {
		return !(compare == *this);
	}
};
} // End of namespace Graphics
#endif
