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

#include "titanic/star_control/unmarked_camera_mover.h"
#include "titanic/star_control/dmatrix.h"
#include "titanic/star_control/dvector.h"
#include "titanic/titanic.h"
#include "common/textconsole.h"

namespace Titanic {

CUnmarkedCameraMover::CUnmarkedCameraMover(const CNavigationInfo *src) :
		CCameraMover(src) {
}

void CUnmarkedCameraMover::moveTo(const FVector &srcV, const FVector &destV, const FMatrix &orientation) {
	if (isLocked())
		decLockCount();

	debugC(DEBUG_BASIC, kDebugStarfield, "Starfield move %s to %s", srcV.toString().c_str(),
		destV.toString().c_str());
	_autoMover.setPath(srcV, destV, orientation);
}

void CUnmarkedCameraMover::proc10(const FVector &v1, const FVector &v2, const FVector &v3, const FMatrix &m) {
	if (isLocked())
		decLockCount();

	DVector vector1 = v1;
	DVector vector2 = v2;
	DMatrix matrix1 = vector2.fn4(vector1);
	DMatrix matrix2 = matrix1.fn4(m);

	_autoMover.proc3(m, matrix2);
	incLockCount();
}

void CUnmarkedCameraMover::updatePosition(CErrorCode &errorCode, FVector &pos, FMatrix &orientation) {
	if (_autoMover.isActive()) {
		decLockCount();
		int val = _autoMover.proc5(errorCode, pos, orientation);
		if (val == 1)
			incLockCount();
		if (val == 2) {
			stop();
			if (_starVector)
				_starVector->apply();
		}
	} else if (_speed != 0.0) {
		pos._x += orientation._row3._x * _speed;
		pos._y += orientation._row3._y * _speed;
		pos._z += orientation._row3._z * _speed;
		errorCode.set();
	}
}

} // End of namespace Titanic
