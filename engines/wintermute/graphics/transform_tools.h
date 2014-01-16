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

#ifndef WINTERMUTE_TRANSFORM_TOOLS_H
#define WINTERMUTE_TRANSFORM_TOOLS_H

#include "engines/wintermute/math/rect32.h"
#include "engines/wintermute/math/floatpoint.h"
#include "engines/wintermute/graphics/transform_struct.h"

namespace Wintermute {

class TransformTools {
public:
	/**
	 * Basic transform (scale + rotate) for a single point
	 */
	static FloatPoint transformPoint(FloatPoint point, const float rotate, const Point32 &zoom, const bool mirrorX = false, const bool mirrorY = false);

	/**
	 * @param &point the point on which the transform is to be applied
	 * @param rotate the angle in degrees
	 * @param &zoom  zoom x,y in percent
	 * @param mirrorX flip along the vertical axis?
	 * @param mirrorY flip along the horizontal axis?
	 * @return the smallest rect that can contain the transformed sprite
	 * and, as a side-effect, "newHotspot" will tell you where the hotspot will
	 * have ended up in the new rect, for centering.
	 */
	static Rect32 newRect(const Rect32 &oldRect, const TransformStruct &transform, Point32 *newHotspot);
};

} // End of namespace Wintermute
#endif
