#include "backends/common/keymap-manager.h"

#define GLOBAL_ID_STR "___GLOBAL"

namespace Common {

KeymapManager::KeymapManager() {

}

bool KeymapManager::registerSuperGlobalKeymap(const Keymap& map) {
	return registerKeymap(GLOBAL_ID_STR, GLOBAL_ID_STR, map);
}

bool KeymapManager::registerGlobalKeymap(const String& name, const Keymap& map) {
	return registerKeymap(name, GLOBAL_ID_STR, map);
}

bool KeymapManager::registerKeymap(const String& name, const String& domain, const Keymap& map) {
	if (findEntry(name, domain) != _keymaps.end()) {
		warning("Keymap with given name and domain already exists\n");
		return false;
	}
	Entry *ent = new Entry;
	ent->_name = name;
	ent->_domain = domain;
	ent->_keymap = new Keymap(map);
	_keymaps.push_back(ent);
	return true;
}

bool KeymapManager::unregisterSuperGlobalKeymap() {
	return unregisterKeymap(GLOBAL_ID_STR, GLOBAL_ID_STR);
}

bool KeymapManager::unregisterGlobalKeymap(const String& name) {
	return unregisterKeymap(name, GLOBAL_ID_STR);
}

bool KeymapManager::unregisterKeymap(const String& name, const String& domain) {
	Iterator it = findEntry(name, domain);
	if (it == _keymaps.end())
		return true;
	delete (*it)->_keymap;
	delete *it;
	_keymaps.erase(it);
	return true;
}

KeymapManager::Iterator KeymapManager::findEntry(const String& name, const String& domain) {
	Iterator it;
	for (it = _keymaps.begin(); it != _keymaps.end(); it++) {
		if ((*it)->_name == name && (*it)->_domain == domain)
			break;
	}
	return it;
}
 
} // end of namespace Common