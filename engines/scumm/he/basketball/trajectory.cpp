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
#include "scumm/he/basketball/basketball.h"
#include "scumm/he/basketball/collision/bball_collision_support_obj.h"

namespace Scumm {

int LogicHEBasketball::u32_userComputeTrajectoryToTarget(const U32FltPoint3D &sourcePoint, const U32FltPoint3D &targetPoint, int speed) {
	U32FltVector3D trajectory; // The final trajectory of the object
	float xDist, yDist, zDist; // The distance that the object will move in xyz space
	float hDist, totalDist;
	float hAngle, vAngle;      // The angle of trajectory from the x axis and from the xy plane
	float hSpeed;              // The speed of the object along the xy plane

	xDist = targetPoint.x - sourcePoint.x;
	yDist = targetPoint.y - sourcePoint.y;
	zDist = targetPoint.z - sourcePoint.z;

	hDist = sqrt((xDist * xDist) + (yDist * yDist));
	totalDist = sqrt((hDist * hDist) + (zDist * zDist));

	if (totalDist < speed)
		speed = (int)(totalDist + 0.5F);

	hAngle = atan2(yDist, xDist);
	vAngle = atan2(zDist, totalDist);

	hSpeed = speed * cos(vAngle);

	trajectory.x = hSpeed * cos(hAngle);
	trajectory.y = hSpeed * sin(hAngle);
	trajectory.z = speed * sin(vAngle);

	writeScummVar(_vm1->VAR_U32_USER_VAR_A, _vm->_basketball->u32FloatToInt(trajectory.x));
	writeScummVar(_vm1->VAR_U32_USER_VAR_B, _vm->_basketball->u32FloatToInt(trajectory.y));
	writeScummVar(_vm1->VAR_U32_USER_VAR_C, _vm->_basketball->u32FloatToInt(trajectory.z));

	return 1;
}

int LogicHEBasketball::u32_userComputeLaunchTrajectory(const U32FltPoint2D &sourcePoint, const U32FltPoint2D &targetPoint, int launchAngle, int iVelocity) {
	U32FltVector3D trajectory; // The final trajectory of the object
	float xDist, yDist;        // The distance that the object will move in xyz space
	float vAngle;              // The angle of trajectory from the x axis and from the xy plane
	float hAngle;
	float hVelocity;           // The speed of the object along the xy plane

	xDist = targetPoint.x - sourcePoint.x;
	yDist = targetPoint.y - sourcePoint.y;

	hAngle = atan2(yDist, xDist);
	vAngle = (launchAngle * BBALL_M_PI) / 180;

	hVelocity = iVelocity * cos(vAngle);

	trajectory.x = hVelocity * cos(hAngle);
	trajectory.y = hVelocity * sin(hAngle);
	trajectory.z = iVelocity * sin(vAngle);

	writeScummVar(_vm1->VAR_U32_USER_VAR_A, _vm->_basketball->u32FloatToInt(trajectory.x));
	writeScummVar(_vm1->VAR_U32_USER_VAR_B, _vm->_basketball->u32FloatToInt(trajectory.y));
	writeScummVar(_vm1->VAR_U32_USER_VAR_C, _vm->_basketball->u32FloatToInt(trajectory.z));

	return 1;
}

int LogicHEBasketball::u32_userComputeAngleBetweenVectors(const U32FltVector3D &vector1, const U32FltVector3D &vector2) {
	float radiansCosine;
	float radians;
	float angle;

	if ((vector1.magnitude() * vector2.magnitude()) == 0) {
		angle = 0;
	} else {
		radiansCosine = (vector1 * vector2) / (vector1.magnitude() * vector2.magnitude());

		if (radiansCosine > 1) {
			radiansCosine = 1;
		} else if (radiansCosine < -1) {
			radiansCosine = -1;
		}

		radians = acos(radiansCosine);
		angle = (radians * 180) / BBALL_M_PI;
	}

	writeScummVar(_vm1->VAR_U32_USER_VAR_A, _vm->_basketball->u32FloatToInt(angle));

	return 1;
}

} // End of namespace Scumm
