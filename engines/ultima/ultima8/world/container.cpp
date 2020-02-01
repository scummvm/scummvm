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

#include "ultima/ultima8/world/container.h"

#include "ultima/ultima8/kernel/object_manager.h"
#include "ultima/ultima8/usecode/uc_machine.h"
#include "ultima/ultima8/usecode/uc_list.h"
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/odata_source.h"
#include "ultima/ultima8/world/item_factory.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/kernel/core_app.h"

#include "ultima/ultima8/graphics/shape_info.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(Container, Item)

Container::Container() {

}


Container::~Container() {
	// TODO: handle container's contents.
	// Either destroy the contents, or move them up to this container's parent?



	// if we don't have an objid, we _must_ delete children
	if (objid == 0xFFFF) {
		Std::list<Item *>::iterator iter;
		for (iter = contents.begin(); iter != contents.end(); ++iter) {
			delete(*iter);
		}
	}
}


ObjId Container::assignObjId() {
	ObjId id = Item::assignObjId();

	Std::list<Item *>::iterator iter;
	for (iter = contents.begin(); iter != contents.end(); ++iter) {
		(*iter)->assignObjId();
		(*iter)->setParent(id);
	}

	return id;
}

void Container::clearObjId() {
	Item::clearObjId();

	Std::list<Item *>::iterator iter;
	for (iter = contents.begin(); iter != contents.end(); ++iter) {
		// make sure we don't clear the ObjId of an Actor
		assert((*iter)->getObjId() >= 256);

		(*iter)->clearObjId();
	}
}


bool Container::CanAddItem(Item *item, bool checkwghtvol) {
	if (!item) return false;
	if (item->getParent() == this->getObjId()) return true; // already in here

	if (item->getObjId() < 256) return false; // actors don't fit in containers

	Container *c = p_dynamic_cast<Container *>(item);
	if (c) {
		// To quote Exult: "Watch for snake eating itself."
		Container *p = this;
		do {
			if (p == c)
				return false;
		} while ((p = p->getParentAsContainer()) != 0);
	}

	if (checkwghtvol) {

		uint32 volume = getContentVolume();
		uint32 capacity = getCapacity();

		// Because Avatar should be able to remove backpack and haul a barrel
		// or chest on his back instead, artificially increase backpack volume
		// for chests or barrels.
		uint32 shapeid = item->getShape();
		if (GAME_IS_U8 && (shapeid == 115 /*Barrel*/
		                   || shapeid == 78 || shapeid == 117 /*Chests*/)) {
			// TODO: make this off by default, but can enable it through
			// pentagram.ini
			MainActor *avatar = getMainActor();
			ObjId bp = avatar->getEquip(7); // !! constant
			Container *avatarbackpack = getContainer(bp);
			if (avatarbackpack == this) {
				capacity = 500;
			}
		}

		// FIXME: this check isn't entirely correct. (combining items,...?)
		if (volume + item->getVolume() > capacity)
			return false;

		Item *p = getTopItem();
		Item *current = item->getTopItem();

		// From outside to inside Avatar's inventory?
		if (p->getObjId() == 1 && current->getObjId() != 1) {
			MainActor *av = getMainActor();
			unsigned int str = av->getStr();
			// FIXME: this check isn't entirely correct. (combining items,...?)
			//CONSTANT!
			if (p->getTotalWeight() + item->getTotalWeight() > 40 * str)
				return false;
		}
	}

	return true;
}

bool Container::addItem(Item *item, bool checkwghtvol) {
	if (!CanAddItem(item, checkwghtvol)) return false;
	if (item->getParent() == objid) return true; // already in here

	contents.push_back(item);
	return true;
}


bool Container::removeItem(Item *item) {
	Std::list<Item *>::iterator iter;

	for (iter = contents.begin(); iter != contents.end(); ++iter) {
		if (*iter == item) {
			contents.erase(iter);
			return true;
		}
	}
	return false;
}

bool Container::moveItemToEnd(Item *item) {
	Std::list<Item *>::iterator iter;

	for (iter = contents.begin(); iter != contents.end(); ++iter) {
		if (*iter == item) {
			// found; move to end
			contents.erase(iter);
			contents.push_back(item);
			return true;
		}
	}

	// not found

	return false;
}

void Container::removeContents() {
	// CHECKME: ethereal items?

	Container *parentCon = getParentAsContainer();
	if (parentCon) {
		// move contents to parent
		while (contents.begin() != contents.end()) {
			Item *item = *(contents.begin());
			item->moveToContainer(parentCon);
		}
	} else {
		// move contents to our coordinates
		while (contents.begin() != contents.end()) {
			Item *item = *(contents.begin());
			item->move(x, y, z);
		}
	}
}


void Container::destroyContents() {
	while (contents.begin() != contents.end()) {
		Item *item = *(contents.begin());
		Container *cont = p_dynamic_cast<Container *>(item);
		if (cont) cont->destroyContents();
		item->destroy(true); // we destroy the item immediately
	}
}

void Container::setFlagRecursively(uint32 mask) {
	setFlag(mask);

	Std::list<Item *>::iterator iter;
	for (iter = contents.begin(); iter != contents.end(); ++iter) {
		(*iter)->setFlag(mask);
		Container *cont = p_dynamic_cast<Container *>(*iter);
		if (cont) cont->setFlagRecursively(mask);
	}
}

void Container::destroy(bool delnow) {
	//! What do we do with our contents?
	//! (in Exult we remove the contents)

	removeContents();

	Item::destroy(delnow);
}

uint32 Container::getTotalWeight() {
	uint32 weight = Item::getTotalWeight();

	// CONSTANT!
	if (GAME_IS_U8 && getShape() == 79) {
		// contents of keyring don't weigh anything
		return weight;
	}

	// CONSTANT!
	if (GAME_IS_U8 && getShape() == 115) {
		// barrel weight is unreasonably heavy
		weight = 300;
	}

	Std::list<Item *>::iterator iter;

	for (iter = contents.begin(); iter != contents.end(); ++iter) {
		weight += (*iter)->getTotalWeight();
	}

	return weight;
}

uint32 Container::getCapacity() {
	uint32 volume = getShapeInfo()->volume;

	return (volume == 0) ? 32 : volume;
}

uint32 Container::getContentVolume() {
	uint32 volume = 0;

	Std::list<Item *>::iterator iter;

	for (iter = contents.begin(); iter != contents.end(); ++iter) {
		volume += (*iter)->getVolume();
	}

	return volume;
}

void Container::containerSearch(UCList *itemlist, const uint8 *loopscript,
                                uint32 scriptsize, bool recurse) {
	Std::list<Item *>::iterator iter;
	for (iter = contents.begin(); iter != contents.end(); ++iter) {
		// check item against loopscript
		if ((*iter)->checkLoopScript(loopscript, scriptsize)) {
			uint16 oId = (*iter)->getObjId();
			uint8 buf[2];
			buf[0] = static_cast<uint8>(oId);
			buf[1] = static_cast<uint8>(oId >> 8);
			itemlist->append(buf);
		}

		if (recurse) {
			// recurse into child-containers
			Container *container = p_dynamic_cast<Container *>(*iter);
			if (container)
				container->containerSearch(itemlist, loopscript,
				                           scriptsize, recurse);
		}
	}
}

void Container::dumpInfo() {
	Item::dumpInfo();

	pout << "Volume: " << getContentVolume() << "/" << getCapacity()
	     << ", total weight: " << getTotalWeight() << Std::endl;
}

void Container::saveData(ODataSource *ods) {
	Item::saveData(ods);
	ods->write4(static_cast<uint32>(contents.size()));
	Std::list<Item *>::iterator iter;
	for (iter = contents.begin(); iter != contents.end(); ++iter) {
		(*iter)->save(ods);
	}
}

bool Container::loadData(IDataSource *ids, uint32 version) {
	if (!Item::loadData(ids, version)) return false;

	uint32 contentcount = ids->read4();

	// read contents
	for (unsigned int i = 0; i < contentcount; ++i) {
		Object *obj = ObjectManager::get_instance()->loadObject(ids, version);
		Item *item = p_dynamic_cast<Item *>(obj);
		if (!item) return false;

		addItem(item);
		item->setParent(objid);
	}

	return true;
}


uint32 Container::I_removeContents(const uint8 *args, unsigned int /*argsize*/) {
	ARG_CONTAINER_FROM_PTR(container);
	if (!container) return 0;

	container->removeContents();
	return 0;
}

uint32 Container::I_destroyContents(const uint8 *args, unsigned int /*argsize*/) {
	ARG_CONTAINER_FROM_PTR(container);
	if (!container) return 0;

	container->destroyContents();
	return 0;
}

} // End of namespace Ultima8
} // End of namespace Ultima
