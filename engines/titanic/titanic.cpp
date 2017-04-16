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
#include "common/archive.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "engines/util.h"
#include "graphics/scaler.h"
#include "graphics/thumbnail.h"
#include "titanic/titanic.h"
#include "titanic/debugger.h"
#include "titanic/carry/hose.h"
#include "titanic/core/saveable_object.h"
#include "titanic/game/get_lift_eye2.h"
#include "titanic/game/television.h"
#include "titanic/game/parrot/parrot_lobby_object.h"
#include "titanic/game/sgt/sgt_navigation.h"
#include "titanic/game/sgt/sgt_state_room.h"
#include "titanic/moves/enter_exit_first_class_state.h"
#include "titanic/moves/enter_exit_sec_class_mini_lift.h"
#include "titanic/moves/exit_pellerator.h"
#include "titanic/pet_control/pet_control.h"
#include "titanic/sound/music_room_instrument.h"
#include "titanic/support/simple_file.h"
#include "titanic/true_talk/tt_npc_script.h"

namespace Titanic {

TitanicEngine *g_vm;

TitanicEngine::TitanicEngine(OSystem *syst, const TitanicGameDescription *gameDesc)
		: _gameDescription(gameDesc), Engine(syst), _randomSource("Titanic") {
	g_vm = this;
	_debugger = nullptr;
	_events = nullptr;
	_filesManager = nullptr;
	_window = nullptr;
	_screen = nullptr;
	_screenManager = nullptr;
	_scriptHandler = nullptr;
	_script = nullptr;
	CMusicRoom::_musicHandler = nullptr;

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

void TitanicEngine::initialize() {
	_debugger = new Debugger(this);
	_filesManager = new CFilesManager(this);

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

	_window->applicationStarting();
}

void TitanicEngine::deinitialize() {
	delete _debugger;
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
	initialize();

	// Main event loop
	while (!shouldQuit()) {
		_events->pollEventsAndWait();
	}

	deinitialize();
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
	if (!_window->_inputAllowed)
		return false;
	CProjectItem *project = _window->_gameManager->_project;
	if (project) {
		CPetControl *pet = project->getPetControl();
		if (pet && !pet->isAreaUnlocked())
			return false;
	}

	return true;
}

bool TitanicEngine::canSaveGameStateCurrently() {
	return canLoadGameStateCurrently();
}

Common::Error TitanicEngine::loadGameState(int slot) {
	_window->_project->loadGame(slot);
	return Common::kNoError;
}

Common::Error TitanicEngine::saveGameState(int slot, const Common::String &desc) {
	_window->_project->saveGame(slot, desc);
	return Common::kNoError;
}

CString TitanicEngine::generateSaveName(int slot) {
	return CString::format("%s.%03d", _targetName.c_str(), slot);
}

CString TitanicEngine::getSavegameName(int slot) {
	// Try and open up the savegame for access
	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(
		generateSaveName(slot));

	if (in) {
		// Read in the savegame header data
		CompressedFile file;
		file.open(in);

		TitanicSavegameHeader header;
		bool isValid = CProjectItem::readSavegameHeader(&file, header);
		if (header._thumbnail) {
			header._thumbnail->free();
			delete header._thumbnail;
		}

		file.close();

		if (isValid)
			// Set the name text
			return header._saveName;
	}

	return CString();
}

} // End of namespace Titanic
