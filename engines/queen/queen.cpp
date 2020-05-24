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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "base/plugins.h"

#include "common/config-manager.h"
#include "common/events.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "engines/util.h"

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
#include "queen/resource.h"
#include "queen/sound.h"
#include "queen/talk.h"
#include "queen/walk.h"

namespace Queen {

QueenEngine::QueenEngine(OSystem *syst)
	: Engine(syst), _bam(nullptr), _bankMan(nullptr), _command(nullptr), _debugger(nullptr),
	_display(nullptr), _graphics(nullptr), _grid(nullptr), _input(nullptr), _logic(nullptr),
	_sound(nullptr), _resource(nullptr), _walk(nullptr), _gameStarted(false),
	randomizer("queen") {
}

QueenEngine::~QueenEngine() {
	delete _bam;
	delete _resource;
	delete _bankMan;
	delete _command;
	delete _display;
	delete _graphics;
	delete _grid;
	delete _input;
	delete _logic;
	delete _sound;
	delete _walk;
	//_debugger is deleted by Engine
}

void QueenEngine::registerDefaultSettings() {
	ConfMan.registerDefault("talkspeed", Logic::DEFAULT_TALK_SPEED);
	ConfMan.registerDefault("subtitles", true);
	_subtitles = true;
}

void QueenEngine::checkOptionSettings() {
	// check talkspeed value
	if (_talkSpeed < MIN_TEXT_SPEED) {
		_talkSpeed = MIN_TEXT_SPEED;
	} else if (_talkSpeed > MAX_TEXT_SPEED) {
		_talkSpeed = MAX_TEXT_SPEED;
	}

	// demo and interview versions don't have speech at all
	if (_sound->speechOn() && (_resource->isDemo() || _resource->isInterview())) {
		_sound->speechToggle(false);
	}

	// ensure text is always on when voice is off
	if (!_sound->speechOn()) {
		_subtitles = true;
	}
}

void QueenEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	readOptionSettings();
}

void QueenEngine::readOptionSettings() {
	bool mute = false;
	if (ConfMan.hasKey("mute"))
		mute = ConfMan.getBool("mute");

	_sound->setVolume(ConfMan.getInt("music_volume"));
	_sound->musicToggle(!(mute || ConfMan.getBool("music_mute")));
	_sound->sfxToggle(!(mute || ConfMan.getBool("sfx_mute")));
	_sound->speechToggle(!(mute || ConfMan.getBool("speech_mute")));
	_talkSpeed = (ConfMan.getInt("talkspeed") * (MAX_TEXT_SPEED - MIN_TEXT_SPEED) + 255 / 2) / 255 + MIN_TEXT_SPEED;
	_subtitles = ConfMan.getBool("subtitles");
	checkOptionSettings();
}

void QueenEngine::writeOptionSettings() {
	ConfMan.setInt("music_volume", _sound->getVolume());
	ConfMan.setBool("music_mute", !_sound->musicOn());
	ConfMan.setBool("sfx_mute", !_sound->sfxOn());
	ConfMan.setInt("talkspeed", ((_talkSpeed - MIN_TEXT_SPEED) * 255 + (MAX_TEXT_SPEED - MIN_TEXT_SPEED) / 2) / (MAX_TEXT_SPEED - MIN_TEXT_SPEED));
	ConfMan.setBool("speech_mute", !_sound->speechOn());
	ConfMan.setBool("subtitles", _subtitles);
	ConfMan.flushToDisk();
}

void QueenEngine::update(bool checkPlayerInput) {
	_graphics->update(_logic->currentRoom());
	_logic->update();

	int frameDelay = (_lastUpdateTime + Input::DELAY_NORMAL - _system->getMillis());
	if (frameDelay <= 0) {
		frameDelay = 1;
	}
	_input->delay(frameDelay);
	_lastUpdateTime = _system->getMillis();

	if (!_resource->isInterview()) {
		_display->palCustomScroll(_logic->currentRoom());
	}
	BobSlot *joe = _graphics->bob(0);
	_display->update(joe->active, joe->x, joe->y);

	_input->checkKeys();

	if (canLoadOrSave()) {
		if (_input->quickSave()) {
			_input->quickSaveReset();
			saveGameState(SLOT_QUICKSAVE, "Quicksave");
		}
		if (_input->quickLoad()) {
			_input->quickLoadReset();
			loadGameState(SLOT_QUICKSAVE);
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
	_sound->updateMusic();
}

bool QueenEngine::canLoadOrSave() const {
	return !_input->cutawayRunning() && !(_resource->isDemo() || _resource->isInterview()) && _gameStarted;
}

bool QueenEngine::canLoadGameStateCurrently() {
	return canLoadOrSave();
}

bool QueenEngine::canSaveGameStateCurrently() {
	return canLoadOrSave();
}

Common::Error QueenEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	debug(3, "Saving game to slot %d", slot);
	char name[20];
	Common::Error err = Common::kNoError;
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
		file->writeUint32BE('SCVM');
		file->writeUint32BE(SAVESTATE_CUR_VER);
		file->writeUint32BE(0);
		file->writeUint32BE(dataSize);
		char description[32];
		Common::strlcpy(description, desc.c_str(), sizeof(description));
		file->write(description, sizeof(description));

		// write save data
		file->write(saveData, dataSize);
		file->finalize();

		// check for errors
		if (file->err()) {
			warning("Can't write file '%s'. (Disk full?)", name);
			err = Common::kWritingFailed;
		}
		delete[] saveData;
		delete file;
	} else {
		warning("Can't create file '%s', game not saved", name);
		err = Common::kCreatingFileFailed;
	}

	return err;
}

Common::Error QueenEngine::loadGameState(int slot) {
	debug(3, "Loading game from slot %d", slot);
	Common::Error err = Common::kNoError;
	GameStateHeader header;
	Common::InSaveFile *file = readGameStateHeader(slot, &header);
	if (file && header.dataSize != 0) {
		byte *saveData = new byte[header.dataSize];
		byte *p = saveData;
		if (file->read(saveData, header.dataSize) != header.dataSize) {
			warning("Error reading savegame file");
			err = Common::kReadingFailed;
		} else {
			_bam->loadState(header.version, p);
			_grid->loadState(header.version, p);
			_logic->loadState(header.version, p);
			_sound->loadState(header.version, p);
			if (header.dataSize != (uint32)(p - saveData)) {
				warning("Corrupted savegame file");
				err = Common::kReadingFailed;	// FIXME
			} else {
				_logic->setupRestoredGame();
			}
		}
		delete[] saveData;
		delete file;
	} else {
		err = Common::kReadingFailed;
	}

	return err;
}

Common::InSaveFile *QueenEngine::readGameStateHeader(int slot, GameStateHeader *gsh) {
	char name[20];
	makeGameStateName(slot, name);
	Common::InSaveFile *file = _saveFileMan->openForLoading(name);
	if (file && file->readUint32BE() == MKTAG('S','C','V','M')) {
		gsh->version = file->readUint32BE();
		gsh->flags = file->readUint32BE();
		gsh->dataSize = file->readUint32BE();
		file->read(gsh->description, sizeof(gsh->description));
	} else {
		memset(gsh, 0, sizeof(GameStateHeader));
	}
	return file;
}

Common::String QueenEngine::getSaveStateName(int slot) const {
	if (slot == SLOT_LISTPREFIX) {
		return "queen.s??";
	} else if (slot == SLOT_AUTOSAVE) {
		slot = getAutosaveSlot();
	}

	assert(slot >= 0);
	return Common::String::format("queen.s%02d", slot);
}

void QueenEngine::makeGameStateName(int slot, char *buf) const {
	Common::String name = getSaveStateName(slot);
	strcpy(buf, name.c_str());
}

int QueenEngine::getGameStateSlot(const char *filename) const {
	int i = -1;
	const char *slot = strrchr(filename, '.');
	if (slot && (slot[1] == 's' || slot[1] == 'S')) {
		i = atoi(slot + 2);
	}
	return i;
}

void QueenEngine::findGameStateDescriptions(char descriptions[100][32]) {
	char prefix[20];
	makeGameStateName(SLOT_LISTPREFIX, prefix);
	Common::StringArray filenames = _saveFileMan->listSavefiles(prefix);
	for (Common::StringArray::const_iterator it = filenames.begin(); it != filenames.end(); ++it) {
		int i = getGameStateSlot(it->c_str());
		if (i >= 0 && i < SAVESTATE_MAX_NUM) {
			GameStateHeader header;
			Common::InSaveFile *f = readGameStateHeader(i, &header);
			strcpy(descriptions[i], header.description);
			delete f;
		}
	}
}

bool Queen::QueenEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime) ||
		(f == kSupportsSubtitleOptions);
}

Common::Error QueenEngine::run() {
	initGraphics(GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT);

	_resource = new Resource();

	_bam = new BamScene(this);
	_bankMan = new BankManager(_resource);
	_command = new Command(this);
	_debugger = new Debugger(this);
	setDebugger(_debugger);
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

	_sound = Sound::makeSoundInstance(_mixer, this, _resource->getCompression());

	_walk = new Walk(this);
	//_talkspeedScale = (MAX_TEXT_SPEED - MIN_TEXT_SPEED) / 255.0;

	registerDefaultSettings();

	// Setup mixer
	syncSoundSettings();

	_logic->start();
	_gameStarted = true;
	if (ConfMan.hasKey("save_slot") && canLoadOrSave()) {
		loadGameState(ConfMan.getInt("save_slot"));
	}
	_lastUpdateTime = _system->getMillis();

	while (!shouldQuit()) {
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

	return Common::kNoError;
}

} // End of namespace Queen
