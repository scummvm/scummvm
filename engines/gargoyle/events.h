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

#ifndef GARGOYLE_EVENTS_H
#define GARGOYLE_EVENTS_H

#include "common/events.h"

namespace Gargoyle {

class Window;

/**
 * Event types
 */
enum EvType {
	evtype_None         = 0,
	evtype_Timer        = 1,
	evtype_CharInput    = 2,
	evtype_LineInput    = 3,
	evtype_MouseInput   = 4,
	evtype_Arrange      = 5,
	evtype_Redraw       = 6,
	evtype_SoundNotify  = 7,
	evtype_Hyperlink    = 8,
	evtype_VolumeNotify = 9,

	// ScummVM custom events
	evtype_Quit			= 99
};

/**
 * Keycodes
 */
enum Keycode {
	keycode_Unknown  = 0xffffffffU,
	keycode_Left     = 0xfffffffeU,
	keycode_Right    = 0xfffffffdU,
	keycode_Up       = 0xfffffffcU,
	keycode_Down     = 0xfffffffbU,
	keycode_Return   = 0xfffffffaU,
	keycode_Delete   = 0xfffffff9U,
	keycode_Escape   = 0xfffffff8U,
	keycode_Tab      = 0xfffffff7U,
	keycode_PageUp   = 0xfffffff6U,
	keycode_PageDown = 0xfffffff5U,
	keycode_Home     = 0xfffffff4U,
	keycode_End      = 0xfffffff3U,
	keycode_Func1    = 0xffffffefU,
	keycode_Func2    = 0xffffffeeU,
	keycode_Func3    = 0xffffffedU,
	keycode_Func4    = 0xffffffecU,
	keycode_Func5    = 0xffffffebU,
	keycode_Func6    = 0xffffffeaU,
	keycode_Func7    = 0xffffffe9U,
	keycode_Func8    = 0xffffffe8U,
	keycode_Func9    = 0xffffffe7U,
	keycode_Func10   = 0xffffffe6U,
	keycode_Func11   = 0xffffffe5U,
	keycode_Func12   = 0xffffffe4U,

	// non standard keycodes
	keycode_Erase          = 0xffffef7fU,
	keycode_MouseWheelUp   = 0xffffeffeU,
	keycode_MouseWheelDown = 0xffffefffU,
	keycode_SkipWordLeft   = 0xfffff000U,
	keycode_SkipWordRight  = 0xfffff001U,

	// The last keycode is always = 0x100000000 - keycode_MAXVAL)
	keycode_MAXVAL = 28U
};

/**
 * Event structure
 */
struct Event {
	EvType _type;
	Window *_window;
	uint32 _val1, _val2;

	/**
	 * Constructor
	 */
	Event() : _type(evtype_None), _window(nullptr), _val1(0), _val2(0) {}
};

class Events {
public:
	bool _forceClick;
public:
	/**
	 * Constructor
	 */
	Events() : _forceClick(false) {}

	/**
	 * Checks for new events
	 */
	void pollEvents();

	void clearEvent(Event *ev);
};

} // End of namespace Gargoyle

#endif
