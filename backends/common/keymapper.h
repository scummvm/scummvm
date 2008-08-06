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
#include "common/stack.h"
#include "backends/common/hardware-key.h"
#include "backends/common/keymap.h"
#include "backends/common/keymap-manager.h"

namespace Common {

class Keymapper {
public:

	Keymapper(EventManager *eventMan);
	~Keymapper();

	/**
	 * Registers a HardwareKeySet with the Keymapper
	 * @note should only be called once (during backend initialisation)
	 */
	void registerHardwareKeySet(HardwareKeySet *keys);


	/**
	 * Add a general keymap to the global domain.
	 * If a saved key setup exists for it in the ini file it will be used.
	 * Else, the key setup will be automatically mapped.
	 */
	void addGlobalKeymap(const String& name, Keymap *keymap);

	/**
	* Sets the default keymap for the global domain.
	*/
	void setDefaultGlobalKeymap(Keymap *keymap);

	/**
	* Add a general keymap to the game domain.
	* @see addGlobalKeyMap
	* @note initGame() should be called before any game keymaps are added.
	*/
	void addGameKeymap(const String& name, Keymap *keymap);

	/**
	* Sets the default keymap for the game domain.
	*/
	void setDefaultGameKeymap(Keymap *keymap);

	/**
	 * Initialise the keymapper for a new game
	 */
	void initGame();

	/**
	 * Cleanup the keymapper after a game has ended
	 */
	void cleanupGame();

	/**
	 * Push a new keymap to the top of the active stack, activating it for use.
	 * @param name		name of the keymap to push
	 * @param inherit	if true 
	 * @return true if successful
	 */
	bool pushKeymap(const String& name, bool inherit = false);

	/**
	 * Pop the active keymap off the stack.
	 */
	void popKeymap();

	/**
	* @brief Map a key press event.
	* If the active keymap contains a Action mapped to the given key, then 
	* the Action's events are pushed into the EventManager's event queue.
	* @param key		key that was pressed
	* @param isKeyDown	true for key down, false for key up
	* @return			true if key was mapped
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

	void pushKeymap(Keymap *newMap, bool inherit);
	bool checkGameInit();

	typedef List<HardwareKey*>::iterator Iterator;

	EventManager *_eventMan;
	KeymapManager *_keymapMan;

	String _gameId;

	struct MapRecord {
		Keymap* keymap;
		bool inherit;
	};

	Stack<MapRecord> _activeMaps;

};

} // end of namespace Common

#endif
