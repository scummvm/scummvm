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

#include "bladerunner/items.h"

#include "bladerunner/scene.h"
#include "bladerunner/scene_objects.h"
#include "bladerunner/zbuffer.h"

namespace BladeRunner {

Items::Items(BladeRunnerEngine *vm) {
	_vm = vm;
}

Items::~Items() {
	for (int i = _items.size() - 1; i >= 0; i--) {
		delete _items.remove_at(i);
	}
}

void Items::getXYZ(int itemId, float *x, float *y, float *z) {
	int itemIndex = findItem(itemId);
	assert(itemIndex != -1);

	_items[itemIndex]->getXYZ(x, y, z);
}

void Items::getWidthHeight(int itemId, int *width, int *height) {
	int itemIndex = findItem(itemId);
	assert(itemIndex != -1);

	_items[itemIndex]->getWidthHeight(width, height);
}

void Items::tick() {
	int setId = _vm->_scene->getSetId();
	for (int i = 0; i < (int)_items.size(); i++) {
		if (_items[i]->_setId != setId) {
			continue;
		}
		bool set14NotTarget = setId == 14 && !_items[i]->isTargetable();
		Common::Rect screenRect;
		if (_items[i]->tick(&screenRect, set14NotTarget)) {
			_vm->_zbuffer->mark(screenRect);
		}
	}
}

bool Items::addToWorld(int itemId, int animationId, int setId, Vector3 position, int facing, int height, int width, bool isTargetableFlag, bool isVisibleFlag, bool isPoliceMazeEnemyFlag, bool addToSetFlag) {
	if (_items.size() >= 100) {
		return false;
	}
	int itemIndex = findItem(itemId);
	if (itemIndex == -1) {
		itemIndex = _items.size();
	}

	Item *item = new Item(_vm);
	item->setup(itemId, setId, animationId, position, facing, height, width, isTargetableFlag, isVisibleFlag, isPoliceMazeEnemyFlag);
	_items.push_back(item);

	if (addToSetFlag && setId == _vm->_scene->getSetId()) {
		return _vm->_sceneObjects->addItem(itemId + SCENE_OBJECTS_ITEMS_OFFSET, &item->_boundingBox, &item->_screenRectangle, isTargetableFlag, isVisibleFlag);
	}
	return true;
}

bool Items::addToSet(int setId) {
	int itemsCount = _vm->_items->_items.size();
	if (itemsCount == 0) {
		return true;
	}
	for (int i = 0; i < itemsCount; i++) {
		Item *item = _vm->_items->_items[i];
		if (item->_setId == setId) {
			_vm->_sceneObjects->addItem(item->_itemId + SCENE_OBJECTS_ITEMS_OFFSET, &item->_boundingBox, &item->_screenRectangle, item->isTargetable(), item->_isVisible);
		}
	}
	return true;
}

bool Items::remove(int itemId) {
	if (_items.size() == 0) {
		return false;
	}
	int itemIndex = findItem(itemId);
	if (itemIndex == -1) {
		return false;
	}

	if (_items[itemIndex]->_setId == _vm->_scene->getSetId()) {
		_vm->_sceneObjects->remove(itemId + SCENE_OBJECTS_ITEMS_OFFSET);
	}
	_items.remove_at(itemIndex);
	return true;
}

int Items::findItem(int itemId) {
	for (int i = 0; i < (int)_items.size(); i++) {
		if (_items[i]->_itemId == itemId)
			return i;
	}
	return -1;
}

} // End of namespace BladeRunner
