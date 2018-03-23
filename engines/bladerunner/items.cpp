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

#include "bladerunner/game_constants.h"
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

void Items::getXYZ(int itemId, float *x, float *y, float *z) const {
	int itemIndex = findItem(itemId);
	assert(itemIndex != -1);

	_items[itemIndex]->getXYZ(x, y, z);
}

void Items::getWidthHeight(int itemId, int *width, int *height) const {
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
		bool notPoliceMazeTarget = setId == kSetPS10_PS11_PS12_PS13 && !_items[i]->isTarget();
		Common::Rect screenRect;
		if (_items[i]->tick(&screenRect, notPoliceMazeTarget)) {
			_vm->_zbuffer->mark(screenRect);
		}
	}
}

bool Items::addToWorld(int itemId, int animationId, int setId, Vector3 position, int facing, int height, int width, bool isTargetFlag, bool isVisible, bool isPoliceMazeEnemy, bool addToSetFlag) {
	if (_items.size() >= 100) {
		return false;
	}
	int itemIndex = findItem(itemId);
	if (itemIndex == -1) {
		itemIndex = _items.size();
		_items.push_back(new Item(_vm));
	}

	Item *item = _items[itemIndex];
	item->setup(itemId, setId, animationId, position, facing, height, width, isTargetFlag, isVisible, isPoliceMazeEnemy);

	if (addToSetFlag && setId == _vm->_scene->getSetId()) {
		return _vm->_sceneObjects->addItem(itemId + kSceneObjectOffsetItems, &item->_boundingBox, &item->_screenRectangle, isTargetFlag, isVisible);
	}
	return true;
}

bool Items::addToSet(int setId) {
	int itemCount = _items.size();
	if (itemCount == 0) {
		return true;
	}
	for (int i = 0; i < itemCount; i++) {
		Item *item = _items[i];
		if (item->_setId == setId) {
			_vm->_sceneObjects->addItem(item->_itemId + kSceneObjectOffsetItems, &item->_boundingBox, &item->_screenRectangle, item->isTarget(), item->_isVisible);
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
		_vm->_sceneObjects->remove(itemId + kSceneObjectOffsetItems);
	}

	delete _items.remove_at(itemIndex);

	return true;
}

bool Items::isTarget(int itemId) const {
	int itemIndex = findItem(itemId);
	if (itemIndex == -1) {
		return false;
	}
	return _items[itemIndex]->isTarget();
}

bool Items::isPoliceMazeEnemy(int itemId) const {
	int itemIndex = findItem(itemId);
	if (itemIndex == -1) {
		return false;
	}
	return _items[itemIndex]->isTarget();
}

void Items::spinInWorld(int itemId) {
	int itemIndex = findItem(itemId);
	if (itemIndex == -1) {
		return;
	}
	_items[itemIndex]->spinInWorld();
}

int Items::findTargetUnderMouse(int mouseX, int mouseY) const {
	int setId = _vm->_scene->getSetId();
	for (int i = 0 ; i < (int)_items.size(); ++i) {
		if (_items[i]->_setId == setId && _items[i]->isTarget() && _items[i]->isUnderMouse(mouseX, mouseY)) {
			return _items[i]->_itemId;
		}
	}
	return -1;
}

int Items::findItem(int itemId) const {
	for (int i = 0; i < (int)_items.size(); i++) {
		if (_items[i]->_itemId == itemId) {
			return i;
		}
	}
	return -1;
}

} // End of namespace BladeRunner
