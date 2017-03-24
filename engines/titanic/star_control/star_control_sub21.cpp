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

#include "titanic/star_control/star_control_sub21.h"
#include "titanic/star_control/dmatrix.h"
#include "titanic/star_control/dvector.h"
#include "common/textconsole.h"

namespace Titanic {

CStarControlSub21::CStarControlSub21(const CStar20Data *src) :
		CStarControlSub20(src) {
}

void CStarControlSub21::proc9(FVector &v1, FVector &v2, FMatrix &matrix) {
	if (isLocked())
		decLockCount();

	_sub24.proc4(v1, v2, matrix);
}

void CStarControlSub21::proc10(const FVector &v1, const FVector &v2, const FVector &v3, const FMatrix &m) {
	if (isLocked())
		decLockCount();

	DVector vector1 = v1;
	DVector vector2 = v2;
	DMatrix matrix1, matrix2 = m, matrix3;
	vector2.fn4(vector1, matrix1);
	FMatrix matrix4 = matrix1.fn4(matrix2);

	_sub24.proc3(m, matrix4);
	incLockCount();
}

void CStarControlSub21::proc11(CErrorCode &errorCode, FVector &v, FMatrix &m) {
	if (_sub24.get8()) {
		decLockCount();
		int val = _sub24.proc5(errorCode, v, m);
		if (val == 1)
			incLockCount();
		if (val == 2) {
			proc7();
			if (_starVector)
				_starVector->apply();
		}
	} else if (_size != 0.0) {
		v._x += m._row3._x * _size;
		v._y += m._row3._y * _size;
		v._z += m._row3._z * _size;
		errorCode.set();
	}
}

} // End of namespace Titanic
