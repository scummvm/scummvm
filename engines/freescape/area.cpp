//
//  Area.cpp
//  Phantasma
//
//  Created by Thomas Harte on 25/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#include "common/algorithm.h"
#include "freescape/area.h"
#include "freescape/objects/object.h"

Object *Area::objectWithIDFromMap(ObjectMap *map, uint16 objectID)
{
	if(!map) return nullptr;
	//TODO //if(!map->count(objectID)) return nullptr;
	return (*map)[objectID];
}

Object *Area::objectWithID(uint16 objectID)
{
	return objectWithIDFromMap(objectsByID, objectID);
}

Object *Area::entranceWithID(uint16 objectID)
{
	return objectWithIDFromMap(entrancesByID, objectID);
}

uint16 Area::getAreaID()
{
	return areaID;
}

Area::Area(
	uint16 _areaID,
	ObjectMap *_objectsByID,
	ObjectMap *_entrancesByID)
{
	areaID = _areaID;
	objectsByID = _objectsByID;
	entrancesByID = _entrancesByID;
	vertexBuffer = nullptr;
	drawElementsBuffer = nullptr;

	// create a list of drawable objects only
	for(ObjectMap::iterator iterator = objectsByID->begin(); iterator != objectsByID->end(); iterator++)
	{
		if(iterator->_value->isDrawable())
		{
			drawableObjects.push_back(iterator->_value);
		}
	}

	// sort so that those that are planar are drawn last
	struct
	{
		bool operator() (Object *object1, Object *object2)
		{
			if(!object1->isPlanar() && object2->isPlanar()) return true;
			if(object1->isPlanar() && !object2->isPlanar()) return false;
			return object1->getObjectID() < object2->getObjectID();
		};
	} compareObjects;
	
	Common::sort(drawableObjects.begin(), drawableObjects.end(), compareObjects);
}

Area::~Area()
{
	for(ObjectMap::iterator iterator = entrancesByID->begin(); iterator != entrancesByID->end(); iterator++)
		delete iterator->_value;

	for(ObjectMap::iterator iterator = objectsByID->begin(); iterator != objectsByID->end(); iterator++)
		delete iterator->_value;

	delete entrancesByID;
	delete objectsByID;
}

/*void Area::setupOpenGL()
{
	delete vertexBuffer;
	vertexBuffer = GeometricObject::newVertexBuffer();

	delete drawElementsBuffer;
	drawElementsBuffer = GeometricObject::newDrawElementsBuffer();

	for(std::vector<Object *>::iterator iterator = drawableObjects.begin(); iterator != drawableObjects.end(); iterator++)
		(*iterator)->setupOpenGL(vertexBuffer, drawElementsBuffer);
}

void Area::draw(bool allowPolygonOffset, BatchDrawer *batchDrawer)
{
	for(std::vector<Object *>::iterator iterator = drawableObjects.begin(); iterator != drawableObjects.end(); iterator++)
	{
		(*iterator)->draw(vertexBuffer, drawElementsBuffer, batchDrawer, allowPolygonOffset);
	}
}*/
