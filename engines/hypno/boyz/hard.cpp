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
	else if (code->name == "<difficulty_menu>")
		runDifficultyMenu(code);
	else if (code->name == "<retry_menu>")
		runRetryMenu(code);
	else if (code->name == "<check_c3>")
		runCheckC3(code);
	else if (code->name == "<check_ho>")
		runCheckHo(code);
	else if (code->name == "<check_c5>")
		runCheckC5(code);
	else if (code->name == "<alarm_c5>")
		runAlarmC5(code);
	else if (code->name == "<credits>")
		endCredits(code);
	else
		error("invalid hardcoded level: %s", code->name.c_str());
}

void BoyzEngine::runMainMenu(Code *code) {
	resetSceneState();
	resetStatistics();
	_globalStats = ArcadeStats();
	_flashbackMode = false;

	Common::Event event;
	byte *palette;
	Graphics::Surface *menu = decodeFrame("preload/mainmenu.smk", 0, &palette);
	loadPalette(palette, 0, 256);

	drawImage(*menu, 0, 0, false);
	_name.clear();
	bool cont = true;
    uint32 c = kHypnoColorWhiteOrBlue; // white
	uint32 posY = 105;
	Common::StringArray profiles = listProfiles();
	for (Common::StringArray::iterator it = profiles.begin(); it != profiles.end(); ++it) {
		drawString("block05.fgx", *it, 130, posY, 170, c);
		posY = posY + 10;
		if (posY >= 185)
			break;
	}
	g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);
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
				} else if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
					if (runExitMenu())
						quitGame();
					disableCursor();
				}

				else if (Common::isAlpha(event.kbd.keycode)) {
					playSound("sound/m_choice.raw", 1);
					_name = _name + char(event.kbd.keycode - 32);
				}

				drawImage(*menu, 0, 0, false);
				drawString("block05.fgx", _name, 130, 58, 170, c);
				posY = 105;
				for (Common::StringArray::iterator it = profiles.begin(); it != profiles.end(); ++it) {
					drawString("block05.fgx", *it, 130, posY, 170, c);
					posY = posY + 10;
					if (posY >= 185)
						break;
				}
				break;

			default:
				break;
			}
		}

		drawScreen();
		g_system->delayMillis(10);
	}
	g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
	menu->free();
	delete menu;

	if (shouldQuit())
		return;

	_name.toLowercase();
	bool found = loadProfile(_name);
	if (!found) {
		_nextLevel = code->levelIfWin;
	} else if (_unlockAllLevels) {
		_nextLevel = "<select_t1>";
		unlockAllLevels();
		_flashbackMode = true;
	}

	assert(!_nextLevel.empty());

}

bool BoyzEngine::runExitMenu() {
	changeCursor("crosshair");
	bool quit = false;
	Common::Event event;
	byte *palette;
	Graphics::Surface *menu = decodeFrame("preload/mainmenu.smk", 8, &palette);
	loadPalette(palette, 0, 256);
	drawImage(*menu, 0, 0, false);
	Common::Rect yesBox(142, 87, 179, 102);
	Common::Rect noBox(142, 104, 179, 119);
	bool cont = true;
	while (!shouldQuit() && cont) {
		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Point mousePos = g_system->getEventManager()->getMousePos();
			// Events
			switch (event.type) {

			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				break;

			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_y) {
					quit = true;
					cont = false;

				} else if (event.kbd.keycode == Common::KEYCODE_n) {
					quit = false;
					cont = false;
					break;
				}
			break;

			case Common::EVENT_LBUTTONDOWN:
				if (yesBox.contains(mousePos)) {
					quit = true;
					cont = false;
					break;
				} else if (noBox.contains(mousePos)) {
					quit = false;
					cont = false;
					break;
				}
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
	return quit;
}

void BoyzEngine::runDifficultyMenu(Code *code) {
	changeCursor("crosshair");
	_difficulty.clear();
	Common::Rect chumpBox(121, 62, 199, 77);
	Common::Rect punkBox(121, 81, 199, 96);
	Common::Rect badAssBox(121, 100, 199, 115);
	Common::Rect cancelBox(121, 138, 245, 153);

	Common::Event event;
	Common::Point mousePos;
	byte *palette;
	Graphics::Surface *menu = decodeFrame("preload/mainmenu.smk", 1, &palette);
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
				if (chumpBox.contains(mousePos)) {
					_difficulty = "chump";
					cont = false;
				} else if (punkBox.contains(mousePos)) {
					_difficulty = "punk";
					cont = false;
				} else if (badAssBox.contains(mousePos)) {
					_difficulty = "bad ass";
					cont = false;
				} else if (cancelBox.contains(mousePos)) {
					cont = false;
				}
				break;

			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_c) {
					_difficulty = "chump";
					cont = false;
				} else if (event.kbd.keycode == Common::KEYCODE_p) {
					_difficulty = "punk";
					cont = false;
				} else if (event.kbd.keycode == Common::KEYCODE_b) {
					_difficulty = "bad ass";
					cont = false;
				} else if (event.kbd.keycode == Common::KEYCODE_a) {
					cont = false;
				}
				break;

			default:
				break;
			}
		}

		drawScreen();
		g_system->delayMillis(10);
	}

	if (_difficulty.empty())
		_nextLevel = "<main_menu>";
	else {
		saveProfile(_name, 0);
		if (_unlockAllLevels) {
			_nextLevel = "<select_t1>";
			unlockAllLevels();
			_flashbackMode = true;
		} else
			_nextLevel = code->levelIfWin;
	}

	menu->free();
	delete menu;
}

void BoyzEngine::runRetryMenu(Code *code) {
	incLivesUsed();
	uint32 idx = _rnd->getRandomNumber(_deathVideo.size() - 1);
	Filename filename = _deathVideo[idx];
	MVideo video(filename, Common::Point(0, 0), false, true, false);
	disableCursor();
	runIntro(video);
	changeCursor("crosshair");

	Common::Rect retryMissionBox(73, 62, 245, 77);
	Common::Rect restartTerritoryBox(73, 81, 245, 96);
	Common::Rect restartMissionBox(73, 100, 245, 114);
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
					_stats = _globalStats;
					_nextLevel = firstLevelTerritory(_checkpoint);
					cont = false;
				} else if (restartMissionBox.contains(mousePos)) {
					_nextLevel = "<main_menu>";
					cont = false;
				} else if (quitBox.contains(mousePos))
					quitGame();
				break;

			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_s) {
					_nextLevel = _checkpoint;
					cont = false;
				} else if (event.kbd.keycode == Common::KEYCODE_n) {
					_nextLevel = "<main_menu>";
					cont = false;
				} else if (event.kbd.keycode == Common::KEYCODE_t) {
					// Restore initial health for the team
					_health = _maxHealth;
					_stats = _globalStats;
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

void BoyzEngine::runAlarmC5(Code *code) {
	MVideo video1("misc/alrm_c5s.smk", Common::Point(0, 0), false, true, false);
	disableCursor();
	runIntro(video1);

	MVideo video2("preload/deathn4s.smk", Common::Point(0, 0), false, true, false);
	disableCursor();
	runIntro(video2);

	_nextLevel = "<check_c5>";
}


void BoyzEngine::runCheckC5(Code *code) {
	if (_sceneState["GS_C5MAP"]) {
		if (!_sceneState["GS_MINEMAP_VIEWED"]) {
			MVideo video("c5/c5_maps.smk", Common::Point(0, 0), false, true, false);
			disableCursor();
			runIntro(video);
			defaultCursor();
			waitForUserClick(1);
			_sceneState["GS_MINEMAP_VIEWED"] = true;
		}
	}

	Common::String nextLevel;
	if (_sceneState["GS_SEQ_51"] &&
		_sceneState["GS_SEQ_52"] &&\
		_sceneState["GS_SEQ_53"]) {
		MVideo video("c5/c5intrbs.smk", Common::Point(0, 0), false, true, false);
		disableCursor();
		runIntro(video);
		nextLevel = "c54.mi_";
	}

	if (nextLevel.empty())
		nextLevel = "<select_c5>";

	_nextLevel = nextLevel;
	saveProfile(_name, 531);
}


void BoyzEngine::runCheckC3(Code *code) {
	Common::String nextLevel;
	if (_sceneState["GS_SEQ_31"] && _sceneState["GS_SEQ_32"] &&\
		_sceneState["GS_SEQ_33"] && _sceneState["GS_SEQ_34"] &&\
		_sceneState["GS_HOTELDONE"]) {
		nextLevel = "c36.mi_";
	}

	if (nextLevel.empty())
		nextLevel = "<select_c3>";

	_nextLevel = nextLevel;
	saveProfile(_name, 3591);
}

void BoyzEngine::runCheckHo(Code *code) {
	Common::String nextLevel;
	if (_sceneState["GS_SEQ_351"] && _sceneState["GS_SEQ_352"] &&\
		_sceneState["GS_SEQ_353"] && _sceneState["GS_SEQ_354"] &&\
		_sceneState["GS_SEQ_355"]) {
		_sceneState["GS_HOTELDONE"] = 1;
		nextLevel = "<check_c3>";
	}

	if (nextLevel.empty())
		nextLevel = "<select_ho>";

	_nextLevel = nextLevel;
	saveProfile(_name, 3592);
}

void BoyzEngine::endCredits(Code *code) {
	_flashbackMode = true;
	saveProfile(_name, 59);
	showCredits();
	_nextLevel = "<select_t1>";
}

void BoyzEngine::showCredits() {
	MVideo c1("intro/sbcred1.smk", Common::Point(0, 0), false, true, false);
	runIntro(c1);
	MVideo c2("intro/sbcred2.smk", Common::Point(0, 0), false, true, false);
	runIntro(c2);
}

int BoyzEngine::getTerritory(const Common::String &level) {
	if (Common::matchString(level.c_str(), "c1#.mi_"))
		return 1;
	else if (Common::matchString(level.c_str(), "c2#.mi_"))
		return 2;
	else if (Common::matchString(level.c_str(), "c3#.mi_"))
		return 3;
	else if (Common::matchString(level.c_str(), "c3##.mi_"))
		return 3;
	else if (Common::matchString(level.c_str(), "c4#.mi_"))
		return 4;
	else if (Common::matchString(level.c_str(), "c5#.mi_"))
		return 5;
	else
		error("Invalid territory for level %s", level.c_str());
}

Common::String BoyzEngine::firstLevelTerritory(const Common::String &level) {
	if (Common::matchString(level.c_str(), "c1#.mi_"))
		return "c19.mi_";
	else if (Common::matchString(level.c_str(), "c2#.mi_"))
		return "c21.mi_";
	else if (Common::matchString(level.c_str(), "c3#.mi_"))
		return "c31.mi_";
	else if (Common::matchString(level.c_str(), "c3##.mi_"))
		return "c31.mi_";
	else if (Common::matchString(level.c_str(), "c4#.mi_"))
		return "c41.mi_";
	else if (Common::matchString(level.c_str(), "c5#.mi_"))
		return "c51.mi_";
	else
		error("Invalid territory for level %s", level.c_str());
}

Common::String BoyzEngine::lastLevelTerritory(const Common::String &level) {
	if (Common::matchString(level.c_str(), "c1#.mi_"))
		return "c18.mi_";
	else if (Common::matchString(level.c_str(), "c2#.mi_"))
		return "c22.mi_";
	else if (Common::matchString(level.c_str(), "c3#.mi_"))
		return "c38.mi_";
	else if (Common::matchString(level.c_str(), "c3##.mi_"))
		return "c38.mi_";
	else if (Common::matchString(level.c_str(), "c4#.mi_"))
		return "c42.mi_";
	else if (Common::matchString(level.c_str(), "c5#.mi_"))
		return "c59.mi_";
	else
		error("Invalid territory for level %s", level.c_str());
}

} // End of namespace Hypno