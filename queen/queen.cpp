/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2004 The ScummVM project
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

#include "backends/fs/fs.h"

#include "base/gameDetector.h"
#include "base/plugins.h"

#include "common/config-manager.h"
#include "common/file.h"
#include "common/timer.h"

#include "queen/queen.h"
#include "queen/bankman.h"
#include "queen/command.h"
#include "queen/cutaway.h"
#include "queen/debug.h"
#include "queen/display.h"
#include "queen/graphics.h"
#include "queen/grid.h"
#include "queen/input.h"
#include "queen/logic.h"
#include "queen/music.h"
#include "queen/resource.h"
#include "queen/sound.h"
#include "queen/talk.h"
#include "queen/walk.h"

#include "sound/mididrv.h"

#ifdef _WIN32_WCE

extern bool toolbar_drawn;
extern bool draw_keyboard;

#endif

/* Flight of the Amazon Queen */
static const GameSettings queen_setting =
	{ "queen", "Flight of the Amazon Queen", 0 };

GameList Engine_QUEEN_gameList() {
	GameList games;
	games.push_back(queen_setting);
	return games;
}

DetectedGameList Engine_QUEEN_detectGames(const FSList &fslist) {
	DetectedGameList detectedGames;

	// Iterate over all files in the given directory
	for (FSList::ConstIterator file = fslist.begin(); file != fslist.end(); ++file) {
		const char *gameName = file->displayName().c_str();

		if (0 == scumm_stricmp("queen.1", gameName) || 0 == scumm_stricmp("queen.1c", gameName)) {
			// Match found, add to list of candidates, then abort loop.
			detectedGames.push_back(queen_setting);
			break;
		}
	}
	return detectedGames;
}

Engine *Engine_QUEEN_create(GameDetector *detector, OSystem *syst) {
	return new Queen::QueenEngine(detector, syst);
}

REGISTER_PLUGIN("Flight of the Amazon Queen", Engine_QUEEN_gameList, Engine_QUEEN_create, Engine_QUEEN_detectGames)

namespace Queen {

QueenEngine::QueenEngine(GameDetector *detector, OSystem *syst)
	: Engine(syst) {

	if (!_mixer->isReady())
		warning("Sound initialisation failed.");

	_mixer->setVolume(ConfMan.getInt("sfx_volume"));

	_system->init_size(GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT);
}

QueenEngine::~QueenEngine() {
	_timer->removeTimerProc(&timerHandler);
	delete _bam;
	delete _resource;
	delete _bankMan;
	delete _command;
	delete _debugger;
	delete _display;
	delete _graphics;
	delete _grid;
	delete _input;
	delete _logic;
	delete _music;
	delete _sound;
	delete _walk;	
}

void QueenEngine::registerDefaultSettings() {
	ConfMan.registerDefault("master_volume", 255);
	ConfMan.registerDefault("music_mute", false);
	ConfMan.registerDefault("sfx_mute", false);
	ConfMan.registerDefault("talkspeed", Logic::DEFAULT_TALK_SPEED);
	ConfMan.registerDefault("speech_mute", _resource->isFloppy());
	ConfMan.registerDefault("subtitles", true);
}

void QueenEngine::checkOptionSettings() {
	// check talkspeed value
	if (_talkSpeed < 4) {
		_talkSpeed = 4;
	} else if (_talkSpeed > 95) {
		_talkSpeed = 100;
	}

	// XXX check master_volume value

	// only CD-ROM version has speech
	if (_resource->isFloppy() && _sound->speechOn()) {
		_sound->speechToggle(false);
	}

	// ensure text is always on when voice is off
	if (!_sound->speechOn()) {
		_subtitles = true;
	}
}

void QueenEngine::readOptionSettings() {
	// XXX master_volume
	_sound->musicToggle(!ConfMan.getBool("music_mute"));
	_sound->sfxToggle(!ConfMan.getBool("sfx_mute"));
	_talkSpeed = ConfMan.getInt("talkspeed");
	_sound->speechToggle(!ConfMan.getBool("speech_mute"));
	_subtitles = ConfMan.getBool("subtitles");
	checkOptionSettings();
}

void QueenEngine::writeOptionSettings() {
	// XXX master_volume
	ConfMan.set("music_mute", !_sound->musicOn());
	ConfMan.set("sfx_mute", !_sound->sfxOn());
	ConfMan.set("talkspeed", _talkSpeed);
	ConfMan.set("speech_mute", !_sound->speechOn());
	ConfMan.set("subtitles", _subtitles);
	ConfMan.flushToDisk();
}

void QueenEngine::update(bool checkPlayerInput) {
	if (_debugger->isAttached()) {
		_debugger->onFrame();
	}

	_graphics->update(_logic->currentRoom());
	_logic->update();

	_input->delay();

	if (!_resource->isInterview()) {
		_display->palCustomScroll(_logic->currentRoom());
	}
	BobSlot *joe = _graphics->bob(0);
	_display->update(joe->active, joe->x, joe->y);
	
	_input->checkKeys();
	if (_input->debugger()) {
		_input->debuggerReset();
		_debugger->attach();
	}
	if (!_input->cutawayRunning()) {
		if (_input->quickSave()) {
			_input->quickSaveReset();
			_logic->gameSave(0, "Quicksave");
		}
		if (_input->quickLoad()) {
			_input->quickLoadReset();
			_logic->gameLoad(0);
		}
		if (checkPlayerInput) {
			_command->updatePlayer();
		}
		if (_input->idleTime() >= Input::DELAY_SCREEN_BLANKER) {
			_display->blankScreen();
		}
	}
}

void QueenEngine::errorString(const char *buf1, char *buf2) {
	strcpy(buf2, buf1);
}

void QueenEngine::go() {
	initialise();

	registerDefaultSettings();
	readOptionSettings();

	_logic->oldRoom(0);
	_logic->newRoom(_logic->currentRoom());

	for (;;) {
		// queen.c lines 4080-4104
		if (_logic->newRoom() > 0) {
			_display->clearTexts(151, 151);
			_logic->update();
			_logic->oldRoom(_logic->currentRoom());
			_logic->currentRoom(_logic->newRoom());
			_logic->changeRoom();
			_display->fullscreen(false);
			if (_logic->currentRoom() == _logic->newRoom()) {
				_logic->newRoom(0);
			}
		} else {
			if (_logic->joeWalk() == JWM_EXECUTE) {
				_logic->joeWalk(JWM_NORMAL);
				_command->executeCurrentAction();
			} else {
				_logic->joeWalk(JWM_NORMAL);
				update(true);
			}
		}
	}
}

void QueenEngine::initialise(void) {
	_bam = new BamScene(this);
	_resource = new Resource(_gameDataPath, _system->get_savefile_manager(), getSavePath());
	_bankMan = new BankManager(_resource);
	_command = new Command(this);
	_debugger = new Debugger(this);
	_display = new Display(this, _system);
	_graphics = new Graphics(this);
	_grid = new Grid(this);
	_input = new Input(_resource->getLanguage(), _system);

	if (_resource->isDemo()) {
		_logic = new LogicDemo(this);
	} else if (_resource->isInterview()) {
		_logic = new LogicInterview(this);
	} else {
		_logic = new LogicGame(this);
	}

	MidiDriver *driver = GameDetector::createMidi(GameDetector::detectMusicDriver(MDT_NATIVE | MDT_ADLIB | MDT_PREFER_NATIVE));
	if (!driver)
		driver = MidiDriver_ADLIB_create(_mixer);
	
	_music = new Music(driver, this);
	_sound = Sound::giveSound(_mixer, this, _resource->compression());
	_walk = new Walk(this);
	_timer->installTimerProc(&timerHandler, 1000000 / 50, this); //call 50 times per second
}

void QueenEngine::timerHandler(void *ptr) {
	((QueenEngine *)ptr)->gotTimerTick();
}

void QueenEngine::gotTimerTick() {
	_display->handleTimer();
}

} // End of namespace Queen
