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

#include "titanic/star_control/photo_crosshairs.h"
#include "titanic/star_control/star_markers.h"
#include "titanic/star_control/star_camera.h"
#include "titanic/star_control/star_field.h"
#include "titanic/star_control/star_ref.h"

namespace Titanic {

CPhotoCrosshairs::CPhotoCrosshairs() : _matchIndex(-1), _entryIndex(-1) {
}

void CPhotoCrosshairs::selectStar(int index, CVideoSurface *surface,
		CStarField *starField, CStarMarkers *markers) {
	if (_entryIndex >= 0) {
		if (_entryIndex == _matchIndex) {
			if (_matchIndex != 2) {
				if (_positions[index] != _entries[_entryIndex]) {
					surface->lock();

					CSurfaceArea surfaceArea(surface);
					drawStar(index, &surfaceArea);
					surface->unlock();

					++_entryIndex;
					CStarPosition &newP = _entries[_entryIndex];
					newP = _positions[index];

					const CBaseStarEntry *starP = starField->getDataPtr(newP._index1);
					markers->addStar(starP);
				}
			}
		} else if (_entryIndex == _matchIndex + 1) {
			if (_positions[index] == _entries[_entryIndex]) {
				surface->lock();
				CSurfaceArea surfaceArea(surface);
				drawCurrent(&surfaceArea);
				surface->unlock();

				--_entryIndex;
				const CBaseStarEntry *starP = starField->getDataPtr(_positions[index]._index1);
				markers->addStar(starP);
			} else {
				surface->lock();
				CSurfaceArea surfaceArea(surface);
				drawCurrent(&surfaceArea);
				drawStar(index, &surfaceArea);
				surface->unlock();

				const CBaseStarEntry *starP;
				starP = starField->getDataPtr(_entries[_entryIndex]._index1);
				markers->addStar(starP);
				starP = starField->getDataPtr(_positions[index]._index1);
				markers->addStar(starP);

				CStarPosition &newP = _entries[_entryIndex];
				newP = _positions[index];
			}
		}
	} else {
		surface->lock();
		CSurfaceArea surfaceArea(surface);
		drawStar(index, &surfaceArea);
		surface->unlock();

		++_entryIndex;
		const CStarPosition &srcPos = _positions[index];
		CStarPosition &destPos = _entries[_entryIndex];
		destPos = srcPos;

		const CBaseStarEntry *starP = starField->getDataPtr(destPos._index1);
		markers->addStar(starP);
	}
}

bool CPhotoCrosshairs::fn1(CStarField *starField, CSurfaceArea *surfaceArea, CStarCamera *camera) {
	int count = starField->baseFn2(surfaceArea, camera);

	if (count > 0) {
		allocate(count);
		CStarRefArray starRef(starField, &_positions);
		starRef.process(surfaceArea, camera);
		return true;
	} else {
		clear();
		return false;
	}
}

void CPhotoCrosshairs::fn2(CVideoSurface *surface, CStarField *starField, CStarMarkers *markers) {
	if (_matchIndex <= -1) {
		if (_entryIndex > -1) {
			drawEntry(_entryIndex, surface, starField, markers);
			--_entryIndex;
		}
	} else {
		--_matchIndex;
		if (_entryIndex - _matchIndex > 1) {
			drawEntry(_entryIndex, surface, starField, markers);
			--_entryIndex;
		}
	}
}

void CPhotoCrosshairs::incMatches() {
	if (_matchIndex < 3)
		++_matchIndex;
}

FPoint CPhotoCrosshairs::getPosition() const {
	return (_entryIndex >= 0 && _entryIndex <= 2) ? 
		FPoint(_entries[_entryIndex]) : FPoint();
}

void CPhotoCrosshairs::draw(CSurfaceArea *surfaceArea) {
	if (!_positions.empty()) {
		uint savedPixel = surfaceArea->_pixel;
		surfaceArea->_pixel = 0xff;
		surfaceArea->setColorFromPixel();
		SurfaceAreaMode savedMode = surfaceArea->setMode(SA_NONE);

		for (int idx = 0; idx < _entryIndex; ++idx) {
			const CStarPosition &src = _entries[idx];
			double xp = src.x, yp = src.y;

			surfaceArea->fillRect(FRect(xp - 8.0, yp, xp - 4.0, yp));
			surfaceArea->fillRect(FRect(xp + 4.0, yp, xp + 8.0, yp));
			surfaceArea->fillRect(FRect(xp, yp - 8.0, xp, yp - 4.0));
			surfaceArea->fillRect(FRect(xp, yp + 4.0, xp, yp + 8.0));
		}

		surfaceArea->_pixel = savedPixel;
		surfaceArea->setColorFromPixel();
		surfaceArea->setMode(savedMode);
	}
}

void CPhotoCrosshairs::allocate(int count) {
	if (!_positions.empty()) {
		if ((int)_positions.size() == count)
			return;

		clear();
	}

	_positions.resize(count);
}

void CPhotoCrosshairs::clear() {
	_positions.clear();
	_matchIndex = _entryIndex = -1;
}

int CPhotoCrosshairs::indexOf(const Common::Point &pt) const {
	Common::Rect r(pt.x - 2, pt.y - 2, pt.x + 2, pt.y + 2);

	for (int idx = 0; idx < (int)_positions.size(); ++idx) {
		if (r.contains(_positions[idx]))
			return idx;
	}

	return -1;
}

void CPhotoCrosshairs::drawStar(int index, CSurfaceArea *surfaceArea) {
	if (index >= 0 && index < (int)_positions.size()) {
		const CStarPosition &pt = _positions[index];
		drawAt(pt, surfaceArea);
	}
}

void CPhotoCrosshairs::drawEntry(int index, CVideoSurface *surface, CStarField *starField, CStarMarkers *markers) {
	surface->lock();
	CSurfaceArea surfaceArea(surface);
	drawAt(_entries[index], &surfaceArea);
	surface->unlock();

	const CBaseStarEntry *starP = starField->getDataPtr(_entries[index]._index1);
	markers->addStar(starP);
}

void CPhotoCrosshairs::drawCurrent(CSurfaceArea *surfaceArea) {
	assert(_entryIndex >= 0);
	const CStarPosition &pt = _entries[_entryIndex];
	drawAt(pt, surfaceArea);
}

void CPhotoCrosshairs::drawAt(const FPoint &pt, CSurfaceArea *surfaceArea) {
	uint savedPixel = surfaceArea->_pixel;
	surfaceArea->_pixel = 255;
	surfaceArea->setColorFromPixel();
	SurfaceAreaMode savedMode = surfaceArea->setMode(SA_MODE3);


	surfaceArea->fillRect(FRect(pt._x - 8.0, pt._y, pt._x - 4.0, pt._y));
	surfaceArea->fillRect(FRect(pt._x - -4.0, pt._y, pt._x + 8.0, pt._y));
	surfaceArea->fillRect(FRect(pt._x, pt._y - 8.0, pt._x, pt._y - 4.0));
	surfaceArea->fillRect(FRect(pt._x, pt._y + 4.0, pt._x, pt._y + 8.0));

	surfaceArea->_pixel = savedPixel;
	surfaceArea->setColorFromPixel();
	surfaceArea->setMode(savedMode);
}

} // End of namespace Titanic
