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

void CStarControlSub8::selectStar(int index, CVideoSurface *surface,
		CStarField *starField, CStarControlSub7 *sub7) {
	if (_entryIndex >= 0) {
		if (_entryIndex == _field8) {
			if (_field8 != 2) {
				if (_positions[index] != _positions[_entryIndex + 1]) {
					surface->lock();

					CSurfaceArea surfaceArea(surface);
					fn4(index, &surfaceArea);
					surface->unlock();

					++_entryIndex;
					CStarPosition &newP = _positions[_entryIndex + 1];
					newP = _positions[index];

					const CBaseStarEntry *starP = starField->getDataPtr(_positions[index]._index1);
					sub7->addStar(starP);
				}
			}
		} else if (_entryIndex == _field8 + 1) {
			if (_positions[index] == _positions[_entryIndex + 1]) {
				surface->lock();
				CSurfaceArea surfaceArea(surface);
				fn6(&surfaceArea);
				surface->unlock();

				--_entryIndex;
				const CBaseStarEntry *starP = starField->getDataPtr(_positions[index]._index1);
				sub7->addStar(starP);
			} else {
				surface->lock();
				CSurfaceArea surfaceArea(surface);
				fn6(&surfaceArea);
				fn4(index, &surfaceArea);
				surface->unlock();

				const CBaseStarEntry *starP;
				starP = starField->getDataPtr(_positions[_entryIndex]._index1);
				sub7->addStar(starP);
				starP = starField->getDataPtr(_positions[index]._index1);
				sub7->addStar(starP);

				CStarPosition &newP = _positions[_entryIndex + 1];
				newP = _positions[index];
			}
		}
	} else {
		surface->lock();
		CSurfaceArea surfaceArea(surface);
		fn4(index, &surfaceArea);
		surface->unlock();

		++_entryIndex;
		CStarPosition &newP = _positions[_entryIndex + 1];
		newP = _positions[index];

		const CBaseStarEntry *starP = starField->getDataPtr(_positions[index]._index1);
		sub7->addStar(starP);
	}
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
}

void CStarControlSub8::fn2(CVideoSurface *surface, CStarField *starField, CStarControlSub7 *sub7) {
	if (_field8 <= -1) {
		if (_entryIndex > -1) {
			fn5(_entryIndex, surface, starField, sub7);
			--_entryIndex;
		}
	} else {
		--_field8;
		if (_entryIndex - _field8 > 1) {
			fn5(_entryIndex, surface, starField, sub7);
			--_entryIndex;
		}
	}
}

void CStarControlSub8::fn3() {
	if (_field8 < 3)
		++_field8;
}

FPoint CStarControlSub8::getPosition() const {
	return (_entryIndex >= 0 && _entryIndex <= 2) ? 
		FPoint(_entries[_entryIndex]) : FPoint();
}

void CStarControlSub8::draw(CSurfaceArea *surfaceArea) {
	if (!_positions.empty()) {
		uint savedPixel = surfaceArea->_pixel;
		surfaceArea->_pixel = 0xff;
		surfaceArea->setColorFromPixel();
		SurfaceAreaMode savedMode = surfaceArea->setMode(SA_NONE);

		for (int idx = 0; idx < _entryIndex; ++idx) {
			const CStarPosition &src = _entries[idx];
			double xp = src.x, yp = src.y;

			surfaceArea->fn1(FRect(xp - 8.0, yp, xp - 4.0, yp));
			surfaceArea->fn1(FRect(xp + 4.0, yp, xp + 8.0, yp));
			surfaceArea->fn1(FRect(xp, yp - 8.0, xp, yp - 4.0));
			surfaceArea->fn1(FRect(xp, yp + 4.0, xp, yp + 8.0));
		}

		surfaceArea->_pixel = savedPixel;
		surfaceArea->setColorFromPixel();
		surfaceArea->setMode(savedMode);
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
		if (r.contains(_positions[idx]))
			return idx;
	}

	return -1;
}

void CStarControlSub8::fn4(int index, CSurfaceArea *surfaceArea) {
	if (index >= 0 && index < (int)_positions.size()) {
		const CStarPosition &pt = _positions[index];
		fn7(pt, surfaceArea);
	}
}

void CStarControlSub8::fn5(int index, CVideoSurface *surface, CStarField *starField, CStarControlSub7 *sub7) {
	surface->lock();
	CSurfaceArea surfaceArea(surface);
	fn7(_positions[index + 1], &surfaceArea);
	surface->unlock();

	const CBaseStarEntry *starP = starField->getDataPtr(_positions[index + 1]._index1);
	sub7->addStar(starP);
}

void CStarControlSub8::fn6(CSurfaceArea *surfaceArea) {
	const CStarPosition &pt = _positions[_entryIndex];
	fn7(pt, surfaceArea);
}

void CStarControlSub8::fn7(const FPoint &pt, CSurfaceArea *surfaceArea) {
	uint savedPixel = surfaceArea->_pixel;
	surfaceArea->_pixel = 255;
	surfaceArea->setColorFromPixel();
	SurfaceAreaMode savedMode = surfaceArea->setMode(SA_MODE3);


	surfaceArea->fn1(FRect(pt._x - 8.0, pt._y, pt._x - 4.0, pt._y));
	surfaceArea->fn1(FRect(pt._x - -4.0, pt._y, pt._x + 8.0, pt._y));
	surfaceArea->fn1(FRect(pt._x, pt._y - 8.0, pt._x, pt._y - 4.0));
	surfaceArea->fn1(FRect(pt._x, pt._y + 4.0, pt._x, pt._y + 8.0));

	surfaceArea->_pixel = savedPixel;
	surfaceArea->setColorFromPixel();
	surfaceArea->setMode(savedMode);
}

} // End of namespace Titanic
