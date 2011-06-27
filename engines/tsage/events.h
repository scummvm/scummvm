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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TSAGE_EVENTS_H
#define TSAGE_EVENTS_H

#include "common/events.h"
#include "common/array.h"
#include "common/str.h"
#include "graphics/surface.h"
#include "tsage/saveload.h"

namespace tSage {

enum EventType {EVENT_NONE = 0, EVENT_BUTTON_DOWN = 1, EVENT_BUTTON_UP = 2, EVENT_KEYPRESS = 4,
	EVENT_MOUSE_MOVE = 8};

enum ButtonShiftFlags {BTNSHIFT_LEFT = 0, BTNSHIFT_RIGHT = 3, BTNSHIFT_MIDDLE = 4};

// Intrinisc game delay between execution frames. This runs at 60Hz
#define GAME_FRAME_RATE 60
#define GAME_FRAME_TIME (1000 / 60)

class GfxManager;

class Event {
public:
	EventType eventType;
	Common::Point mousePos;
	int btnState;
	Common::KeyState kbd;
	int ctr;
	GfxManager *gfxMan;
	bool handled;
};

enum CursorType {
	OBJECT_STUNNER = 0, OBJECT_SCANNER = 1, OBJECT_STASIS_BOX = 2,
	OBJECT_INFODISK = 3, OBJECT_STASIS_NEGATOR = 4, OBJECT_KEY_DEVICE = 5, OBJECT_MEDKIT = 6,
	OBJECT_LADDER = 7, OBJECT_ROPE = 8, OBJECT_KEY = 9, OBJECT_TRANSLATOR = 10, OBJECT_ALE = 11,
	OBJECT_PAPER = 12, OBJECT_WALDOS = 13, OBJECT_STASIS_BOX2 = 14, OBJECT_RING = 15,
	OBJECT_CLOAK = 16, OBJECT_TUNIC = 17, OBJECT_CANDLE = 18, OBJECT_STRAW = 19, OBJECT_SCIMITAR = 20,
	OBJECT_SWORD = 21, OBJECT_HELMET = 22, OBJECT_ITEMS = 23, OBJECT_CONCENTRATOR = 24,
	OBJECT_NULLIFIER = 25, OBJECT_PEG = 26, OBJECT_VIAL = 27, OBJECT_JACKET = 28,
	OBJECT_TUNIC2 = 29, OBJECT_BONE = 30, OBJECT_EMPTY_JAR = 31, OBJECT_JAR = 32,

	CURSOR_WALK = 0x100, CURSOR_LOOK = 0x200, CURSOR_700 = 700, CURSOR_USE = 0x400, CURSOR_TALK = 0x800,
	CURSOR_NONE = -1, CURSOR_CROSSHAIRS = -2, CURSOR_ARROW = -3
};

class EventsClass : public SaveListener {
private:
	Common::Event _event;
	uint32 _frameNumber;
	uint32 _prevDelayFrame;
	uint32 _priorFrameTime;
public:
	EventsClass();

	Common::Point _mousePos;
	CursorType _currentCursor;
	CursorType _lastCursor;

	void setCursor(CursorType cursorType);
	void pushCursor(CursorType cursorType);
	void popCursor();
	void setCursor(Graphics::Surface &cursor, int transColor, const Common::Point &hotspot, CursorType cursorId);
	void setCursorFromFlag();
	CursorType getCursor() const { return _currentCursor; }
	void showCursor();
	void hideCursor();
	bool isCursorVisible() const;

	bool pollEvent();
	void waitForPress(int eventMask = EVENT_BUTTON_DOWN | EVENT_KEYPRESS);

	bool getEvent(Event &evt, int eventMask = ~EVENT_MOUSE_MOVE);
	Common::Event event() { return _event; }
	Common::EventType type() { return _event.type; }
	uint32 getFrameNumber() const { return _frameNumber; }
	void delay(int numFrames);

	virtual void listenerSynchronize(Serializer &s);
};

} // End of namespace tSage

#endif
