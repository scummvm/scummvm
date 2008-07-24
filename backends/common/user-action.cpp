#include "backends/common/user-action.h"
#include "backends/common/keymap.h"

namespace Common {

UserAction::UserAction(String des, UserActionCategory cat, UserActionType ty, 
					   int pr, int gr, int fl) {
	description = des;
	category = cat;
	type = ty;
	priority = pr;
	group = gr;
	flags = fl;
	_hwKey = 0;
	_parent = 0;
}

void UserAction::setParent(Keymap *parent) {
	_parent = parent;
}

void UserAction::mapKey(const HardwareKey *key) {
	assert(_parent);
	if (_hwKey) _parent->unregisterMapping(this);
	_hwKey = key;
	if (_hwKey) _parent->registerMapping(this, key);
}

const HardwareKey *UserAction::getMappedKey() const {
	return _hwKey;
}

} // end of namespace Common