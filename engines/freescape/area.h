//
//  Area.h
//  Phantasma
//
//  Created by Thomas Harte on 25/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#ifndef FREESCAPE_AREA_H
#define FREESCAPE_AREA_H

#include "common/hashmap.h"
#include "common/array.h"

#include "math/vector3d.h"
#include "math/ray.h"

#include "freescape/gfx.h"
#include "freescape/objects/object.h"

namespace Freescape {

typedef Common::HashMap<uint16, Object *> ObjectMap;

class Area {
public:
	Area(
		uint16 areaID,
		ObjectMap *objectsByID,
		ObjectMap *entrancesByID,
		uint8 scale,
		uint8 skyColor,
		uint8 groundColor,
		Graphics::PixelBuffer *palette = nullptr);
	virtual ~Area();

	Object *objectWithID(uint16 objectID);
	Object *entranceWithID(uint16 objectID);
	Object *firstEntrance();
	uint16 getAreaID();
	uint8 getScale();
	void draw(Renderer *gfx);
	void show();

	Object *shootRay(const Math::Ray &ray);
	Object *checkCollisions(const Math::AABB &boundingBox);


private:
	uint16 areaID;
	uint8 scale;
	Graphics::PixelBuffer *palette;
	uint8 skyColor;
	uint8 groundColor;
	ObjectMap *objectsByID;
	ObjectMap *entrancesByID;
	Common::Array<Object *> drawableObjects;
	Object *objectWithIDFromMap(ObjectMap *map, uint16 objectID);
};

} // End of namespace Freescape

#endif
