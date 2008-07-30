/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#include "common/endian.h"
#include "common/events.h"

#include "base/plugins.h"
#include "common/config-manager.h"
#include "common/md5.h"
#include "sound/mididrv.h"

#include "gob/gob.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/dataio.h"
#include "gob/game.h"
#include "gob/sound/sound.h"
#include "gob/init.h"
#include "gob/inter.h"
#include "gob/draw.h"
#include "gob/goblin.h"
#include "gob/map.h"
#include "gob/mult.h"
#include "gob/palanim.h"
#include "gob/parse.h"
#include "gob/scenery.h"
#include "gob/videoplayer.h"
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
	_vm = this;

	_sound     = 0; _mult     = 0; _game   = 0;
	_global    = 0; _dataIO   = 0; _goblin = 0;
	_vidPlayer = 0; _init     = 0; _inter  = 0;
	_map       = 0; _palAnim  = 0; _parse  = 0;
	_scenery   = 0; _draw     = 0; _util   = 0;
	_video     = 0; _saveLoad = 0;

	_pauseStart = 0;

	// Setup mixer
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	_copyProtection = ConfMan.getBool("copy_protection");
	_quitRequested = false;

	Common::addSpecialDebugLevel(kDebugFuncOp, "FuncOpcodes", "Script FuncOpcodes debug level");
	Common::addSpecialDebugLevel(kDebugDrawOp, "DrawOpcodes", "Script DrawOpcodes debug level");
	Common::addSpecialDebugLevel(kDebugGobOp, "GoblinOpcodes", "Script GoblinOpcodes debug level");
	Common::addSpecialDebugLevel(kDebugSound, "Sound", "Sound output debug level");
	Common::addSpecialDebugLevel(kDebugParser, "Parser", "Parser debug level");
	Common::addSpecialDebugLevel(kDebugGameFlow, "Gameflow", "Gameflow debug level");
	Common::addSpecialDebugLevel(kDebugFileIO, "FileIO", "File Input/Output debug level");
	Common::addSpecialDebugLevel(kDebugSaveLoad, "SaveLoad", "Saving/Loading debug level");
	Common::addSpecialDebugLevel(kDebugGraphics, "Graphics", "Graphics debug level");
	Common::addSpecialDebugLevel(kDebugVideo, "Video", "IMD/VMD video debug level");
	Common::addSpecialDebugLevel(kDebugCollisions, "Collisions", "Collisions debug level");

	syst->getEventManager()->registerRandomSource(_rnd, "gob");
}

GobEngine::~GobEngine() {
	deinitGameParts();

	// Stop all mixer streams (except for the permanent ones).
	_vm->_mixer->stopAll();

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

const char *GobEngine::getLangDesc(int16 language) const {
	if ((language < 0) || (language > 8))
		language = 2;
	return Common::getLanguageDescription(_gobToScummVMLang[language]);
}

void GobEngine::validateLanguage() {
	if (_vm->_global->_languageWanted != _vm->_global->_language) {
		warning("Your game version doesn't support the requested language %s",
				getLangDesc(_vm->_global->_languageWanted));

		if (((_vm->_global->_languageWanted == 2) && (_vm->_global->_language == 5)) ||
		    ((_vm->_global->_languageWanted == 5) && (_vm->_global->_language == 2)))
			warning("Using %s instead", getLangDesc(_vm->_global->_language));
		else
			warning("Using the first language available: %s",
					getLangDesc(_vm->_global->_language));

		_vm->_global->_languageWanted = _vm->_global->_language;
	}
}

void GobEngine::validateVideoMode(int16 videoMode) {
	if ((videoMode != 0x10) && (videoMode != 0x13) &&
		  (videoMode != 0x14) && (videoMode != 0x18))
		error("Video mode 0x%X is not supported!", videoMode);
}

Endianness GobEngine::getEndianness() const {
	if ((_vm->getPlatform() == Common::kPlatformAmiga) ||
	    (_vm->getPlatform() == Common::kPlatformMacintosh) ||
	    (_vm->getPlatform() == Common::kPlatformAtariST))
		return kEndiannessBE;

	return kEndiannessLE;
}

Common::Platform GobEngine::getPlatform() const {
	return _platform;
}

GameType GobEngine::getGameType() const {
	return _gameType;
}

bool GobEngine::isCD() const {
	return (_features & kFeaturesCD) != 0;
}

bool GobEngine::isEGA() const {
	return (_features & kFeaturesEGA) != 0;
}

bool GobEngine::is640() const {
	return (_features & kFeatures640) != 0;
}

bool GobEngine::hasAdlib() const {
	return (_features & kFeaturesAdlib) != 0;
}

int GobEngine::init() {
	if (!initGameParts()) {
		GUIErrorMessage("GobEngine::init(): Unknown version of game engine");
		return -1;
	}

	_video->setSize(is640());

	// On some systems it's not safe to run CD audio games from the CD.
	if (isCD())
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

void GobEngine::pauseEngineIntern(bool pause) {
	if (pause) {
		_pauseStart = _system->getMillis();
	} else {
		uint32 duration = _system->getMillis() - _pauseStart;

		_vm->_vidPlayer->notifyPaused(duration);

		_vm->_game->_startTimeKey += duration;
		_vm->_draw->_cursorTimeKey += duration;
		if (_vm->_inter->_soundEndTimeKey != 0)
			_vm->_inter->_soundEndTimeKey += duration;
	}

	_mixer->pauseAll(pause);
}

bool GobEngine::initGameParts() {
	_noMusic = MidiDriver::parseMusicDriver(ConfMan.get("music_driver")) == MD_NULL;

	_saveLoad = 0;

	_global = new Global(this);
	_util = new Util(this);
	_dataIO = new DataIO(this);
	_palAnim = new PalAnim(this);
	_vidPlayer = new VideoPlayer(this);
	_sound = new Sound(this);

	switch (_gameType) {
		case kGameTypeGob1:
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
			break;

		case kGameTypeGob2:
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
			break;

		case kGameTypeBargon:
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
			break;

		case kGameTypeWeen:
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
			break;

		case kGameTypeGob3:
		case kGameTypeInca2:
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
			break;

		case kGameTypeLostInTime:
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
			_saveLoad = new SaveLoad_v3(this, _targetName.c_str(), 4768, 0, 50);
			break;

		case kGameTypeWoodruff:
			_init = new Init_v3(this);
			_video = new Video_v2(this);
			_inter = new Inter_v4(this);
			_parse = new Parse_v2(this);
			_mult = new Mult_v2(this);
			_draw = new Draw_v2(this);
			_game = new Game_v2(this);
			_map = new Map_v4(this);
			_goblin = new Goblin_v4(this);
			_scenery = new Scenery_v2(this);
			_saveLoad = new SaveLoad_v4(this, _targetName.c_str());
			break;

		default:
			deinitGameParts();
			return false;
			break;
	}

	if (is640()) {
		_video->_surfWidth = _width = 640;
		_video->_surfHeight = _video->_splitHeight1 = _height = 480;
		_global->_mouseMaxX = 640;
		_global->_mouseMaxY = 480;
		_mode = 0x18;
		_global->_primarySurfDesc = new SurfaceDesc(0x18, 640, 480);
	} else {
		_video->_surfWidth = _width = 320;
		_video->_surfHeight = _video->_splitHeight1 = _height = 200;
		_global->_mouseMaxX = 320;
		_global->_mouseMaxY = 200;
		_mode = 0x14;
		_global->_primarySurfDesc = new SurfaceDesc(0x14, 320, 200);
	}

	return true;
}

void GobEngine::deinitGameParts() {
	delete _saveLoad;  _saveLoad = 0;
	delete _mult;      _mult = 0;
	delete _vidPlayer; _vidPlayer = 0;
	delete _game;      _game = 0;
	delete _global;    _global = 0;
	delete _goblin;    _goblin = 0;
	delete _init;      _init = 0;
	delete _inter;     _inter = 0;
	delete _map;       _map = 0;
	delete _palAnim;   _palAnim = 0;
	delete _parse;     _parse = 0;
	delete _scenery;   _scenery = 0;
	delete _draw;      _draw = 0;
	delete _util;      _util = 0;
	delete _video;     _video = 0;
	delete _sound;     _sound = 0;
	delete _dataIO;    _dataIO = 0;
}

} // End of namespace Gob
