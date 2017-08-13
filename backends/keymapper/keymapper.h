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
 */

#ifndef COMMON_KEYMAPPER_H
#define COMMON_KEYMAPPER_H

#include "common/scummsys.h"

#ifdef ENABLE_KEYMAPPER

#include "common/events.h"
#include "common/list.h"
#include "common/hashmap.h"
#include "common/stack.h"
#include "backends/keymapper/hardware-input.h"
#include "backends/keymapper/keymap.h"

namespace Common {

const char *const kGuiKeymapName = "gui";
const char *const kGlobalKeymapName = "global";

/**
 * Hash function for KeyState
 */
template<> struct Hash<KeyState>
		: public UnaryFunction<KeyState, uint> {

	uint operator()(const KeyState &val) const {
		return (uint)val.keycode | ((uint)val.flags << 24);
	}
};

class Keymapper : public Common::DefaultEventMapper {
public:

	struct MapRecord {
		Keymap* keymap;
		bool transparent;
	};

	Keymapper(EventManager *eventMan);
	~Keymapper();

	// EventMapper interface
	virtual List<Event> mapEvent(const Event &ev, EventSource *source);

	/**
	 * Registers a HardwareInputSet with the Keymapper
	 * @note should only be called once (during backend initialisation)
	 */
	void registerHardwareInputSet(HardwareInputSet *inputs);

	/**
	 * Get a list of all registered HardwareInputs
	 */
	const List<const HardwareInput *> &getHardwareInputs() const {
		assert(_hardwareInputs);
		return _hardwareInputs->getHardwareInputs();
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
	 */
	Keymap *getKeymap(const String &name);

	const Array<Keymap *> &getKeymaps() const { return _keymaps; }

	/**
	 * Push a new keymap to the top of the active stack, activating
	 * it for use.
	 * @param name			name of the keymap to push
	 * @param transparent	if true keymapper will iterate down the
	 *						stack if it cannot find a key in the new map
	 * @return				true if successful
	 */
	bool pushKeymap(const String& name, bool transparent = false);

	/**
	 * Pop the top keymap off the active stack.
	 * @param name	(optional) name of keymap expected to be popped
	 * 				if provided, will not pop unless name is the same
	 * 				as the top keymap
	 */
	void popKeymap(const char *name = 0);

	/**
	 * Enable/disable the keymapper
	 */
	void setEnabled(bool enabled) { _enabled = enabled; }

	/**
	 * Return a HardwareInput pointer for the given event
	 */
	const HardwareInput *findHardwareInput(const Event &event);

	const Stack<MapRecord>& getActiveStack() const { return _activeMaps; }

	/**
	 * Register the binding of a hardware input to an action
	 */
	void registerMapping(Action *action, const HardwareInput *input);

	/**
	 * Unbind hardware inputs from an action
	 */
	void clearMapping(Action *action);

private:

	enum IncomingEventType {
		kIncomingKeyDown,
		kIncomingKeyUp,
		kIncomingNonKey
	};

	void initKeymap(Keymap *keymap, ConfigManager::Domain *domain);

	HardwareInputSet *_hardwareInputs;

	void pushKeymap(Keymap *newMap, bool transparent);

	Event executeAction(const Action *act, IncomingEventType incomingType);
	EventType convertDownToUp(EventType eventType);
	IncomingEventType convertToIncomingEventType(const Event &ev) const;

	EventManager *_eventMan;

	bool _enabled;

	typedef Array<Keymap *> KeymapArray;
	KeymapArray _keymaps;

	Stack<MapRecord> _activeMaps;

};

} // End of namespace Common

#endif // #ifdef ENABLE_KEYMAPPER

#endif // #ifndef COMMON_KEYMAPPER_H
