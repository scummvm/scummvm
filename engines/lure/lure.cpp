/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"

#include "backends/fs/fs.h"

#include "base/gameDetector.h"
#include "base/plugins.h"

#include "common/config-manager.h"
#include "common/file.h"
#include "common/system.h"
#include "common/md5.h"

#include "sound/mixer.h"
#include "sound/mididrv.h"
#include "sound/audiostream.h"

#include "lure/luredefs.h"
#include "lure/surface.h"
#include "lure/lure.h"
#include "lure/intro.h"
#include "lure/game.h"
#include "lure/system.h"

using namespace Lure;

enum {
	// We only compute MD5 of the first megabyte of our data files.
	kMD5FileSizeLimit = 1024 * 1024
};

struct LureGameSettings {
	const char *gameid;
	const char *description;
	byte id;
	uint32 features;
	const char *md5sum;
	const char *checkFile;
};

//
static const LureGameSettings lure_games[] = {
	{ "lure", "Lure of the Temptress (Floppy, English)", GI_LURE, GF_ENGLISH | GF_FLOPPY, 
										"e45ea5d279a268c7d3c6524c2f63a2d2", "disk1.vga" },
	{ 0, 0, 0, 0, 0, 0 }
};

// Keep list of different supported games

static const GameSettings lure_list[] = {
	{ "lure", "Lure of the Temptress" },
	{ 0, 0 }
};

GameList Engine_LURE_gameIDList() {
	GameList games;
	const GameSettings *g = lure_list;

	while (g->gameid) {
		games.push_back(*g);
		g++;
	}
	return games;
}

GameSettings Engine_LURE_findGameID(const char *gameid) {
	const GameSettings *g = lure_list;
	while (g->gameid) {
		if (0 == strcmp(gameid, g->gameid))
			break;
		g++;
	}
	return *g;
}

DetectedGameList Engine_LURE_detectGames(const FSList &fslist) {
	DetectedGameList detectedGames;
	const LureGameSettings *g;
	FSList::const_iterator file;

	// Iterate over all files in the given directory
	bool isFound = false;
	for (file = fslist.begin(); file != fslist.end(); file++) {
		if (file->isDirectory())
			continue;

		for (g = lure_games; g->gameid; g++) {
			if (scumm_stricmp(file->displayName().c_str(), g->checkFile) == 0)
				isFound = true;
		}
		if (isFound)
			break;
	}

	if (file == fslist.end())
		return detectedGames;

	uint8 md5sum[16];
	char md5str[32 + 1];

	if (Common::md5_file(file->path().c_str(), md5sum, NULL, kMD5FileSizeLimit)) {
		for (int i = 0; i < 16; i++) {
			sprintf(md5str + i * 2, "%02x", (int)md5sum[i]);
		}
		for (g = lure_games; g->gameid; g++) {
			if (strcmp(g->md5sum, (char *)md5str) == 0) {
				detectedGames.push_back(toGameSettings(*g));
			}
		}
		if (detectedGames.isEmpty()) {
			debug("Unknown MD5 (%s)! Please report the details (language, platform, etc.) of this game to the ScummVM team\n", md5str);

			const GameSettings *g1 = lure_list;
			while (g1->gameid) {
				detectedGames.push_back(*g1);
				g1++;
			}
		}
	}
	return detectedGames;
}

Engine *Engine_LURE_create(GameDetector *detector, OSystem *system) {
	return new LureEngine(system);
}

REGISTER_PLUGIN(LURE, "Lure of the Temptress Engine")

namespace Lure {

LureEngine::LureEngine(OSystem *system): Engine(system) {
	// Setup mixer
/*
	if (!_mixer->isReady()) {
		warning("Sound initialization failed.");
	}

	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, ConfMan.getInt("speech_volume"));
*/
	_features = 0;
	_game = 0;
}

void LureEngine::detectGame() {
	// Make sure all the needed files are present
	
	if (!Common::File::exists(SUPPORT_FILENAME))
		error("Missing %s - this is a custom file containing resources from the\n"
			"Lure of the Temptress executable. See the documentation for creating it.",
			SUPPORT_FILENAME);

	for (uint8 fileNum = 1; fileNum <= 4; ++fileNum)
	{
		char sFilename[10];
		sprintf(sFilename, "disk%d.vga", fileNum);

		if (!Common::File::exists(sFilename))
			error("Missing disk%d.vga", fileNum);
	}

	// Check the version of the lure.dat file
	Common::File f;
	if (!f.open(SUPPORT_FILENAME)) {
		error("Error opening %s for validation", SUPPORT_FILENAME);
	} else {
		f.seek(0xbf * 8);
		VersionStructure version;
		f.read(&version, sizeof(VersionStructure));
		f.close();

		if (READ_LE_UINT16(&version.id) != 0xffff)
			error("Error validating %s - file is invalid or out of date", SUPPORT_FILENAME);
		else if ((version.vMajor != LURE_DAT_MAJOR) || (version.vMinor != LURE_DAT_MINOR))
			error("Incorrect version of %s file - expected %d.%d but got %d.%d",
				SUPPORT_FILENAME, LURE_DAT_MAJOR, LURE_DAT_MINOR, 
				version.vMajor, version.vMinor);
	}

	// Do an md5 check 
	
	uint8 md5sum[16];
	char md5str[32 + 1];
	const LureGameSettings *g;
	bool found = false;

	*md5str = 0;

	for (g = lure_games; g->gameid; g++) {
		if (!Common::File::exists(g->checkFile))
			continue;

		if (Common::md5_file(g->checkFile, md5sum, ConfMan.get("path").c_str(), kMD5FileSizeLimit)) {
			for (int j = 0; j < 16; j++) {
				sprintf(md5str + j * 2, "%02x", (int)md5sum[j]);
			}
		} else
			continue;

		if (strcmp(g->md5sum, (char *)md5str) == 0) {
			_features = g->features;
			_game = g->id;

			if (g->description)
				g_system->setWindowCaption(g->description);

			found = true;
			break;
		}
	}

	if (!found) {
		debug("Unknown MD5 (%s)! Please report the details (language, platform, etc.) of this game to the ScummVM team", md5str);
		_features = GF_LNGUNK || GF_FLOPPY;
		_game = GI_LURE;
	}
}

int LureEngine::init(GameDetector &detector) {
	_system->beginGFXTransaction();
		initCommonGFX(detector, false);
		_system->initSize(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT);
	_system->endGFXTransaction();

	detectGame();

	_sys = new System(_system);
	_disk = new Disk(_gameDataPath);
	_resources = new Resources();
	_strings = new StringData();
	_screen = new Screen(*_system);
	_mouse = new Mouse(*_system);
	_events = new Events(*_system);
	_menu = new Menu();
	Surface::initialise();
	_room = new Room();

	return 0;
}

LureEngine::~LureEngine() {
	Surface::deinitialise();
	delete _room;
	delete _menu;
	delete _events;
	delete _mouse;
	delete _screen;
	delete _strings;
	delete _resources;
	delete _disk;
	delete _sys;
}

int LureEngine::go() {
	// Show the introduction
	Introduction *intro = new Introduction(*_screen, *_system);
	intro->show();
	delete intro;

	// Play the game
	if (!_events->quitFlag) {
		// Play the game
		Game *gameInstance = new Game();
		gameInstance->execute();
		delete gameInstance;
	}

	//quitGame();
	return 0;
}

void LureEngine::errorString(const char *buf1, char *buf2) {
	strcpy(buf2, buf1);
}

void LureEngine::quitGame() {
	_system->quit();
}

} // End of namespace Lure
