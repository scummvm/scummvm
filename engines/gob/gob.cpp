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
#include "common/endian.h"

#include "base/plugins.h"
#include "common/config-manager.h"
#include "common/fs.h"
#include "common/md5.h"
#include "sound/mididrv.h"

#include "gob/gob.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/dataio.h"
#include "gob/game.h"
#include "gob/sound.h"
#include "gob/init.h"
#include "gob/inter.h"
#include "gob/draw.h"
#include "gob/cdrom.h"
#include "gob/goblin.h"
#include "gob/map.h"
#include "gob/mult.h"
#include "gob/palanim.h"
#include "gob/parse.h"
#include "gob/scenery.h"
#include "gob/music.h"
#include "gob/imd.h"
#include "gob/saveload.h"

namespace Gob {

#define MAX_TIME_DELTA 100

const Common::Language GobEngine::_gobToScummVMLang[] = {
	Common::FR_FRA,
	Common::DE_DEU,
	Common::EN_GRB,
	Common::ES_ESP,
	Common::IT_ITA,
	Common::EN_USA,
	Common::NL_NLD,
	Common::KO_KOR,
	Common::HB_ISR
};

GobEngine::GobEngine(OSystem *syst) : Engine(syst) {
	// Setup mixer
	if (!_mixer->isReady()) {
		warning("Sound initialization failed.");
	}

	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	_copyProtection = ConfMan.getBool("copy_protection");
	_quitRequested = false;

	Common::addSpecialDebugLevel(kDebugFuncOp, "FuncOpcodes", "Script FuncOpcodes debug level");
	Common::addSpecialDebugLevel(kDebugDrawOp, "DrawOpcodes", "Script DrawOpcodes debug level");
	Common::addSpecialDebugLevel(kDebugGobOp, "GoblinOpcodes", "Script GoblinOpcodes debug level");
	Common::addSpecialDebugLevel(kDebugMusic, "Music", "CD, Adlib and Infogrames music debug level");
	Common::addSpecialDebugLevel(kDebugParser, "Parser", "Parser debug level");
	Common::addSpecialDebugLevel(kDebugGameFlow, "Gameflow", "Gameflow debug level");
	Common::addSpecialDebugLevel(kDebugFileIO, "FileIO", "File Input/Output debug level");
	Common::addSpecialDebugLevel(kDebugGraphics, "Graphics", "Graphics debug level");
	Common::addSpecialDebugLevel(kDebugCollisions, "Collisions", "Collisions debug level");
}

GobEngine::~GobEngine() {
	// Stop all mixer streams (except for the permanent ones).
	_vm->_mixer->stopAll();

	delete _snd;
	delete _adlib;
	delete _mult;
	delete _game;
	delete _global;
	delete _cdrom;
	delete _dataIO;
	delete _goblin;
	delete _imdPlayer;
	delete _init;
	delete _inter;
	delete _map;
	delete _palAnim;
	delete _parse;
	delete _scenery;
	delete _draw;
	delete _util;
	delete _video;
	delete _saveLoad;
	delete[] _startTot;
	delete[] _startTot0;
}

int GobEngine::go() {
	_init->initGame(0);

	return 0;
}

void GobEngine::shutdown() {
	_quitRequested = true;
}

void GobEngine::validateLanguage() {
	if (_vm->_global->_languageWanted != _vm->_global->_language) {
		warning("Your game version doesn't support the requested language");
		warning("Using the first language available: %s",
				getLangDesc(_vm->_global->_language));
		_vm->_global->_languageWanted = _vm->_global->_language;
	}
}

void GobEngine::validateVideoMode(int16 videoMode) {
	if ((videoMode != 0x13) && (videoMode != 0x14))
		error("Video mode 0x%X is not supported!", videoMode);
}

int GobEngine::init() {
	// Detect game
	if (!detectGame()) {
		GUIErrorMessage("No valid games were found in the specified directory.");
		return -1;
	}

	_adlib = 0;
	_saveLoad = 0;
	_global = new Global(this);
	_util = new Util(this);
	_dataIO = new DataIO(this);
	_palAnim = new PalAnim(this);
	_imdPlayer = new ImdPlayer(this);
	_cdrom = new CDROM(this);
	_snd = new Snd(this);
	if (_features & Gob::GF_GOB1) {
		_init = new Init_v1(this);
		_video = new Video_v1(this);
		_inter = new Inter_v1(this);
		_parse = new Parse_v1(this);
		_mult = new Mult_v1(this);
		_draw = new Draw_v1(this);
		_game = new Game_v1(this);
		_map = new Map_v1(this);
		_goblin = new Goblin_v1(this);
		_scenery = new Scenery_v1(this);
	} else if (_features & Gob::GF_GOB2) {
		_init = new Init_v2(this);
		_video = new Video_v2(this);
		_inter = new Inter_v2(this);
		_parse = new Parse_v2(this);
		_mult = new Mult_v2(this);
		_draw = new Draw_v2(this);
		_game = new Game_v2(this);
		_map = new Map_v2(this);
		_goblin = new Goblin_v2(this);
		_scenery = new Scenery_v2(this);
		_saveLoad = new SaveLoad_v2(this, _targetName.c_str());
	} else if (_features & Gob::GF_BARGON) {
		_init = new Init_v2(this);
		_video = new Video_v2(this);
		_inter = new Inter_Bargon(this);
		_parse = new Parse_v2(this);
		_mult = new Mult_v2(this);
		_draw = new Draw_Bargon(this);
		_game = new Game_v2(this);
		_map = new Map_v2(this);
		_goblin = new Goblin_v2(this);
		_scenery = new Scenery_v2(this);
		_saveLoad = new SaveLoad_v2(this, _targetName.c_str());
	} else if (_features & Gob::GF_GOB3) {
		_init = new Init_v3(this);
		_video = new Video_v2(this);
		_inter = new Inter_v3(this);
		_parse = new Parse_v2(this);
		_mult = new Mult_v2(this);
		_draw = new Draw_v2(this);
		_game = new Game_v2(this);
		_map = new Map_v2(this);
		_goblin = new Goblin_v3(this);
		_scenery = new Scenery_v2(this);
		_saveLoad = new SaveLoad_v3(this, _targetName.c_str());
	} else
		error("GobEngine::init(): Unknown version of game engine");

	_noMusic = MidiDriver::parseMusicDriver(ConfMan.get("music_driver")) == MD_NULL;
	if (!_noMusic && !(_platform == Common::kPlatformAmiga) &&
		 !(_platform == Common::kPlatformAtariST) &&
	   (((_platform == Common::kPlatformMacintosh) && (_features & Gob::GF_GOB1)) ||
	     (_features & Gob::GF_GOB2) || (_features & Gob::GF_GOB3)))
		_adlib = new Adlib(this);
	_vm = this;

	_map->init();

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
	_video->_doRangeClamp = true;

	// WORKAROUND: Some versions check the video mode to detect the system
	if (_platform == Common::kPlatformAmiga)
		_global->_fakeVideoMode = 0x11;
	else if (_platform == Common::kPlatformAtariST)
		_global->_fakeVideoMode = 0x10;
	else
		_global->_fakeVideoMode = 0x13;

	_global->_videoMode = 0x13;
	_global->_useMouse = 1;
	_global->_soundFlags = MIDI_FLAG | SPEAKER_FLAG | BLASTER_FLAG | ADLIB_FLAG;

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
	case Common::EN_ANY:
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
	_global->_languageWanted = _global->_language;

	// FIXME: This is the ugly way of reducing redraw overhead. It works
	//        well for 320x200 but it's unclear how well it will work for
	//        640x480.

	g_system->setFeatureState(OSystem::kFeatureAutoComputeDirtyRects, true);
	return 0;
}

} // End of namespace Gob
