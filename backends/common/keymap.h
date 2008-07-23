#ifndef COMMON_KEYMAP
#define COMMON_KEYMAP

#include "backends/common/hardware-key.h"
#include "backends/common/user-action.h"
#include "common/array.h"
#include "common/keyboard.h"
#include "common/func.h"
#include "common/hashmap.h"

namespace Common {

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
	void addAction(const UserAction& action);

	/**
	* Maps a HardwareKey to the given UserAction
	* @param action must point to a UserAction in this Keymap
	* @param key pointer to HardwareKey to map
	* @note if action does not point to a UserAction in this Keymap a
	*       fatal error will occur
	*/
	void mapKeyToAction(UserAction *action, HardwareKey *key);

	/**
	 * Maps a HardwareKey to the UserAction of the given id
	 * @param id id of the UserAction to map to
	 * @param key pointer to HardwareKey to map
	 */
	void mapKeyToAction(int32 id, HardwareKey *key);

	/**
	 * Retrieves the UserAction with the given id
	 * @param id id of UserAction to retrieve
	 * @return Pointer to the UserAction or 0 if not found
	 */
	const UserAction *getUserAction(int32 id) const;

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
	
	UserAction *findUserAction(int32 id);
	const UserAction *findUserAction(int32 id) const;

	void internalMapKey(UserAction *action, HardwareKey *hwKey);

	Array<UserAction> _actions;
	HashMap<KeyState, UserAction*> _keymap; 

};


} // end of namespace Common

#endif