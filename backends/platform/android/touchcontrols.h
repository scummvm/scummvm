/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ANDROID_TOUCHCONTROLS_H_
#define ANDROID_TOUCHCONTROLS_H_

#if defined(__ANDROID__)

#include "common/events.h"

#include "backends/graphics3d/android/texture.h"

class TouchControls {
public:
	// action type
	enum Action {
		JACTION_DOWN = 0,
		JACTION_MOVE = 1,
		JACTION_UP = 2,
		JACTION_CANCEL = 3
	};

	TouchControls();
	~TouchControls();

	void init(int width, int height);
	void draw();
	void update(Action action, int ptr, int x, int y);

private:
	int _screen_width, _screen_height;

	enum Function {
		kFunctionNone = -1,
		kFunctionJoystick = 0,
		kFunctionCenter = 1,
		kFunctionRight = 2,
		kFunctionMax = 2
	};
	Function getFunction(int x, int y);

	struct Pointer {
		Pointer() : id(-1), startX(-1), startY(-1), currentX(-1), currentY(-1), function(kFunctionNone), active(false) {}
		void reset() { id = -1; startX = startY = currentX = currentY = -1; function = kFunctionNone; active = false; }

		int id;
		uint16 startX, startY;
		uint16 currentX, currentY;
		Function function;
		bool active;
	};

	enum { kNumPointers = 5 };
	Pointer _pointers[kNumPointers];

	Pointer *getPointerFromId(int ptr, bool createNotFound);
	Pointer *findPointerFromFunction(Function function);

	struct FunctionState {
		FunctionState() : main(Common::KEYCODE_INVALID), modifier(Common::KEYCODE_INVALID) { }
		void reset() { main = Common::KEYCODE_INVALID; modifier = Common::KEYCODE_INVALID; clip = Common::Rect(); }

		Common::KeyCode main;
		Common::KeyCode modifier;
		Common::Rect clip;
	};

	FunctionState _functionStates[kFunctionMax + 1];

	GLESTexture *_arrows_texture;
	void keyDown(Common::KeyCode kc);
	void keyUp(Common::KeyCode kc);
	void keyPress(Common::KeyCode kc);

	/* Functions implementations */
	struct FunctionBehavior {
		void (*touchToState)(int, int, TouchControls::FunctionState &);
		bool keyPressOnRelease;
		float xRatio;
		float yRatio;
	};
	static FunctionBehavior functionBehaviors[TouchControls::kFunctionMax+1];

	static void touchToJoystickState(int dX, int dY, FunctionState &state);
	static void touchToCenterState(int dX, int dY, FunctionState &state);
	static void touchToRightState(int dX, int dY, FunctionState &state);


};

#endif

#endif
