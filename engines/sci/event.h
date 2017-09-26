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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef SCI_EVENT_H
#define SCI_EVENT_H

#include "common/list.h"
#include "common/rect.h"

namespace Sci {

enum SciEventType {
	kSciEventNone         = 0,
	kSciEventMousePress   = 1,
	kSciEventMouseRelease = 1 << 1,
	kSciEventMouse        = kSciEventMousePress | kSciEventMouseRelease,
	kSciEventKeyDown      = 1 << 2,
	kSciEventKeyUp        = 1 << 3,
	kSciEventKey          = kSciEventKeyDown | kSciEventKeyUp,
	kSciEventDirection    = 1 << 6,
	kSciEventSaid         = 1 << 7,
#ifdef ENABLE_SCI32
	kSciEventHotRectangle = 1 << 10,
#endif
	kSciEventQuit         = 1 << 11,
	kSciEventPeek         = 1 << 15,

	kSciEventAny          = ~kSciEventPeek
};

inline SciEventType operator|(const SciEventType a, const SciEventType b) {
	return static_cast<SciEventType>((int)a | (int)b);
}

inline SciEventType &operator|=(SciEventType &a, const SciEventType b) {
	return a = static_cast<SciEventType>((int)a | (int)b);
}

enum SciKeyCode {
	kSciKeyEtx       = 3,
	kSciKeyBackspace = 8,
	kSciKeyTab       = '\t',
	kSciKeyEnter     = 13,
	kSciKeyEsc       = 27,
	kSciKeyShiftTab  = 15 << 8,

	kSciKeyHome      = 71 << 8, // numpad 7
	kSciKeyUp        = 72 << 8, // numpad 8
	kSciKeyPageUp    = 73 << 8, // numpad 9
	kSciKeyLeft      = 75 << 8, // numpad 4
	kSciKeyCenter    = 76 << 8, // numpad 5
	kSciKeyRight     = 77 << 8, // numpad 6
	kSciKeyEnd       = 79 << 8, // numpad 1
	kSciKeyDown      = 80 << 8, // numpad 2
	kSciKeyPageDown  = 81 << 8, // numpad 3
	kSciKeyInsert    = 82 << 8, // numpad 0
	kSciKeyDelete    = 83 << 8, // numpad .

	kSciKeyF1        = 59 << 8,
	kSciKeyF2        = 60 << 8,
	kSciKeyF3        = 61 << 8,
	kSciKeyF4        = 62 << 8,
	kSciKeyF5        = 63 << 8,
	kSciKeyF6        = 64 << 8,
	kSciKeyF7        = 65 << 8,
	kSciKeyF8        = 66 << 8,
	kSciKeyF9        = 67 << 8,
	kSciKeyF10       = 68 << 8,

	kSciKeyShiftF1   = 84 << 8,
	kSciKeyShiftF2   = 85 << 8,
	kSciKeyShiftF3   = 86 << 8,
	kSciKeyShiftF4   = 87 << 8,
	kSciKeyShiftF5   = 88 << 8,
	kSciKeyShiftF6   = 89 << 8,
	kSciKeyShiftF7   = 90 << 8,
	kSciKeyShiftF8   = 91 << 8,
	kSciKeyShiftF9   = 92 << 8,
	kSciKeyShiftF10  = 93 << 8
};

enum SciKeyModifiers {
	kSciKeyModNone      = 0,
	kSciKeyModRShift    = 1,
	kSciKeyModLShift    = 1 << 1,
	kSciKeyModShift     = kSciKeyModRShift | kSciKeyModLShift,
	kSciKeyModCtrl      = 1 << 2,
	kSciKeyModAlt       = 1 << 3,
	kSciKeyModScrLock   = 1 << 4,
	kSciKeyModNumLock   = 1 << 5,
	kSciKeyModCapsLock  = 1 << 6,
	kSciKeyModInsert    = 1 << 7,
	kSciKeyModNonSticky = kSciKeyModRShift | kSciKeyModLShift | kSciKeyModCtrl | kSciKeyModAlt,
	kSciKeyModAll       = ~kSciKeyModNone
};

inline SciKeyModifiers &operator|=(SciKeyModifiers &a, SciKeyModifiers b) {
	return a = static_cast<SciKeyModifiers>((int)a | (int)b);
}

struct SciEvent {
	SciEventType type;
	SciKeyModifiers modifiers;
	/**
	 * For keyboard events: the actual character of the key that was pressed
	 * For 'Alt', characters are interpreted by their
	 * PC keyboard scancodes.
	 */
	uint16 character;

	/**
	 * The mouse position at the time the event was created, in script
	 * coordinates (SCI16) or display coordinates (SCI32).
	 */
	Common::Point mousePos;

#ifdef ENABLE_SCI32
	/**
	 * The mouse position at the time the event was created, in script
	 * coordinates. Used only by SCI32.
	 */
	Common::Point mousePosSci;

	/**
	 * The currently active hot rectangle, or -1 if no hot rectangle is active.
	 * Used only by the chase scene in Phantasmagoria 1.
	 */
	int16 hotRectangleIndex;
#endif
};

class EventManager {
public:
	EventManager(bool fontIsExtended);
	~EventManager();

	void updateScreen();
	SciEvent getSciEvent(SciEventType mask);
	void flushEvents();

private:
	SciEvent getScummVMEvent();

	const bool _fontIsExtended;
	Common::List<SciEvent> _events;
#ifdef ENABLE_SCI32
public:
	void setHotRectanglesActive(const bool active);
	void setHotRectangles(const Common::Array<Common::Rect> &rects);
	void checkHotRectangles(const Common::Point &mousePosition);

private:
	bool _hotRectanglesActive;
	Common::Array<Common::Rect> _hotRects;
	int16 _activeRectIndex;
#endif
};

} // End of namespace Sci

#endif
