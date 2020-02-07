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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/error.h"
#include "common/system.h"
#include "common/events.h"
#include "common/fs.h"
#include "common/savefile.h"
#include "graphics/screen.h"
#include "graphics/cursorman.h"

#include "engines/util.h"

#include "mutationofjb/mutationofjb.h"
#include "mutationofjb/game.h"
#include "mutationofjb/gamedata.h"
#include "mutationofjb/gamescreen.h"
#include "mutationofjb/debug.h"
#include "mutationofjb/room.h"

namespace MutationOfJB {

MutationOfJBEngine::MutationOfJBEngine(OSystem *syst, const ADGameDescription *gameDesc)
	: Engine(syst),
	  _gameDesc(gameDesc),
	  _screen(nullptr),
	  _game(nullptr),
	  _mapObjectId(0),
	  _cursorState(CURSOR_IDLE),
	  _currentScreen(nullptr) {

	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "data");
}

MutationOfJBEngine::~MutationOfJBEngine() {}

void MutationOfJBEngine::setupCursor() {
	const uint8 cursor[] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	};

	updateCursorPalette();

	CursorMan.disableCursorPalette(true);
	CursorMan.pushCursor(cursor, 15, 15, 7, 7, 0);
	CursorMan.showMouse(true);
}

void MutationOfJBEngine::updateCursorPalette() {
	if (_cursorState == CURSOR_OFF) {
		return;
	}

	const uint8 white[] = {0xFF, 0xFF, 0xFF};
	const uint8 blue[] = {0x00, 0xFF, 0xC3};

	_screen->setPalette(_cursorState == CURSOR_ACTIVE ? blue : white, 0xFF, 1);
}

Graphics::Screen *MutationOfJBEngine::getScreen() const {
	return _screen;
}

Game &MutationOfJBEngine::getGame() {
	return *_game;
}

void MutationOfJBEngine::setCursorState(CursorState cursorState) {
	if (_cursorState == cursorState) {
		return;
	}

	_cursorState = cursorState;
	updateCursorPalette();
}

bool MutationOfJBEngine::hasFeature(Engine::EngineFeature f) const {
	if (f == kSupportsLoadingDuringRuntime || f == kSupportsSavingDuringRuntime) {
		return true;
	}

	return false;
}

bool MutationOfJBEngine::canLoadGameStateCurrently() {
	return _game->loadSaveAllowed();
}

Common::Error MutationOfJBEngine::loadGameState(int slot) {
	const Common::String saveName = getSaveStateName(slot);
	Common::InSaveFile *const saveFile = g_system->getSavefileManager()->openForLoading(saveName);
	if (!saveFile)
		return Common::kReadingFailed;

	Common::Serializer sz(saveFile, nullptr);

	SaveHeader saveHdr;
	saveHdr.sync(sz);
	_game->getGameData().saveLoadWithSerializer(sz);
	delete saveFile;

	_game->changeScene(_game->getGameData()._currentScene, _game->getGameData()._partB);
	_game->getGameScreen().markDirty();

	return Common::kNoError;
}

bool MutationOfJBEngine::canSaveGameStateCurrently() {
	return _game->loadSaveAllowed();
}

Common::Error MutationOfJBEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	Common::OutSaveFile *const saveFile = g_system->getSavefileManager()->openForSaving(
		getSaveStateName(slot));
	if (!saveFile)
		return Common::kWritingFailed;

	Common::Serializer sz(nullptr, saveFile);

	SaveHeader saveHdr;
	saveHdr._description = desc;
	saveHdr.sync(sz);
	_game->getGameData().saveLoadWithSerializer(sz);
	saveFile->finalize();
	delete saveFile;

	return Common::kNoError;
}

const ADGameDescription *MutationOfJBEngine::getGameDescription() const {
	return _gameDesc;
}

Common::Error MutationOfJBEngine::run() {
	initGraphics(320, 200);

	setDebugger(new Console(this));
	_screen = new Graphics::Screen();
	_game = new Game(this);
	_currentScreen = &_game->getGameScreen();

	setupCursor();

	if (ConfMan.hasKey("save_slot")) {
		const Common::Error err = loadGameState(ConfMan.getInt("save_slot"));
		if (err.getCode() != Common::kNoError)
			return err;
	} else {
		_game->changeScene(13, false); // Initial scene.
	}

	while (!shouldQuit()) {
		Common::Event event;
		while (_eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN: {
				if (event.kbd.keycode == Common::KEYCODE_F5 && event.kbd.hasFlags(0)) {
					openMainMenuDialog();
				}
				break;
			}
			default:
				break;
			}

			if (_currentScreen)
				_currentScreen->handleEvent(event);
		}

		_game->update();
		if (_currentScreen)
			_currentScreen->update();

		_system->delayMillis(10);
		_screen->update();
	}

	return Common::kNoError;
}

bool SaveHeader::sync(Common::Serializer &sz) {
	const uint32 SAVE_MAGIC_NUMBER = MKTAG('M', 'O', 'J', 'B');
	const uint32 SAVE_FILE_VERSION = 1;

	if (sz.isLoading()) {
		uint32 magic = 0;
		sz.syncAsUint32BE(magic);
		if (magic != SAVE_MAGIC_NUMBER) {
			warning("Invalid save");
			return false;
		}
	} else {
		uint32 magic = SAVE_MAGIC_NUMBER;
		sz.syncAsUint32BE(magic);
	}

	sz.syncVersion(SAVE_FILE_VERSION);
	sz.syncString(_description);

	return true;
}

}
