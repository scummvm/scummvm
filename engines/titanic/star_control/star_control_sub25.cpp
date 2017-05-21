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

#include "titanic/star_control/star_control_sub25.h"
#include "titanic/star_control/dmatrix.h"

namespace Titanic {

void CStarControlSub25::fn1(const FMatrix &m1, const FMatrix &m2) {
	_matrix1 = m1;
	_matrix2 = m2;

	DMatrix matrix = _matrix2;
	_sub1.fn4(matrix);
	matrix = _matrix2;
	_sub2.fn4(matrix);
}

void CStarControlSub25::fn2(double val, FMatrix &orientation) {
	if (val <= 0.0) {
		orientation = _matrix1;
	} else if (val > 1.0) {
		orientation = _matrix2;
	} else {
		CStarControlSub26 sub26 = _sub1.fn5(val, &_sub2);

		DMatrix m1;
		m1.fn3(&sub26);
		orientation = m1;
	}
}

} // End of namespace Titanic
