#include "backends/common/keymap-manager.h"

#define GLOBAL_ID_STR "___GLOBAL"

namespace Common {


void KeymapManager::Domain::addDefaultKeymap(Keymap *map) {
	_defaultKeymap = map;
}

void KeymapManager::Domain::addKeymap(const String& name, Keymap *map) {
	if (_keymaps.contains(name))
		delete _keymaps[name];
	_keymaps[name] = map;
}

void KeymapManager::Domain::deleteAllKeyMaps() {
	KeymapMap::iterator it;
	for (it = _keymaps.begin(); it != _keymaps.end(); it++)
		delete it->_value;
	_keymaps.clear();
}

Keymap *KeymapManager::Domain::getDefaultKeymap() {
	return _defaultKeymap;
}

Keymap *KeymapManager::Domain::getKeymap(const String& name) {
	KeymapMap::iterator it = _keymaps.find(name);
	if (it != _keymaps.end())
		return it->_value;
	else
		return 0;
}


void KeymapManager::registerDefaultGlobalKeymap(Keymap *map) {
	_globalDomain.addDefaultKeymap(map);
}

void KeymapManager::registerGlobalKeymap(const String& name, Keymap *map) {
	_globalDomain.addKeymap(name, map);
}

void KeymapManager::registerDefaultGameKeymap(Keymap *map) {
	_gameDomain.addDefaultKeymap(map);
}

void KeymapManager::registerGameKeymap(const String& name, Keymap *map) {
	_gameDomain.addKeymap(name, map);
}

void KeymapManager::unregisterAllGameKeymaps() {
	_gameDomain.deleteAllKeyMaps();
}

Keymap *KeymapManager::getKeymap(const String& name) {
	Keymap *keymap = _gameDomain.getKeymap(name);
	if (!keymap)
		_globalDomain.getKeymap(name);
	return keymap;
}

} // end of namespace Common