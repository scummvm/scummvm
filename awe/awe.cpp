/* AWE - Another World Engine
 * Copyright (C) 2004 Gregory Montoir
 * Copyright (C) 2004 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "common/stdafx.h"

#include "base/gameDetector.h"
#include "base/plugins.h"
#include "backends/fs/fs.h"

#include "sound/mixer.h"
#include "common/file.h"
#include "common/config-manager.h"

#include "awe/awe.h"
#include "awe/engine.h"
#include "awe/systemstub.h"
#include "awe/util.h"


struct AWEGameSettings {
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

static const AWEGameSettings awe_settings[] = {
	/* Another World - Out of this World - Europe DOS version */
	{ "worlde", "Another World - Out of this World - (Europe, DOS)", Awe::GID_WORLDE, MDT_ADLIB, "bank01" },
	{ NULL, NULL, 0, 0, NULL }
};

GameList Engine_AWE_gameList() {
	const AWEGameSettings *g = awe_settings;
	GameList games;
	while (g->name) {
		games.push_back(g->toGameSettings());
		g++;
	}
	return games;
}

DetectedGameList Engine_AWE_detectGames(const FSList &fslist) {
	DetectedGameList detectedGames;
	const AWEGameSettings *g;

	for (g = awe_settings; g->name; ++g) {
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

Engine *Engine_AWE_create(GameDetector *detector, OSystem *syst) {
	return new Awe::AweEngine(detector, syst);
}

REGISTER_PLUGIN("AWE Engine", Engine_AWE_gameList, Engine_AWE_create, Engine_AWE_detectGames)

namespace Awe {

AweEngine::AweEngine(GameDetector *detector, OSystem *syst)
	: Engine(syst) {

	// Setup mixer
	if (!_mixer->isReady()) {
		::warning("Sound initialization failed.");
	}

	_mixer->setVolume(ConfMan.getInt("sfx_volume") * ConfMan.getInt("master_volume") / 255);

	_dataPath = getGameDataPath();
	_savePath = getSavePath();

	// Initialize backend
	syst->initSize(320, 200);
}

AweEngine::~AweEngine() {

}

void AweEngine::errorString(const char *buf1, char *buf2) {
	strcpy(buf2, buf1);
}

void AweEngine::go() {
	g_debugMask = DBG_INFO;
	SystemStub *stub = SystemStub_SDL_create();
	debug(0, "%s %s", _dataPath, _savePath);
	Awe::Engine *e = new Awe::Engine(stub, _dataPath, _savePath);
	e->run();
	delete e;
	delete stub;
}

void AweEngine::shutdown() {
	_system->quit();
}

}
