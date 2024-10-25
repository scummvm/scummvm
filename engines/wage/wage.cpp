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
 * MIT License:
 *
 * Copyright (c) 2009 Alexei Svitkine, Eugene Sandulenko
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/events.h"
#include "common/punycode.h"
#include "common/system.h"

#include "engines/engine.h"
#include "engines/util.h"

#include "graphics/macgui/macdialog.h"

#include "wage/wage.h"
#include "wage/entities.h"
#include "wage/gui.h"
#include "wage/script.h"
#include "wage/world.h"

namespace Wage {

WageEngine::WageEngine(OSystem *syst, const ADGameDescription *desc) : Engine(syst), _gameDescription(desc) {
	_rnd = new Common::RandomSource("wage");

	_aim = -1;
	_opponentAim = -1;
	_temporarilyHidden = false;
	_isGameOver = false;
	_monster = NULL;
	_running = NULL;
	_lastScene = NULL;

	_loopCount = 0;
	_turn = 0;

	_commandWasQuick = false;

	_shouldQuit = false;

	_gui = NULL;
	_world = NULL;
	_offer = NULL;

	_resManager = NULL;

	debug("WageEngine::WageEngine()");
}

WageEngine::~WageEngine() {
	debug("WageEngine::~WageEngine()");

	delete _world;
	delete _resManager;
	delete _gui;
	delete _rnd;
}

bool WageEngine::pollEvent(Common::Event &event) {
	return _eventMan->pollEvent(event);
}

Common::Error WageEngine::run() {
	debug("WageEngine::init");

	int width = 512;
	int height = 342;

	if (getFeatures() & GF_RES800) {
		width = 800;
		height = 600;
	} else if (getFeatures() & GF_RES1024) {
		width = 1024;
		height = 768;
	}

	initGraphics(width, height);

	setDebugger(new Debugger(this));

	// Your main event loop should be (invoked from) here.
	_resManager = new Common::MacResManager();
	if (!_resManager->open(Common::Path(getGameFile()).punycodeDecode()))
		error("Could not open %s as a resource fork", getGameFile());

	_world = new World(this);

	if (!_world->loadWorld(_resManager))
		return Common::kNoGameDataFoundError;

	_shouldQuit = false;

	_gui = new Gui(this);

	_temporarilyHidden = true;
	performInitialSetup();
	if (ConfMan.hasKey("save_slot")) {
		int saveSlot = ConfMan.getInt("save_slot");
		loadGame(saveSlot);
		_gui->regenCommandsMenu();
		_gui->regenWeaponsMenu();
	}

	_gui->_consoleWindow->setTextWindowFont(_world->_player->_currentScene->getFont());

	Common::String input("look");
	processTurn(&input, NULL);
	_temporarilyHidden = false;

	while (!_shouldQuit) {
		processEvents();

		if (_restartRequested)
			restart();

		if (_gui)
			_gui->draw();

		g_system->updateScreen();
		g_system->delayMillis(50);

		if (!_soundToPlay.empty()) {
			playSound(_soundToPlay);
			_soundToPlay.clear();
		}
	}

	return Common::kNoError;
}

void WageEngine::restart() {
	_restartRequested = false;
	delete _gui;
	delete _world;

	_gui = nullptr;

	_world = new World(this);

	if (!_world->loadWorld(_resManager)) {
		_shouldQuit = true;
		return;
	}

	_shouldQuit = false;

	_gui = new Gui(this);

	_temporarilyHidden = true;
	performInitialSetup();

	Common::String input("look");
	processTurn(&input, NULL);
}

void WageEngine::processEvents() {
	Common::Event event;

	while (_eventMan->pollEvent(event)) {
		if (_gui->processEvent(event))
			continue;

		switch (event.type) {
		case Common::EVENT_QUIT:
			if (saveDialog())
				_shouldQuit = true;
			break;
		case Common::EVENT_KEYDOWN:
			switch (event.kbd.keycode) {
			case Common::KEYCODE_RETURN: {
					_inputText = Common::convertFromU32String(_gui->_consoleWindow->getInput());
					Common::String inp = _inputText + '\n';

					_gui->appendText(inp.c_str());

					_gui->_consoleWindow->clearInput();

					if (_inputText.empty())
						break;

					processTurn(&_inputText, NULL);
					_gui->disableUndo();
					_gui->enableRevert();
					break;
				}
			default:
				break;
			}
			break;

		default:
			break;
		}
	}
}

void WageEngine::setMenu(Common::String menu) {
	_world->_commandsMenu = menu;

	_gui->regenCommandsMenu();
}

void WageEngine::appendText(const char *str) {
	Common::String s(str);

	s += '\n';

	_gui->appendText(s.c_str());

	_inputText.clear();
}

void WageEngine::gameOver() {
	Graphics::MacDialogButtonArray buttons;

	buttons.push_back(new Graphics::MacDialogButton("OK", 66, 67, 68, 28));

	Graphics::MacFont font;

	Graphics::MacText gameOverMessage(*_world->_gameOverMessage, _gui->_wm, &font, Graphics::kColorBlack,
									  Graphics::kColorWhite, 199, Graphics::kTextAlignCenter);

	Graphics::MacDialog gameOverDialog(&_gui->_screen, _gui->_wm,  199, &gameOverMessage, 199, &buttons, 0);

	int button = gameOverDialog.run();

	if (button == Graphics::kMacDialogQuitRequested)
		_shouldQuit = true;

	doClose();

	_gui->disableAllMenus();
	_gui->enableNewGameMenus();
}

bool WageEngine::saveDialog() {
	Graphics::MacDialogButtonArray buttons;

	buttons.push_back(new Graphics::MacDialogButton("No", 19, 67, 68, 28));
	buttons.push_back(new Graphics::MacDialogButton("Yes", 112, 67, 68, 28));
	buttons.push_back(new Graphics::MacDialogButton("Cancel", 205, 67, 68, 28));

	Graphics::MacFont font;

	Graphics::MacText saveBeforeCloseMessage(*_world->_saveBeforeCloseMessage, _gui->_wm, &font, Graphics::kColorBlack,
									  Graphics::kColorWhite, 291, Graphics::kTextAlignCenter);

	Graphics::MacDialog save(&_gui->_screen, _gui->_wm, 291, &saveBeforeCloseMessage, 291, &buttons, 1);

	int button = save.run();

	if (button == Graphics::kMacDialogQuitRequested)
		_shouldQuit = true;
	else if (button == 2) // Cancel
		return false;
	else if (button == 1)
		saveGame();

	doClose();

	return true;
}

void WageEngine::aboutDialog() {
	Graphics::MacDialogButtonArray buttons;

	buttons.push_back(new Graphics::MacDialogButton("OK", 191, 167, 68, 28));

	Graphics::MacText aboutMessage(_world->_aboutMessage, _gui->_wm, _gui->_consoleWindow->getTextWindowFont(), Graphics::kColorBlack,
											 Graphics::kColorWhite, 400, Graphics::kTextAlignCenter);

	Common::U32String disclaimer("\n\n\n\nThis adventure was produced with World Builder\xAA\nthe adventure game creation system.\n© Copyright 1986 by William C. Appleton, All Right Reserved\nPublished by Silicon Beach Software, Inc.");

	aboutMessage.appendText(disclaimer, 3, 9, 0, false);

	Graphics::MacDialog about(&_gui->_screen, _gui->_wm, 450, &aboutMessage, 400, &buttons, 0);

	int button = about.run();

	if (button == Graphics::kMacDialogQuitRequested)
		_shouldQuit = true;
}

void WageEngine::saveGame() {
	if (_defaultSaveSlot != -1 && _defaultSaveSlot != getAutosaveSlot())
		saveGameState(_defaultSaveSlot, _defaultSaveDescritpion, false);
	else
		scummVMSaveLoadDialog(true);
}

void WageEngine::performInitialSetup() {
	debug(5, "Resetting Objs: %d", _world->_orderedObjs.size());
	for (uint i = 0; i < _world->_orderedObjs.size() - 1; i++)
		_world->move(_world->_orderedObjs[i], _world->_storageScene, true);

	_world->move(_world->_orderedObjs[_world->_orderedObjs.size() - 1], _world->_storageScene);

	debug(5, "Resetting Chrs: %d", _world->_orderedChrs.size());
	for (uint i = 0; i < _world->_orderedChrs.size() - 1; i++)
		_world->move(_world->_orderedChrs[i], _world->_storageScene, true);

	_world->move(_world->_orderedChrs[_world->_orderedChrs.size() - 1], _world->_storageScene);

	debug(5, "Resetting Owners: %d", _world->_orderedObjs.size());
	for (uint i = 0; i < _world->_orderedObjs.size(); i++) {
		Obj *obj = _world->_orderedObjs[i];
		if (!isStorageScene(obj->_sceneOrOwner)) {
			Common::String location = obj->_sceneOrOwner;
			location.toLowercase();
			Scene *scene = getSceneByName(location);
			if (scene != NULL) {
				_world->move(obj, scene);
			} else {
				if (!_world->_chrs.contains(location)) {
					// Note: PLAYER@ is not a valid target here.
					warning("Couldn't move %s to \"%s\"", obj->_name.c_str(), obj->_sceneOrOwner.c_str());
				} else {
					// TODO: Add check for max items.
					_world->move(obj, _world->_chrs[location]);
				}
			}
		}
	}

	bool playerPlaced = false;
	for (uint i = 0; i < _world->_orderedChrs.size(); i++) {
		Chr *chr = _world->_orderedChrs[i];
		if (!isStorageScene(chr->_initialScene)) {
			Common::String key = chr->_initialScene;
			key.toLowercase();
			if (_world->_scenes.contains(key) && _world->_scenes[key] != NULL) {
				_world->move(chr, _world->_scenes[key]);

				if (chr->_playerCharacter)
					debug(0, "Initial scene: %s", key.c_str());
			} else {
				_world->move(chr, _world->getRandomScene());
			}
			if (chr->_playerCharacter) {
				playerPlaced = true;
			}
		}
		chr->wearObjs();
	}
	if (!playerPlaced) {
		_world->move(_world->_player, _world->getRandomScene());
	}

	// Set the console window's dimensions early here because
	// flowText() that needs them gets called before they're set
	_gui->_consoleWindow->setDimensions(*_world->_player->_currentScene->_textBounds);
}

void WageEngine::wearObjs(Chr* chr) {
	if (chr != nullptr)
		chr->wearObjs();
}

void WageEngine::doClose() {
	// No op on ScummVM since we do not allow to load arbitrary games
}

Scene *WageEngine::getSceneByName(Common::String &location) {
	if (location.equals("random@")) {
		return _world->getRandomScene();
	} else {
		if (_world->_scenes.contains(location))
			return _world->_scenes[location];
		else
			return NULL;
	}
}

void WageEngine::onMove(Designed *what, Designed *from, Designed *to) {
	Chr *player = _world->_player;
	Scene *currentScene = player->_currentScene;
	if (currentScene == _world->_storageScene && !_temporarilyHidden) {
		if (!_isGameOver) {
			_isGameOver = true;
			gameOver();
		}
		return;
	}

	if (from == currentScene || to == currentScene ||
			(what->_classType == CHR && ((Chr *)what)->_currentScene == currentScene) ||
			(what->_classType == OBJ && ((Obj *)what)->_currentScene == currentScene))
		_gui->setSceneDirty();

	if ((from == player || to == player) && !_temporarilyHidden)
		_gui->regenWeaponsMenu();

	if (what != player && what->_classType == CHR) {
		Chr *chr = (Chr *)what;
		if (to == _world->_storageScene) {
			int returnTo = chr->_returnTo;
			if (returnTo != Chr::RETURN_TO_STORAGE) {
				Common::String returnToSceneName;
				if (returnTo == Chr::RETURN_TO_INITIAL_SCENE) {
					returnToSceneName = chr->_initialScene;
					returnToSceneName.toLowercase();
				} else {
					returnToSceneName = "random@";
				}
				Scene *scene = getSceneByName(returnToSceneName);
				if (scene != NULL && scene != _world->_storageScene) {
					_world->move(chr, scene);
					// To avoid sleeping twice, return if the above move command would cause a sleep.
					if (scene == currentScene)
						return;
				}
			}
		} else if (to == player->_currentScene) {
			if (getMonster() == NULL) {
				_monster = chr;
				encounter(player, chr);
			}
		}
	}
	if (!_temporarilyHidden) {
		if (to == currentScene || from == currentScene) {
			redrawScene();
			g_system->updateScreen();
			g_system->delayMillis(100);
		}
	}
}

void WageEngine::redrawScene() {
	Scene *currentScene = _world->_player->_currentScene;

	if (currentScene != NULL) {
		bool firstTime = (_lastScene != currentScene);

		_gui->draw();
		updateSoundTimerForScene(currentScene, firstTime);
	}
}

void WageEngine::processTurnInternal(Common::String *textInput, Designed *clickInput) {
	Scene *playerScene = _world->_player->_currentScene;
	if (playerScene == _world->_storageScene)
		return;

	bool shouldEncounter = false;

	if (playerScene != _lastScene) {
		_loopCount = 0;
		_lastScene = playerScene;
		_monster = NULL;
		_running = NULL;
		_offer = NULL;

		for (ChrList::const_iterator it = playerScene->_chrs.begin(); it != playerScene->_chrs.end(); ++it) {
			if (!(*it)->_playerCharacter) {
				_monster = *it;
				shouldEncounter = true;
				break;
			}
		}
	}

	bool monsterWasNull = (_monster == NULL);
	Script *script = playerScene->_script != NULL ? playerScene->_script : _world->_globalScript;
	bool handled = script->execute(_world, _loopCount++, textInput, clickInput);

	playerScene = _world->_player->_currentScene;

	if (playerScene == _world->_storageScene)
		return;

	if (playerScene != _lastScene) {
		_temporarilyHidden = true;
		_gui->clearOutput();
		_gui->_consoleWindow->setTextWindowFont(_world->_player->_currentScene->getFont());
		regen();
		Common::String input("look");
		processTurnInternal(&input, NULL);

		if (_shouldQuit)
			return;

		redrawScene();
		_temporarilyHidden = false;
	} else if (_loopCount == 1) {
		redrawScene();
		if (shouldEncounter && getMonster() != NULL) {
			encounter(_world->_player, _monster);
		}
	} else if (textInput != NULL && !handled) {
		if (monsterWasNull && getMonster() != NULL)
			return;

		const char *rant = _rnd->getRandomNumber(1) ? "What?" : "Huh?";

		appendText(rant);
		_commandWasQuick = true;
	}
}

void WageEngine::processTurn(Common::String *textInput, Designed *clickInput) {
	_commandWasQuick = false;
	Scene *prevScene = _world->_player->_currentScene;
	Chr *prevMonster = getMonster();
	Common::String input;

	if (textInput)
		input = *textInput;

	input.toLowercase();

	processTurnInternal(&input, clickInput);
	Scene *playerScene = _world->_player->_currentScene;

	if (prevScene != playerScene && playerScene != _world->_storageScene) {
		if (prevMonster != NULL) {
			bool followed = false;
			if (getMonster() == NULL) {
				// TODO: adjacent scenes doesn't contain up/down etc... verify that monsters can't follow these...
				if (_world->scenesAreConnected(playerScene, prevMonster->_currentScene)) {
					int chance = _rnd->getRandomNumber(255);
					followed = (chance < prevMonster->_followsOpponent);
				}
			}

			char buf[512];

			if (followed) {
				snprintf(buf, 512, "%s%s follows you.", prevMonster->getDefiniteArticle(true), prevMonster->_name.c_str());
				appendText(buf);

				_world->move(prevMonster, playerScene);
			} else {
				snprintf(buf, 512, "You escape %s%s.", prevMonster->getDefiniteArticle(false), prevMonster->_name.c_str());
				appendText(buf);
			}
		}
	}
	if (!_commandWasQuick && getMonster() != NULL) {
		performCombatAction(getMonster(), _world->_player);
	}

	_inputText.clear();
}


} // End of namespace Wage
