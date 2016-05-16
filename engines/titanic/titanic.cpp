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
#include "titanic/support/simple_file.h"

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
}

TitanicEngine::~TitanicEngine() {
	delete _debugger;
	delete _events;
	delete _screen;
	delete _window;
	delete _screenManager;
	delete _filesManager;
	CSaveableObject::freeClassList();
	_activeMovies.clear();
}

void TitanicEngine::initializePath(const Common::FSNode &gamePath) {
	Engine::initializePath(gamePath);
	SearchMan.addSubDirectoryMatching(gamePath, "assets");
}

void TitanicEngine::initialize() {
	// Set up debug channels
	DebugMan.addDebugChannel(kDebugCore, "core", "Core engine debug level");
	DebugMan.addDebugChannel(kDebugScripts, "scripts", "Game scripts");
	DebugMan.addDebugChannel(kDebugGraphics, "graphics", "Graphics handling");
	DebugMan.addDebugChannel(kDebugSound, "sound", "Sound and Music handling");

	CSaveableObject::initClassList();
	CEnterExitFirstClassState::init();
	CGetLiftEye2::init();
	CHose::init();
	CParrotLobbyObject::init();
	CSGTNavigation::init();
	CSGTStateRoom::init();
	CExitPellerator::init();
	CEnterExitSecClassMiniLift::init();
	CTelevision::init();
	OSVideoSurface::setup();

	_debugger = new Debugger(this);
	_events = new Events(this);
	_filesManager = new CFilesManager();
	_screen = new Graphics::Screen(0, 0);
	_screenManager = new OSScreenManager(this);
	_window = new CMainGameWindow(this);

	setItemNames();
	setRoomNames();
	setParserStrings();

	_window->applicationStarting();
}

void TitanicEngine::deinitialize() {
	CEnterExitFirstClassState::deinit();
	CGetLiftEye2::deinit();
	CHose::deinit();
	CSGTNavigation::deinit();
	CSGTStateRoom::deinit();
	CExitPellerator::deinit();
	CEnterExitSecClassMiniLift::deinit();
	CTelevision::deinit();
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

static CString readString(Common::SeekableReadStream *s) {
	CString result;
	char c;
	while ((c = s->readByte()) != '\0')
		result += c;

	return result;
}

void TitanicEngine::setItemNames() {
	Common::SeekableReadStream *r;
	r = g_vm->_filesManager->getResource("TEXT/ITEM_NAMES");
	while (r->pos() < r->size())
		_itemNames.push_back(readString(r));
	delete r;

	r = g_vm->_filesManager->getResource("TEXT/ITEM_DESCRIPTIONS");
	while (r->pos() < r->size())
		_itemNames.push_back(readString(r));
	delete r;

	r = g_vm->_filesManager->getResource("TEXT/ITEM_IDS");
	while (r->pos() < r->size())
		_itemIds.push_back(readString(r));
	delete r;
}

void TitanicEngine::setRoomNames() {
	Common::SeekableReadStream *r = g_vm->_filesManager->getResource("TEXT/ROOM_NAMES");
	while (r->pos() < r->size())
		_roomNames.push_back(readString(r));
	delete r;
}

void TitanicEngine::setParserStrings() {
	Common::SeekableReadStream *r;
	r = g_vm->_filesManager->getResource("TEXT/REPLACEMENTS1");
	while (r->pos() < r->size())
		_replacements1.push_back(readString(r));
	delete r;

	r = g_vm->_filesManager->getResource("TEXT/REPLACEMENTS2");
	while (r->pos() < r->size())
		_replacements2.push_back(readString(r));
	delete r;

	r = g_vm->_filesManager->getResource("TEXT/REPLACEMENTS3");
	while (r->pos() < r->size())
		_replacements3.push_back(readString(r));
	delete r;

	r = g_vm->_filesManager->getResource("TEXT/PHRASES");
	while (r->pos() < r->size())
		_phrases.push_back(readString(r));
	delete r;
}

} // End of namespace Titanic
