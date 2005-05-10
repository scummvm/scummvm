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
#include "common/md5.h"

#include "gob/gob.h"

#include "gob/global.h"
#include "gob/game.h"
#include "gob/sound.h"
#include "gob/init.h"

enum {
	// We only compute MD5 of the first megabyte of our data files.
	kMD5FileSizeLimit = 1024 * 1024
};

struct GobGameSettings {
	const char *name;
	const char *description;
	uint32 features;
	const char *md5sum;
	GameSettings toGameSettings() const {
		GameSettings dummy = { name, description, features };
		return dummy;
	}
};

static const GobGameSettings gob_games[] = {
	{"gob1", "Gobliiins 1 (CD)", 0, "45f9c1162dd7040fd05fd013ccc176e2"},
	{"gob1", "Gobliiins 1 (DOS)", 0, "82aea70ef26f41fa963dfae270993e49"},
	{"gob1", "Gobliiins 1 (Windows)", 0, "8a5e850c49d7cacdba5f5eb1fcc77b89"},
	{"gob1", "Gobliiins 1 (Demo)", 0, "4f5bf4b9e4c39ebb93579747fc678e97"},

#if 0
	// Not supported yet
	{"gob2", "Gobliins 2 (CD)", 0, "410e632682ab11969bc3b3b588066d95"},
	{"gob2", "Gobliins 2 (Demo)", 0, "be8b111191f965ac9b28fe530580d14e"},
	{"gob3", "Goblins Quest 3 (Demo)", 0, "5024e7de8d6377fbbeabbaa92e0452bc"},
	{"woodruff", "The Bizarre Adventures of Woodruff and the Schnibble", 0, "c27402cee260d2ff1c4cecb2006a630a"},
#endif
	{0, 0, 0, 0}
};

GameList Engine_GOB_gameList() {
	GameList games;
	const GobGameSettings *g = gob_games;

	while (g->name) {
		games.push_back(g->toGameSettings());
		g++;
	}

	return games;
}

DetectedGameList Engine_GOB_detectGames(const FSList &fslist) {
	DetectedGameList detectedGames;
	const GobGameSettings *g;
	FSList::const_iterator file;

	// Iterate over all files in the given directory
	for (file = fslist.begin(); file != fslist.end(); file++) {
		if (file->isDirectory())
			continue;

		// All the supported games have an intro.stk file.
		if (scumm_stricmp(file->displayName().c_str(), "intro.stk") == 0)
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
		for (g = gob_games; g->name; g++) {
			if (strcmp(g->md5sum, (char *)md5str) == 0) {
				detectedGames.push_back(g->toGameSettings());
			}
		}
		if (detectedGames.isEmpty()) {
			printf("Unknown MD5 (%s)! Please report the details (language, platform, etc.) of this game to the ScummVM team\n", md5str);
		}
	}
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
	_system->beginGFXTransaction();
		initCommonGFX(detector);
		_system->initSize(320, 200);
	_system->endGFXTransaction();

	int cd_num = ConfMan.getInt("cdrom");
	if (cd_num >= 0)
		_system->openCD(cd_num);

	debugFlag = 1;
	breakSet = 0;
	doRangeClamp = 1;
	trySmallForBig = 0;

	checkMemFlag = 0;
	videoMode = 0x13;
	snd_soundPort = 1;
	useMouse = 1;
	soundFlags = 0;

	switch (Common::parseLanguage(ConfMan.get("language"))) {
	case Common::FR_FRA:
		language = 0;
		break;
	case Common::DE_DEU:
		language = 1;
		break;
	case Common::ES_ESP:
		language = 3;
		break;
	case Common::IT_ITA:
		language = 4;
		break;
	default:
		// Default to English
		language = 2;
		break;
	}

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
