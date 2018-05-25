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


#include "graphics/transform_tools.h"
#include "common/math.h"
#include <math.h>

namespace Graphics {

FloatPoint TransformTools::transformPoint(FloatPoint point, const float rotate, const Common::Point &zoom, const bool mirrorX, const bool mirrorY) {
	float rotateRad = Common::deg2rad<float>(rotate);
	float x = point.x;
	float y = point.y;
	x = (x * zoom.x) / kDefaultZoomX;
	y = (y * zoom.y) / kDefaultZoomY;
#if 0
	// TODO: Mirroring should be done before rotation, but the blitting
	// code does the inverse, so we match that for now.
	if (mirrorX)
		x *= -1;
	if (mirrorY)
		y *= -1;
#endif
	FloatPoint newPoint;
	newPoint.x = x * cos(rotateRad) - y * sin(rotateRad);
	newPoint.y = x * sin(rotateRad) + y * cos(rotateRad);
	if (mirrorX) {
		newPoint.x *= -1;
	}
	if (mirrorY) {
		newPoint.y *= -1;
	}
	return newPoint;
}

Common::Rect TransformTools::newRect(const Common::Rect &oldRect, const TransformStruct &transform, Common::Point *newHotspot) {
	Common::Point nw(oldRect.left, oldRect.top);
	Common::Point ne(oldRect.right, oldRect.top);
	Common::Point sw(oldRect.left, oldRect.bottom);
	Common::Point se(oldRect.right, oldRect.bottom);

	FloatPoint nw1, ne1, sw1, se1;

	nw1 = transformPoint(nw - transform._hotspot, transform._angle, transform._zoom);
	ne1 = transformPoint(ne - transform._hotspot, transform._angle, transform._zoom);
	sw1 = transformPoint(sw - transform._hotspot, transform._angle, transform._zoom);
	se1 = transformPoint(se - transform._hotspot, transform._angle, transform._zoom);

	float top = MIN(nw1.y, MIN(ne1.y, MIN(sw1.y, se1.y)));
	float bottom = MAX(nw1.y, MAX(ne1.y, MAX(sw1.y, se1.y)));
	float left = MIN(nw1.x, MIN(ne1.x, MIN(sw1.x, se1.x)));
	float right = MAX(nw1.x, MAX(ne1.x, MAX(sw1.x, se1.x)));

	if (newHotspot) {
		newHotspot->y = (uint32)(-floor(top));
		newHotspot->x = (uint32)(-floor(left));
	}

	Common::Rect res;
	res.top = (int32)(floor(top)) + transform._hotspot.y;
	res.bottom = (int32)(ceil(bottom)) + transform._hotspot.y;
	res.left = (int32)(floor(left)) + transform._hotspot.x;
	res.right = (int32)(ceil(right)) + transform._hotspot.x;

	return res;
}

} // End of namespace Graphics
