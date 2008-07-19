#include "backends/common/keymap-manager.h"

namespace Common {

void KeymapManager::registerGlobalKeymap(const String& name, const Keymap& map) {
	if (name.size() == 0) {
		warning("Name must be specified when registering global keymap");
		return;
	}
	insertEntry(name, "", map);
}

void KeymapManager::registerKeymap(const String& name, const String& domain, const Keymap& map) {
	if (name.size() == 0 || domain.size() == 0) {
		warning("Name and domain must be specified when registering keymap");
		return;
	}
	insertEntry(name, domain, map);
}

void KeymapManager::insertEntry(const String& name, const String& domain, const Keymap& map) {
	Entry *ent = new Entry;
	ent->_name = name;
	ent->_domain = domain;
	ent->_keymap = new Keymap(map);
	_keymaps.push_back(ent);
}
 
} // end of namespace Common