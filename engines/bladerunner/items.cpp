#include "bladerunner/items.h"

#include "bladerunner/scene.h"
#include "bladerunner/scene_objects.h"

namespace BladeRunner {

Items::Items(BladeRunnerEngine *vm) {
	_vm = vm;
}

Items::~Items() {
	for(int i = _items.size() -1; i >= 0; i--) {
		delete _items.remove_at(i);
	}
}

void Items::getXYZ(int itemId, float* x, float* y, float* z) {
	int itemIndex = findItem(itemId);
	assert(itemIndex != -1);

	_items[itemIndex]->getXYZ(x, y, z);
}

void Items::tick() {
	int setId = _vm->_scene->getSetId();
	for(int i = 0; i < (int)_items.size(); i++) {
		if(_items[i]->_setId != setId) {
			continue;
		}
		bool set14NotTarget = setId == 14 && !_items[i]->isTargetable();
		_items[i]->tick(set14NotTarget);
	}
}

bool Items::add(int itemId, int animationId, int setId, Vector3 position, int facing, int height, int width, bool isTargetable, bool isVisible, bool isPoliceMazeEnemy, bool addToSet) {
	if (_items.size() >= 100) {
		return false;
	}
	int i = findItem(itemId);
	if(i == -1) {
		i = _items.size();
	}

	Item *item = new Item(_vm);
	item->init(itemId, setId, animationId, position, facing, height, width, isTargetable, isVisible, isPoliceMazeEnemy);
	_items.push_back(item);

	if(addToSet && setId == _vm->_scene->getSetId()) {
		return _vm->_sceneObjects->addItem(itemId + SCENE_OBJECTS_ITEMS_OFFSET, &item->_boundingBox, &item->_screenRectangle, isTargetable, isVisible);
	}
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
