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

#ifndef SCUMM_HE_BASKETBALL_COLLISION_BBALL_COLLISION_OBJECT_H
#define SCUMM_HE_BASKETBALL_COLLISION_BBALL_COLLISION_OBJECT_H

#ifdef ENABLE_HE

#include "common/str.h"

#include "scumm/he/basketball/collision/bball_collision_support_obj.h"
#include "scumm/he/basketball/collision/bball_collision_stack.h"

namespace Scumm {

#define COLLISION_EPSILON               0.5F
#define COLLISION_SMALL_TIME_INCREMENT  0.05F
#define COLLISION_BACK_OUT_TIME_LIMIT   20.0F

class CCollisionSphere;
class CCollisionBox;
class CCollisionCylinder;
class CCollisionObjectStack;
class CCollisionObjectVector;

enum EObjectShape {
	kNoObjectShape = 0,
	kSphere = 1,
	kBox = 2,
	kCylinder = 3
};

enum EObjectType {
	kNoObjectType = 0,
	kBackboard = 1,
	kRim = 2,
	kOtherType = 3,
	kFloor = 4,
	kBall = 5,
	kPlayer = 6
};

class ICollisionObject {
public:
	ICollisionObject(EObjectShape shape = kNoObjectShape);
	virtual ~ICollisionObject() {}

	// Comparison operator
	bool operator==(const ICollisionObject &otherObject) const;

	// Get the distance between the outside of this object and the outside of a target object
	virtual float getObjectDistance(const ICollisionObject &targetObject) const;
	virtual float getObjectDistance(const CCollisionSphere &targetObject) const { return 0.0F; }
	virtual float getObjectDistance(const CCollisionBox &targetObject) const { return 0.0F; }
	virtual float getObjectDistance(const CCollisionCylinder &targetObject) const { return 0.0F; }

	// testObjectIntersection is used to determine if this object is intersecting a target object
	virtual bool testObjectIntersection(const ICollisionObject &targetObject, U32Distance3D *distance) const;
	virtual bool testObjectIntersection(const CCollisionSphere &targetObject, U32Distance3D *distance) const { return false; }
	virtual bool testObjectIntersection(const CCollisionBox &targetObject, U32Distance3D *distance) const { return false; }
	virtual bool testObjectIntersection(const CCollisionCylinder &targetObject, U32Distance3D *distance) const { return false; }

	// Just because this object is intersecting another, doesn't mean that it collided with
	// that object. That object could have collided with us. This function verifies, that
	// we collided with another object...
	virtual bool validateCollision(const ICollisionObject &targetObject, U32Distance3D *distance);
	virtual bool validateCollision(const CCollisionSphere &targetObject, U32Distance3D *distance) { return true; }
	virtual bool validateCollision(const CCollisionBox &targetObject, U32Distance3D *distance) { return true; }
	virtual bool validateCollision(const CCollisionCylinder &targetObject, U32Distance3D *distance) { return true; }

	// backOutOfObject moves this object backwards along its velocity vector to a point
	// where it is guaranteed not to be intersecting a target object
	virtual bool backOutOfObject(const ICollisionObject &targetObject, U32Distance3D *distance, float *timeUsed);
	virtual bool backOutOfObject(const CCollisionSphere &targetObject, U32Distance3D *distance, float *timeUsed) { return true; }
	virtual bool backOutOfObject(const CCollisionBox &targetObject, U32Distance3D *distance, float *timeUsed) { return true; }
	virtual bool backOutOfObject(const CCollisionCylinder &targetObject, U32Distance3D *distance, float *timeUsed) { return true; }

	// nudgeObject moves this object forward along its velocity vector to a point
	// where it is guaranteed to be touching the target object at the exact point
	// of collision
	virtual bool nudgeObject(const ICollisionObject &targetObject, U32Distance3D *distance, float *timeUsed);
	virtual bool nudgeObject(const CCollisionSphere &targetObject, U32Distance3D *distance, float *timeUsed) { return true; }
	virtual bool nudgeObject(const CCollisionBox &targetObject, U32Distance3D *distance, float *timeUsed) { return true; }
	virtual bool nudgeObject(const CCollisionCylinder &targetObject, U32Distance3D *distance, float *timeUsed) { return true; }

	// Some collisions between objects are recorded, but not handled physically. This
	// function determines whether a collision gets handled or not...
	virtual bool isCollisionHandled(const ICollisionObject &targetObject) const;
	virtual bool isCollisionHandled(const CCollisionSphere &targetObject) const { return true; }
	virtual bool isCollisionHandled(const CCollisionBox &targetObject) const { return true; }
	virtual bool isCollisionHandled(const CCollisionCylinder &targetObject) const { return true; }

	// handleCollisions alters this objects velocity and position as a result of
	// a collision with one or more target objects
	virtual void handleCollisions(CCollisionObjectVector *pCollisionVector, float *timeUsed, bool advanceObject);
	virtual void handleCollision(const CCollisionSphere &targetObject, float *timeUsed, U32Distance3D *distance, bool advanceObject) {}
	virtual void handleCollision(const CCollisionBox &targetObject, float *timeUsed, U32Distance3D *distance, bool advanceObject) {}
	virtual void handleCollision(const CCollisionCylinder &targetObject, float *timeUsed, U32Distance3D *distance, bool advanceObject) {}

	// Determine if this object is resting, running, or rolling on another object.
	// An object doing this would not necessarily trigger an intersection...
	virtual bool isOnObject(const ICollisionObject &targetObject, const U32Distance3D &distance) const;
	virtual bool isOnObject(const CCollisionSphere &targetObject, const U32Distance3D &distance) const { return false; }
	virtual bool isOnObject(const CCollisionBox &targetObject, const U32Distance3D &distance) const { return false; }
	virtual bool isOnObject(const CCollisionCylinder &targetObject, const U32Distance3D &distance) const { return false; }

	// Return a 3-dimensional bounding box for this object
	virtual U32BoundingBox getBoundingBox() const = 0;
	virtual U32BoundingBox getBigBoundingBox() const = 0;

	// Return the point on the surface of the object that is closest to the input point
	virtual U32FltPoint3D findNearestPoint(const U32FltPoint3D &testPoint) const;

	// Call this function when it is known that the object is at a legal location and is
	// not intersecting any other objects
	virtual void save() {}

	// Put the object at the last place that it was guaranteed to not be intersecting any
	// other objects, and negate it's velocity vector
	virtual void restore() {}

	Common::String _description;
	EObjectShape _objectShape;
	EObjectType _objectType;
	int _objectID; // An identifier that uniquely identifies this object

	U32FltVector3D _velocity;

	float _collisionEfficiency; // A multiplier (range 0 to 1) that represents
								// the percentage of velocity (perpendicular with
								// the collision) that a target object retains
								// when it collides with this object

	float _friction; // A multiplier (range 0 to 1) that represents
					 // the percentage of velocity (parallel with
					 // the collision) that a target object loses
					 // when it collides with this object

	int _soundNumber;

	bool _ignore;  // There could be times when you want to ignore
				   // an object for collision purposes. This flag
				   // determines that.

	CCollisionObjectVector _objectCollisionHistory; // Stack with pointers to all the
													// objects that this object collided
													// with this frame

	CCollisionObjectVector _objectRollingHistory; // Stack with pointers to all the
												  // objects that this object rolled
												  // on this frame

protected:
	// Get the amount of time it took this sphere to penetrate whatever object it is
	// currently intersecting in the given dimension
	virtual float getPenetrationTime(const ICollisionObject &targetObject, const U32Distance3D &distance, EDimension dimension) const;
	virtual float getPenetrationTime(const CCollisionSphere &targetObject, const U32Distance3D &distance, EDimension dimension) const { return 0.0F; }
	virtual float getPenetrationTime(const CCollisionBox &targetObject, const U32Distance3D &distance, EDimension dimension) const { return 0.0F; }
	virtual float getPenetrationTime(const CCollisionCylinder &targetObject, const U32Distance3D &distance, EDimension dimension) const { return 0.0F; }

	// Sometimes when this object collides with another object, it may want to treat
	// the object at the point of collision like a plane. This can help simplify the
	// collision. defineReflectionPlane defines a plane between this object and the
	// target object at the point of collision...
	virtual void defineReflectionPlane(const ICollisionObject &targetObject, const U32Distance3D &distance, U32Plane *collisionPlane) const;
	virtual void defineReflectionPlane(const CCollisionSphere &targetObject, const U32Distance3D &distance, U32Plane *collisionPlane) const;
	virtual void defineReflectionPlane(const CCollisionBox &targetObject, const U32Distance3D &distance, U32Plane *collisionPlane) const;
	virtual void defineReflectionPlane(const CCollisionCylinder &targetObject, const U32Distance3D &distance, U32Plane *collisionPlane) const;
};

} // End of namespace Scumm

#endif // ENABLE_HE

#endif // SCUMM_HE_BASKETBALL_COLLISION_BBALL_COLLISION_OBJECT_H
