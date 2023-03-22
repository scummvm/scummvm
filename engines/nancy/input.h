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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NANCY_INPUT_H
#define NANCY_INPUT_H

#include "engines/nancy/commontypes.h"

#include "common/rect.h"
#include "common/keyboard.h"

namespace Common {
class Keymap;
typedef class Array<Keymap *> KeymapArray;
}

namespace Nancy {

namespace State {
class State;
}

struct NancyInput {
	enum InputType : uint16 {
		kLeftMouseButtonDown	= 1 << 0,
		kLeftMouseButtonHeld	= 1 << 1,
		kLeftMouseButtonUp		= 1 << 2,
		kRightMouseButtonDown	= 1 << 3,
		kRightMouseButtonHeld	= 1 << 4,
		kRightMouseButtonUp		= 1 << 5,
		kMoveUp					= 1 << 6,
		kMoveDown				= 1 << 7,
		kMoveLeft				= 1 << 8,
		kMoveRight				= 1 << 9,
		kMoveFastModifier		= 1 << 10,

		kLeftMouseButton		= kLeftMouseButtonDown | kLeftMouseButtonHeld | kLeftMouseButtonUp,
		kRightMouseButton		= kRightMouseButtonDown | kRightMouseButtonHeld | kRightMouseButtonUp
	};

	Common::Point mousePos;
	uint16 input;
	Common::Array<Common::KeyState> otherKbdInput;

	void eatMouseInput() { mousePos.x = -1; input &= ~(kLeftMouseButton | kRightMouseButton); }
};

// This class handles collecting events and translating them to a NancyInput object,
// which can then be pulled by interested classes through getInput()
class InputManager {
	friend class NancyConsole;

enum NancyAction {
	kNancyActionMoveUp,
	kNancyActionMoveDown,
	kNancyActionMoveLeft,
	kNancyActionMoveRight,
	kNancyActionMoveFast,
	kNancyActionLeftClick,
	kNancyActionRightClick
};

public:
	InputManager() :
		_inputs(0),
		_mouseEnabled(true),
		_inputBeginState(NancyState::kNone) {}

	void processEvents();

	NancyInput getInput() const;
	void forceCleanInput();
	void setMouseInputEnabled(bool enabled) { _mouseEnabled = enabled; }

	static void initKeymaps(Common::KeymapArray &keymaps);

private:
	uint16 _inputs;
	Common::Array<Common::KeyState> _otherKbdInput;
	bool _mouseEnabled;
	NancyState::NancyState _inputBeginState;
};

} // End of namespace Nancy

#endif // NANCY_INPUT_H
