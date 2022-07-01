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
#include "common/fs.h"
#include "common/system.h"
#include "engines/util.h"
#include "chewy/chewy.h"
#include "chewy/cursor.h"
#include "chewy/console.h"
#include "chewy/events.h"
#include "chewy/globals.h"
#include "chewy/main.h"
#include "chewy/resource.h"
#include "chewy/sound.h"
#include "chewy/video/video_player.h"

namespace Chewy {

ChewyEngine *g_engine;
Graphics::Screen *g_screen;

ChewyEngine::ChewyEngine(OSystem *syst, const ChewyGameDescription *gameDesc)
		: Engine(syst),
		_gameDescription(gameDesc),
		_rnd("chewy") {

	g_engine = this;
	g_screen = nullptr;
	const Common::FSNode gameDataDir(ConfMan.get("path"));

	SearchMan.addSubDirectoryMatching(gameDataDir, "back");
	SearchMan.addSubDirectoryMatching(gameDataDir, "cut");
	SearchMan.addSubDirectoryMatching(gameDataDir, "err");
	SearchMan.addSubDirectoryMatching(gameDataDir, "misc");
	SearchMan.addSubDirectoryMatching(gameDataDir, "room");
	SearchMan.addSubDirectoryMatching(gameDataDir, "sound");
	SearchMan.addSubDirectoryMatching(gameDataDir, "txt");
}

ChewyEngine::~ChewyEngine() {
	delete _events;
	delete _globals;
	delete _screen;
	delete _sound;
	delete _video;
	g_engine = nullptr;
	g_screen = nullptr;
}

void ChewyEngine::initialize() {
	g_screen = _screen = new Graphics::Screen();
	_globals = new Globals();
	_events = new EventsManager(_screen);
	_sound = new Sound(_mixer);
	_video = new VideoPlayer();

	setDebugger(new Console());

	syncSoundSettings();
}

Common::Error ChewyEngine::run() {
	// Initialize backend
	//initGraphics(640, 480);
	initGraphics(320, 200);

	initialize();

	game_main();

	return Common::kNoError;
}

#define SCUMMVM_TAG MKTAG('S', 'C', 'V', 'M')

void ChewyEngine::syncSoundSettings() {
	_sound->syncSoundSettings();

	Engine::syncSoundSettings();
}

Common::Error ChewyEngine::loadGameStream(Common::SeekableReadStream *stream) {
	exit_room(-1);

	Common::Serializer s(stream, nullptr);
	if (!_G(gameState).synchronize(s)) {
		error("loadGameStream error");
		return Common::kReadingFailed;

	} else {
		if (stream->readUint32BE() != SCUMMVM_TAG ||
			stream->readUint32LE() != _G(atds)->getAtdsStreamSize())
			return Common::kReadingFailed;
		_G(atds)->loadAtdsStream(stream);

		_G(flags).LoadGame = true;

		if (_G(cur)->usingInventoryCursor()) {
			_G(menu_item) = CUR_USE;
		}

		if (_G(cur)->usingInventoryCursor())
			_G(gameState).room_m_obj[_G(cur)->getInventoryCursor()].RoomNr = -1;
		_G(room)->loadRoom(&_G(room_blk), _G(gameState)._personRoomNr[P_CHEWY], &_G(gameState));
		load_chewy_taf(_G(gameState).ChewyAni);

		_G(fx_blend) = BLEND1;
		_G(room)->calc_invent(&_G(room_blk), &_G(gameState));

		if (_G(cur)->usingInventoryCursor())
			_G(gameState).room_m_obj[_G(cur)->getInventoryCursor()].RoomNr = 255;
		_G(obj)->sort();

		for (int i = 0; i < MAX_PERSON; i++) {
			setPersonPos(_G(gameState).X[i], _G(gameState).Y[i], i, _G(gameState).Phase[i]);
		}

		_G(auto_obj) = 0;

		enter_room(-1);
		_G(flags).LoadGame = false;

		return Common::kNoError;
	}
}

Common::Error ChewyEngine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	Common::Serializer s(nullptr, stream);

	for (int i = 0; i < MAX_PERSON; i++) {
		_G(gameState).X[i] = _G(moveState)[i].Xypos[0];
		_G(gameState).Y[i] = _G(moveState)[i].Xypos[1];
		_G(gameState).Phase[i] = _G(person_end_phase)[i];
	}

	if (!_G(gameState).synchronize(s))
		return Common::kWritingFailed;

	stream->writeUint32BE(SCUMMVM_TAG);
	stream->writeUint32LE(_G(atds)->getAtdsStreamSize());
	_G(atds)->saveAtdsStream(stream);

	return Common::kNoError;
}

SaveStateList ChewyEngine::listSaves() {
	return getMetaEngine()->listSaves(_targetName.c_str());
}

void ChewyEngine::showGmm(bool isInGame) {
	_canLoad = true;
	_canSave = isInGame;

	openMainMenuDialog();
	_events->clearEvents();

	_canLoad = false;
	_canSave = false;
}

} // End of namespace Chewy
