/* Copyright (C) 1994-2003 Revolution Software Ltd
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "common/stdafx.h"
#include "base/engine.h"
#include "common/timer.h"

#include "driver96.h"

#include "_mouse.h"
#include "keyboard.h"
#include "rdwin.h"
#include "d_draw.h"
#include "palette.h"
#include "render.h"
#include "menu.h"
#include "d_sound.h"
#include "bs2/sword2.h"

#define MENUDEEP 40		// Temporary, until menu.h is written!

// ---------------------------------------------------------------------------
// OSystem Event Handler. Full of cross platform goodness and 99% fat free!
// ---------------------------------------------------------------------------

void Sword2State::parseEvents() {
	OSystem::Event event;
	
	while (_system->poll_event(&event)) {
		switch(event.event_code) {
		case OSystem::EVENT_KEYDOWN:
			WriteKey(event.kbd.ascii, event.kbd.keycode, event.kbd.flags);
			break;
		case OSystem::EVENT_MOUSEMOVE:
			mousex = event.mouse.x;
			mousey = event.mouse.y - MENUDEEP;
			break;
		case OSystem::EVENT_LBUTTONDOWN:
			LogMouseEvent(RD_LEFTBUTTONDOWN);
			break;
		case OSystem::EVENT_RBUTTONDOWN:
			LogMouseEvent(RD_RIGHTBUTTONDOWN);
			break;
		case OSystem::EVENT_LBUTTONUP:
			LogMouseEvent(RD_LEFTBUTTONUP);
			break;
		case OSystem::EVENT_RBUTTONUP:
			LogMouseEvent(RD_RIGHTBUTTONUP);
			break;
		case OSystem::EVENT_QUIT:
			Close_game();
			CloseAppWindow();
			break;
		default:
			break;
		}
	}
}

/**
 * Quit the game.
 */

int32 CloseAppWindow(void) {
	warning("stub CloseAppWindow");
/*
	DestroyWindow(hwnd);
*/
	// just quit for now
	g_system->quit();
	return RD_OK;
}

static bool _needRedraw = false;

void SetNeedRedraw() {
	_needRedraw = true;
}

/**
 * This function should be called at a high rate (> 20 per second) to service
 * windows and the interface it provides.
 */

int32 ServiceWindows(void) {
	g_sword2->parseEvents();
	FadeServer();

	// FIXME: We re-render the entire picture area of the screen for each
	// frame, which is pretty horrible.

	if (_needRedraw) {
		g_system->copy_rect(lpBackBuffer + MENUDEEP * screenWide, screenWide, 0, MENUDEEP, screenWide, screenDeep - 2 * MENUDEEP);
		_needRedraw = false;
	}

	// We still need to update because of fades, menu animations, etc.
	g_system->update_screen();

	return RD_OK;
}

/**
 * Set the window name to windowName and stores this name in gameName for
 * future use.
 */

void SetWindowName(const char *windowName) {
	warning("stub SetWindowName( %s )", windowName);
	// SetWindowText(hwnd, windowName);
	// strcpy(gameName, windowName);
}
