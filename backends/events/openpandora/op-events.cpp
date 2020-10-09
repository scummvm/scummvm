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

#include "common/scummsys.h"

/*
 * OpenPandora: Device Specific Event Handling.
 */

#if defined(OPENPANDORA)

#include "backends/events/openpandora/op-events.h"
#include "backends/graphics/openpandora/op-graphics.h"
#include "backends/platform/openpandora/op-sdl.h"

#include "common/translation.h"
#include "common/util.h"
#include "common/events.h"

enum {
	/* Touchscreen TapMode */
	TAPMODE_LEFT        = 0,
	TAPMODE_RIGHT       = 1,
	TAPMODE_HOVER       = 2,
	TAPMODE_HOVER_DPAD  = 3
};

OPEventSource::OPEventSource()
	: _buttonStateL(false),
	  _tapmodeLevel(TAPMODE_LEFT) {
}

void OPEventSource::ToggleTapMode() {
	if (_tapmodeLevel == TAPMODE_LEFT) {
		_tapmodeLevel = TAPMODE_RIGHT;
	} else if (_tapmodeLevel == TAPMODE_RIGHT) {
		_tapmodeLevel = TAPMODE_HOVER;
	} else if (_tapmodeLevel == TAPMODE_HOVER) {
		_tapmodeLevel = TAPMODE_HOVER_DPAD;
	} else if (_tapmodeLevel == TAPMODE_HOVER_DPAD) {
		_tapmodeLevel = TAPMODE_LEFT;
	} else {
		_tapmodeLevel = TAPMODE_LEFT;
	}
}

/* Custom handleMouseButtonDown/handleMouseButtonUp to deal with 'Tap Mode' for the touchscreen */

bool OPEventSource::handleMouseButtonDown(SDL_Event &ev, Common::Event &event) {
	if (ev.button.button != SDL_BUTTON_LEFT)
		return SdlEventSource::handleMouseButtonDown(ev, event);

	if (_buttonStateL == true) /* _buttonStateL = Left Trigger Held, force Right Click */
		event.type = Common::EVENT_RBUTTONDOWN;
	else if (_tapmodeLevel == TAPMODE_LEFT) /* TAPMODE_LEFT = Left Click Tap Mode */
		event.type = Common::EVENT_LBUTTONDOWN;
	else if (_tapmodeLevel == TAPMODE_RIGHT) /* TAPMODE_RIGHT = Right Click Tap Mode */
		event.type = Common::EVENT_RBUTTONDOWN;
	else if (_tapmodeLevel == TAPMODE_HOVER) /* TAPMODE_HOVER = Hover (No Click) Tap Mode */
		event.type = Common::EVENT_MOUSEMOVE;
	else if (_tapmodeLevel == TAPMODE_HOVER_DPAD) /* TAPMODE_HOVER_DPAD = Hover (DPad Clicks) Tap Mode */
		event.type = Common::EVENT_MOUSEMOVE;
	else
		event.type = Common::EVENT_LBUTTONDOWN; /* For normal mice etc. */

	return processMouseEvent(event, ev.button.x, ev.button.y);
}

bool OPEventSource::handleMouseButtonUp(SDL_Event &ev, Common::Event &event) {
	if (ev.button.button != SDL_BUTTON_LEFT)
		return SdlEventSource::handleMouseButtonUp(ev, event);

	if (_buttonStateL == true) /* _buttonStateL = Left Trigger Held, force Right Click */
		event.type = Common::EVENT_RBUTTONUP;
	else if (_tapmodeLevel == TAPMODE_LEFT) /* TAPMODE_LEFT = Left Click Tap Mode */
		event.type = Common::EVENT_LBUTTONUP;
	else if (_tapmodeLevel == TAPMODE_RIGHT) /* TAPMODE_RIGHT = Right Click Tap Mode */
		event.type = Common::EVENT_RBUTTONUP;
	else if (_tapmodeLevel == TAPMODE_HOVER) /* TAPMODE_HOVER = Hover (No Click) Tap Mode */
		event.type = Common::EVENT_MOUSEMOVE;
	else if (_tapmodeLevel == TAPMODE_HOVER_DPAD) /* TAPMODE_HOVER_DPAD = Hover (DPad Clicks) Tap Mode */
		event.type = Common::EVENT_MOUSEMOVE;
	else
		event.type = Common::EVENT_LBUTTONUP; /* For normal mice etc. */

	return processMouseEvent(event, ev.button.x, ev.button.y);
}

/* On the OpenPandora by default the ABXY and L/R Trigger buttons are returned by SDL as
   (A): SDLK_HOME (B): SDLK_END (X): SDLK_PAGEDOWN (Y): SDLK_PAGEUP (L): SDLK_RSHIFT (R): SDLK_RCTRL
*/

bool OPEventSource::remapKey(SDL_Event &ev, Common::Event &event) {

	if (_tapmodeLevel == TAPMODE_HOVER_DPAD) {
		switch (ev.key.keysym.sym) {
		case SDLK_LEFT:
			event.type = (ev.type == SDL_KEYDOWN) ? Common::EVENT_LBUTTONDOWN : Common::EVENT_LBUTTONUP;
			processMouseEvent(event, _mouseY, _mouseY);
			return true;
			break;
		case SDLK_RIGHT:
			event.type = (ev.type == SDL_KEYDOWN) ? Common::EVENT_RBUTTONDOWN : Common::EVENT_RBUTTONUP;
			processMouseEvent(event, _mouseX, _mouseY);
			return true;
			break;
#if defined(SDL_BUTTON_MIDDLE)
		case SDLK_UP:
			event.type = (ev.type == SDL_KEYDOWN) ? Common::EVENT_MBUTTONDOWN : Common::EVENT_MBUTTONUP;
			processMouseEvent(event, _mouseX, _mouseY);
			return true;
			break;
#endif
		default:
		  break;
		}
	}

	if (ev.type == SDL_KEYDOWN) {
		switch (ev.key.keysym.sym) {
		case SDLK_HOME:
			event.type = Common::EVENT_LBUTTONDOWN;
			processMouseEvent(event, _mouseX, _mouseY);
			return true;
			break;
		case SDLK_END:
			event.type = Common::EVENT_RBUTTONDOWN;
			processMouseEvent(event, _mouseX, _mouseY);
			return true;
			break;
		case SDLK_PAGEDOWN:
			event.type = Common::EVENT_MAINMENU;
			return true;
			break;
		case SDLK_PAGEUP:
			ToggleTapMode();
			if (_tapmodeLevel == TAPMODE_LEFT) {
				g_system->displayMessageOnOSD(_("Touchscreen 'Tap Mode' - Left Click"));
			} else if (_tapmodeLevel == TAPMODE_RIGHT) {
				g_system->displayMessageOnOSD(_("Touchscreen 'Tap Mode' - Right Click"));
			} else if (_tapmodeLevel == TAPMODE_HOVER) {
				g_system->displayMessageOnOSD(_("Touchscreen 'Tap Mode' - Hover (No Click)"));
			} else if (_tapmodeLevel == TAPMODE_HOVER_DPAD) {
				g_system->displayMessageOnOSD(_("Touchscreen 'Tap Mode' - Hover (DPad Clicks)"));
			}
			break;
		case SDLK_RSHIFT:
			_buttonStateL = true;
			break;
		case SDLK_RCTRL:
			break;
		default:
			return false;
			break;
		}
		return false;
	} else {
		switch (ev.key.keysym.sym) {
		case SDLK_HOME:
			event.type = Common::EVENT_LBUTTONUP;
			processMouseEvent(event, _mouseX, _mouseY);
			return true;
			break;
		case SDLK_END:
			event.type = Common::EVENT_RBUTTONUP;
			processMouseEvent(event, _mouseX, _mouseY);
			return true;
			break;
		case SDLK_PAGEDOWN:
			event.type = Common::EVENT_MAINMENU;
			return true;
			break;
		case SDLK_PAGEUP:
			break;
		case SDLK_RSHIFT:
			_buttonStateL = false;
			break;
		case SDLK_RCTRL:
			break;
		default:
			return false;
			break;
		}
		return false;
	}
	return false;
}

#endif
