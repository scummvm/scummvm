/* Copyright (C) 1994-2004 Revolution Software Ltd
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
#include "sword2/driver/menu.h"

namespace Sword2 {

// ---------------------------------------------------------------------------
// OSystem Event Handler. Full of cross platform goodness and 99% fat free!
// ---------------------------------------------------------------------------

void Input::parseEvents(void) {
	OSystem::Event event;
	
	while (_vm->_system->poll_event(&event)) {
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
		case OSystem::EVENT_WHEELUP:
			logMouseEvent(RD_WHEELUP);
			break;
		case OSystem::EVENT_WHEELDOWN:
			logMouseEvent(RD_WHEELDOWN);
			break;
		case OSystem::EVENT_QUIT:
			_vm->closeGame();
			break;
		default:
			break;
		}
	}
}

/**
 * Tell updateDisplay() that the scene needs to be completely updated.
 */

void Graphics::setNeedFullRedraw(void) {
	_needFullRedraw = true;
}

/**
 * This function has two purposes: It redraws the scene, and it handles input
 * events, palette fading, etc. It should be called at a high rate (> 20 per
 * second), but the scene is usually only redrawn about 12 times per second,
 * except when then screen is scrolling.
 *
 * @param redrawScene If true, redraw the scene.
 */

void Graphics::updateDisplay(bool redrawScene) {
	_vm->_input->parseEvents();
	fadeServer();

	if (redrawScene) {
		int i;

		// Note that the entire scene is always rendered, which is less
		// than optimal, but at least we can try to be intelligent
		// about updating the screen afterwards.

		if (_needFullRedraw) {
			// Update the entire screen. This is necessary when
			// scrolling, fading, etc.

			_vm->_system->copy_rect(_buffer + MENUDEEP * _screenWide, _screenWide, 0, MENUDEEP, _screenWide, _screenDeep - 2 * MENUDEEP);
			_needFullRedraw = false;
		} else {
			// Update only the dirty areas of the screen

			int j, x, y;
			int stripWide;

			for (i = 0; i < _gridDeep; i++) {
				stripWide = 0;

				for (j = 0; j < _gridWide; j++) {
					if (_dirtyGrid[i * _gridWide + j]) {
						stripWide++;
					} else if (stripWide) {
						x = CELLWIDE * (j - stripWide);
						y = CELLDEEP * i;
						_vm->_system->copy_rect(_buffer + y * _screenWide + x, _screenWide, x, y, stripWide * CELLWIDE, CELLDEEP);
						stripWide = 0;
					}
				}

				if (stripWide) {
					x = CELLWIDE * (j - stripWide);
					y = CELLDEEP * i;
					_vm->_system->copy_rect(_buffer + y * _screenWide + x, _screenWide, x, y, stripWide * CELLWIDE, CELLDEEP);
					stripWide = 0;
				}
			}
		}

		// Age the dirty cells one generation. This way we keep track
		// of both the cells that were updated this time, and the ones
		// that were updated the last time.

		for (i = 0; i < _gridWide * _gridDeep; i++)
			_dirtyGrid[i] >>= 1;
	}

	// We always need to update because of fades, menu animations, etc.
	_vm->_system->update_screen();
}

/**
 * Set the window title
 */

void Graphics::setWindowName(const char *windowName) {
	OSystem::Property prop;

	prop.caption = windowName;
	_vm->_system->property(OSystem::PROP_SET_WINDOW_CAPTION, &prop);
}

} // End of namespace Sword2
