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

#ifndef COMMON_KEYMAPPER
#define COMMON_KEYMAPPER

#include "common/events.h"
#include "common/list.h"


namespace Common {

struct HardwareKey;
class HardwareKeySet;
class KeymapManager;
class Keymap;

class Keymapper {
public:

	Keymapper(EventManager *eventMan);

	/**
	 * Registers a HardwareKeySet with the Keymapper
	 * @note should only be called once (during backend initialisation)
	 */
	void registerHardwareKeySet(HardwareKeySet *keys);

	/**
	 * Add a keymap to the global domain.
	 * If a saved key setup exists for it in the ini file it will be used.
	 * Else, the key setup will be automatically mapped.
	 */
	void addGlobalKeyMap(const String& name, Keymap *keymap);

	/**
	* Add a keymap to the game domain.
	* @see addGlobalKeyMap
	* @note initGame() should be called before any game keymaps are added.
	*/
	void addGameKeyMap(const String& name, Keymap *keymap);

	/**
	 * Initialise the keymapper for a new game
	 */
	void initGame();

	/**
	 * Cleanup the keymapper after a game has ended
	 */
	void cleanupGame();

	/**
	 * Switch the active keymap.
	 * @param name name of the new keymap
	 * @return true if successful
	 */
	bool switchKeymap(const String& name);

	/**
	* @brief Map a key press event.
	* If the active keymap contains a Action mapped to the given key, then 
	* the Action's events are pushed into the EventManager's event queue.
	* @param key key that was pressed
	* @param isKeyDown true for key down, false for key up
	* @return true if key was mapped
	*/
	bool mapKey(const KeyState& key, bool isKeyDown);

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

private:

	typedef List<HardwareKey*>::iterator Iterator;

	EventManager *_eventMan;
	KeymapManager *_keymapMan;

	String _gameId;

	Keymap *_currentMap;

};

} // end of namespace Common

#endif
