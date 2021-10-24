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
#include "chewy/events.h"
#include "chewy/main.h"
#include "chewy/resource.h"
#include "chewy/sound.h"

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
	delete _screen;
	delete _sound;
	g_engine = nullptr;
	g_screen = nullptr;
}

void ChewyEngine::initialize() {
	g_screen = _screen = new Graphics::Screen();
	_events = new EventsManager(_screen);
	_sound = new Sound(_mixer);

	_tempFiles.add(ADSH_TMP);
	SearchMan.add("temp", &_tempFiles, 99, false);
}

Common::Error ChewyEngine::run() {
	// Initialize backend
	//initGraphics(640, 480);
	initGraphics(320, 200);

	initialize();

	game_main();

	return Common::kNoError;
}

Common::Error ChewyEngine::loadGameStream(Common::SeekableReadStream *stream) {
	exit_room(-1);

	Common::Serializer s(stream, nullptr);
	if (!spieler.synchronize(s)) {
		fcode = READFEHLER;
		modul = DATEI;
		return Common::kReadingFailed;

	} else {
		flags.LoadGame = true;
		ERROR

		if (spieler.inv_cur == true && spieler.AkInvent != -1) {
			menu_item = CUR_USE;
		}

		if (spieler.AkInvent != -1)
			spieler.room_m_obj[spieler.AkInvent].RoomNr = -1;
		room->load_room(&room_blk, spieler.PersonRoomNr[P_CHEWY], &spieler);
		ERROR
		load_chewy_taf(spieler.ChewyAni);

		fx_blende = 1;
		room->calc_invent(&room_blk, &spieler);

		if (spieler.AkInvent != -1)
			spieler.room_m_obj[spieler.AkInvent].RoomNr = 255;
		obj->sort();

		set_speed();

		for (int i = 0; i < MAX_PERSON; i++) {
			set_person_pos(spieler.X[i], spieler.Y[i], i, spieler.Phase[i]);
		}

		auto_obj = 0;

		enter_room(-1);
		flags.LoadGame = false;

		return Common::kNoError;
	}
}

Common::Error ChewyEngine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	Common::Serializer s(nullptr, stream);
	int16 spr_nr;
	int16 i;
	spr_nr = chewy_ph[spieler_vector[P_CHEWY].Phase * 8 + spieler_vector[P_CHEWY].PhNr];
	for (i = 0; i < MAX_PERSON; i++) {
		spieler.X[i] = spieler_vector[i].Xypos[0];
		spieler.Y[i] = spieler_vector[i].Xypos[1];
		spieler.Phase[i] = person_end_phase[i];
	}

	return spieler.synchronize(s) ? Common::kNoError :
		Common::kWritingFailed;
}

} // End of namespace Chewy
