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

#include "sherlock/inventory.h"
#include "sherlock/sherlock.h"
#include "sherlock/scalpel/scalpel_inventory.h"
#include "sherlock/scalpel/scalpel_user_interface.h"
#include "sherlock/tattoo/tattoo_inventory.h"

namespace Sherlock {

InventoryItem::InventoryItem(int requiredFlag, const Common::String &name,
		const Common::String &description, const Common::String &examine) :
		_requiredFlag(requiredFlag), _requiredFlag1(0), _name(name), _description(description),
		_examine(examine), _lookFlag(0) {
}

InventoryItem::InventoryItem(int requiredFlag, const Common::String &name,
		const Common::String &description, const Common::String &examine, const Common::String &verbName) :
		_requiredFlag(requiredFlag), _requiredFlag1(0), _name(name), _description(description),
		_examine(examine), _lookFlag(0) {
	_verb._verb = verbName;
}

void InventoryItem::synchronize(Serializer &s) {
	s.syncAsSint16LE(_requiredFlag);
	s.syncAsSint16LE(_lookFlag);
	s.syncString(_name);
	s.syncString(_description);
	s.syncString(_examine);
	_verb.synchronize(s);
}

/*----------------------------------------------------------------*/

Inventory *Inventory::init(SherlockEngine *vm) {
	if (vm->getGameID() == GType_SerratedScalpel)
		return new Scalpel::ScalpelInventory(vm);
	else
		return new Tattoo::TattooInventory(vm);
}

Inventory::Inventory(SherlockEngine *vm) : Common::Array<InventoryItem>(), _vm(vm) {
	_invGraphicsLoaded = false;
	_invIndex = 0;
	_holdings = 0;
	_invMode = INVMODE_EXIT;
}

Inventory::~Inventory() {
	freeGraphics();
}

void Inventory::freeInv() {
	freeGraphics();

	_names.clear();
	_invGraphicsLoaded = false;
}

void Inventory::freeGraphics() {
	int count = _invShapes.size();
	for (int idx = 0; idx < count; ++idx)
		delete _invShapes[idx];
	_invShapes.clear();
	_invShapes.resize(count);

	_invGraphicsLoaded = false;
}

void Inventory::loadGraphics() {
	if (_invGraphicsLoaded)
		return;

	for (int idx = _invIndex; (idx < _holdings) && (idx - _invIndex) < (int)_invShapes.size(); ++idx) {
		// Get the name of the item to be displayed, figure out its accompanying
		// .VGS file with its picture, and then load it
		int invNum = findInv((*this)[idx]._name);
		Common::String filename = Common::String::format("item%02d.vgs", invNum + 1);

		if (!IS_3DO) {
			// PC
			_invShapes[idx - _invIndex] = new ImageFile(filename);
		} else {
			_invShapes[idx - _invIndex] = new ImageFile3DO(filename, kImageFile3DOType_RoomFormat);
		}
	}

	_invGraphicsLoaded = true;
}

int Inventory::findInv(const Common::String &name) {
	for (int idx = 0; idx < (int)_names.size(); ++idx) {
		if (name.equalsIgnoreCase(_names[idx]))
			return idx;
	}

	// Couldn't find the desired item
	error("Couldn't find inventory item - %s", name.c_str());
}

int Inventory::putNameInInventory(const Common::String &name) {
	Scene &scene = *_vm->_scene;
	int matches = 0;

	for (uint idx = 0; idx < scene._bgShapes.size(); ++idx) {
		Object &o = scene._bgShapes[idx];
		if (name.equalsIgnoreCase(o._name) && o._type != INVALID) {
			putItemInInventory(o);
			++matches;
		}
	}

	return matches;
}

int Inventory::putItemInInventory(Object &obj) {
	Scene &scene = *_vm->_scene;
	int matches = 0;
	bool pickupFound = false;

	if (obj._pickupFlag)
		_vm->setFlags(obj._pickupFlag);

	for (int useNum = 0; useNum < USE_COUNT; ++useNum) {
		if (obj._use[useNum]._target.equalsIgnoreCase("*PICKUP*")) {
			pickupFound = true;

			for (int namesNum = 0; namesNum < NAMES_COUNT; ++namesNum) {
				for (uint bgNum = 0; bgNum < scene._bgShapes.size(); ++bgNum) {
					Object &bgObj = scene._bgShapes[bgNum];
					if (obj._use[useNum]._names[namesNum].equalsIgnoreCase(bgObj._name)) {
						copyToInventory(bgObj);
						if (bgObj._pickupFlag)
							_vm->setFlags(bgObj._pickupFlag);

						if (bgObj._type == ACTIVE_BG_SHAPE || bgObj._type == NO_SHAPE || bgObj._type == HIDE_SHAPE) {
							if (bgObj._imageFrame == nullptr || bgObj._frameNumber < 0)
								// No shape to erase, so flag as hidden
								bgObj._type = INVALID;
							else
								bgObj._type = REMOVE;
						} else if (bgObj._type == HIDDEN) {
							bgObj._type = INVALID;
						}

						++matches;
					}
				}
			}
		}
	}

	if (!pickupFound) {
		// No pickup item found, so add the passed item
		copyToInventory(obj);
		matches = 0;
	}

	if (matches == 0) {
		if (!pickupFound)
			matches = 1;

		if (obj._type == ACTIVE_BG_SHAPE || obj._type == NO_SHAPE || obj._type == HIDE_SHAPE) {
			if (obj._imageFrame == nullptr || obj._frameNumber < 0)
				// No shape to erase, so flag as hidden
				obj._type = INVALID;
			else
				obj._type = REMOVE;
		} else if (obj._type == HIDDEN) {
			obj._type = INVALID;
		}
	}

	return matches;
}

void Inventory::copyToInventory(Object &obj) {
	InventoryItem invItem;
	invItem._name = obj._name;
	invItem._description = obj._description;
	invItem._examine = obj._examine;
	invItem._lookFlag = obj._lookFlag;
	invItem._requiredFlag = obj._requiredFlag[0];

	insert_at(_holdings, invItem);
	++_holdings;
}

int Inventory::deleteItemFromInventory(const Common::String &name) {
	int invNum = -1;

	for (int idx = 0; idx < (int)size() && invNum == -1; ++idx) {
		if (name.equalsIgnoreCase((*this)[idx]._name))
			invNum = idx;
	}

	if (invNum == -1)
		// Item not present
		return 0;

	// Item found, so delete it
	remove_at(invNum);
	--_holdings;

	return 1;
}

void Inventory::synchronize(Serializer &s) {
	s.syncAsSint16LE(_holdings);

	uint count = size();
	s.syncAsUint16LE(count);
	if (s.isLoading()) {
		resize(count);

		// Reset inventory back to start
		_invIndex = 0;
	}

	for (uint idx = 0; idx < size(); ++idx) {
		(*this)[idx].synchronize(s);

	}
}

} // End of namespace Sherlock
