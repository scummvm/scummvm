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

#include "backends/keymapper/keymap.h"

#include "common/array.h"
#include "common/config-manager.h"
#include "common/events.h"

namespace Common {

const char *const kGuiKeymapName = "gui";
const char *const kGlobalKeymapName = "global";

struct Action;
class DelayedEventSource;
struct HardwareInput;
class HardwareInputSet;
class KeymapperDefaultBindings;

class Keymapper : public Common::EventMapper {
public:

	Keymapper(EventManager *eventMan);
	~Keymapper();

	// EventMapper interface
	virtual List<Event> mapEvent(const Event &ev);

	/**
	 * Registers a HardwareInputSet and platform-specific default mappings with the Keymapper
	 *
	 * Transfers ownership to the Keymapper
	 */
	void registerHardwareInputSet(HardwareInputSet *inputs, KeymapperDefaultBindings *backendDefaultBindings);

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
	 * @param id		name of the keymap to return
	 */
	Keymap *getKeymap(const String &id) const;

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
	Keymap::KeymapType enabledKeymapType() const { return _enabledKeymapType; }

	/**
	 * Enable/disable the keymapper
	 */
	void setEnabled(bool enabled) { _enabled = enabled; }

	/**
	 * Clear all the keymaps and hardware input sets
	 */
	void clear();

	/**
	 * Return a HardwareInput pointer for the given event
	 */
	HardwareInput findHardwareInput(const Event &event);

	void initKeymap(Keymap *keymap, ConfigManager::Domain *domain);
	void reloadKeymapMappings(Keymap *keymap);

private:
	EventManager *_eventMan;
	HardwareInputSet *_hardwareInputs;
	KeymapperDefaultBindings *_backendDefaultBindings;
	DelayedEventSource *_delayedEventSource;

	enum IncomingEventType {
		kIncomingEventIgnored,
		kIncomingEventStart,
		kIncomingEventEnd,
		kIncomingEventInstant
	};

	enum {
		kJoyAxisPressedTreshold   = Common::JOYAXIS_MAX / 2,
		kJoyAxisUnpressedTreshold = Common::JOYAXIS_MAX / 4
	};

	bool _enabled;
	Keymap::KeymapType _enabledKeymapType;

	KeymapArray _keymaps;

	bool _joystickAxisPreviouslyPressed[6];

	Keymap::KeymapMatch getMappedActions(const Event &event, Keymap::ActionArray &actions, Keymap::KeymapType keymapType) const;
	Event executeAction(const Action *act, const Event &incomingEvent);
	EventType convertStartToEnd(EventType eventType);
	IncomingEventType convertToIncomingEventType(const Event &ev) const;

	void hardcodedEventMapping(Event ev);
	void resetInputState();
};

/**
 * RAII helper to temporarily enable a keymap type
 */
class KeymapTypeEnabler {
public:
	KeymapTypeEnabler(Keymapper *keymapper, Keymap::KeymapType keymapType) :
			_keymapper(keymapper) {
		assert(keymapper);
		_previousKeymapType = keymapper->enabledKeymapType();
		keymapper->setEnabledKeymapType(keymapType);
	}

	~KeymapTypeEnabler() {
		_keymapper->setEnabledKeymapType(_previousKeymapType);
	}

private:
	Keymapper *_keymapper;
	Keymap::KeymapType _previousKeymapType;
};

class DelayedEventSource : public EventSource {
public:
	// EventSource API
	bool pollEvent(Event &event) override;
	bool allowMapping() const override;

	/**
	 * Schedule an event to be produced after the specified delay
	 */
	void scheduleEvent(const Event &ev, uint32 delayMillis);

private:
	struct DelayedEventsEntry {
		const uint32 timerOffset;
		const Event event;
		DelayedEventsEntry(const uint32 offset, const Event ev) : timerOffset(offset), event(ev) { }
	};

	Queue<DelayedEventsEntry> _delayedEvents;
	uint32 _delayedEffectiveTime;
};

} // End of namespace Common

#endif // #ifndef COMMON_KEYMAPPER_H
