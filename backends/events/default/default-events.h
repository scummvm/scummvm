/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2007 The ScummVM project
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
 * $URL$
 * $Id$
 *
 */

#if !defined(BACKEND_EVENTS_DEFAULT_H) && !defined(DISABLE_DEFAULT_EVENTMANAGER)
#define BACKEND_EVENTS_DEFAULT_H

#include "common/stdafx.h"
#include "common/events.h"

/*
At some point we will remove pollEvent from OSystem and change
DefaultEventManager to use a "boss" derived from this class:
class EventProvider {
public
	virtual bool pollEvent(Common::Event &event) = 0;
};

Backends which wish to use the DefaultEventManager then simply can
use a subclass of EventProvider.
*/

class DefaultEventManager : public Common::EventManager {
	OSystem *_boss;

	Common::Point _mousePos;
	int _buttonState;
	int _modifierState;
	bool _shouldQuit;

	// for continuous events (keyDown)
	enum {
		kKeyRepeatInitialDelay = 400,
		kKeyRepeatSustainDelay = 100
	};

	struct {
		uint16 ascii;
		byte flags;
		int keycode;
	} _currentKeyDown;
	uint32 _keyRepeatTime;

public:
	DefaultEventManager(OSystem *boss);

	virtual bool pollEvent(OSystem::Event &event);

	virtual Common::Point getMousePos() const { return _mousePos; }
	virtual int getButtonState() const { return _buttonState; }
	virtual int getModifierState() const { return _modifierState; }
	virtual int shouldQuit() const { return _shouldQuit; }
};

#endif
