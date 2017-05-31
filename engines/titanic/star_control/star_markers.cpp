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

#include "titanic/star_control/star_markers.h"
#include "titanic/star_control/star_camera.h"

namespace Titanic {

void CStarMarkers::draw(CSurfaceArea *surfaceArea, CStarCamera *camera, CStarCloseup *closeup) {
	if (_data.empty())
		return;

	FPose pose = camera->getPose();
	double threshold = camera->getThreshold();
	FPoint center((double)surfaceArea->_width * 0.5,
		surfaceArea->_height * 0.5);
	FVector newV;
	double x1, x2, x3, y1, y2, y3;

	uint savedPixel = surfaceArea->_pixel;
	surfaceArea->_pixel = 0xffff;
	surfaceArea->setColorFromPixel();

	for (uint idx = 0; idx < _data.size(); ++idx) {
		const CBaseStarEntry &star = _data[idx];
		newV._x = pose._row1._x * star._position._x + pose._row3._x * star._position._z
			+ pose._row2._x * star._position._y + pose._vector._x;
		newV._y = pose._row1._y * star._position._x + pose._row3._y * star._position._z
			+ pose._row2._y * star._position._y + pose._vector._y;
		newV._z = pose._row1._z * star._position._x + pose._row3._z * star._position._z
			+ pose._row2._z * star._position._y + pose._vector._z;

		if (newV._z > threshold) {
			FVector vTemp = camera->proc28(2, newV);

			x2 = center._x + vTemp._x;
			y1 = center._y + vTemp._y;
			y2 = y1 + 4.0;
			x1 = x2 - 4.0;
			x3 = x2 + 4.0;
			y3 = y1 - 4.0;
			surfaceArea->drawLine(FPoint(x1, y1), FPoint(x2, y2));
			surfaceArea->drawLine(FPoint(x2, y2), FPoint(x3, y1));
			surfaceArea->drawLine(FPoint(x3, y1), FPoint(x2, y3));
			surfaceArea->drawLine(FPoint(x2, y3), FPoint(x1, y1));
		}
	}

	surfaceArea->_pixel = savedPixel;
	surfaceArea->setColorFromPixel();
}

bool CStarMarkers::addStar(const CBaseStarEntry *entry) {
	// iterate through the existing stars
	for (uint idx = 0; idx < _data.size(); ++idx) {
		CBaseStarEntry &star = _data[idx];
		if (star == *entry) {
			// Found a matching star at the exact same position, so remove it instead
			_data.remove_at(idx);
			return true;
		}
	}

	// No existing match
	if (_data.size() == 32)
		// Out of space, so delete oldest star
		_data.remove_at(0);

	// Add new star
	_data.push_back(*entry);
	return true;
}

} // End of namespace Titanic
