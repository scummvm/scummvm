/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2006 The ScummVM project
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
 * $Header$
 *
 */

#include "common/stdafx.h"

#include "backends/fs/fs.h"

#include "base/gameDetector.h"
#include "base/plugins.h"

#include "common/config-manager.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/system.h"

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
bool isSmartphone();
#endif

/* Flight of the Amazon Queen */
static const GameSettings queen_setting[] = {
	{ "queen", "Flight of the Amazon Queen", 0 },
	{ "queen", "Flight of the Amazon Queen (Demo)", 0 },
	{ "queen", "Flight of the Amazon Queen (Interview)", 0 },
	{ 0, 0, 0 }
};

GameList Engine_QUEEN_gameList() {
	GameList games;
	const GameSettings *g = queen_setting;

	while (g->gameid) {
		games.push_back(*g);
		g++;
	}
	return games;
}

GameSettings determineTarget(uint32 size) {
	switch (size) {
	case 3724538:	//regular demo
	case 3732177:
		return queen_setting[1];
		break;
	case 1915913:   //interview demo
		return queen_setting[2];
		break;
	default:	//non-demo
		return queen_setting[0];
		break;
	}
	return queen_setting[0];
}

DetectedGameList Engine_QUEEN_detectGames(const FSList &fslist) {
	DetectedGameList detectedGames;

	// Iterate over all files in the given directory
	for (FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (!file->isDirectory()) {
			const char *gameName = file->displayName().c_str();

			if (0 == scumm_stricmp("queen.1", gameName) || 0 == scumm_stricmp("queen.1c", gameName)) {
				Common::File dataFile;
				dataFile.open(file->path().c_str());
				assert(dataFile.isOpen());

				if (0 == scumm_stricmp("queen.1", gameName)) {	//an unmodified file
					detectedGames.push_back(determineTarget(dataFile.size()));
				} else if (0 == scumm_stricmp("queen.1c", gameName)) { //oh joy, it's a rebuilt file
					char header[9];
					dataFile.read(header, 9);
					if (0 == scumm_strnicmp("QTBL", header, 4)) { //check validity
						uint8 version = 0;	//default to full/normal version

						if (0 == scumm_strnicmp("PE100", header + 4, 5)) //One of the 2 regular demos
							version = 1;
						if (0 == scumm_strnicmp("PEint", header + 4, 5)) //Interview demo
							version = 2;

						detectedGames.push_back(queen_setting[version]);
					}
				}

				dataFile.close();
				break;
			}
		}
	}
	return detectedGames;
}

Engine *Engine_QUEEN_create(GameDetector *detector, OSystem *syst) {
	return new Queen::QueenEngine(detector, syst);
}

REGISTER_PLUGIN(QUEEN, "Flight of the Amazon Queen")

namespace Queen {

QueenEngine::QueenEngine(GameDetector *detector, OSystem *syst)
	: Engine(syst), _debugger(0) {
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
}

void QueenEngine::registerDefaultSettings() {
	ConfMan.registerDefault("music_mute", false);
	ConfMan.registerDefault("sfx_mute", false);
	ConfMan.registerDefault("talkspeed", Logic::DEFAULT_TALK_SPEED);
	ConfMan.registerDefault("speech_mute", _resource->isDemo() || _resource->isInterview());
	ConfMan.registerDefault("subtitles", true);
}

void QueenEngine::checkOptionSettings() {
	// check talkspeed value
	if (_talkSpeed < MIN_TEXT_SPEED) {
		_talkSpeed = MIN_TEXT_SPEED;
	} else if (_talkSpeed > MAX_TEXT_SPEED) {
		_talkSpeed = MAX_TEXT_SPEED;
	}

	// ensure text is always on when voice is off
	if (!_sound->speechOn()) {
		_subtitles = true;
	}

	// demo and interview versions don't have speech at all
	if (_sound->speechOn() && (_resource->isDemo() || _resource->isInterview())) {
		_sound->speechToggle(false);
	}
}

void QueenEngine::readOptionSettings() {
	_music->setVolume(ConfMan.getInt("music_volume"));
	_sound->musicToggle(!ConfMan.getBool("music_mute"));
	_sound->sfxToggle(!ConfMan.getBool("sfx_mute"));
	_talkSpeed = ConfMan.getInt("talkspeed");
	_sound->speechToggle(!ConfMan.getBool("speech_mute"));
	_subtitles = ConfMan.getBool("subtitles");
	checkOptionSettings();
}

void QueenEngine::writeOptionSettings() {
	ConfMan.set("music_volume", _music->volume());
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
	if (canLoadOrSave()) {
		if (_input->quickSave()) {
			_input->quickSaveReset();
			saveGameState(0, "Quicksave");
		}
		if (_input->quickLoad()) {
			_input->quickLoadReset();
			loadGameState(0);
		}
		if (shouldPerformAutoSave(_lastSaveTime)) {
			saveGameState(AUTOSAVE_SLOT, "Autosave");
			_lastSaveTime = _system->getMillis();
		}
	}
	if (!_input->cutawayRunning()) {
		if (checkPlayerInput) {
			_command->updatePlayer();
		}
		if (_input->idleTime() >= Input::DELAY_SCREEN_BLANKER) {
			_display->blankScreen();
		}
	}
}

bool QueenEngine::canLoadOrSave() const {
	return !_input->cutawayRunning() && !(_resource->isDemo() || _resource->isInterview());
}

void QueenEngine::saveGameState(uint16 slot, const char *desc) {
	debug(3, "Saving game to slot %d", slot);
	char name[20];
	makeGameStateName(slot, name);
	Common::OutSaveFile *file = _saveFileMan->openForSaving(name);
	if (file) {
		// save data
		byte *saveData = new byte[SAVESTATE_MAX_SIZE];
		byte *p = saveData;
		_bam->saveState(p);
		_grid->saveState(p);
		_logic->saveState(p);
		_sound->saveState(p);
		uint32 dataSize = p - saveData;
		assert(dataSize < SAVESTATE_MAX_SIZE);

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
		file->write(saveData, dataSize);
		file->flush();

		// check for errors
		if (file->ioFailed()) {
			warning("Can't write file '%s'. (Disk full?)", name);
		}
		delete[] saveData;
		delete file;
	} else {
		warning("Can't create file '%s', game not saved", name);
	}
}

void QueenEngine::loadGameState(uint16 slot) {
	debug(3, "Loading game from slot %d", slot);
	GameStateHeader header;
	Common::InSaveFile *file = readGameStateHeader(slot, &header);
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
			if (header.dataSize != (uint32)(p - saveData)) {
				warning("Corrupted savegame file");
			} else {
				_logic->setupRestoredGame();
			}
		}
		delete[] saveData;
		delete file;
	}
}

Common::InSaveFile *QueenEngine::readGameStateHeader(uint16 slot, GameStateHeader *gsh) {
	char name[20];
	makeGameStateName(slot, name);
	Common::InSaveFile *file = _saveFileMan->openForLoading(name);
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
	if (slot == AUTOSAVE_SLOT) {
		strcpy(buf, "queen.asd");
	} else {
		sprintf(buf, "queen.s%02d", slot);
	}
}

void QueenEngine::findGameStateDescriptions(char descriptions[100][32]) {
	char filename[20];
	makeGameStateName(0, filename);
	filename[strlen(filename) - 2] = 0;
	bool marks[SAVESTATE_MAX_NUM];
	_saveFileMan->listSavefiles(filename, marks, SAVESTATE_MAX_NUM);
	for (int i = 0; i < SAVESTATE_MAX_NUM; ++i) {
		if (marks[i]) {
			GameStateHeader header;
			Common::InSaveFile *f = readGameStateHeader(i, &header);
			strcpy(descriptions[i], header.description);
			delete f;
		}
	}
}

void QueenEngine::errorString(const char *buf1, char *buf2) {
	strcpy(buf2, buf1);

#ifdef _WIN32_WCE
	if (isSmartphone())
		return;
#endif

	// Unless an error -originated- within the debugger, spawn the
	// debugger. Otherwise exit out normally.
	if (_debugger && !_debugger->isAttached()) {
		// (Print it again in case debugger segfaults)
		printf("%s\n", buf2);
		_debugger->attach(buf2);
		_debugger->onFrame();
	}
}

int QueenEngine::go() {
	_logic->start();
	if (ConfMan.hasKey("save_slot") && canLoadOrSave()) {
		loadGameState(ConfMan.getInt("save_slot"));
	}
	_lastSaveTime = _system->getMillis();
	_quit = false;
	while (!_quit) {
		if (_logic->newRoom() > 0) {
			_logic->update();
			_logic->oldRoom(_logic->currentRoom());
			_logic->currentRoom(_logic->newRoom());
			_logic->changeRoom();
			_display->fullscreen(false);
			if (_logic->currentRoom() == _logic->newRoom()) {
				_logic->newRoom(0);
			}
		} else if (_logic->joeWalk() == JWM_EXECUTE) {
			_logic->joeWalk(JWM_NORMAL);
			_command->executeCurrentAction();
		} else {
			_logic->joeWalk(JWM_NORMAL);
			update(true);
		}
	}
	return 0;
}

int QueenEngine::init(GameDetector &detector) {
	_system->beginGFXTransaction();
		initCommonGFX(detector);
		_system->initSize(GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT);
	_system->endGFXTransaction();

	_bam = new BamScene(this);
	_resource = new Resource();
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

	if (!_mixer->isReady())
		warning("Sound initialisation failed");
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	// Set mixer music volume to maximum, since music volume is regulated by MusicPlayer's MIDI messages
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, Audio::Mixer::kMaxMixerVolume);

	int midiDriver = MidiDriver::detectMusicDriver(MDT_MIDI | MDT_ADLIB | MDT_PREFER_MIDI);
	bool native_mt32 = ((midiDriver == MD_MT32) || ConfMan.getBool("native_mt32"));

	MidiDriver *driver = MidiDriver::createMidi(midiDriver);
	if (native_mt32)
		driver->property(MidiDriver::PROP_CHANNEL_MASK, 0x03FE);

	_music = new Music(driver, this);
	_music->hasNativeMT32(native_mt32);

	_sound = Sound::giveSound(_mixer, this, _resource->compression());
	_walk = new Walk(this);

	registerDefaultSettings();
	readOptionSettings();

	return 0;
}

} // End of namespace Queen
