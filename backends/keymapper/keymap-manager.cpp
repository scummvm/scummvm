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

#include "backends/keymapper/keymap-manager.h"
#include "common/algorithm.h"

namespace Common {

void KeymapManager::Domain::addKeymap(Keymap *map) {
	KeymapMap::iterator it = _keymaps.find(map->getName());
	if (it != _keymaps.end())
		delete _keymaps[map->getName()];
	_keymaps[map->getName()] = map;
}

void KeymapManager::Domain::deleteAllKeyMaps() {
	KeymapMap::iterator it;
	for (it = _keymaps.begin(); it != _keymaps.end(); it++)
		delete it->_value;
	_keymaps.clear();
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
	_globalDomain.setConfigDomain(ConfMan.getDomain(ConfigManager::kApplicationDomain));
}
	
KeymapManager::~KeymapManager() {
	delete _hardwareKeys;
}

void KeymapManager::registerHardwareKeySet(HardwareKeySet *keys) {
	if (_hardwareKeys)
		error("Hardware key set already registered!");
	_hardwareKeys = keys;
}

void KeymapManager::registerGlobalKeymap(Keymap *map) {
	initKeymap(_globalDomain.getConfigDomain(), map);
	_globalDomain.addKeymap(map);
}

void KeymapManager::refreshGameDomain() {
	if (_gameDomain.getConfigDomain() != ConfMan.getActiveDomain()) {
		_gameDomain.deleteAllKeyMaps();

		_gameDomain.setConfigDomain(ConfMan.getActiveDomain());
	}
}

void KeymapManager::registerGameKeymap(Keymap *map) {
	refreshGameDomain();
	initKeymap(_gameDomain.getConfigDomain(), map);
	_gameDomain.addKeymap(map);
}

void KeymapManager::initKeymap(ConfigManager::Domain *domain, 
							   Keymap *map) {
	map->setConfigDomain(domain);
	map->loadMappings(_hardwareKeys);
	if (map->isComplete(_hardwareKeys) == false) {
		automaticMap(map);
		map->saveMappings(domain);
		ConfMan.flushToDisk();
	}
}

// TODO:
// - current weaknesses:
//     - if an action finds a key with required type / category but a parent 
//       action with higher priority is using it, that key is never used
void KeymapManager::automaticMap(Keymap *map) {
	// Create local copies of action and key lists.
	List<Action*> actions(map->getActions());
	List<const HardwareKey*> keys(_hardwareKeys->getHardwareKeys());

	List<Action*>::iterator actIt;
	List<const HardwareKey*>::iterator keyIt, selectedKey;
	
	// Remove actions and keys from local lists that have already been mapped.
	actIt = actions.begin();
	while (actIt != actions.end()) {
		Action *act = *actIt;
		const HardwareKey *key = act->getMappedKey();
		if (key) {
			keys.remove(key);
			actIt = actions.erase(actIt);
		} else {
			++actIt;
		}
	}

	// Sort remaining actions by priority.
	ActionPriorityComp priorityComp;
	sort(actions.begin(), actions.end(), priorityComp);
	
	// First mapping pass:
	// - Match if a key's preferred type or category is same as the action's.
	// - Priority is given to:
	//     - keys that match type over category.
	//     - keys that have not been used by parent maps.
	// - If a key has been used by a parent map the new action must have a
	//   higher priority than the parent action.
	// - As soon as the number of skipped actions equals the number of keys 
	//   remaining we stop matching. This means that the second pass will assign keys
	//   to these higher priority skipped actions.
	uint skipped = 0;
	actIt = actions.begin();
	while (actIt != actions.end() && skipped < keys.size()) {
		selectedKey = keys.end();
		int matchRank = 0;
		Action *act = *actIt;
		for (keyIt = keys.begin(); keyIt != keys.end(); ++keyIt) {
			if ((*keyIt)->preferredType == act->type) {
				Action *parentAct = getParentMappedAction(map, (*keyIt)->key);
				if (!parentAct) {
					selectedKey = keyIt;
					break;
				} else if (parentAct->priority <= act->priority && matchRank < 3) {
					selectedKey = keyIt;
					matchRank = 3;
				}
			} else if ((*keyIt)->preferredCategory == act->category && matchRank < 2) {
				Action *parentAct = getParentMappedAction(map, (*keyIt)->key);
				if (!parentAct) {
					selectedKey = keyIt;
					matchRank = 2;
				} else if (parentAct->priority <= act->priority && matchRank < 1) {
					selectedKey = keyIt;
					matchRank = 1;
				}
			}
		}
		if (selectedKey != keys.end()) {
			// Map action and delete action & key from local lists.
			act->mapKey(*selectedKey);
			keys.erase(selectedKey);
			actIt = actions.erase(actIt);
		} else {
			// Skip action (will be mapped in next pass).
			++actIt;
			++skipped;
		}
	}

	// Second mapping pass:
	// - Maps any remaining actions to keys
	// - priority given to:
	//     - keys that have no parent action
	//     - keys whose parent action has lower priority than the new action
	//     - keys whose parent action has the lowest priority
	// - is guarantees to match a key if one is available
	for (actIt = actions.begin(); actIt != actions.end(); ++actIt) {
		selectedKey = keys.end();
		int matchRank = 0;
		int lowestPriority = 0;
		Action *act = *actIt;
		for (keyIt = keys.begin(); keyIt != keys.end(); ++keyIt) {
			Action *parentAct = getParentMappedAction(map, (*keyIt)->key);
			if (!parentAct) {
				selectedKey = keyIt;
				break;
			} else if (matchRank < 2) {
				if (parentAct->priority <= act->priority) {
					matchRank = 2;
					selectedKey = keyIt;
				} else if (parentAct->priority < lowestPriority || matchRank == 0) {
					matchRank = 1;
					lowestPriority = parentAct->priority;
					selectedKey = keyIt;
				}
			}
		}
		if (selectedKey != keys.end()) {
			act->mapKey(*selectedKey);
			keys.erase(selectedKey);
		} else {// no match = no keys left 
			break;
		}
	}
	map->saveMappings();
	ConfMan.flushToDisk();
}

Action *KeymapManager::getParentMappedAction(Keymap *map, KeyState key) {
	Keymap *parent = map->getParent();
	if (parent) {
		Action *act = parent->getMappedAction(key);
		if (act)
			return act;
		else
			return getParentMappedAction(parent, key);
	} else {
		return 0;
	}
}

Keymap *KeymapManager::getKeymap(const String& name, bool *global) {
	Keymap *keymap = _gameDomain.getKeymap(name);
	*global = false;
	if (!keymap) {
		keymap = _globalDomain.getKeymap(name);
		*global = true;
	}
	return keymap;
}

} // end of namespace Common
