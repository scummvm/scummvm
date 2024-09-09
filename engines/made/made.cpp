/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "made/made.h"
#include "made/console.h"
#include "made/pmvplayer.h"
#include "made/resource.h"
#include "made/screen.h"
#include "made/database.h"
#include "made/script.h"
#include "made/music.h"

#include "common/config-manager.h"
#include "common/events.h"
#include "common/system.h"
#include "common/error.h"

#include "engines/util.h"

#include "backends/audiocd/audiocd.h"

namespace Made {

MadeEngine::MadeEngine(OSystem *syst, const MadeGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {

	_eventNum = 0;
	_eventMouseX = _eventMouseY = 0;
	_eventKey = 0;
	_autoStopSound = false;
	_soundEnergyIndex = 0;
	_soundEnergyArray = nullptr;
	_musicBeatStart = 0;
	_cdTimeStart = 0;
	_introMusicDigital = true;
	if (ConfMan.hasKey("intro_music_digital"))
		_introMusicDigital = ConfMan.getBool("intro_music_digital");

	_rnd = new Common::RandomSource("made");

	setDebugger(new MadeConsole(this));

	_system->getAudioCDManager()->open();

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

	_music = nullptr;

	_soundRate = 0;

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
		// Return to Zork sets it itself via a script function
		break;
	default:
		break;
	}
}

MadeEngine::~MadeEngine() {
	_system->getAudioCDManager()->stop();

	delete _rnd;
	delete _pmvPlayer;
	delete _res;
	delete _screen;
	delete _dat;
	delete _script;
	delete _music;
}

void MadeEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	_music->syncSoundSettings();
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
	return Common::String::format("%s.%03d", getTargetName().c_str(), saveNum);
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
			// Handle any special keys here
			// Supported keys taken from https://web.archive.org/web/20141114142447/http://www.allgame.com/game.php?id=13542&tab=controls
			if (event.kbd.keycode == Common::KEYCODE_BACKSPACE) {
				_eventNum = 5;
				_eventKey = 9;
			} else {
				_eventNum = 5;
				_eventKey = event.kbd.ascii;
			}
			break;
		case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
			switch (event.customType) {
			case kActionCursorUp:
				_eventMouseY = MAX<int16>(0, _eventMouseY - 1);
				g_system->warpMouse(_eventMouseX, _eventMouseY);
				break;
			case kActionCursorDown:
				_eventMouseY = MIN<int16>(199, _eventMouseY + 1);
				g_system->warpMouse(_eventMouseX, _eventMouseY);
				break;
			case kActionCursorLeft:
				_eventMouseX = MAX<int16>(0, _eventMouseX - 1);
				g_system->warpMouse(_eventMouseX, _eventMouseY);
				break;
			case kActionCursorRight:
				_eventMouseX = MIN<int16>(319, _eventMouseX + 1);
				g_system->warpMouse(_eventMouseX, _eventMouseY);
				break;
			case kActionMenu:
				_eventNum = 5;
				_eventKey = 21; //KEYCODE F1
				break;
			case kActionSaveGame:
				_eventNum = 5;
				_eventKey = 22; //KEYCODE F2
				break;
			case kActionLoadGame:
				_eventNum = 5;
				_eventKey = 23; //KEYCODE F3
				break;
			case kActionRepeatMessage:
				_eventNum = 5;
				_eventKey = 24; //KEYCODE F4
				break;
			default:
				break;
			}
			break;
		default:
			break;

		}
	}

	_system->getAudioCDManager()->update();

}

Common::Error MadeEngine::run() {
	_music = new MusicPlayer(this, getGameID() == GID_RTZ);
	syncSoundSettings();

	// Initialize backend
	initGraphics(320, 200);

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

	if ((getFeatures() & GF_CD) || (getFeatures() & GF_CD_COMPRESSED)) {
		if (!existExtractedCDAudioFiles()
		    && !isDataAndCDAudioReadFromSameCD()) {
			warnMissingExtractedCDAudio();
		}
	}

	_autoStopSound = false;
	_eventNum = _eventKey = _eventMouseX = _eventMouseY = 0;

#ifdef DUMP_SCRIPTS
	_script->dumpAllScripts();
#else
	_screen->setDefaultMouseCursor();
	_script->runScript(_dat->getMainCodeObjectIndex());
#endif

	_music->close();

	return Common::kNoError;
}

void MadeEngine::pauseEngineIntern(bool pause) {
	Engine::pauseEngineIntern(pause);

	if (pause) {
		if (_music)
			_music->pause();
	} else {
		if (_music)
			_music->resume();
	}
}

} // End of namespace Made
