/* ScummVM - Graphic Adventure Engine
*
* ScummVM is the legal property of its developers, whose names
* are too numerous to list here. Please refer to the COPYRIGHT
* file distributed with this source distribution.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
* $URL$
* $Id$
*
*/

#include "backends/common/keymap-manager.h"
#include "common/algorithm.h"

namespace Common {

void KeymapManager::Domain::setDefaultKeymap(Keymap *map) {
	delete _defaultKeymap;
	_defaultKeymap = map;
}

void KeymapManager::Domain::addKeymap(const String& name, Keymap *map) {
	KeymapMap::iterator it = _keymaps.find(name);
	if (it != _keymaps.end())
		delete _keymaps[name];
	_keymaps[name] = map;
}

void KeymapManager::Domain::deleteAllKeyMaps() {
	KeymapMap::iterator it;
	for (it = _keymaps.begin(); it != _keymaps.end(); it++) {
		//it->_value->saveMappings()
		delete it->_value;
	}
	_keymaps.clear();
	delete _defaultKeymap;
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

KeymapManager::KeymapManager() {
	_hardwareKeys = 0;
}
	
KeymapManager::~KeymapManager() {
	delete _hardwareKeys;
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
	_globalDomain.setDefaultKeymap(map);
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
	_gameDomain.setDefaultKeymap(map);
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
	map->loadMappings(domain, name, _hardwareKeys);
	if (isMapComplete(map) == false)
		automaticMap(map);
}

bool KeymapManager::isMapComplete(const Keymap *map) {
	const List<Action*>& actions = map->getActions();
	List<Action*>::const_iterator it;
	bool allMapped = true;
	uint numberMapped = 0;
	for (it = actions.begin(); it != actions.end(); it++) {
		if ((*it)->getMappedKey()) {
			numberMapped++;
		} else {
			allMapped = false;
		}
	}
	return allMapped || (numberMapped == _hardwareKeys->count());
}

void KeymapManager::automaticMap(Keymap *map) {
	List<Action*> actions(map->getActions()), unmapped;
	List<Action*>::iterator actIt;
	List<HardwareKey*> keys = _hardwareKeys->getHardwareKeys();
	List<HardwareKey*>::iterator keyIt, selectedKey;

	// sort by priority
	ActionPriorityComp priorityComp;
	sort(actions.begin(), actions.end(), priorityComp);

	for (actIt = actions.begin(); actIt != actions.end(); actIt++) {
		selectedKey = keys.end();
		Action *act = *actIt;
		for (keyIt = keys.begin(); keyIt != keys.end(); keyIt++) {
			if ((*keyIt)->preferredType == act->type) {
				selectedKey = keyIt;
				break;
			} else if ((*keyIt)->preferredCategory == act->category && selectedKey == keys.end()) {
				selectedKey = keyIt;
			}
		}
		if (selectedKey != keys.end()) {
			act->mapKey(*selectedKey);
			keys.erase(selectedKey);
		} else
			unmapped.push_back(act);
	}

	actIt = unmapped.begin();
	keyIt = keys.begin();
	while (actIt != unmapped.end() && keyIt != keys.end())
		(*actIt)->mapKey(*keyIt);

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
