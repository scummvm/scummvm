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

#include "scumm/he/intern_he.h"

#include "scumm/he/basketball/collision/bball_collision_support_obj.h"
#include "scumm/he/basketball/collision/bball_collision.h"
#include "scumm/he/basketball/court.h"
#include "scumm/he/basketball/geo_translations.h"
#include "scumm/he/basketball/shooting.h"

namespace Scumm {

int LogicHEBasketball::u32_userComputeInitialShotVelocity(int theta, int hDist, int vDist, int gravity) {
	double velocity;
	double trajectoryAngle;
	double targetAngle;

	assert(gravity > 0);

	if ((hDist == 0) && (vDist == 0)) {
		velocity = 0;
	} else {
		trajectoryAngle = (theta * BBALL_M_PI) / 180;

		// Find the angle between the horizon and the line between the source and the target...
		targetAngle = atan2(vDist, hDist);

		// If the shot is impossible, then return a 0 for velocity, otherwise calculate the
		// required velocity...
		if ((theta == 90) && (hDist == 0)) {
			velocity = sqrt(abs(vDist) * 2 * gravity);
		} else if (((targetAngle >= 0) && (targetAngle < (BBALL_M_PI / 2))) &&
				   ((trajectoryAngle <= targetAngle) || (trajectoryAngle >= (BBALL_M_PI / 2)))) {
			velocity = 0;
		} else if (((targetAngle >= (BBALL_M_PI / 2) && (targetAngle < BBALL_M_PI))) &&
				   ((trajectoryAngle >= targetAngle) || (trajectoryAngle <= (BBALL_M_PI / 2)))) {
			velocity = 0;
		} else if (((targetAngle >= -BBALL_M_PI && (targetAngle < -(BBALL_M_PI / 2)))) &&
				   ((trajectoryAngle >= targetAngle) || (trajectoryAngle <= (BBALL_M_PI / 2)))) {
			velocity = 0;
		} else if (((targetAngle >= -(BBALL_M_PI / 2) && (targetAngle < 0))) &&
				   ((trajectoryAngle <= targetAngle) || (trajectoryAngle >= (BBALL_M_PI / 2)))) {
			velocity = 0;
		} else {
			float numerator = gravity * hDist * hDist;
			float denominator = (2 * vDist * cos(trajectoryAngle) * cos(trajectoryAngle)) +
								(2 * hDist * sin(trajectoryAngle) * cos(trajectoryAngle));

			assert((numerator / denominator) >= 0);

			if ((numerator / denominator) < 0) {
				velocity = 0;
			} else {
				velocity = sqrt(numerator / denominator);
			}

			// Hack, because this is off somehow, or because to go in the hoop,
			// the ball should hit a little in front...
			velocity -= 4;
		}
	}

	writeScummVar(_vm1->VAR_U32_USER_VAR_A, _vm->_basketball->u32DoubleToInt(velocity));

	return 1;
}

static double getShotAngle(int hDist, int vDist) {
	double degrees;
	float hUnits;
	float a, b, c;

	assert(hDist >= 0);

	if (vDist == 0) {
		if (hDist == 0)
			degrees = MAX_SHOT_ANGLE;
		else
			degrees = MIN_SHOT_ANGLE;
	} else {
		hUnits = hDist / -(float)vDist;

		a = (float)((2.0 - 4.0) / ((TEN_FOOT_SHOT_ANGLE - MIN_SHOT_ANGLE) * 4.0 * 2.0 - 4.0 * (MAX_SHOT_ANGLE - MIN_SHOT_ANGLE)));
		b = a / ((MAX_SHOT_ANGLE - MIN_SHOT_ANGLE) * a - 1.0);
		c = MIN_SHOT_ANGLE;

		degrees = 1.0 / (a * (hUnits + 1.0) * (hUnits + 1.0)) + 1.0 / (b * (hUnits + 1.0)) + c;
	}

	return degrees;
}

int LogicHEBasketball::u32_userComputeAngleOfShot(int hDist, int vDist) {
	double degrees;

	degrees = getShotAngle(hDist, vDist);

	writeScummVar(_vm1->VAR_U32_USER_VAR_A, _vm->_basketball->u32DoubleToInt(degrees));

	return 1;
}

int LogicHEBasketball::u32_userComputeBankShotTarget(U32FltPoint3D basketLoc, int ballRadius) {
	U32FltPoint3D targetPoint;
	float backboardDist;
	CCollisionBox *backboard;

	if (basketLoc.x < (MAX_WORLD_X / 2)) {
		// Left basket...
		backboard = &(_vm->_basketball->_court->_objectList[_vm->_basketball->_court->_backboardIndex[LEFT_BASKET]]);
		backboardDist = basketLoc.x - backboard->maxPoint.x;
	} else {
		// Right basket...
		backboard = &(_vm->_basketball->_court->_objectList[_vm->_basketball->_court->_backboardIndex[RIGHT_BASKET]]);
		backboardDist = basketLoc.x - backboard->minPoint.x;
	}

	targetPoint.x = basketLoc.x - (2 * backboardDist);
	targetPoint.y = basketLoc.y;
	targetPoint.z = basketLoc.z + ballRadius;

	writeScummVar(_vm1->VAR_U32_USER_VAR_A, _vm->_basketball->u32FloatToInt(targetPoint.x));
	writeScummVar(_vm1->VAR_U32_USER_VAR_B, _vm->_basketball->u32FloatToInt(targetPoint.y));
	writeScummVar(_vm1->VAR_U32_USER_VAR_C, _vm->_basketball->u32FloatToInt(targetPoint.z));

	return 1;
}

int LogicHEBasketball::u32_userComputeSwooshTarget(const U32FltPoint3D &basketLoc, int ballRadius) {
	U32FltPoint3D targetPoint;

	targetPoint.x = basketLoc.x;
	targetPoint.y = basketLoc.y;
	targetPoint.z = basketLoc.z + ballRadius;

	writeScummVar(_vm1->VAR_U32_USER_VAR_A, _vm->_basketball->u32FloatToInt(targetPoint.x));
	writeScummVar(_vm1->VAR_U32_USER_VAR_B, _vm->_basketball->u32FloatToInt(targetPoint.y));
	writeScummVar(_vm1->VAR_U32_USER_VAR_C, _vm->_basketball->u32FloatToInt(targetPoint.z));

	return 1;
}

int LogicHEBasketball::u32_userDetectShotMade(const U32Sphere &basketball, const U32IntVector3D &ballVector, int gravity, int whichBasket) {
	int shotWasMade = 0;
	U32Distance3D distance;

	distance.x = basketball.center.x - _vm->_basketball->_court->_shotSpot[whichBasket].center.x;
	distance.y = basketball.center.y - _vm->_basketball->_court->_shotSpot[whichBasket].center.y;
	distance.z = basketball.center.z - _vm->_basketball->_court->_shotSpot[whichBasket].center.z;

	if (distance.magnitude() <= (_vm->_basketball->_court->_shotSpot[whichBasket].radius + basketball.radius)) {
		if (ballVector.z < 0) {
			shotWasMade = 1;
		}
	}

	writeScummVar(_vm1->VAR_U32_USER_VAR_A, shotWasMade);

	return 1;
}

} // End of namespace Scumm
