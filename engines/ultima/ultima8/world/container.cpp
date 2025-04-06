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


#include "ultima/ultima8/world/container.h"

#include "ultima/ultima8/kernel/object_manager.h"
#include "ultima/ultima8/usecode/uc_machine.h"
#include "ultima/ultima8/usecode/uc_list.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/ultima8.h"


namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(Container)

Container::Container() {
}


Container::~Container() {
	// TODO: handle container's _contents.
	// Either destroy the _contents, or move them up to this container's parent?



	// if we don't have an _objId, we _must_ delete children
	if (_objId == 0xFFFF) {
		for (auto *item : _contents) {
			delete item;
		}
	}
}


ObjId Container::assignObjId() {
	ObjId id = Item::assignObjId();

	for (auto *item : _contents) {
		item->assignObjId();
		item->setParent(id);
	}

	return id;
}

void Container::clearObjId() {
	Item::clearObjId();

	for (auto *item : _contents) {
		// make sure we don't clear the ObjId of an Actor
		assert(item->getObjId() >= 256);

		item->clearObjId();
	}
}


bool Container::CanAddItem(Item *item, bool checkwghtvol) {
	if (!item) return false;
	if (item->getParent() == this->getObjId()) return true; // already in here

	if (item->getObjId() < 256) return false; // actors don't fit in containers

	Container *c = dynamic_cast<Container *>(item);
	if (c) {
		// To quote Exult: "Watch for snake eating itself."
		Container *p = this;
		do {
			if (p == c)
				return false;
		} while ((p = p->getParentAsContainer()) != nullptr);
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
			// TODO: make this off by default, but can enable it through config
			MainActor *avatar = getMainActor();
			ObjId bp = avatar->getEquip(ShapeInfo::SE_BACKPACK);
			Container *avatarbackpack = getContainer(bp);
			if (avatarbackpack == this) {
				capacity = 500;
			}
		}

		// FIXME: this check isn't entirely correct. (combining items,...?)
		if (volume + item->getVolume() > capacity)
			return false;

		const Item *p = getTopItem();
		const Item *current = item->getTopItem();

		// From outside to inside Avatar's inventory?
		if (p->getObjId() == kMainActorId && current->getObjId() != kMainActorId) {
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
	if (item->getParent() == _objId) return true; // already in here

	_contents.push_back(item);
	return true;
}


bool Container::removeItem(Item *item) {
	Std::list<Item *>::iterator iter;

	for (iter = _contents.begin(); iter != _contents.end(); ++iter) {
		if (*iter == item) {
			_contents.erase(iter);
			return true;
		}
	}
	return false;
}

bool Container::moveItemToEnd(Item *item) {
	Std::list<Item *>::iterator iter;

	for (iter = _contents.begin(); iter != _contents.end(); ++iter) {
		if (*iter == item) {
			// found; move to end
			_contents.erase(iter);
			_contents.push_back(item);
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
		// move _contents to parent
		while (_contents.begin() != _contents.end()) {
			Item *item = *(_contents.begin());
			item->moveToContainer(parentCon);
		}
	} else {
		// move _contents to our coordinates
		while (_contents.begin() != _contents.end()) {
			Item *item = *(_contents.begin());
			item->move(_x, _y, _z);
		}
	}
}


void Container::destroyContents() {
	while (_contents.begin() != _contents.end()) {
		Item *item = *(_contents.begin());
		assert(item);
		Container *cont = dynamic_cast<Container *>(item);
		if (cont) cont->destroyContents();
		item->destroy(true); // we destroy the item immediately
	}
}

void Container::setFlagRecursively(uint32 mask) {
	setFlag(mask);

	for (auto *item : _contents) {
		item->setFlag(mask);
		Container *cont = dynamic_cast<Container *>(item);
		if (cont) cont->setFlagRecursively(mask);
	}
}

void Container::destroy(bool delnow) {
	//! What do we do with our _contents?
	//! (in Exult we remove the _contents)

	removeContents();

	Item::destroy(delnow);
}

uint32 Container::getTotalWeight() const {
	uint32 weight = Item::getTotalWeight();

	// CONSTANT!
	if (GAME_IS_U8 && getShape() == 79) {
		// _contents of keyring don't weigh anything
		return weight;
	}

	// CONSTANT!
	if (GAME_IS_U8 && getShape() == 115) {
		// barrel weight is unreasonably heavy
		weight = 300;
	}

	for (const auto *item : _contents) {
		weight += item->getTotalWeight();
	}

	return weight;
}

uint32 Container::getCapacity() const {
	uint32 volume = getShapeInfo()->_volume;

	return (volume == 0) ? 32 : volume;
}

uint32 Container::getContentVolume() const {
	uint32 volume = 0;

	for (const auto *item : _contents) {
		volume += item->getVolume();
	}

	return volume;
}

void Container::containerSearch(UCList *itemlist, const uint8 *loopscript,
								uint32 scriptsize, bool recurse) const {
	for (auto *item : _contents) {
		// check item against loopscript
		if (item->checkLoopScript(loopscript, scriptsize)) {
			assert(itemlist->getElementSize() == 2);
			uint16 oId = item->getObjId();
			itemlist->appenduint16(oId);
		}

		if (recurse) {
			// recurse into child-containers
			Container *container = dynamic_cast<Container *>(item);
			if (container)
				container->containerSearch(itemlist, loopscript,
				                           scriptsize, recurse);
		}
	}
}

Item *Container::getFirstItemWithShape(uint16 shapeno, bool recurse) {
	for (auto *item : _contents) {
		if (item->getShape() == shapeno)
			return item;

		if (recurse) {
			// recurse into child-containers
			Container *container = dynamic_cast<Container *>(item);
			if (container) {
				Item *result = container->getFirstItemWithShape(shapeno, recurse);
				if (result)
					return result;
			}
		}
	}

	return nullptr;
}

void Container::getItemsWithShapeFamily(Std::vector<Item *> &itemlist, uint16 family, bool recurse) {
	for (auto *item : _contents) {
		if (item->getShapeInfo()->_family == family)
			itemlist.push_back(item);

		if (recurse) {
			// recurse into child-containers
			Container *container = dynamic_cast<Container *>(item);
			if (container) {
				container->getItemsWithShapeFamily(itemlist, family, recurse);
			}
		}
	}

}

Common::String Container::dumpInfo() const {
	return Item::dumpInfo() +
		Common::String::format("; Container vol: %u/%u, total weight: %u, items: %u",
			getContentVolume(), getCapacity(), getTotalWeight(), _contents.size());
}

void Container::saveData(Common::WriteStream *ws) {
	Item::saveData(ws);
	ws->writeUint32LE(static_cast<uint32>(_contents.size()));
	for (auto *item : _contents) {
		ObjectManager::get_instance()->saveObject(ws, item);
	}
}

bool Container::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Item::loadData(rs, version)) return false;

	uint32 contentcount = rs->readUint32LE();

	// read contents
	for (unsigned int i = 0; i < contentcount; ++i) {
		Object *obj = ObjectManager::get_instance()->loadObject(rs, version);
		Item *item = dynamic_cast<Item *>(obj);
		if (!item) return false;

		addItem(item);
		item->setParent(_objId);
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
