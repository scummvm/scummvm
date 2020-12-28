/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/kernel/object_manager.h"
#include "ultima/shared/std/containers.h"
#include "ultima/ultima8/misc/id_man.h"
#include "ultima/ultima8/kernel/object.h"
#include "ultima/ultima8/world/item.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/gumps/gump.h"
#include "ultima/ultima8/world/item_factory.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/world/egg.h"
#include "ultima/ultima8/world/monster_egg.h"
#include "ultima/ultima8/world/teleport_egg.h"
#include "ultima/ultima8/world/glob_egg.h"
#include "ultima/ultima8/gumps/game_map_gump.h"
#include "ultima/ultima8/gumps/desktop_gump.h"
#include "ultima/ultima8/gumps/ask_gump.h"
#include "ultima/ultima8/gumps/bark_gump.h"
#include "ultima/ultima8/gumps/container_gump.h"
#include "ultima/ultima8/gumps/paperdoll_gump.h"
#include "ultima/ultima8/gumps/widgets/text_widget.h"
#include "ultima/ultima8/gumps/widgets/button_widget.h"
#include "ultima/ultima8/gumps/widgets/sliding_widget.h"
#include "ultima/ultima8/gumps/mini_stats_gump.h"
#include "ultima/ultima8/gumps/minimap_gump.h"
#include "ultima/ultima8/gumps/cru_status_gump.h"
#include "ultima/ultima8/gumps/cru_pickup_gump.h"
#include "ultima/ultima8/gumps/cru_pickup_area_gump.h"
#include "ultima/ultima8/gumps/translucent_gump.h"

namespace Ultima {
namespace Ultima8 {

ObjectManager *ObjectManager::_objectManager = nullptr;


// a template class  to prevent having to write a load function for
// every object separately
template<class T>
struct ObjectLoader {
	static Object *load(Common::ReadStream *rs, uint32 version) {
		T *p = new T();
		bool ok = p->loadData(rs, version);
		if (!ok) {
			delete p;
			p = nullptr;
		}
		return p;
	}
};

ObjectManager::ObjectManager() {
	debugN(MM_INFO, "Creating ObjectManager...\n");

	_objectManager = this;

	setupLoaders();

	_objects.resize(65536);

	//!CONSTANTS
	_objIDs = new idMan(256, 32766, 8192); // Want range of 256 to 32766
	_actorIDs = new idMan(1, 255, 255);
}

ObjectManager::~ObjectManager() {
	reset();
	debugN(MM_INFO, "Destroying ObjectManager...\n");

	_objectManager = nullptr;

	delete _objIDs;
	delete _actorIDs;
}

void ObjectManager::reset() {
	debugN(MM_INFO, "Resetting ObjectManager...\n");

	unsigned int i;

	for (i = 0; i < _objects.size(); ++i) {
		if (_objects[i] == nullptr) continue;
#if 0
		Item *item = dynamic_cast<Item *>(_objects[i]);
		if (item && item->getParent()) continue; // will be deleted by parent
#endif
		Gump *gump = dynamic_cast<Gump *>(_objects[i]);
		if (gump && gump->GetParent()) continue; // will be deleted by parent
		delete _objects[i];
	}

	for (i = 0; i < _objects.size(); ++i) {
		assert(_objects[i] == nullptr);
	}


	//!CONSTANTS
	_objects.clear();
	_objects.resize(65536);
	_objIDs->clearAll(32766);
	_objIDs->reserveID(666);     // 666 is reserved for the Guardian Bark hack
	_actorIDs->clearAll();
}

void ObjectManager::objectStats() {
	unsigned int i, npccount = 0, objcount = 0;

	//!constants
	for (i = 1; i < 256; i++) {
		if (_objects[i] != nullptr)
			npccount++;
	}
	for (i = 256; i < _objects.size(); i++) {
		if (_objects[i] != nullptr)
			objcount++;
	}

	g_debugger->debugPrintf("Object memory stats:\n");
	g_debugger->debugPrintf("NPCs       : %u/255\n", npccount);
	g_debugger->debugPrintf("Objects    : %u/32511\n", objcount);
}

void ObjectManager::objectTypes() {
	g_debugger->debugPrintf("Current object types:\n");
	Std::map<Common::String, unsigned int> objecttypes;
	for (unsigned int i = 1; i < _objects.size(); ++i) {
		Object *o = _objects[i];
		if (!o) continue;
		objecttypes[o->GetClassType()._className]++;
	}

	Std::map<Common::String, unsigned int>::const_iterator iter;
	for (iter = objecttypes.begin(); iter != objecttypes.end(); ++iter) {
		g_debugger->debugPrintf("%s: %u\n", (*iter)._key.c_str(), (*iter)._value);
	}
}

uint16 ObjectManager::assignObjId(Object *obj, ObjId new_objid) {
	if (new_objid == 0xFFFF)
		new_objid = _objIDs->getNewID();
	else
		_objIDs->reserveID(new_objid);

	// failure???
	if (new_objid != 0) {
		assert(_objects[new_objid] == nullptr);
		_objects[new_objid] = obj;
	}
	return new_objid;
}

uint16 ObjectManager::assignActorObjId(Actor *actor, ObjId new_objid) {
	if (new_objid == 0xFFFF)
		new_objid = _actorIDs->getNewID();
	else
		_actorIDs->reserveID(new_objid);

	// failure???
	if (new_objid != 0) {
		assert(_objects[new_objid] == 0);
		_objects[new_objid] = actor;
	}
	return new_objid;
}

bool ObjectManager::reserveObjId(ObjId objid) {
	if (objid >= 256) // !constant
		return _objIDs->reserveID(objid);
	else
		return _actorIDs->reserveID(objid);
}

void ObjectManager::clearObjId(ObjId objid) {
	// need to make this assert check only permanent NPCs
//	assert(objid >= 256); // !constant
	if (objid >= 256) // !constant
		_objIDs->clearID(objid);
	else
		_actorIDs->clearID(objid);

	_objects[objid] = nullptr;
}

Object *ObjectManager::getObject(ObjId objid) const {
	return _objects[objid];
}

void ObjectManager::allow64kObjects() {
	_objIDs->setNewMax(65534);
}


void ObjectManager::save(Common::WriteStream *ws) {
	_objIDs->save(ws);
	_actorIDs->save(ws);

	for (unsigned int i = 0; i < _objects.size(); ++i) {
		Object *object = _objects[i];
		if (!object) continue;

		// child items/gumps are saved by their parent.
		Item *item = dynamic_cast<Item *>(object);
		if (item && item->getParent()) continue;
		Gump *gump = dynamic_cast<Gump *>(object);

		// don't save Gumps with DONT_SAVE and Gumps with parents, unless
		// the parent is a core gump
		// FIXME: This leaks _objIDs. See comment in ObjectManager::load().
		if (gump && !gump->mustSave(true)) continue;

		saveObject(ws, object);
	}
 
	ws->writeUint16LE(0);
}


bool ObjectManager::load(Common::ReadStream *rs, uint32 version) {
	if (!_objIDs->load(rs, version)) return false;
	if (!_actorIDs->load(rs, version)) return false;

	do {
		// peek ahead for terminator
		uint16 classlen = rs->readUint16LE();
		if (classlen == 0) break;
		char *buf = new char[classlen + 1];
		rs->read(buf, classlen);
		buf[classlen] = 0;

		Std::string classname = buf;
		delete[] buf;

		Object *obj = loadObject(rs, classname, version);
		if (!obj) return false;

		// top level gumps have to be added to the correct core gump
		Gump *gump = dynamic_cast<Gump *>(obj);
		if (gump) {
			assert(gump->GetParent() == nullptr);
			Ultima8Engine::get_instance()->addGump(gump);
		}

	} while (true);

	// ObjectManager::save() doesn't save Gumps with the DONT_SAVE flag, but
	// their IDs are still marked in use in _objIDs.
	// As a workaround, we clear all IDs still in use without actual _objects.
	// We only do this with IDs >= 1024 because below there are truly reserved
	// _objIDs (up to 511 is reserved by U8Game, 666 is reserved for Guardian
	// barks).
	// FIXME: Properly fix this objID leak and increment the savegame number.
	//        This check can then be turned into an savegame corruption check
	//        for saves with the new savegame version.
	// We also fail loading when we're out of _objIDs since this could
	// have caused serious issues when critical _objects haven't been created.
	if (_objIDs->isFull()) {
		perr << "Savegame has been corrupted by running out of _objIDs."
		     << Std::endl;
		return false;
	}
	unsigned int count = 0;
	for (unsigned int i = 1024; i < _objects.size(); i++) {
		if (_objects[i] == nullptr && _objIDs->isIDUsed(i)) {
			_objIDs->clearID(i);
			count++;
		}
	}
	pout << "Reclaimed " << count << " _objIDs on load." << Std::endl;

	return true;
}

void ObjectManager::saveObject(Common::WriteStream *ws, Object *obj) const {
	const Std::string & classname = obj->GetClassType()._className; // note: virtual

	Std::map<Common::String, ObjectLoadFunc>::iterator iter;
	iter = _objectLoaders.find(classname);
	if (iter == _objectLoaders.end()) {
		error("Object class cannot save without registered loader: %s", classname.c_str());
	}

	ws->writeUint16LE(classname.size());
	ws->write(classname.c_str(), classname.size());
	obj->saveData(ws);
}

Object *ObjectManager::loadObject(Common::ReadStream *rs, uint32 version) {
	uint16 classlen = rs->readUint16LE();
	char *buf = new char[classlen + 1];
	rs->read(buf, classlen);
	buf[classlen] = 0;

	Std::string classname = buf;
	delete[] buf;

	return loadObject(rs, classname, version);
}

Object *ObjectManager::loadObject(Common::ReadStream *rs, Std::string classname,
                                  uint32 version) {
	Std::map<Common::String, ObjectLoadFunc>::iterator iter;
	iter = _objectLoaders.find(classname);

	if (iter == _objectLoaders.end()) {
		perr << "Unknown Object class: " << classname << Std::endl;
		return nullptr;
	}

	Object *obj = (*(iter->_value))(rs, version);

	if (!obj) {
		perr << "Error loading object of type " << classname << Std::endl;
		return nullptr;
	}
	uint16 objid = obj->getObjId();

	if (objid != 0xFFFF) {
		_objects[objid] = obj;
		bool used;
		if (objid >= 256)
			used = _objIDs->isIDUsed(objid);
		else
			used = _actorIDs->isIDUsed(objid);
		if (!used) {
			perr << "Error: object ID " << objid
			     << " used but marked available. " << Std::endl;
			return nullptr;
		}
	}

	return obj;
}

void ObjectManager::setupLoaders() {
	addObjectLoader("Item", ObjectLoader<Item>::load);
	addObjectLoader("Container", ObjectLoader<Container>::load);
	addObjectLoader("Actor", ObjectLoader<Actor>::load);
	addObjectLoader("MainActor", ObjectLoader<MainActor>::load);
	addObjectLoader("Egg", ObjectLoader<Egg>::load);
	addObjectLoader("MonsterEgg", ObjectLoader<MonsterEgg>::load);
	addObjectLoader("TeleportEgg", ObjectLoader<TeleportEgg>::load);
	addObjectLoader("GlobEgg", ObjectLoader<GlobEgg>::load);
	addObjectLoader("Gump", ObjectLoader<Gump>::load);
	addObjectLoader("ItemRelativeGump", ObjectLoader<ItemRelativeGump>::load);
	addObjectLoader("AskGump", ObjectLoader<AskGump>::load);
	addObjectLoader("BarkGump", ObjectLoader<BarkGump>::load);
	addObjectLoader("ContainerGump", ObjectLoader<ContainerGump>::load);
	addObjectLoader("PaperdollGump", ObjectLoader<PaperdollGump>::load);
	addObjectLoader("TextWidget", ObjectLoader<TextWidget>::load);
	addObjectLoader("ButtonWidget", ObjectLoader<ButtonWidget>::load);
	addObjectLoader("SlidingWidget", ObjectLoader<SlidingWidget>::load);
	addObjectLoader("MiniStatsGump", ObjectLoader<MiniStatsGump>::load);
	addObjectLoader("MiniMapGump", ObjectLoader<MiniMapGump>::load);
	addObjectLoader("CruStatusGump", ObjectLoader<CruStatusGump>::load);
	addObjectLoader("CruPickupAreaGump", ObjectLoader<CruPickupAreaGump>::load);
	addObjectLoader("CruPickupGump", ObjectLoader<CruPickupGump>::load);
	addObjectLoader("TranslucentGump", ObjectLoader<TranslucentGump>::load);
}

} // End of namespace Ultima8
} // End of namespace Ultima
