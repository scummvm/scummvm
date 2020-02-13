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

#include "titanic/star_control/motion_control_unmarked.h"
#include "titanic/star_control/base_stars.h"
#include "titanic/star_control/fpose.h"
#include "titanic/star_control/error_code.h"
#include "titanic/star_control/fmatrix.h"
#include "titanic/debugger.h"
#include "titanic/titanic.h"

namespace Titanic {

CMotionControlUnmarked::CMotionControlUnmarked(const CNavigationInfo *src) :
		CMotionControl(src) {
}

void CMotionControlUnmarked::moveTo(const FVector &srcV, const FVector &destV, const FMatrix &orientation) {
	if (isLocked())
		decLockCount();

	debugC(DEBUG_BASIC, kDebugStarfield, "Starfield move %s to %s", srcV.toString().c_str(),
		destV.toString().c_str());
	_autoMover.setPathOrient(srcV, destV, orientation);
}

// TODO: v3 is unused
void CMotionControlUnmarked::transitionBetweenOrientations(const FVector &v1, const FVector &v2, const FVector &v3, const FMatrix &m) {
	if (isLocked())
		decLockCount();

	FVector vector1 = v1;
	FVector vector2 = v2;
	FPose matrix1 = vector2.getFrameTransform(vector1);
	FPose matrix2 = matrix1.compose(m);

	_autoMover.setOrientations(m, matrix2);
	incLockCount();
}

void CMotionControlUnmarked::updatePosition(CErrorCode &errorCode, FVector &pos, FMatrix &orientation) {
	if (_autoMover.isActive()) {
		decLockCount();
		MoverState moverState = _autoMover.move(errorCode, pos, orientation);
		if (moverState == MOVING)
			incLockCount();
		if (moverState == DONE_MOVING) {
			stop();
			if (_callback)
				_callback->apply();
		}
	} else if (_currVelocity != 0.0) {
		pos._x += orientation._row3._x * _currVelocity;
		pos._y += orientation._row3._y * _currVelocity;
		pos._z += orientation._row3._z * _currVelocity;
		errorCode.set();
	}
}

} // End of namespace Titanic
