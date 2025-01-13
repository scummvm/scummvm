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
#include "common/config-manager.h"
#include "darkseed/menu.h"
#include "darkseed/darkseed.h"
#include "darkseed/langtext.h"

namespace Darkseed {

constexpr I18NTextWithPosition kMenu_load = {
	{127, 62, "Load"},
	{119, 62, "CARGAR"},
	{118, 62, "CHARGER"},
	{121, 62, "LADEN"}
};

constexpr I18NTextWithPosition kMenu_silent = {
	{123, 136, "Silent"},
	{115, 136, "SIN SON"},
	{118, 136, "SILENCE"},
	{129, 136, "RUHE"}
};

constexpr I18NTextWithPosition kMenu_sounds = {
	{117, 136, "Sounds"},
	{121, 136, "SONIDO"},
	{126, 136, "SONS"},
	{123, 136, "SOUND"}
};

constexpr I18NTextWithPosition kMenu_save = {
	{127, 99, "Save"},
	{119, 99, "GRABAR"},
	{121, 99, "SAUVER"},
	{115, 99, "SICHERN"}
};

constexpr I18NTextWithPosition kMenu_resume = {
	{118, 173, "Resume"},
	{122, 173, "SEQUIR"},
	{124, 173, "JOUER"},
	{118, 173, "WEITER"}
};

constexpr I18NTextWithPosition kMenu_quit = {
	{129, 210, "Quit"},
	{125, 210, "SALIR"},
	{117, 210, "QUITTER"},
	{129, 210, "ENDE"}
};

void drawMenuItem(const I18NTextWithPosition &menuText) {
	const TextWithPosition &textWithPosition = getI18NTextWithPosition(menuText);
	g_engine->_console->drawStringAt(textWithPosition.x, textWithPosition.y, textWithPosition.text);
}

void Menu::drawSoundMenuItem() {
	g_engine->_screen->fillRect({{115, 136}, 62, 11}, 0);
	if (g_engine->_sound->isMuted()) {
		drawMenuItem(kMenu_silent);
	} else {
		drawMenuItem(kMenu_sounds);
	}
}

Common::KeyCode Menu::getLocalisedConfirmToQuitKeycode() {
	switch (g_engine->getLanguage()) {
	case Common::ES_ESP : return Common::KeyCode::KEYCODE_s;
	case Common::FR_FRA : return Common::KeyCode::KEYCODE_o;
	case Common::DE_DEU : return Common::KeyCode::KEYCODE_j;
	default : return Common::KeyCode::KEYCODE_y;
	}
}

void Menu::loadMenu() {
	_open = true;
	Graphics::Surface screenCopy;
	screenCopy.copyFrom(*g_engine->_screen);

	g_engine->showFullscreenPic(g_engine->_room->isGiger() ? "glady.pic" : "clady.pic");
	g_engine->_frame.draw();
	g_engine->drawFullscreenPic();
	g_engine->_console->draw(true);

	drawMenuItem(kMenu_load);
	drawMenuItem(kMenu_save);
	drawSoundMenuItem();
	drawMenuItem(kMenu_resume);
	drawMenuItem(kMenu_quit);

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

		if (menuItemIdx > -1) {
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
			_open = false; // mark menu as closed to allow saving.
			g_engine->saveGameDialog();
			break;
		}
		if (menuItemIdx == 2) { // sound settings
			g_engine->flipMute();
			ConfMan.flushToDisk();
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
				if (g_engine->_lastKeyPressed == getLocalisedConfirmToQuitKeycode() || g_engine->_isLeftMouseClicked) {
					g_engine->quitGame();
					break;
				}
				if (g_engine->_lastKeyPressed == Common::KeyCode::KEYCODE_n ||
					g_engine->_lastKeyPressed == Common::KeyCode::KEYCODE_ESCAPE ||
					g_engine->_isRightMouseClicked) {
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
	_open = false;
}

} // End of namespace Darkseed
