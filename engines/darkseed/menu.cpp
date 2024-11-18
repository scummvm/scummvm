/* ScummVM - Graphic Adventure Engine
*
* ScummVM is the legal property of its developers, whose names
* are too numerous to list here. Please refer to the COPYRIGHT
* file distributed with this source distribution.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#include "darkseed/menu.h"
#include "darkseed/darkseed.h"

namespace Darkseed {

void loadMenu() {
	g_engine->showFullscreenPic(g_engine->_room->isGiger() ? "glady.pic" : "clady.pic");
	g_engine->_frame.draw();
	g_engine->drawFullscreenPic();

	g_engine->_console->drawStringAt(127, 62, "Load");
	g_engine->_console->drawStringAt(127, 99, "Save");
	if (true) { // TODO check if sound is enabled
		g_engine->_console->drawStringAt(117, 136, "Sounds");
	} else {
		g_engine->_console->drawStringAt(123, 136, "Silent");
	}
	g_engine->_console->drawStringAt(118, 173, "Resume");
	g_engine->_console->drawStringAt(129, 210, "Quit");

	g_engine->_screen->makeAllDirty();
	g_engine->_screen->update();

	g_engine->zeroMouseButtons();

	while (!g_engine->shouldQuit()) {
		g_engine->updateEvents();

		if (g_engine->_isLeftMouseClicked) {
			g_engine->zeroMouseButtons();

			int menuItemIdx = -1;
			Common::Point cursorPos = g_engine->_cursor.getPosition();

			if (cursorPos.x > 111 && cursorPos.x < 178 && cursorPos.y > 50 && cursorPos.y < 235) {
				menuItemIdx = (cursorPos.y - 50) / 37;
			}
			if (menuItemIdx > -1 && menuItemIdx < 5) {
				// FUN_1208_0dac_sound_related(14);
			}

			if (menuItemIdx == 0) {
				g_engine->loadGameDialog();
				break;
			}
			if (menuItemIdx == 1) {
				g_engine->saveGameDialog();
				break;
			}
			if (menuItemIdx == 3) { // Resume
				break;
			}

			if (menuItemIdx == 4) {
				g_engine->quitGame();
				break;
			}

		}

		g_engine->_screen->update();
	}

	g_engine->removeFullscreenPic();
}

} // End of namespace Darkseed
