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

#ifndef SCUMM_HE_BASKETBALL_COLLISION_BBALL_COLLISION_CYLINDER_H
#define SCUMM_HE_BASKETBALL_COLLISION_BBALL_COLLISION_CYLINDER_H

#ifdef ENABLE_HE

#include "scumm/he/basketball/collision/bball_collision_object.h"
#include "scumm/he/basketball/collision/bball_collision_stack.h"

namespace Scumm {

#define MAX_STEP_HEIGHT 50
	
// An object's velocity can either be carried
// out as straight or circular movement...
enum EMovementType {
	kStraight = 0,
	kCircular = 1 // 2D circular motion along the horizon plane
};

class CCollisionCylinder : public ICollisionObject, public U32Cylinder {
public:
	CCollisionCylinder() : ICollisionObject(kCylinder),
						   height(0),
						   _shieldRadius(0),
						   _movementType(EMovementType::kStraight),
						   _revCenter(nullptr),
						   _positionSaved(false) {}

	~CCollisionCylinder() {}

	using ICollisionObject::getObjectDistance;
	float getObjectDistance(const CCollisionSphere &targetObject) const override;
	float getObjectDistance(const CCollisionBox &targetObject) const override;
	float getObjectDistance(const CCollisionCylinder &targetObject) const override;

	using ICollisionObject::testObjectIntersection;
	bool testObjectIntersection(const CCollisionSphere &targetObject, U32Distance3D *distance) const override;
	bool testObjectIntersection(const CCollisionBox &targetObject, U32Distance3D *distance) const override;
	bool testObjectIntersection(const CCollisionCylinder &targetObject, U32Distance3D *distance) const override;

	using ICollisionObject::validateCollision;
	bool validateCollision(const CCollisionCylinder &targetObject, U32Distance3D *distance) override;

	using ICollisionObject::backOutOfObject;
	bool backOutOfObject(const CCollisionBox &targetObject, U32Distance3D *distance, float *timeUsed) override;
	bool backOutOfObject(const CCollisionCylinder &targetObject, U32Distance3D *distance, float *timeUsed) override;

	using ICollisionObject::nudgeObject;
	bool nudgeObject(const CCollisionBox &targetObject, U32Distance3D *distance, float *timeUsed) override;
	bool nudgeObject(const CCollisionCylinder &targetObject, U32Distance3D *distance, float *timeUsed) override;

	using ICollisionObject::isCollisionHandled;
	bool isCollisionHandled(const CCollisionSphere &targetObject) const override { return false; };

	void handleCollisions(CCollisionObjectVector *collisionVector, float *timeUsed, bool advanceObject) override;
	void handleCollision(const CCollisionCylinder &targetCylinder, float *timeUsed, U32Distance3D *distance, bool advanceObject) override;
	void handleCollision(const CCollisionBox &targetBox, float *pTimeUsed, U32Distance3D *distance, bool advanceObject) override;

	using ICollisionObject::isOnObject;
	bool isOnObject(const CCollisionSphere &targetObject, const U32Distance3D &distance) const override;
	bool isOnObject(const CCollisionBox &targetObject, const U32Distance3D &distance) const override;
	bool isOnObject(const CCollisionCylinder &targetObject, const U32Distance3D &distance) const override;

	U32FltPoint3D findNearestPoint(const U32FltPoint3D &testPoint) const override;
	int getEquidistantPoint(U32FltPoint2D inPoint1, float distance1, U32FltPoint2D inPoint2, float distance2, U32FltPoint2D *outPoint1, U32FltPoint2D *outPoint2);

	U32BoundingBox getBoundingBox() const override;
	U32BoundingBox getBigBoundingBox() const override;

	void save() override;
	void restore() override;

	float height;
	float _shieldRadius;  // Sometimes we may want ot temporarily increase a player's
						  // effective collision radius.  This value keeps track of
						  // how much their radius has been increased by.

	EMovementType _movementType;

	const ICollisionObject *_revCenter;   // If an object is moving with circular motion,
										  // this is the object it is revolving around.
	U32FltPoint2D _revCenterPt;           // This is the center of the _revCenter.

protected:
	float getDimensionDistance(const CCollisionBox &targetObject, EDimension dimension) const;
	float getDimensionDistance(const CCollisionSphere &targetObject, EDimension dimension) const;
	float getDimensionDistance(const CCollisionCylinder &targetObject, EDimension dimension) const;

private:
	bool _positionSaved;
	U32FltPoint3D _safetyPoint;
	U32FltVector3D _safetyVelocity;

	bool backStraightOutOfObject(const ICollisionObject &targetObject, U32Distance3D *distance, float *timeUsed);

	bool circleOutOfObject(const CCollisionBox &targetObject, U32Distance3D *distance, float *timeUsed);
	bool circleOutOfObject(const CCollisionCylinder &targetObject, U32Distance3D *distance, float *timeUsed);

	ERevDirection getRevDirection() const;

	using ICollisionObject::getPenetrationTime;
	float getPenetrationTime(const CCollisionBox &targetObject, const U32Distance3D &distance, EDimension dimension) const override;
	float getPenetrationTime(const CCollisionCylinder &targetObject, const U32Distance3D &distance, EDimension dimension) const override;

	float getPointOfCollision(const CCollisionBox &targetBox, U32Distance3D distance, EDimension dimension) const;

	void forceOutOfObject(const ICollisionObject &targetObject, U32Distance3D *distance);

	bool getCornerIntersection(const CCollisionBox &targetObject, const U32Distance3D &distance, U32FltPoint2D *intersection);
};

} // End of namespace Scumm

#endif // ENABLE_HE

#endif // SCUMM_HE_BASKETBALL_COLLISION_BBALL_COLLISION_CYLINDER_H
