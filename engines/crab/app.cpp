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

/*
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#include "crab/app.h"
#include "common/events.h"
#include "graphics/screen.h"
#include "crab/crab.h"
#include "crab/game.h"
#include "crab/mainmenu.h"
#include "crab/splash.h"

namespace Crab {

void App::run() {
	// State IDs
	GameStateID currentStateId = GAMESTATE_NULL, nextStateId = GAMESTATE_TITLE;
	bool shouldChangeState = true;

	// Set the current game state object
	GameState *currentState = nullptr;
	Timer fps;
	Common::Event e;
	int fpscount = 0, fpsval = 1, lasts = 0;

	_game = new Game();

	g_engine->_filePath->load("res/paths.xml");

	loadSettings("res/settings.xml");
	g_engine->_screenSettings->_inGame = false;

	// While the user hasn't quit - This is the main game loop
	while (currentStateId != GAMESTATE_EXIT && !SHOULD_QUIT) {
		// Start the frame timer
		fps.start();

		// Change state if needed
		if (shouldChangeState) {
			// Delete the current state
			if (currentState != _game) {
				delete currentState;
				currentState = nullptr;
			}

			if (nextStateId == GAMESTATE_EXIT)
				break;

			// Change the state
			switch (nextStateId) {
			case GAMESTATE_TITLE:
				currentState = new Splash();
				g_engine->_screenSettings->_inGame = false;

				// Now apply all settings - except resolution because that's already set at the start
				g_engine->_screenSettings->setFullscreen();
				g_engine->_screenSettings->setWindowBorder();
				g_engine->_screenSettings->setVsync();
				g_engine->_screenSettings->setGamma();
				g_engine->_screenSettings->setMouseTrap();
				break;

			case GAMESTATE_MAIN_MENU:
				currentState = new MainMenu();
				g_engine->_screenSettings->_inGame = false;
				break;

			case GAMESTATE_NEW_GAME:
				_game->startNewGame();
				currentState = _game;
				g_engine->_screenSettings->_inGame = true;
				break;

			case GAMESTATE_LOAD_GAME:
				currentState = _game;
				g_engine->_screenSettings->_inGame = true;
				break;

			default:
				// Encountering an undefined state, exit with an error code
				return;
			}

			// Change the current state ID
			currentStateId = nextStateId;

			// NULL the next state ID
			nextStateId = GAMESTATE_NULL;

			// No need to change state until further notice
			shouldChangeState = false;
		}

		// Do state InternalEvents
		currentState->internalEvents(shouldChangeState, nextStateId);

		while (g_system->getEventManager()->pollEvent(e)) {

			switch (e.type) {
				case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
					g_engine->_inputManager->_ivState[e.customType] = true;
					break;

				case Common::EVENT_CUSTOM_ENGINE_ACTION_END:
					g_engine->_inputManager->_ivState[e.customType] = false;
					break;

				// explicitly specify the default block to turn off unhandled case warnings
				default:
					break;
			}

			// Do state Event handling
			currentState->handleEvents(e, shouldChangeState, nextStateId);
		}


		// Do we have to reposition our interface?
		if (g_engine->_screenSettings->_changeInterface) {
			currentState->setUI();
			g_engine->_screenSettings->_changeInterface = false;
		}

		// Do state Drawing
		currentState->draw();

		if (g_system->getMillis() - lasts > 1000) {
				lasts = g_system->getMillis();
				fpsval = fpscount;
				fpscount = 1;
			} else
				++fpscount;

			if (currentStateId >= 0)
				g_engine->_textManager->draw(0, 0, numberToString(fpsval).c_str(), 0);

		g_engine->_screen->update();

		// Cap the frame rate
		if (fps.ticks() < 1000u / g_engine->_screenSettings->_fps) {
			uint32 delay = (1000u / g_engine->_screenSettings->_fps) - fps.ticks();
			g_system->delayMillis(delay);
		}
	}

	if (currentState != _game)
		delete currentState;
	delete _game;
}

void App::loadSettings(const Common::String &filename) {
	XMLDoc settings(filename);
	if (settings.ready()) {
		rapidxml::xml_node<char> *node = settings.doc()->first_node("settings");
		if (nodeValid(node)) {
			// Load the version
			loadNum(g_engine->_screenSettings->_version, "version", node);

			// Load screen settings
			if (nodeValid("screen", node))
				g_engine->_screenSettings->load(node->first_node("screen"));

			// Start the sound subsystem
			g_engine->_musicManager->load(node);
		}
	}
}

App::~App() {
	g_engine->_imageManager->quit();
	g_engine->_musicManager->quit();
	g_engine->_textManager->quit();
	g_engine->_inputManager->quit();
	g_engine->_loadingScreen->quit();
	g_engine->_mouse->quit();
}

} // End of namespace Crab
