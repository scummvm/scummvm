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
	for (FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
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

	_system->initSize(GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT);
}

QueenEngine::~QueenEngine() {
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
	delete _saveFileMan;
}

void QueenEngine::registerDefaultSettings() {
	ConfMan.registerDefault("master_volume", 255);
	ConfMan.registerDefault("music_mute", false);
	ConfMan.registerDefault("sfx_mute", false);
	ConfMan.registerDefault("talkspeed", Logic::DEFAULT_TALK_SPEED);
	ConfMan.registerDefault("speech_mute", (_resource->isDemo() || _resource->isInterview()));
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
			saveGameState(0, "Quicksave");
		}
		if (_input->quickLoad()) {
			_input->quickLoadReset();
			loadGameState(0);
		}
		if (checkPlayerInput) {
			_command->updatePlayer();
		}
		if (_input->idleTime() >= Input::DELAY_SCREEN_BLANKER) {
			_display->blankScreen();
		}
	}
}

void QueenEngine::saveGameState(uint16 slot, const char *desc) {
	debug(3, "Saving game to slot %d", slot);
	char name[20];
	makeGameStateName(slot, name);
	SaveFile *file = _saveFileMan->open_savefile(name, getSavePath(), true);
	if (file) {
		// save data
		byte *saveData = new byte[30000];
		byte *p = saveData;
		_bam->saveState(p);
		_grid->saveState(p);
		_logic->saveState(p);
		_sound->saveState(p);
		uint32 dataSize = p - saveData;

		// write header
		GameStateHeader header;
		memset(&header, 0, sizeof(header));
		file->writeUint32BE('SCVM');
		header.version = TO_BE_32(SAVESTATE_CUR_VER);
		header.flags = TO_BE_32(0);
		header.dataSize = TO_BE_32(dataSize);
		strncpy(header.description, desc, sizeof(header.description) - 1);
		file->write(&header, sizeof(header));

		// write save data
		if (file->write(saveData, dataSize) != dataSize) {
			warning("Can't write file '%s'. (Disk full?)", name);
		}
		delete saveData;
		delete file;
	} else {
		warning("Can't create file '%s', game not saved", name);
	}
}

void QueenEngine::loadGameState(uint16 slot) {
	debug(3, "Loading game from slot %d", slot);
	GameStateHeader header;
	SaveFile *file = readGameStateHeader(slot, &header);
	if (file && header.dataSize != 0) {
		byte *saveData = new byte[header.dataSize];
		byte *p = saveData;
		if (file->read(saveData, header.dataSize) != header.dataSize) {
			warning("Error reading savegame file");
		} else {
			_bam->loadState(header.version, p);
			_grid->loadState(header.version, p);
			_logic->loadState(header.version, p);
			_sound->loadState(header.version, p);
			assert(header.dataSize == (uint32)(p - saveData));
			_logic->setupRestoredGame();
		}
		delete[] saveData;
		delete file;
	}
}

SaveFile *QueenEngine::readGameStateHeader(uint16 slot, GameStateHeader *gsh) {
	char name[20];
	makeGameStateName(slot, name);
	SaveFile *file = _saveFileMan->open_savefile(name, getSavePath(), false);
	if (file && file->readUint32BE() == 'SCVM') {
		gsh->version = file->readUint32BE();
		gsh->flags = file->readUint32BE();
		gsh->dataSize = file->readUint32BE();
		file->read(gsh->description, sizeof(gsh->description));
	} else {
		memset(gsh, 0, sizeof(GameStateHeader));
	}
	return file;
}

void QueenEngine::makeGameStateName(uint16 slot, char *buf) {
	sprintf(buf, "queen.s%02d", slot);
}

void QueenEngine::findGameStateDescriptions(char descriptions[100][32]) {
	char filename[20];
	makeGameStateName(0, filename);
	filename[strlen(filename) - 2] = 0;
	bool marks[SAVESTATE_MAX];
	_saveFileMan->list_savefiles(filename, getSavePath(), marks, SAVESTATE_MAX);
	for (int i = 0; i < SAVESTATE_MAX; ++i) {
		if (marks[i]) {
			GameStateHeader header;
			SaveFile *f = readGameStateHeader(i, &header);
			strcpy(descriptions[i], header.description);
			delete f;
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
	_resource = new Resource(_gameDataPath);
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
	else if (ConfMan.getBool("native_mt32"))
		driver->property(MidiDriver::PROP_CHANNEL_MASK, 0x03FE);
		
	_music = new Music(driver, this);
	_music->hasNativeMT32(ConfMan.getBool("native_mt32"));
	
	_sound = Sound::giveSound(_mixer, this, _resource->compression());
	_walk = new Walk(this);
	_saveFileMan = _system->get_savefile_manager();
}

} // End of namespace Queen
