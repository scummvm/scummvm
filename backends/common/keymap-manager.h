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

#include "backends/common/hardware-key.h"
#include "backends/common/keymap.h"
#include "common/config-manager.h"
#include "common/hash-str.h"
#include "common/hashmap.h"

namespace Common {

class KeymapManager {
public:

	class Domain {
	public:
		Domain() : _defaultKeymap(0) {}
		~Domain() { deleteAllKeyMaps(); }

		void setDefaultKeymap(Keymap *map);
		void addKeymap(const String& name, Keymap *map);

		void deleteAllKeyMaps();

		Keymap *getDefaultKeymap();
		Keymap *getKeymap(const String& name);

	private:
		typedef HashMap<String, Keymap*, 
			IgnoreCase_Hash, IgnoreCase_EqualTo> KeymapMap;

		Keymap *_defaultKeymap;
		KeymapMap _keymaps;
	};

	KeymapManager();
	~KeymapManager();

	void registerHardwareKeySet(HardwareKeySet *keys);

	void registerDefaultGlobalKeymap(Keymap *map);
	void registerGlobalKeymap(const String& name, Keymap *map);

	void registerDefaultGameKeymap(Keymap *map);
	void registerGameKeymap(const String& name, Keymap *map);

	void unregisterAllGameKeymaps();

	Keymap *getKeymap(const String& name);

private:

	void initKeymap(ConfigManager::Domain *domain, const String& name, Keymap *keymap);
	bool loadKeymap(ConfigManager::Domain *domain, const String& name, Keymap *keymap);
	void saveKeymap(ConfigManager::Domain *domain, const String& name, const Keymap *keymap);
	void automaticMap(Keymap *map);
	bool isMapComplete(const Keymap *map);

	Domain _globalDomain;
	Domain _gameDomain;

	HardwareKeySet *_hardwareKeys;
};

} // end of namespace Common

#endif
