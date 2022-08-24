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

// Based on Phantasma code by Thomas Harte (2013)

#include "freescape/area.h"
#include "freescape/objects/geometricobject.h"
#include "freescape/objects/entrance.h"
#include "common/algorithm.h"

namespace Freescape {

Object *Area::objectWithIDFromMap(ObjectMap *map, uint16 objectID) {
	if (!map)
		return nullptr;
	if(!map->contains(objectID))
		return nullptr;
	return (*map)[objectID];
}

Object *Area::objectWithID(uint16 objectID) {
	return objectWithIDFromMap(objectsByID, objectID);
}

Object *Area::entranceWithID(uint16 objectID) {
	return objectWithIDFromMap(entrancesByID, objectID);
}

uint16 Area::getAreaID() {
	return areaID;
}

uint16 Area::getAreaFlags() {
	return areaFlags;
}

uint8 Area::getScale() {
	return scale;
}

Area::Area(
	uint16 _areaID,
	uint16 _areaFlags,
	ObjectMap *_objectsByID,
	ObjectMap *_entrancesByID,
	uint8 _scale,
	uint8 _skyColor,
	uint8 _groundColor,
	Graphics::PixelBuffer *_palette) {
	scale = _scale;
	palette = _palette;
	skyColor = _skyColor;
	groundColor = _groundColor;
	areaID = _areaID;
	areaFlags = _areaFlags;
	objectsByID = _objectsByID;
	entrancesByID = _entrancesByID;

	// create a list of drawable objects only
	for (ObjectMap::iterator iterator = objectsByID->begin(); iterator != objectsByID->end(); iterator++) {
		if (iterator->_value->isDrawable()) {
			drawableObjects.push_back(iterator->_value);
		}
	}

	// sort so that those that are planar are drawn last
	struct
	{
		bool operator()(Object *object1, Object *object2) {
			if (!object1->isPlanar() && object2->isPlanar())
				return true;
			if (object1->isPlanar() && !object2->isPlanar())
				return false;
			return object1->getObjectID() > object2->getObjectID();
		};
	} compareObjects;

	Common::sort(drawableObjects.begin(), drawableObjects.end(), compareObjects);
	gasPocketRadius = 0;
}

Area::~Area() {
	for (ObjectMap::iterator iterator = entrancesByID->begin(); iterator != entrancesByID->end(); iterator++)
		delete iterator->_value;

	for (ObjectMap::iterator iterator = objectsByID->begin(); iterator != objectsByID->end(); iterator++)
		delete iterator->_value;

	delete entrancesByID;
	delete objectsByID;
}

void Area::show() {
	debug("Area name: %s", name.c_str());
	for (ObjectMap::iterator it = objectsByID->begin(); it != objectsByID->end(); it++)
		debug("objID: %d, type: %d", (*it)._value->getObjectID(), (*it)._value->getType());

	for (ObjectMap::iterator it = entrancesByID->begin(); it != entrancesByID->end(); it++)
		debug("objID: %d, type: %d (entrance)", (*it)._value->getObjectID(), (*it)._value->getType());
}

void Area::loadObjectFlags(Common::SeekableReadStream *stream) {
	drillPosition.setValue(0, stream->readFloatLE());
	drillPosition.setValue(1, stream->readFloatLE());
	drillPosition.setValue(2, stream->readFloatLE());

	for (int i = 0; i < int(objectsByID->size()); i++) {
		uint16 key = stream->readUint32LE();
		assert(objectsByID->contains(key));
		Object *obj = (*objectsByID)[key];
		obj->setObjectFlags(stream->readUint32LE());
	}
}

void Area::saveObjectFlags(Common::WriteStream *stream) {
	stream->writeFloatLE(drillPosition.x());
	stream->writeFloatLE(drillPosition.y());
	stream->writeFloatLE(drillPosition.z());

	//stream->writeUint32LE(objectsByID->size());

	for (ObjectMap::iterator iterator = objectsByID->begin(); iterator != objectsByID->end(); iterator++) {
		stream->writeUint32LE(iterator->_key);
		stream->writeUint32LE(iterator->_value->getObjectFlags());
	}
}

void Area::draw(Freescape::Renderer *gfx) {
	if (palette)
		gfx->_palette = palette;

	gfx->clear();
	if (skyColor != 255) {
		gfx->_keyColor = 0;
		gfx->drawSky(skyColor);
	} else
		gfx->_keyColor = 255;

	assert(drawableObjects.size() > 0);
	for (Common::Array<Object *>::iterator it = drawableObjects.begin(); it != drawableObjects.end(); it++) {
		if (!(*it)->isDestroyed() && !(*it)->isInvisible()) {
			(*it)->draw(gfx);
		}
	}
}

Object *Area::shootRay(const Math::Ray &ray) {
	float size = 16 * 8192; // TODO: check if this is max size
	Object *collided = nullptr;
	for (int i = 0; i < int(drawableObjects.size()); i++) {
		float objSize = drawableObjects[i]->getSize().length();
		if (!drawableObjects[i]->isDestroyed() && !drawableObjects[i]->isInvisible()
		  && drawableObjects[i]->_boundingBox.isValid()
		  && ray.intersectAABB(drawableObjects[i]->_boundingBox)
		  && size >= objSize) {
			debug("shot obj id: %d", drawableObjects[i]->getObjectID());
			collided = drawableObjects[i];
			size = objSize;
		}
	}
	return collided;
}

Object *Area::checkCollisions(const Math::AABB &boundingBox) {
	float size = 3 * 8192 * 8192; // TODO: check if this is max size
	Object *collided = nullptr;
	for (int i = 0; i < int(drawableObjects.size()); i++) {
		if (!drawableObjects[i]->isDestroyed() && !drawableObjects[i]->isInvisible()) {
			GeometricObject *obj = (GeometricObject*) drawableObjects[i];
			float objSize = obj->getSize().length();
			if (obj->collides(boundingBox) && size > objSize) {
				collided = obj;
				size = objSize;
			}
		}
	}
	return collided;
}

void Area::addDrill(Area *global, const Math::Vector3d position) {
	//int drillObjectIDs[8] = {255, 254, 253, 252, 251, 250, 248, 247};
	drillPosition = position;
	Object *obj = nullptr;
	Math::Vector3d offset = position;

	int16 id;

	id = 255;
	debug("Adding object %d to room structure", id);
	obj = global->objectWithID(id);
	obj->setOrigin(offset);
	offset.setValue(1, offset.y() + obj->getSize().y());

	//offset.setValue(1, offset.y() + obj->getSize().y());
	assert(obj);
	obj->makeVisible();
	drawableObjects.insert_at(0, obj);

	id = 254;
	debug("Adding object %d to room structure", id);
	obj = global->objectWithID(id);
	offset.setValue(1, offset.y() + obj->getSize().y());
	obj->setOrigin(offset);
	assert(obj);
	obj->makeVisible();
	drawableObjects.insert_at(0, obj);

	id = 253;
	debug("Adding object %d to room structure", id);
	obj = global->objectWithID(id);
	obj->setOrigin(offset);
	offset.setValue(1, offset.y() + obj->getSize().y());
	assert(obj);
	obj->makeVisible();
	drawableObjects.insert_at(0, obj);

	id = 252;
	debug("Adding object %d to room structure", id);
	obj = global->objectWithID(id);
	obj->setOrigin(offset);
	offset.setValue(1, offset.y() + obj->getSize().y());
	assert(obj);
	obj->makeVisible();
	drawableObjects.insert_at(0, obj);
}

void Area::removeDrill() {
	drillPosition = Math::Vector3d();
	for (int16 id = 252; id < 256; id++) {
		objectsByID->erase(id);
		assert(drawableObjects[0]->getObjectID() == id);
		drawableObjects.remove_at(0);
	}
}

bool Area::drillDeployed() {
	return (drawableObjects[0]->getObjectID() == 252);
}

void Area::addObjectFromArea(int16 id, Area *global) {
	debug("Adding object %d to room structure", id);
	Object *obj = global->objectWithID(id);
	if (!obj) {
		assert(global->entranceWithID(id));
		(*entrancesByID)[id] = global->entranceWithID(id);
	} else {
		(*objectsByID)[id] = global->objectWithID(id);
		if (obj->isDrawable())
			drawableObjects.insert_at(0, obj);
	}
}

void Area::addStructure(Area *global) {
	Object *obj = nullptr;
	if (!global || !entrancesByID->contains(255)) {
		int id = 254;
		Common::Array<uint8> *gColors = new Common::Array<uint8>;
		for (int i = 0; i < 6; i++)
			gColors->push_back(groundColor);

		obj = (Object*) new GeometricObject(
			Object::Type::Cube,
			id,
			0, // flags
			Math::Vector3d(0, -1, 0), // Position
			Math::Vector3d(4128, 1, 4128), // size
			gColors,
			nullptr,
			FCLInstructionVector()
		);
		(*objectsByID)[id] = obj;
		drawableObjects.insert_at(0, obj);
		return;
	}
	RoomStructure *rs = (RoomStructure*) (*entrancesByID)[255];

	for (int i = 0; i < int(rs->structure.size()); i++) {
		int16 id = rs->structure[i];
		if (id == 0)
			continue;

		addObjectFromArea(id, global);
	}
}

} // End of namespace Freescape