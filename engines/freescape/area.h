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

#ifndef FREESCAPE_AREA_H
#define FREESCAPE_AREA_H

#include "math/ray.h"
#include "math/vector3d.h"

#include "freescape/language/instruction.h"
#include "freescape/objects/object.h"
#include "freescape/objects/group.h"


namespace Freescape {

typedef Common::HashMap<uint16, Object *> ObjectMap;
typedef Common::Array<Object *> ObjectArray;
class Area {
public:
	Area(uint16 areaID, uint16 areaFlags, ObjectMap *objectsByID, ObjectMap *entrancesByID);
	virtual ~Area();

	Common::String _name;
	Object *objectWithID(uint16 objectID);
	Object *entranceWithID(uint16 objectID);
	void changeObjectID(uint16 objectID, uint16 newObjectID);
	ObjectArray getSensors();
	uint16 getAreaID();
	uint16 getAreaFlags();
	uint8 getScale();
	void remapColor(int index, int color);
	void unremapColor(int index);
	void draw(Renderer *gfx, uint32 animationTicks, Math::Vector3d camera, Math::Vector3d direction);
	void drawGroup(Renderer *gfx, Group *group, bool runAnimation);
	void show();

	Object *checkCollisionRay(const Math::Ray &ray, int raySize);
	bool checkInSight(const Math::Ray &ray, float maxDistance);
	Math::Vector3d separateFromWall(const Math::Vector3d &position);
	ObjectArray checkCollisions(const Math::AABB &boundingBox);
	bool checkIfPlayerWasCrushed(const Math::AABB &boundingBox);
	Math::Vector3d resolveCollisions(Math::Vector3d const &lastPosition, Math::Vector3d const &newPosition, int playerHeight);
	void addObjectFromArea(int16 id, Area *global);
	void addGroupFromArea(int16 id, Area *global);
	void addObject(Object *obj);
	void addFloor();
	void addStructure(Area *global);
	void removeObject(int16 id);
	void resetArea();
	void resetAreaGroups();
	bool isOutside();
	bool hasActiveGroups();

	Common::Array<Common::String> _conditionSources;
	Common::Array<FCLInstructionVector> _conditions;

	// Serialization
	void saveObjects(Common::WriteStream *stream);
	void loadObjects(Common::SeekableReadStream *stream, Area *global);

	// Driller specific
	Common::Point _gasPocketPosition;
	uint32 _gasPocketRadius;

	uint8 _scale;
	uint8 _skyColor;
	uint8 _groundColor;
	uint8 _usualBackgroundColor;
	uint8 _underFireBackgroundColor;
	uint8 _inkColor;
	uint8 _paperColor;
	uint8 _extraColor[4];
	ColorReMap _colorRemaps;

	uint32 _lastTick;

private:
	uint16 _areaID;
	uint16 _areaFlags;
	ObjectMap *_objectsByID;
	ObjectMap *_entrancesByID;
	ObjectArray _drawableObjects;
	ObjectMap _addedObjects;
	Object *objectWithIDFromMap(ObjectMap *map, uint16 objectID);
};

} // End of namespace Freescape

#endif // FREESCAPE_AREA_H
