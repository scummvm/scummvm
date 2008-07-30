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

#include "common/array.h"
#include "common/keyboard.h"
#include "common/func.h"
#include "common/hashmap.h"
#include "backends/common/user-action.h"

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
	Keymap() { init(); }
	Keymap(const Keymap& km);
private:
	void init();

public:
	/**
	 * Adds a new UserAction to this Map, 
	 * adding it at the back of the internal array
	 * @param action the UserAction to add
	 */
	void addAction(UserAction& action);

	/**
	 * Retrieves the UserAction with the given id
	 * @param id id of UserAction to retrieve
	 * @return Pointer to the UserAction or 0 if not found
	 */
	UserAction *getUserAction(int32 id);

	/**
	 * Get a read-only array of all the UserActions contained in this Keymap
	 */
	const Array<UserAction>& getUserActions() const { return _actions; }

	/**
	 * Find the UserAction that a key is mapped to
	 * @param key the key that is mapped to the required UserAction
	 * @return a pointer to the UserAction or 0 if no
	 */
	UserAction *getMappedAction(const KeyState& ks) const;

private:
	friend struct UserAction;
	/**
	* Registers a HardwareKey to the given UserAction
	* @param action UserAction in this Keymap
	* @param key pointer to HardwareKey to map
	* @see UserAction::mapKey
	*/
	void registerMapping(UserAction *action, const HardwareKey *key);

	/**
	* Unregisters a HardwareKey from the given UserAction (if one is mapped)
	* @param action UserAction in this Keymap
	* @see UserAction::mapKey
	*/
	void unregisterMapping(UserAction *action);

	UserAction *findUserAction(int32 id);
	const UserAction *findUserAction(int32 id) const;

	void internalMapKey(UserAction *action, HardwareKey *hwKey);

	Array<UserAction> _actions;
	HashMap<KeyState, UserAction*> _keymap; 

};


} // end of namespace Common

#endif
