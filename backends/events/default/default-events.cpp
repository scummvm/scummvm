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

#if !defined(DISABLE_DEFAULT_EVENTMANAGER)

#include "common/stdafx.h"
#include "common/system.h"
#include "backends/events/default/default-events.h"

DefaultEventManager::DefaultEventManager(OSystem *boss) :
	_boss(boss),
	_buttonState(0),
	_modifierState(0),
	_shouldQuit(false) {

 	assert(_boss);
}

bool DefaultEventManager::pollEvent(OSystem::Event &event) {
	bool result;
	
	result = _boss->pollEvent(event);
	
	if (result) {
		switch (event.type) {
		case OSystem::EVENT_KEYDOWN:
		case OSystem::EVENT_KEYUP:
			_modifierState = event.kbd.flags;
			break;
		case OSystem::EVENT_MOUSEMOVE:
			_mousePos = event.mouse;
			break;

		case OSystem::EVENT_LBUTTONDOWN:
			_mousePos = event.mouse;
			_buttonState |= LBUTTON;
			break;
		case OSystem::EVENT_LBUTTONUP:
			_mousePos = event.mouse;
			_buttonState &= ~LBUTTON;
			break;

		case OSystem::EVENT_RBUTTONDOWN:
			_mousePos = event.mouse;
			_buttonState |= RBUTTON;
			break;
		case OSystem::EVENT_RBUTTONUP:
			_mousePos = event.mouse;
			_buttonState &= ~RBUTTON;
			break;

		case OSystem::EVENT_QUIT:
			_shouldQuit = true;
			break;

		default:
			break;
		}
	}
	
	return result;
}

#endif // !defined(DISABLE_DEFAULT_EVENTMANAGER)
