//
//  Area.cpp
//  Phantasma
//
//  Created by Thomas Harte on 25/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#include "freescape/area.h"
#include "freescape/objects/geometricobject.h"
#include "common/algorithm.h"
#include "freescape/objects/object.h"

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

Object *Area::firstEntrance() {
	return entrancesByID->begin()->_value;
}

uint16 Area::getAreaID() {
	return areaID;
}

uint8 Area::getScale() {
	return scale;
}

Area::Area(
	uint16 _areaID,
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
			return object1->getObjectID() < object2->getObjectID();
		};
	} compareObjects;

	Common::sort(drawableObjects.begin(), drawableObjects.end(), compareObjects);
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
	for (ObjectMap::iterator it = objectsByID->begin(); it != objectsByID->end(); it++)
		debug("objID: %d, type: %d", (*it)._value->getObjectID(), (*it)._value->getType());

	for (ObjectMap::iterator it = entrancesByID->begin(); it != entrancesByID->end(); it++)
		debug("objID: %d, type: %d (entrance)", (*it)._value->getObjectID(), (*it)._value->getType());
}

void Area::draw(Freescape::Renderer *gfx) {
	if (palette)
		gfx->_palette = palette;

	gfx->clear();
	if (skyColor != 255)
		gfx->drawSky(skyColor);
	assert(drawableObjects.size() > 0);
	for (Common::Array<Object *>::iterator it = drawableObjects.begin(); it != drawableObjects.end(); it++) {
		if (!(*it)->isDestroyed() && !(*it)->isInvisible()) {
			(*it)->draw(gfx);
		}
	}
	if (groundColor != 255)
		gfx->drawFloor(groundColor);
}

Object *Area::shootRay(const Math::Ray &ray) {
	float distance = 16 * 8192; // TODO: check if this is max distance
	Object *collided = nullptr;
	for (int i = drawableObjects.size() - 1; i >= 0; i--) {
		if (!drawableObjects[i]->isDestroyed() && !drawableObjects[i]->isInvisible() && drawableObjects[i]->_boundingBox.isValid() && ray.intersectAABB(drawableObjects[i]->_boundingBox)) {
			if (ray.getOrigin().getDistanceTo(drawableObjects[i]->getOrigin()) < distance ) {
				collided = drawableObjects[i];
				distance = ray.getOrigin().getDistanceTo(collided->getOrigin());
			}
		}
	}
	return collided;
}

Object *Area::checkCollisions(const Math::AABB &boundingBox) {
	for (int i = drawableObjects.size() - 1; i >= 0; i--) {
		if (drawableObjects[i]->isDrawable() && !drawableObjects[i]->isDestroyed() && !drawableObjects[i]->isInvisible()) {
			GeometricObject *obj = (GeometricObject*) drawableObjects[i];
			if (obj->collides(boundingBox))
				return drawableObjects[i];
		}
	}
	return nullptr;
}

} // End of namespace Freescape