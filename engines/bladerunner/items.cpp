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
#include "bladerunner/savefile.h"
#include "bladerunner/scene.h"
#include "bladerunner/scene_objects.h"
#include "bladerunner/zbuffer.h"

namespace BladeRunner {

Items::Items(BladeRunnerEngine *vm) {
	_vm = vm;
}

Items::~Items() {
	reset();
}

void Items::reset() {
	for (int i = _items.size() - 1; i >= 0; --i) {
		delete _items.remove_at(i);
	}
}

void Items::getXYZ(int itemId, float *x, float *y, float *z) const {
	int itemIndex = findItem(itemId);
	assert(itemIndex != -1);

	_items[itemIndex]->getXYZ(x, y, z);
}

void Items::setXYZ(int itemId, Vector3 position) {
	int itemIndex = findItem(itemId);
	assert(itemIndex != -1);

	_items[itemIndex]->setXYZ(position);
}

void Items::getWidthHeight(int itemId, int *width, int *height) const {
	int itemIndex = findItem(itemId);
	assert(itemIndex != -1);

	_items[itemIndex]->getWidthHeight(width, height);
}

void Items::getAnimationId(int itemId, int *animationId) const {
	int itemIndex = findItem(itemId);
	assert(itemIndex != -1);

	_items[itemIndex]->getAnimationId(animationId);
}

void Items::tick() {
	int setId = _vm->_scene->getSetId();
	for (int i = 0; i < (int)_items.size(); ++i) {
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

bool Items::addToWorld(int itemId, int animationId, int setId, Vector3 position, int facing, int height, int width, bool isTargetFlag, bool isVisibleFlag, bool isPoliceMazeEnemyFlag, bool addToSetFlag) {
	if (_items.size() >= 100) {
		return false;
	}
	int itemIndex = findItem(itemId);
	if (itemIndex == -1) {
		itemIndex = _items.size();
		_items.push_back(new Item(_vm));
	}

	Item *item = _items[itemIndex];
	item->setup(itemId, setId, animationId, position, facing, height, width, isTargetFlag, isVisibleFlag, isPoliceMazeEnemyFlag);

	if (addToSetFlag && setId == _vm->_scene->getSetId()) {
		return _vm->_sceneObjects->addItem(itemId + kSceneObjectOffsetItems, item->_boundingBox, item->_screenRectangle, isTargetFlag, isVisibleFlag);
	}
	return true;
}

bool Items::addToSet(int setId) {
	int itemCount = _items.size();
	if (itemCount == 0) {
		return true;
	}
	for (int i = 0; i < itemCount; ++i) {
		Item *item = _items[i];
		if (item->_setId == setId) {
			_vm->_sceneObjects->addItem(item->_itemId + kSceneObjectOffsetItems, item->_boundingBox, item->_screenRectangle, item->isTarget(), item->_isVisible);
		}
	}
	return true;
}

#if !BLADERUNNER_ORIGINAL_BUGS
bool Items::removeFromCurrentSceneOnly(int itemId) {
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
	return true;
}
#endif // !BLADERUNNER_ORIGINAL_BUGS

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

void Items::setIsTarget(int itemId, bool val) {
	int itemIndex = findItem(itemId);
	if (itemIndex == -1) {
		return;
	}
	_items[itemIndex]->setIsTarget(val);
	_vm->_sceneObjects->setIsTarget(itemId + kSceneObjectOffsetItems, val);
}

bool Items::isTarget(int itemId) const {
	int itemIndex = findItem(itemId);
	if (itemIndex == -1) {
		return false;
	}
	return _items[itemIndex]->isTarget();
}

bool Items::isSpinning(int itemId) const {
	int itemIndex = findItem(itemId);
	if (itemIndex == -1) {
		return false;
	}
	return _items[itemIndex]->isSpinning();
}

bool Items::isVisible(int itemId) const {
	int itemIndex = findItem(itemId);
	if (itemIndex == -1) {
		return false;
	}
	return _items[itemIndex]->isVisible();
}

bool Items::isPoliceMazeEnemy(int itemId) const {
	int itemIndex = findItem(itemId);
	if (itemIndex == -1) {
		return false;
	}
	return _items[itemIndex]->isPoliceMazeEnemy();
}

void Items::setPoliceMazeEnemy(int itemId, bool val) {
	int itemIndex = findItem(itemId);
	if (itemIndex == -1) {
		return;
	}
	_items[itemIndex]->setPoliceMazeEnemy(val);
}

void Items::setIsObstacle(int itemId, bool val) {
	int itemIndex = findItem(itemId);
	if (itemIndex == -1) {
		return;
	}
	_items[itemIndex]->setVisible(val);
	_vm->_sceneObjects->setIsClickable(itemId + kSceneObjectOffsetItems, val);
}

const BoundingBox &Items::getBoundingBox(int itemId) {
	int itemIndex = findItem(itemId);
	// if (itemIndex == -1) {
	// 	return nullptr;
	// }
	return _items[itemIndex]->getBoundingBox();
}

const Common::Rect &Items::getScreenRectangle(int itemId) {
	int itemIndex = findItem(itemId);
	// if (itemIndex == -1) {
	// 	return nullptr;
	// }
	return _items[itemIndex]->getScreenRectangle();
}

int Items::getFacing(int itemId) const {
	int itemIndex = findItem(itemId);
	if (itemIndex == -1) {
		return 0;
	}
	return _items[itemIndex]->getFacing();
}

void Items::setFacing(int itemId, int facing) {
	int itemIndex = findItem(itemId);
	if (itemIndex == -1) {
		return;
	}
	_items[itemIndex]->setFacing(facing);
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
	for (int i = 0; i < (int)_items.size(); ++i) {
		if (_items[i]->_itemId == itemId) {
			return i;
		}
	}
	return -1;
}

void Items::save(SaveFileWriteStream &f) {
	int size = (int)_items.size();

	f.writeInt(size);
	int i;
	for (i = 0; i != size; ++i) {
		_items[i]->save(f);
	}

	// Always write out 100 items
	for (; i != 100; ++i) {
		f.padBytes(0x174); // bbox + rect + 18 float fields
	}
}

void Items::load(SaveFileReadStream &f) {
	for (int i = _items.size() - 1; i >= 0; --i) {
		delete _items.remove_at(i);
	}
	_items.resize(f.readInt());

	int size = (int)_items.size();

	int i;
	for (i = 0; i != size; ++i) {
		_items[i] = new Item(_vm);
		_items[i]->load(f);
	}

	// Always read out 100 items
	for (; i != 100; ++i) {
		f.skip(0x174); // bbox + rect + 18 float fields
	}
}

} // End of namespace BladeRunner
