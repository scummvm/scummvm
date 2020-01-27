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

#include "backends/keymapper/keymap.h"

#include "common/array.h"
#include "common/config-manager.h"
#include "common/events.h"

namespace Common {

const char *const kGuiKeymapName = "gui";
const char *const kGlobalKeymapName = "global";

class Action;
struct HardwareInput;
class HardwareInputSet;
class KeymapperDefaultBindings;

typedef Array<Keymap *> KeymapArray;

class Keymapper : public Common::DefaultEventMapper {
public:

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
	 * Registers platform-specific default mappings for keymap actions
	 */
	void registerBackendDefaultBindings(const KeymapperDefaultBindings *backendDefaultBindings);

	/**
	 * Add a keymap to the global domain.
	 * If a saved key setup exists for it in the ini file it will be used.
	 * Else, the key setup will be automatically mapped.
	 *
	 * Transfers ownership of the keymap to the Keymapper
	 */
	void addGlobalKeymap(Keymap *keymap);

	/**
	 * Add a keymap to the game domain.
	 *
	 * Transfers ownership of the keymap to the Keymapper
	 *
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

	/**
	 * Obtain a list of all the keymaps registered with the keymapper
	 */
	const KeymapArray &getKeymaps() const { return _keymaps; }

	/**
	 * reload the mappings for all the keymaps from the configuration manager
	 */
	void reloadAllMappings();

	/**
	 * Set which kind of keymap is currently used to map events
	 *
	 * Keymaps with the global type are always enabled
	 */
	void setEnabledKeymapType(Keymap::KeymapType type);

	/**
	 * Enable/disable the keymapper
	 */
	void setEnabled(bool enabled) { _enabled = enabled; }

	/**
	 * Return a HardwareInput pointer for the given event
	 */
	const HardwareInput *findHardwareInput(const Event &event);

	void initKeymap(Keymap *keymap, ConfigManager::Domain *domain);

private:

	enum IncomingEventType {
		kIncomingKeyDown,
		kIncomingKeyUp,
		kIncomingNonKey
	};

	HardwareInputSet *_hardwareInputs;
	const KeymapperDefaultBindings *_backendDefaultBindings;

	Event executeAction(const Action *act, IncomingEventType incomingType);
	EventType convertDownToUp(EventType eventType);
	IncomingEventType convertToIncomingEventType(const Event &ev) const;

	EventManager *_eventMan;

	bool _enabled;
	Keymap::KeymapType _enabledKeymapType;

	KeymapArray _keymaps;

};

} // End of namespace Common

#endif // #ifdef ENABLE_KEYMAPPER

#endif // #ifndef COMMON_KEYMAPPER_H
