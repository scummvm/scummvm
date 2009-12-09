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
#include "common/EventRecorder.h"
#include "common/keyboard.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/config-manager.h"
#include "common/stream.h"

#include "graphics/cursorman.h"

#include "base/plugins.h"
#include "base/version.h"

#include "sound/audiocd.h"
#include "sound/mixer.h"

#include "made/made.h"
#include "made/database.h"
#include "made/pmvplayer.h"
#include "made/resource.h"
#include "made/screen.h"
#include "made/script.h"
#include "made/sound.h"
#include "made/music.h"
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

	const GameSettings *g;

	const char *gameid = ConfMan.get("gameid").c_str();
	for (g = madeSettings; g->gameid; ++g)
		if (!scumm_stricmp(g->gameid, gameid))
			_gameId = g->id;

	_rnd = new Common::RandomSource();
	g_eventRec.registerRandomSource(*_rnd, "made");

	int cd_num = ConfMan.getInt("cdrom");
	if (cd_num >= 0)
		_system->openCD(cd_num);

	_pmvPlayer = new PmvPlayer(this, _mixer);
	_res = new ResourceReader();
	_screen = new Screen(this);

	if (getGameID() == GID_LGOP2 || getGameID() == GID_MANHOLE || getGameID() == GID_RODNEY) {
		_dat = new GameDatabaseV2(this);
	} else if (getGameID() == GID_RTZ) {
		_dat = new GameDatabaseV3(this);
	} else {
		error("Unknown GameID");
	}

	_script = new ScriptInterpreter(this);

	MidiDriverType midiDriver = MidiDriver::detectMusicDriver(MDT_MIDI | MDT_ADLIB | MDT_PREFER_MIDI);
	bool native_mt32 = ((midiDriver == MD_MT32) || ConfMan.getBool("native_mt32"));
	//bool adlib = (midiDriver == MD_ADLIB);

	MidiDriver *driver = MidiDriver::createMidi(midiDriver);
	if (native_mt32)
		driver->property(MidiDriver::PROP_CHANNEL_MASK, 0x03FE);

	_music = new MusicPlayer(driver);
	_music->setNativeMT32(native_mt32);
	//_music->setAdlib(adlib);

	// Set default sound frequency
	switch (getGameID()) {
	case GID_RODNEY:
		_soundRate = 11025;
		break;
	case GID_MANHOLE:
		_soundRate = 11025;
		break;
	case GID_LGOP2:
		_soundRate = 8000;
		break;
	case GID_RTZ:
		// Return to Zork sets it itself via a script funtion
		break;
	}

	syncSoundSettings();
}

MadeEngine::~MadeEngine() {
	AudioCD.stop();

	delete _rnd;
	delete _pmvPlayer;
	delete _res;
	delete _screen;
	delete _dat;
	delete _script;
	delete _music;
}

void MadeEngine::syncSoundSettings() {
	_music->setVolume(ConfMan.getInt("music_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, ConfMan.getInt("speech_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));
}

int16 MadeEngine::getTicks() {
	return g_system->getMillis() * 30 / 1000;
}

int16 MadeEngine::getTimer(int16 timerNum) {
	if (timerNum > 0 && timerNum <= ARRAYSIZE(_timers) && _timers[timerNum - 1] != -1)
		return (getTicks() - _timers[timerNum - 1]);
	else
		return 32000;
}

void MadeEngine::setTimer(int16 timerNum, int16 value) {
	if (timerNum > 0 && timerNum <= ARRAYSIZE(_timers))
		_timers[timerNum - 1] = value;
}

void MadeEngine::resetTimer(int16 timerNum) {
	if (timerNum > 0 && timerNum <= ARRAYSIZE(_timers))
		_timers[timerNum - 1] = getTicks();
}

int16 MadeEngine::allocTimer() {
	for (int i = 0; i < ARRAYSIZE(_timers); i++) {
		if (_timers[i] == -1) {
			_timers[i] = getTicks();
			return i + 1;
		}
	}
	return 0;
}

void MadeEngine::freeTimer(int16 timerNum) {
	if (timerNum > 0 && timerNum <= ARRAYSIZE(_timers))
		_timers[timerNum - 1] = -1;
}

void MadeEngine::resetAllTimers() {
	for (int i = 0; i < ARRAYSIZE(_timers); i++)
		_timers[i] = -1;
}

Common::String MadeEngine::getSavegameFilename(int16 saveNum) {
	char filename[256];
	snprintf(filename, 256, "%s.%03d", getTargetName().c_str(), saveNum);
	return filename;
}

void MadeEngine::handleEvents() {

	Common::Event event;
	Common::EventManager *eventMan = _system->getEventManager();

	// NOTE: Don't reset _eventNum to 0 here or no events will get through to the scripts.

	while (eventMan->pollEvent(event)) {
		switch (event.type) {

		case Common::EVENT_MOUSEMOVE:
			_eventMouseX = event.mouse.x;
			_eventMouseY = event.mouse.y;
			break;

		case Common::EVENT_LBUTTONDOWN:
			_eventNum = 2;
			break;

		case Common::EVENT_LBUTTONUP:
			_eventNum = 1;
			break;

		case Common::EVENT_RBUTTONDOWN:
			_eventNum = 4;
			break;

		case Common::EVENT_RBUTTONUP:
			_eventNum = 3;
			break;

		case Common::EVENT_KEYDOWN:
			_eventKey = event.kbd.ascii;
			// For unknown reasons, the game accepts ASCII code
			// 9 as backspace
			if (_eventKey == Common::KEYCODE_BACKSPACE)
				_eventKey = 9;
			_eventNum = 5;
			break;

		default:
			break;

		}
	}

	AudioCD.updateCD();

}

Common::Error MadeEngine::run() {

	// Initialize backend
	initGraphics(320, 200, false);

	resetAllTimers();

	if (getGameID() == GID_RTZ) {
		if (getFeatures() & GF_DEMO) {
			_dat->open("demo.dat");
			_res->open("demo.prj");
		} else if (getFeatures() & GF_CD) {
			_dat->open("rtzcd.dat");
			_res->open("rtzcd.prj");
		} else if (getFeatures() & GF_CD_COMPRESSED) {
			_dat->openFromRed("rtzcd.red", "rtzcd.dat");
			_res->open("rtzcd.prj");
		} else if (getFeatures() & GF_FLOPPY) {
			_dat->open("rtz.dat");
			_res->open("rtz.prj");
		} else {
			error("Unknown RTZ game features");
		}
	} else if (getGameID() == GID_MANHOLE) {
		_dat->open("manhole.dat");

		if (getVersion() == 2) {
			_res->open("manhole.prj");
		} else {
			_res->openResourceBlocks();
		}
	} else if (getGameID() == GID_LGOP2) {
		_dat->open("lgop2.dat");
		_res->open("lgop2.prj");
	} else if (getGameID() == GID_RODNEY) {
		_dat->open("rodneys.dat");
		_res->open("rodneys.prj");
	} else {
		error ("Unknown MADE game");
	}

	if ((getFeatures() & GF_CD) || (getFeatures() & GF_CD_COMPRESSED))
		checkCD();

	_autoStopSound = false;
	_eventNum = _eventKey = _eventMouseX = _eventMouseY = 0;

#ifdef DUMP_SCRIPTS
	_script->dumpAllScripts();
#else
	_screen->setDefaultMouseCursor();
	_script->runScript(_dat->getMainCodeObjectIndex());
#endif

	return Common::kNoError;
}

} // End of namespace Made
