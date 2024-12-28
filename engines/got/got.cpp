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

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/paletteman.h"
#include "got/got.h"
#include "got/detection.h"
#include "got/console.h"
#include "got/game/init.h"
#include "got/game/main.h"
#include "got/gfx/image.h"
#include "got/utils/res_archive.h"
#include "got/views/game_content.h"

namespace Got {

#define SAVEGAME_VERSION 1

GotEngine *g_engine;

GotEngine::GotEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("Got") {
	g_engine = this;
}

GotEngine::~GotEngine() {
	_mixer->stopAll();
}

uint32 GotEngine::getFeatures() const {
	return _gameDescription->flags;
}

bool GotEngine::isDemo() const {
	return (_gameDescription->flags & ADGF_DEMO) != 0;
}

Common::String GotEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Error GotEngine::run() {
	// Initialize 320x240 palleted graphics mode. Note that the original
	// main menu/dialogs ran at 320x200, but the game ran at 320x240.
	initGraphics(320, 240);

	// Set the engine's debugger console
	setDebugger(new Console());

	// Initialize resources and variables
	resInit();
	_vars.load();

	// General initialization
	initialize();
	syncSoundSettings();

	runGame();

	return Common::kNoError;
}

Common::Error GotEngine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	stream->writeByte(SAVEGAME_VERSION);
	Common::Serializer s(nullptr, stream);
	s.setVersion(SAVEGAME_VERSION);

	return syncGame(s);
}

Common::Error GotEngine::loadGameStream(Common::SeekableReadStream *stream) {
	byte version = stream->readByte();
	if (version != SAVEGAME_VERSION)
		error("Invalid savegame version");

	Common::Serializer s(stream, nullptr);
	s.setVersion(version);

	return syncGame(s);
}


Common::Error GotEngine::syncGame(Common::Serializer &s) {
	_G(setup).sync(s);
	_G(thor_info).sync(s);
	_G(sd_data).sync(s);

	if (s.isLoading())
		savegameLoaded();

	return Common::kNoError;
}

void GotEngine::savegameLoaded() {
	int area = _G(setup).area;
	if (area == 0)
		area = 1;

	g_vars->setArea(area);

	_G(current_area) = _G(thor_info).last_screen;

	_G(thor)->x = (_G(thor_info).last_icon % 20) * 16;
	_G(thor)->y = ((_G(thor_info).last_icon / 20) * 16) - 1;
	if (_G(thor)->x < 1) _G(thor)->x = 1;
	if (_G(thor)->y < 0) _G(thor)->y = 0;
	_G(thor)->dir = _G(thor_info).last_dir;
	_G(thor)->last_dir = _G(thor_info).last_dir;
	_G(thor)->health = _G(thor_info).last_health;
	_G(thor)->num_moves = 1;
	_G(thor)->vunerable = 60;
	_G(thor)->show = 60;
	_G(thor)->speed_count = 6;
	load_new_thor();

	if (!_G(music_flag))
		_G(setup).music = 0;
	if (!_G(sound_flag))
		_G(setup).dig_sound = 0;
	if (_G(setup).music == 1) {
		if (GAME1 == 1 && _G(current_area) == 59) {
//				if (flag)
				music_play(5, 1);
		} else {
			//if (flag)
			music_play(_G(level_type), 1);
		}
	} else {
		_G(setup).music = 1;
		music_pause();
		_G(setup).music = 0;
	}

	_G(game_over) = _G(setup).game_over;
	_G(slow_mode) = _G(setup).speed;

	g_events->replaceView("Game", true);
	setup_load();
}

bool GotEngine::canSaveGameStateCurrently(Common::U32String *msg) {
	if (_G(key_flag)[key_magic] || _G(tornado_used) || _G(lightning_used) ||
			_G(thunder_flag) || _G(hourglass_flag) || _G(thor)->num_moves > 1 ||
			_G(shield_on))
		return false;

	// Only allow if not in the middle of area transition, dying, etc.
	return _G(gameMode) == MODE_NORMAL;
}

void GotEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	bool allSoundIsMuted = ConfMan.getBool("mute");

	_mixer->muteSoundType(Audio::Mixer::kSFXSoundType,
		ConfMan.getBool("sfx_mute") || allSoundIsMuted);
	_mixer->muteSoundType(Audio::Mixer::kMusicSoundType,
		ConfMan.getBool("music_mute") || allSoundIsMuted);
}

} // End of namespace Got
