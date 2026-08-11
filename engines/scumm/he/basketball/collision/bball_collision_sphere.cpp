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
#include "scumm/he/basketball/collision/bball_collision_object.h"
#include "scumm/he/basketball/collision/bball_collision_sphere.h"
#include "scumm/he/basketball/collision/bball_collision_box.h"
#include "scumm/he/basketball/collision/bball_collision_cylinder.h"
#include "scumm/he/basketball/collision/bball_collision_stack.h"
#include "scumm/he/basketball/collision/bball_collision_node.h"
#include "scumm/he/basketball/collision/bball_collision_tree.h"

namespace Scumm {

float CCollisionSphere::getDimensionDistance(const CCollisionBox &targetObject, EDimension dimension) const {
	if (center[dimension] < targetObject.minPoint[dimension]) {
		return (center[dimension] - targetObject.minPoint[dimension]);
	} else if (center[dimension] > targetObject.maxPoint[dimension]) {
		return (center[dimension] - targetObject.maxPoint[dimension]);
	} else {
		return 0;
	}
}

float CCollisionSphere::getDimensionDistance(const CCollisionCylinder &targetObject, EDimension dimension) const {
	float centerDistance = center[dimension] - targetObject.center[dimension];

	if (dimension == Z_INDEX) {
		if (centerDistance < -(targetObject.height / 2)) {
			return (centerDistance + (targetObject.height / 2));
		} else if (centerDistance > (targetObject.height / 2)) {
			return (centerDistance - (targetObject.height / 2));
		} else {
			return 0;
		}
	} else {
		return centerDistance;
	}
}

float CCollisionSphere::getObjectDistance(const CCollisionBox &targetObject) const {
	U32Distance3D distance;

	distance.x = getDimensionDistance(targetObject, X_INDEX);
	distance.y = getDimensionDistance(targetObject, Y_INDEX);
	distance.z = getDimensionDistance(targetObject, Z_INDEX);

	float totalDistance = distance.magnitude() - radius;
	if (totalDistance < 0)
		totalDistance = 0;

	return totalDistance;
}

float CCollisionSphere::getObjectDistance(const CCollisionCylinder &targetObject) const {
	U32Distance3D distance;

	distance.x = getDimensionDistance(targetObject, X_INDEX);
	distance.y = getDimensionDistance(targetObject, Y_INDEX);
	distance.z = getDimensionDistance(targetObject, Z_INDEX);

	float xyDistance = distance.xyMagnitude() - radius - targetObject.radius;
	if (xyDistance < 0)
		xyDistance = 0;

	float zDistance = fabs(distance.z) - radius - (targetObject.height / 2);
	if (zDistance < 0)
		zDistance = 0;

	float totalDistance = sqrt((xyDistance * xyDistance) + (zDistance * zDistance));

	return totalDistance;
}

bool CCollisionSphere::testObjectIntersection(const CCollisionBox &targetObject, U32Distance3D *distance) const {
	// Get the distance between the ball and the bounding box...
	distance->x = getDimensionDistance(targetObject, X_INDEX);
	distance->y = getDimensionDistance(targetObject, Y_INDEX);
	distance->z = getDimensionDistance(targetObject, Z_INDEX);

	// Determine if the ball intersects the bounding box.
	return (distance->magnitude() < radius);
}

bool CCollisionSphere::testObjectIntersection(const CCollisionCylinder &targetObject, U32Distance3D *distance) const {
	// Get the distance between the ball and the cylinder...
	distance->x = getDimensionDistance(targetObject, X_INDEX);
	distance->y = getDimensionDistance(targetObject, Y_INDEX);
	distance->z = getDimensionDistance(targetObject, Z_INDEX);

	if (distance->xyMagnitude() < (radius + targetObject.radius)) {
		return (fabs(distance->z) < radius);
	} else {
		return false;
	}
}

bool CCollisionSphere::validateCollision(const CCollisionBox &targetObject, U32Distance3D *distance) {
	U32FltPoint3D targetPoint = targetObject.findNearestPoint(center);
	U32FltVector3D centerVector = targetPoint - center;
	float aMag = _velocity.magnitude();
	float bMag = centerVector.magnitude();
	float aDotb = _velocity * centerVector;

	if (aMag == 0) {
		// If this object isn't moving, this can't be a valid collision...
		return false;
	}

	if (bMag == 0) {
		// bMag is 0, it is possible that this sphere penetrated too far into the target
		// object. If this is the case, we'll go ahead and validate the collision...
		return true;
	}

	double angleCosine = aDotb / (aMag * bMag);

	if (angleCosine > 1) {
		angleCosine = 1;
	} else if (angleCosine < -1) {
		angleCosine = -1;
	}

	float sourceMovementAngle = acos(angleCosine);

	if (sourceMovementAngle < (BBALL_M_PI / 2)) {
		return true;
	} else {
		return false;
	}
}

bool CCollisionSphere::validateCollision(const CCollisionCylinder &targetObject, U32Distance3D *distance) {
	// Create a vector from the center of the target cylinder to the center of
	// this sphere. If the sphere is not hitting above or below the cylinder,
	// negate any z component of the center vector...
	U32FltVector3D centerVector = targetObject.center - center;

	if (center.z > (targetObject.center.z + targetObject.height / 2)) {
		centerVector.z = targetObject.center.z + (targetObject.height / 2) - center.z;
	} else if (center.z < (targetObject.center.z - targetObject.height / 2)) {
		centerVector.z = targetObject.center.z - (targetObject.height / 2) - center.z;
	} else if (_velocity.xyMagnitude() != 0) {
		centerVector.z = 0;
	}

	float aMag = _velocity.magnitude();
	float bMag = centerVector.magnitude();
	float aDotb = _velocity * centerVector;

	if (aMag == 0) {
		// If this object isn't moving, this can't be a valid collision...
		return false;
	}

	if (bMag == 0) {
		// bMag is 0, it is possible that this sphere penetrated too far into the target
		// object. If this is the case, we'll go ahead and validate the collision...
		return true;
	}

	// Calculate the angle between this object's velocity and the vector from the target
	// objects center to our center. If the angle between the two is greater than
	// 90 degrees, then the target object is not colliding with us...
	double angleCosine = aDotb / (aMag * bMag);

	if (angleCosine > 0) {
		return true;
	} else {
		return false;
	}
}

float CCollisionSphere::getPenetrationTime(const CCollisionBox &targetObject, const U32Distance3D &distance, EDimension dimension) const {
	float collisionDepth1;
	float collisionDepth2;

	float boxWidth = targetObject.maxPoint[dimension] - targetObject.minPoint[dimension];

	if (distance[dimension] > 0) {
		collisionDepth1 = (radius - distance[dimension]);
		collisionDepth2 = (-radius + distance[dimension] - boxWidth);
	} else if (distance[dimension] < 0) {
		collisionDepth1 = (-radius - distance[dimension]);
		collisionDepth2 = (radius - distance[dimension] + boxWidth);
	} else {
		return 0;
	}

	float time1 = (_velocity[dimension] == 0) ? 0 : collisionDepth1 / -_velocity[dimension];
	float time2 = (_velocity[dimension] == 0) ? 0 : collisionDepth2 / -_velocity[dimension];
	float tFinal = MIN_GREATER_THAN_ZERO(time1, time2);

	return tFinal;
}

float CCollisionSphere::getPenetrationTime(const CCollisionCylinder &targetObject, const U32Distance3D &distance, EDimension dimension) const {
	float collisionDepth;

	if (dimension == Z_INDEX) {
		if (distance[dimension] > 0) {
			collisionDepth = (radius - distance[dimension]);
		} else if (distance[dimension] < 0) {
			collisionDepth = (-radius - distance[dimension]);
		} else {
			collisionDepth = 0;
		}
	} else {
		if (distance[dimension] > 0) {
			collisionDepth = (radius + targetObject.radius - distance[dimension]);
		} else if (distance[dimension] < 0) {
			collisionDepth = (-radius - targetObject.radius - distance[dimension]);
		} else {
			collisionDepth = 0;
		}
	}

	float tFinal = (_velocity[dimension] == 0) ? 0 : (collisionDepth / -_velocity[dimension]);
	return tFinal;
}

bool CCollisionSphere::backStraightOutOfObject(const ICollisionObject &targetObject, U32Distance3D *distance, float *timeUsed) {
	if (_velocity.magnitude() == 0)
		return true;

	U32FltPoint3D startPosition = center;

	int loopCounter = 0;

	while (testObjectIntersection(targetObject, distance)) {
		float collisionTimes[3];

		collisionTimes[X_INDEX] = getPenetrationTime(targetObject, *distance, X_INDEX);
		collisionTimes[Y_INDEX] = getPenetrationTime(targetObject, *distance, Y_INDEX);
		collisionTimes[Z_INDEX] = getPenetrationTime(targetObject, *distance, Z_INDEX);

		Std::sort(collisionTimes, collisionTimes + Z_INDEX + 1);

		float collisionTime = COLLISION_SMALL_TIME_INCREMENT;
		if (collisionTimes[2] > 0)
			collisionTime = collisionTimes[2];
		if (collisionTimes[1] > 0)
			collisionTime = collisionTimes[1];
		if (collisionTimes[0] > 0)
			collisionTime = collisionTimes[0];

		*timeUsed += collisionTime;

		// If we take too long to back out, something is wrong.
		// Restore the object to an ok state...
		if ((*timeUsed > COLLISION_BACK_OUT_TIME_LIMIT) && (*timeUsed != collisionTime)) {
			warning("CCollisionSphere::backStraightOutOfObject(): It took too long for one object to back out of another.  Ignore and U32 will attempt to correct.");
			center = startPosition;
			restore();
			return false;
		}

		center.x -= (collisionTime * _velocity.x);
		center.y -= (collisionTime * _velocity.y);
		center.z -= (collisionTime * _velocity.z);

		// Make doubly sure we don't loop forever...
		if (++loopCounter > 500)
			return false;
	}

	return true;
}

bool CCollisionSphere::backOutOfObject(const CCollisionBox &targetObject, U32Distance3D *distance, float *timeUsed) {
	return backStraightOutOfObject(targetObject, distance, timeUsed);
}

bool CCollisionSphere::backOutOfObject(const CCollisionCylinder &targetObject, U32Distance3D *distance, float *timeUsed) {
	return backStraightOutOfObject(targetObject, distance, timeUsed);
}

bool CCollisionSphere::nudgeObject(const CCollisionBox &targetObject, U32Distance3D *distance, float *timeUsed) {
	// To nudge the sphere precisely against the box, we need to calculate when the
	// square root of the sum of the squared distances between the sphere and the three
	// planes equals the radius of the sphere. Here we will construct a quadratic
	// equation to solve for time....

	double a = 0;
	double b = 0;
	double c = -(radius * radius);

	for (int i = X_INDEX; i <= Z_INDEX; ++i) {
		EDimension dim = (EDimension)i;

		// If the ball is already within the boundaries of the box in a certain dimension,
		// we don't want to include that dimension in the equation...
		if ((*distance)[dim] != 0) {
			a += (_velocity[dim] * _velocity[dim]);
			b += (2 * _velocity[dim] * (*distance)[dim]);
			c += ((*distance)[dim] * (*distance)[dim]);
		}
	}

	if (((b * b) < (4 * a * c)) || (a == 0)) {
		warning("CCollisionSphere::nudgeObject(): Tried to use sqrt on a negative number");
		return false;
	}

	// Now we have two answer candidates. We want the smallest of the two that is
	// greater than 0...
	double t1 = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
	double t2 = (-b - sqrt(b * b - 4 * a * c)) / (2 * a);

	double tFinal = 0;
	if ((0 <= t1) && (t1 <= t2)) {
		tFinal = t1;
	} else if ((0 <= t2) && (t2 <= t1)) {
		tFinal = t2;
	}

	// Update the position of the ball...
	center.x += _velocity.x * tFinal;
	center.y += _velocity.y * tFinal;
	center.z += _velocity.z * tFinal;
	*timeUsed -= tFinal;

	testObjectIntersection(targetObject, distance);
	return true;
}

bool CCollisionSphere::nudgeObject(const CCollisionCylinder &targetObject, U32Distance3D *distance, float *timeUsed) {
	float collisionTimeFinal;

	U32FltVector3D xyVelocity = _velocity;
	xyVelocity.z = 0;

	// Find the distance between the two centers that would indicate an exact collision...
	float intersectionDist = radius + targetObject.radius;

	// Set up a vector pointing from the center of this sphere to the center of
	// the target cylinder...
	U32FltVector3D centerVector;
	centerVector.x = targetObject.center.x - center.x;
	centerVector.y = targetObject.center.y - center.y;

	float centerDistance = centerVector.xyMagnitude();

	if (centerDistance > intersectionDist) {
		// Project the center vector onto the velocity vector.
		// This is the distance along the velocity vector from the
		// center of the sphere to the point that is parallel with the center
		// of the cylinder...
		float parallelDistance = centerVector.projectScalar(xyVelocity);

		// Find the distance between the center of the target object to the point that
		// that is distance2 units along the the velocity vector...
		parallelDistance = ((centerDistance * centerDistance) >=
							(parallelDistance * parallelDistance))
							   ? parallelDistance
							   : COPY_SIGN(centerDistance, parallelDistance);

		// Make sure we don't try to sqrt by negative number...
		if (parallelDistance > centerDistance) {
			warning("CCollisionSphere::nudgeObject(): Tried to sqrt by negative number.");
			centerDistance = parallelDistance;
		}

		float perdistance = sqrt(centerDistance * centerDistance - parallelDistance * parallelDistance);

		// Now we need to find the point along the velocity vector where the distance 
		// between that point and the center of the target cylinder equals intersectionDist.
		// We calculate using distance 2, distance3 and intersectionDist...
		perdistance = ((intersectionDist * intersectionDist) >=
						(perdistance * perdistance))
						   ? perdistance
						   : COPY_SIGN(intersectionDist, perdistance);

		// Make sure we don't try to sqrt by negative number...
		if (perdistance > intersectionDist) {
			warning("CCollisionSphere::nudgeObject(): Tried to sqrt by negative number.");
			intersectionDist = perdistance;
		}

		float xyCollisionDist = parallelDistance - sqrt(intersectionDist * intersectionDist - perdistance * perdistance);
		collisionTimeFinal = (_velocity.xyMagnitude() == 0) ? 0 : (xyCollisionDist / _velocity.xyMagnitude());
	} else {
		collisionTimeFinal = -getPenetrationTime(targetObject, *distance, Z_INDEX);
	}

	center.x += collisionTimeFinal * _velocity.x;
	center.y += collisionTimeFinal * _velocity.y;
	center.z += collisionTimeFinal * _velocity.z;
	*timeUsed -= collisionTimeFinal;

	testObjectIntersection(targetObject, distance);

	// We may still have a little ways to go in the z direction...
	if (fabs(distance->z) >= (radius + COLLISION_EPSILON)) {
		collisionTimeFinal = -getPenetrationTime(targetObject, *distance, Z_INDEX);

		center.x += collisionTimeFinal * _velocity.x;
		center.y += collisionTimeFinal * _velocity.y;
		center.z += collisionTimeFinal * _velocity.z;
		*timeUsed -= collisionTimeFinal;

		testObjectIntersection(targetObject, distance);
	}

	return true;
}

void CCollisionSphere::defineReflectionPlane(const CCollisionBox &targetObject, const U32Distance3D &distance, U32Plane *collisionPlane) const {
	// Find the point of collision...
	collisionPlane->point = targetObject.findNearestPoint(center);

	// Find the normal of the collision plane...
	collisionPlane->normal.x = (distance.magnitude() == 0) ? 0 : distance.x / distance.magnitude();
	collisionPlane->normal.y = (distance.magnitude() == 0) ? 0 : distance.y / distance.magnitude();
	collisionPlane->normal.z = (distance.magnitude() == 0) ? 0 : distance.z / distance.magnitude();

	collisionPlane->friction = targetObject._friction;
	collisionPlane->collisionEfficiency = targetObject._collisionEfficiency;
}

void CCollisionSphere::defineReflectionPlane(const CCollisionCylinder &targetObject, const U32Distance3D &distance, U32Plane *collisionPlane) const {
	// Find the point of collision...
	collisionPlane->point = targetObject.findNearestPoint(center);

	// Find the normal of the collision plane...
	collisionPlane->normal.x = (distance.magnitude() == 0) ? 0 : distance.x / distance.magnitude();
	collisionPlane->normal.y = (distance.magnitude() == 0) ? 0 : distance.y / distance.magnitude();
	collisionPlane->normal.z = (distance.magnitude() == 0) ? 0 : distance.z / distance.magnitude();

	collisionPlane->friction = targetObject._friction;
	collisionPlane->collisionEfficiency = targetObject._collisionEfficiency;
}

void CCollisionSphere::reboundOffPlane(const U32Plane &collisionPlane, bool isOnObject) {
	U32FltVector3D normalVector;
	U32FltVector3D reflectionVector;
	U32FltVector3D parallelVector;
	U32FltVector3D perpendicularVector;

	// Calculate the reflection vector...
	normalVector = _velocity.projectVector(collisionPlane.normal) * -1;
	reflectionVector = (normalVector * 2) + _velocity;

	// Break down the components of the refelction vector that are perpendicular
	// and parallel to the collision plane. This way we can account for drag and
	// inelastic collisions...
	perpendicularVector = normalVector;
	perpendicularVector *= collisionPlane.collisionEfficiency * _collisionEfficiency;

	parallelVector = reflectionVector - normalVector;

	if ((!isOnObject) || ((_rollingCount % ROLL_SLOWDOWN_FREQUENCY) == 0)) {
		parallelVector -= ((parallelVector * collisionPlane.friction) + (parallelVector * _friction));
	}

	// Now put the reflection vector back together again...
	reflectionVector = parallelVector + perpendicularVector;

	_velocity = reflectionVector;
}

void CCollisionSphere::increaseVelocity(float minSpeed) {
	if (_velocity.magnitude() < minSpeed) {
		// Make sure we're moving along the XY plane...
		if (_velocity.xyMagnitude() == 0) {
			// Randomly add some velocity...
			int random = (g_scumm->_rnd.getRandomNumber(0x7FFF) * 4) / 0x7FFF;
			switch (random) {
			case 0:
				_velocity.x = minSpeed;
				break;

			case 1:
				_velocity.x = -minSpeed;
				break;

			case 2:
				_velocity.y = minSpeed;
				break;

			case 3:
			case 4:
				_velocity.y = -minSpeed;
				break;

			default:
				warning("CCollisionSphere::increaseVelocity(): Invalid random number.");
			}
		} else {
			// Increase the current velocity...
			float oldVelocity = _velocity.magnitude();
			_velocity.x = (_velocity.x / oldVelocity) * minSpeed;
			_velocity.y = (_velocity.y / oldVelocity) * minSpeed;
			_velocity.z = (_velocity.z / oldVelocity) * minSpeed;
		}
	}
}

void CCollisionSphere::handleCollisions(CCollisionObjectVector *collisionVector, float *timeUsed, bool advanceObject) {
	Common::Array<U32Plane> planeVector;
	Common::Array<bool> rollingRecord;
	U32Plane collisionPlane;
	U32Distance3D distance;
	bool isRollingOnObject;

	for (CCollisionObjectVector::const_iterator objectIt = collisionVector->begin();
		 objectIt != collisionVector->end();
		 ++objectIt) {
		const ICollisionObject *currentObject = *objectIt;

		testObjectIntersection(*currentObject, &distance);

		// See if we are rolling on this object...
		isRollingOnObject = isOnObject(*currentObject, distance);
		if (isRollingOnObject) {
			// See if rolling has slowed to the point that we are stopped.
			// Never stop on the backboard or rim...
			if ((currentObject->_objectType == kBackboard) ||
				(currentObject->_objectType == kRim)) {
				increaseVelocity(ROLL_SPEED_LIMIT);
			} else {
				if (_velocity.xyMagnitude() < ROLL_SPEED_LIMIT) {
					_velocity.x = 0;
					_velocity.y = 0;
				}
			}

			_velocity.z = 0;
		} else {
			// See if rolling has slowed to the point that we are stopped.
			// Never stop on the backboard or rim...
			if ((currentObject->_objectType == kBackboard) ||
				(currentObject->_objectType == kRim)) {
				increaseVelocity(ROLL_SPEED_LIMIT);
			}
		}

		defineReflectionPlane(*currentObject, distance, &collisionPlane);
		planeVector.push_back(collisionPlane);
		rollingRecord.push_back(isRollingOnObject);
	}

	collisionVector->clear();

	if (!planeVector.empty()) {
		collisionPlane = planeVector[0];
		isRollingOnObject = rollingRecord[0];

		Common::Array<U32Plane>::const_iterator planeIt;
		Common::Array<bool>::const_iterator recordIt;

		for (planeIt = planeVector.begin(), recordIt = rollingRecord.begin();
			 planeIt != planeVector.end();
			 ++planeIt, ++recordIt) {

			collisionPlane.average(*planeIt);
			isRollingOnObject = isRollingOnObject || *recordIt;
		}

		reboundOffPlane(collisionPlane, isRollingOnObject);
	}

	if (advanceObject) {
		// Move the ball for the amount of time remaining...
		center.x += (_velocity.x * *timeUsed);
		center.y += (_velocity.y * *timeUsed);
		center.z += (_velocity.z * *timeUsed);

		*timeUsed = 0;
	}
}

bool CCollisionSphere::isOnObject(const CCollisionBox &targetObject, const U32Distance3D &distance) const {
	float distancePercentage = fabs(distance.z - radius) / radius;

	// See if bouncing has slowed to the point that we are rolling...
	return ((distancePercentage < .1) &&
			(distance.xyMagnitude() == 0) &&
			(fabs(_velocity.z) <= BOUNCE_SPEED_LIMIT));
}

bool CCollisionSphere::isOnObject(const CCollisionCylinder &targetObject, const U32Distance3D &distance) const {
	float distancePercentage = fabs(distance.z - radius) / radius;

	// See if bouncing has slowed to the point that we are rolling...
	return ((distancePercentage < .1) &&
			(distance.xyMagnitude() <= targetObject.radius) &&
			(fabs(_velocity.z) <= BOUNCE_SPEED_LIMIT));
}

U32BoundingBox CCollisionSphere::getBoundingBox() const {
	U32BoundingBox outBox;

	outBox.minPoint.x = center.x - radius;
	outBox.minPoint.y = center.y - radius;
	outBox.minPoint.z = center.z - radius;

	outBox.maxPoint.x = center.x + radius;
	outBox.maxPoint.y = center.y + radius;
	outBox.maxPoint.z = center.z + radius;

	return outBox;
}

U32BoundingBox CCollisionSphere::getBigBoundingBox() const {
	U32BoundingBox outBox;

	float velocity = _velocity.magnitude();

	outBox.minPoint.x = center.x - (radius + velocity);
	outBox.minPoint.y = center.y - (radius + velocity);
	outBox.minPoint.z = center.z - (radius + velocity);

	outBox.maxPoint.x = center.x + (radius + velocity);
	outBox.maxPoint.y = center.y + (radius + velocity);
	outBox.maxPoint.z = center.z + (radius + velocity);

	return outBox;
}

void CCollisionSphere::save() {
	_positionSaved = true;
	_safetyPoint = center;
	_safetyVelocity = _velocity;
}

void CCollisionSphere::restore() {
	if (_positionSaved) {
		if (_safetyVelocity.magnitude() != 0) {
			debug("CCollisionSphere::Restore(): Restoring");
			center = _safetyPoint;

			_velocity.x = 0;
			_velocity.y = 0;
			_velocity.z = 0;
		}
	} else {
		warning("CCollisionSphere::Restore(): No save point.");
	}
}

} // End of namespace Scumm
