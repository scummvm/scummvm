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
	virtual void touchControlDraw(uint8 alpha, int16 x, int16 y, int16 w, int16 h, const Common::Rect &clip) = 0;

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
	void beforeDraw();
	void draw();
	void update(Action action, int ptr, int x, int y);

private:
	TouchControlsDrawer *_drawer;

	unsigned int _screen_width, _screen_height;
	unsigned int _scale, _scale2;

	Graphics::ManagedSurface *_svg;

	unsigned int _zombieCount;

	enum State {
		kFunctionInactive = 0,
		kFunctionActive = 1,
		kFunctionZombie = 2
	};

	struct Function {
		virtual bool isInside(int, int) = 0;
		virtual void touch(int, int, Action) = 0;
		virtual void draw(uint8 alpha) = 0;
		virtual void resetState() {}

		Function(const TouchControls *parent_) :
			parent(parent_), pointerId(-1),
			startX(-1), startY(-1),
			currentX(-1), currentY(-1),
			lastActivable(0), status(kFunctionInactive) {}
		virtual ~Function() {}

		void reset() {
			pointerId = -1;
			startX = startY = currentX = currentY = -1;
			lastActivable = 0;
			status = kFunctionInactive;
			resetState();
		}

		const TouchControls *parent;

		int pointerId;
		uint16 startX, startY;
		uint16 currentX, currentY;
		uint32 lastActivable;
		State status;
	};
	Function *getFunctionFromPointerId(int ptr);
	Function *getZombieFunctionFromPos(int x, int y);

	enum FunctionId {
		kFunctionNone = -1,
		kFunctionLeft = 0,
		kFunctionRight = 1,
		kFunctionCenter = 2,
		kFunctionCount = 3
	};
	FunctionId getFunctionId(int x, int y);

	Function *_functions[kFunctionCount];

	static void buttonDown(Common::JoystickButton jb);
	static void buttonUp(Common::JoystickButton jb);
	static void buttonPress(Common::JoystickButton jb);

	/**
	 * Draws a part of the joystick surface on the screen
	 *
	 * @param x     The left coordinate in fixed-point screen pixels
	 * @param y     The top coordinate in fixed-point screen pixels
	 * @param offX  The left offset in SVG pixels
	 * @param offY  The top offset in SVG pixels
	 * @param clip  The clipping rectangle in source surface in SVG pixels
	 */
	void drawSurface(uint8 alpha, int x, int y, int offX, int offY, const Common::Rect &clip) const;


	// Functions implementations
	struct FunctionLeft : Function {
		FunctionLeft(const TouchControls *parent) :
			Function(parent), mask(0) {}
		void resetState() override { mask = 0; }

		bool isInside(int, int) override;
		void touch(int, int, Action) override;
		void draw(uint8 alpha) override;

		uint32 mask;
		void maskToLeftButtons(uint32 oldMask, uint32 newMask);
	};

	struct FunctionRight : Function {
		FunctionRight(const TouchControls *parent) :
			Function(parent), button(0) {}
		void resetState() override { button = 0; }

		bool isInside(int, int) override;
		void touch(int, int, Action) override;
		void draw(uint8 alpha) override;

		uint32 button;
	};

	struct FunctionCenter : Function {
		FunctionCenter(const TouchControls *parent) :
			Function(parent), button(0) {}
		void resetState() override { button = 0; }

		bool isInside(int, int) override;
		void touch(int, int, Action) override;
		void draw(uint8 alpha) override;

		uint32 button;
	};
};

#endif
