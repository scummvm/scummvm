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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

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

TransformStruct::TransformStruct(int32 zoomX, int32 zoomY, TSpriteBlendMode blendMode, uint32 rgbaMod, bool mirrorX, bool mirrorY) {
	init(Point32(zoomX, zoomY), 
		DEFAULT_ANGLE, 
		Point32(DEFAULT_HOTSPOT_X, DEFAULT_HOTSPOT_Y), 
		false, 
		blendMode, 
		rgbaMod, 
		mirrorX, 
		mirrorY,  
		Point32(DEFAULT_OFFSET_X, DEFAULT_OFFSET_Y));
}

TransformStruct::TransformStruct(int32 zoom, TSpriteBlendMode blendMode, uint32 rgbaMod, bool mirrorX, bool mirrorY) {
	init(Point32(zoom, zoom), 
		DEFAULT_ANGLE, 
		Point32(DEFAULT_HOTSPOT_X, DEFAULT_HOTSPOT_Y), 
		false,
		blendMode, 
		rgbaMod, 
		mirrorX, mirrorY, 
		Point32(DEFAULT_OFFSET_X, DEFAULT_OFFSET_Y));
}

TransformStruct::TransformStruct(int32 zoom, bool mirrorX, bool mirrorY) {
	init(Point32(zoom, zoom), 
		DEFAULT_ANGLE,
		Point32(DEFAULT_HOTSPOT_X, DEFAULT_HOTSPOT_Y), 
		true, 
		BLEND_NORMAL, 
		DEFAULT_RGBAMOD, 
		mirrorX, mirrorY,  
		Point32(DEFAULT_OFFSET_X, DEFAULT_OFFSET_Y));
}

TransformStruct::TransformStruct(int32 zoomX, int32 zoomY, uint32 angle, int32 hotspotX, int32 hotspotY) {
	init(Point32(zoomX, zoomY), 
		angle, 
		Point32(hotspotX, hotspotY), 
		true, 
		BLEND_NORMAL, 
		DEFAULT_RGBAMOD, 
		false, false,
		Point32(DEFAULT_OFFSET_X, DEFAULT_OFFSET_Y));
}

TransformStruct::TransformStruct(int32 zoom) {
	init(Point32(zoom, zoom), 
		DEFAULT_ANGLE, 
		Point32(DEFAULT_HOTSPOT_X, DEFAULT_HOTSPOT_Y), 
		true, 
		BLEND_NORMAL, 
		DEFAULT_RGBAMOD, 
		false, false,  
		Point32(DEFAULT_OFFSET_X, DEFAULT_OFFSET_Y));
}

TransformStruct::TransformStruct() {
	init(Point32(DEFAULT_ZOOM_X, DEFAULT_ZOOM_Y), 
		DEFAULT_ANGLE, 
		Point32(DEFAULT_HOTSPOT_X, DEFAULT_HOTSPOT_Y), 
		true, 
		BLEND_NORMAL, 
		DEFAULT_RGBAMOD, 
		false, false,  
		Point32(DEFAULT_OFFSET_X, DEFAULT_OFFSET_Y));
}

bool TransformStruct::mirrorX() const {
	return (bool)(_flip & TransparentSurface::FLIP_H);
}

bool TransformStruct::mirrorY() const {
	return (bool)(_flip & TransparentSurface::FLIP_V);
}

}
