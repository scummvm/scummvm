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

#ifndef FREESCAPE_AREA_H
#define FREESCAPE_AREA_H

#include "math/vector3d.h"
#include "math/ray.h"

#include "freescape/objects/object.h"
#include "freescape/language/instruction.h"

namespace Freescape {

typedef Common::HashMap<uint16, Object *> ObjectMap;

class Area {
public:
	Area(
		uint16 areaID,
		uint16 areaFlags,
		ObjectMap *objectsByID,
		ObjectMap *entrancesByID,
		uint8 scale,
		uint8 skyColor,
		uint8 groundColor,
		Graphics::PixelBuffer *palette = nullptr);
	virtual ~Area();

	Common::String name;
	Object *objectWithID(uint16 objectID);
	Object *entranceWithID(uint16 objectID);
	uint16 getAreaID();
	uint16 getAreaFlags();
	uint8 getScale();
	void draw(Renderer *gfx);
	void show();

	Object *shootRay(const Math::Ray &ray);
	Object *checkCollisions(const Math::AABB &boundingBox);
	void addObjectFromArea(int16 id, Area *global);
	void addObject(Object *obj);
	void addStructure(Area *global);

	Common::Array<Common::String*> conditionSources;
	Common::Array<FCLInstructionVector> conditions;

	// Serialization
	void saveObjects(Common::WriteStream *stream);
	void loadObjects(Common::SeekableReadStream *stream, Area *global);

	// Driller specific
	void removeDrill();
	bool drillDeployed();
	Math::Vector3d drillPosition;
	Common::Point gasPocketPosition;
	uint32 gasPocketRadius;

private:
	uint16 areaID;
	uint16 areaFlags;
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
