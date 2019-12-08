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

#include "ultima8/misc/pent_include.h"
#include "ultima8/kernel/object_manager.h"
#include "ultima8/std/containers.h"
#include "ultima8/misc/id_man.h"
#include "ultima8/kernel/object.h"
#include "ultima8/world/item.h"
#include "ultima8/world/actors/actor.h"
#include "ultima8/gumps/gump.h"
#include "ultima8/filesys/idata_source.h"
#include "ultima8/filesys/odata_source.h"
#include "ultima8/world/item_factory.h"
#include "ultima8/ultima8.h"

#include "ultima8/world/actors/main_actor.h"
#include "ultima8/world/egg.h"
#include "ultima8/world/monster_egg.h"
#include "ultima8/world/teleport_egg.h"
#include "ultima8/world/glob_egg.h"
#include "ultima8/gumps/game_map_gump.h"
#include "ultima8/gumps/desktop_gump.h"
#include "ultima8/gumps/console_gump.h"
#include "ultima8/gumps/ask_gump.h"
#include "ultima8/gumps/bark_gump.h"
#include "ultima8/gumps/container_gump.h"
#include "ultima8/gumps/paperdoll_gump.h"
#include "ultima8/gumps/widgets/text_widget.h"
#include "ultima8/gumps/widgets/button_widget.h"
#include "ultima8/gumps/widgets/sliding_widget.h"
#include "ultima8/gumps/mini_stats_gump.h"
#include "ultima8/gumps/minimap_gump.h"

namespace Ultima8 {

ObjectManager *ObjectManager::objectmanager = 0;


// a template class  to prevent having to write a load function for
// every object separately
template<class T>
struct ObjectLoader {
	static Object *load(IDataSource *ids, uint32 version) {
		T *p = new T();
		bool ok = p->loadData(ids, version);
		if (!ok) {
			delete p;
			p = 0;
		}
		return p;
	}
};

ObjectManager::ObjectManager() {
	con->Print(MM_INFO, "Creating ObjectManager...\n");

	objectmanager = this;

	setupLoaders();

	objects.resize(65536);

	//!CONSTANTS
	objIDs = new idMan(256, 32766, 8192); // Want range of 256 to 32766
	actorIDs = new idMan(1, 255, 255);
}

ObjectManager::~ObjectManager() {
	reset();
	con->Print(MM_INFO, "Destroying ObjectManager...\n");

	objectmanager = 0;

	delete objIDs;
	delete actorIDs;
}

void ObjectManager::reset() {
	con->Print(MM_INFO, "Resetting ObjectManager...\n");

	unsigned int i;

	for (i = 0; i < objects.size(); ++i) {
		if (objects[i] == 0) continue;
#if 0
		Item *item = p_dynamic_cast<Item *>(objects[i]);
		if (item && item->getParent()) continue; // will be deleted by parent
#endif
		Gump *gump = p_dynamic_cast<Gump *>(objects[i]);
		if (gump && gump->GetParent()) continue; // will be deleted by parent
		delete objects[i];
	}

	for (i = 0; i < objects.size(); ++i) {
		assert(objects[i] == 0);
	}


	//!CONSTANTS
	objects.clear();
	objects.resize(65536);
	objIDs->clearAll(32766);
	objIDs->reserveID(666);     // 666 is reserved for the Guardian Bark hack
	actorIDs->clearAll();
}

void ObjectManager::objectStats() {
	unsigned int i, npccount = 0, objcount = 0;

	//!constants
	for (i = 1; i < 256; i++) {
		if (objects[i] != 0)
			npccount++;
	}
	for (i = 256; i < objects.size(); i++) {
		if (objects[i] != 0)
			objcount++;
	}

	pout << "Object memory stats:" << std::endl;
	pout << "NPCs       : " << npccount << "/255" << std::endl;
	pout << "Objects    : " << objcount << "/32511" << std::endl;
}

void ObjectManager::objectTypes() {
	pout << "Current object types:" << std::endl;
	std::map<Common::String, unsigned int> objecttypes;
	for (unsigned int i = 1; i < objects.size(); ++i) {
		Object *o = objects[i];
		if (!o) continue;
		objecttypes[o->GetClassType().class_name]++;
	}

	std::map<Common::String, unsigned int>::iterator iter;
	for (iter = objecttypes.begin(); iter != objecttypes.end(); ++iter) {
		pout << (*iter)._key << ": " << (*iter)._value << std::endl;
	}
}

void ObjectManager::ConCmd_objectTypes(const Console::ArgvType & /*argv*/) {
	ObjectManager::get_instance()->objectTypes();
}

void ObjectManager::ConCmd_objectInfo(const Console::ArgvType &argv) {
	if (argv.size() != 2) {
		pout << "usage: objectInfo <objectnum>" << std::endl;
		return;
	}

	ObjectManager *objman = ObjectManager::get_instance();

	ObjId objid = static_cast<ObjId>(strtol(argv[1].c_str(), 0, 0));

	Object *obj = objman->getObject(objid);
	if (obj == 0) {
		bool reserved = false;
		if (objid >= 256) // CONSTANT!
			reserved = objman->objIDs->isIDUsed(objid);
		else
			reserved = objman->actorIDs->isIDUsed(objid);
		if (reserved)
			pout << "Reserved objid: " << objid << std::endl;
		else
			pout << "No such object: " << objid << std::endl;
	} else {
		obj->dumpInfo();
	}
}


uint16 ObjectManager::assignObjId(Object *obj, ObjId new_objid) {
	if (new_objid == 0xFFFF)
		new_objid = objIDs->getNewID();
	else
		objIDs->reserveID(new_objid);

	// failure???
	if (new_objid != 0) {
		assert(objects[new_objid] == 0);
		objects[new_objid] = obj;
	}
	return new_objid;
}

uint16 ObjectManager::assignActorObjId(Actor *actor, ObjId new_objid) {
	if (new_objid == 0xFFFF)
		new_objid = actorIDs->getNewID();
	else
		actorIDs->reserveID(new_objid);

	// failure???
	if (new_objid != 0) {
		assert(objects[new_objid] == 0);
		objects[new_objid] = actor;
	}
	return new_objid;
}

bool ObjectManager::reserveObjId(ObjId objid) {
	if (objid >= 256) // !constant
		return objIDs->reserveID(objid);
	else
		return actorIDs->reserveID(objid);
}

void ObjectManager::clearObjId(ObjId objid) {
	// need to make this assert check only permanent NPCs
//	assert(objid >= 256); // !constant
	if (objid >= 256) // !constant
		objIDs->clearID(objid);
	else
		actorIDs->clearID(objid);

	objects[objid] = 0;
}

Object *ObjectManager::getObject(ObjId objid) const {
	return objects[objid];
}

void ObjectManager::allow64kObjects() {
	objIDs->setNewMax(65534);
}


void ObjectManager::save(ODataSource *ods) {
	objIDs->save(ods);
	actorIDs->save(ods);

	for (unsigned int i = 0; i < objects.size(); ++i) {
		Object *object = objects[i];
		if (!object) continue;

		// child items/gumps are saved by their parent.
		Item *item = p_dynamic_cast<Item *>(object);
		if (item && item->getParent()) continue;
		Gump *gump = p_dynamic_cast<Gump *>(object);

		// don't save Gumps with DONT_SAVE and Gumps with parents, unless
		// the parent is a core gump
		// FIXME: This leaks objIDs. See comment in ObjectManager::load().
		if (gump && !gump->mustSave(true)) continue;

		object->save(ods);
	}

	ods->write2(0);
}


bool ObjectManager::load(IDataSource *ids, uint32 version) {
	if (!objIDs->load(ids, version)) return false;
	if (!actorIDs->load(ids, version)) return false;

	do {
		// peek ahead for terminator
		uint16 classlen = ids->read2();
		if (classlen == 0) break;
		char *buf = new char[classlen + 1];
		ids->read(buf, classlen);
		buf[classlen] = 0;

		std::string classname = buf;
		delete[] buf;

		Object *obj = loadObject(ids, classname, version);
		if (!obj) return false;

		// top level gumps have to be added to the correct core gump
		Gump *gump = p_dynamic_cast<Gump *>(obj);
		if (gump) {
			Ultima8Engine::get_instance()->addGump(gump);
		}

	} while (true);

	// ObjectManager::save() doesn't save Gumps with the DONT_SAVE flag, but
	// their IDs are still marked in use in objIDs.
	// As a workaround, we clear all IDs still in use without actual objects.
	// We only do this with IDs >= 1024 because below there are truly reserved
	// objIDs (up to 511 is reserved by U8Game, 666 is reserved for Guardian
	// barks).
	// FIXME: Properly fix this objID leak and increment the savegame number.
	//        This check can then be turned into an savegame corruption check
	//        for saves with the new savegame version.
	// We also fail loading when we're out of objIDs since this could
	// have caused serious issues when critical objects haven't been created.
	if (objIDs->isFull()) {
		perr << "Savegame has been corrupted by running out of objIDs."
		     << std::endl;
		return false;
	}
	unsigned int count = 0;
	for (unsigned int i = 1024; i < objects.size(); i++) {
		if (objects[i] == 0 && objIDs->isIDUsed(i)) {
			objIDs->clearID(i);
			count++;
		}
	}
	pout << "Reclaimed " << count << " objIDs on load." << std::endl;

	return true;
}

Object *ObjectManager::loadObject(IDataSource *ids, uint32 version) {
	uint16 classlen = ids->read2();
	char *buf = new char[classlen + 1];
	ids->read(buf, classlen);
	buf[classlen] = 0;

	std::string classname = buf;
	delete[] buf;

	return loadObject(ids, classname, version);
}

Object *ObjectManager::loadObject(IDataSource *ids, std::string classname,
                                  uint32 version) {
	std::map<Common::String, ObjectLoadFunc>::iterator iter;
	iter = objectloaders.find(classname);

	if (iter == objectloaders.end()) {
		perr << "Unknown Object class: " << classname << std::endl;
		return 0;
	}

	Object *obj = (*(iter->_value))(ids, version);

	if (!obj) {
		perr << "Error loading object of type " << classname << std::endl;
		return 0;
	}
	uint16 objid = obj->getObjId();

	if (objid != 0xFFFF) {
		objects[objid] = obj;
		bool used;
		if (objid >= 256)
			used = objIDs->isIDUsed(objid);
		else
			used = actorIDs->isIDUsed(objid);
		if (!used) {
			perr << "Error: object ID " << objid
			     << " used but marked available. " << std::endl;
			return 0;
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
}

} // End of namespace Ultima8
