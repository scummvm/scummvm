/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RIPPER_INPUT_H
#define RIPPER_INPUT_H

#include "common/events.h"
#include "common/queue.h"

namespace Ripper {

enum MouseButtonFlags {
	kMouseButtonLeft = 1 << 0,
	kMouseButtonRight = 1 << 1,
	kMouseButtonMiddle = 1 << 2
};

struct MouseState {
	Common::Point position;
	uint16 buttons;
	uint16 pressed;
	uint16 released;

	MouseState() : position(0, 0), buttons(0), pressed(0), released(0) {}
};

class InputManager {
public:
	explicit InputManager(Common::EventManager *eventManager);

	bool pollEvents();
	bool hasPendingKey() const;
	uint16 consumeKey();
	void drainKeys();
	MouseState publishMouseState();

private:
	static uint16 translateKey(const Common::KeyState &key);
	void updateMousePosition(const Common::Event &event);
	void updateMouseButton(const Common::Event &event, uint16 button, bool pressed);

	Common::EventManager *_eventManager;
	Common::Queue<uint16> _pendingKeys;
	MouseState _mouseState;
};

} // End of namespace Ripper

#endif // RIPPER_INPUT_H
