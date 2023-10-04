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

#include "ultima/ultima8/world/sort_item.h"

namespace Ultima {
namespace Ultima8 {

void SortItem::setBoxBounds(const Box& box, int32 sx, int32 sy) {
	_x = box._x;
	_y = box._y;
	_z = box._z;
	_xLeft = _x - box._xd;
	_yFar = _y - box._yd;
	_zTop = _z + box._zd;

	// NOTE: Precision loss from integer division is intention to fix
	// rendering issue at MainActor::teleport 37 18168 17656 104

	// Screenspace bounding box left extent    (LNT x coord)
	_sxLeft = _xLeft / 4 - _y / 4 - sx;
	// Screenspace bounding box right extent   (RFT x coord)
	_sxRight = _x / 4 - _yFar / 4 - sx;

	// Screenspace bounding box top x coord    (LFT x coord)
	_sxTop = _xLeft / 4 - _yFar / 4 - sx;
	// Screenspace bounding box top extent     (LFT y coord)
	_syTop = _xLeft / 8 + _yFar / 8 - _zTop - sy;

	// Screenspace bounding box bottom x coord (RNB x coord)
	_sxBot = _x / 4 - _y / 4 - sx;
	// Screenspace bounding box bottom extent  (RNB y coord)
	_syBot = _x / 8 + _y / 8 - _z - sy;

	// Screenspace rect - replace with shape frame calculations
	_sr.left = _sxLeft;
	_sr.top = _syTop;
	_sr.right = _sxRight + 1;
	_sr.bottom = _syBot + 1;

	// These help out with sorting. We calc them now, so it will be faster
	_fbigsq = box._xd == box._yd && box._xd >= 128;
	_flat = box._zd == 0;
}

bool SortItem::below(const SortItem &si2) const {
	const SortItem &si1 = *this;

	if (si1._sprite != si2._sprite)
		return si1._sprite < si2._sprite;

	// Check clearly in Z
	if (si1._flat && si2._flat) {
		if (si1._z != si2._z)
			return si1._z < si2._z;
	} else if (si1._invitem == si2._invitem) {
		// Lower item cannot be inventory
		if (si1._zTop <= si2._z)
			return true;
		if (si1._z >= si2._zTop)
			return false;
	}

	// Check clearly in Y
	bool yFlat1 = si1._yFar == si1._y;
	bool yFlat2 = si2._yFar == si2._y;
	if (yFlat1 && yFlat2) {
		// Check with a precision loss based on footpad calculations
		if (si1._y / 32 != si2._y / 32)
			return si1._y < si2._y;
	} else {
		if (si1._y <= si2._yFar)
			return true;
		if (si1._yFar >= si2._y)
			return false;
	}

	// Check clearly in X
	bool xFlat1 = si1._xLeft == si1._x;
	bool xFlat2 = si2._xLeft == si2._x;
	if (xFlat1 && xFlat2) {
		// Check with a precision loss based on footpad calculations
		if (si1._x / 32 != si2._x / 32)
			return si1._x < si2._x;
	} else {
		if (si1._x <= si2._xLeft)
			return true;
		if (si1._xLeft >= si2._x)
			return false;
	}

	// Check with a tolerance based on footpad calculations
	if (si1._zTop - 8 <= si2._z && si1._z < si2._zTop - 8)
		return true;
	if (si1._z >= si2._zTop - 8 && si1._zTop - 8 > si2._z)
		return false;

	// Y-flat vs non-flat handling
	if (yFlat1 != yFlat2) {
		// Check with a precision loss based on footpad calculations
		if (si1._y / 32 <= si2._yFar / 32)
			return true;
		if (si1._yFar / 32 >= si2._y / 32)
			return false;

		int32 yCenter1 = (si1._yFar / 32 + si1._y / 32) / 2;
		int32 yCenter2 = (si2._yFar / 32 + si2._y / 32) / 2;
		if (yCenter1 != yCenter2)
			return yCenter1 < yCenter2;
	}

	// X-flat vs non-flat handling
	if (xFlat1 != xFlat2) {
		// Check with a precision loss based on footpad calculations
		if (si1._x / 32 <= si2._xLeft / 32)
			return true;
		if (si1._xLeft / 32 >= si2._x / 32)
			return false;

		int32 xCenter1 = (si1._xLeft / 32 + si1._x / 32) / 2;
		int32 xCenter2 = (si2._xLeft / 32 + si2._x / 32) / 2;
		if (xCenter1 != xCenter2)
			return xCenter1 < xCenter2;
	}

	// Specialist z flat handling
	if (si1._flat || si2._flat) {
		// Lower z-bottom drawn before
		if (si1._z != si2._z)
			return si1._z < si2._z;

		// Inv items always drawn after
		if (si1._invitem != si2._invitem)
			return si1._invitem < si2._invitem;

		// Flat gets drawn before
		if (si1._flat != si2._flat)
			return si1._flat > si2._flat;

		// Trans always gets drawn after
		if (si1._trans != si2._trans)
			return si1._trans < si2._trans;

		// Animated always gets drawn after
		if (si1._anim != si2._anim)
			return si1._anim < si2._anim;

		// Draw always gets drawn first
		if (si1._draw != si2._draw)
			return si1._draw > si2._draw;

		// Solid always gets drawn first
		if (si1._solid != si2._solid)
			return si1._solid > si2._solid;

		// Occludes always get drawn first
		if (si1._occl != si2._occl)
			return si1._occl > si2._occl;

		// Large flats squares get drawn first
		if (si1._fbigsq != si2._fbigsq)
			return si1._fbigsq > si2._fbigsq;
	}

	// Specialist handling for same location
	if (si1._x == si2._x && si1._y == si2._y) {
		// Trans always gets drawn after
		if (si1._trans != si2._trans)
			return si1._trans < si2._trans;
	}

	// Disabled: Land always gets drawn first
	// if (si1._land != si2._land)
	//	return si1._land > si2._land;

	// Land always gets drawn before roof
	if (si1._land && si2._land && si1._roof != si2._roof)
		return si1._roof < si2._roof;

	// Roof always gets drawn first
	if (si1._roof != si2._roof)
		return si1._roof > si2._roof;

	// Lower z-bottom drawn before
	if (si1._z != si2._z)
		return si1._z < si2._z;

	if (xFlat1 || xFlat2 || yFlat1 || yFlat2) {
		// Higher screenspace left drawn before?
		if (si1._sxLeft != si2._sxLeft)
			return si1._sxLeft > si2._sxLeft;

		// Lower screenspace bottom drawn before?
		if (si1._syBot != si2._syBot)
			return si1._syBot < si2._syBot;
	}

	// Partial in X + Y front
	if (si1._x + si1._y != si2._x + si2._y)
		return (si1._x + si1._y < si2._x + si2._y);

	// Partial in X + Y back
	if (si1._xLeft + si1._yFar != si2._xLeft + si2._yFar)
		return (si1._xLeft + si1._yFar < si2._xLeft + si2._yFar);

	// Partial in y?
	if (si1._y != si2._y)
		return si1._y < si2._y;

	// Partial in x?
	if (si1._x != si2._x)
		return si1._x < si2._x;

	// Just sort by shape number
	if (si1._shapeNum != si2._shapeNum)
		return si1._shapeNum < si2._shapeNum;

	// And then by _frame
	return si1._frame < si2._frame;
}

Common::String SortItem::dumpInfo() const {
	Box b = getBoxBounds();
	Common::String info = Common::String::format("%u:%u (%d, %d, %d, %d, %d, %d): ",
								_shapeNum, _frame, b._x, b._y, b._z, b._xd, b._yd, b._zd);
	if (_sprite)
		info += "sprite ";
	if (_flat)
		info += "flat ";
	if (_anim)
		info += "anim ";
	if (_trans)
		info += "trans ";
	if (_draw)
		info += "draw ";
	if (_solid)
		info += "solid ";
	if (_occl)
		info += "occl ";
	if (_fbigsq)
		info += "fbigsq ";
	if (_roof)
		info += "roof ";
	if (_land)
		info += "land ";
	if (_noisy)
		info += "noisy ";
	if (_fixed)
		info += "fixed ";

	return info;
}

} // End of namespace Ultima8
} // End of namespace Ultima
