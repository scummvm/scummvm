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

#include "graphics/cursorman.h"
#include "graphics/font.h"
 
#include "draci/draci.h"
#include "draci/barchive.h"
#include "draci/script.h"
#include "draci/font.h"
#include "draci/sprite.h"
#include "draci/screen.h"
#include "draci/mouse.h"

namespace Draci {

// Data file paths

const Common::String objectsPath("OBJEKTY.DFW");
const Common::String palettePath("PALETY.DFW");
const Common::String spritesPath("OBR_AN.DFW");
const Common::String overlaysPath("OBR_MAS.DFW");
const Common::String roomsPath("MIST.DFW");
const Common::String animationsPath("ANIM.DFW");
const Common::String iconsPath("HRA.DFW");
const Common::String walkingMapsPath("MAPY.DFW");
const Common::String itemsPath("IKONY.DFW");
const Common::String itemImagesPath("OBR_IK.DFW");
const Common::String initPath("INIT.DFW");
const Common::String stringsPath("RETEZCE.DFW");

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
 
	// Don't forget to register your random source
	_eventMan->registerRandomSource(_rnd, "draci");
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

	// Load the game's fonts
	_smallFont = new Font(kFontSmall);
	_bigFont = new Font(kFontBig);

	_screen = new Screen(this);
	_anims = new AnimationManager(this);
	_mouse = new Mouse(this);
	_script = new Script(this);
	_game = new Game(this);

	if(!_objectsArchive->isOpen()) {
		debugC(2, kDraciGeneralDebugLevel, "ERROR - Opening objects archive failed");
		return Common::kUnknownError;
	}	

	if(!_spritesArchive->isOpen()) {
		debugC(2, kDraciGeneralDebugLevel, "ERROR - Opening sprites archive failed");
		return Common::kUnknownError;
	}	

	if(!_paletteArchive->isOpen()) {
		debugC(2, kDraciGeneralDebugLevel, "ERROR - Opening palette archive failed");
		return Common::kUnknownError;
	}

	if(!_roomsArchive->isOpen()) {
		debugC(2, kDraciGeneralDebugLevel, "ERROR - Opening rooms archive failed");
		return Common::kUnknownError;
	}

	if(!_overlaysArchive->isOpen()) {
		debugC(2, kDraciGeneralDebugLevel, "ERROR - Opening overlays archive failed");
		return Common::kUnknownError;
	}

	if(!_animationsArchive->isOpen()) {
		debugC(2, kDraciGeneralDebugLevel, "ERROR - Opening animations archive failed");
		return Common::kUnknownError;
	}

	if(!_iconsArchive->isOpen()) {
		debugC(2, kDraciGeneralDebugLevel, "ERROR - Opening icons archive failed");
		return Common::kUnknownError;
	}

	if(!_walkingMapsArchive->isOpen()) {
		debugC(2, kDraciGeneralDebugLevel, "ERROR - Opening walking maps archive failed");
		return Common::kUnknownError;
	}

	_showWalkingMap = false;

	// Basic archive test
	debugC(2, kDraciGeneralDebugLevel, "Running archive tests...");	
	Common::String path("INIT.DFW");	
	BArchive ar(path);
	BAFile *f;
	debugC(3, kDraciGeneralDebugLevel, "Number of file streams in archive: %d", ar.size());	
	
	if(ar.isOpen()) {
		f = ar.getFile(0);	
	} else {
		debugC(2, kDraciGeneralDebugLevel, "ERROR - Archive not opened");
		return Common::kUnknownError;
	}	
		
	debugC(3, kDraciGeneralDebugLevel, "First 10 bytes of file %d: ", 0);
	for (unsigned int i = 0; i < 10; ++i) {
		debugC(3, kDraciGeneralDebugLevel, "0x%02x%c", f->_data[i], (i < 9) ? ' ' : '\n');
	}

	return Common::kNoError;
}

int DraciEngine::go() {
	debugC(1, kDraciGeneralDebugLevel, "DraciEngine::go()");
 
	_game->init();
	_game->start();

	return Common::kNoError;
}

bool DraciEngine::handleEvents() {
	Common::Event event;
	bool quit = false;

	while (_eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
			_game->setQuit(true);
			break;
		case Common::EVENT_KEYDOWN:
			if (event.kbd.keycode == Common::KEYCODE_RIGHT) {
				_game->setRoomNum(_game->nextRoomNum());
				_game->setGateNum(0);
				_game->_roomChange = true;
			}
			else if (event.kbd.keycode == Common::KEYCODE_LEFT) {
				_game->setRoomNum(_game->prevRoomNum());
				_game->setGateNum(0);
				_game->_roomChange = true;
			}
			else if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
				int escRoom = _game->getEscRoom();

				// Check if there is an escape room defined for the current room
				if (escRoom != kNoEscRoom) {				

					// Schedule room change
					_game->setRoomNum(_game->getEscRoom());
					_game->setGateNum(0);
					_game->_roomChange = true;

					// End any currently running GPL programs
					_script->endCurrentProgram();
				}
			}
			// Show walking map toggle
			else if (event.kbd.keycode == Common::KEYCODE_w) { 
				_showWalkingMap = !_showWalkingMap;
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

	return quit;
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
	
	// Remove all of our debug levels here
	Common::clearAllDebugChannels();
}

Common::Error DraciEngine::run() {
	init();
	go();
	return Common::kNoError;
}

} // End of namespace Draci
