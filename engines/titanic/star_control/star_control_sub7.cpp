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

#include "titanic/star_control/star_control_sub7.h"
#include "titanic/star_control/star_control_sub12.h"

namespace Titanic {

void CStarControlSub7::draw(CSurfaceArea *surfaceArea, CStarControlSub12 *sub12, CStarControlSub5 *sub5) {
	if (_data.empty())
		return;

	CStarControlSub6 sub6 = sub12->proc23();
	double threshold = sub12->proc25();
	FPoint center((double)surfaceArea->_width * 0.5,
		surfaceArea->_height * 0.5);
	FVector newV;

	uint savedPixel = surfaceArea->_pixel;
	surfaceArea->_pixel = 0xffff;
	surfaceArea->setColorFromPixel();

	for (uint idx = 0; idx < _data.size(); ++idx) {
		const CBaseStarEntry &star = _data[idx];

		newV._x = sub6._row1._x * star._position._x + sub6._row3._x * star._position._z
			+ sub6._row2._x * star._position._y + sub6._vector._x;
		newV._y = sub6._row1._y * star._position._x + sub6._row3._y * star._position._z
			+ sub6._row2._y * star._position._x + sub6._vector._y;
		newV._z = sub6._row1._z * star._position._x + sub6._row3._z * star._position._z
			+ sub6._row2._z * star._position._y + sub6._vector._z; 

		if (newV._z > threshold) {
			FVector vTemp;
			sub12->proc28(2, newV, vTemp);

			FRect r1(center._x + vTemp._x, center._y + vTemp._y,
				center._x + vTemp._x + 4.0, center._y + vTemp._y + 4.0);
			surfaceArea->fillRect(r1);

			FRect r2(r1.right, r1.bottom, r1.right + 4.0, r1.top);
			surfaceArea->fillRect(r2);

			FRect r3(r2.right, r1.top, r1.right, r1.top - 4.0);
			surfaceArea->fillRect(r3);

			FRect r4(r1.right, r1.top - 4.0, r1.left, r1.top);
			surfaceArea->fillRect(r4);
		}
	}

	surfaceArea->_pixel = savedPixel;
	surfaceArea->setColorFromPixel();
}

bool CStarControlSub7::addStar(const CBaseStarEntry *entry) {
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
