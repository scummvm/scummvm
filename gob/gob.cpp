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
#include "common/stdafx.h"

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
	// Supplied by Florian Zeitz on scummvm-devel
	{"gob1", "Gobliiins (DOS EGA)", GF_GOB1, "82aea70ef26f41fa963dfae270993e49"},
	{"gob1", "Gobliiins (DOS EGA)", GF_GOB1, "1f499458837008058b8ba6ae07057214"},
	{"gob1", "Gobliiins (Windows)", GF_GOB1, "8a5e850c49d7cacdba5f5eb1fcc77b89"},

	// Supplied by Theruler76 in bug report #1201233
	{"gob1", "Gobliiins (DOS VGA)", GF_GOB1, "a5e232fcd02733c7dffff107d22d36eb"},

	// CD 1.000 version. Multilingual
	{"gob1", "Gobliiins (CD)", GF_GOB1, "037db48ebce94bdfe42e2c9510da9211"},
	// CD 1.02 version. Multilingual
	{"gob1", "Gobliiins (CD)", GF_GOB1, "45f9c1162dd7040fd05fd013ccc176e2"},

	{"gob1", "Gobliiins (Amiga)", GF_GOB1, "d9f8736b7dc0ea891cd06592a72e8a72"},
	{"gob1", "Gobliiins (Amiga)", GF_GOB1, "69f9ae85252271e7dfa62883e581e5e9"},
	{"gob1", "Gobliiins (Amiga)", GF_GOB1, "26de406cb09228d902274446a6a2eceb"},

	{"gob1", "Gobliiins (Interactive Demo)", GF_GOB1, "4f5bf4b9e4c39ebb93579747fc678e97"},

#if 0
	{"gob2", "Gobliins 2 (DOS)", GF_GOB2, "abb5f762f9979d4253002de20f6e7b56"},
	{"gob2", "Gobliins 2 (DOS)", GF_GOB2, "9b6de65d811c08eebf50391b84fcba92"},
	{"gob2", "Gobliins 2 (DOS)", GF_GOB2, "54d59c200e3823ad0af11a605a6fd06a"},
	{"gob2", "Gobliins 2 (DOS Ru)", GF_GOB2, "b6d47494bf88398ae59c1b5656cafce4"},
	// CD 1.000.
	{"gob2", "Gobliins 2 (CD)", GF_GOB2, "02bf538fd8003b8da23a3546299c3df4"},
	// CD 1.01
	{"gob2", "Gobliins 2 (CD)", GF_GOB2, "410e632682ab11969bc3b3b588066d95"},
	{"gob2", "Gobliins 2 (Demo)", GF_GOB2, "be8b111191f965ac9b28fe530580d14e"},

	{"gob3", "Goblins Quest 3", GF_GOB3, "36d9b4032b39a794c8640e500e98893a"},
	{"gob3", "Goblins Quest 3", GF_GOB3, "d129f639f6ca8d6b5f0f4e15edb91058"},
	{"gob3", "Goblins Quest 3", GF_GOB3, "8d17b0abc514b1512fdedc6072acd48b"},
	// CD 1.000
	{"gob3", "Goblins Quest 3 (CD)", GF_GOB3, "8d17b0abc514b1512fdedc6072acd48b"},
	// CD 1.02. Spanish "Computer Gaming World"* distribution in Spain
	{"gob3", "Goblins Quest 3 (CD)", GF_GOB3, "7d7ab9a987be7208b9b685846fbd3e82"},

	{"gob3", "Goblins Quest 3 (Interactive Demo)", GF_GOB3, "4986b44cec309589508d7904f924c217"},
	{"gob3", "Goblins Quest 3 (Demo)", GF_GOB3, "5024e7de8d6377fbbeabbaa92e0452bc"},
	{"gob3", "Goblins Quest 3 (Interactive Demo)", GF_GOB3, "59ab69dab5fddbbf698c77a84297a5a2"},

	// CD 1.0
	{"woodruff", "The Bizarre Adventures of Woodruff and the Schnibble", GF_WOODRUFF, "c27402cee260d2ff1c4cecb2006a630a"},

	// CD 1.00, German release (INTRO.STRK seems to be multilingual, though?)
	{"woodruff", "The Bizarre Adventures of Woodruff and the Schnibble", GF_WOODRUFF, "751ba028d215e0db1e0254853de6a7e4"},
#endif
	{0, 0, 0, 0}
};

// Keep list of different supported games
static const struct GobGameList {
	const char *name;
	const char *description;
	uint32 features;
	GameSettings toGameSettings() const {
		GameSettings dummy = { name, description, features };
		return dummy;
	}
} gob_list[] = {
	{"gob1", "Gobliiins", GF_GOB1},
	{"gob2", "Gobliins 2", GF_GOB2},
	{0, 0, 0}
};


GameList Engine_GOB_gameList() {
	GameList games;
	const GobGameList *g = gob_list;

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

			const GobGameList *g1 = gob_list;
			while (g1->name) {
				detectedGames.push_back(g1->toGameSettings());
				g1++;
			}
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

	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	_vm = this;

	// Detect game features based on MD5
	uint8 md5sum[16];
	char md5str[32 + 1];

	if (Common::md5_file("intro.stk", md5sum, ConfMan.get("path").c_str(), kMD5FileSizeLimit)) {
		for (int j = 0; j < 16; j++) {
			sprintf(md5str + j*2, "%02x", (int)md5sum[j]);
		}
	} else {
		error("GobEngine::GobEngine(): Cannot find intro.stk");
	}

	const GobGameSettings *g;
	bool found = false;

	// TODO
	// Fallback. Maybe we will be able to determine game type from game
	// data contents
	_features = GF_GOB1;

	for (g = gob_games; g->name; g++) {
		if (strcmp(g->md5sum, (char *)md5str) == 0) {
			_features = g->features;

			if (g->description)
				g_system->setWindowCaption(g->description);

			found = true;
			break;
		}
	}

	if (!found) {
		printf("Unknown MD5 (%s)! Please report the details (language, platform, etc.) of this game to the ScummVM team\n", md5str);
	}
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
	doRangeClamp = 1;

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

	// FIXME: This is the ugly way of reducing redraw overhead. It works
	//        well for 320x200 but it's unclear how well it will work for
	//        640x480.

	g_system->setFeatureState(OSystem::kFeatureAutoComputeDirtyRects, true);

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
