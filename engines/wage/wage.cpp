/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

#include "common/scummsys.h"

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/file.h"
#include "common/fs.h"

#include "engines/engine.h"
#include "engines/util.h"
#include "gui/EventRecorder.h"

#include "wage/wage.h"
#include "wage/design.h"
#include "wage/entities.h"
#include "wage/gui.h"
#include "wage/script.h"
#include "wage/world.h"

namespace Wage {

WageEngine::WageEngine(OSystem *syst, const ADGameDescription *desc) : Engine(syst), _gameDescription(desc) {
	_rnd = new Common::RandomSource("wage");

	_aim = -1;
	_temporarilyHidden = false;
	_isGameOver = false;
	_monster = NULL;
	_running = NULL;
	_lastScene = NULL;

	_commandWasQuick = false;

	debug("WageEngine::WageEngine()");
}

WageEngine::~WageEngine() {
	debug("WageEngine::~WageEngine()");

	DebugMan.clearAllDebugChannels();
	delete _rnd;
}

Common::Error WageEngine::run() {
	initGraphics(512, 342, true);

	// Create debugger console. It requires GFX to be initialized
	_console = new Console(this);

	debug("WageEngine::init");

	_gui = new Gui(this);

	// Your main event loop should be (invoked from) here.
	_resManager = new Common::MacResManager();
	_resManager->open(getGameFile());

	_world = new World(this);

	if (!_world->loadWorld(_resManager))
		return Common::kNoGameDataFoundError;

	_temporarilyHidden = true;
	performInitialSetup();
	Common::String input("look");
	processTurn(&input, NULL);
	_temporarilyHidden = false;

	while (true) {
		processEvents();

		_gui->draw();
		g_system->updateScreen();
		g_system->delayMillis(50);
	}

	//_world->_orderedScenes[1]->_design->paint(&screen, _world->_patterns, false);
	//_world->_objs["frank.1"]->_design->setBounds(&r);
	//_world->_objs["frank.1"]->_design->paint(&screen, _world->_patterns, false);
	//_world->_scenes["temple of the holy mackeral"]->_design->setBounds(&r);
	//_world->_scenes["temple of the holy mackeral"]->_design->paint(&screen, _world->_patterns, false);
	//_world->_scenes["tower level 3"]->_design->setBounds(&r);
	//_world->_scenes["tower level 3"]->_design->paint(&screen, _world->_patterns, false);

	return Common::kNoError;
}

void WageEngine::processEvents() {
	Common::Event event;

	while (_eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
			error("Exiting");
			break;
		case Common::EVENT_MOUSEMOVE:
			_gui->mouseMove(event.mouse.x, event.mouse.y);
			break;
		case Common::EVENT_LBUTTONDOWN:
			break;
		case Common::EVENT_LBUTTONUP:
			{
				Designed *obj = _gui->getClickTarget(event.mouse.x, event.mouse.y);
				if (obj != NULL)
					processTurn(NULL, obj);
			}
		default:
			break;
		}
	}
}

void WageEngine::playSound(String soundName) {
	warning("STUB: WageEngine::playSound(%s)", soundName.c_str());
}

void WageEngine::setMenu(String soundName) {
	warning("STUB: WageEngine::setMenu");
}

void WageEngine::appendText(String &str) {
	_gui->appendText(str);
}

Obj *WageEngine::getOffer() {
	warning("STUB: WageEngine::getOffer");

	return NULL;
}

Chr *WageEngine::getMonster() {
	if (_monster != NULL && _monster->_currentScene != _world->_player->_currentScene) {
		_monster = NULL;
	}
	return _monster;
}

void WageEngine::gameOver() {
	warning("STUB: WageEngine::gameOver()");
}

void WageEngine::performInitialSetup() {
	debug(5, "Resetting Objs: %d", _world->_orderedObjs.size());
	for (int i = 0; i < _world->_orderedObjs.size() - 1; i++)
		_world->move(_world->_orderedObjs[i], _world->_storageScene, true);

	_world->move(_world->_orderedObjs[_world->_orderedObjs.size() - 1], _world->_storageScene);

	debug(5, "Resetting Chrs: %d", _world->_orderedChrs.size());
	for (int i = 0; i < _world->_orderedChrs.size() - 1; i++)
		_world->move(_world->_orderedChrs[i], _world->_storageScene, true);

	_world->move(_world->_orderedChrs[_world->_orderedChrs.size() - 1], _world->_storageScene);

	debug(5, "Resetting Owners: %d", _world->_orderedObjs.size());
	for (int i = 0; i < _world->_orderedObjs.size(); i++) {
		Obj *obj = _world->_orderedObjs[i];
		if (!obj->_sceneOrOwner.equalsIgnoreCase(STORAGESCENE)) {
			String location = obj->_sceneOrOwner;
			location.toLowercase();
			if (_world->_scenes.contains(location)) {
				_world->move(obj, _world->_scenes[location]);
			} else {
				if (!_world->_chrs.contains(location)) {
					// Note: PLAYER@ is not a valid target here.
					warning("Couldn't move %s to %s", obj->_name.c_str(), obj->_sceneOrOwner.c_str());
				} else {
					// TODO: Add check for max items.
					_world->move(obj, _world->_chrs[location]);
				}
			}
		}
	}

	bool playerPlaced = false;
	for (int i = 0; i < _world->_orderedChrs.size(); i++) {
		Chr *chr = _world->_orderedChrs[i];
		if (!chr->_initialScene.equalsIgnoreCase(STORAGESCENE)) {
			String key = chr->_initialScene;
			key.toLowercase();
			if (_world->_scenes.contains(key)) {
				_world->move(chr, _world->_scenes[key]);

				if (chr->_playerCharacter)
					warning("Initial scene: %s", key.c_str());
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
}

Scene *WageEngine::getSceneByName(String &location) {
	Scene *scene;
	if (location.equals("random@")) {
		scene = _world->getRandomScene();
	} else {
		scene = _world->_scenes[location];
	}
	return scene;
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

	if (what != player && what->_classType == CHR) {
		Chr *chr = (Chr *)what;
		if (to == _world->_storageScene) {
			int returnTo = chr->_returnTo;
			if (returnTo != Chr::RETURN_TO_STORAGE) {
				String returnToSceneName;
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
			g_system->delayMillis(100);
		}
	}
}

void WageEngine::encounter(Chr *player, Chr *chr) {
	warning("STUB WageEngine::encounter()");
}

void WageEngine::performCombatAction(Chr *npc, Chr *player) {
	warning("STUB WageEngine::performCombatAction()");
}

void WageEngine::redrawScene() {
	Scene *currentScene = _world->_player->_currentScene;
	if (currentScene != NULL) {
		//bool firstTime = (_lastScene != currentScene);
		_lastScene = currentScene;

		warning("STUB: WageEngine::redrawScene()");

		//updateConsoleForScene(console, currentScene);
		//updateSceneViewerForScene(viewer, currentScene);
		//viewer.paintImmediately(viewer.getBounds());
		//getContentPane().validate();
		//getContentPane().repaint();
		//console.postUpdateUI();
		//soundManager.updateSoundTimerForScene(currentScene, firstTime);
	}
}

void WageEngine::regen() {
	warning("STUB WageEngine::regen()");
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

		for (Common::List<Chr *>::const_iterator it = playerScene->_chrs.begin(); it != playerScene->_chrs.end(); ++it) {
			if (!(*it)->_playerCharacter) {
				_monster = *it;
				shouldEncounter = true;
				break;
			}
		}
	}

	bool monsterWasNull = (_monster == NULL);
	bool handled = playerScene->_script->execute(_world, _loopCount++, textInput, clickInput, this);

	playerScene = _world->_player->_currentScene;

	if (playerScene == _world->_storageScene)
		return;

	if (playerScene != _lastScene) {
		_temporarilyHidden = true;
		_gui->clearOutput();
		regen();
		Common::String input("look");
		processTurnInternal(&input, NULL);
		redrawScene();
		_temporarilyHidden = false;
	} else if (_loopCount == 1) {
		redrawScene();
		if (shouldEncounter && _monster != NULL) {
			encounter(_world->_player, _monster);
		}
	} else if (textInput != NULL && !handled) {
		if (monsterWasNull && _monster != NULL)
			return;

		Common::String rant(_rnd->getRandomNumber(1) ? "What?" : "Huh?");

		appendText(rant);
		_commandWasQuick = true;
	}
}

void WageEngine::processTurn(Common::String *textInput, Designed *clickInput) {
	_commandWasQuick = false;
	Scene *prevScene = _world->_player->_currentScene;
	Chr *prevMonster = _monster;
	processTurnInternal(textInput, clickInput);
	Scene *playerScene = _world->_player->_currentScene;

	if (prevScene != playerScene && playerScene != _world->_storageScene) {
		if (prevMonster != NULL) {
			bool followed = false;
			if (_monster == NULL) {
				warning("STUB: processTurn(), monster");
				//Set<Scene> scenes = world.getAdjacentScenes(prevMonster.getState().getCurrentScene());
				// TODO: adjacent scenes doesn't contain up/down etc... verify that monsters can't follow these...
				//if (scenes.contains(playerScene)) {
				//	int chance = (int) (Math.random() * 255);
				//	followed = (chance < prevMonster.getFollowsOpponent());
				//}
			}

			Common::String msg;

			if (followed) {
				msg = prevMonster->getNameWithDefiniteArticle(true);
				msg += " follows you.";
				appendText(msg);
				_world->move(prevMonster, playerScene);
			} else {
				msg = "You escape ";
				msg += prevMonster->getNameWithDefiniteArticle(false);
				msg += ".";
				appendText(msg);
			}
		}
	}
	if (!_commandWasQuick && _monster != NULL) {
		performCombatAction(_monster, _world->_player);
	}
}


} // End of namespace Wage
