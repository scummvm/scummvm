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

#ifndef SCUMM_HE_BASKETBALL_COLLISION_BBALL_COLLISION_SPHERE_H
#define SCUMM_HE_BASKETBALL_COLLISION_BBALL_COLLISION_SPHERE_H

#ifdef ENABLE_HE

#include "scumm/he/basketball/collision/bball_collision_object.h"
#include "scumm/he/basketball/collision/bball_collision_stack.h"

namespace Scumm {

const int BOUNCE_SPEED_LIMIT = 20;      // If the z-direction speed of the ball falls below this limit during a bounce, then the ball will begin to roll.
const int ROLL_SPEED_LIMIT = 20;        // If the xy-direction speed of the ball falls below this limit during a roll, then the ball will stop.
const int ROLL_SLOWDOWN_FREQUENCY = 10; // The ball slows down every x frames while rolling.

class CCollisionSphere : public ICollisionObject, public U32Sphere {
public:
	CCollisionSphere() : ICollisionObject(kSphere),
						 _rollingCount(0),
						 _positionSaved(false) {}

	~CCollisionSphere() {}

	using ICollisionObject::getObjectDistance;
	float getObjectDistance(const CCollisionBox &targetObject) const override;
	float getObjectDistance(const CCollisionCylinder &targetObject) const override;

	using ICollisionObject::testObjectIntersection;
	bool testObjectIntersection(const CCollisionBox &targetObject, U32Distance3D *pDistance) const override;
	bool testObjectIntersection(const CCollisionCylinder &targetObject, U32Distance3D *pDistance) const override;

	using ICollisionObject::validateCollision;
	bool validateCollision(const CCollisionBox &targetObject, U32Distance3D *pDistance) override;
	bool validateCollision(const CCollisionCylinder &targetObject, U32Distance3D *pDistance) override;

	using ICollisionObject::backOutOfObject;
	bool backOutOfObject(const CCollisionBox &targetObject, U32Distance3D *pDistance, float *pTimeUsed) override;
	bool backOutOfObject(const CCollisionCylinder &targetObject, U32Distance3D *pDistance, float *pTimeUsed) override;

	using ICollisionObject::nudgeObject;
	bool nudgeObject(const CCollisionBox &targetObject, U32Distance3D *pDistance, float *pTimeUsed) override;
	bool nudgeObject(const CCollisionCylinder &targetObject, U32Distance3D *pDistance, float *pTimeUsed) override;

	void handleCollisions(CCollisionObjectVector *pCollisionVector, float *pTimeUsed, bool advanceObject) override;

	using ICollisionObject::isOnObject;
	bool isOnObject(const CCollisionBox &targetObject, const U32Distance3D &distance) const override;
	bool isOnObject(const CCollisionCylinder &targetObject, const U32Distance3D &distance) const override;

	U32BoundingBox getBoundingBox() const override;
	U32BoundingBox getBigBoundingBox() const override;

	void save() override;
	void restore() override;

	int _rollingCount;


protected:
	float getDimensionDistance(const CCollisionBox &targetObject, EDimension dimension) const;
	float getDimensionDistance(const CCollisionCylinder &targetObject, EDimension dimension) const;

private:
	bool _positionSaved;
	U32FltPoint3D _safetyPoint;
	U32FltVector3D _safetyVelocity;

	bool backStraightOutOfObject(const ICollisionObject &targetObject, U32Distance3D *pDistance, float *pTimeUsed);

	using ICollisionObject::defineReflectionPlane;
	void defineReflectionPlane(const CCollisionBox &targetObject, const U32Distance3D &distance, U32Plane *collisionPlane) const override;
	void defineReflectionPlane(const CCollisionCylinder &targetObject, const U32Distance3D &distance, U32Plane *collisionPlane) const override;

	void reboundOffPlane(const U32Plane &collisionPlane, bool isOnPlane);

	using ICollisionObject::getPenetrationTime;
	float getPenetrationTime(const CCollisionBox &targetObject, const U32Distance3D &distance, EDimension dimension) const override;
	float getPenetrationTime(const CCollisionCylinder &targetObject, const U32Distance3D &distance, EDimension dimension) const override;

	void increaseVelocity(float minSpeed);
};


} // End of namespace Scumm

#endif // ENABLE_HE

#endif // SCUMM_HE_BASKETBALL_COLLISION_BBALL_COLLISION_SPHERE_H
