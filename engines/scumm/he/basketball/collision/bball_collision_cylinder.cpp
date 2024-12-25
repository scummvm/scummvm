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

#include "scumm/he/basketball/collision/bball_collision.h"
#include "scumm/he/basketball/collision/bball_collision_support_obj.h"
#include "scumm/he/basketball/collision/bball_collision_object.h"
#include "scumm/he/basketball/collision/bball_collision_sphere.h"
#include "scumm/he/basketball/collision/bball_collision_box.h"
#include "scumm/he/basketball/collision/bball_collision_cylinder.h"
#include "scumm/he/basketball/collision/bball_collision_stack.h"
#include "scumm/he/basketball/collision/bball_collision_node.h"
#include "scumm/he/basketball/collision/bball_collision_tree.h"

namespace Scumm {

float CCollisionCylinder::getDimensionDistance(const CCollisionBox &targetObject, EDimension dimension) const {
	if (center[dimension] < targetObject.minPoint[dimension]) {
		return (center[dimension] - targetObject.minPoint[dimension]);
	} else if (center[dimension] > targetObject.maxPoint[dimension]) {
		return (center[dimension] - targetObject.maxPoint[dimension]);
	} else {
		return 0;
	}
}

float CCollisionCylinder::getDimensionDistance(const CCollisionSphere &targetObject, EDimension dimension) const {
	float centerDistance = center[dimension] - targetObject.center[dimension];

	if (dimension == Z_INDEX) {
		if (centerDistance < -(targetObject.radius)) {
			return (centerDistance + targetObject.radius);
		} else if (centerDistance > (targetObject.radius)) {
			return (centerDistance - targetObject.radius);
		} else {
			return 0;
		}
	} else {
		return centerDistance;
	}
}

float CCollisionCylinder::getDimensionDistance(const CCollisionCylinder &targetObject, EDimension dimension) const {
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

float CCollisionCylinder::getObjectDistance(const CCollisionSphere &targetObject) const {
	U32Distance3D distance;

	distance.x = getDimensionDistance(targetObject, X_INDEX);
	distance.y = getDimensionDistance(targetObject, Y_INDEX);
	distance.z = getDimensionDistance(targetObject, Z_INDEX);

	float xyDistance = distance.xyMagnitude() - radius - targetObject.radius;
	if (xyDistance < 0)
		xyDistance = 0;

	float zDistance = fabs(distance.z) - (height / 2) - targetObject.radius;
	if (zDistance < 0)
		zDistance = 0;

	float totalDistance = sqrt((xyDistance * xyDistance) + (zDistance * zDistance));
	return (totalDistance);
}

float CCollisionCylinder::getObjectDistance(const CCollisionBox &targetObject) const {
	U32Distance3D distance;

	distance.x = getDimensionDistance(targetObject, X_INDEX);
	distance.y = getDimensionDistance(targetObject, Y_INDEX);
	distance.z = getDimensionDistance(targetObject, Z_INDEX);

	float xyDistance = distance.xyMagnitude() - radius;
	if (xyDistance < 0)
		xyDistance = 0;

	float zDistance = fabs(distance.z) - (height / 2);
	if (zDistance < 0)
		zDistance = 0;

	float totalDistance = sqrt((xyDistance * xyDistance) + (zDistance * zDistance));
	return (totalDistance);
}

float CCollisionCylinder::getObjectDistance(const CCollisionCylinder &targetObject) const {
	U32Distance3D distance;

	distance.x = getDimensionDistance(targetObject, X_INDEX);
	distance.y = getDimensionDistance(targetObject, Y_INDEX);
	distance.z = getDimensionDistance(targetObject, Z_INDEX);

	float xyDistance = distance.xyMagnitude() - radius - targetObject.radius;
	if (xyDistance < 0)
		xyDistance = 0;

	float zDistance = fabs(distance.z) - (height / 2) - (targetObject.height / 2);
	if (zDistance < 0)
		zDistance = 0;

	float totalDistance = sqrt((xyDistance * xyDistance) + (zDistance * zDistance));
	return (totalDistance);
}

bool CCollisionCylinder::testObjectIntersection(const CCollisionSphere &targetObject, U32Distance3D *distance) const {
	// Get the distance between the ball and the cylinder...
	distance->x = getDimensionDistance(targetObject, X_INDEX);
	distance->y = getDimensionDistance(targetObject, Y_INDEX);
	distance->z = getDimensionDistance(targetObject, Z_INDEX);

	if (distance->xyMagnitude() < (radius + targetObject.radius)) {
		return (fabs(distance->z) < (height / 2));
	} else {
		return false;
	}
}

bool CCollisionCylinder::testObjectIntersection(const CCollisionBox &targetObject, U32Distance3D *distance) const {
	// Get the distance between the ball and the cylinder...
	distance->x = getDimensionDistance(targetObject, X_INDEX);
	distance->y = getDimensionDistance(targetObject, Y_INDEX);
	distance->z = getDimensionDistance(targetObject, Z_INDEX);
	;

	if (distance->xyMagnitude() < radius) {
		return (fabs(distance->z) < (height / 2));
	} else {
		return false;
	}
}

bool CCollisionCylinder::testObjectIntersection(const CCollisionCylinder &targetObject, U32Distance3D *distance) const {
	// Get the distance between the ball and the cylinder...
	distance->x = getDimensionDistance(targetObject, X_INDEX);
	distance->y = getDimensionDistance(targetObject, Y_INDEX);
	distance->z = getDimensionDistance(targetObject, Z_INDEX);

	if (distance->xyMagnitude() < (radius + targetObject.radius)) {
		return (fabs(distance->z) < (height / 2));
	} else {
		return false;
	}
}

float CCollisionCylinder::getPenetrationTime(const CCollisionBox &targetObject, const U32Distance3D &distance, EDimension dimension) const {
	float collisionDepth;

	if (dimension == Z_INDEX) {
		if (distance[dimension] > 0) {
			collisionDepth = (height / 2) - distance[dimension];
		} else if (distance[dimension] < 0) {
			collisionDepth = -(height / 2) - distance[dimension];
		} else {
			collisionDepth = 0;
		}
	} else {
		if (distance[dimension] > 0) {
			collisionDepth = radius - distance[dimension];
		} else if (distance[dimension] < 0) {
			collisionDepth = -radius - distance[dimension];
		} else {
			return 0;
		}
	}

	float tFinal = (_velocity[dimension] == 0) ? 0 : (collisionDepth / -_velocity[dimension]);

	return tFinal;
}

float CCollisionCylinder::getPenetrationTime(const CCollisionCylinder &targetObject, const U32Distance3D &distance, EDimension dimension) const {
	float collisionDepth;

	if (dimension == Z_INDEX) {
		if (distance[dimension] > 0) {
			collisionDepth = (height / 2) - distance[dimension];
		} else if (distance[dimension] < 0) {
			collisionDepth = (height / 2) + distance[dimension];
		} else {
			collisionDepth = 0;
		}
	} else {
		if (distance[dimension] > 0) {
			collisionDepth = radius + targetObject.radius - distance[dimension];
		} else if (distance[dimension] < 0) {
			collisionDepth = -radius - targetObject.radius - distance[dimension];
		} else {
			collisionDepth = 0;
		}
	}

	float tFinal = (_velocity[dimension] == 0) ? 0 : (collisionDepth / -_velocity[dimension]);

	return tFinal;
}

bool CCollisionCylinder::validateCollision(const CCollisionCylinder &targetObject, U32Distance3D *distance) {
	float zCollisionTime = getPenetrationTime(targetObject, *distance, Z_INDEX);

	if (((zCollisionTime > 1) || (zCollisionTime == 0)) &&
		(_velocity.xyMagnitude() == 0) &&
		(_velocity.z != 0)) {
		forceOutOfObject(targetObject, distance);
		return _ignore;
	} else {
		// If a player raises their shields and another player is standing inside
		// the shields, that player should be pushed out...
		if ((_velocity.magnitude() == 0) &&
			(_shieldRadius == 0) &&
			(targetObject._shieldRadius != 0)) {
			return true;
		} else {
			// See if we're hitting the top or bottom of this...
			if (((distance->z > 0) && (_velocity.z <= 0)) ||
				((distance->z < 0) && (_velocity.z >= 0))) {
				return true;
			} else {
				// Create a vector from the center of the target cylinder to the center of
				// this cylinder...
				U32FltVector2D centerVector = targetObject.center - center;

				float aMag = _velocity.magnitude();
				float bMag = centerVector.magnitude();
				float aDotb = centerVector * _velocity;

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

				if (angleCosine > 0) {
					return true;
				} else {
					return false;
				}
			}
		}
	}
}

bool CCollisionCylinder::backOutOfObject(const CCollisionBox &targetObject, U32Distance3D *distance, float *timeUsed) {
	// See if this is an object we can simply step up onto...
	if (((height / 2) - distance->z) <= MAX_STEP_HEIGHT) {
		center.z += ((height / 2) - distance->z);
		return true;
	} else if (_movementType == kCircular) {
		// Since we are moving with a circular motion, try circling out of the object...
		if (circleOutOfObject(targetObject, distance, timeUsed)) {
			return true;
		} else {
			// If we aren't able to circle out of the object, back out in a straight line...
			_movementType = kStraight;
			return backStraightOutOfObject(targetObject, distance, timeUsed);
		}
	} else {
		_movementType = kStraight;
		return backStraightOutOfObject(targetObject, distance, timeUsed);
	}
}

bool CCollisionCylinder::backOutOfObject(const CCollisionCylinder &targetObject, U32Distance3D *distance, float *timeUsed) {
	if (_velocity.magnitude() == 0) {
		forceOutOfObject(targetObject, distance);
		return true;
	} else if ((_movementType == kCircular) && (&targetObject != _revCenter)) {
		// Since we are moving with a circular motion, try circling out of the object...
		if (circleOutOfObject(targetObject, distance, timeUsed)) {
			return true;
		} else {
			// If we aren't able to circle out of the object, back out in a straight line...
			_movementType = kStraight;
			return backStraightOutOfObject(targetObject, distance, timeUsed);
		}
	} else {
		_movementType = kStraight;
		return backStraightOutOfObject(targetObject, distance, timeUsed);
	}
}

bool CCollisionCylinder::backStraightOutOfObject(const ICollisionObject &targetObject, U32Distance3D *distance, float *timeUsed) {
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

		// If we take too long to back out, something is wrong,
		// so restore the object to an ok state...
		if ((*timeUsed > COLLISION_BACK_OUT_TIME_LIMIT) && (*timeUsed != collisionTime)) {
			warning("CCollisionCylinder::backStraightOutOfObject(): It took too long for one object to back out of another.  Ignore and U32 will attempt to correct.");
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

bool CCollisionCylinder::getCornerIntersection(const CCollisionBox &targetObject, const U32Distance3D &distance, U32FltPoint2D *intersection) {
	float centerDistance = (center - _revCenterPt).xyMagnitude();
	U32BoundingBox testBox = targetObject;
	U32FltPoint2D boxCorner;

	// Get the corner that we think we've collided with...
	if (distance.x < 0) {
		boxCorner.x = targetObject.minPoint.x;
		testBox.maxPoint.x += centerDistance;
	} else if (distance.x > 0) {
		boxCorner.x = targetObject.maxPoint.x;
		testBox.minPoint.x -= centerDistance;
	} else {
		return false;
	}

	if (distance.y < 0) {
		boxCorner.y = targetObject.minPoint.y;
		testBox.maxPoint.y += centerDistance;
	} else if (distance.y > 0) {
		boxCorner.y = targetObject.maxPoint.y;
		testBox.maxPoint.y -= centerDistance;
	} else {
		return false;
	}

	U32FltPoint2D point1;
	U32FltPoint2D point2;

	int pointCount = getEquidistantPoint(boxCorner, radius,
										 _revCenterPt, centerDistance,
										 &point1,
										 &point2);

	switch (pointCount) {
	case 0:
		return false;
		break;

	case 1:
		*intersection = point1;
		break;

	case 2:
		if (!testBox.isPointWithin(point1)) {
			*intersection = point1;
		} else if (!testBox.isPointWithin(point2)) {
			*intersection = point2;
		} else {
			return false;
		}
		break;
	}

	return true;
}

bool CCollisionCylinder::circleOutOfObject(const CCollisionBox &targetObject, U32Distance3D *distance, float *timeUsed) {
	bool validXIntercept = true;
	bool validYIntercept = true;

	// If this object isn't moving, push it out of the collision...
	if (_velocity.xyMagnitude() == 0) {
		forceOutOfObject(targetObject, distance);
		testObjectIntersection(targetObject, distance);
		return false;
	}

	// Keep track of where we started...
	U32FltPoint3D oldCenter = center;

	// Get the distance between the center of this cylinder and the point it is
	// revolving around...
	float centerDistance = (center - _revCenterPt).xyMagnitude();

	// Find out in which direction we are going to revolve around the target point...
	ERevDirection revDirection = (getRevDirection() == kClockwise) ? kCounterClockwise : kClockwise;

	// Find where this cylinder would touch the box in the x dimension...
	U32FltPoint2D xIntercept;
	if (distance->y < 0) {
		xIntercept.y = targetObject.minPoint.y - radius;
		float yDist = xIntercept.y - _revCenterPt.y;

		float xInterceptSquared = centerDistance * centerDistance - yDist * yDist;

		if (xInterceptSquared < 0) {
			validXIntercept = getCornerIntersection(targetObject,
													*distance,
													&xIntercept);
		} else {
			if (revDirection == kClockwise) {
				xIntercept.x = sqrt(xInterceptSquared) + _revCenterPt.x;
			} else {
				xIntercept.x = -sqrt(xInterceptSquared) + _revCenterPt.x;
			}
		}
	} else if (distance->y > 0) {
		xIntercept.y = targetObject.maxPoint.y + radius;
		float yDist = xIntercept.y - _revCenterPt.y;

		float xInterceptSquared = centerDistance * centerDistance - yDist * yDist;

		if (xInterceptSquared < 0) {
			validXIntercept = getCornerIntersection(targetObject,
													*distance,
													&xIntercept);
		} else {
			if (revDirection == kClockwise) {
				xIntercept.x = -sqrt(xInterceptSquared) + _revCenterPt.x;
			} else {
				xIntercept.x = sqrt(xInterceptSquared) + _revCenterPt.x;
			}
		}
	} else {
		validXIntercept = false;
	}

	// If we found a valid place to back out to, try going there...
	if (validXIntercept) {
		center.x = xIntercept.x;
		center.y = xIntercept.y;

		forceOutOfObject(targetObject, distance);
		testObjectIntersection(targetObject, distance);
		return true;
	}

	// Find where this cylinder would touch the box in the y dimension...
	U32FltPoint2D yIntercept;
	if (distance->x < 0) {
		yIntercept.x = targetObject.minPoint.x - radius;
		float xDist = yIntercept.x - _revCenterPt.x;

		float yInterceptSquared = centerDistance * centerDistance - xDist * xDist;

		if (yInterceptSquared < 0) {
			validYIntercept = getCornerIntersection(targetObject,
													*distance,
													&yIntercept);
		} else {
			if (revDirection == kClockwise) {
				yIntercept.y = -sqrt(yInterceptSquared) + _revCenterPt.y;
			} else {
				yIntercept.y = sqrt(yInterceptSquared) + _revCenterPt.y;
			}
		}
	} else if (distance->x > 0) {
		yIntercept.x = targetObject.maxPoint.x + radius;
		float xDist = yIntercept.x - _revCenterPt.x;

		float yInterceptSquared = centerDistance * centerDistance - xDist * xDist;

		if (yInterceptSquared < 0) {
			validYIntercept = getCornerIntersection(targetObject,
													*distance,
													&yIntercept);
		} else {
			if (revDirection == kClockwise) {
				yIntercept.y = sqrt(yInterceptSquared) + _revCenterPt.y;
			} else {
				yIntercept.y = -sqrt(yInterceptSquared) + _revCenterPt.y;
			}
		}
	} else {
		validYIntercept = false;
	}

	// If we found a valid place to back out to, try going there...
	if (validYIntercept) {
		center.x = yIntercept.x;
		center.y = yIntercept.y;

		forceOutOfObject(targetObject, distance);
		testObjectIntersection(targetObject, distance);
		return true;
	}

	// If we get here, we weren't able to circle out...
	center = oldCenter;
	testObjectIntersection(targetObject, distance);
	return false;
}

bool CCollisionCylinder::circleOutOfObject(const CCollisionCylinder &targetObject, U32Distance3D *distance, float *timeUsed) {
	// Get the distance from the target cylinder that we want to be...
	float targetDistance = radius + targetObject.radius;

	// Get the distance from the revolution center that we want to be...
	U32FltVector2D sourceVector = center - _revCenterPt;
	float revDistance = sourceVector.magnitude();

	// Find out in which direction we are going to revolve around the target point...
	ERevDirection revDirection = (getRevDirection() == kClockwise) ? kCounterClockwise : kClockwise;

	U32FltPoint2D point1;
	U32FltPoint2D point2;

	int points = getEquidistantPoint(targetObject.center, targetDistance,
									 _revCenterPt, revDistance,
									 &point1,
									 &point2);

	switch (points) {
	case 0:
		warning("CCollisionCylinder::circleOutOfObject(): Could not find point of intersection.");
		return false;
		break;

	case 1:
		center.x = point1.x;
		center.y = point1.y;
		break;

	case 2:
		U32FltVector2D vector1 = point1 - _revCenterPt;
		U32FltVector2D vector2 = point2 - _revCenterPt;

		ERevDirection direction1 = (sourceVector.getRevDirection(vector1));
		ERevDirection direction2 = (sourceVector.getRevDirection(vector2));

		if (direction1 == direction2) {
			warning("CCollisionCylinder::circleOutOfObject(): Both directions are the same.  That's weird.");
		} else if (direction1 == revDirection) {
			center.x = point1.x;
			center.y = point1.y;
		} else {
			center.x = point2.x;
			center.y = point2.y;
		}
		break;
	}

	testObjectIntersection(targetObject, distance);
	forceOutOfObject(targetObject, distance);

	return true;
}

bool CCollisionCylinder::nudgeObject(const CCollisionBox &targetObject, U32Distance3D *distance, float *timeUsed) {
	float tFinal = 0;

	// To nudge the cylinder precisely against the box, we need to calculate when the
	// square root of the sum of the squared distances between the sphere and the three
	// planes equals the radius of the sphere.
	// Here we will construct a quadratic equation to solve for time.

	double a = 0;
	double b = 0;
	double c = -(radius * radius);

	// See if we are standing on the object...
	if ((distance->z == (height / 2)) &&
		(distance->xyMagnitude() == 0)) {
		return true;
	}

	// Only do calculations for the x and y dimensions, as we will handle z separately...
	for (int i = X_INDEX; i <= Y_INDEX; ++i) {
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
		tFinal = -getPenetrationTime(targetObject, *distance, Z_INDEX);

		assert(tFinal >= 0);
	} else {
		// Now we have two answer candidates, and we want the smallest
		// of the two that is greater than 0...
		double t1 = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
		double t2 = (-b - sqrt(b * b - 4 * a * c)) / (2 * a);

		double tXY = 0;
		if ((0 <= t1) && (t1 <= t2)) {
			tXY = t1;
		} else if ((0 <= t2) && (t2 <= t1)) {
			tXY = t2;
		}

		float tZ = -getPenetrationTime(targetObject, *distance, Z_INDEX);
		tFinal = ((0 < tZ) && (tZ < tXY)) ? tZ : tXY;
	}

	// Update the position of the ball...
	center.x += _velocity.x * tFinal;
	center.y += _velocity.y * tFinal;
	center.z += _velocity.z * tFinal;
	*timeUsed -= tFinal;

	testObjectIntersection(targetObject, distance);
	return true;
}

bool CCollisionCylinder::nudgeObject(const CCollisionCylinder &targetObject, U32Distance3D *distance, float *timeUsed) {
	float collisionTimeFinal;

	U32FltVector3D xyVelocity = _velocity;
	xyVelocity.z = 0;

	// Find the distance between the two centers that would indicate an exact collision...
	float intersectionDist = radius + targetObject.radius;

	// Set up a vector pointing from the center of this sphere to the
	// center of the target cylinder...
	U32FltVector3D centerVector;
	centerVector.x = targetObject.center.x - center.x;
	centerVector.y = targetObject.center.y - center.y;

	float centerDistance = centerVector.xyMagnitude();

	if (centerDistance > intersectionDist) {
		// Project the center vector onto the velocity vector.
		// This is the distance along the velocity vector from the center
		// of the sphere to the point that is parallel with the center
		// of the cylinder...
		float parallelDistance = centerVector.projectScalar(xyVelocity);

		// Find the distance between the center of the target object to the
		// point that that is distance2 units along the the velocity vector...
		parallelDistance = ((centerDistance * centerDistance) >=
							(parallelDistance * parallelDistance))
							   ? parallelDistance
							   : COPY_SIGN(centerDistance, parallelDistance);

		// Make sure we don't try to sqrt by negative number...
		if (parallelDistance > centerDistance) {
			warning("CCollisionCylinder::nudgeObject(): Tried to sqrt by negative number.");
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
			warning("CCollisionCylinder::nudgeObject(): Tried to sqrt by negative number.");
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
	return true;
}

float CCollisionCylinder::getPointOfCollision(const CCollisionBox &targetBox, U32Distance3D distance, EDimension dimension) const {
	if (dimension == Z_INDEX) {
		if ((distance[dimension] + (height / 2)) < 0) {
			return targetBox.minPoint[dimension];
		} else if ((distance[dimension] - (height / 2)) > 0) {
			return targetBox.maxPoint[dimension];
		} else {
			return center[dimension];
		}
	} else {
		if (distance[dimension] < 0) {
			return targetBox.minPoint[dimension];
		} else if (distance[dimension] > 0) {
			return targetBox.maxPoint[dimension];
		} else {
			return center[dimension];
		}
	}
}

ERevDirection CCollisionCylinder::getRevDirection() const {
	if (_movementType != kCircular)
		warning("CCollisionCylinder::getRevDirection(): We can't get a revolution direction if we aren't moving in a circular fashion.");

	U32FltVector3D targetVector = _revCenterPt - center;

	if ((_velocity.xyMagnitude() != 0) &&
		(targetVector.xyMagnitude() != 0)) {
		U32FltVector3D vector1 = targetVector;
		U32FltVector3D vector2 = _velocity;

		vector1.z = 0;
		vector2.z = 0;

		U32FltVector3D vector3 = vector1.cross(vector2);

		return (vector3.z > 0) ? kClockwise : kCounterClockwise;
	} else {
		warning("CCollisionCylinder::getRevDirection(): Division by zero attempted, ignoring...");
		return kNone;
	}
}

void CCollisionCylinder::forceOutOfObject(const ICollisionObject &targetObject, U32Distance3D *distance) {
	U32FltVector3D targetVector;
	U32FltPoint3D intersectionPoint;

	const CCollisionBox *pTargetBox = nullptr;
	const CCollisionCylinder *pTargetCylinder = nullptr;

	float targetHeight;
	float targetRadius;
	U32FltPoint3D targetCenter;

	if (_ignore)
		return;

	switch (targetObject._objectShape) {
	case kBox:
		pTargetBox = static_cast<const CCollisionBox *>(&targetObject);

		for (int i = X_INDEX; i <= Y_INDEX; ++i) {
			EDimension dim = (EDimension)i;

			if (center[dim] < pTargetBox->minPoint[dim]) {
				intersectionPoint[dim] = pTargetBox->minPoint[dim];
			} else if (center[dim] > pTargetBox->maxPoint[dim]) {
				intersectionPoint[dim] = pTargetBox->maxPoint[dim];
			} else {
				intersectionPoint[dim] = center[dim];
			}
		}

		targetVector = (center - intersectionPoint);
		targetVector.z = 0;
		targetVector = targetVector.normalize() * radius;

		center.x = intersectionPoint.x + targetVector.x;
		center.y = intersectionPoint.y + targetVector.y;
		testObjectIntersection(targetObject, distance);

		break;

	case kCylinder:
		pTargetCylinder = static_cast<const CCollisionCylinder *>(&targetObject);
		targetHeight = pTargetCylinder->height;
		targetRadius = pTargetCylinder->radius;
		targetCenter = pTargetCylinder->center;

		// Handle the case where two players are at the exact same spot...
		if (!((distance->x != 0) || (distance->y != 0)))
			warning("CCollisionCylinder::forceOutOfObject(): These two cylinders have the same center, so we don't know which direction to push this one out.");

		if (distance->xyMagnitude() == 0) {
			distance->x = 1;
		}

		if (distance->z < (targetHeight / 2)) {
			targetVector.x = distance->x;
			targetVector.y = distance->y;
			targetVector.z = 0;

			targetVector = targetVector.normalize() * (radius + targetRadius);

			center.x = targetCenter.x + targetVector.x;
			center.y = targetCenter.y + targetVector.y;
			testObjectIntersection(targetObject, distance);
		}

		break;

	default:
		return;
	}
}

U32FltPoint3D CCollisionCylinder::findNearestPoint(const U32FltPoint3D &testPoint) const {
	U32FltPoint3D cylinderPoint;

	if (testPoint.z <= center.z - (height / 2)) {
		cylinderPoint.x = testPoint.x;
		cylinderPoint.y = testPoint.y;
		cylinderPoint.z = center.z - (height / 2);
	} else if (testPoint.z >= center.z + (height / 2)) {
		cylinderPoint.x = testPoint.x;
		cylinderPoint.y = testPoint.y;
		cylinderPoint.z = center.z + (height / 2);
	} else {
		U32FltVector3D centerVector = testPoint - center;
		centerVector = centerVector.normalize() * radius;

		cylinderPoint.x = center.x + centerVector.x;
		cylinderPoint.y = center.y + centerVector.y;
		cylinderPoint.z = testPoint.z;
	}

	return cylinderPoint;
}

void CCollisionCylinder::handleCollision(const CCollisionBox &targetBox, float *timeUsed, U32Distance3D *distance,  bool advanceObject) {
	// Handle collisions in the z direction.
	// If we're on something, make sure we aren't going up or down...
	if (((distance->z >= (height / 2)) && (_velocity.z < 0)) ||
		((distance->z <= -(height / 2)) && (_velocity.z > 0))) {
		_velocity.z = 0;
	}

	// Handle collisions in the xy direction...
	if (_movementType == kCircular) {
		_velocity.x = 0;
		_velocity.y = 0;
	} else {
		U32FltPoint2D collisionPoint;
		collisionPoint.x = getPointOfCollision(targetBox, *distance, X_INDEX);
		collisionPoint.y = getPointOfCollision(targetBox, *distance, Y_INDEX);

		U32FltVector2D targetVector;
		targetVector.x = collisionPoint.x - center.x;
		targetVector.y = collisionPoint.y - center.y;

		// Project the velocity vector onto that vector.
		// This will give us the component of the velocity vector that collides directly
		// with the target object. This part of the velocity vector will be lost in the
		// collision...
		_velocity -= _velocity.projectVector(targetVector);
	}

	if (advanceObject) {
		center.x += (_velocity.x * *timeUsed);
		center.y += (_velocity.y * *timeUsed);
		center.z += (_velocity.z * *timeUsed);
		*timeUsed = 0;
	}

	return;
}

void CCollisionCylinder::handleCollision(const CCollisionCylinder &targetCylinder, float *timeUsed, U32Distance3D *distance, bool advanceObject) {
	// Handle collisions in the z direction.
	// If we're on something, make sure we aren't going up or down...
	if (((distance->z >= (height / 2)) && (_velocity.z < 0)) ||
		((distance->z <= -(height / 2)) && (_velocity.z > 0))) {
		_velocity.z = 0;
	}

	// Handle collisions in the xy direction...
	if (_movementType == kCircular) {
		// If we are moving in a circular motion, then we must have hit at
		// least 2 objects this frame, so we stop our movement altogether...
		_velocity.x = 0;
		_velocity.y = 0;

		*timeUsed = 1;
	} else {
		// Find the vector from the center of this object to the center of
		// the collision object...
		U32FltVector2D targetVector = targetCylinder.center - center;

		// Project the velocity vector onto that vector.
		// This will give us the component of the velocity vector that collides directly
		// with the target object. This part of the velocity vector will be lost in the
		// collision...
		_velocity -= _velocity.projectVector(targetVector);

		// If we are moving forward, do so, then update the vector...
		if (advanceObject) {
			float finalMagnitude = _velocity.xyMagnitude();

			if (finalMagnitude != 0) {
				// Revolve this cylinder around the targer cylinder...
				_movementType = kCircular;
				_revCenter = dynamic_cast<const ICollisionObject *>(&targetCylinder);
				_revCenterPt = targetCylinder.center;

				// Find out in which direction we are going to revolve around the target cylinder...
				ERevDirection revDirection = getRevDirection();

				// Find the diameter of the circle we are moving around...
				double movementDiameter = 2 * BBALL_M_PI * (targetCylinder.radius + radius);

				// Find how many radians around the circle we will go...
				double movementRadians = 2 * BBALL_M_PI * ((finalMagnitude * MAX(*timeUsed, (float)1)) / movementDiameter) * revDirection;

				center.x = (distance->x * cos(movementRadians) - distance->y * sin(movementRadians)) + targetCylinder.center.x;
				center.y = (distance->x * sin(movementRadians) + distance->y * cos(movementRadians)) + targetCylinder.center.y;
				testObjectIntersection(targetCylinder, distance);

				// Adjust for rounding errors in pi by pushing this cylinder straight out
				// or in so that it is in contact in just one location with the target
				// cylinder...
				forceOutOfObject(targetCylinder, distance);

				// Calculate the final vector. The final vector should be tangent to the
				// target cylinder at the point of intersection...
				_velocity.x = targetVector.y;
				_velocity.y = targetVector.x;

				_velocity.x = _velocity.x / targetVector.magnitude();
				_velocity.y = _velocity.y / targetVector.magnitude();

				_velocity.x *= finalMagnitude;
				_velocity.y *= finalMagnitude;

				if (revDirection == kClockwise) {
					_velocity.x *= -1;
				} else {
					_velocity.y *= -1;
				}
			}

			// Update the z position...
			center.z += _velocity.z * *timeUsed;

			*timeUsed = 0;
		} else {
			// Just update the vector...
			float finalMagnitude = _velocity.xyMagnitude();
			ERevDirection revDirection = getRevDirection();

			_velocity.x = targetVector.y;
			_velocity.y = targetVector.x;

			_velocity.x = _velocity.x / targetVector.magnitude();
			_velocity.y = _velocity.y / targetVector.magnitude();

			_velocity.x *= finalMagnitude;
			_velocity.y *= finalMagnitude;

			if (revDirection == kClockwise) {
				_velocity.x *= -1;
			} else {
				_velocity.y *= -1;
			}
		}
	}
}

void CCollisionCylinder::handleCollisions(CCollisionObjectVector *pCollisionVector, float *timeUsed, bool advanceObject) {
	if (pCollisionVector->size() > 1) {
		_velocity.x = 0;
		_velocity.y = 0;
		return;
	}

	ICollisionObject::handleCollisions(pCollisionVector, timeUsed, advanceObject);
}

bool CCollisionCylinder::isOnObject(const CCollisionBox &targetObject, const U32Distance3D &distance) const {
	return ((distance.z == (height / 2)) &&
			(distance.xyMagnitude() < radius));
}

bool CCollisionCylinder::isOnObject(const CCollisionSphere &targetObject, const U32Distance3D &distance) const {
	return ((distance.z == (height / 2)) &&
			(distance.xyMagnitude() < (radius + targetObject.radius)));
}

bool CCollisionCylinder::isOnObject(const CCollisionCylinder &targetObject, const U32Distance3D &distance) const {
	return ((distance.z == (height / 2)) &&
			(distance.xyMagnitude() < (radius + targetObject.radius)));
}

U32BoundingBox CCollisionCylinder::getBoundingBox() const {
	U32BoundingBox outBox;

	outBox.minPoint.x = center.x - radius;
	outBox.minPoint.y = center.y - radius;
	outBox.minPoint.z = center.z - height;

	outBox.maxPoint.x = center.x + radius;
	outBox.maxPoint.y = center.y + radius;
	outBox.maxPoint.z = center.z + height;

	return outBox;
}

U32BoundingBox CCollisionCylinder::getBigBoundingBox() const {
	U32BoundingBox outBox;

	float xyVelocity = _velocity.xyMagnitude();

	outBox.minPoint.x = center.x - (radius + xyVelocity);
	outBox.minPoint.y = center.y - (radius + xyVelocity);
	outBox.minPoint.z = center.z - (height + _velocity.z);

	outBox.maxPoint.x = center.x + (radius + xyVelocity);
	outBox.maxPoint.y = center.y + (radius + xyVelocity);
	outBox.maxPoint.z = center.z + (height + _velocity.z);

	return outBox;
}

void CCollisionCylinder::save() {
	_positionSaved = true;
	_safetyPoint = center;
	_safetyVelocity = _velocity;
}

void CCollisionCylinder::restore() {
	if (_positionSaved) {
		if (_safetyVelocity.magnitude() != 0) {
			debug("CCollisionCylinder::Restore(): Restoring");
			center = _safetyPoint;

			_velocity.x = 0;
			_velocity.y = 0;
			_velocity.z = 0;
		}
	} else {
		warning("CCollisionCylinder::Restore(): No save point.");
	}
}

int CCollisionCylinder::getEquidistantPoint(U32FltPoint2D inPoint1, float distance1, U32FltPoint2D inPoint2, float distance2, U32FltPoint2D *outPoint1, U32FltPoint2D *outPoint2) {
	double distDiff = (distance2 * distance2) - (distance1 * distance1);
	double xDiff = (inPoint1.x * inPoint1.x) - (inPoint2.x * inPoint2.x);
	double yDiff = (inPoint1.y * inPoint1.y) - (inPoint2.y * inPoint2.y);

	double k1 = 0;
	double k2 = 0;

	if (xDiff != 0) {
		k1 = (distDiff + xDiff + yDiff) / (2 * (inPoint1.x - inPoint2.x));
		k2 = (inPoint2.y - inPoint1.y) / (inPoint1.x - inPoint2.x);
	}

	double a = (k2 * k2) + 1;

	double b = (2 * k1 * k2) -
			   (2 * k2 * inPoint1.x) -
			   (2 * inPoint1.y);

	double c = (inPoint1.x * inPoint1.x) +
			   (inPoint1.y * inPoint1.y) +
			   (k1 * k1) -
			   (2 * k1 * inPoint1.x) -
			   (distance1 * distance1);

	if (((b * b) < (4 * a * c)) || (a == 0)) {
		return 0;
	} else {
		outPoint1->y = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
		outPoint2->y = (-b - sqrt(b * b - 4 * a * c)) / (2 * a);

		outPoint1->x = k1 + (k2 * outPoint1->y);
		outPoint2->x = k1 + (k2 * outPoint2->y);

		if (*outPoint1 == *outPoint2) {
			return 1;
		} else {
			return 2;
		}
	}
}

} // End of namespace Scumm
