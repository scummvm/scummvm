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

#include "base/gameDetector.h"
#include "base/plugins.h"

#include "backends/fs/fs.h"

#include "sound/mididrv.h"
#include "sound/mixer.h"

#include "cine/cine.h"
#include "cine/sound_driver.h"


namespace Cine {

Audio::Mixer * cine_g_mixer;
AdlibMusic *g_cine_adlib;

static void initialize();

char *savePath;

} // End of namespace Cine



struct CINEGameSettings {
	const char *name;
	const char *description;
	byte id;
	uint32 features;
	const char *detectname;
	GameSettings toGameSettings() const {
		GameSettings dummy = { name, description };
		return dummy;
	}
};

static const CINEGameSettings cine_settings[] = {
	{"fw", "Future Wars", Cine::GID_FW, MDT_ADLIB, "AUTO00.PRC"},
	{"os", "Operation Stealth", Cine::GID_OS, MDT_ADLIB, "PROCS00"},
	{NULL, NULL, 0, 0, NULL}
};

// Keep list of different supported games
static const GameSettings cine_list[] = {
	{"fw", "Future Wars"},
	{"os", "Operation Stealth"},
	{0, 0}
};

GameList Engine_CINE_gameIDList() {
	GameList games;
	const GameSettings *g = cine_list;

	while (g->gameid) {
		games.push_back(*g);
		g++;
	}

	return games;
}

GameSettings Engine_CINE_findGameID(const char *gameid) {
	const GameSettings *g = cine_list;
	while (g->gameid) {
		if (0 == strcmp(gameid, g->gameid))
			break;
		g++;
	}
	return *g;
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
				detectedGames.push_back(g->toGameSettings());
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

	cine_g_mixer = _mixer;
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

	g_cine_adlib = new AdlibMusic(_mixer);

	initialize();

	return 0;
}

int CineEngine::go() {
	_system->showMouse(true);

	mainLoop(1);

	if (gameType == Cine::GID_FW)
		snd_clearBasesonEntries();

	delete g_cine_adlib;

	return 0;
}


int gameType;

static void initialize() {
	uint16 i;

	init_video();

	textDataPtr = (uint8 *)malloc(8000);

	partBuffer = (partBufferStruct *)malloc(255 * sizeof(animDataStruct));

	loadTextData("texte.dat", textDataPtr);
	snd_loadBasesonEntries("BASESON.SND");

	for (i = 0; i < NUM_MAX_OBJECT; i++) {
		objectTable[i].part = 0;
		objectTable[i].name[0] = 0;
	}

	for (i = 0; i < NUM_MAX_OBJECTDATA; i++) {
		globalVars[i] = 0;
	}

	globalVars[255] = 1;

	for (i = 0; i < NUM_MAX_SCRIPT; i++) {
		scriptTable[i].ptr = NULL;
		scriptTable[i].var4 = 0;
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

	for (i = 0; i < NUM_MAX_PARTDATA; i++) {
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

	processPendingUpdates(0);
}

} // End of namespace Cine
