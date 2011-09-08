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

namespace TsAGE {

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
	// Ringworld objects
	OBJECT_STUNNER = 0, OBJECT_SCANNER = 1, OBJECT_STASIS_BOX = 2,
	OBJECT_INFODISK = 3, OBJECT_STASIS_NEGATOR = 4, OBJECT_KEY_DEVICE = 5, OBJECT_MEDKIT = 6,
	OBJECT_LADDER = 7, OBJECT_ROPE = 8, OBJECT_KEY = 9, OBJECT_TRANSLATOR = 10, OBJECT_ALE = 11,
	OBJECT_PAPER = 12, OBJECT_WALDOS = 13, OBJECT_STASIS_BOX2 = 14, OBJECT_RING = 15,
	OBJECT_CLOAK = 16, OBJECT_TUNIC = 17, OBJECT_CANDLE = 18, OBJECT_STRAW = 19, OBJECT_SCIMITAR = 20,
	OBJECT_SWORD = 21, OBJECT_HELMET = 22, OBJECT_ITEMS = 23, OBJECT_CONCENTRATOR = 24,
	OBJECT_NULLIFIER = 25, OBJECT_PEG = 26, OBJECT_VIAL = 27, OBJECT_JACKET = 28,
	OBJECT_TUNIC2 = 29, OBJECT_BONE = 30, OBJECT_EMPTY_JAR = 31, OBJECT_JAR = 32,

	// Blue Force objects
	INV_NONE = 0, INV_COLT45 = 1, INV_AMMO_CLIP = 2, INV_SPARE_CLIP = 3, INV_HANDCUFFS = 4, 
	INV_GREENS_GUN = 5,	INV_TICKET_BOOK = 6, INV_MIRANDA_CARD = 7, INV_FOREST_RAP = 8, 
	INV_GREEN_ID = 9, INV_BASEBALL_CARD = 10, INV_BOOKING_GREEN = 11, INV_FLARE = 12, 
	INV_COBB_RAP = 13, INV_22_BULLET = 14, INV_AUTO_RIFLE = 15, INV_WIG = 16, INV_FRANKIE_ID = 17,
	INV_TYRONE_ID = 18, INV_22_SNUB = 19, INV_BOOKING_FRANKIE = 21, INV_BOOKING_GANG = 22,
	INV_FBI_TELETYPE = 23, INV_DA_NOTE = 24, INV_PRINT_OUT = 25, INV_WHAREHOUSE_KEYS = 26,
	INV_CENTER_PUNCH = 27, INV_TRANQ_GUN = 28, INV_HOOK = 29, INV_RAGS = 30, INV_JAR = 31,
	INV_SCREWDRIVER = 32, INV_D_FLOPPY = 33, INV_BLANK_DISK = 34, INV_STICK = 35,
	INV_CRATE1 = 36, INV_CRATE2 = 37, INV_SHOEBOX = 38, INV_BADGE = 39, INV_RENTAL_COUPON = 41,
	INV_NICKEL = 42, INV_LYLE_CARD = 43, INV_CARTER_NOTE = 44, INV_MUG_SHOT = 45, 
	INV_CLIPPING = 46, INV_MICROFILM  = 47, INV_WAVE_KEYS = 48,	INV_RENTAL_KEYS = 49, 
	INV_NAPKIN = 50, INV_DMV_PRINTOUT = 51, INV_FISHING_NET = 52, INV_ID = 53, 
	INV_9MM_BULLETS = 54, INV_SCHEDULE = 55, INV_GRENADES = 56, INV_YELLOW_CORD = 57,
	INV_HALF_YELLOW_CORD = 58, INV_BLACK_CORD = 59, INV_HALF_BLACK_CORD = 61, INV_WARRANT = 62,
	INV_JACKET = 63, INV_GREENS_KNIFE = 64, INV_DOG_WHISTLE = 65, INV_AMMO_BELT = 66,
	BF_LAST_INVENT = 67,

	// Cursors
	CURSOR_WALK = 0x100, CURSOR_LOOK = 0x200, CURSOR_700 = 700, CURSOR_USE = 0x400, CURSOR_TALK = 0x800,
	CURSOR_1000 = 0x1000, CURSOR_EXIT = 0x7004,
	CURSOR_NONE = -1, CURSOR_CROSSHAIRS = -2, CURSOR_ARROW = -3
};

class GfxSurface;

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
	void setCursor(GfxSurface &cursor);
	void setCursorFromFlag();
	CursorType getCursor() const { return _currentCursor; }
	void showCursor();
	CursorType hideCursor();
	bool isCursorVisible() const;

	bool pollEvent();
	void waitForPress(int eventMask = EVENT_BUTTON_DOWN | EVENT_KEYPRESS);

	bool getEvent(Event &evt, int eventMask = ~EVENT_MOUSE_MOVE);
	Common::Event event() { return _event; }
	Common::EventType type() { return _event.type; }
	uint32 getFrameNumber() const { return _frameNumber; }
	void delay(int numFrames);
	bool isInventoryIcon() const { return _currentCursor < 256; }

	virtual void listenerSynchronize(Serializer &s);
	static void loadNotifierProc(bool postFlag);
};

} // End of namespace TsAGE

#endif
