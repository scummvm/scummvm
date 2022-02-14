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
#include "graphics/palette.h"
#include "chewy/chewy.h"
#include "chewy/debugger.h"
#include "chewy/events.h"
#include "chewy/global.h"
#include "chewy/main.h"
#include "chewy/resource.h"
#include "chewy/sound.h"
#include "chewy/video/cfo_decoder.h"

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
	g_engine = nullptr;
	g_screen = nullptr;
}

void ChewyEngine::initialize() {
	g_screen = _screen = new Graphics::Screen();
	_globals = new Globals();
	_events = new EventsManager(_screen);
	_sound = new Sound(_mixer);

	_tempFiles.add(ADSH_TMP, 5710);
	SearchMan.add("temp", &_tempFiles, 99, false);
	setDebugger(new Debugger());
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

Common::Error ChewyEngine::loadGameStream(Common::SeekableReadStream *stream) {
	exit_room(-1);

	Common::Serializer s(stream, nullptr);
	if (!_G(spieler).synchronize(s)) {
		error("loadGameStream error");
		return Common::kReadingFailed;

	} else {
		Common::SeekableWriteStream *adh = _tempFiles.createWriteStreamForMember(ADSH_TMP);
		if (stream->readUint32BE() != SCUMMVM_TAG ||
			stream->readUint32LE() != adh->size())
			return Common::kReadingFailed;
		adh->writeStream(stream, adh->size());

		_G(flags).LoadGame = true;

		if (_G(spieler).inv_cur && _G(spieler).AkInvent != -1) {
			_G(menu_item) = CUR_USE;
		}

		if (_G(spieler).AkInvent != -1)
			_G(spieler).room_m_obj[_G(spieler).AkInvent].RoomNr = -1;
		_G(room)->load_room(&_G(room_blk), _G(spieler).PersonRoomNr[P_CHEWY], &_G(spieler));
		load_chewy_taf(_G(spieler).ChewyAni);

		_G(fx_blend) = BLEND1;
		_G(room)->calc_invent(&_G(room_blk), &_G(spieler));

		if (_G(spieler).AkInvent != -1)
			_G(spieler).room_m_obj[_G(spieler).AkInvent].RoomNr = 255;
		_G(obj)->sort();

		set_speed();

		for (int i = 0; i < MAX_PERSON; i++) {
			set_person_pos(_G(spieler).X[i], _G(spieler).Y[i], i, _G(spieler).Phase[i]);
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
		_G(spieler).X[i] = _G(spieler_vector)[i].Xypos[0];
		_G(spieler).Y[i] = _G(spieler_vector)[i].Xypos[1];
		_G(spieler).Phase[i] = _G(person_end_phase)[i];
	}

	if (!_G(spieler).synchronize(s))
		return Common::kWritingFailed;

	Common::SeekableReadStream *rs = _tempFiles.createReadStreamForMember(ADSH_TMP);
	stream->writeUint32BE(SCUMMVM_TAG);
	stream->writeUint32LE(rs->size());
	stream->writeStream(rs);

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

void ChewyEngine::playVideo(uint num) {
	CfoDecoder *cfoDecoder = new CfoDecoder(g_engine->_sound);
	VideoResource *videoResource = new VideoResource("cut.tap");
	Common::SeekableReadStream *videoStream = videoResource->getVideoStream(num);

	if (!cfoDecoder->loadStream(videoStream)) {
		delete videoResource;
		delete cfoDecoder;
		return;
	}

	uint16 x = (g_system->getWidth() - cfoDecoder->getWidth()) / 2;
	uint16 y = (g_system->getHeight() - cfoDecoder->getHeight()) / 2;
	bool skipVideo = false;
	byte curPalette[256 * 3];

	g_system->getPaletteManager()->grabPalette(curPalette, 0, 256);
	//save_palette(curPalette);
	_G(cur)->hide_cur();

	cfoDecoder->start();

	while (!g_engine->shouldQuit() && !cfoDecoder->endOfVideo() && !skipVideo) {
		if (cfoDecoder->needsUpdate()) {
			const ::Graphics::Surface *frame = cfoDecoder->decodeNextFrame();
			if (frame) {
				g_system->copyRectToScreen(frame->getPixels(), frame->pitch, x, y, frame->w, frame->h);

				if (cfoDecoder->hasDirtyPalette())
					g_system->getPaletteManager()->setPalette(cfoDecoder->getPalette(), 0, 256);
					//setScummVMPalette(cfoDecoder->getPalette(), 0, 256);

				g_system->updateScreen();
			}
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			if ((event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_ESCAPE) || event.type == Common::EVENT_LBUTTONUP)
				skipVideo = true;
		}

		g_system->delayMillis(10);
	}

	cfoDecoder->close();

	g_system->getPaletteManager()->setPalette(curPalette, 0, 256);
	//setScummVMPalette(curPalette, 0, 256);
	_G(cur)->show_cur();

	delete videoResource;
	delete cfoDecoder;
}

} // End of namespace Chewy
