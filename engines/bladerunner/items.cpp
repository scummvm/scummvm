#include "bladerunner/items.h"

namespace BladeRunner {

Items::Items(BladeRunnerEngine *vm) {
	_vm = vm;
}

Items::~Items() {
}

void Items::getXYZ(int itemId, float* x, float* y, float* z) {
	int itemIndex = findItem(itemId);
	assert(itemIndex != -1);

	_items[itemIndex].getXYZ(x, y, z);
}

int Items::findItem(int itemId) {
	for (int i = 0; i < (int)_items.size(); i++) {
		if (_items[i]._itemId == itemId)
			return i;
	}
	return -1;
}

} // End of namespace BladeRunner
