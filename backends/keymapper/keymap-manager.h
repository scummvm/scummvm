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

#ifndef COMMON_KEYMAP_MANAGER
#define COMMON_KEYMAP_MANAGER

#include "backends/keymapper/hardware-key.h"
#include "backends/keymapper/keymap.h"
#include "common/hash-str.h"
#include "common/hashmap.h"

namespace Common {

class KeymapManager {
public:

	class Domain {
		typedef HashMap<String, Keymap*, 
			IgnoreCase_Hash, IgnoreCase_EqualTo> KeymapMap;

	public:
		Domain() : _configDomain(0) {}
		~Domain() { 
			deleteAllKeyMaps();
		}

		void setConfigDomain(ConfigManager::Domain *confDom) { 
			_configDomain = confDom;
		}
		ConfigManager::Domain *getConfigDomain() {
			return _configDomain;
		}

		void addKeymap(Keymap *map);

		void deleteAllKeyMaps();

		Keymap *getKeymap(const String& name);
		
		typedef KeymapMap::iterator iterator;
		typedef KeymapMap::const_iterator const_iterator;
		iterator begin() { return _keymaps.begin(); }
		const_iterator begin() const { return _keymaps.begin(); }
		iterator end() { return _keymaps.end(); }
		const_iterator end() const { return _keymaps.end(); }

		uint32 count() { return _keymaps.size(); }
	private:
		ConfigManager::Domain *_configDomain;
		KeymapMap _keymaps;
	};

	KeymapManager();
	~KeymapManager();

	void registerHardwareKeySet(HardwareKeySet *keys);
	HardwareKeySet *getHardwareKeySet() { return _hardwareKeys; }

	void registerGlobalKeymap(Keymap *map);

	void refreshGameDomain();
	void registerGameKeymap(Keymap *map);

	Keymap *getKeymap(const String& name, bool *global);

	Domain& getGlobalDomain() { return _globalDomain; }
	Domain& getGameDomain() { return _gameDomain; }

private:

	void initKeymap(ConfigManager::Domain *domain, Keymap *keymap);
	void automaticMap(Keymap *map);
	Action *getParentMappedAction(Keymap *map, KeyState key);

	Domain _globalDomain;
	Domain _gameDomain;

	HardwareKeySet *_hardwareKeys;
};

} // end of namespace Common

#endif
