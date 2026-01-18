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

// Based on Phantasma code by Thomas Harte (2013),
// available at https://github.com/TomHarte/Phantasma/ (MIT)

#include "common/algorithm.h"
#include "common/hash-ptr.h"

#include "freescape/freescape.h"
#include "freescape/area.h"
#include "freescape/objects/global.h"
#include "freescape/sweepAABB.h"

namespace Freescape {

Object *Area::objectWithIDFromMap(ObjectMap *map, uint16 objectID) {
	if (!map)
		return nullptr;
	if (!map->contains(objectID))
		return nullptr;
	return (*map)[objectID];
}

Object *Area::objectWithID(uint16 objectID) {
	return objectWithIDFromMap(_objectsByID, objectID);
}

Object *Area::entranceWithID(uint16 objectID) {
	return objectWithIDFromMap(_entrancesByID, objectID);
}

uint16 Area::getAreaID() {
	return _areaID;
}

uint16 Area::getAreaFlags() {
	return _areaFlags;
}

uint8 Area::getScale() {
	return _scale;
}

Area::Area(uint16 areaID_, uint16 areaFlags_, ObjectMap *objectsByID_, ObjectMap *entrancesByID_, bool isCastle_) {
	_areaID = areaID_;
	_areaFlags = areaFlags_;
	_objectsByID = objectsByID_;
	_entrancesByID = entrancesByID_;
	_isCastle = isCastle_;

	_scale = 0;
	_skyColor = 255;
	_groundColor = 255;
	_usualBackgroundColor = 255;
	_underFireBackgroundColor = 255;
	_inkColor = 255;
	_paperColor = 255;

	_gasPocketRadius = 0;

	// create a list of drawable objects only
	for (auto &it : *_objectsByID) {
		if (it._value->isDrawable()) {
			_drawableObjects.push_back(it._value);
		}
	}

	_lastTick = 0;
}

Area::~Area() {
	if (_entrancesByID) {
		for (auto &it : *_entrancesByID) {
			if (!_addedObjects.contains(it._value->getObjectID()))
				delete it._value;
		}
	}

	if (_objectsByID) {
		for (auto &it : *_objectsByID) {
			if (!_addedObjects.contains(it._value->getObjectID()))
				delete it._value;
		}
	}

	delete _entrancesByID;
	delete _objectsByID;
}

ObjectArray Area::getSensors() {
	ObjectArray sensors;
	debugC(1, kFreescapeDebugMove, "Area name: %s", _name.c_str());
	for (auto &it : *_objectsByID) {
		if (it._value->getType() == kSensorType)
			sensors.push_back(it._value);
	}
	return sensors;
}

void Area::show() {
	debugC(1, kFreescapeDebugMove, "Area name: %s", _name.c_str());
	for (auto &it : *_objectsByID)
		debugC(1, kFreescapeDebugMove, "objID: %d, type: %d", it._value->getObjectID(), it._value->getType());

	for (auto &it : *_entrancesByID)
		debugC(1, kFreescapeDebugMove, "objID: %d, type: %d (entrance)", it._value->getObjectID(), it._value->getType());
}

void Area::loadObjects(Common::SeekableReadStream *stream, Area *global) {
	int objectsByIDSize = stream->readUint32LE();

	for (int i = 0; i < objectsByIDSize; i++) {
		uint16 key = stream->readUint32LE();
		uint32 flags = stream->readUint32LE();
		float x = stream->readFloatLE();
		float y = stream->readFloatLE();
		float z = stream->readFloatLE();
		Object *obj = nullptr;
		if (!_objectsByID->contains(key))
			addObjectFromArea(key, global);

		obj = (*_objectsByID)[key];
		assert(obj);
		obj->setObjectFlags(flags);
		obj->setOrigin(Math::Vector3d(x, y, z));
	}

	_colorRemaps.clear();
	int colorRemapsSize = stream->readUint32LE();

	for (int i = 0; i < colorRemapsSize; i++) {
		int src = stream->readUint32LE();
		int dst = stream->readUint32LE();
		remapColor(src, dst);
	}
}

void Area::saveObjects(Common::WriteStream *stream) {
	stream->writeUint32LE(_objectsByID->size());

	for (auto &it : *_objectsByID) {
		Object *obj = it._value;
		stream->writeUint32LE(it._key);
		stream->writeUint32LE(obj->getObjectFlags());
		stream->writeFloatLE(obj->getOrigin().x());
		stream->writeFloatLE(obj->getOrigin().y());
		stream->writeFloatLE(obj->getOrigin().z());
	}

	stream->writeUint32LE(_colorRemaps.size());
	for (auto &it : _colorRemaps) {
		stream->writeUint32LE(it._key);
		stream->writeUint32LE(it._value);
	}
}

void Area::remapColor(int index, int color) {
	_colorRemaps[index] = color;
}

void Area::unremapColor(int index) {
	_colorRemaps.clear(index);
}

void Area::resetAreaGroups() {
	debugC(1, kFreescapeDebugMove, "Resetting groups from area: %s", _name.c_str());
	if (_objectsByID) {
		for (auto &it : *_objectsByID) {
			Object *obj = it._value;

			if (obj->getType() == ObjectType::kGroupType)
				((Group *)obj)->reset();
		}
	}
}

void Area::resetArea() {
	debugC(1, kFreescapeDebugMove, "Resetting objects from area: %s", _name.c_str());
	_colorRemaps.clear();
	if (_objectsByID) {
		for (auto &it : *_objectsByID) {
			Object *obj = it._value;
			if (obj->isDestroyed())
				obj->restore();

			if (obj->isInitiallyInvisible())
				obj->makeInvisible();
			else
				obj->makeVisible();
		}
	}
	if (_entrancesByID) {
		for (auto &it : *_entrancesByID) {
			Object *obj = it._value;
			if (obj->isDestroyed())
				obj->restore();

			if (obj->isInitiallyInvisible())
				obj->makeInvisible();
			else
				obj->makeVisible();
		}
	}
}


void Area::draw(Freescape::Renderer *gfx, uint32 animationTicks, Math::Vector3d camera, Math::Vector3d direction, bool insideWait) {
	bool runAnimation = animationTicks != _lastTick;
	bool cameraChanged = camera != _lastCameraPosition;
	bool sort = runAnimation || cameraChanged || _sortedObjects.empty();

	assert(_drawableObjects.size() > 0);
	if (sort)
		_sortedObjects.clear();

	Object *floor = nullptr;

	for (auto &obj : _drawableObjects) {
		if (!obj->isDestroyed() && !obj->isInvisible()) {
			if (obj->getObjectID() == 0 && _groundColor < 255 && _skyColor < 255) {
				floor = obj;
				continue;
			}

			if (obj->getType() == ObjectType::kGroupType) {
				drawGroup(gfx, (Group *)obj, runAnimation && !insideWait);
				continue;
			}

			if (sort)
				_sortedObjects.push_back(obj);
		}
	}

	if (floor) {
		floor->draw(gfx);
	}

	// Corresponds to L9c66 in assembly (bounding_box_axis_loop)
	auto checkAxis = [](float minA, float maxA, float minB, float maxB) -> int {
		bool signMinA = minA >= 0;
		bool signMaxA = maxA >= 0;
		bool signMinB = minB >= 0;
		bool signMaxB = maxB >= 0;
		if (minA >= maxB - 0.5f) { // A is clearly "greater" than B (L9c9b_one_object_clearly_further_than_the_other)
			if (signMinA != signMaxB) // A covers 0 (L9ce6_first_object_is_closer)
				return 1; // A is closer
			if (signMinB != signMaxB) // B covers 0 (L9cec_second_object_is_closer)
				return 2; // B is closer

			if (signMinA != signMinB) // Different sides (L9cf3_objects_incomparable_in_this_axis)
				return 0;

			// Same side
			if (!signMinA) { // Negative side (sign bit set in asm)
				if (minA > minB) return 1; // A closer
				if (minA < minB) return 2; // B closer
				if (maxA > maxB) return 1; // A closer
				return 2; // B closer
			} else { // Positive side (sign bit clear in asm)
				if (minA < minB) return 1; // A closer
				if (minA > minB) return 2; // B closer
				if (maxA > maxB) return 2; // B closer
				return 1; // A closer
			}
		} else if (minB >= maxA - 0.5f) { // B is clearly "greater" than A
			if (signMinB != signMaxB) // B covers 0 (L9cec_second_object_is_closer)
				return 2; // B is closer
			if (signMinA != signMaxA) // A covers 0 (L9ce6_first_object_is_closer)
				return 1; // A is closer

			if (signMinA != signMinB) // Different sides (L9cf3_objects_incomparable_in_this_axis)
				return 0;

			// Same side
			if (!signMinB) { // Negative side
				if (minB > minA) return 2; // B closer
				if (minB < minA) return 1; // A closer
				if (maxB > maxA) return 2; // B closer
				return 1; // A closer
			} else { // Positive side
				if (minB < minA) return 2; // B closer
				if (minB > minA) return 1; // A closer
				if (maxB > maxA) return 1; // A closer
				return 2; // B closer
			}
		}
		return 0; // Overlap (L9cf3_objects_incomparable_in_this_axis)
	};

	// Bubble sort as implemented in castlemaster2-annotated.asm (L9c2d_sort_objects_for_rendering)
	// NOTE: The sorting is performed on unprojected world-space coordinates relative to the player (L847f).
	// The rotation/view matrix (computed in L95de) is NOT applied to the bounding boxes used for sorting.
	// It is only applied to the vertices during the projection phase (L850f/L9177).
	int n = _sortedObjects.size();
	if (n > 1 && sort) {
		for (int i = 0; i < n; i++) { // L9c31_whole_object_pass_loop
			bool changed = false;
			for (int j = 0; j < n - 1; j++) { // L9c45_objects_loop
				Object *a = _sortedObjects[j];
				Object *b = _sortedObjects[j + 1];

				Math::AABB bboxA = a->_occlusionBox;
				Math::AABB bboxB = b->_occlusionBox;
				Math::Vector3d minA = bboxA.getMin() - camera;
				Math::Vector3d maxA = bboxA.getMax() - camera;
				Math::Vector3d minB = bboxB.getMin() - camera;
				Math::Vector3d maxB = bboxB.getMax() - camera;

				int result = 0;

				// X axis
				result = (result << 2) | checkAxis(minA.x(), maxA.x(), minB.x(), maxB.x());
				// Y axis
				result = (result << 2) | checkAxis(minA.y(), maxA.y(), minB.y(), maxB.y());
				// Z axis
				result = (result << 2) | checkAxis(minA.z(), maxA.z(), minB.z(), maxB.z());

				bool keepOrder = false;
				// If result indicates B is closer in at least one axis, AND A is NEVER closer in any axis, keep order (A before B)
				// Codes where B is closer (2) and A is not (1):
				// 2 (Z), 8 (Y), 32 (X) -> hex: 02, 08, 20
				// 2+8=10 (0A), 2+32=34 (22), 8+32=40 (28)
				// 2+8+32=42 (2A)
				// L9d37_next_object (Keep order)
				if (result == 0x02 || result == 0x08 || result == 0x20 ||
					result == 0x0A || result == 0x22 || result == 0x28 || result == 0x2A)
					keepOrder = true; // A before B

				if (result == 0) {
					bool aInFront = minA.z() < 0;
					bool bInFront = minB.z() < 0;
					// if signs differ that means one object is positive in the z axis i.e. not visible in the camera
					if (aInFront != bInFront) {
						// one is visible, one is behind us.
						// we want the one behind the camera (positive Z) to be drawn first
						// so the visible object draws on top of it
						// the one with the larger Z (positive) should come first
						if (minA.z() > minB.z()) {
							keepOrder = true;
						}
					}
				}

				if (!keepOrder) {
					// Swap objects (L9d2c_flip_objects_loop)
					_sortedObjects[j] = b;
					_sortedObjects[j + 1] = a;
					changed = true;
				}
			}
			if (!changed)
				break;
		}
	}

	for (auto &obj : _sortedObjects) {
		obj->draw(gfx);
	}

	_lastTick = animationTicks;
	if (sort)
		_lastCameraPosition = camera;
}

void Area::drawGroup(Freescape::Renderer *gfx, Group* group, bool runAnimation) {
	if (runAnimation) {
		group->run();
		group->draw(gfx);
		group->step();
	} else
		group->draw(gfx);
}

bool Area::hasActiveGroups() {
	for (auto &obj : _drawableObjects) {
		if (obj->getType() == kGroupType) {
			Group *group = (Group *)obj;
			if (group->isActive())
				return true;
		}
	}
	return false;
}

Object *Area::checkCollisionRay(const Math::Ray &ray, int raySize) {
	float distance = 1.0;
	float size = 16.0 * 8192.0; // TODO: check if this is the max size
	Math::AABB boundingBox(ray.getOrigin(), ray.getOrigin());
	Object *collided = nullptr;
	for (auto &obj : _drawableObjects) {
		if (obj->getType() == kLineType)
			// If the line is not along an axis, the AABB is wildly inaccurate so we skip it
			if (((GeometricObject *)obj)->isLineButNotStraight())
				continue;

		if (!obj->isDestroyed() && !obj->isInvisible() && obj->isGeometric()) {
			GeometricObject *gobj = (GeometricObject *)obj;
			Math::Vector3d collidedNormal;
			float collidedDistance = sweepAABB(boundingBox, gobj->_boundingBox, raySize * ray.getDirection(), collidedNormal);
			debugC(1, kFreescapeDebugMove, "reached obj id: %d with distance %f", obj->getObjectID(), collidedDistance);
			if (collidedDistance >= 1.0)
				continue;

			if (collidedDistance == 0.0 && signbit(collidedDistance))
				continue;

			if (collidedDistance < distance || (ABS(collidedDistance - distance) <= 0.05 && gobj->getSize().length() < size)) {
				collided = obj;
				size = gobj->getSize().length();
				distance = collidedDistance;
			}
		}
	}
	return collided;
}

ObjectArray Area::checkCollisions(const Math::AABB &boundingBox) {
	ObjectArray collided;
	for (auto &obj : _drawableObjects) {
		if (!obj->isDestroyed() && !obj->isInvisible() && obj->isGeometric()) {
			GeometricObject *gobj = (GeometricObject *)obj;
			if (gobj->collides(boundingBox)) {
				collided.push_back(gobj);
			}
		}
	}
	return collided;
}

bool Area::checkIfPlayerWasCrushed(const Math::AABB &boundingBox) {
	for (auto &obj : _drawableObjects) {
		if (!obj->isDestroyed() && !obj->isInvisible() && obj->getType() == kGroupType) {
			Group *group = (Group *)obj;
			if (group->collides(boundingBox)) {
				return true;
			}
		}
	}
	return false;
}

Math::Vector3d Area::separateFromWall(const Math::Vector3d &_position) {
	Math::Vector3d position = _position;
	float sep = 8 / _scale;
	for (auto &obj : _drawableObjects) {
		if (!obj->isDestroyed() && !obj->isInvisible() && obj->isGeometric()) {
			GeometricObject *gobj = (GeometricObject *)obj;
			Math::Vector3d distance = gobj->_boundingBox.distance(position);
			if (distance.length() > 0.0001)
				continue;

			position.z() = position.z() + sep;
			distance = gobj->_boundingBox.distance(position);
			if (distance.length() > 0.0001)
				return position;

			position = _position;
			position.z() = position.z() - sep;
			distance = gobj->_boundingBox.distance(position);
			if (distance.length() > 0.0001)
				return position;

			position = _position;
			position.x() = position.x() + sep;
			distance = gobj->_boundingBox.distance(position);
			if (distance.length() > 0.0001)
				return position;

			position = _position;
			position.x() = position.x() - sep;
			distance = gobj->_boundingBox.distance(position);
			if (distance.length() > 0.0001)
				return position;
		}
	}
	return position;
}

Math::Vector3d Area::resolveCollisions(const Math::Vector3d &lastPosition_, const Math::Vector3d &newPosition_, int playerHeight) {
	Math::Vector3d position = newPosition_;
	Math::Vector3d lastPosition = lastPosition_;

	float reductionHeight = 0.0;
	// Ugly hack to fix the collisions in tight spaces in the stores and junk room
	// for Castle Master
	if (_isCastle && _areaID == 62) {
		reductionHeight = 0.3f;
	} else if (_isCastle && _areaID == 61) {
		reductionHeight = 0.3f;
	}

	Math::AABB boundingBox = createPlayerAABB(lastPosition, playerHeight, reductionHeight);

	float epsilon = 1.5;
	int i = 0;
	while (true) {
		float distance = 1.0;
		Math::Vector3d normal;
		Math::Vector3d direction = position - lastPosition;

		for (auto &obj : _drawableObjects) {
			if (!obj->isDestroyed() && !obj->isInvisible() && obj->isGeometric()) {
				GeometricObject *gobj = (GeometricObject *)obj;
				Math::Vector3d collidedNormal;
				float collidedDistance = sweepAABB(boundingBox, gobj->_boundingBox, direction, collidedNormal);
				if (collidedDistance < distance) {
					distance = collidedDistance;
					normal = collidedNormal;
				}
			}
		}
		position = lastPosition + distance * direction + epsilon * normal;
		if (i > 1 || distance >= 1.0)
			break;
		i++;
	}
	return position;
}

bool Area::checkInSight(const Math::Ray &ray, float maxDistance) {
	Math::Vector3d direction = ray.getDirection();
	direction.normalize();
	GeometricObject point(kCubeType,
			0,
			0,
			Math::Vector3d(0, 0, 0),
			Math::Vector3d(maxDistance / 30, maxDistance / 30, maxDistance / 30), // size
			nullptr,
			nullptr,
			nullptr,
			FCLInstructionVector(),
			"");

	for (int distanceMultiplier = 2; distanceMultiplier <= 10; distanceMultiplier++) {
		Math::Vector3d origin = ray.getOrigin() + distanceMultiplier * (maxDistance / 10) * direction;
		point.setOrigin(origin);

		for (auto &obj : _drawableObjects) {
			if (obj->getType() != kSensorType && !obj->isDestroyed() && !obj->isInvisible() && obj->_boundingBox.isValid() && point.collides(obj->_boundingBox)) {
				return false;
			}
		}
	}

	return true;
}

void Area::addObject(Object *obj) {
	assert(obj);
	int id = obj->getObjectID();
	debugC(1, kFreescapeDebugParser, "Adding object %d to room %d", id, _areaID);
	assert(!_objectsByID->contains(id));
	(*_objectsByID)[id] = obj;
	if (obj->isDrawable())
		_drawableObjects.insert_at(0, obj);

	_addedObjects[id] = obj;
}

void Area::removeObject(int16 id) {
	assert(_objectsByID->contains(id));
	for (uint i = 0; i < _drawableObjects.size(); i++) {
		if (_drawableObjects[i]->getObjectID() == id) {
			_drawableObjects.remove_at(i);
			break;
		}
	}
	_objectsByID->erase(id);
	_addedObjects.erase(id);
}

Common::List<int> Area::getEntranceIds() {
	Common::List<int> ids;
	for (auto &it : *_entrancesByID) {
		ids.push_back(it._key);
	}
	return ids;
}

void Area::addObjectFromArea(int16 id, Area *global) {
	debugC(1, kFreescapeDebugParser, "Adding object %d to room structure in area %d", id, _areaID);
	Object *obj = global->objectWithID(id);
	if (!obj) {
		assert(global->entranceWithID(id));
		obj = global->entranceWithID(id);
		obj = obj->duplicate();
		obj->scale(_scale);
		_addedObjects[id] = obj;
		(*_entrancesByID)[id] = obj;
	} else {
		obj = obj->duplicate();
		obj->scale(_scale);
		(*_objectsByID)[id] = obj;
		_addedObjects[id] = obj;
		if (obj->isDrawable()) {
			_drawableObjects.insert_at(0, obj);
		}
	}
}

void Area::addGroupFromArea(int16 id, Area *global) {
	debugC(1, kFreescapeDebugParser, "Adding group %d to room structure in area %d", id, _areaID);
	Object *obj = global->objectWithID(id);
	assert(obj);
	assert(obj->getType() == ObjectType::kGroupType);

	addObjectFromArea(id, global);
	Group *group = (Group *)objectWithID(id);
	for (auto &it : ((Group *)obj)->_objectIds) {
		if (it == 0 || it == 0xffff)
			break;
		if (!global->objectWithID(it))
			continue;

		if (!objectWithID(it))
			addObjectFromArea(it, global);
		group->linkObject(objectWithID(it));
	}
}


void Area::addFloor() {
	int id = 0;
	assert(!_objectsByID->contains(id));
	Common::Array<uint8> *gColors = new Common::Array<uint8>;
	for (int i = 0; i < 6; i++)
		gColors->push_back(_groundColor);

	int maxSize = 10000000 / 4;
	Object *obj = (Object *)new GeometricObject(
		ObjectType::kCubeType,
		id,
		0,                                           // flags
		Math::Vector3d(-maxSize, -3, -maxSize),      // Position
		Math::Vector3d(maxSize * 4, 3, maxSize * 4), // size
		gColors,
		nullptr,
		nullptr,
		FCLInstructionVector());
	(*_objectsByID)[id] = obj;
	_drawableObjects.insert_at(0, obj);
}

void Area::addStructure(Area *global) {
	if (!global || !_entrancesByID->contains(255)) {
		return;
	}
	GlobalStructure *rs = (GlobalStructure *)(*_entrancesByID)[255];

	for (uint i = 0; i < rs->_structure.size(); i++) {
		int16 id = rs->_structure[i];
		if (id == 0)
			continue;

		addObjectFromArea(id, global);
	}
}

void Area::changeObjectID(uint16 objectID, uint16 newObjectID) {
	assert(!objectWithID(newObjectID));
	Object *obj = objectWithID(objectID);
	assert(obj);
	obj->_objectID = newObjectID;
	_addedObjects.erase(objectID);
	_addedObjects[newObjectID] = obj;

	(*_objectsByID).erase(objectID);
	(*_objectsByID)[newObjectID] = obj;
}


bool Area::isOutside() {
	return _skyColor < 255 && _groundColor < 255;
}

} // End of namespace Freescape
