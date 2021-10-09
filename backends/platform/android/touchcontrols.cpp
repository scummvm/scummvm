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

#if defined(__ANDROID__)

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

#include "common/fs.h"
#include "common/stream.h"
#include "common/archive.h"
#include "image/tga.h"

#include "backends/graphics3d/android/texture.h"
#include "backends/platform/android/android.h"
#include "backends/platform/android/touchcontrols.h"

TouchControls::TouchControls() :
	_arrows_texture(NULL),
	_screen_width(0),
	_screen_height(0) {
}

TouchControls::~TouchControls() {
	if (_arrows_texture) {
		delete _arrows_texture;
		_arrows_texture = 0;
	}
}

TouchControls::Function TouchControls::getFunction(int x, int y) {
	float xPercent = float(x) / _screen_width;

	if (xPercent < 0.3)
		return kFunctionJoystick;
	else if (xPercent < 0.8)
		return kFunctionCenter;
	else
		return kFunctionRight;
}

void TouchControls::touchToJoystickState(int dX, int dY, FunctionState &state) {
	int sqNorm = dX * dX + dY * dY;
	if (sqNorm < 50 * 50) {
		return;
	}

	if (dY > abs(dX)) {
		state.main = Common::KEYCODE_DOWN;
		state.clip = Common::Rect(256, 0, 384, 128);
	} else if (dX > abs(dY)) {
		state.main = Common::KEYCODE_RIGHT;
		state.clip = Common::Rect(128, 0, 256, 128);
	} else if (-dY > abs(dX)) {
		state.main = Common::KEYCODE_UP;
		state.clip = Common::Rect(0, 0, 128, 128);
	} else if (-dX > abs(dY)) {
		state.main = Common::KEYCODE_LEFT;
		state.clip = Common::Rect(384, 0, 512, 128);
	} else {
		return;
	}

	if (sqNorm > 20000) {
		state.modifier = Common::KEYCODE_LSHIFT;
	}

}

void TouchControls::touchToCenterState(int dX, int dY, FunctionState &state) {
	int sqNorm = dX * dX + dY * dY;
	if (sqNorm < 50 * 50) {
		state.main = Common::KEYCODE_RETURN;
	}
}

void TouchControls::touchToRightState(int dX, int dY, FunctionState &state) {
	if (dX * dX + dY * dY < 100 * 100)
		return;

	if (dY > abs(dX)) {
		// down
		state.main = Common::KEYCODE_PAGEDOWN;
		state.clip = Common::Rect(256, 0, 384, 128);
		return;
	} else if (-dY > abs(dX)) {
		// up
		state.main = Common::KEYCODE_PAGEUP;
		state.clip = Common::Rect(0, 0, 128, 128);
		return;
	}

	static Common::KeyCode keycodes[5] = {
		Common::KEYCODE_i, Common::KEYCODE_p, // left zone
		Common::KEYCODE_INVALID, // center
		Common::KEYCODE_u, Common::KEYCODE_l // right zone
	};

	static int16 clips[5][4] = {
		{   0, 128, 128, 256 }, // i
		{ 128, 128, 256, 256 }, // p
		{   0,   0,   0,   0 }, // center
		{ 256, 128, 384, 256 }, // u
		{ 384, 128, 512, 256 }  // l
	};
	static const unsigned int offset = (ARRAYSIZE(keycodes) - 1) / 2;

	int idx = (dX / 100) + offset;
	if (idx < 0) {
		idx = 0;
	}
	if (idx >= ARRAYSIZE(keycodes)) {
		idx = ARRAYSIZE(keycodes) - 1;
	}
	state.main = keycodes[idx];
	state.clip = Common::Rect(clips[idx][0], clips[idx][1], clips[idx][2], clips[idx][3]);
}

TouchControls::FunctionBehavior TouchControls::functionBehaviors[TouchControls::kFunctionMax+1] = {
	{ touchToJoystickState, false, .2f, .5f },
	{ touchToCenterState,   true,  .5f, .5f },
	{ touchToRightState,    true,  .8f, .5f }
};

static GLES8888Texture *loadBuiltinTexture(const char *filename) {
	Common::ArchiveMemberPtr member = SearchMan.getMember(filename);
	Common::SeekableReadStream *str = member->createReadStream();
	Image::TGADecoder dec;
	dec.loadStream(*str);
	const void *pixels = dec.getSurface()->getPixels();

	GLES8888Texture *ret = new GLES8888Texture();
	uint16 w = dec.getSurface()->w;
	uint16 h = dec.getSurface()->h;
	uint16 pitch = dec.getSurface()->pitch;
	ret->allocBuffer(w, h);
	ret->updateBuffer(0, 0, w, h, pixels, pitch);

	delete str;
	return ret;
}

void TouchControls::init(int width, int height) {
	_arrows_texture = loadBuiltinTexture("arrows.tga");
	_screen_width = width;
	_screen_height = height;
}

TouchControls::Pointer *TouchControls::getPointerFromId(int ptrId, bool createNotFound) {
	unsigned int freeEntry = -1;
	for (unsigned int i = 0; i < kNumPointers; i++) {
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
	for (unsigned int i = 0; i < kNumPointers; i++) {
		Pointer &ptr = _pointers[i];
		if (ptr.active && (ptr.function == function)) {
			return &ptr;
		}
	}
	return nullptr;
}

void TouchControls::draw() {
	for (unsigned int i = 0; i < kFunctionMax+1; i++) {
		FunctionState &state = _functionStates[i];
		FunctionBehavior behavior = functionBehaviors[i];

		if (state.clip.isEmpty()) {
			continue;
		}
		_arrows_texture->drawTexture(_screen_width * behavior.xRatio, _screen_height * behavior.yRatio, 64, 64, state.clip);

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

		if (!behavior.keyPressOnRelease) {
			// send key presses continuously
			// first old remove main key, then update modifier, then press new main key
			if (oldState.main != newState.main) {
				keyUp(oldState.main);
			}
			if (oldState.modifier != newState.modifier) {
				keyUp(oldState.modifier);
				keyDown(newState.modifier);
			}
			if (oldState.main != newState.main) {
				keyDown(newState.main);
			}
		}
		oldState = newState;
	} else if (action == JACTION_UP) {
		Pointer *ptr = getPointerFromId(ptrId, false);
		if (!ptr || ptr->function == kFunctionNone) {
			return;
		}

		FunctionBehavior &behavior = functionBehaviors[ptr->function];
		FunctionState &functionState = _functionStates[ptr->function];

		if (!behavior.keyPressOnRelease) {
			// We sent key down continously: keyUp everything
			keyUp(functionState.main);
			keyUp(functionState.modifier);
		} else {
			int dX = x - ptr->startX;
			int dY = y - ptr->startY;

			FunctionState newState;
			functionBehaviors[ptr->function].touchToState(dX, dY, newState);

			keyDown(newState.modifier);
			keyPress(newState.main);
			keyUp(newState.modifier);
		}

		functionState.reset();
		ptr->active = false;
	} else if (action == JACTION_CANCEL) {
		for (unsigned int i = 0; i < kNumPointers; i++) {
			Pointer &ptr = _pointers[i];
			ptr.reset();
		}

		for (unsigned int i = 0; i < kFunctionMax+1; i++) {
			FunctionBehavior &behavior = functionBehaviors[i];
			FunctionState &functionState = _functionStates[i];

			if (!behavior.keyPressOnRelease) {
				// We sent key down continously: keyUp everything
				keyUp(functionState.main);
				keyUp(functionState.modifier);
			}

			functionState.reset();
		}
	}
}

void TouchControls::keyDown(Common::KeyCode kc) {
	if (kc == Common::KEYCODE_INVALID) {
		return;
	}

	Common::Event ev;
	ev.type = Common::EVENT_KEYDOWN;
	ev.kbd.keycode = kc;
	dynamic_cast<OSystem_Android *>(g_system)->pushEvent(ev);
}

void TouchControls::keyUp(Common::KeyCode kc) {
	if (kc == Common::KEYCODE_INVALID) {
		return;
	}

	Common::Event ev;
	ev.type = Common::EVENT_KEYUP;
	ev.kbd.keycode = kc;
	dynamic_cast<OSystem_Android *>(g_system)->pushEvent(ev);
}

void TouchControls::keyPress(Common::KeyCode kc) {
	if (kc == Common::KEYCODE_INVALID) {
		return;
	}

	Common::Event ev;
	ev.kbd.keycode = kc;
	dynamic_cast<OSystem_Android *>(g_system)->pushKeyPressEvent(ev);
}

#endif
