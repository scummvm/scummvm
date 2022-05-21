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

#include "common/bitarray.h"
#include "gui/message.h"
#include "common/events.h"
#include "common/config-manager.h"
#include "common/savefile.h"

#include "hypno/hypno.h"

namespace Hypno {

void BoyzEngine::runCode(Code *code) {
	if (code->name == "<main_menu>")
		runMainMenu(code);
	else if (code->name == "<retry_menu>")
		runRetryMenu(code);
	else
		error("invalid hardcoded level: %s", code->name.c_str());
}

void BoyzEngine::runMainMenu(Code *code) {
	Common::Event event;
	byte *palette;
	Graphics::Surface *menu = decodeFrame("preload/mainmenu.smk", 0, &palette);
	loadPalette(palette, 0, 256);

	drawImage(*menu, 0, 0, false);
	_name.clear();
	bool cont = true;
    uint32 c = kHypnoColorWhiteOrBlue; // white
	while (!shouldQuit() && cont) {
		while (g_system->getEventManager()->pollEvent(event)) {
			// Events
			switch (event.type) {

			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				break;

			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_BACKSPACE)
					_name.deleteLastChar();
				else if (event.kbd.keycode == Common::KEYCODE_RETURN && !_name.empty()) {
					cont = false;
				}
				else if (Common::isAlpha(event.kbd.keycode)) {
					playSound("sound/m_choice.raw", 1);
					_name = _name + char(event.kbd.keycode - 32);
				}

				drawImage(*menu, 0, 0, false);
				drawString("block05.fgx", _name, 130, 57, 170, c);
				break;

			default:
				break;
			}
		}

		drawScreen();
		g_system->delayMillis(10);
	}

	_name.toLowercase();

	_nextLevel = code->levelIfWin;
	menu->free();
	delete menu;
}

void BoyzEngine::runRetryMenu(Code *code) {
	_lives--;

	uint32 idx = _rnd->getRandomNumber(_deathVideo.size() - 1);
	Filename filename = _deathVideo[idx];
	MVideo video(filename, Common::Point(0, 0), false, true, false);
	disableCursor();
	runIntro(video);
	changeCursor("crosshair");

	Common::Rect retryMissionBox(73, 62, 245, 77);
	Common::Rect restartTerritoryBox(73, 81, 245, 96);
	Common::Rect quitBox(73, 119, 245, 133);

	Common::Event event;
	Common::Point mousePos;
	byte *palette;
	Graphics::Surface *menu = decodeFrame("preload/mainmenu.smk", 5, &palette);
	loadPalette(palette, 0, 256);
	drawImage(*menu, 0, 0, false);
	bool cont = true;
	while (!shouldQuit() && cont) {
		while (g_system->getEventManager()->pollEvent(event)) {
			mousePos = g_system->getEventManager()->getMousePos();

			// Events
			switch (event.type) {

			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				break;

			case Common::EVENT_LBUTTONDOWN:
				if (retryMissionBox.contains(mousePos)) {
					_nextLevel = _checkpoint;
					cont = false;
				} else if (restartTerritoryBox.contains(mousePos)) {
					// Restore initial health for the team
					_health = _maxHealth;
					_nextLevel = firstLevelTerritory(_checkpoint);
					cont = false;
				} else if (quitBox.contains(mousePos))
					quitGame();
				break;

			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_s) {
					_nextLevel = _checkpoint;
					cont = false;
				} else if (event.kbd.keycode == Common::KEYCODE_t) {
					// Restore initial health for the team
					_health = _maxHealth;
					_nextLevel = firstLevelTerritory(_checkpoint);
					cont = false;
				} else if (event.kbd.keycode == Common::KEYCODE_q)
					quitGame();
				break;

			default:
				break;
			}
		}

		drawScreen();
		g_system->delayMillis(10);
	}

	menu->free();
	delete menu;
}

void BoyzEngine::endCredits(Code *code) {
	showCredits();
	_nextLevel = "<main_menu>";
}

void BoyzEngine::showCredits() {
	MVideo c1("intro/sbcred1.smk", Common::Point(0, 0), false, true, false);
	runIntro(c1);
	MVideo c2("intro/sbcred2.smk", Common::Point(0, 0), false, true, false);
	runIntro(c2);
}

Common::String BoyzEngine::firstLevelTerritory(const Common::String &level) {
	if (Common::matchString(level.c_str(), "c1#.mi_"))
		return "c19.mi_";
	else if (Common::matchString(level.c_str(), "c2#.mi_"))
		return "c21.mi_";
	else if (Common::matchString(level.c_str(), "c3#.mi_"))
		return "c31.mi_";
	else if (Common::matchString(level.c_str(), "c4#.mi_"))
		return "c41.mi_";
	else if (Common::matchString(level.c_str(), "c5#.mi_"))
		return "c51.mi_";
	else
		error("Invalid territory for level %s", level.c_str());
}

} // End of namespace Hypno