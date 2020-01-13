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

#include "titanic/star_control/star_crosshairs.h"
#include "titanic/star_control/star_markers.h"
#include "titanic/star_control/camera.h"
#include "titanic/star_control/star_field.h"
#include "titanic/star_control/star_ref.h"
#include "titanic/support/simple_file.h"

namespace Titanic {

CStarCrosshairs::CStarCrosshairs() : _matchIndex(-1), _entryIndex(-1) {
}

void CStarCrosshairs::selectStar(int index, CVideoSurface *surface,
		CStarField *starField, CStarMarkers *markers) {
	if (_entryIndex >= 0) {
		// There are existing selected stars already
		if (_entryIndex == _matchIndex) {
			// All the stars selected so far have been matched. Only allow
			// a selection addition if not all three stars have been found
			if (!isSolved()) {
				// Don't allow the most recent match or the one before
				// it to be re-selected (while they are locked/matched)
				if (_positions[index] != _entries[_entryIndex]) {
					if (_entryIndex == 1) {
						// 2 stars are matched
						if (_positions[index] == _entries[_entryIndex - 1])
							return;
					}

					surface->lock();

					// Draw crosshairs around the selected star
					CSurfaceArea surfaceArea(surface);
					drawStar(index, &surfaceArea);
					surface->unlock();

					// Copy the star into the list of selected ones
					++_entryIndex;
					CStarPosition &newP = _entries[_entryIndex];
					newP = _positions[index];

					// Set up a marker in the main starfield for that same star
					const CBaseStarEntry *starP = starField->getDataPtr(newP._index1);
					markers->addStar(starP);
				}
			}
		} else if (_entryIndex == _matchIndex + 1) {
			// There is a most recently selected star that has not yet been matched.
			// So we allow the user to reselect it to remove the selection, or shift
			// the selection to some other star
			if (_positions[index] == _entries[_entryIndex]) {
				// Player has selected the most recent star
				// Remove the crosshairs for the previously selected star
				surface->lock();
				CSurfaceArea surfaceArea(surface);
				eraseCurrent(&surfaceArea);
				surface->unlock();

				// Decrement number of selections
				--_entryIndex;

				// Call the markers addStar method, which will remove the existing marker
				const CBaseStarEntry *starP = starField->getDataPtr(_positions[index]._index1);
				markers->addStar(starP);
			} else {
				// Player has selected some other star other than the most recent
				// Remove/Add it if it is not one of the other star(s) already matched

				// Check that it is not a previously star and don't remove it if it is
				for (int i = 0; i < _entryIndex; ++i) {
					if (_positions[index] == _entries[i])
						return;
				}

				// Erase the prior selection and draw the new one
				surface->lock();
				CSurfaceArea surfaceArea(surface);
				eraseCurrent(&surfaceArea);
				drawStar(index, &surfaceArea);
				surface->unlock();

				// Remove the old selection from the starfield markers
				const CBaseStarEntry *starP;
				starP = starField->getDataPtr(_entries[_entryIndex]._index1);
				markers->addStar(starP);

				// Add the new selection to the markers list
				starP = starField->getDataPtr(_positions[index]._index1);
				markers->addStar(starP);

				// Copy the newly selected star's details into our selections list
				CStarPosition &newP = _entries[_entryIndex];
				newP = _positions[index];
			}
		}
	} else {
		// Very first star being selected
		// Draw crosshairs around the selected star
		surface->lock();
		CSurfaceArea surfaceArea(surface);
		drawStar(index, &surfaceArea);
		surface->unlock();

		// Copy the star into the list of selected ones
		++_entryIndex;
		const CStarPosition &srcPos = _positions[index];
		CStarPosition &destPos = _entries[_entryIndex];
		destPos = srcPos;

		// Set up a marker in the main starfield for that same star
		const CBaseStarEntry *starP = starField->getDataPtr(destPos._index1);
		markers->addStar(starP);
	}
}

bool CStarCrosshairs::fn1(CStarField *starField, CSurfaceArea *surfaceArea, CCamera *camera) {
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

void CStarCrosshairs::decMatches(CVideoSurface *surface, CStarField *starField, CStarMarkers *markers) {
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

void CStarCrosshairs::incMatches() {
	if (_matchIndex < 3)
		++_matchIndex;
}

FPoint CStarCrosshairs::getPosition() const {
	return (_entryIndex >= 0 && _entryIndex <= 2) ? 
		FPoint(_entries[_entryIndex]) : FPoint();
}

void CStarCrosshairs::draw(CSurfaceArea *surfaceArea) {
	if (!_positions.empty()) {
		uint savedPixel = surfaceArea->_pixel;
		surfaceArea->_pixel = 0xff;
		surfaceArea->setColorFromPixel();
		SurfaceAreaMode savedMode = surfaceArea->setMode(SA_SOLID);

		for (int idx = 0; idx <= _entryIndex; ++idx) {
			const CStarPosition &src = _entries[idx];
			double xp = src.x, yp = src.y;

			surfaceArea->drawLine(FRect(xp - 8.0, yp, xp - 4.0, yp));
			surfaceArea->drawLine(FRect(xp + 4.0, yp, xp + 8.0, yp));
			surfaceArea->drawLine(FRect(xp, yp - 8.0, xp, yp - 4.0));
			surfaceArea->drawLine(FRect(xp, yp + 4.0, xp, yp + 8.0));
		}

		surfaceArea->_pixel = savedPixel;
		surfaceArea->setColorFromPixel();
		surfaceArea->setMode(savedMode);
	}
}

void CStarCrosshairs::allocate(int count) {
	if (!_positions.empty()) {
		if ((int)_positions.size() == count)
			return;

		clear();
	}

	_positions.resize(count);
}

void CStarCrosshairs::clear() {
	_positions.clear();
	_matchIndex = _entryIndex = -1;
}

int CStarCrosshairs::indexOf(const Common::Point &pt) const {
	Common::Rect r(pt.x - 2, pt.y - 2, pt.x + 2, pt.y + 2);

	for (int idx = 0; idx < (int)_positions.size(); ++idx) {
		if (r.contains(_positions[idx]))
			return idx;
	}

	return -1;
}

void CStarCrosshairs::drawStar(int index, CSurfaceArea *surfaceArea) {
	if (index >= 0 && index < (int)_positions.size()) {
		const CStarPosition &pt = _positions[index];
		drawAt(pt, surfaceArea);
	}
}

void CStarCrosshairs::drawEntry(int index, CVideoSurface *surface, CStarField *starField, CStarMarkers *markers) {
	surface->lock();
	CSurfaceArea surfaceArea(surface);
	drawAt(_entries[index], &surfaceArea);
	surface->unlock();

	const CBaseStarEntry *starP = starField->getDataPtr(_entries[index]._index1);
	markers->addStar(starP);
}

void CStarCrosshairs::eraseCurrent(CSurfaceArea *surfaceArea) {
	assert(_entryIndex >= 0);
	const CStarPosition &pt = _entries[_entryIndex];
	drawAt(pt, surfaceArea);
}

void CStarCrosshairs::drawAt(const FPoint &pt, CSurfaceArea *surfaceArea) {
	uint savedPixel = surfaceArea->_pixel;
	surfaceArea->_pixel = 255;
	surfaceArea->setColorFromPixel();
	SurfaceAreaMode savedMode = surfaceArea->setMode(SA_XOR);


	surfaceArea->drawLine(FRect(pt._x - 8.0, pt._y, pt._x - 4.0, pt._y));
	surfaceArea->drawLine(FRect(pt._x + 4.0, pt._y, pt._x + 8.0, pt._y));
	surfaceArea->drawLine(FRect(pt._x, pt._y - 8.0, pt._x, pt._y - 4.0));
	surfaceArea->drawLine(FRect(pt._x, pt._y + 4.0, pt._x, pt._y + 8.0));

	surfaceArea->_pixel = savedPixel;
	surfaceArea->setColorFromPixel();
	surfaceArea->setMode(savedMode);
}

} // End of namespace Titanic
