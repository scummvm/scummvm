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

#include "access/polygon.h"

namespace Access {

namespace Polygon {

bool pointInside(const int16 (*points)[2], int16 x, int16 y) {
	bool result = false;
	uint j = 0;
	while (points[j][0] >= 0 && points[j][1] >= 0)
		j++;
	for (uint k = 0; points[k][0] >= 0 && points[k][1] >= 0; k++) {
		if ((points[k][1] <= y && y < points[j][1]) ||
			(points[j][1] <= y && y < points[k][1])) {
			if (x < (points[j][0] - points[k][0]) * (y - points[k][1]) /
			  (points[j][1] - points[k][1]) + points[k][0]) {
			  result = !result;
			}
		}
		j = k;
	}
	return result;
}

} // end namespace Polygon

} // end namespace Access
