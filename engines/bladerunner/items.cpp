#include "bladerunner/items.h"

namespace BladeRunner {

Items::Items(BladeRunnerEngine *vm) {
	_vm = vm;
}

Items::~Items() {
}


void Items::getXyz(int itemId, float* x, float* y, float* z) {
	int itemIndex = findItem(itemId);
	assert(itemIndex != -1);

	_items[itemIndex].getXyz(x, y, z);
}

int Items::findItem(int itemId) {
	int i;
	for (i = 0; i < _items.size();i++) {
		if (_items[i]._itemId == itemId)
			return i;
	}
	return -1;
}
}