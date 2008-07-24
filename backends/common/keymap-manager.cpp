#include "backends/common/keymap-manager.h"

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

void KeymapManager::registerHardwareKeySet(HardwareKeySet *keys) {
	if (_hardwareKeys)
		error("Hardware key set already registered!");
	_hardwareKeys = keys;
}

void KeymapManager::registerDefaultGlobalKeymap(Keymap *map) {
	ConfigManager::Domain *dom = ConfMan.getDomain(ConfigManager::kApplicationDomain);
	assert(dom);
	initKeymap(dom, "default", map);
	_globalDomain.addDefaultKeymap(map);
}

void KeymapManager::registerGlobalKeymap(const String& name, Keymap *map) {
	ConfigManager::Domain *dom = ConfMan.getDomain(ConfigManager::kApplicationDomain);
	assert(dom);

	initKeymap(dom, name, map);
	_globalDomain.addKeymap(name, map);
}

void KeymapManager::registerDefaultGameKeymap(Keymap *map) {
	ConfigManager::Domain *dom = ConfMan.getActiveDomain();
	assert(dom);

	initKeymap(dom, "default", map);
	_gameDomain.addDefaultKeymap(map);
}

void KeymapManager::registerGameKeymap(const String& name, Keymap *map) {
	ConfigManager::Domain *dom = ConfMan.getActiveDomain();
	assert(dom);

	initKeymap(dom, name, map);
	_gameDomain.addKeymap(name, map);
}

void KeymapManager::initKeymap(ConfigManager::Domain *domain, 
							   const String& name, 
							   Keymap *map) {
	if (!loadKeymap(domain, name, map))
		return;
	automaticMap(map);
}

bool KeymapManager::loadKeymap(ConfigManager::Domain *domain, 
							   const String& name, 
							   Keymap *map) {
	ConfigManager::Domain::iterator it;	
	String prefix = "km_" + name + "_";
	for (it = domain->begin(); it != domain->end(); it++) {
		const String& key = it->_key;
		if (!key.hasPrefix(prefix.c_str()))
			continue;

		// parse UserAction ID
		const char *actionIdStart = key.c_str() + prefix.size();
		char *err;
		int32 actionId = (int32) strtol(actionIdStart, &err, 0);
		if (err == actionIdStart) {
			warning("'%s' is not a valid UserAction ID", err);
			continue;
		}
		UserAction *ua = map->getUserAction(actionId);
		if (!ua) {
			warning("'%s' keymap does not contain UserAction with ID %d", 
				name.c_str(), (int)actionId);
			continue;
		}

		// parse HardwareKey ID
		int32 hwKeyId = (int32) strtol(it->_value.c_str(), &err, 0);
		if (err == it->_value.c_str()) {
			warning("'%s' is not a valid HardwareKey ID", err);
			continue;
		}
		const HardwareKey *hwKey = _hardwareKeys->findHardwareKey(hwKeyId);
		if (!hwKey) {
			warning("HardwareKey with ID %d not known", (int)hwKeyId);
			continue;
		}

		ua->mapKey(hwKey);
	}
	return isMapComplete(map);
}

bool KeymapManager::isMapComplete(const Keymap *map) {
	return false;
}

void KeymapManager::saveKeymap(ConfigManager::Domain *domain, 
							   const String& name, 
							   const Keymap *map) {
	const Array<UserAction>& actions = map->getUserActions();
	Array<UserAction>::const_iterator it;
	char buf[11];
	for (it = actions.begin(); it != actions.end(); it++) {
		String key("km_");
		sprintf(buf, "%d", it->id);
		key += name + "_" + buf;
		if (it->getMappedKey())
			sprintf(buf, "%d", it->getMappedKey()->id);
		else
			strcpy(buf, "");
		domain->setVal(key, buf);
	}
}


void KeymapManager::automaticMap(Keymap *map) {

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