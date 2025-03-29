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

#include "got/got.h"
#include "common/config-manager.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "common/translation.h"
#include "engines/util.h"
#include "got/console.h"
#include "got/game/init.h"
#include "got/game/main.h"
#include "got/game/move.h"
#include "got/gfx/image.h"
#include "got/utils/res_archive.h"
#include "got/views/game_content.h"
#include "graphics/paletteman.h"

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
	// Initialize 320x240 paletted graphics mode. Note that the original
	// main menu/dialogs ran at 320x200, but the game ran at 320x240.
	initGraphics(320, 240);

	// Set the engine's debugger console
	setDebugger(new Console());

	// Initialize resources and variables
	resInit();
	_vars.load();

	// General initialization
	if (_G(demo))
		initGame();
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
	char title[32];
	Common::fill(title, title + 32, 0);
	Common::strcpy_s(title, _G(playerName).c_str());
	s.syncBytes((byte *)title, 32);
	if (s.isLoading())
		_G(playerName) = title;

	_G(setup).sync(s);

	if (s.isLoading()) {
		// For savegames loaded directly from the ScummVM launcher,
		// take care of initializing game defaults before rest of loading
		if (!firstView() || firstView()->getName() != "Game")
			initGame();

		int area = _G(setup)._areaNum;
		if (area == 0)
			area = 1;

		if (area != _G(area)) {
			_G(area) = area;
			g_vars->loadArea();
		}
	}

	_G(thorInfo).sync(s);
	_G(sdData).sync(s);

	if (s.isLoading())
		savegameLoaded();

	return Common::kNoError;
}

void GotEngine::savegameLoaded() {
	_G(currentArea) = _G(thorInfo)._lastScreen;

	_G(thor)->_x = (_G(thorInfo)._lastIcon % 20) * 16;
	_G(thor)->_y = ((_G(thorInfo)._lastIcon / 20) * 16) - 1;
	if (_G(thor)->_x < 1)
		_G(thor)->_x = 1;
	if (_G(thor)->_y < 0)
		_G(thor)->_y = 0;
	_G(thor)->_dir = _G(thorInfo)._lastDir;
	_G(thor)->_lastDir = _G(thorInfo)._lastDir;
	_G(thor)->_health = _G(thorInfo)._lastHealth;
	_G(thor)->_numMoves = 1;
	_G(thor)->_vulnerableCountdown = 60;
	_G(thor)->_show = 60;
	_G(thor)->_moveCountdown = 6;
	if (_G(area) == 2)
		_G(thorInfo)._armor = 1;
	if (_G(area) == 3)
		_G(thorInfo)._armor = 10;
	loadNewThor();

	g_vars->resetEndGameFlags();

	_G(setup)._musicEnabled = _G(musicFlag);
	_G(setup)._digitalSound = _G(soundFlag);

	if (_G(setup)._musicEnabled) {
		if (GAME1 && _G(currentArea) == 59) {
			musicPlay(5, true);
		} else {
			musicPlay(_G(levelMusic), true);
		}
	} else {
		_G(setup)._musicEnabled = true;
		musicPause();
		_G(setup)._musicEnabled = false;
	}

	_G(gameOver) = _G(setup)._gameOver != 0;
	_G(slowMode) = _G(setup)._slowMode;

	g_events->replaceView("Game", true);
	setupLoad();
}

bool GotEngine::canLoadGameStateCurrently(Common::U32String *msg) {
	if (_G(demo)) {
		*msg = _("Savegames are not available in demo mode");
		return false;
	}

	// Only allow if not in the middle of area transition, dying, etc.
	return _G(gameMode) == MODE_NORMAL;
}

bool GotEngine::canSaveGameStateCurrently(Common::U32String *msg) {
	if (_G(demo)) {
		*msg = _("Savegames are not available in demo mode");
		return false;
	}

	// Don't allowing saving when not in-game
	if (!firstView() || firstView()->getName() != "Game" || _G(gameOver))
		return false;

	// Only allow if not in the middle of area transition, dying, etc.
	return _G(gameMode) == MODE_NORMAL;
}

void GotEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	_G(sound).syncSoundSettings();
}

void GotEngine::pauseEngineIntern(bool pause) {
	g_vars->clearKeyFlags();
	if (_G(gameMode) == MODE_LIGHTNING)
		_G(gameMode) = MODE_NORMAL;

	if (_G(tornadoUsed)) {
		_G(tornadoUsed) = false;
		actorDestroyed(&_G(actor[2]));
	}

	if (_G(shieldOn)) {
		_G(actor[2])._dead = 2;
		_G(actor[2])._active = false;
		_G(shieldOn) = false;
	}

	_G(thunderSnakeCounter) = 0;

	if (pause) {
		_G(sound).musicPause();
	}
	else {
		_G(sound).musicResume();
	}

	Engine::pauseEngineIntern(pause);
}

Common::String GotEngine::getHighScoresSaveName() const {
	return Common::String::format("%s-scores.dat", _targetName.c_str());
}

} // End of namespace Got
