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
#include "sword2/sword2.h"
#include "sword2/driver/driver96.h"
#include "sword2/driver/d_draw.h"
#include "sword2/driver/render.h"
#include "sword2/driver/menu.h"
#include "sword2/driver/d_sound.h"

namespace Sword2 {

// ---------------------------------------------------------------------------
// OSystem Event Handler. Full of cross platform goodness and 99% fat free!
// ---------------------------------------------------------------------------

void Input::parseEvents(void) {
	OSystem::Event event;
	
	while (g_system->poll_event(&event)) {
		switch (event.event_code) {
		case OSystem::EVENT_KEYDOWN:
			writeKey(event.kbd.ascii, event.kbd.keycode, event.kbd.flags);
			break;
		case OSystem::EVENT_MOUSEMOVE:
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y - MENUDEEP;
			break;
		case OSystem::EVENT_LBUTTONDOWN:
			logMouseEvent(RD_LEFTBUTTONDOWN);
			break;
		case OSystem::EVENT_RBUTTONDOWN:
			logMouseEvent(RD_RIGHTBUTTONDOWN);
			break;
		case OSystem::EVENT_LBUTTONUP:
			logMouseEvent(RD_LEFTBUTTONUP);
			break;
		case OSystem::EVENT_RBUTTONUP:
			logMouseEvent(RD_RIGHTBUTTONUP);
			break;
		case OSystem::EVENT_QUIT:
			g_sword2->closeGame();
			break;
		default:
			break;
		}
	}
}

void Graphics::setNeedFullRedraw() {
	_needFullRedraw = true;
}

/**
 * This function should be called at a high rate (> 20 per second) to service
 * windows and the interface it provides.
 */

void Graphics::updateDisplay(void) {
	g_input->parseEvents();
	fadeServer();

	// FIXME: We re-render the entire picture area of the screen for each
	// frame, which is pretty horrible.

	if (_needFullRedraw) {
		g_system->copy_rect(_buffer + MENUDEEP * _screenWide, _screenWide, 0, MENUDEEP, _screenWide, _screenDeep - 2 * MENUDEEP);
		_needFullRedraw = false;
	}

	// We still need to update because of fades, menu animations, etc.
	g_system->update_screen();
}

/**
 * Set the window title
 */

void Graphics::setWindowName(const char *windowName) {
	OSystem::Property prop;

	prop.caption = windowName;
	g_system->property(OSystem::PROP_SET_WINDOW_CAPTION, &prop);
}

} // End of namespace Sword2
