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

#include "common/keyboard.h"
#include "darkseed/menu.h"
#include "darkseed/darkseed.h"

namespace Darkseed {

void drawSoundMenuItem() {
	g_engine->_screen->fillRect({{117, 136}, 55, 11}, 0);
	if (g_engine->_sound->isMuted()) {
		g_engine->_console->drawStringAt(123, 136, "Silent");
	} else {
		g_engine->_console->drawStringAt(117, 136, "Sounds");
	}
}

void loadMenu() {
	Graphics::Surface screenCopy;
	screenCopy.copyFrom(*g_engine->_screen);

	g_engine->showFullscreenPic(g_engine->_room->isGiger() ? "glady.pic" : "clady.pic");
	g_engine->_frame.draw();
	g_engine->drawFullscreenPic();
	g_engine->_console->draw(true);

	g_engine->_console->drawStringAt(127, 62, "Load");
	g_engine->_console->drawStringAt(127, 99, "Save");
	drawSoundMenuItem();
	g_engine->_console->drawStringAt(118, 173, "Resume");
	g_engine->_console->drawStringAt(129, 210, "Quit");

	g_engine->_screen->makeAllDirty();
	g_engine->_screen->update();

	g_engine->zeroMouseButtons();

	g_engine->_lastKeyPressed = Common::KeyCode::KEYCODE_INVALID;

	while (!g_engine->shouldQuit()) {
		g_engine->updateEvents();
		int menuItemIdx = -1;

		if (g_engine->_isLeftMouseClicked) {
			g_engine->zeroMouseButtons();

			Common::Point cursorPos = g_engine->_cursor.getPosition();

			if (cursorPos.x > 111 && cursorPos.x < 178 && cursorPos.y > 50 && cursorPos.y < 235) {
				menuItemIdx = (cursorPos.y - 50) / 37;
			}
		}

		if (g_engine->_lastKeyPressed != Common::KeyCode::KEYCODE_INVALID) {
			switch (g_engine->_lastKeyPressed) {
				case Common::KeyCode::KEYCODE_l: menuItemIdx = 0; break;
				case Common::KeyCode::KEYCODE_s: menuItemIdx = 1; break;
				case Common::KeyCode::KEYCODE_r: menuItemIdx = 3; break;
				case Common::KeyCode::KEYCODE_q: menuItemIdx = 4; break;
				default: break;
			}
			g_engine->_lastKeyPressed = Common::KeyCode::KEYCODE_INVALID;
		}

		if (menuItemIdx > -1 && menuItemIdx < 5) {
			g_engine->_sound->playSfx(5, 0, -1);
			g_engine->waitForSpeechOrSfx();
		}

		if (menuItemIdx == 0) {
			g_engine->loadGameDialog();
			break;
		}
		if (menuItemIdx == 1) {
			// restore game screen back for the save game thumbnail
			g_engine->_screen->copyRectToSurface(screenCopy, 0, 0, {screenCopy.w, screenCopy.h});
			g_engine->_screen->update();
			g_engine->saveGameDialog();
			break;
		}
		if (menuItemIdx == 2) { // sound settings
			g_engine->flipMute();
			drawSoundMenuItem();
		}
		if (menuItemIdx == 3) { // Resume
			break;
		}
		if (menuItemIdx == 4) {
			g_engine->_console->printTosText(16);
			g_engine->_console->draw();
			g_engine->_screen->update();
			g_engine->_lastKeyPressed = Common::KeyCode::KEYCODE_INVALID;
			while (!g_engine->shouldQuit()) {
				g_engine->updateEvents();
				if (g_engine->_lastKeyPressed == Common::KeyCode::KEYCODE_y) {
					g_engine->quitGame();
					break;
				}
				if (g_engine->_lastKeyPressed == Common::KeyCode::KEYCODE_n || g_engine->_lastKeyPressed == Common::KeyCode::KEYCODE_ESCAPE) {
					g_engine->_console->printTosText(17);
					g_engine->_console->draw();
					g_engine->_screen->update();
					break;
				}
				g_engine->_screen->update();
				g_engine->wait();
			}
		}

		g_engine->_screen->update();
		g_engine->wait();
	}

	g_engine->removeFullscreenPic();
}

} // End of namespace Darkseed
