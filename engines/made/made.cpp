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

#include "common/events.h"
#include "common/keyboard.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/config-manager.h"
#include "common/stream.h"

#include "graphics/cursorman.h"

#include "base/plugins.h"
#include "base/version.h"

#include "sound/mixer.h"

#include "made/made.h"
#include "made/database.h"
#include "made/pmvplayer.h"
#include "made/resource.h"
#include "made/screen.h"
#include "made/script.h"
#include "made/sound.h"
#include "made/redreader.h"

namespace Made {

struct GameSettings {
	const char *gameid;
	const char *description;
	byte id;
	uint32 features;
	const char *detectname;
};

static const GameSettings madeSettings[] = {
	{"made", "Made game", 0, 0, 0},

	{NULL, NULL, 0, 0, NULL}
};

MadeEngine::MadeEngine(OSystem *syst, const MadeGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {

	// Setup mixer
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	const GameSettings *g;

	const char *gameid = ConfMan.get("gameid").c_str();
	for (g = madeSettings; g->gameid; ++g)
		if (!scumm_stricmp(g->gameid, gameid))
			_gameId = g->id;

	_rnd = new Common::RandomSource();
	syst->getEventManager()->registerRandomSource(*_rnd, "made");

	int cd_num = ConfMan.getInt("cdrom");
	if (cd_num >= 0)
		_system->openCD(cd_num);
		
	_pmvPlayer = new PmvPlayer(this, _mixer);
	_res = new ProjectReader();
	_screen = new Screen(this);
	_dat = new GameDatabase();
	_script = new ScriptInterpreter(this);

}

MadeEngine::~MadeEngine() {
	delete _rnd;
	delete _pmvPlayer;
	delete _res;
	delete _screen;
	delete _dat;
	delete _script;
}

int MadeEngine::init() {
	// Initialize backend
	_system->beginGFXTransaction();
	initCommonGFX(false);
	_system->initSize(320, 200);
	_system->endGFXTransaction();

	return 0;
}

int16 MadeEngine::getTimer(int16 timerNum) {
	return (_system->getMillis() - _timers[timerNum]) / 60;
}

void MadeEngine::setTimer(int16 timerNum, int16 value) {
	_timers[timerNum] = value * 60;
}

void MadeEngine::resetTimer(int16 timerNum) {
	_timers[timerNum] = _system->getMillis();
}

int16 MadeEngine::allocTimer() {
	for (int i = 0; i < ARRAYSIZE(_timers); i++) {
		if (_timers[i] == -1) {
		 	resetTimer(i);
			return i + 1;
		}
	}
	return 0;
}

void MadeEngine::freeTimer(int16 timerNum) {
	_timers[timerNum] = -1;
}

int MadeEngine::go() {

	for (int i = 0; i < ARRAYSIZE(_timers); i++)
		_timers[i] = -1;

	if (getFeatures() & GF_DEMO) {
		_dat->open("demo.dat");
		_res->open("demo.prj");
	} else {
		if (Common::File::exists("rtzcd.dat"))
			_dat->open("rtzcd.dat");
		else
			_dat->openFromRed("rtzcd.red", "rtzcd.dat");
		_res->open("rtzcd.prj");
	}

	_eventMouseX = _eventMouseY = 0;
	_script->runScript(_dat->getMainCodeObjectIndex());

	return 0;
}

} // End of namespace Made
