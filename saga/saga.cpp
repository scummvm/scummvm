/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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

#include "sound/mixer.h"
#include "common/file.h"
#include "common/config-manager.h"

#include "saga.h"

#include "gamedesc.h"


struct SAGAGameSettings {
	const char *name;
	const char *description;
	byte id;
	uint32 features;
	const char *detectname;
	GameSettings toGameSettings() const {
		GameSettings dummy = { name, description, features };
		return dummy;
	}
};

static const SAGAGameSettings saga_settings[] = {
	/* Inherit the Earth - Original floppy version */
  	{ "ite", "Inherit the Earth (DOS)", Saga::GID_ITE,
	 MDT_ADLIB, "ite.rsc" },
	/* Inherit the Earth - CD version */
	{ "itecd", "Inherit the Earth (DOS CD Version)", Saga::GID_ITECD,
	 MDT_ADLIB, "sounds.rsc" },
	/* I Have No Mouth and I Must Scream */
	{ "ihnm", "I Have No Mouth and I Must Scream (DOS)", Saga::GID_IHNM,
	 MDT_ADLIB, "scream.res" },

	{ NULL, NULL, 0, 0, NULL }
};

GameList Engine_SAGA_gameList() {
	const SAGAGameSettings *g = saga_settings;
	GameList games;
	while (g->name) {
		games.push_back(g->toGameSettings());
		g++;
	}
	return games;
}

DetectedGameList Engine_SAGA_detectGames(const FSList &fslist) {
	DetectedGameList detectedGames;
	const SAGAGameSettings *g;
	
	for (g = saga_settings; g->name; ++g) {
		// Iterate over all files in the given directory
		for (FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
			const char *gameName = file->displayName().c_str();

			if (0 == scumm_stricmp(g->detectname, gameName)) {
				// Match found, add to list of candidates, then abort inner loop.
				detectedGames.push_back(g->toGameSettings());
				break;
			}
		}
	}
	return detectedGames;
}

Engine *Engine_SAGA_create(GameDetector *detector, OSystem *syst) {
	return new Saga::SagaEngine(detector, syst);
}

REGISTER_PLUGIN("SAGA Engine", Engine_SAGA_gameList, Engine_SAGA_create, Engine_SAGA_detectGames)

namespace Saga {

SagaEngine::SagaEngine(GameDetector *detector, OSystem *syst)
	: Engine(syst) {

	setGameDirectory(getGameDataPath());
	openGame();

	// Setup mixer
	if (!_mixer->isReady()) {
		warning("Sound initialization failed.");
	}

	_mixer->setVolume(ConfMan.getInt("sfx_volume") * ConfMan.getInt("master_volume") / 255);

	// Initialize backend
	syst->initSize(320, 240);
}

SagaEngine::~SagaEngine() {

}

void SagaEngine::errorString(const char *buf1, char *buf2) {
	strcpy(buf2, buf1);
}

void SagaEngine::go() {


}

void SagaEngine::shutdown() {

	_system->quit();
}

}
