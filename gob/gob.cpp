/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
#include "stdafx.h"

#include "base/gameDetector.h"
#include "base/plugins.h"
#include "backends/fs/fs.h"

#include "gob/gob.h"

#include "gob/global.h"
#include "gob/game.h"
#include "gob/sound.h"
#include "gob/init.h"

static const GameSettings gob_games[] = {
	{"gob1", "Gobliiins", 0},
	{0, 0, 0}
};

GameList Engine_GOB_gameList() {
	GameList games;
	const GameSettings *g = gob_games;

	while (g->name) {
		games.push_back(*g);
		g++;
	}

	return games;
}

DetectedGameList Engine_GOB_detectGames(const FSList &fslist) {
	DetectedGameList detectedGames;

	return detectedGames;
}

Engine *Engine_GOB_create(GameDetector * detector, OSystem *syst) {
	return new Gob::GobEngine(detector, syst);
}

REGISTER_PLUGIN(GOB, "Gob Engine")

namespace Gob {
#define MAX_TIME_DELTA 100
	GobEngine *_vm = NULL;

GobEngine::GobEngine(GameDetector *detector, OSystem * syst) : Engine(syst) {

	// Setup mixer
	if (!_mixer->isReady()) {
		warning("Sound initialization failed.");
	}

	_mixer->setVolumeForSoundType(SoundMixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(SoundMixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	_vm = this;
}

GobEngine::~GobEngine() {
}

void GobEngine::errorString(const char *buf1, char *buf2) {
	strcpy(buf2, buf1);
}

int GobEngine::init(GameDetector &detector) {
	debugFlag = 1;
	breakSet = 0;
	doRangeClamp = 1;
	trySmallForBig = 0;

	checkMemFlag = 0;
	videoMode = 0x13;
	snd_soundPort = 1;
	useMouse = 1;
	soundFlags = 0;
	language = 5;

	return 0;
}

int GobEngine::go() {
	init_initGame(0);

	return 0;
}

void GobEngine::shutdown() {
	_system->quit();
}

} // End of namespace Gob
