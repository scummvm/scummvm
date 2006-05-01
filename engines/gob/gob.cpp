/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2006 The ScummVM project
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

#include "base/plugins.h"
#include "backends/fs/fs.h"
#include "common/config-manager.h"
#include "common/md5.h"

#include "gob/gob.h"

#include "gob/global.h"
#include "gob/game.h"
#include "gob/sound.h"
#include "gob/init.h"
#include "gob/inter.h"
#include "gob/draw.h"
#include "gob/anim.h"
#include "gob/cdrom.h"
#include "gob/goblin.h"
#include "gob/map.h"
#include "gob/mult.h"
#include "gob/pack.h"
#include "gob/palanim.h"
#include "gob/parse.h"
#include "gob/scenery.h"
#include "gob/timer.h"
#include "gob/util.h"
#include "gob/music.h"

namespace Gob {

enum {
	// We only compute MD5 of the first megabyte of our data files.
	kMD5FileSizeLimit = 1024 * 1024
};

struct GameSettings {
	const char *gameid;
	const char *description;
	uint32 features;
	Common::Language lang;
	const char *md5sum;
};

static const GameSettings gob_games[] = {
	// Supplied by Florian Zeitz on scummvm-devel
	{"gob1", "Gobliiins (DOS EGA)", GF_GOB1 | GF_EGA, Common::UNK_LANG, "82aea70ef26f41fa963dfae270993e49"},
	{"gob1", "Gobliiins (DOS EGA)", GF_GOB1 | GF_EGA, Common::UNK_LANG, "1f499458837008058b8ba6ae07057214"},
	{"gob1", "Gobliiins (Windows)", GF_GOB1, Common::UNK_LANG, "8a5e850c49d7cacdba5f5eb1fcc77b89"},

	// Supplied by Theruler76 in bug report #1201233
	{"gob1", "Gobliiins (DOS VGA)", GF_GOB1, Common::UNK_LANG, "a5e232fcd02733c7dffff107d22d36eb"},

	// CD 1.000 version. Multilingual
	{"gob1", "Gobliiins (CD)", GF_GOB1 | GF_CD, Common::UNK_LANG, "037db48ebce94bdfe42e2c9510da9211"},
	// CD 1.02 version. Multilingual
	{"gob1", "Gobliiins (CD)", GF_GOB1 | GF_CD, Common::UNK_LANG, "45f9c1162dd7040fd05fd013ccc176e2"},

	{"gob1", "Gobliiins (Amiga)", GF_GOB1, Common::UNK_LANG, "d9f8736b7dc0ea891cd06592a72e8a72"},
	{"gob1", "Gobliiins (Amiga)", GF_GOB1, Common::UNK_LANG, "69f9ae85252271e7dfa62883e581e5e9"},
	{"gob1", "Gobliiins (Amiga)", GF_GOB1, Common::UNK_LANG, "26de406cb09228d902274446a6a2eceb"},
	{"gob1", "Gobliiins (Amiga)", GF_GOB1, Common::UNK_LANG, "baf88a95928edb3f51067983f2dffa93"},

	{"gob1", "Gobliiins (Interactive Demo)", GF_GOB1, Common::UNK_LANG, "4f5bf4b9e4c39ebb93579747fc678e97"},
	
	{"gob1", "Gobliiins (Mac)", GF_GOB1 | GF_MAC, Common::UNK_LANG, "4c0e8ce4a2f66ee8226952ad3c6c1155"},

	{"gob2", "Gobliins 2 (DOS)", GF_GOB2, Common::FR_FRA, "abb5f762f9979d4253002de20f6e7b56"},
	{"gob2", "Gobliins 2 (DOS)", GF_GOB2, Common::EN_GRB, "9b6de65d811c08eebf50391b84fcba92"},
	{"gob2", "Gobliins 2 (DOS)", GF_GOB2, Common::EN_USA, "54d59c200e3823ad0af11a605a6fd06a"},
	{"gob2", "Gobliins 2 (DOS Ru)", GF_GOB2, Common::RU_RUS, "b6d47494bf88398ae59c1b5656cafce4"},
	// CD 1.000.
	{"gob2", "Gobliins 2 (CD)", GF_GOB2, Common::EN_USA, "02bf538fd8003b8da23a3546299c3df4"},
	// CD 1.01
	{"gob2", "Gobliins 2 (CD)", GF_GOB2, Common::UNK_LANG, "410e632682ab11969bc3b3b588066d95"},
	{"gob2", "Gobliins 2 (Demo)", GF_GOB2, Common::UNK_LANG, "be8b111191f965ac9b28fe530580d14e"},

#if 0
	{"gob3", "Goblins Quest 3", GF_GOB3, Common::UNK_LANG, "36d9b4032b39a794c8640e500e98893a"},
	{"gob3", "Goblins Quest 3", GF_GOB3, Common::UNK_LANG, "d129f639f6ca8d6b5f0f4e15edb91058"},
	{"gob3", "Goblins Quest 3", GF_GOB3, Common::UNK_LANG, "8d17b0abc514b1512fdedc6072acd48b"},
	// CD 1.000
	{"gob3", "Goblins Quest 3 (CD)", GF_GOB3, Common::UNK_LANG, "8d17b0abc514b1512fdedc6072acd48b"},
	// CD 1.02. Spanish "Computer Gaming World"* distribution in Spain
	{"gob3", "Goblins Quest 3 (CD)", GF_GOB3, Common::UNK_LANG, "7d7ab9a987be7208b9b685846fbd3e82"},

	{"gob3", "Goblins Quest 3 (Interactive Demo)", GF_GOB3, Common::UNK_LANG, "4986b44cec309589508d7904f924c217"},
	{"gob3", "Goblins Quest 3 (Demo)", GF_GOB3, "5024e7de8d6377fbbeabbaa92e0452bc"},
	{"gob3", "Goblins Quest 3 (Interactive Demo)", GF_GOB3, Common::UNK_LANG, "59ab69dab5fddbbf698c77a84297a5a2"},

	// CD 1.0
	{"woodruff", "The Bizarre Adventures of Woodruff and the Schnibble", GF_WOODRUFF, Common::UNK_LANG, "c27402cee260d2ff1c4cecb2006a630a"},

	// CD 1.00, German release (INTRO.STRK seems to be multilingual, though?)
	{"woodruff", "The Bizarre Adventures of Woodruff and the Schnibble", GF_WOODRUFF, Common::UNK_LANG, "751ba028d215e0db1e0254853de6a7e4"},
#endif
	{0, 0, 0, Common::UNK_LANG, 0}
};

// Keep list of different supported games
static const PlainGameDescriptor gob_list[] = {
	{"gob1", "Gobliiins"},
	{"gob2", "Gobliins 2"},
	{0, 0}
};


#define MAX_TIME_DELTA 100

GobEngine::GobEngine(OSystem * syst, uint32 features, Common::Language lang)
 : Engine(syst) {
	// Setup mixer
	if (!_mixer->isReady()) {
		warning("Sound initialization failed.");
	}

	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	_features = features;
	_language = lang;
	_copyProtection = ConfMan.getBool("copy_protection");

	Common::addSpecialDebugLevel(DEBUG_FUNCOP, "FuncOpcodes", "Script FuncOpcodes debug level");
	Common::addSpecialDebugLevel(DEBUG_DRAWOP, "DrawOpcodes", "Script DrawOpcodes debug level");
	Common::addSpecialDebugLevel(DEBUG_GOBOP, "GoblinOpcodes", "Script GoblinOpcodes debug level");
	Common::addSpecialDebugLevel(DEBUG_MUSIC, "Music", "CD and adlib music debug level");
	Common::addSpecialDebugLevel(DEBUG_PARSER, "Parser", "Parser debug level");
	Common::addSpecialDebugLevel(DEBUG_GAMEFLOW, "Gameflow", "Gameflow debug level");
	Common::addSpecialDebugLevel(DEBUG_FILEIO, "FileIO", "File Input/Output debug level");
	Common::addSpecialDebugLevel(DEBUG_GRAPHICS, "Graphics", "Graphics debug level");
	Common::addSpecialDebugLevel(DEBUG_COLLISIONS, "Collisions", "Collisions debug level");
}

GobEngine::~GobEngine() {
	delete _game;
	delete _snd;
	delete _video;
	delete _global;
	delete _draw;
	delete _anim;
	delete _cdrom;
	delete _dataio;
	delete _goblin;
	delete _init;
	delete _inter;
	delete _map;
	delete _mult;
	delete _pack;
	delete _palanim;
	delete _parse;
	delete _scenery;
	delete _gtimer;
	delete _util;
	delete _inter;
	delete _music;
}

void GobEngine::errorString(const char *buf1, char *buf2) {
	strcpy(buf2, buf1);
}

int GobEngine::go() {
	_init->initGame(0);

	return 0;
}

void GobEngine::shutdown() {
	_system->quit();
}

int GobEngine::init() {
	_snd = new Snd(this);
	_global = new Global(this);
	_anim = new Anim();
	_cdrom = new CDROM(this);
	_dataio = new DataIO(this);
	_goblin = new Goblin(this);
	_map = new Map(this);
	_pack = new Pack();
	_palanim = new PalAnim(this);
	_scenery = new Scenery(this);
	_gtimer = new GTimer();
	_util = new Util(this);
	if (_features & Gob::GF_GOB1) {
		_inter = new Inter_v1(this);
		_parse = new Parse_v1(this);
		_mult = new Mult_v1(this);
		_draw = new Draw_v1(this);
		_game = new Game_v1(this);
		_video = new Video_v1(this);
		_init = new Init_v1(this);
	}
	else if (_features & Gob::GF_GOB2) {
		_inter = new Inter_v2(this);
		_parse = new Parse_v2(this);
		_mult = new Mult_v2(this);
		_draw = new Draw_v2(this);
		_game = new Game_v2(this);
		_video = new Video_v2(this);
		_init = new Init_v2(this);
	}
	else
		error("GobEngine::init(): Unknown version of game engine");
	if ((_features & Gob::GF_MAC) || (_features & Gob::GF_GOB1))
		_music = new Music(this);

	_system->beginGFXTransaction();
		initCommonGFX(false);
		_system->initSize(320, 200);
	_system->endGFXTransaction();

	// On some systems it's not safe to run CD audio games from the CD.
	if (_features & GF_CD) 
		checkCD();

	int cd_num = ConfMan.getInt("cdrom");
	if (cd_num >= 0)
		_system->openCD(cd_num);

	_global->_debugFlag = 1;
	_global->_doRangeClamp = 1;

	_global->_videoMode = 0x13;
	_global->_useMouse = 1;
	_global->_soundFlags = 0;

	if (ConfMan.hasKey("language"))
		_language = Common::parseLanguage(ConfMan.get("language"));

	switch (_language) {
	case Common::FR_FRA:
	case Common::RU_RUS:
		_global->_language = 0;
		break;
	case Common::DE_DEU:
		_global->_language = 1;
		break;
	case Common::EN_GRB:
		_global->_language = 2;
		break;
	case Common::ES_ESP:
		_global->_language = 3;
		break;
	case Common::IT_ITA:
		_global->_language = 4;
		break;
	case Common::EN_USA:
		_global->_language = 5;
		break;
	case Common::NL_NLD:
		_global->_language = 6;
		break;
	case Common::KO_KOR:
		_global->_language = 7;
		break;
	case Common::HB_ISR:
		_global->_language = 8;
		break;
	default:
		// Default to English
		_global->_language = 2;
		break;
	}

	// FIXME: This is the ugly way of reducing redraw overhead. It works
	//        well for 320x200 but it's unclear how well it will work for
	//        640x480.

	g_system->setFeatureState(OSystem::kFeatureAutoComputeDirtyRects, true);

	return 0;
}

} // End of namespace Gob

using namespace Gob;

GameList Engine_GOB_gameIDList() {
	GameList games;
	const PlainGameDescriptor *g = gob_list;

	while (g->gameid) {
		games.push_back(*g);
		g++;
	}

	return games;
}

GameDescriptor Engine_GOB_findGameID(const char *gameid) {
	const PlainGameDescriptor *g = gob_list;
	while (g->gameid) {
		if (0 == scumm_stricmp(gameid, g->gameid))
			break;
		g++;
	}
	return *g;
}

DetectedGameList Engine_GOB_detectGames(const FSList &fslist) {
	DetectedGameList detectedGames;
	const GameSettings *g;
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

	if (Common::md5_file(file->path().c_str(), md5sum, kMD5FileSizeLimit)) {
		for (int i = 0; i < 16; i++) {
			sprintf(md5str + i * 2, "%02x", (int)md5sum[i]);
		}
		for (g = gob_games; g->gameid; g++) {
			if (strcmp(g->md5sum, (char *)md5str) == 0) {
				detectedGames.push_back(DetectedGame(g->gameid, g->description));
			}
		}
		if (detectedGames.empty()) {
			printf("Unknown MD5 (%s)! Please report the details (language, platform, etc.) of this game to the ScummVM team\n", md5str);

			const PlainGameDescriptor *g1 = gob_list;
			while (g1->gameid) {
				detectedGames.push_back(*g1);
				g1++;
			}
		}
	}
	return detectedGames;
}

PluginError Engine_GOB_create(OSystem *syst, Engine **engine) {
	// Detect game features based on MD5
	uint8 md5sum[16];
	char md5str[32 + 1];

	if (Common::md5_file("intro.stk", md5sum, kMD5FileSizeLimit)) {
		for (int j = 0; j < 16; j++) {
			sprintf(md5str + j*2, "%02x", (int)md5sum[j]);
		}
	} else {
		error("Engine_GOB_create(): Cannot find intro.stk");
	}

	const GameSettings *g;
	bool found = false;

	// TODO
	// Fallback. Maybe we will be able to determine game type from game
	// data contents
	Common::String realGame(ConfMan.get("gameid"));
	uint32 features;
	if (!scumm_stricmp(realGame.c_str(), "gob2"))
		features = GF_GOB2;
	else
		features = GF_GOB1;

	for (g = gob_games; g->gameid; g++) {
		if (strcmp(g->md5sum, (char *)md5str) == 0) {
			features = g->features;

			if (g->description)
				g_system->setWindowCaption(g->description);

			found = true;
			break;
		}
	}

	if (!found) {
		printf("Unknown MD5 (%s)! Please report the details (language, platform, etc.) of this game to the ScummVM team\n", md5str);
	}

	assert(engine);
	*engine = new GobEngine(syst, features, g->lang);
	return kNoError;
}

REGISTER_PLUGIN(GOB, "Gob Engine");
