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

#ifndef COMMON_KEYMAP
#define COMMON_KEYMAP

#include "common/func.h"
#include "common/hashmap.h"
#include "common/keyboard.h"
#include "common/list.h"
#include "backends/common/action.h"

namespace Common {

struct HardwareKey;

/**
 * Hash function for KeyState
 */
template<> struct Hash<KeyState>
	: public UnaryFunction<KeyState, uint> {

	uint operator()(const KeyState &val) const { 
		return (uint)(val.keycode * (val.flags << 1));
	}
};

class Keymap {
public:
	Keymap() {}
	Keymap(const Keymap& km);

public:
	/**
	 * Adds a new Action to this Map, 
	 * adding it at the back of the internal array
	 * @param action the Action to add
	 */
	void addAction(Action *action);

	/**
	 * Retrieves the Action with the given id
	 * @param id id of Action to retrieve
	 * @return Pointer to the Action or 0 if not found
	 */
	Action *getAction(int32 id);

	/**
	 * Get a read-only array of all the Actions contained in this Keymap
	 */
	const List<Action*>& getActions() const { return _actions; }

	/**
	 * Find the Action that a key is mapped to
	 * @param key the key that is mapped to the required Action
	 * @return a pointer to the Action or 0 if no
	 */
	Action *getMappedAction(const KeyState& ks) const;

private:
	friend struct Action;
	/**
	* Registers a HardwareKey to the given Action
	* @param action Action in this Keymap
	* @param key pointer to HardwareKey to map
	* @see Action::mapKey
	*/
	void registerMapping(Action *action, const HardwareKey *key);

	/**
	* Unregisters a HardwareKey from the given Action (if one is mapped)
	* @param action Action in this Keymap
	* @see Action::mapKey
	*/
	void unregisterMapping(Action *action);

	Action *findAction(int32 id);
	const Action *findAction(int32 id) const;

	void internalMapKey(Action *action, HardwareKey *hwKey);

	List<Action*> _actions;
	HashMap<KeyState, Action*> _keymap; 

};


} // end of namespace Common

#endif
