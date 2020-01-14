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

#include "titanic/star_control/star_ref.h"
#include "titanic/star_control/camera.h"
#include "titanic/star_control/surface_area.h"

namespace Titanic {

void CBaseStarRef::process(CSurfaceArea *surface, CCamera *camera) {
	if (_stars->_data.empty())
		return;

	const double MAX_VAL = 1.0e9 * 1.0e9;
	FPose pose = camera->getPose();
	double threshold = camera->getFrontClip();
	double vWidth2 = (double)surface->_width * 0.5;
	double vHeight2 = (double)surface->_height * 0.5;
	FVector vTemp, vector1, vector2;
	double val1, green, blue, red;

	for (int idx = 0; idx < _stars->size(); ++idx) {
		const CBaseStarEntry &se = _stars->_data[idx];
		vTemp = se._position;
		vector1._x = vTemp._x * pose._row1._x + vTemp._y * pose._row2._x + vTemp._z * pose._row3._x + pose._vector._x;
		vector1._y = vTemp._x * pose._row1._y + vTemp._y * pose._row2._y + vTemp._z * pose._row3._y + pose._vector._y;
		vector1._z = vTemp._x * pose._row1._z + vTemp._y * pose._row2._z + vTemp._z * pose._row3._z + pose._vector._z;
		double hyp = vector1._x * vector1._x + vector1._y * vector1._y + vector1._z * vector1._z;

		if (vector1._z > threshold && hyp >= 1.0e12 && hyp < MAX_VAL) {
			vector2 = camera->getRelativePos(2, vector1);

			const Common::Point pt((int)(vector2._x + vWidth2 - -0.5),
				(int)(vector2._y + vHeight2 - -0.5));
			if (pt.y >= 0 && pt.y < (surface->_height - 1) &&
					pt.x >= 0 && pt.x < (surface->_width - 1)) {
				val1 = sqrt(hyp);
				if (val1 >= 100000.0)
					val1 = 1.0 - (val1 - 100000.0) / 1000000000.0;
				else
					val1 = 1.0;

				red = val1 * (double)se._red;
				green = val1 * (double)se._green;
				blue = val1 * (double)se._blue;

				int count = 0;
				if (red < 0.0)
					++count;
				if (green < 0.0)
					++count;
				if (blue < 0.0)
					++count;

				if (count < 3) {
					if (!check(pt, idx))
						break;
				}
			}
		}
	}
}

/*------------------------------------------------------------------------*/

bool CStarRef1::check(const Common::Point &pt, int index) {
	Common::Rect r(pt.x - 2, pt.y - 2, pt.x + 2, pt.y + 2);
	if (r.contains(_position)) {
		_index = index;
		return false;
	} else {
		return true;
	}
}

/*------------------------------------------------------------------------*/

bool CStarRefArray::check(const Common::Point &pt, int index) {
	if (_index >= (int)_positions->size())
		// Positions array full, so ignore
		return false;

	CStarPosition &sp = (*_positions)[_index++];
	sp.x = pt.x;
	sp.y = pt.y;
	sp._index1 = sp._index2 = index;

	return true;
}

/*------------------------------------------------------------------------*/

bool CStarRef3::check(const Common::Point &pt, int index) {
	++_index;
	return true;
}

} // End of namespace Titanic
