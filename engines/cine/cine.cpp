/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * cinE Engine is (C) 2004-2005 by CinE Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/config-manager.h"
#include "common/system.h"

#include "base/gameDetector.h"
#include "base/plugins.h"

#include "backends/fs/fs.h"

#include "sound/mididrv.h"
#include "sound/mixer.h"

#include "cine/cine.h"
#include "cine/main_loop.h"
#include "cine/object.h"
#include "cine/sfx_player.h"
#include "cine/sound_driver.h"
#include "cine/various.h"


namespace Cine {

SoundDriver *g_soundDriver;
SfxPlayer *g_sfxPlayer;

static void initialize();

char *savePath;

} // End of namespace Cine



struct CINEGameSettings {
	const char *name;
	const char *description;
	byte id;
	uint32 features;
	const char *detectname;
	GameDescriptor toGameDescriptor() const {
		return GameDescriptor(name, description);
	}
};

static const CINEGameSettings cine_settings[] = {
	{"fw", "Future Wars", Cine::GID_FW, MDT_ADLIB, "AUTO00.PRC"},
	{"os", "Operation Stealth", Cine::GID_OS, MDT_ADLIB, "PROCS00"},
	{NULL, NULL, 0, 0, NULL}
};

GameList Engine_CINE_gameIDList() {
	GameList games;
	const CINEGameSettings *g = cine_settings;

	while (g->name) {
		games.push_back(g->toGameDescriptor());
		g++;
	}

	return games;
}

GameDescriptor Engine_CINE_findGameID(const char *gameid) {
	const CINEGameSettings *g = cine_settings;
	while (g->name) {
		if (0 == scumm_stricmp(gameid, g->name))
			break;
		g++;
	}
	return g->toGameDescriptor();
}

DetectedGameList Engine_CINE_detectGames(const FSList &fslist) {
	DetectedGameList detectedGames;
	const CINEGameSettings *g;

	for (g = cine_settings; g->name; ++g) {
		// Iterate over all files in the given directory
		for (FSList::const_iterator file = fslist.begin();
		    file != fslist.end(); ++file) {
			const char *gameName = file->displayName().c_str();

			if (0 == scumm_stricmp(g->detectname, gameName)) {
				// Match found, add to list of candidates, then abort inner loop.
				detectedGames.push_back(g->toGameDescriptor());
				break;
			}
		}
	}
	return detectedGames;
}

Engine *Engine_CINE_create(GameDetector *detector, OSystem *syst) {
	return new Cine::CineEngine(detector, syst);
}

REGISTER_PLUGIN(CINE, "CINE Engine")

namespace Cine {

CineEngine::CineEngine(GameDetector *detector, OSystem *syst) : Engine(syst) {

	// Setup mixer
	if (!_mixer->isReady()) {
		warning("Sound initialization failed.");
	}

	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	_dataPath = getGameDataPath();
	_savePath = _saveFileMan->getSavePath();

	Common::File::addDefaultDirectory(_gameDataPath);

	const CINEGameSettings *g;

	for (g = cine_settings; g->name; ++g)
		if (!scumm_stricmp(g->name, detector->_targetName.c_str()))
			_gameId = g->id;

	gameType = _gameId;
}

CineEngine::~CineEngine() {
}

void CineEngine::errorString(const char *buf1, char *buf2) {
	strcpy(buf2, buf1);
}

int CineEngine::init(GameDetector &detector) {
	// Initialize backend
	_system->beginGFXTransaction();
	initCommonGFX(detector, false);
	_system->initSize(320, 200);
	_system->endGFXTransaction();

	if (gameType == GID_FW) {
		g_soundDriver = new AdlibSoundDriverINS(_mixer);
	} else {
		g_soundDriver = new AdlibSoundDriverADL(_mixer);
	}
	g_sfxPlayer = new SfxPlayer(g_soundDriver);

	initialize();

	return 0;
}

int CineEngine::go() {
	_system->showMouse(true);

	mainLoop(1);

	if (gameType == Cine::GID_FW)
		snd_clearBasesonEntries();

	delete g_sfxPlayer;
	delete g_soundDriver;
	return 0;
}


int gameType;

static void initialize() {
	uint16 i;

	init_video();

	textDataPtr = (uint8 *)malloc(8000);

	partBuffer = (PartBuffer *)malloc(NUM_MAX_PARTDATA * sizeof(PartBuffer));

	animDataTable = (AnimData *)malloc(NUM_MAX_ANIMDATA * sizeof(AnimData));
	
	loadTextData("texte.dat", textDataPtr);
	if (gameType == Cine::GID_FW)
		snd_loadBasesonEntries("BASESON.SND");

	for (i = 0; i < NUM_MAX_OBJECT; i++) {
		objectTable[i].part = 0;
		objectTable[i].name[0] = 0;
	}

	for (i = 0; i < NUM_MAX_OBJECTDATA; i++) {
		globalVars[i] = 0;
	}

	// bypass protection
	if (gameType == GID_OS) {
		globalVars[255] = 1;
	}

	for (i = 0; i < NUM_MAX_SCRIPT; i++) {
		scriptTable[i].ptr = NULL;
		scriptTable[i].size = 0;
	}

	for (i = 0; i < NUM_MAX_MESSAGE; i++) {
		messageTable[i].ptr = NULL;
		messageTable[i].len = 0;
	}

	for (i = 0; i < NUM_MAX_REL; i++) {
		relTable[i].data = NULL;
		relTable[i].size = 0;
		relTable[i].obj1Param1 = 0;
		relTable[i].obj1Param2 = 0;
		relTable[i].obj2Param = 0;
	}

	for (i = 0; i < NUM_MAX_ANIMDATA; i++) {
		animDataTable[i].ptr1 = NULL;
		animDataTable[i].ptr2 = NULL;
	}

	overlayHead.next = NULL;
	overlayHead.previous = NULL;

	var8 = 0;
	var9 = NULL;

	objScriptList.next = NULL;
	globalScriptsHead.next = NULL;

	objScriptList.scriptPtr = NULL;
	globalScriptsHead.scriptPtr = NULL;

	var2 = 0;
	var3 = 0;
	var4 = 0;
	var5 = 0;

	freePrcLinkedList();

	loadPrc(BOOT_PRC_NAME);
	strcpy(currentPrcName, BOOT_PRC_NAME);

	setMouseCursor(MOUSE_CURSOR_NORMAL);
}

} // End of namespace Cine
