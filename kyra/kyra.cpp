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
#include "kyra.h"

static const GameSettings kyra_setting =
        { "kyra", "Legend of Kyrandia", 0 };

GameList Engine_KYRA_gameList() {
        GameList games;

        games.push_back(kyra_setting);
        return games;
}

// TODO: Improve this :)
DetectedGameList Engine_KYRA_detectGames(const FSList &fslist) {
	DetectedGameList detectedGames;
	File test_file;

	// Iterate over all files in the given directory
	for (FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		const char *name = file->displayName().c_str();
		if ((0 == scumm_stricmp("chapter1.vrm", name))  ||
		    (0 == scumm_stricmp("chapter5.vrm", name))) {
			detectedGames.push_back(kyra_setting);
			printf("Detecting Kyra...\n");
			break;
		}
	}

	return detectedGames;
}

Engine *Engine_KYRA_create(GameDetector *detector, OSystem *syst) {
	return new Kyra::KyraEngine(detector, syst);
}

REGISTER_PLUGIN("Legend of Kyrandia Engine", Engine_KYRA_gameList, Engine_KYRA_create, Engine_KYRA_detectGames)

namespace Kyra {
KyraEngine::KyraEngine(GameDetector *detector, OSystem *syst)
	: Engine(syst) {

	// Setup mixer
	if (!_mixer->isReady()) {
		warning("Sound initialization failed.");
	}

	_mixer->setVolume(ConfMan.getInt("sfx_volume") * ConfMan.getInt("master_volume") / 255);

	//getGameDataPath();

	// Initialize backend
	syst->initSize(320, 240);
}

KyraEngine::~KyraEngine() {
}

void KyraEngine::errorString(const char *buf1, char *buf2) {
	strcpy(buf2, buf1);
}

void KyraEngine::go() {
	warning("Kyrandia Engine ::go()");
}

void KyraEngine::shutdown() {
	_system->quit();
}
} // End of namespace KYRA

