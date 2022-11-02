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

#ifndef ULTIMA8_KERNEL_OBJECTMANAGER_H
#define ULTIMA8_KERNEL_OBJECTMANAGER_H

#include "ultima/shared/std/containers.h"
#include "ultima/ultima8/misc/common_types.h"

namespace Ultima {
namespace Ultima8 {

class idMan;
class Object;
class Actor;

typedef Object *(*ObjectLoadFunc)(Common::ReadStream *rs, uint32);

class ObjectManager {
public:
	ObjectManager();
	~ObjectManager();

	static ObjectManager *get_instance() {
		return _objectManager;
	}

	void reset();

	uint16 assignObjId(Object *obj, ObjId id = 0xFFFF);
	uint16 assignActorObjId(Actor *obj, ObjId id = 0xFFFF);
	bool reserveObjId(ObjId objid);
	void clearObjId(ObjId objid);
	Object *getObject(ObjId objid) const;

	//! increase the maximum allowed object ID
	//! Note: this shouldn't be used in normal circumstances.
	//!       It exists for dumpMap currently. If that is rewritten not
	//!       to need more than 32768 object IDs, this function should be
	//!       deleted.
	void allow64kObjects();


	void objectStats();
	void objectTypes();

	void save(Common::WriteStream *ws);
	bool load(Common::ReadStream *rs, uint32 version);

	void saveObject(Common::WriteStream *ws, Object *obj) const;
	Object *loadObject(Common::ReadStream *rs, uint32 version);

	Std::vector<Object *> _objects;
	idMan *_objIDs;
	idMan *_actorIDs;

private:
	Object *loadObject(Common::ReadStream *rs, Std::string classname, uint32 version);

	void setupLoaders();

	void addObjectLoader(Std::string classname, ObjectLoadFunc func) {
		_objectLoaders[classname] = func;
	}
	Common::HashMap<Common::String, ObjectLoadFunc> _objectLoaders;

	static ObjectManager *_objectManager;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
