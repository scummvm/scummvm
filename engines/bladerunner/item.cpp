#include "bladerunner/item.h"

namespace BladeRunner {

Item::Item() {
	_animationId = -1;
	_itemId = -1;
	_setId = -1;
}

Item::~Item() {
}

void Item::getXYZ(float *x, float *y, float *z) {
	*x = _position.x;
	*y = _position.y;
	*z = _position.z;
}

} // End of namespace BladeRunner
