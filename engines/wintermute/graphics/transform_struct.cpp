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

#include "engines/wintermute/graphics/transform_struct.h"
#include "engines/wintermute/graphics/transparent_surface.h"

namespace Wintermute {
void TransformStruct::init(Point32 zoom, uint32 angle, Point32 hotspot, bool alphaDisable, TSpriteBlendMode blendMode, uint32 rgbaMod, bool mirrorX, bool mirrorY, Point32 offset) {
	_zoom = zoom;
	_angle = angle;
	_hotspot = hotspot;
	_blendMode = blendMode;
	_rgbaMod = rgbaMod;
	_alphaDisable = alphaDisable;
	_flip = 0;
	_flip += TransparentSurface::FLIP_H * mirrorX;
	_flip += TransparentSurface::FLIP_V * mirrorY;
	_offset = offset;
	_numTimesX = 1;
	_numTimesY = 1;
}

TransformStruct::TransformStruct(int32 zoomX, int32 zoomY, uint32 angle, int32 hotspotX, int32 hotspotY, TSpriteBlendMode blendMode, uint32 rgbaMod, bool mirrorX, bool mirrorY, int32 offsetX, int32 offsetY) {
	init(Point32(zoomX, zoomY),
		angle,
		Point32(hotspotX, hotspotY),
		false,
		blendMode,
		rgbaMod,
		mirrorX, mirrorY,
		Point32(offsetX, offsetY));
}

TransformStruct::TransformStruct(float zoomX, float zoomY, uint32 angle, int32 hotspotX, int32 hotspotY, TSpriteBlendMode blendMode, uint32 rgbaMod, bool mirrorX, bool mirrorY, int32 offsetX, int32 offsetY) {
	init(Point32((int32)(zoomX / 100.0 * kDefaultZoomX),
		     (int32)(zoomY / 100.0 * kDefaultZoomY)),
		angle,
		Point32(hotspotX, hotspotY),
		false,
		blendMode,
		rgbaMod,
		mirrorX, mirrorY,
		Point32(offsetX, offsetY));
}

TransformStruct::TransformStruct(int32 zoomX, int32 zoomY, TSpriteBlendMode blendMode, uint32 rgbaMod, bool mirrorX, bool mirrorY) {
	init(Point32(zoomX, zoomY),
		kDefaultAngle,
		Point32(kDefaultHotspotX, kDefaultHotspotY),
		false,
		blendMode,
		rgbaMod,
		mirrorX,
		mirrorY,
		Point32(kDefaultOffsetX, kDefaultOffsetY));
}

TransformStruct::TransformStruct(int32 zoomX, int32 zoomY, uint32 angle, int32 hotspotX, int32 hotspotY) {
	init(Point32(zoomX, zoomY),
		angle,
		Point32(hotspotX, hotspotY),
		true,
		BLEND_NORMAL,
		kDefaultRgbaMod,
		false, false,
		Point32(kDefaultOffsetX, kDefaultOffsetY));
}

TransformStruct::TransformStruct(int32 numTimesX, int32 numTimesY) {
	init(Point32(kDefaultZoomX, kDefaultZoomY),
		kDefaultAngle,
		Point32(kDefaultHotspotX, kDefaultHotspotY),
		false,
		BLEND_NORMAL,
		kDefaultRgbaMod,
		false, false,
		Point32(kDefaultOffsetX, kDefaultOffsetY));
	_numTimesX = numTimesX;
	_numTimesY = numTimesY;
}

TransformStruct::TransformStruct() {
	init(Point32(kDefaultZoomX, kDefaultZoomY),
		kDefaultAngle,
		Point32(kDefaultHotspotX, kDefaultHotspotY),
		true,
		BLEND_NORMAL,
		kDefaultRgbaMod,
		false, false,
		Point32(kDefaultOffsetX, kDefaultOffsetY));
}

bool TransformStruct::getMirrorX() const {
	return (bool)(_flip & TransparentSurface::FLIP_H);
}

bool TransformStruct::getMirrorY() const {
	return (bool)(_flip & TransparentSurface::FLIP_V);
}
} // End of namespace Wintermute
