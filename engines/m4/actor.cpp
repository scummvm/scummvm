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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/system.h"
#include "common/array.h"
#include "common/textconsole.h"
#include "m4/actor.h"
#include "m4/m4_views.h"
#include "m4/assets.h"

namespace M4 {

#define WALKER_BURGER "Wilbur0%i"	// wilbur, with a number denoting his current direction

Actor::Actor(MadsM4Engine *vm) : _vm(vm) {
	_scaling = 100;
	_direction = 5;
	_walkerSprites.resize(10);
	loadWalkers();
}

Actor::~Actor() {
	unloadWalkers();
}

int Actor::getWalkerWidth() { return _walkerSprites[kFacingSouth]->getFrame(0)->width(); }
int Actor::getWalkerHeight() { return _walkerSprites[kFacingSouth]->getFrame(0)->height(); }

void Actor::placeWalkerSpriteAt(int spriteNum, int x, int y) {
	if (_direction < 1 || _direction > 9) {
		warning("Direction is %i, fixing", _direction);
		_direction = 1;		// TODO: this is a temporary fix
	}
	SpriteInfo info;
	info.sprite = _walkerSprites[_direction]->getFrame(spriteNum);
	info.hotX = info.hotY = 0;
	info.width = info.sprite->width();
	info.height = info.sprite->height();
	info.scaleX = info.scaleY = _scaling;
	info.palette = _walkerSprites[_direction]->getPalette();
	info.inverseColorTable = _m4Vm->scene()->getInverseColorTable();

	_vm->_scene->drawSprite(x, y, info, Common::Rect(640, 400));
}

void Actor::loadWalkers() {
	for (uint8 i = 1; i < 10; i++) {
		if (i == 6)
			continue;	// walker sprite 6 is unused
		loadWalkerDirection(i);
	}
}

void Actor::loadWalkerDirection(uint8 direction) {
	char name[20];
	Common::SeekableReadStream *walkerS;

	if (_vm->getGameType() == GType_Burger) {
		sprintf(name, WALKER_BURGER, direction);
	} else {
		//warning("Actor::loadWalkerDirection: unspecified walker type, not loading walker");
		// TODO: Master Lu walkers
		return;
	}

	walkerS = _vm->res()->get(name);
	_walkerSprites.insert_at(direction, new SpriteAsset(_vm, walkerS, walkerS->size(), name));
	_vm->res()->toss(name);
}

void Actor::unloadWalkers() {
	for (uint8 i = 9; i > 0; i--) {
		if (i == 6)
			continue;	// walker sprite 6 is unused
		SpriteAsset *tempSprite = _walkerSprites[i];
		_walkerSprites.remove_at(i);
		delete tempSprite;
	}
}

void Actor::setWalkerPalette() {
	_vm->_palette->setPalette(_walkerSprites[kFacingSouthEast]->getPalette(), 0,
							  _walkerSprites[kFacingSouthEast]->getColorCount());
}

Inventory::Inventory(MadsM4Engine *vm) : _vm(vm) {
}

Inventory::~Inventory() {
	_inventory.clear();
}

void Inventory::registerObject(char* name, int32 scene, int32 icon) {
	InventoryObject *newObject = new InventoryObject();
	int newObjectIndex = 0;

	// Capitalize registered inventory object names
	str_upper(name);

	newObject->name = strdup(name);
	newObject->scene = scene;
	newObject->icon = icon;

	newObjectIndex = _inventory.size();

	_inventory.push_back(newObject);

	if (scene == BACKPACK)
		addToBackpack(newObjectIndex);
}

void Inventory::moveObject(char* name, int32 scene) {
	uint i = 0;

	for (i = 0; i < _inventory.size(); i++) {
		if (!scumm_stricmp(_inventory[i]->name, name)) {
			if (_inventory[i]->scene == BACKPACK && scene != BACKPACK)
				removeFromBackpack(i);

			_inventory[i]->scene = scene;

			if (scene == BACKPACK)
				addToBackpack(i);

			return;
		}
	}
}

void Inventory::addToBackpack(uint32 objectIndex) {
	_m4Vm->scene()->getInterface()->inventoryAdd(_inventory[objectIndex]->name, "", _inventory[objectIndex]->icon);
}

void Inventory::removeFromBackpack(uint32 objectIndex) {
	_m4Vm->scene()->getInterface()->inventoryRemove(_inventory[objectIndex]->name);
}

bool Inventory::isInCurrentScene(char* name) {
	return (getScene(name) == _vm->_scene->getCurrentScene());
}

int Inventory::getScene(char* name) {
	uint i = 0;

	for (i = 0; i < _inventory.size(); i++) {
		if (!scumm_stricmp(_inventory[i]->name, name))
			return _inventory[i]->scene;
	}
	return UNKNOWN_OBJECT;
}

int Inventory::getIcon(char* name) {
	uint i = 0;

	for (i = 0; i < _inventory.size(); i++) {
		if (!scumm_stricmp(_inventory[i]->name, name))
			return _inventory[i]->icon;
	}
	return UNKNOWN_OBJECT;
}

int Inventory::getIndex(char* name) {
	uint i = 0;

	for (i = 0; i < _inventory.size(); i++) {
		if (!scumm_stricmp(_inventory[i]->name, name))
			return i;
	}
	return UNKNOWN_OBJECT;
}

void Inventory::clear() {
	for (uint i = 0; i < _inventory.size(); i++) {
		delete _inventory[i]->name;
		delete _inventory[i];
		_inventory.remove_at(i);
	}
}

} // End of namespace M4
