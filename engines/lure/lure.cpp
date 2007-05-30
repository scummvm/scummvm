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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"

#include "common/config-manager.h"
#include "common/system.h"
#include "common/savefile.h"

#include "lure/luredefs.h"
#include "lure/surface.h"
#include "lure/lure.h"
#include "lure/intro.h"
#include "lure/game.h"

namespace Lure {

static LureEngine *int_engine = NULL;

LureEngine::LureEngine(OSystem *system): Engine(system) {

	Common::addSpecialDebugLevel(kLureDebugScripts, "scripts", "Scripts debugging");
	Common::addSpecialDebugLevel(kLureDebugAnimations, "animations", "Animations debugging");
	Common::addSpecialDebugLevel(kLureDebugHotspots, "hotspots", "Hotspots debugging");
	// Setup mixer
/*
	if (!_mixer->isReady()) {
		warning("Sound initialization failed.");
	}

	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, ConfMan.getInt("speech_volume"));
*/
	
	_features = 0;
	_game = 0;
}

int LureEngine::init() {
	_system->beginGFXTransaction();
		initCommonGFX(false);
		_system->initSize(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT);
	_system->endGFXTransaction();

	detectGame();
	_disk = new Disk();
	_resources = new Resources();
	_strings = new StringData();
	_screen = new Screen(*_system);
	_mouse = new Mouse();
	_events = new Events();
	_menu = new Menu();
	Surface::initialise();
	_room = new Room();
	int_engine = this;
	return 0;
}

LureEngine::~LureEngine() {
	// Remove all of our debug levels here
	Common::clearAllSpecialDebugLevels();

	// Delete and deinitialise subsystems
	Surface::deinitialise();
	delete _room;
	delete _menu;
	delete _events;
	delete _mouse;
	delete _screen;
	delete _strings;
	delete _resources;
	delete _disk;
}

LureEngine &LureEngine::getReference() {
	return *int_engine;
}

int LureEngine::go() {
	if (ConfMan.getInt("boot_param") == 0) {
		// Show the introduction
		Introduction *intro = new Introduction(*_screen, *_system);
		intro->show();
		delete intro;
	}

	// Play the game
	if (!_events->quitFlag) {
		// Play the game
		Game *gameInstance = new Game();
		gameInstance->execute();
		delete gameInstance;
	}

	//quitGame();
	return 0;
}

void LureEngine::quitGame() {
	_system->quit();
}

const char *LureEngine::generateSaveName(int slotNumber) {
	static char buffer[15];

	sprintf(buffer, "lure.%.3d", slotNumber);
	return buffer;
}

bool LureEngine::saveGame(uint8 slotNumber, Common::String &caption) {
	Common::WriteStream *f = this->_saveFileMan->openForSaving(
		generateSaveName(slotNumber));
	if (f == NULL) {
		warning("saveGame: Failed to save slot %d", slotNumber);
		return false;
	}

	f->write("lure", 5);
	f->writeByte(_language);
	f->writeByte(LURE_DAT_MINOR);
	f->writeString(caption);
	f->writeByte(0); // End of string terminator

	Resources::getReference().saveToStream(f);
	Room::getReference().saveToStream(f);

	delete f;
	return true;
}

#define FAILED_MSG "loadGame: Failed to load slot %d"

bool LureEngine::loadGame(uint8 slotNumber) {
	Common::ReadStream *f = this->_saveFileMan->openForLoading(
		generateSaveName(slotNumber));
	if (f == NULL) {
		warning(FAILED_MSG, slotNumber);
		return false;
	}

	// Check for header
	char buffer[5];
	f->read(buffer, 5);
	if (memcmp(buffer, "lure", 5) != 0)
	{
		warning(FAILED_MSG, slotNumber);
		delete f;
		return false;
	}

	// Check language version 
	uint8 language = f->readByte();
	uint8 version = f->readByte();
	if ((language != _language) || (version != LURE_DAT_MINOR))
	{
		warning("loadGame: Failed to load slot %d - incorrect version", slotNumber);
		delete f;
		return false;
	}

	// Read in and discard the savegame caption
	while (f->readByte() != 0) ;

	// Load in the data
	Resources::getReference().loadFromStream(f);
	Room::getReference().loadFromStream(f);

	delete f;
	return true;
}

Common::String *LureEngine::detectSave(int slotNumber) {
	Common::ReadStream *f = this->_saveFileMan->openForLoading(
		generateSaveName(slotNumber));
	if (f == NULL) return NULL;
	Common::String *result = NULL;

	// Check for header
	char buffer[5];
	f->read(&buffer[0], 5);
	if (memcmp(&buffer[0], "lure", 5) == 0) {
		// Check language version 
		uint8 language = f->readByte();
		uint8 version = f->readByte();
		if ((language == _language) && (version == LURE_DAT_MINOR)) {
			// Read in the savegame title
			char saveName[MAX_DESC_SIZE];
			char *p = saveName;
			int decCtr = MAX_DESC_SIZE - 1;
			while ((decCtr > 0) && ((*p++ = f->readByte()) != 0)) --decCtr;
			*p = '\0';
			result = new Common::String(saveName);
		}
	}

	delete f;
	return result;
}

} // End of namespace Lure
