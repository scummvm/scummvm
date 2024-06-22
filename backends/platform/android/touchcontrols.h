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

#ifndef ANDROID_TOUCHCONTROLS_H_
#define ANDROID_TOUCHCONTROLS_H_

#include "common/events.h"

namespace Graphics {
class ManagedSurface;
}

class TouchControlsDrawer {
public:
	virtual void touchControlInitSurface(const Graphics::ManagedSurface &surf) = 0;
	virtual void touchControlNotifyChanged() = 0;
	virtual void touchControlDraw(int16 x, int16 y, int16 w, int16 h, const Common::Rect &clip) = 0;

protected:
	~TouchControlsDrawer() {}
};

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

	void init(float scale);
	void setDrawer(TouchControlsDrawer *drawer, int width, int height);
	void draw();
	void update(Action action, int ptr, int x, int y);

private:
	TouchControlsDrawer *_drawer;

	unsigned int _screen_width, _screen_height;
	unsigned int _scale, _scale2;

	Graphics::ManagedSurface *_svg;

	enum Function {
		kFunctionNone = -1,
		kFunctionLeft = 0,
		kFunctionRight = 1,
		kFunctionCenter = 2,
		kFunctionMax = 2
	};
	Function getFunction(int x, int y);

	struct FunctionState {
		FunctionState() : mask(0) {}
		void reset() {
			mask = 0;
		}

		uint32 mask;
	};

	struct Pointer {
		Pointer() : id(-1), startX(-1), startY(-1),
			currentX(-1), currentY(-1),
			function(kFunctionNone), active(false) {}
		void reset() {
			id = -1;
			startX = startY = currentX = currentY = -1;
			function = kFunctionNone;
			active = false;

			state.reset();
		}

		int id;
		uint16 startX, startY;
		uint16 currentX, currentY;
		Function function;
		bool active;

		FunctionState state;
	};

	enum { kNumPointers = 5 };
	Pointer _pointers[kNumPointers];

	Pointer *getPointerFromId(int ptr, bool createNotFound);
	Pointer *findPointerFromFunction(Function function);

	void buttonDown(Common::JoystickButton jb);
	void buttonUp(Common::JoystickButton jb);
	void buttonPress(Common::JoystickButton jb);

	/**
	 * Draws a part of the joystick surface on the screen
	 *
	 * @param x     The left coordinate in fixed-point screen pixels
	 * @param y     The top coordinate in fixed-point screen pixels
	 * @param offX  The left offset in SVG pixels
	 * @param offY  The top offset in SVG pixels
	 * @param clip  The clipping rectangle in source surface in SVG pixels
	 */
	void drawSurface(int x, int y, int offX, int offY, const Common::Rect &clip);

	/* Functions implementations */
	struct FunctionBehavior {
		void (TouchControls::*touch)(int, int, Action, TouchControls::FunctionState &);
		void (TouchControls::*draw)(int, int, const TouchControls::FunctionState &);
	};
	static const FunctionBehavior functionBehaviors[TouchControls::kFunctionMax + 1];

	void touchLeft(int dX, int dY, Action action, FunctionState &state);
	void maskToLeftButtons(uint32 oldMask, uint32 newMask);
	void drawLeft(int centerX, int centerY, const FunctionState &state);
	void touchRight(int dX, int dY, Action action, FunctionState &state);
	void drawRight(int centerX, int centerY, const FunctionState &state);
	void touchCenter(int dX, int dY, Action action, FunctionState &state);
	void drawCenter(int centerX, int centerY, const FunctionState &state);

};

#endif
