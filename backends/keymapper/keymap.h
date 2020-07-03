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

#ifndef COMMON_KEYMAP_H
#define COMMON_KEYMAP_H

#include "common/scummsys.h"

#include "backends/keymapper/hardware-input.h"

#include "common/config-manager.h"
#include "common/func.h"
#include "common/hashmap.h"
#include "common/hash-ptr.h"
#include "common/list.h"
#include "common/str-array.h"

namespace Common {

const char *const kStandardActionsKeymapName = "standard-actions";

struct Action;
struct Event;
struct HardwareInput;
class HardwareInputSet;
class KeymapperDefaultBindings;

struct HardwareInput_EqualTo {
	bool operator()(const HardwareInput& x, const HardwareInput& y) const {
		return (x.type == y.type)
		        && (x.key.keycode == y.key.keycode)
		        && (x.key.flags == y.key.flags)
		        && (x.inputCode == y.inputCode);
	}
};

struct HardwareInput_Hash {
	uint operator()(const HardwareInput& x) const {
		uint hash = 7;
		hash = 31 * hash + x.type;
		hash = 31 * hash + x.key.keycode;
		hash = 31 * hash + x.key.flags;
		hash = 31 * hash + x.inputCode;
		return hash;
	}
};

class Keymap {
public:
	enum KeymapType {
		kKeymapTypeGlobal,
		kKeymapTypeGui,
		kKeymapTypeGame
	};

	enum KeymapMatch {
		kKeymapMatchNone,
		kKeymapMatchPartial,
		kKeymapMatchExact
	};

	typedef Array<Action *> ActionArray;

	Keymap(KeymapType type, const String &id, const U32String &description);
	Keymap(KeymapType type, const String &id, const String &description);
	~Keymap();
	void setConfigDomain(ConfigManager::Domain *configDomain);
	void setHardwareInputs(HardwareInputSet *hardwareInputSet);
	void setBackendDefaultBindings(const KeymapperDefaultBindings *backendDefaultBindings);

	/**
	* Registers a HardwareInput to the given Action
	* @param action Action in this Keymap
	* @param key pointer to HardwareInput to map
	* @see Action::mapKey
	*/
	void registerMapping(Action *action, const HardwareInput &input);

	/**
	* Unregisters a HardwareInput from the given Action (if one is mapped)
	* @param action Action in this Keymap
	* @see Action::mapKey
	*/
	void unregisterMapping(Action *action);

	/**
	 * Reset an action's mapping to its defaults
	 * @param action
	 */
	void resetMapping(Action *action);

	/**
	 * Find the hardware input an action is mapped to, if any
	 */
	Array<HardwareInput> getActionMapping(Action *action) const;

	/**
	 * Find the Actions that a hardware input is mapped to
	 * @param hardwareInput	the input that is mapped to the required Action
	 * @param actions an array containing pointers to the actions
	 * @return	the matching status for the retieved actions
	 */
	KeymapMatch getMappedActions(const Event &event, ActionArray &actions) const;

	/**
	 * Adds a new Action to this Map
	 *
	 * Takes ownership of the action.
	 *
	 * @param action the Action to add
	 */
	void addAction(Action *action);

	/**
	 * Get the list of all the Actions contained in this Keymap
	 */
	const ActionArray &getActions() const { return _actions; }

	/**
	 * Get the default input mappings for an action.
	 *
	 * Backend-specific mappings replace the default mappings
	 * specified when creating the keymap.
	 */
	StringArray getActionDefaultMappings(Action *action);

	/**
	 * Load this keymap's mappings from the config manager.
	 * @param hwInputs	the set to retrieve hardware input pointers from
	 */
	void loadMappings();

	/**
	 * Save this keymap's mappings to the config manager
	 * @note Changes are *not* flushed to disk, to do so call ConfMan.flushToDisk()
	 */
	void saveMappings();

	const String &getId() const { return _id; }
	const U32String &getDescription() const { return _description; }
	KeymapType getType() const { return _type; }

	/**
	 * Defines if the keymap is considered when mapping events
	 */
	bool isEnabled() const { return _enabled; }
	void setEnabled(bool enabled) { _enabled = enabled; }

	/** Helper to return an array with a single keymap element */
	static Array<Keymap *> arrayOf(Keymap *keymap) {
		return Array<Keymap *>(1, keymap);
	}

private:

	const Action *findAction(const char *id) const;

	void registerMappings(Action *action, const StringArray &hwInputIds);
	bool areMappingsIdentical(const Array<HardwareInput> &inputs, const StringArray &mapping);

	typedef HashMap<HardwareInput, ActionArray, HardwareInput_Hash, HardwareInput_EqualTo> HardwareActionMap;

	KeymapType _type;
	String _id;
	U32String _description;

	bool _enabled;

	ActionArray _actions;
	HardwareActionMap _hwActionMap;

	ConfigManager::Domain *_configDomain;
	HardwareInputSet *_hardwareInputSet;
	const KeymapperDefaultBindings *_backendDefaultBindings;
};

typedef Array<Keymap *> KeymapArray;

} // End of namespace Common

#endif // #ifndef COMMON_KEYMAP_H
