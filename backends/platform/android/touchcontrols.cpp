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
//   __attribute__ ((format(__printf__, 3, 4)))
// or by refining our printf override to use a varadic macro
// (which then wouldn't be portable, though).
// Anyway, for now we just disable the printf override globally
// for the Android port
#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#include "backends/platform/android/android.h"
#include "backends/platform/android/touchcontrols.h"

#include "common/file.h"
#include "graphics/svg.h"

#define SVG_WIDTH          384
#define SVG_HEIGHT         256
#define BG_WIDTH           128
#define BG_HEIGHT          128
#define ARROW_DEAD          21
#define ARROW_SUPER         42
#define ARROW_WIDTH         64
#define ARROW_SEMI_HEIGHT   52
#define ARROW_HEIGHT        64
#define ARROW_HL_LEFT        0
#define ARROW_HL_TOP       128
#define BUTTON_DEAD         12
#define BUTTON_END          64
#define BUTTON_WIDTH        64
#define BUTTON_HEIGHT       64
#define BUTTON_HL_LEFT     128
#define BUTTON_HL_TOP      128
#define BUTTON2_HL_LEFT    256
#define BUTTON2_HL_TOP     128

// The scale factor is stored as a fixed point 30.2 bits
// This avoids floating point operations
#define SCALE_FACTOR_FXP 4

// gamepad.svg was designed with a basis of 128x128 sized widget
// As it's too small on screen, we apply a factor of 1.5x
// It can be tuned here and in SVG viewBox without changing anything else
#define SVG_UNSCALED(x) ((x) * 3 / 2)
#define SVG_SQ_UNSCALED(x) ((x * x) * 9 / 4)

#define SVG_SCALED(x) (SVG_UNSCALED(x) * _scale)
#define SVG_SQ_SCALED(x) (SVG_SQ_UNSCALED(x) * _scale2)
#define SCALED_PIXELS(x) ((x) / SCALE_FACTOR_FXP)

#define SVG_PIXELS(x) SCALED_PIXELS(SVG_SCALED(x))

TouchControls::TouchControls() :
	_drawer(nullptr),
	_screen_width(0),
	_screen_height(0),
	_svg(nullptr),
	_scale(0),
	_scale2(0) {
}

void TouchControls::init(float scale) {
	_scale = scale * SCALE_FACTOR_FXP;
	// As scale is small, this should fit in int
	_scale2 = _scale * _scale;

	Common::File stream;

	if (!stream.open("gamepad.svg")) {
		error("Failed to fetch gamepad image");
	}

	_svg = new Graphics::SVGBitmap(&stream, SVG_PIXELS(SVG_WIDTH), SVG_PIXELS(SVG_HEIGHT));
}

TouchControls::~TouchControls() {
	delete _svg;
}

TouchControls::Function TouchControls::getFunction(int x, int y) {
	if (_screen_width == 0) {
		// Avoid divide by 0 error
		return kFunctionNone;
	}

	float xRatio = float(x) / _screen_width;

	if (xRatio < 0.3) {
		return kFunctionLeft;
	} else if (xRatio < 0.7) {
		return kFunctionCenter;
	} else {
		return kFunctionRight;
	}
}

void TouchControls::maskToLeftButtons(uint32 oldMask, uint32 newMask) {
	static const Common::JoystickButton buttons[] = {
		Common::JOYSTICK_BUTTON_DPAD_UP, Common::JOYSTICK_BUTTON_DPAD_RIGHT,
		Common::JOYSTICK_BUTTON_DPAD_DOWN, Common::JOYSTICK_BUTTON_DPAD_LEFT
	};

	uint32 diff = newMask ^ oldMask;

	for(int i = 0, m = 1; i < ARRAYSIZE(buttons); i++, m <<= 1) {
		if (!(diff & m)) {
			continue;
		}
		if (oldMask & m) {
			buttonUp(buttons[i]);
		}
	}
	if (diff & 16) {
		if (oldMask & 16) {
			buttonUp(Common::JOYSTICK_BUTTON_RIGHT_SHOULDER);
		} else {
			buttonDown(Common::JOYSTICK_BUTTON_RIGHT_SHOULDER);
		}
	}
	for(int i = 0, m = 1; i < ARRAYSIZE(buttons); i++, m <<= 1) {
		if (!(diff & m)) {
			continue;
		}
		if (newMask & m) {
			buttonDown(buttons[i]);
		}
	}
}

void TouchControls::touchLeft(int dX, int dY, Action action, FunctionState &state) {
	if (action == JACTION_CANCEL ||
		action == JACTION_UP) {
		maskToLeftButtons(state.mask, 0);
		state.reset();
		return;
	}

	FunctionState newState;

	// norm 2 squared (to avoid square root)
	unsigned int sqNorm = (unsigned int)(dX * dX) + (unsigned int)(dY * dY);

	if (sqNorm >= SVG_SQ_SCALED(ARROW_DEAD)) {
		// We are far enough from the center
		// For right we use angles -60,60 as a sensitive zone
		// For left it's the same but mirrored in negative
		// We must be between the two lines which are of tan(60),tan(-60) and this corrsponds to sqrt(3)
		// For up down we use angles -30,30  as a sensitive zone
		// We must be outside the two lines which are of tan(30),tan(-30) and this corrsponds to 1/sqrt(3)
		/*
		static const double SQRT3 = 1.73205080756887719318;
		unsigned int sq3 = SQRT3 * abs(dX);
		*/
		// Optimize by using an approximation of sqrt(3)
		unsigned int sq3 = abs(dX) * 51409 / 29681;
		unsigned int isq3 = abs(dX) * 29681 / 51409;

		unsigned int adY = abs(dY);

		if (adY <= sq3) {
			// Left or right
			if (dX < 0) {
				newState.mask |= 8;
			} else {
				newState.mask |= 2;
			}
		}
		if (adY >= isq3) {
			// Up or down
			if (dY < 0) {
				newState.mask |= 1;
			} else {
				newState.mask |= 4;
			}

		}
	}

	if (sqNorm > SVG_SQ_SCALED(ARROW_SUPER)) {
		newState.mask |= 16;
	}

	if (state.mask != newState.mask) {
		maskToLeftButtons(state.mask, newState.mask);
	}

	state = newState;
}

void TouchControls::drawLeft(int centerX, int centerY, const FunctionState &state) {
	// Draw background
	{
		Common::Rect clip(0, 0, BG_WIDTH, BG_HEIGHT);
		TouchControls::drawSurface(centerX, centerY, -clip.width() / 2, -clip.height() / 2, clip);
	}

	if (state.mask == 0) {
		return;
	}


	// Width and height here are rotated for left/right
	uint16 width = ARROW_WIDTH;
	uint16 height;
	if (state.mask & 16) {
		height = ARROW_HEIGHT;
	} else {
		height = ARROW_SEMI_HEIGHT;
	}

	// We can draw multiple arrows
	if (state.mask & 1) {
		// Draw UP
		Common::Rect clip(width, height);
		clip.translate(0, ARROW_HL_TOP + ARROW_HEIGHT - height);
		int16 offX = -1, offY = -2;
		TouchControls::drawSurface(centerX, centerY, offX * clip.width() / 2, offY * clip.height() / 2, clip);
	}
	if (state.mask & 2) {
		// Draw RIGHT
		Common::Rect clip(height, width);
		clip.translate(ARROW_WIDTH, ARROW_HL_TOP);
		int16 offX = 0, offY = -1;
		TouchControls::drawSurface(centerX, centerY, offX * clip.width() / 2, offY * clip.height() / 2, clip);
	}
	if (state.mask & 4) {
		// Draw DOWN
		Common::Rect clip(width, height);
		clip.translate(0, ARROW_HL_TOP + ARROW_HEIGHT);
		int16 offX = -1, offY = 0;
		TouchControls::drawSurface(centerX, centerY, offX * clip.width() / 2, offY * clip.height() / 2, clip);
	}
	if (state.mask & 8) {
		// Draw LEFT
		Common::Rect clip(height, width);
		clip.translate(ARROW_WIDTH + ARROW_WIDTH - height, ARROW_HL_TOP + ARROW_HEIGHT);
		int16 offX = -2, offY = -1;
		TouchControls::drawSurface(centerX, centerY, offX * clip.width() / 2, offY * clip.height() / 2, clip);
	}
}

void TouchControls::touchRight(int dX, int dY, Action action, FunctionState &state) {
	if (action == JACTION_CANCEL) {
		state.reset();
		return;
	}

	// norm 2 squared (to avoid square root)
	unsigned int sqNorm = (unsigned int)(dX * dX) + (unsigned int)(dY * dY);

	if (sqNorm >= SVG_SQ_SCALED(BUTTON_DEAD) && sqNorm <= SVG_SQ_SCALED(BUTTON_END)) {
		// We are far enough from the center
		// For right we use angles -45,45 as a sensitive zone
		// For left it's the same but mirrored in negative
		// We must be between the two lines which are of tan(45),tan(-45) and this corrsponds to 1
		// For up down we use angles -45,45  as a sensitive zone
		// We must be outside the two lines which are of tan(45),tan(-45) and this corrsponds to 1
		unsigned int adX = abs(dX);
		unsigned int adY = abs(dY);

		if (adY <= adX) {
			// X or B
			if (dX < 0) {
				state.mask = 4;
			} else {
				state.mask = 2;
			}
		} else {
			// Y or A
			if (dY < 0) {
				state.mask = 1;
			} else {
				state.mask = 3;
			}

		}
	} else {
		state.reset();
	}

	static const Common::JoystickButton buttons[] = {
		Common::JOYSTICK_BUTTON_Y, Common::JOYSTICK_BUTTON_B,
		Common::JOYSTICK_BUTTON_A, Common::JOYSTICK_BUTTON_X
	};
	static const Common::JoystickButton modifiers[] = {
		Common::JOYSTICK_BUTTON_INVALID, Common::JOYSTICK_BUTTON_INVALID,
		Common::JOYSTICK_BUTTON_INVALID, Common::JOYSTICK_BUTTON_INVALID
	};
	if (action == JACTION_UP && state.mask) {
		buttonDown(modifiers[state.mask - 1]);
		buttonPress(buttons[state.mask - 1]);
		buttonUp(modifiers[state.mask - 1]);
	}
}

void TouchControls::drawRight(int centerX, int centerY, const FunctionState &state) {
	// Draw background
	{
		Common::Rect clip(BG_WIDTH, 0, 2 * BG_WIDTH, BG_HEIGHT);
		TouchControls::drawSurface(centerX, centerY, -clip.width() / 2, -clip.height() / 2, clip);
	}

	if (state.mask == 0) {
		return;
	}


	Common::Rect clip(BUTTON_WIDTH, BUTTON_HEIGHT);

	int16 offX, offY;

	if (state.mask == 1) {
		// Draw Y
		clip.translate(BUTTON_HL_LEFT, BUTTON_HL_TOP);
		offX = -1;
		offY = -2;
	} else if (state.mask == 2) {
		// Draw B
		clip.translate(BUTTON_HL_LEFT + BUTTON_WIDTH, BUTTON_HL_TOP);
		offX = 0;
		offY = -1;
	} else if (state.mask == 3) {
		// Draw A
		clip.translate(BUTTON_HL_LEFT, BUTTON_HL_TOP + BUTTON_HEIGHT);
		offX = -1;
		offY = 0;
	} else if (state.mask == 4) {
		// Draw X
		clip.translate(BUTTON_HL_LEFT + BUTTON_WIDTH, BUTTON_HL_TOP + BUTTON_HEIGHT);
		offX = -2;
		offY = -1;
	} else {
		return;
	}
	TouchControls::drawSurface(centerX, centerY, offX * clip.width() / 2, offY * clip.height() / 2, clip);
}

void TouchControls::touchCenter(int dX, int dY, Action action, FunctionState &state) {
	if (action == JACTION_CANCEL) {
		state.reset();
		return;
	}

	// norm 2 squared (to avoid square root)
	unsigned int sqNorm = (unsigned int)(dX * dX) + (unsigned int)(dY * dY);

	if (sqNorm >= SVG_SQ_SCALED(BUTTON_DEAD) && sqNorm <= SVG_SQ_SCALED(BUTTON_END)) {
		// We are far enough from the center
		// For right we use angles -45,45 as a sensitive zone
		// For left it's the same but mirrored in negative
		// We must be between the two lines which are of tan(45),tan(-45) and this corrsponds to 1
		// For up down we use angles -45,45  as a sensitive zone
		// We must be outside the two lines which are of tan(45),tan(-45) and this corrsponds to 1
		unsigned int adX = abs(dX);
		unsigned int adY = abs(dY);

		if (adY <= adX) {
			// X or B
			if (dX < 0) {
				state.mask = 4;
			} else {
				state.mask = 2;
			}
		} else {
			// Y or A
			if (dY < 0) {
				state.mask = 1;
			} else {
				state.mask = 3;
			}

		}
	} else {
		state.reset();
	}

	static const Common::JoystickButton buttons[] = {
		Common::JOYSTICK_BUTTON_GUIDE, Common::JOYSTICK_BUTTON_RIGHT_STICK,
		Common::JOYSTICK_BUTTON_START, Common::JOYSTICK_BUTTON_LEFT_STICK
	};
	static const Common::JoystickButton modifiers[] = {
		Common::JOYSTICK_BUTTON_INVALID, Common::JOYSTICK_BUTTON_INVALID,
		Common::JOYSTICK_BUTTON_INVALID, Common::JOYSTICK_BUTTON_INVALID
	};
	if (action == JACTION_UP && state.mask) {
		buttonDown(modifiers[state.mask - 1]);
		buttonPress(buttons[state.mask - 1]);
		buttonUp(modifiers[state.mask - 1]);
	}
}

void TouchControls::drawCenter(int centerX, int centerY, const FunctionState &state) {
	// Draw background
	{
		Common::Rect clip(BG_WIDTH * 2, 0, 3 * BG_WIDTH, BG_HEIGHT);
		TouchControls::drawSurface(centerX, centerY, -clip.width() / 2, -clip.height() / 2, clip);
	}

	if (state.mask == 0) {
		return;
	}

	Common::Rect clip(BUTTON_WIDTH, BUTTON_HEIGHT);

	int16 offX, offY;

	if (state.mask == 1) {
		// Draw Y
		clip.translate(BUTTON2_HL_LEFT, BUTTON2_HL_TOP);
		offX = -1;
		offY = -2;
	} else if (state.mask == 2) {
		// Draw B
		clip.translate(BUTTON2_HL_LEFT + BUTTON_WIDTH, BUTTON2_HL_TOP);
		offX = 0;
		offY = -1;
	} else if (state.mask == 3) {
		// Draw A
		clip.translate(BUTTON2_HL_LEFT, BUTTON2_HL_TOP + BUTTON_HEIGHT);
		offX = -1;
		offY = 0;
	} else if (state.mask == 4) {
		// Draw X
		clip.translate(BUTTON2_HL_LEFT + BUTTON_WIDTH, BUTTON2_HL_TOP + BUTTON_HEIGHT);
		offX = -2;
		offY = -1;
	} else {
		return;
	}
	TouchControls::drawSurface(centerX, centerY, offX * clip.width() / 2, offY * clip.height() / 2, clip);
}

const TouchControls::FunctionBehavior TouchControls::functionBehaviors[TouchControls::kFunctionMax + 1] =
{
	{ &TouchControls::touchLeft,    &TouchControls::drawLeft },
	{ &TouchControls::touchRight,   &TouchControls::drawRight },
	{ &TouchControls::touchCenter,  &TouchControls::drawCenter }
};

void TouchControls::setDrawer(TouchControlsDrawer *drawer, int width, int height) {
	_drawer = drawer;
	_screen_width = width * SCALE_FACTOR_FXP;
	_screen_height = height * SCALE_FACTOR_FXP;

	if (drawer) {
		drawer->touchControlInitSurface(*_svg);
	}
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

	for (uint i = 0; i < kNumPointers; i++) {
		Pointer &ptr = _pointers[i];
		if (!ptr.active) {
			continue;
		}
		if (ptr.function == kFunctionNone) {
			continue;
		}
		const FunctionBehavior &behavior = functionBehaviors[ptr.function];
		(this->*behavior.draw)(ptr.startX, ptr.startY, ptr.state);
	}
}

void TouchControls::update(Action action, int ptrId, int x, int y) {
	x *= SCALE_FACTOR_FXP;
	y *= SCALE_FACTOR_FXP;
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

		const FunctionBehavior &behavior = functionBehaviors[ptr->function];
		(this->*behavior.touch)(0, 0, action, ptr->state);
		if (_drawer) {
			_drawer->touchControlNotifyChanged();
		}
	} else if (action == JACTION_MOVE) {
		Pointer *ptr = getPointerFromId(ptrId, false);
		if (!ptr || ptr->function == kFunctionNone) {
			return;
		}

		ptr->currentX = x;
		ptr->currentY = y;

		int dX = x - ptr->startX;
		int dY = y - ptr->startY;

		(this->*functionBehaviors[ptr->function].touch)(dX, dY, action, ptr->state);
		if (_drawer) {
			_drawer->touchControlNotifyChanged();
		}
	} else if (action == JACTION_UP) {
		Pointer *ptr = getPointerFromId(ptrId, false);
		if (!ptr || ptr->function == kFunctionNone) {
			return;
		}

		int dX = x - ptr->startX;
		int dY = y - ptr->startY;

		(this->*functionBehaviors[ptr->function].touch)(dX, dY, action, ptr->state);
		ptr->active = false;
		if (_drawer) {
			_drawer->touchControlNotifyChanged();
		}
	} else if (action == JACTION_CANCEL) {
		for (uint i = 0; i < kNumPointers; i++) {
			Pointer &ptr = _pointers[i];

			if (!ptr.active) {
				continue;
			}
			if (ptr.function != kFunctionNone) {
				(this->*functionBehaviors[ptr.function].touch)(0, 0, action, ptr.state);
			}
			ptr.reset();
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

	//LOGD("TouchControls::buttonDown: %d", jb);
	Common::Event ev;
	ev.type = Common::EVENT_JOYBUTTON_DOWN;
	ev.joystick.button = jb;
	dynamic_cast<OSystem_Android *>(g_system)->pushEvent(ev);
}

void TouchControls::buttonUp(Common::JoystickButton jb) {
	if (jb == Common::JOYSTICK_BUTTON_INVALID) {
		return;
	}

	//LOGD("TouchControls::buttonUp: %d", jb);
	Common::Event ev;
	ev.type = Common::EVENT_JOYBUTTON_UP;
	ev.joystick.button = jb;
	dynamic_cast<OSystem_Android *>(g_system)->pushEvent(ev);
}

void TouchControls::buttonPress(Common::JoystickButton jb) {
	if (jb == Common::JOYSTICK_BUTTON_INVALID) {
		return;
	}

	//LOGD("TouchControls::buttonPress: %d", jb);
	Common::Event ev1, ev2;
	ev1.type = Common::EVENT_JOYBUTTON_DOWN;
	ev1.joystick.button = jb;
	ev2.type = Common::EVENT_JOYBUTTON_UP;
	ev2.joystick.button = jb;
	dynamic_cast<OSystem_Android *>(g_system)->pushEvent(ev1, ev2);
}

void TouchControls::drawSurface(int x, int y, int offX, int offY, const Common::Rect &clip) {
	Common::Rect clip_(SVG_PIXELS(clip.left), SVG_PIXELS(clip.top),
			SVG_PIXELS(clip.right), SVG_PIXELS(clip.bottom));
	_drawer->touchControlDraw(
			SCALED_PIXELS(x + SVG_SCALED(offX)), SCALED_PIXELS(y + SVG_SCALED(offY)),
			clip_.width(), clip_.height(), clip_);
}
