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

#include "scumm/he/basketball/collision/bball_collision_support_obj.h"
#include "scumm/he/basketball/collision/bball_collision_object.h"
#include "scumm/he/basketball/collision/bball_collision_sphere.h"
#include "scumm/he/basketball/collision/bball_collision_box.h"
#include "scumm/he/basketball/collision/bball_collision_cylinder.h"
#include "scumm/he/basketball/collision/bball_collision_stack.h"
#include "scumm/he/basketball/collision/bball_collision_node.h"
#include "scumm/he/basketball/collision/bball_collision_tree.h"

namespace Scumm {

ICollisionObject::ICollisionObject(EObjectShape shape) : _objectShape(shape),
														 _objectID(0),
														 _objectType(kNoObjectType),
														 _description(""),
														 _collisionEfficiency(0.0F),
														 _friction(0.0F),
														 _soundNumber(0),
														 _ignore(false) {
	_velocity.x = 0.0F;
	_velocity.y = 0.0F;
	_velocity.z = 0.0F;
}

bool ICollisionObject::operator==(const ICollisionObject &otherObject) const {
	return ((otherObject._objectShape == _objectShape) &&
			(otherObject._objectType == _objectType) &&
			(otherObject._objectID == _objectID));
}

float ICollisionObject::getObjectDistance(const ICollisionObject &targetObject) const {
	float result;

	switch (targetObject._objectShape) {
	case kNoObjectShape:
		warning("ICollisionObject::getObjectDistance(): Tried to interact with an object of undefined type");
		result = 0.0F;
		break;
	case kSphere:
		result = getObjectDistance(dynamic_cast<const CCollisionSphere &>(targetObject));
		break;
	case kBox:
		result = getObjectDistance(dynamic_cast<const CCollisionBox &>(targetObject));
		break;
	case kCylinder:
		result = getObjectDistance(dynamic_cast<const CCollisionCylinder &>(targetObject));
		break;
	default:
		warning("ICollisionObject::getObjectDistance(): Tried to interact with an object of undefined type");
		result = 0.0F;
	}

	return result;
}

bool ICollisionObject::testObjectIntersection(const ICollisionObject &targetObject, U32Distance3D *distance) const {
	bool result;

	switch (targetObject._objectShape) {
	case kNoObjectShape:
		warning("ICollisionObject::testObjectIntersection(): Tried to interact with an object of undefined type");
		result = false;
		break;
	case kSphere:
		result = testObjectIntersection(dynamic_cast<const CCollisionSphere &>(targetObject), distance);
		break;
	case kBox:
		result = testObjectIntersection(dynamic_cast<const CCollisionBox &>(targetObject), distance);
		break;
	case kCylinder:
		result = testObjectIntersection(dynamic_cast<const CCollisionCylinder &>(targetObject), distance);
		break;
	default:
		warning("ICollisionObject::testObjectIntersection(): Tried to interact with an object of undefined type");
		result = false;
	}

	return result;
}

bool ICollisionObject::validateCollision(const ICollisionObject &targetObject, U32Distance3D *distance) {
	if (_ignore)
		return true;

	bool result;

	switch (targetObject._objectShape) {
	case kNoObjectShape:
		warning("ICollisionObject::validateCollision(): Tried to interact with an object of undefined type");
		result = false;
		break;
	case kSphere:
		result = validateCollision(dynamic_cast<const CCollisionSphere &>(targetObject), distance);
		break;
	case kBox:
		result = validateCollision(dynamic_cast<const CCollisionBox &>(targetObject), distance);
		break;
	case kCylinder:
		result = validateCollision(dynamic_cast<const CCollisionCylinder &>(targetObject), distance);
		break;
	default:
		warning("ICollisionObject::validateCollision(): Tried to interact with an object of undefined type");
		result = false;
	}

	return result;
}

bool ICollisionObject::backOutOfObject(const ICollisionObject &targetObject, U32Distance3D *distance, float *timeUsed) {
	if (_ignore)
		return true;

	bool result;

	switch (targetObject._objectShape) {
	case kNoObjectShape:
		warning("ICollisionObject::backOutOfObject(): Tried to interact with an object of undefined type");
		result = false;
		break;
	case kSphere:
		result = backOutOfObject(dynamic_cast<const CCollisionSphere &>(targetObject), distance, timeUsed);
		break;
	case kBox:
		result = backOutOfObject(dynamic_cast<const CCollisionBox &>(targetObject), distance, timeUsed);
		break;
	case kCylinder:
		result = backOutOfObject(dynamic_cast<const CCollisionCylinder &>(targetObject), distance, timeUsed);
		break;
	default:
		warning("ICollisionObject::backOutOfObject(): Tried to interact with an object of undefined type");
		result = false;
	}

	return result;
}

bool ICollisionObject::nudgeObject(const ICollisionObject &targetObject, U32Distance3D *distance, float *timeUsed) {
	if (_velocity.magnitude() == 0)
		return true;

	if (_ignore)
		return true;

	if (*timeUsed == 0)
		return true;

	bool result;

	switch (targetObject._objectShape) {
	case kNoObjectShape:
		warning("ICollisionObject::nudgeObject(): Tried to interact with an object of undefined type");
		result = false;
		break;
	case kSphere:
		result = nudgeObject(dynamic_cast<const CCollisionSphere &>(targetObject), distance, timeUsed);
		break;
	case kBox:
		result = nudgeObject(dynamic_cast<const CCollisionBox &>(targetObject), distance, timeUsed);
		break;
	case kCylinder:
		result = nudgeObject(dynamic_cast<const CCollisionCylinder &>(targetObject), distance, timeUsed);
		break;
	default:
		warning("ICollisionObject::nudgeObject(): Tried to interact with an object of undefined type");
		result = false;
	}

	return result;
}

bool ICollisionObject::isCollisionHandled(const ICollisionObject &targetObject) const {
	bool result;

	switch (targetObject._objectShape) {
	case kNoObjectShape:
		warning("ICollisionObject::isCollisionHandled(): Tried to interact with an object of undefined type");
		result = false;
		break;
	case kSphere:
		result = isCollisionHandled(dynamic_cast<const CCollisionSphere &>(targetObject));
		break;
	case kBox:
		result = isCollisionHandled(dynamic_cast<const CCollisionBox &>(targetObject));
		break;
	case kCylinder:
		result = isCollisionHandled(dynamic_cast<const CCollisionCylinder &>(targetObject));
		break;
	default:
		warning("ICollisionObject::isCollisionHandled(): Tried to interact with an object of undefined type");
		result = false;
	}

	return result;
}

void ICollisionObject::handleCollisions(CCollisionObjectVector *collisionVector, float *timeUsed, bool advanceObject) {
	if (_velocity.magnitude() == 0)
		return;

	if (_ignore)
		return;

	for (CCollisionObjectVector::const_iterator objectIt = collisionVector->begin(); objectIt != collisionVector->end(); ++objectIt) {
		const ICollisionObject *pTargetObject = *objectIt;

		U32Distance3D distance;
		testObjectIntersection(*pTargetObject, &distance);

		switch (pTargetObject->_objectShape) {
		case kNoObjectShape:
			warning("ICollisionObject::handleCollisions(): Tried to interact with an object of undefined type");
			break;
		case kSphere:
			handleCollision(*static_cast<const CCollisionSphere *>(pTargetObject), timeUsed, &distance, advanceObject);
			break;
		case kBox:
			handleCollision(*static_cast<const CCollisionBox *>(pTargetObject), timeUsed, &distance, advanceObject);
			break;
		case kCylinder:
			handleCollision(*static_cast<const CCollisionCylinder *>(pTargetObject), timeUsed, &distance, advanceObject);
			break;
		default:
			warning("ICollisionObject::handleCollisions(): Tried to interact with an object of undefined type");
		}
	}
}

bool ICollisionObject::isOnObject(const ICollisionObject &targetObject, const U32Distance3D &distance) const {
	bool result;

	switch (targetObject._objectShape) {
	case kNoObjectShape:
		warning("ICollisionObject::isOnObject(): Tried to interact with an object of undefined type");
		result = false;
		break;
	case kSphere:
		result = isOnObject(static_cast<const CCollisionSphere &>(targetObject), distance);
		break;
	case kBox:
		result = isOnObject(static_cast<const CCollisionBox &>(targetObject), distance);
		break;
	case kCylinder:
		result = isOnObject(static_cast<const CCollisionCylinder &>(targetObject), distance);
		break;
	default:
		warning("ICollisionObject::isOnObject(): Tried to interact with an object of undefined type");
		result = false;
	}

	return result;
}

U32FltPoint3D ICollisionObject::findNearestPoint(const U32FltPoint3D &testPoint) const {
	return testPoint;
}

float ICollisionObject::getPenetrationTime(const ICollisionObject &targetObject, const U32Distance3D &distance, EDimension dimension) const {
	float result;

	switch (targetObject._objectShape) {
	case kNoObjectShape:
		warning("ICollisionObject::getPenetrationTime(): Tried to interact with an object of undefined type");
		result = 0.0F;
		break;
	case kSphere:
		result = getPenetrationTime(*static_cast<const CCollisionSphere *>(&targetObject), distance, dimension);
		break;
	case kBox:
		result = getPenetrationTime(*static_cast<const CCollisionBox *>(&targetObject), distance, dimension);
		break;
	case kCylinder:
		result = getPenetrationTime(*static_cast<const CCollisionCylinder *>(&targetObject), distance, dimension);
		break;
	default:
		warning("ICollisionObject::getPenetrationTime(): Tried to interact with an object of undefined type");
		result = 0.0F;
	}

	return result;
}

void ICollisionObject::defineReflectionPlane(const ICollisionObject &targetObject, const U32Distance3D &distance, U32Plane *collisionPlane) const {
	switch (targetObject._objectShape) {
	case kNoObjectShape:
		warning("ICollisionObject::defineReflectionPlane(): Tried to interact with an object of undefined type");
		break;
	case kSphere:
		defineReflectionPlane(dynamic_cast<const CCollisionSphere &>(targetObject), distance, collisionPlane);
		break;
	case kBox:
		defineReflectionPlane(dynamic_cast<const CCollisionBox &>(targetObject), distance, collisionPlane);
		break;
	case kCylinder:
		defineReflectionPlane(dynamic_cast<const CCollisionCylinder &>(targetObject), distance, collisionPlane);
		break;
	default:
		warning("ICollisionObject::defineReflectionPlane(): Tried to interact with an object of undefined type");
	}
}

void ICollisionObject::defineReflectionPlane(const CCollisionSphere &targetObject, const U32Distance3D &distance, U32Plane *collisionPlane) const {
	collisionPlane->clear();
}

void ICollisionObject::defineReflectionPlane(const CCollisionBox &targetObject, const U32Distance3D &distance, U32Plane *collisionPlane) const {
	collisionPlane->clear();
}

void ICollisionObject::defineReflectionPlane(const CCollisionCylinder &targetObject, const U32Distance3D &distance, U32Plane *collisionPlane) const {
	collisionPlane->clear();
}

} // End of namespace Scumm
