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

#ifndef COMMON_KEYMAPPER_H
#define COMMON_KEYMAPPER_H

#include "common/scummsys.h"

#ifdef ENABLE_KEYMAPPER

#include "common/events.h"
#include "common/list.h"
#include "common/hashmap.h"
#include "common/stack.h"
#include "backends/keymapper/hardware-key.h"
#include "backends/keymapper/keymap.h"

namespace Common {

class Keymapper : public Common::EventMapper, private Common::ArtificialEventSource {
public:

	struct MapRecord {
		Keymap* keymap;
		bool inherit;
		bool global;
	};

	/* Nested class that represents a set of keymaps */
	class Domain : public HashMap<String, Keymap*,
				IgnoreCase_Hash, IgnoreCase_EqualTo>  {
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

	private:
		ConfigManager::Domain *_configDomain;
	};

	Keymapper(EventManager *eventMan);
	~Keymapper();

	/**
	 * Registers a HardwareKeySet with the Keymapper
	 * @note should only be called once (during backend initialisation)
	 */
	void registerHardwareKeySet(HardwareKeySet *keys);

	/**
	 * Get a list of all registered HardwareKeys
	 */
	const List<const HardwareKey*> &getHardwareKeys() const {
		assert(_hardwareKeys);
		return _hardwareKeys->getHardwareKeys();
	}

	/**
	 * Add a keymap to the global domain.
	 * If a saved key setup exists for it in the ini file it will be used.
	 * Else, the key setup will be automatically mapped.
	 */
	void addGlobalKeymap(Keymap *keymap);

	/**
	 * Add a keymap to the game domain.
	 * @see addGlobalKeyMap
	 * @note initGame() should be called before any game keymaps are added.
	 */
	void addGameKeymap(Keymap *keymap);

	/**
	 * Should be called at end of game to tell Keymapper to deactivate and free
	 * any game keymaps that are loaded.
	 */
	void cleanupGameKeymaps();

	/**
	 * Obtain a keymap of the given name from the keymapper.
	 * Game keymaps have priority over global keymaps
	 * @param name		name of the keymap to return
	 * @param global	set to true if returned keymap is global, false if game
	 */
	Keymap *getKeymap(const String& name, bool &global);

	/**
	 * Push a new keymap to the top of the active stack, activating
	 * it for use.
	 * @param name		name of the keymap to push
	 * @param inherit	if true keymapper will iterate down the
	 *					stack if it cannot find a key in the new map
	 * @return			true if succesful
	 */
	bool pushKeymap(const String& name, bool inherit = false);

	/**
	 * Pop the top keymap off the active stack.
	 */
	void popKeymap();

	// Implementation of the EventMapper interface
	bool notifyEvent(const Common::Event &ev);
	bool pollEvent(Common::Event &ev) { return Common::ArtificialEventSource::pollEvent(ev); }

	/**
	 * @brief Map a key press event.
	 * If the active keymap contains a Action mapped to the given key, then
	 * the Action's events are pushed into the EventManager's event queue.
	 * @param key		key that was pressed
	 * @param keyDown	true for key down, false for key up
	 * @return			true if key was mapped
	 */
	bool mapKey(const KeyState& key, bool keyDown);

	/**
	 * @brief Map a key down event.
	 * @see mapKey
	 */
	bool mapKeyDown(const KeyState& key);

	/**
	 * @brief Map a key up event.
	 * @see mapKey
	 */
	bool mapKeyUp(const KeyState& key);

	/**
	 * Enable/disable the keymapper
	 */
	void setEnabled(bool enabled) { _enabled = enabled; }

	/**
	 * Return a HardwareKey pointer for the given key state
	 */
	const HardwareKey *findHardwareKey(const KeyState& key);

	Domain& getGlobalDomain() { return _globalDomain; }
	Domain& getGameDomain() { return _gameDomain; }
	const Stack<MapRecord>& getActiveStack() const { return _activeMaps; }

private:

	void initKeymap(Domain &domain, Keymap *keymap);

	Domain _globalDomain;
	Domain _gameDomain;

	HardwareKeySet *_hardwareKeys;

	void pushKeymap(Keymap *newMap, bool inherit, bool global);

	Action *getAction(const KeyState& key);
	void executeAction(const Action *act, bool keyDown);

	EventManager *_eventMan;

	bool _enabled;

	Stack<MapRecord> _activeMaps;
	HashMap<KeyState, Action*> _keysDown;

};

} // End of namespace Common

#endif // #ifdef ENABLE_KEYMAPPER

#endif // #ifndef COMMON_KEYMAPPER_H
