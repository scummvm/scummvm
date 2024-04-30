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

#ifndef ULTIMA8_KERNEL_MOUSE_H
#define ULTIMA8_KERNEL_MOUSE_H

#include "common/system.h"
#include "common/rect.h"
#include "ultima/ultima8/misc/common_types.h"
#include "ultima/ultima8/misc/direction.h"

namespace Ultima {
namespace Ultima8 {

enum MouseButtonState {
	MBS_DOWN = 0x1,
	MBS_HANDLED = 0x2		// Mousedown event handled
};

struct MButton {
	uint16 _downGump;
	uint32 _lastDown;
	uint32 _curDown;
	Common::Point _downPoint;
	int _state;

	MButton() : _downGump(0), _curDown(0), _lastDown(0), _state(MBS_HANDLED)
	{
	}

	bool isState(MouseButtonState state) const {
		return _state & state;
	}
	void setState(MouseButtonState state) {
		_state |= state;
	}
	void clearState(MouseButtonState state) {
		_state &= ~state;
	}

	//! True if the current state is unhandled and past the double click timeout.
	bool isUnhandledPastTimeout(uint32 now, uint32 timeout) {
		return !isState(MBS_HANDLED) && _curDown > 0 && (now - _curDown) > timeout;
	}

	//! True if the current state is unhandled and within the double click timeout.
	bool isUnhandledDoubleClick(uint32 timeout) {
		return !isState(MBS_HANDLED) && _lastDown > 0 && (_curDown - _lastDown) <= timeout;
	}

};

class Gump;

class Mouse {
public:
	enum MouseButton {
		BUTTON_NONE = 0,
		BUTTON_LEFT = 1,
		BUTTON_RIGHT = 2,
		BUTTON_MIDDLE = 3,
		MOUSE_LAST
	};

	enum MouseCursor {
		MOUSE_NORMAL = 0,
		MOUSE_NONE = 1,
		MOUSE_TARGET = 2,
		MOUSE_WAIT = 3,
		MOUSE_HAND = 4,
		MOUSE_QUILL = 5,
		MOUSE_MAGGLASS = 6,
		MOUSE_CROSS = 7
	};

	enum DraggingState {
		DRAG_NOT = 0,
		DRAG_OK = 1,
		DRAG_INVALID = 2,
		DRAG_TEMPFAIL = 3
	};
private:
	static Mouse *_instance;
	Common::Stack<MouseCursor> _cursors;
	int _lastMouseFrame;

	/**
	 * Time mouse started flashing, or 0
	 */
	uint32 _flashingCursorTime;

	// mouse input state
	MButton _mouseButton[MOUSE_LAST];

	uint16 _mouseOverGump;
	Common::Point _mousePos;
	Common::Point _draggingOffset;
	DraggingState _dragging;

	ObjId _dragging_objId;
	uint16 _draggingItem_startGump;
	uint16 _draggingItem_lastGump;

	int _walkThreshold;
	int _runThreshold;
private:
	void startDragging(int mx, int my);
	void moveDragging(int mx, int my);
	void stopDragging(int mx, int my);
	int mouseFrameForDir(Direction mousedir) const;

public:
	static Mouse *get_instance() { return _instance; }
public:
	Mouse();
	~Mouse();

	/**
	 * Called when a mouse button is pressed down
	 */
	bool buttonDown(MouseButton button);

	/**
	 * Called when a mouse ubtton is released
	 */
	bool buttonUp(MouseButton button);

	//! get mouse cursor length. 0 = short, 1 = medium, 2 = long
	int getMouseLength(int mx, int my) const;

	//! get mouse cursor length for the current coordinates
	int getMouseLength() const {
		return getMouseLength(_mousePos.x, _mousePos.y);
	}

	//! get mouse cursor direction on the screen. 0 = up, 1 = up-right, 2 = right, etc...
	Direction getMouseDirectionScreen(int mx, int my) const;

	//! get mouse cursor direction on the screen using the current coordinates.
	Direction getMouseDirectionScreen() const {
		return getMouseDirectionScreen(_mousePos.x, _mousePos.y);
	}

	//! get mouse cursor direction in the world. 0 = up, 1 = up-right, 2 = right, etc...
	Direction getMouseDirectionWorld(int mx, int my) const;

	//! get mouse cursor direction in the world using the current coordinates.
	Direction getMouseDirectionWorld() const {
		return getMouseDirectionWorld(_mousePos.x, _mousePos.y);
	}

	//! get current mouse cursor location
	void getMouseCoords(int32 &mx, int32 &my) const {
		mx = _mousePos.x;
		my = _mousePos.y;
	}

	//! set current mouse cursor location
	void setMouseCoords(int mx, int my);

	bool isMouseDownEvent(MouseButton button) const;

	//! remove all existing cursors
	void popAllCursors();

	//! set the current mouse cursor
	void setMouseCursor(MouseCursor cursor);

	//! flash the red cross mouse cursor for a brief while
	void flashCrossCursor();

	//! push the current mouse cursor to the stack
	void pushMouseCursor(MouseCursor cursor);

	//! pop the last mouse cursor from the stack
	void popMouseCursor();

	//! get the current mouse frame
	int getMouseFrame();

	DraggingState dragging() const { return _dragging; }

	void setDraggingOffset(int32 x, int32 y) {
		_draggingOffset.x = x;
		_draggingOffset.y = y;
	}
	void getDraggingOffset(int32 &x, int32 &y) {
		x = _draggingOffset.x;
		y = _draggingOffset.y;
	}

	uint32 getDoubleClickTime() const;

	void handleDelayedEvents();

	Gump *getMouseOverGump() const;
	void resetMouseOverGump() { _mouseOverGump = 0; }

	void update();
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
