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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// Allow use of stuff in <time.h>
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h

//
// Disable printf override in common/forbidden.h to avoid
// clashes with log.h from the Android SDK.
// That header file uses
//   __attribute__ ((format(printf, 3, 4)))
// which gets messed up by our override mechanism; this could
// be avoided by either changing the Android SDK to use the equally
// legal and valid
//   __attribute__ ((format(printf, 3, 4)))
// or by refining our printf override to use a varadic macro
// (which then wouldn't be portable, though).
// Anyway, for now we just disable the printf override globally
// for the Android port
#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#include "backends/platform/android/android.h"
#include "backends/platform/android/touchcontrols.h"

TouchControls::TouchControls() :
	_drawer(nullptr),
	_screen_width(0),
	_screen_height(0) {
}

TouchControls::Function TouchControls::getFunction(int x, int y) {
	if (_screen_width == 0) {
		// Avoid divide by 0 error
		return kFunctionNone;
	}

	float xPercent = float(x) / _screen_width;

	if (xPercent < 0.3) {
		return kFunctionJoystick;
	} else if (xPercent < 0.8) {
		return kFunctionCenter;
	} else {
		return kFunctionRight;
	}
}

void TouchControls::touchToJoystickState(int dX, int dY, FunctionState &state) {
	int sqNorm = dX * dX + dY * dY;
	if (sqNorm < 50 * 50) {
		return;
	}

	if (dY > abs(dX)) {
		state.main = Common::JOYSTICK_BUTTON_DPAD_DOWN;
		state.clip = Common::Rect(256, 0, 384, 128);
	} else if (dX > abs(dY)) {
		state.main = Common::JOYSTICK_BUTTON_DPAD_RIGHT;
		state.clip = Common::Rect(128, 0, 256, 128);
	} else if (-dY > abs(dX)) {
		state.main = Common::JOYSTICK_BUTTON_DPAD_UP;
		state.clip = Common::Rect(0, 0, 128, 128);
	} else if (-dX > abs(dY)) {
		state.main = Common::JOYSTICK_BUTTON_DPAD_LEFT;
		state.clip = Common::Rect(384, 0, 512, 128);
	} else {
		return;
	}

	if (sqNorm > 20000) {
		state.modifier = Common::JOYSTICK_BUTTON_RIGHT_SHOULDER;
	}

}

void TouchControls::touchToCenterState(int dX, int dY, FunctionState &state) {
	int sqNorm = dX * dX + dY * dY;
	if (sqNorm < 50 * 50) {
		state.main = Common::JOYSTICK_BUTTON_GUIDE;
	}
}

void TouchControls::touchToRightState(int dX, int dY, FunctionState &state) {
	if (dX * dX + dY * dY < 100 * 100) {
		return;
	}

	if (dX > abs(dY)) {
		// right
		state.main = Common::JOYSTICK_BUTTON_RIGHT_STICK;
		state.clip = Common::Rect(512, 128, 640, 256);
		return;
	} else if (-dX > abs(dY)) {
		// left
		state.main = Common::JOYSTICK_BUTTON_LEFT_STICK;
		state.clip = Common::Rect(512, 0, 640, 128);
		return;
	}

	static Common::JoystickButton buttons[5] = {
		Common::JOYSTICK_BUTTON_Y, Common::JOYSTICK_BUTTON_B, // top zone
		Common::JOYSTICK_BUTTON_INVALID, // center
		Common::JOYSTICK_BUTTON_A, Common::JOYSTICK_BUTTON_X // bottom zone
	};

	static int16 clips[5][4] = {
		{   0, 128, 128, 256 }, // y
		{ 128, 128, 256, 256 }, // b
		{   0,   0,   0,   0 }, // center
		{ 256, 128, 384, 256 }, // a
		{ 384, 128, 512, 256 }  // x
	};
	static const uint offset = (ARRAYSIZE(buttons) - 1) / 2;

	int idx = (dY / 100) + offset;
	if (idx < 0) {
		idx = 0;
	}
	if (idx >= ARRAYSIZE(buttons)) {
		idx = ARRAYSIZE(buttons) - 1;
	}
	state.main = buttons[idx];
	state.clip = Common::Rect(clips[idx][0], clips[idx][1], clips[idx][2], clips[idx][3]);
}

TouchControls::FunctionBehavior TouchControls::functionBehaviors[TouchControls::kFunctionMax + 1] =
{
	{ touchToJoystickState, false, .2f, .5f },
	{ touchToCenterState,   true,  .5f, .5f },
	{ touchToRightState,    true,  .8f, .5f }
};

void TouchControls::init(TouchControlsDrawer *drawer, int width, int height) {
	_drawer = drawer;
	_screen_width = width;
	_screen_height = height;
}

TouchControls::Pointer *TouchControls::getPointerFromId(int ptrId, bool createNotFound) {
	uint freeEntry = -1;
	for (uint i = 0; i < kNumPointers; i++) {
		Pointer &ptr = _pointers[i];
		if (ptr.active && (ptr.id == ptrId)) {
			return &ptr;
		}
		if (createNotFound && (freeEntry == -1) && !ptr.active) {
			freeEntry = i;
		}
	}
	// Too much fingers or not found
	if (freeEntry == -1) {
		return nullptr;
	}

	Pointer &ptr = _pointers[freeEntry];
	ptr.reset();
	ptr.id = ptrId;

	return &ptr;
}

TouchControls::Pointer *TouchControls::findPointerFromFunction(Function function) {
	for (uint i = 0; i < kNumPointers; i++) {
		Pointer &ptr = _pointers[i];
		if (ptr.active && (ptr.function == function)) {
			return &ptr;
		}
	}
	return nullptr;
}

void TouchControls::draw() {
	assert(_drawer != nullptr);

	for (uint i = 0; i < kFunctionMax + 1; i++) {
		FunctionState &state = _functionStates[i];
		FunctionBehavior behavior = functionBehaviors[i];

		if (state.clip.isEmpty()) {
			continue;
		}
		_drawer->touchControlDraw(_screen_width * behavior.xRatio, _screen_height * behavior.yRatio,
		                             64, 64, state.clip);

	}
}

void TouchControls::update(Action action, int ptrId, int x, int y) {
	if (action == JACTION_DOWN) {
		Pointer *ptr = getPointerFromId(ptrId, true);
		if (!ptr) {
			return;
		}

		TouchControls::Function function = getFunction(x, y);
		// ptrId is active no matter what
		ptr->active = true;

		if (function == kFunctionNone) {
			// No function for this finger
			return;
		}
		if (findPointerFromFunction(function)) {
			// Some finger is already using this function: don't do anything
			return;
		}

		ptr->startX = ptr->currentX = x;
		ptr->startY = ptr->currentY = y;
		ptr->function = function;
	} else if (action == JACTION_MOVE) {
		Pointer *ptr = getPointerFromId(ptrId, false);
		if (!ptr || ptr->function == kFunctionNone) {
			return;
		}

		FunctionBehavior &behavior = functionBehaviors[ptr->function];

		ptr->currentX = x;
		ptr->currentY = y;

		int dX = x - ptr->startX;
		int dY = y - ptr->startY;

		FunctionState newState;
		functionBehaviors[ptr->function].touchToState(dX, dY, newState);

		FunctionState &oldState = _functionStates[ptr->function];

		if (!behavior.pressOnRelease) {
			// send key presses continuously
			// first old remove main key, then update modifier, then press new main key
			if (oldState.main != newState.main) {
				buttonUp(oldState.main);
			}
			if (oldState.modifier != newState.modifier) {
				buttonUp(oldState.modifier);
				buttonDown(newState.modifier);
			}
			if (oldState.main != newState.main) {
				buttonDown(newState.main);
			}
		}
		oldState = newState;
		if (_drawer) {
			_drawer->touchControlNotifyChanged();
		}
	} else if (action == JACTION_UP) {
		Pointer *ptr = getPointerFromId(ptrId, false);
		if (!ptr || ptr->function == kFunctionNone) {
			return;
		}

		FunctionBehavior &behavior = functionBehaviors[ptr->function];
		FunctionState &functionState = _functionStates[ptr->function];

		if (!behavior.pressOnRelease) {
			// We sent key down continously: buttonUp everything
			buttonUp(functionState.main);
			buttonUp(functionState.modifier);
		} else {
			int dX = x - ptr->startX;
			int dY = y - ptr->startY;

			FunctionState newState;
			functionBehaviors[ptr->function].touchToState(dX, dY, newState);

			buttonDown(newState.modifier);
			buttonPress(newState.main);
			buttonUp(newState.modifier);
		}

		functionState.reset();
		ptr->active = false;
		if (_drawer) {
			_drawer->touchControlNotifyChanged();
		}
	} else if (action == JACTION_CANCEL) {
		for (uint i = 0; i < kNumPointers; i++) {
			Pointer &ptr = _pointers[i];
			ptr.reset();
		}

		for (uint i = 0; i < kFunctionMax + 1; i++) {
			FunctionBehavior &behavior = functionBehaviors[i];
			FunctionState &functionState = _functionStates[i];

			if (!behavior.pressOnRelease) {
				// We sent key down continously: buttonUp everything
				buttonUp(functionState.main);
				buttonUp(functionState.modifier);
			}

			functionState.reset();
		}
		if (_drawer) {
			_drawer->touchControlNotifyChanged();
		}
	}
}

void TouchControls::buttonDown(Common::JoystickButton jb) {
	if (jb == Common::JOYSTICK_BUTTON_INVALID) {
		return;
	}

	Common::Event ev;
	ev.type = Common::EVENT_JOYBUTTON_DOWN;
	ev.joystick.button = jb;
	dynamic_cast<OSystem_Android *>(g_system)->pushEvent(ev);
}

void TouchControls::buttonUp(Common::JoystickButton jb) {
	if (jb == Common::JOYSTICK_BUTTON_INVALID) {
		return;
	}

	Common::Event ev;
	ev.type = Common::EVENT_JOYBUTTON_UP;
	ev.joystick.button = jb;
	dynamic_cast<OSystem_Android *>(g_system)->pushEvent(ev);
}

void TouchControls::buttonPress(Common::JoystickButton jb) {
	if (jb == Common::JOYSTICK_BUTTON_INVALID) {
		return;
	}

	Common::Event ev1, ev2;
	ev1.type = Common::EVENT_JOYBUTTON_DOWN;
	ev1.joystick.button = jb;
	ev2.type = Common::EVENT_JOYBUTTON_UP;
	ev2.joystick.button = jb;
	dynamic_cast<OSystem_Android *>(g_system)->pushEvent(ev1, ev2);
}
