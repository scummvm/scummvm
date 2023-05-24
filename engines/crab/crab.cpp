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

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "crab/crab.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "crab/console.h"
#include "crab/detection.h"
#include "crab/ScreenSettings.h"
#include "crab/app.h"
#include "engines/util.h"
#include "graphics/palette.h"

namespace Crab {

CrabEngine *g_engine;

CrabEngine::CrabEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
																		   _gameDescription(gameDesc), _randomSource("Crab") {
	g_engine = this;
}

CrabEngine::~CrabEngine() {
	delete _screen;
}

uint32 CrabEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String CrabEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Error CrabEngine::run() {
	// Initialize 320x200 paletted graphics mode
	_format = new Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24);
	initGraphics(1280, 720, _format);
	_renderSurface = new Graphics::ManagedSurface(1920, 1080, *_format);
	_screen = new Graphics::Screen(1280, 720, *_format);

	_imageManager = new pyrodactyl::image::ImageManager();
	_mouse = new pyrodactyl::input::Cursor();
	_eventStore = new pyrodactyl::event::GameEventStore();
	_textManager = new pyrodactyl::text::TextManager();
	_loadingScreen = new LoadingScreen();
	_helpScreen = new pyrodactyl::ui::SlideShow();
	_optionMenu = new pyrodactyl::ui::OptionMenu();
	_loadMenu = new pyrodactyl::ui::FileMenu<pyrodactyl::ui::SaveFileData>();

	Graphics::Cursor *cursor = Graphics::makeDefaultWinCursor();
	CursorMan.replaceCursor(cursor);
	delete cursor;

	CursorMan.showMouse(true);

	// Set the engine's debugger console
	setDebugger(new Console());

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);

	_app = new App();

	_app->Run();
	return Common::kNoError;
}

void CrabEngine::initializePath(const Common::FSNode &gamePath) {
	Engine::initializePath(gamePath);
	SearchMan.addDirectory("res", gamePath, 0, 5);
}

Common::Error CrabEngine::syncGame(Common::Serializer &s) {
	// The Serializer has methods isLoading() and isSaving()
	// if you need to specific steps; for example setting
	// an array size after reading it's length, whereas
	// for saving it would write the existing array's length
	int dummy = 0;
	s.syncAsUint32LE(dummy);

	return Common::kNoError;
}

} // End of namespace Crab
