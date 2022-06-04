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

void WetEngine::endCredits(Code *code) {
	showCredits();
	_nextLevel = "<main_menu>";
}

void WetEngine::runCode(Code *code) {
	changeScreenMode("320x200");
	if (code->name == "<main_menu>")
		runMainMenu(code);
	else if (code->name == "<level_menu>")
		runLevelMenu(code);
	else if (code->name == "<check_lives>")
		runCheckLives(code);
	else if (code->name == "<credits>")
		endCredits(code);
	else
		error("invalid hardcoded level: %s", code->name.c_str());
}

void WetEngine::runCheckLives(Code *code) {
	if (_lives < 0) {
		_nextLevel = "<game_over>";
		_score = 0;
		_lives = 2;
		restoreScoreMilestones(_score);
		saveProfile(_name, _lastLevel);
	} else
		_nextLevel = _checkpoint;
}

void WetEngine::runLevelMenu(Code *code) {
	if (_lastLevel == 0) {
		_nextLevel = Common::String::format("c%d", _ids[0]);
		return;
	}

	Common::Event event;
	byte *palette;
	Graphics::Surface *menu = decodeFrame("c_misc/menus.smk", 20, &palette);
	loadPalette(palette, 0, 256);
	byte black[3] = {0x00, 0x00, 0x00}; // Always red?
	byte lime[3] = {0x00, 0xFF, 0x00}; // Always red?
	byte green[3] = {0x2C, 0x82, 0x28}; // Always red?
	int maxLevel = 20;
	int currentLevel = 0;
	for (int i = 0; i < maxLevel; i++)
		if (i <= _lastLevel)
			loadPalette((byte *) &green, 192+i, 1);
		else
			loadPalette((byte *) &black, 192+i, 1);

	loadPalette((byte *) &lime, 192+currentLevel, 1);
	drawImage(*menu, 0, 0, false);
	bool cont = true;
	playSound("sound/bub01.raw", 0, 22050);
	while (!shouldQuit() && cont) {
		while (g_system->getEventManager()->pollEvent(event)) {
			// Events
			switch (event.type) {

			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				break;

			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_DOWN && currentLevel < _lastLevel) {
					playSound("sound/m_hilite.raw", 1, 11025);
					currentLevel++;
				} else if (event.kbd.keycode == Common::KEYCODE_UP && currentLevel > 0) {
					playSound("sound/m_hilite.raw", 1, 11025);
					currentLevel--;
				} else if (event.kbd.keycode == Common::KEYCODE_RETURN ) {
					playSound("sound/m_choice.raw", 1, 11025);
					_nextLevel = Common::String::format("c%d", _ids[currentLevel]);
					cont = false;
				} else if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
					openMainMenuDialog();
				}

				for (int i = 0; i < maxLevel; i++)
					if (i <= _lastLevel)
						loadPalette((byte *) &green, 192+i, 1);
					else
						loadPalette((byte *) &black, 192+i, 1);


				loadPalette((byte *) &lime, 192+currentLevel, 1);
				drawImage(*menu, 0, 0, false);
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

void WetEngine::runMainMenu(Code *code) {
	Common::Event event;
	uint32 c = 252; // green
	byte *palette;
	Graphics::Surface *menu = decodeFrame("c_misc/menus.smk", 16, &palette);
	Graphics::Surface *overlay = decodeFrame("c_misc/menus.smk", 18, nullptr);
	loadPalette(palette, 0, 256);
	Common::Rect subName(21, 10, 169, 24);

	drawImage(*menu, 0, 0, false);
	Graphics::Surface surName = overlay->getSubArea(subName);
	drawImage(surName, subName.left, subName.top, true);
	drawString("scifi08.fgx", _enterNameString, 48, 50, 100, c);
	_name.clear();
	bool cont = true;
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
				} else if (Common::isAlpha(event.kbd.keycode)) {
					playSound("sound/m_choice.raw", 1);
					_name = _name + char(event.kbd.keycode - 32);
				} if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
					openMainMenuDialog();
				}

				drawImage(*menu, 0, 0, false);
				drawImage(surName, subName.left, subName.top, true);
				drawString("scifi08.fgx", _enterNameString, 48, 50, 100, c);
				drawString("scifi08.fgx", _name, 140, 50, 170, c);
				break;


			default:
				break;
			}
		}

		drawScreen();
		g_system->delayMillis(10);
	}

	if (_name == "COOLCOLE") {
		_lastLevel = 19;
		playSound("sound/extra.raw", 1);
	} else
		_lastLevel = 0;

	if (_name == "ELRAPIDO") {
		_infiniteAmmoCheat = true;
		playSound("sound/extra.raw", 1);
	}

	if (_name == "SAVANNAH") {
		_infiniteHealthCheat = true;
		playSound("sound/extra.raw", 1);
	}

	_name.toLowercase();
	bool found = loadProfile(_name);

	if (found) {
		menu->free();
		delete menu;
		overlay->free();
		delete overlay;
		return;
	}


	saveProfile(_name, _ids[_lastLevel]);
	_name.toUppercase();  // We do this in order to show it again

	Common::Rect subDifficulty(20, 104, 233, 119);
	Graphics::Surface surDifficulty = overlay->getSubArea(subDifficulty);
	drawImage(*menu, 0, 0, false);
	drawImage(surDifficulty, subDifficulty.left, subDifficulty.top, true);

	Common::Rect subWet(129, 149, 195, 159);
	Graphics::Surface surWet = overlay->getSubArea(subWet);
	drawImage(surWet, subWet.left, subWet.top, true);
	playSound("sound/no_rapid.raw", 1, 11025);

	Common::Rect subDamp(52, 149, 115, 159);
	Graphics::Surface surDamp = overlay->getSubArea(subDamp);

	Common::Rect subSoaked(202, 149, 272, 159);
	Graphics::Surface surSoaked = overlay->getSubArea(subSoaked);

	Common::Array<Common::String> difficulties;
	difficulties.push_back("0");
	difficulties.push_back("1");
	difficulties.push_back("2");
	uint32 idx = 1;

	drawString("scifi08.fgx", _enterNameString, 48, 50, 100, c);
	drawString("scifi08.fgx", _name, 140, 50, 170, c);

	cont = true;
	while (!shouldQuit() && cont) {
		while (g_system->getEventManager()->pollEvent(event)) {
			// Events
			switch (event.type) {

			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				break;

			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_LEFT && idx > 0) {
					playSound("sound/no_rapid.raw", 1, 11025);
					idx--;
				} else if (event.kbd.keycode == Common::KEYCODE_RIGHT && idx < 2) {
					playSound("sound/no_rapid.raw", 1, 11025);
					idx++;
				} else if (event.kbd.keycode == Common::KEYCODE_RETURN)
					cont = false;

				drawImage(*menu, 0, 0, false);
				drawImage(surDifficulty, subDifficulty.left, subDifficulty.top, true);

				if (difficulties[idx] == "0")
					drawImage(surDamp, subDamp.left, subDamp.top, true);
				else if (difficulties[idx] == "1")
					drawImage(surWet, subWet.left, subWet.top, true);
				else if (difficulties[idx] == "2")
					drawImage(surSoaked, subSoaked.left, subSoaked.top, true);
				else
					error("Invalid difficulty: %s", difficulties[idx].c_str());

				drawString("scifi08.fgx", _enterNameString, 48, 50, 100, c);
				drawString("scifi08.fgx", _name, 140, 50, 170, c);

				break;
			default:
				break;
			}
		}
		drawScreen();
		g_system->delayMillis(10);
	}
	_name.toLowercase(); // make sure it is lowercase when we finish
	_difficulty = difficulties[idx];
	_nextLevel = code->levelIfWin;
	menu->free();
	delete menu;
	overlay->free();
	delete overlay;
}

void WetEngine::showDemoScore() {
	Common::String fmessage = "You finished this demo level with an accuracy of %d%% and a score of %d points";
	Common::String message = Common::String::format(fmessage.c_str(), accuracyRatio(), _score);
	GUI::MessageDialog dialog(message);
	dialog.runModal();
}

Common::String WetEngine::getLocalizedString(const Common::String name) {
	if (name == "name") {
		switch (_language) {
		case Common::FR_FRA:
			return "NOM :";
		case Common::ES_ESP:
			return "NOMBRE :";
		default:
			return "ENTER NAME :";
		}
	} else if (name == "health") {
		switch (_language) {
		case Common::FR_FRA:
			return "ENERGIE";
		case Common::ES_ESP:
			return "ENERGIA";
		default:
			return "HEALTH";
		}
	} else if (name == "objectives") {
		switch (_language) {
		case Common::FR_FRA:
			return "OBJ.";
		case Common::ES_ESP:
			return "O. M.";
		default:
			return "M. O.";
		}
	} else if (name == "score") {
		switch (_language) {
		case Common::ES_ESP:
			return "PUNTOS";
		default:
			return "SCORE";
		}
	} else if (name == "target") {
		switch (_language) {
		case Common::FR_FRA:
			return "VERROUILLAGE";
		case Common::ES_ESP:
			return "BLANCO FIJADO";
		default:
			return "TARGET ACQUIRED";
		}
	} else if (name == "direction") {
		switch (_language) {
		case Common::FR_FRA:
			return "DIRECTION ?";
		case Common::ES_ESP:
			return "ELIGE DIRECCION";
		default:
			return "CHOOSE DIRECTION";
		}
	} else
		error("Invalid string name to localize: %s", name.c_str());
}

} // End of namespace Hypno
