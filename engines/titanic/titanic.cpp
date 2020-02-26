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

#include "titanic/titanic.h"
#include "titanic/carry/hose.h"
#include "titanic/core/saveable_object.h"
#include "titanic/debugger.h"
#include "titanic/events.h"
#include "titanic/game_manager.h"
#include "titanic/game/get_lift_eye2.h"
#include "titanic/game/television.h"
#include "titanic/game/parrot/parrot_lobby_object.h"
#include "titanic/game/sgt/sgt_navigation.h"
#include "titanic/game/sgt/sgt_state_room.h"
#include "titanic/main_game_window.h"
#include "titanic/moves/enter_exit_first_class_state.h"
#include "titanic/moves/enter_exit_sec_class_mini_lift.h"
#include "titanic/moves/exit_pellerator.h"
#include "titanic/pet_control/pet_control.h"
#include "titanic/sound/music_room.h"
#include "titanic/sound/music_room_instrument.h"
#include "titanic/support/files_manager.h"
#include "titanic/support/simple_file.h"
#include "titanic/true_talk/tt_npc_script.h"
#include "common/archive.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/scummsys.h"
#include "engines/util.h"
#include "graphics/scaler.h"
#include "graphics/screen.h"
#include "gui/saveload.h"

namespace Titanic {

TitanicEngine *g_vm;
Common::Language g_language;

TitanicEngine::TitanicEngine(OSystem *syst, const TitanicGameDescription *gameDesc)
		: _gameDescription(gameDesc), Engine(syst), _randomSource("Titanic") {
	g_vm = this;
	g_language = getLanguage();
	_events = nullptr;
	_filesManager = nullptr;
	_window = nullptr;
	_screen = nullptr;
	_screenManager = nullptr;
	_scriptHandler = nullptr;
	_script = nullptr;
	CMusicRoom::_musicHandler = nullptr;
	_loadSaveSlot = -1;

	// Set up debug channels
	DebugMan.addDebugChannel(kDebugCore, "core", "Core engine debug level");
	DebugMan.addDebugChannel(kDebugScripts, "scripts", "Game scripts");
	DebugMan.addDebugChannel(kDebugGraphics, "graphics", "Graphics handling");
	DebugMan.addDebugChannel(kDebugStarfield, "starfield", "Starfield logic");
}

TitanicEngine::~TitanicEngine() {
}

void TitanicEngine::initializePath(const Common::FSNode &gamePath) {
	Engine::initializePath(gamePath);
	SearchMan.addSubDirectoryMatching(gamePath, "assets");
}

bool TitanicEngine::initialize() {
	_filesManager = new CFilesManager(this);
	if (!_filesManager->loadResourceIndex()) {
		delete _filesManager;
		return false;
	}

	setDebugger(new Debugger(this));

	CSaveableObject::initClassList();
	CEnterExitFirstClassState::init();
	CGameObject::init();
	CGetLiftEye2::init();
	CHose::init();
	CMovie::init();
	CMusicRoomInstrument::init();
	CParrotLobbyObject::init();
	CSGTNavigation::init();
	CSGTStateRoom::init();
	CExitPellerator::init();
	CEnterExitSecClassMiniLift::init();
	CTelevision::init();
	CVideoSurface::setup();
	TTnpcScript::init();

	_events = new Events(this);
	_screen = new Graphics::Screen(0, 0);
	_screenManager = new OSScreenManager(this);
	_window = new CMainGameWindow(this);
	_strings.load();

	setItemNames();
	setRoomNames();

	syncSoundSettings();

	_window->applicationStarting();
	return true;
}

void TitanicEngine::deinitialize() {
	delete _events;
	delete _window;
	delete _screenManager;
	delete _filesManager;
	delete _screen;

	CEnterExitFirstClassState::deinit();
	CGetLiftEye2::deinit();
	CHose::deinit();
	CSGTNavigation::deinit();
	CSGTStateRoom::deinit();
	CExitPellerator::deinit();
	CEnterExitSecClassMiniLift::deinit();
	CGameObject::deinit();
	CTelevision::deinit();
	TTnpcScript::deinit();
	CMovie::deinit();
	CSaveableObject::freeClassList();
}

Common::Error TitanicEngine::run() {
	if (initialize()) {
		// Main event loop
		while (!shouldQuit()) {
			_events->pollEventsAndWait();
		}

		deinitialize();
	}

	return Common::kNoError;
}

void TitanicEngine::setItemNames() {
	Common::SeekableReadStream *r;
	r = g_vm->_filesManager->getResource("TEXT/ITEM_NAMES");
	while (r->pos() < r->size())
		_itemNames.push_back(readStringFromStream(r));
	delete r;

	r = g_vm->_filesManager->getResource("TEXT/ITEM_DESCRIPTIONS");
	while (r->pos() < r->size())
		_itemDescriptions.push_back(readStringFromStream(r));
	delete r;

	r = g_vm->_filesManager->getResource("TEXT/ITEM_IDS");
	while (r->pos() < r->size())
		_itemIds.push_back(readStringFromStream(r));
	delete r;
}

void TitanicEngine::setRoomNames() {
	Common::SeekableReadStream *r = g_vm->_filesManager->getResource("TEXT/ROOM_NAMES");
	while (r->pos() < r->size())
		_roomNames.push_back(readStringFromStream(r));
	delete r;
}

bool TitanicEngine::canLoadGameStateCurrently() {
	CGameManager *gameManager = _window->_gameManager;
	CScreenManager *screenMan = CScreenManager::_screenManagerPtr;

	if (!gameManager)
		// Allow loading from copyright screen and continue dialogs
		return true;

	if (!_window->_inputAllowed)
		return false;
	if (screenMan && screenMan->_inputHandler->isLocked())
		return false;
	if (!gameManager->isntTransitioning())
		return false;

	CProjectItem *project = gameManager->_project;
	if (project) {
		if (gameManager->_gameState._petActive) {
			CPetControl *pet = project->getPetControl();
			if (pet && !pet->isAreaUnlocked())
				return false;
		}
	} else {
		return false;
	}

	return true;
}

bool TitanicEngine::canSaveGameStateCurrently() {
	CGameManager *gameManager = _window->_gameManager;
	if (!gameManager)
		return false;

	return gameManager->_gameState._petActive &&
		canLoadGameStateCurrently();
}

Common::Error TitanicEngine::loadGameState(int slot) {
	CGameManager *gameManager = _window->_gameManager;
	if (!gameManager)
		_loadSaveSlot = slot;
	else
		_window->_project->loadGame(slot);
	return Common::kNoError;
}

Common::Error TitanicEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	_window->_project->saveGame(slot, desc);
	return Common::kNoError;
}

CString TitanicEngine::getSavegameName(int slot) {
	// Try and open up the savegame for access
	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(
		getSaveStateName(slot));

	if (in) {
		// Read in the savegame header data
		CompressedFile file;
		file.open(in);

		TitanicSavegameHeader header;
		bool isValid = CProjectItem::readSavegameHeader(&file, header);
		file.close();

		if (isValid)
			// Set the name text
			return header._saveName;
	}

	return CString();
}

void TitanicEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	if (_window->_project) {
		CPetControl *pet = _window->_project->getPetControl();
		if (pet) {
			pet->syncSoundSettings();
		}
	}
}

} // End of namespace Titanic
