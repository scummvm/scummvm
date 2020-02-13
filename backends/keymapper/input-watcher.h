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

#ifndef COMMON_INPUT_WATCHER_H
#define COMMON_INPUT_WATCHER_H

#include "common/scummsys.h"

#include "backends/keymapper/hardware-input.h"
#include "common/events.h"

namespace Common {

struct HardwareInput;

/**
 * Watches events for inputs that can be bound to actions
 * 
 * When the watch mode is enabled, the watcher disables the Keymapper
 * and sets itself as an event observer. Once an event corresponding
 * to an hardware input is received, it is saved for later retrieval.
 *
 * Used by the remap dialog to capture input.
 */
class InputWatcher : private EventObserver {
public:
	InputWatcher(EventDispatcher *eventDispatcher, Keymapper *keymapper);

	void startWatching();
	void stopWatching();

	bool isWatching() const;
	HardwareInput checkForCapturedInput();

private:
	bool notifyEvent(const Event &event) override;

	EventDispatcher *_eventDispatcher;
	Keymapper *_keymapper;

	bool _watching;
	HardwareInput _hwInput;
};

} // End of namespace Common

#endif // #ifndef COMMON_INPUT_WATCHER_H
