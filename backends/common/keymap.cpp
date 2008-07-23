#include "backends/common/keymap.h"

namespace Common {

Keymap::Keymap(const Keymap& km) : _actions(km._actions), _keymap() {
	init();
	for (uint i = 0; i < _actions.size(); i++) {
		if (_actions[i].hwKey) {
			_keymap[_actions[i].hwKey->key] = &_actions[i];
		}
	}
}

void Keymap::init() {
	_actions.reserve(20);
}

void Keymap::addAction(const UserAction& action) {
	if (findUserAction(action.id))
		error("UserAction with id %d already in KeyMap!\n", action.id);
	_actions.push_back(action);
	_actions[_actions.size()-1].hwKey = 0;
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

void Keymap::mapKeyToAction(int32 id, HardwareKey *key) {
	UserAction *act = findUserAction(id);
	if (act)
		internalMapKey(act, key);
}

void Keymap::internalMapKey(UserAction *action, HardwareKey *hwKey) {
	HashMap<KeyState, UserAction*>::iterator it;
	it = _keymap.find(hwKey->key);
	// if key is already mapped to an action then un-map it
	if (it != _keymap.end())
		it->_value->hwKey = 0;

	action->hwKey = hwKey;
	_keymap[hwKey->key] = action;
}

const UserAction *Keymap::getUserAction(int32 id) const {
	return findUserAction(id);
}

UserAction *Keymap::findUserAction(int32 id) {
	Array<UserAction>::iterator it;
	for (it = _actions.begin(); it != _actions.end(); it++) {
		if (it->id == id)
			return &*it;
	}
	return 0;
}

const UserAction *Keymap::findUserAction(int32 id) const {
	Array<UserAction>::const_iterator it;
	for (it = _actions.begin(); it != _actions.end(); it++) {
		if (it->id == id)
			return &*it;
	}
	return 0;
}

UserAction *Keymap::getMappedAction(const KeyState& ks) const {
	HashMap<KeyState, UserAction*>::iterator it;
	it = _keymap.find(ks);
	if (it == _keymap.end())
		return 0;
	else
		return it->_value;
}

} // end of namespace Common