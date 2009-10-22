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

#include "common/scummsys.h"

#include "common/config-manager.h"
#include "common/events.h"
#include "common/file.h"
#include "common/keyboard.h"
#include "common/EventRecorder.h"

#include "graphics/cursorman.h"
#include "graphics/font.h"

#include "draci/draci.h"
#include "draci/barchive.h"
#include "draci/script.h"
#include "draci/font.h"
#include "draci/sprite.h"
#include "draci/screen.h"
#include "draci/mouse.h"
#include "draci/saveload.h"

namespace Draci {

// Data file paths

const char *objectsPath = "OBJEKTY.DFW";
const char *palettePath = "PALETY.DFW";
const char *spritesPath = "OBR_AN.DFW";
const char *overlaysPath = "OBR_MAS.DFW";
const char *roomsPath = "MIST.DFW";
const char *animationsPath = "ANIM.DFW";
const char *iconsPath = "HRA.DFW";
const char *walkingMapsPath = "MAPY.DFW";
const char *itemsPath = "IKONY.DFW";
const char *itemImagesPath = "OBR_IK.DFW";
const char *initPath = "INIT.DFW";
const char *stringsPath = "RETEZCE.DFW";
const char *soundsPath = "CD2.SAM";
const char *dubbingPath = "CD.SAM";
const char *musicPathMask = "HUDBA%d.MID";

const uint kSoundsFrequency = 13000;
const uint kDubbingFrequency = 22000;

DraciEngine::DraciEngine(OSystem *syst, const ADGameDescription *gameDesc)
 : Engine(syst) {
	// Put your engine in a sane state, but do nothing big yet;
	// in particular, do not load data from files; rather, if you
	// need to do such things, do them from init().

	// Do not initialize graphics here

	// However this is the place to specify all default directories
	//Common::File::addDefaultDirectory(_gameDataPath + "sound/");

	// Here is the right place to set up the engine specific debug levels
	Common::addDebugChannel(kDraciGeneralDebugLevel, "general", "Draci general debug info");
	Common::addDebugChannel(kDraciBytecodeDebugLevel, "bytecode", "GPL bytecode instructions");
	Common::addDebugChannel(kDraciArchiverDebugLevel, "archiver", "BAR archiver debug info");
	Common::addDebugChannel(kDraciLogicDebugLevel, "logic", "Game logic debug info");
	Common::addDebugChannel(kDraciAnimationDebugLevel, "animation", "Animation debug info");
	Common::addDebugChannel(kDraciSoundDebugLevel, "sound", "Sound debug info");

	// Don't forget to register your random source
	g_eventRec.registerRandomSource(_rnd, "draci");
}

bool DraciEngine::hasFeature(EngineFeature f) const {
	return (f == kSupportsSubtitleOptions) ||
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

int DraciEngine::init() {
	// Initialize graphics using following:
	initGraphics(kScreenWidth, kScreenHeight, false);

	// Open game's archives
	_initArchive = new BArchive(initPath);
	_objectsArchive = new BArchive(objectsPath);
	_spritesArchive = new BArchive(spritesPath);
	_paletteArchive = new BArchive(palettePath);
	_roomsArchive = new BArchive(roomsPath);
	_overlaysArchive = new BArchive(overlaysPath);
	_animationsArchive = new BArchive(animationsPath);
	_iconsArchive = new BArchive(iconsPath);
	_walkingMapsArchive = new BArchive(walkingMapsPath);
	_itemsArchive = new BArchive(itemsPath);
	_itemImagesArchive = new BArchive(itemImagesPath);
	_stringsArchive = new BArchive(stringsPath);

	_soundsArchive = new SoundArchive(soundsPath, kSoundsFrequency);
	_dubbingArchive = new SoundArchive(dubbingPath, kDubbingFrequency);
	_sound = new Sound(_mixer);

	int midiDriver = MidiDriver::detectMusicDriver(MDT_MIDI | MDT_ADLIB | MDT_PREFER_MIDI);
	bool native_mt32 = ((midiDriver == MD_MT32) || ConfMan.getBool("native_mt32"));
	//bool adlib = (midiDriver == MD_ADLIB);

	_midiDriver = MidiDriver::createMidi(midiDriver);
	if (native_mt32)
		_midiDriver->property(MidiDriver::PROP_CHANNEL_MASK, 0x03FE);

	_music = new MusicPlayer(_midiDriver, musicPathMask);
	_music->setNativeMT32(native_mt32);
	//_music->setAdlib(adlib);

	// Load the game's fonts
	_smallFont = new Font(kFontSmall);
	_bigFont = new Font(kFontBig);

	_screen = new Screen(this);
	_anims = new AnimationManager(this);
	_mouse = new Mouse(this);
	_script = new Script(this);
	_game = new Game(this);

	if (!_objectsArchive->isOpen()) {
		debugC(2, kDraciGeneralDebugLevel, "ERROR - Opening objects archive failed");
		return Common::kUnknownError;
	}

	if (!_spritesArchive->isOpen()) {
		debugC(2, kDraciGeneralDebugLevel, "ERROR - Opening sprites archive failed");
		return Common::kUnknownError;
	}

	if (!_paletteArchive->isOpen()) {
		debugC(2, kDraciGeneralDebugLevel, "ERROR - Opening palette archive failed");
		return Common::kUnknownError;
	}

	if (!_roomsArchive->isOpen()) {
		debugC(2, kDraciGeneralDebugLevel, "ERROR - Opening rooms archive failed");
		return Common::kUnknownError;
	}

	if (!_overlaysArchive->isOpen()) {
		debugC(2, kDraciGeneralDebugLevel, "ERROR - Opening overlays archive failed");
		return Common::kUnknownError;
	}

	if (!_animationsArchive->isOpen()) {
		debugC(2, kDraciGeneralDebugLevel, "ERROR - Opening animations archive failed");
		return Common::kUnknownError;
	}

	if (!_iconsArchive->isOpen()) {
		debugC(2, kDraciGeneralDebugLevel, "ERROR - Opening icons archive failed");
		return Common::kUnknownError;
	}

	if (!_walkingMapsArchive->isOpen()) {
		debugC(2, kDraciGeneralDebugLevel, "ERROR - Opening walking maps archive failed");
		return Common::kUnknownError;
	}

	if (!_soundsArchive->isOpen()) {
		debugC(2, kDraciGeneralDebugLevel, "ERROR - Opening sounds archive failed");
		return Common::kUnknownError;
	}

	if (!_dubbingArchive->isOpen()) {
		debugC(2, kDraciGeneralDebugLevel, "WARNING - Opening dubbing archive failed");
	}

	_showWalkingMap = false;

	// Basic archive test
	debugC(2, kDraciGeneralDebugLevel, "Running archive tests...");
	Common::String path("INIT.DFW");
	BArchive ar(path);
	const BAFile *f;
	debugC(3, kDraciGeneralDebugLevel, "Number of file streams in archive: %d", ar.size());

	if (ar.isOpen()) {
		f = ar.getFile(0);
	} else {
		debugC(2, kDraciGeneralDebugLevel, "ERROR - Archive not opened");
		return Common::kUnknownError;
	}

	debugC(3, kDraciGeneralDebugLevel, "First 10 bytes of file %d: ", 0);
	for (uint i = 0; i < 10; ++i) {
		debugC(3, kDraciGeneralDebugLevel, "0x%02x%c", f->_data[i], (i < 9) ? ' ' : '\n');
	}

	return Common::kNoError;
}

void DraciEngine::handleEvents() {
	Common::Event event;

	while (_eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RTL:
			_game->setQuit(true);
			break;
		case Common::EVENT_KEYDOWN:
			switch (event.kbd.keycode) {
			case Common::KEYCODE_RIGHT:
				_game->scheduleEnteringRoomUsingGate(_game->nextRoomNum(), 0);
				break;
			case Common::KEYCODE_LEFT:
				_game->scheduleEnteringRoomUsingGate(_game->prevRoomNum(), 0);
				break;
			case Common::KEYCODE_ESCAPE: {
				const int escRoom = _game->getRoomNum() != _game->getMapRoom()
					? _game->getEscRoom() : _game->getPreviousRoomNum();

				// Check if there is an escape room defined for the current room
				if (escRoom != kNoEscRoom) {

					// Schedule room change
					// TODO: gate 0 is not always the best one for returning from the map
					_game->scheduleEnteringRoomUsingGate(escRoom, 0);
					_game->setExitLoop(true);

					// End any currently running GPL programs
					_script->endCurrentProgram();
				}
				break;
			}
			case Common::KEYCODE_m:
				if (_game->getLoopStatus() == kStatusOrdinary) {
					const int new_room = _game->getRoomNum() != _game->getMapRoom()
						? _game->getMapRoom() : _game->getPreviousRoomNum();
					_game->scheduleEnteringRoomUsingGate(new_room, 0);
				}
				break;
			case Common::KEYCODE_w:
				// Show walking map toggle
				_showWalkingMap = !_showWalkingMap;
				break;
			case Common::KEYCODE_i:
				if (_game->getLoopStatus() == kStatusInventory &&
				   _game->getLoopSubstatus() == kSubstatusOrdinary) {
					_game->inventoryDone();
				} else if (_game->getLoopStatus() == kStatusOrdinary &&
				   _game->getLoopSubstatus() == kSubstatusOrdinary) {
					_game->inventoryInit();
				}
				break;
			default:
				break;
			}
			break;
		default:
			_mouse->handleEvent(event);
		}
	}

	// Show walking map overlay
	// If the walking map overlay is already in the wanted state don't
	// start / stop it constantly
	if (_showWalkingMap && !_anims->getAnimation(kWalkingMapOverlay)->isPlaying()) {
		_anims->play(kWalkingMapOverlay);
	} else if (!_showWalkingMap && _anims->getAnimation(kWalkingMapOverlay)->isPlaying()) {
		_anims->stop(kWalkingMapOverlay);
	}
}

DraciEngine::~DraciEngine() {
	// Dispose your resources here

 	// TODO: Investigate possibility of using sharedPtr or similar

	delete _smallFont;
	delete _bigFont;

	delete _mouse;
	delete _script;
	delete _anims;
	delete _game;
	delete _screen;

	delete _initArchive;
	delete _paletteArchive;
	delete _objectsArchive;
	delete _spritesArchive;
	delete _roomsArchive;
	delete _overlaysArchive;
	delete _animationsArchive;
	delete _iconsArchive;
	delete _walkingMapsArchive;
	delete _itemsArchive;
	delete _itemImagesArchive;
	delete _stringsArchive;

	delete _soundsArchive;
	delete _dubbingArchive;
	delete _sound;
	delete _music;
	delete _midiDriver;

	// Remove all of our debug levels here
	Common::clearAllDebugChannels();
}

Common::Error DraciEngine::run() {
	init();
	_engineStartTime = _system->getMillis() / 1000;
	_game->init();

	// Load game from specified slot, if any
	if (ConfMan.hasKey("save_slot")) {
		loadGameState(ConfMan.getInt("save_slot"));
	}

	_game->start();
	return Common::kNoError;
}

void DraciEngine::pauseEngineIntern(bool pause) {
	Engine::pauseEngineIntern(pause);
	if (pause) {
		// Record start of the pause, so that we can later
		// adjust _engineStartTime accordingly.
		_pauseStartTime = _system->getMillis();

		_anims->pauseAnimations();
		_sound->pauseSound();
		_sound->pauseVoice();
		_music->pause();
	} else {
		_anims->unpauseAnimations();
		_sound->resumeSound();
		_sound->resumeVoice();
		_music->resume();

		// Adjust engine start time
		const int delta = _system->getMillis() - _pauseStartTime;
		_engineStartTime += delta / 1000;
		_game->shiftSpeechTick(delta);
		_pauseStartTime = 0;
	}
}

void DraciEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	_sound->setVolume();
	_music->syncVolume();
}

const char *DraciEngine::getSavegameFile(int saveGameIdx) {
	static char buffer[20];
	sprintf(buffer, "draci.s%02d", saveGameIdx);
	return buffer;
}

Common::Error DraciEngine::loadGameState(int slot) {
	// When called from run() using save_slot, the next operation is the
	// call to start() calling enterNewRoom().
	// When called from handleEvents() in the middle of the game, the next
	// operation after handleEvents() exits from loop(), and returns to
	// start() to the same place as above.
	// In both cases, we are safe to override the data structures right
	// here are now, without waiting for any other code to finish, thanks
	// to our constraint in canLoadGameStateCurrently() and to having
	// enterNewRoom() called right after we exit from here.
	//
	// TODO: Handle saving in the map room.  Verify inventory and fix
	// dialogs.
	return loadSavegameData(slot, this);
}

bool DraciEngine::canLoadGameStateCurrently() {
	return (_game->getLoopStatus() == kStatusOrdinary) &&
		(_game->getLoopSubstatus() == kSubstatusOrdinary);
}

Common::Error DraciEngine::saveGameState(int slot, const char *desc) {
	return saveSavegameData(slot, desc, *this);
}

bool DraciEngine::canSaveGameStateCurrently() {
	return (_game->getLoopStatus() == kStatusOrdinary) &&
		(_game->getLoopSubstatus() == kSubstatusOrdinary);
}

} // End of namespace Draci
