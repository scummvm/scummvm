#include "backends/common/keymap.h"

namespace Common {

Keymap::Keymap(const Keymap& km) : _actions(km._actions), _keymap() {
	for (uint i = 0; i < _actions.size(); i++) {
		if (_actions[i]._hwKey) {
			_keymap[_actions[i]._hwKey->key] = &_actions[i];
		}
	}
}

void Keymap::addAction(const UserAction& action) {
	_actions.push_back(action);
	_actions[_actions.size()-1]._hwKey = 0;
}

void Keymap::mapKeyToAction(UserAction *action, HardwareKey *key) {
	for (uint i = 0; i < _actions.size(); i++) {
		if (&_actions[i] == action) {
			internalMapKey(action, key);
			return;
		}
	}
	error("UserAction not contained in KeyMap\n");
}

void Keymap::mapKeyToAction(uint idx, HardwareKey *key) {
	if (idx >= _actions.size())
		error("Key map index out of bounds!\n");
	internalMapKey(&_actions[idx], key);
}

void Keymap::internalMapKey(UserAction *action, HardwareKey *hwKey) {
	HashMap<KeyState, UserAction*>::iterator it;
	it = _keymap.find(hwKey->key);
	// if key is already mapped to an action then un-map it
	if (it != _keymap.end())
		it->_value->_hwKey = 0;

	action->_hwKey = hwKey;
	_keymap[hwKey->key] = action;
}

UserAction *Keymap::getMappedAction(KeyState ks) {
	HashMap<KeyState, UserAction*>::iterator it;
	it = _keymap.find(ks);
	if (it == _keymap.end())
		return 0;
	else
		return it->_value;
}

} // end of namespace Common