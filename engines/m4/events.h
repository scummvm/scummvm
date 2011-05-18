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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef M4_EVENTS_H
#define M4_EVENTS_H

#include "common/events.h"
#include "common/rect.h"

#include "m4/globals.h"
#include "m4/assets.h"
#include "m4/sprite.h"
#include "m4/graphics.h"
#include "m4/console.h"

namespace M4 {

#define LEFT_BUTTON_DOWN	1 << 0
#define RIGHT_BUTTON_DOWN	1 << 1

enum M4EventType {
	MEVENT_NO_EVENT, MEVENT_MOVE,
	MEVENT_LEFT_CLICK, MEVENT_LEFT_HOLD, MEVENT_LEFT_DRAG, MEVENT_LEFT_RELEASE,
	MEVENT_RIGHT_CLICK, MEVENT_RIGHT_HOLD, MEVENT_RIGHT_DRAG, MEVENT_RIGHT_RELEASE,
	MEVENT_BOTH_CLICK, MEVENT_BOTH_HOLD, MEVENT_BOTH_DRAG, MEVENT_BOTH_RELEASE,
	MEVENT_DOUBLECLICK, MEVENT_DOUBLECLICK_HOLD, MEVENT_DOUBLECLICK_DRAG, MEVENT_DOUBLECLICK_RELEASE,
	KEVENT_KEY
};

enum M4MouseState {
	MSTATE_NO_EVENT, MSTATE_LEFT_CLICK_DOWN, MSTATE_RIGHT_CLICK_DOWN, MSTATE_BOTH_CLICK_DOWN,
	MSTATE_DOUBLECLICK_DOWN
};

enum M4CommonCursors {
	CURSOR_ARROW = 0,
	CURSOR_WAIT = 1,
	CURSOR_HOURGLASS = 5,
	CURSOR_LOOK = 6,
	CURSOR_TAKE = 8,
	CURSOR_USE = 9
};

class M4Sprite;
class SpriteAsset;

class Events {
private:
	MadsM4Engine *_vm;
	Common::Event _event;
	M4MouseState _mouseState;
	int32 _keyCode;
	int _mouseButtons;
	Console *_console;
	bool _ctrlFlag;
public:
	bool quitFlag;
	Events(MadsM4Engine *vm);
	virtual ~Events();

	Common::Event &event() { return _event; }
	Common::EventType type() { return _event.type; }

	// M4-centric methods
	M4EventType handleEvents();
	bool kbdCheck(uint32 &keyCode);
	int getMouseButtonsState() { return _mouseButtons; }
	Console* getConsole() { return _console; }
};


class Mouse {
private:
	MadsM4Engine *_vm;
	int _currentCursor, _lockedCursor;
	bool _locked;
	bool _cursorOn;
	M4Sprite *_cursor;
	SpriteAsset *_cursorSprites;
	Common::Rect _hideRect, _showRect;
	Common::Point _currentPos;

	void handleEvent(Common::Event &event);
	bool inHideArea();
	friend class Events;
public:
	Mouse(MadsM4Engine *vm);
	~Mouse();

	bool init(const char *seriesName, RGB8 *palette);
	bool setCursorNum(int cursorIndex);
	int getCursorNum() { return _currentCursor; }
	int cursorCount();
	Common::Point currentPos() const { return _currentPos; }
	M4Sprite *cursor() { return _cursor; }
	void cursorOn();
	void cursorOff();
	bool getCursorOn() { return _cursorOn; }
	void lockCursor(int cursorIndex);
	void unlockCursor();

	const char *getVerb();

	void resetMouse();
	void setHideRect(Common::Rect &r);
	void setShowRect(Common::Rect &r);
	const Common::Rect *getHideRect();
	const Common::Rect *getShowRect();
};

}

#endif
