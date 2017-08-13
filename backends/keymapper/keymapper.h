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
class HardwareInput;
class HardwareInputSet;

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

	/**
	 * Obtain a list of all the keymaps registered with the keymapper
	 */
	const Array<Keymap *> &getKeymaps() const { return _keymaps; }

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

	Event executeAction(const Action *act, IncomingEventType incomingType);
	EventType convertDownToUp(EventType eventType);
	IncomingEventType convertToIncomingEventType(const Event &ev) const;

	EventManager *_eventMan;

	bool _enabled;
	Keymap::KeymapType _enabledKeymapType;

	typedef Array<Keymap *> KeymapArray;
	KeymapArray _keymaps;

};

} // End of namespace Common

#endif // #ifdef ENABLE_KEYMAPPER

#endif // #ifndef COMMON_KEYMAPPER_H
