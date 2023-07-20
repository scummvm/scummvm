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

#include "common/events.h"
#include "crab/app.h"

namespace Crab {

bool App::init() {

#if 0
	// Load all SDL subsystems and the TrueType font subsystem
	if (SDL_Init(SDL_INIT_EVERYTHING) == -1) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Unable to initialize SDL", "Please install libsdl2", NULL);
		return false;
	}

	if (TTF_Init() == -1) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Unable to initialize sdl_ttf", "Please install libsdl2_ttf", NULL);
		return false;
	}

	int flags = IMG_Init(IMG_INIT_PNG);
	if (flags != IMG_INIT_PNG) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Unable to initialize PNG support", "Please install libsdl2_image", NULL);
		return false;
	}
#endif

	// Load paths for important files
	g_engine->_filePath->load("res/paths.xml");

	// Initialize Steam
	// SteamAPI_Init();
#if 0
	// Load the settings corresponding to the latest version
	{
		const Common::String DEFAULT_FILENAME = "res/settings.xml";

		Common::String filename = g_engine->_filePath->appdata;
		filename += "settings.xml";

		using namespace boost::filesystem;
		if (!is_regular_file(filename)) {
			// The other file does not exist, just use the default file
			LoadSettings(DEFAULT_FILENAME);
		} else {
			// We are using the other file, check if it is up to date or not
			if (Version(DEFAULT_FILENAME) > Version(filename)) {
				// The game has been updated to a different control scheme, use the default file
				LoadSettings(DEFAULT_FILENAME);
			} else {
				// The version set by the player is fine, just use that
				LoadSettings(filename);
			}
		}
	}

	SDL_DisplayMode d;
	if (SDL_GetDesktopDisplayMode(0, &d) == 0) {
		// Store the default desktop values before starting our own screen
		g_engine->_screenSettings->desktop.w = d.w;
		g_engine->_screenSettings->desktop.h = d.h;
	}

	// Set up the screen - use windowed mode at start in order to prevent loss of texture on context switching
	gWindow = SDL_CreateWindow("Unrest", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
							   g_engine->_screenSettings->cur.w, g_engine->_screenSettings->cur.h, SDL_WINDOW_SHOWN);

	if (gWindow == nullptr)
		return false;

	// Set the window icon
	SDL_Surface *icon = IMG_load(g_engine->_filePath->icon.c_str());
	SDL_SetWindowIcon(gWindow, icon);
	SDL_FreeSurface(icon);

	// Create the renderer
	gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_PRESENTVSYNC);

	if (gRenderer == nullptr) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Unable to create renderer", ":(", NULL);
		return false;
	}

	// Disable the SDL stock cursor and screen-saver
	SDL_ShowCursor(SDL_DISABLE);
	SDL_DisableScreenSaver();

	// Initial check for controllers on the system
	g_engine->_inputManager->AddController();
#endif
	// Initialize and load input

	loadSettings("res/settings.xml");
	g_engine->_screenSettings->_inGame = false;
	return true;
}

void App::run() {
	init();
	// State IDs
	GameStateID currentStateId = GAMESTATE_NULL, nextStateId = GAMESTATE_TITLE;
	bool shouldChangeState = true;

	// Set the current game state object
	GameState *currentState = NULL;
	Timer fps;
	Common::Event e;
	int fpscount = 0, fpsval = 1, lasts = 0;

	_game = new Game();

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

#if 0
		while (SDL_PollEvent(&Event)) {
			// Do state Event handling
			CurrentState->handleEvents(Event, ShouldChangeState, NextStateID);

			// If the user has closed the window or pressed ALT+F4
			if (Event.type == SDL_QUIT || (Event.type == SDL_KEYDOWN && Event.key.keysym.scancode == SDL_SCANCODE_F4 && Event.key.keysym.mod & KMOD_ALT)) {
				// Save the program state to file
				CurrentState->AutoSave();

				// Quit the program
				ShouldChangeState = true;
				NextStateID = GAMESTATE_EXIT;
			} else if (Event.type == SDL_KEYDOWN && Event.key.keysym.scancode == SDL_SCANCODE_RETURN && Event.key.keysym.mod & KMOD_ALT) {
				// Toggle full-screen if user presses ALT+ENTER
				g_engine->_screenSettings->fullscreen = !g_engine->_screenSettings->fullscreen;
				g_engine->_screenSettings->SetFullscreen();
			} else if (Event.type == SDL_KEYUP && Event.key.keysym.scancode == SDL_SCANCODE_GRAVE && Event.key.keysym.mod & KMOD_CTRL)
				GameDebug = !GameDebug;
			g_engine->_inputManager->HandleController(Event);
		}
#endif
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

#if 0
		if (GameDebug) {
			if (SDL_GetTicks() - lasts > 1000) {
				lasts = SDL_GetTicks();
				fpsval = fpscount;
				fpscount = 1;
			} else
				++fpscount;

			if (CurrentStateID >= 0)
				g_engine->_textManager->draw(0, 0, NumberToString(fpsval), 0);
		}
#endif
		if (g_system->getMillis() - lasts > 1000) {
				lasts = g_system->getMillis();
				fpsval = fpscount;
				fpscount = 1;
			} else
				++fpscount;

			if (currentStateId >= 0)
				g_engine->_textManager->draw(0, 0, numberToString(fpsval).c_str(), 0);
		//const Graphics::ManagedSurface *s = g_engine->_renderSurface;
		//g_system->copyRectToScreen(s->getPixels(), s->pitch, 0, 0, s->w, s->h);
		g_engine->_screen->update();

#if 0
		// Update the screen
		SDL_RenderPresent(gRenderer);
#endif

		// Cap the frame rate
		if (fps.ticks() < 1000u / g_engine->_screenSettings->_fps) {
#if 0
			SDL_Delay((1000u / g_engine->_screenSettings->fps) - fps.Ticks());
#endif
			uint32 delay = (1000u / g_engine->_screenSettings->_fps) - fps.ticks();
			//warning("Delay by %d ms", delay);
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
#if 0
			g_engine->_musicManager->load(node);
#endif
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
#if 0
	// SteamAPI_Shutdown();

	g_engine->_imageManager->Quit();
	g_engine->_musicManager->Quit();
	g_engine->_textManager->Quit();
	g_engine->_inputManager->Quit();
	g_engine->_loadingScreen->Quit();

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);

	// Quit SDL_ttf
	TTF_Quit();

	// Quit the audio stuff
	Mix_CloseAudio();
	Mix_Quit();

	// Quit SDL_image
	IMG_Quit();

	// Quit SDL
	SDL_Quit();
#endif

}

} // End of namespace Crab
