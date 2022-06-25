//
//  Area.h
//  Phantasma
//
//  Created by Thomas Harte on 25/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#ifndef __Phantasma__Area__
#define __Phantasma__Area__

#include "common/hashmap.h"
#include "common/array.h"

#include "math/vector3d.h"
#include "math/ray.h"

#include "freescape/gfx.h"
#include "freescape/objects/object.h"

typedef Common::HashMap<uint16, Object *> ObjectMap;

class BatchDrawer;
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
	uint16 getAreaID();
	uint8 getScale();
	void draw(Freescape::Renderer *gfx);
	void show();

	Object *shootRay(const Math::Ray &ray);

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

#endif /* defined(__Phantasma__Area__) */
