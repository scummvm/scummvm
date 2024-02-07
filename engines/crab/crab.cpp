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


#include "common/config-manager.h"
#include "engines/util.h"
#include "graphics/cursorman.h"
#include "graphics/managed_surface.h"
#include "graphics/screen.h"
#include "crab/crab.h"
#include "crab/app.h"
#include "crab/console.h"
#include "crab/game.h"
#include "crab/GameParam.h"
#include "crab/LoadingScreen.h"
#include "crab/input/cursor.h"

namespace Crab {

CrabEngine *g_engine;

CrabEngine::CrabEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
																		   _gameDescription(gameDesc), _randomSource("Crab") {
	_app = nullptr;
	_thumbnail = nullptr;
	_debugDraw = 0;

	g_engine = this;
}

CrabEngine::~CrabEngine() {
	delete _app;

	delete _screenSettings;
	delete _stringPool;
	delete _tempData;
	delete _filePath;

	delete _loadMenu;
	delete _optionMenu;

	delete _helpScreen;
	delete _loadingScreen;

	delete _eventStore;
	delete _mouse;

	delete _musicManager;
	delete _inputManager;
	delete _textManager;
	delete _imageManager;

	delete _thumbnail;
	delete _screen;
	delete _format;
}

uint32 CrabEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String CrabEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Error CrabEngine::run() {
	_format = new Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24);
	initGraphics(1280, 720, _format);
	_screen = new Graphics::Screen(1280, 720, *_format);
	_thumbnail = new Graphics::ManagedSurface(1280, 720, *_format);

	_imageManager = new pyrodactyl::image::ImageManager();
	_textManager = new pyrodactyl::text::TextManager();
	_inputManager = new pyrodactyl::input::InputManager();
	_musicManager = new pyrodactyl::music::MusicManager();

	_mouse = new pyrodactyl::input::Cursor();
	_eventStore = new pyrodactyl::event::GameEventStore();

	_loadingScreen = new LoadingScreen();
	_helpScreen = new pyrodactyl::ui::SlideShow();

	_optionMenu = new pyrodactyl::ui::OptionMenu();
	_loadMenu = new pyrodactyl::ui::FileMenu<pyrodactyl::ui::SaveFileData>();

	_filePath = new FilePaths();
	_tempData = new TempValue();
	_stringPool = new StringPool();
	_screenSettings = new ScreenSettings();

	_debugDraw = 0;

	CursorMan.showMouse(true);
	_mouse->reset();

	_inputManager->populateKeyTable();
	_inputManager->setKeyBindingMode(pyrodactyl::input::KBM_GAME);

	// Set the engine's debugger console
	setDebugger(new Console());

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);

	_app = new App();

	_app->run();
	return Common::kNoError;
}

void CrabEngine::initializePath(const Common::FSNode &gamePath) {
	SearchMan.addDirectory(gamePath, 0, 5);
}

Common::Error CrabEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	Common::OutSaveFile *saveFile = _saveFileMan->openForSaving(getSaveStateName(slot));

	if (!saveFile)
		return Common::kWritingFailed;

	_app->getGame()->saveState(saveFile);
	getMetaEngine()->appendExtendedSave(saveFile, getTotalPlayTime(), desc, isAutosave);

	saveFile->finalize();

	delete saveFile;
	return Common::kNoError;
}

Common::Error CrabEngine::loadGameState(int slot) {
	Common::Error res = Common::kReadingFailed;
	saveAutosaveIfEnabled();

	Common::InSaveFile *saveFile = _saveFileMan->openForLoading(getSaveStateName(slot));

	if (!saveFile)
		return res;

	if (_app->getGame()->loadState(saveFile)) {
		ExtendedSavegameHeader header;
		if (MetaEngine::readSavegameHeader(saveFile, &header))
			setTotalPlayTime(header.playtime);
		res = Common::kNoError;
	}

	delete saveFile;
	return res;
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

bool CrabEngine::canSaveGameStateCurrently(Common::U32String *msg) {
	return _screenSettings->_inGame;
}

void CrabEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	_musicManager->syncSettings();
}

} // End of namespace Crab
