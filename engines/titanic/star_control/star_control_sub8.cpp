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

#include "titanic/star_control/star_control_sub8.h"
#include "titanic/star_control/star_control_sub7.h"
#include "titanic/star_control/star_control_sub12.h"
#include "titanic/star_control/star_field.h"
#include "titanic/star_control/star_ref.h"

namespace Titanic {

CStarControlSub8::CStarControlSub8() : _field8(-1), _entryIndex(-1) {
}

int CStarControlSub8::findStar(const Common::Point &pt) {
	// TODO
	return -1;
}

void CStarControlSub8::selectStar(int index, CVideoSurface *surface,
		CStarField *starField, CStarControlSub7 *sub7) {
	// TODO
}

bool CStarControlSub8::fn1(CStarField *starField, CSurfaceArea *surfaceArea, CStarControlSub12 *sub12) {
	int count = starField->baseFn2(surfaceArea, sub12);

	if (count > 0) {
		allocate(count);
		CStarRef2 starRef(starField, &_positions);
		starRef.process(surfaceArea, sub12);
		return true;
	} else {
		clear();
		return false;
	}

	// TODO
	return true;
}

void CStarControlSub8::fn2(CVideoSurface *surface, CStarField *starField, CStarControlSub7 *sub7) {
	// TODO
}

void CStarControlSub8::fn3() {
	if (_field8 < 3)
		++_field8;
}

FPoint CStarControlSub8::getPosition() const {
	return (_entryIndex >= 0 && _entryIndex <= 2) ? 
		FPoint(_entries[_entryIndex].left, _entries[_entryIndex].top) : FPoint();
}

void CStarControlSub8::draw(CSurfaceArea *surfaceArea) {
	if (!_positions.empty()) {
		uint oldPixel = surfaceArea->_pixel;
		surfaceArea->_pixel = 0xFF;
		surfaceArea->setColorFromPixel();
		SurfaceAreaMode oldMode = surfaceArea->setMode(SA_NONE);

		// TODO: Loop
		/*
		for (int idx = 0; idx < _entryIndex; ++idx) {
			Common::Rect &r = _entries[idx];

		}
		*/

		surfaceArea->_pixel = oldPixel;
		surfaceArea->setMode(oldMode);
	}
}

void CStarControlSub8::allocate(int count) {
	if (!_positions.empty()) {
		if ((int)_positions.size() == count)
			return;

		clear();
	}

	_positions.resize(count);
}

void CStarControlSub8::clear() {
	_positions.clear();
	_field8 = _entryIndex = -1;
}

int CStarControlSub8::indexOf(const Common::Point &pt) const {
	Common::Rect r(pt.x - 2, pt.y - 2, pt.x + 2, pt.y + 2);

	for (int idx = 0; idx < (int)_positions.size(); ++idx) {
		if (r.contains(_positions[idx]._position))
			return idx;
	}

	return -1;
}

void CStarControlSub8::fn4(int index, CSurfaceArea *surfaceArea) {
	// TODO
}

void CStarControlSub8::fn5(int index, CVideoSurface *surface, CStarField *starField, CStarControlSub7 *sub7) {
	// TODO
}

void CStarControlSub8::fn6(CSurfaceArea *surfaceArea) {
	// TODO
}

void CStarControlSub8::fn7(const FPoint &pt, CSurfaceArea *surfaceArea) {
	// TODO
}

} // End of namespace Titanic
