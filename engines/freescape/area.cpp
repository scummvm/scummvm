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

Area::Area(uint16 areaID_, uint16 areaFlags_, ObjectMap *objectsByID_, ObjectMap *entrancesByID_) {
	_areaID = areaID_;
	_areaFlags = areaFlags_;
	_objectsByID = objectsByID_;
	_entrancesByID = entrancesByID_;

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

	// sort so that those that are planar are drawn last
	struct {
		bool operator()(Object *object1, Object *object2) {
			if (!object1->isPlanar() && object2->isPlanar())
				return true;
			if (object1->isPlanar() && !object2->isPlanar())
				return false;
			return object1->getObjectID() > object2->getObjectID();
		};
	} compareObjects;

	Common::sort(_drawableObjects.begin(), _drawableObjects.end(), compareObjects);
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
		if (_objectsByID->contains(key)) {
			obj = (*_objectsByID)[key];
		} else {
			obj = global->objectWithID(key);
			assert(obj);
			obj = (Object *)((GeometricObject *)obj)->duplicate();
			addObject(obj);
		}
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

void Area::resetArea() {
	debugC(1, kFreescapeDebugMove, "Resetting area name: %s", _name.c_str());
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


void Area::draw(Freescape::Renderer *gfx, uint32 animationTicks) {
	bool runAnimation = animationTicks != _lastTick;
	assert(_drawableObjects.size() > 0);
	for (auto &obj : _drawableObjects) {
		if (!obj->isDestroyed() && !obj->isInvisible()) {
			if (obj->getType() != ObjectType::kGroupType)
				obj->draw(gfx);
			else {
				drawGroup(gfx, (Group *)obj, runAnimation);
			}
		}
	}
	_lastTick = animationTicks;
}

void Area::drawGroup(Freescape::Renderer *gfx, Group* group, bool runAnimation) {
	if (runAnimation) {
		group->run();
		group->draw(gfx);
		group->step();
	} else
		group->draw(gfx);
}

Object *Area::checkCollisionRay(const Math::Ray &ray, int raySize) {
	float distance = 1.0;
	float size = 16.0 * 8192.0; // TODO: check if this is the max size
	Math::AABB boundingBox(ray.getOrigin(), ray.getOrigin());
	Object *collided = nullptr;
	for (auto &obj : _drawableObjects) {
		if (!obj->isDestroyed() && !obj->isInvisible()) {
			GeometricObject *gobj = (GeometricObject *)obj;
			Math::Vector3d collidedNormal;
			float collidedDistance = sweepAABB(boundingBox, gobj->_boundingBox, raySize * ray.getDirection(), collidedNormal);
			debugC(1, kFreescapeDebugMove, "shot obj id: %d with distance %f", obj->getObjectID(), collidedDistance);
			if (collidedDistance >= 1.0)
				continue;

			if (collidedDistance < distance || (collidedDistance == distance && gobj->getSize().length() < size)) {
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
		if (!obj->isDestroyed() && !obj->isInvisible()) {
			GeometricObject *gobj = (GeometricObject *)obj;
			if (gobj->collides(boundingBox)) {
				collided.push_back(gobj);
			}
		}
	}
	return collided;
}

extern Math::AABB createPlayerAABB(Math::Vector3d const position, int playerHeight);

Math::Vector3d Area::resolveCollisions(const Math::Vector3d &lastPosition_, const Math::Vector3d &newPosition_, int playerHeight) {
	Math::Vector3d position = newPosition_;
	Math::Vector3d lastPosition = lastPosition_;
	Math::AABB boundingBox = createPlayerAABB(lastPosition, playerHeight);

	float epsilon = 1.5;
	int i = 0;
	while (true) {
		float distance = 1.0;
		Math::Vector3d normal;
		Math::Vector3d direction = position - lastPosition;

		for (auto &obj : _drawableObjects) {
			if (!obj->isDestroyed() && !obj->isInvisible()) {
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

void Area::addObjectFromArea(int16 id, Area *global) {
	debugC(1, kFreescapeDebugParser, "Adding object %d to room structure", id);
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

void Area::addFloor() {
	int id = 0;
	assert(!_objectsByID->contains(id));
	Common::Array<uint8> *gColors = new Common::Array<uint8>;
	for (int i = 0; i < 6; i++)
		gColors->push_back(_groundColor);

	Object *obj = (Object *)new GeometricObject(
		ObjectType::kCubeType,
		id,
		0,                             // flags
		Math::Vector3d(-4128, -1, -4128),      // Position
		Math::Vector3d(4128 * 4, 1, 4128 * 4), // size
		gColors,
		nullptr,
		FCLInstructionVector());
	(*_objectsByID)[id] = obj;
	_drawableObjects.insert_at(0, obj);
}

void Area::addStructure(Area *global) {
	if (!global || !_entrancesByID->contains(255)) {
		addFloor();
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

} // End of namespace Freescape
