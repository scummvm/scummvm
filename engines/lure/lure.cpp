/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
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

#include "base/plugins.h"

#include "common/config-manager.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/system.h"
#include "common/md5.h"
#include "common/savefile.h"
#include "common/stream.h"

#include "sound/mixer.h"
#include "sound/mididrv.h"
#include "sound/audiostream.h"

#include "lure/luredefs.h"
#include "lure/surface.h"
#include "lure/lure.h"
#include "lure/intro.h"
#include "lure/game.h"
#include "lure/system.h"

namespace Lure {

enum {
	// We only compute MD5 of the first kilobyte of our data files.
	kMD5FileSizeLimit = 1024
};

struct GameSettings {
	const char *gameid;
	const char *description;
	byte id;
	uint32 features;
	Common::Language language;
	const char *md5sum;
	const char *checkFile;
};

//
static const GameSettings lure_games[] = {
	{ "lure", "Lure of the Temptress", GI_LURE, GF_FLOPPY, Common::EN_ANY,
										"b2a8aa6d7865813a17a3c636e063572e", "disk1.vga" },
/*
	{ "lure", "Lure of the Temptress", GI_LURE, GF_FLOPPY, Common::DE_DEU,
										"7aa19e444dab1ac7194d9f7a40ffe54a", "disk1.vga" },
	{ "lure", "Lure of the Temptress", GI_LURE, GF_FLOPPY, Common::FR_FRA,
										"1c94475c1bb7e0e88c1757d3b5377e94", "disk1.vga" },
*/
	{ 0, 0, 0, 0, Common::UNK_LANG, 0, 0 }
};

// Keep list of different supported games

static const PlainGameDescriptor lure_list[] = {
	{ "lure", "Lure of the Temptress" },
	{ 0, 0 }
};

} // End of namespace Lure

using namespace Lure;

GameList Engine_LURE_gameIDList() {
	GameList games;
	const PlainGameDescriptor *g = lure_list;

	while (g->gameid) {
		games.push_back(*g);
		g++;
	}
	return games;
}

GameDescriptor Engine_LURE_findGameID(const char *gameid) {
	const PlainGameDescriptor *g = lure_list;
	while (g->gameid) {
		if (0 == scumm_stricmp(gameid, g->gameid))
			break;
		g++;
	}
	return GameDescriptor(g->gameid, g->description);
}

GameList Engine_LURE_detectGames(const FSList &fslist) {
	GameList detectedGames;
	const GameSettings *g;
	FSList::const_iterator file;

	// Iterate over all files in the given directory
	bool isFound = false;
	for (file = fslist.begin(); file != fslist.end(); file++) {
		if (file->isDirectory())
			continue;

		for (g = lure_games; g->gameid; g++) {
			if (scumm_stricmp(file->name().c_str(), g->checkFile) == 0)
				isFound = true;
		}
		if (isFound)
			break;
	}

	if (file == fslist.end())
		return detectedGames;

	char md5str[32 + 1];

	if (Common::md5_file_string(*file, md5str, kMD5FileSizeLimit)) {
		for (g = lure_games; g->gameid; g++) {
			if (strcmp(g->md5sum, (char *)md5str) == 0) {
				GameDescriptor dg(g->gameid, g->description, g->language);
				dg.updateDesc((g->features & GF_FLOPPY) ? "Floppy" : 0);
				detectedGames.push_back(dg);
			}
		}
		if (detectedGames.empty()) {
			debug("Unknown MD5 (%s)! Please report the details (language, platform, etc.) of this game to the ScummVM team\n", md5str);

			const PlainGameDescriptor *g1 = lure_list;
			while (g1->gameid) {
				detectedGames.push_back(*g1);
				g1++;
			}
		}
	}
	return detectedGames;
}

PluginError Engine_LURE_create(OSystem *syst, Engine **engine) {
	assert(engine);
	*engine = new LureEngine(syst);
	return kNoError;
}

REGISTER_PLUGIN(LURE, "Lure of the Temptress Engine", "Lure of the Temptress (C) Revolution");

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

void LureEngine::detectGame() {
	// Make sure all the needed files are present

	if (!Common::File::exists(SUPPORT_FILENAME))
		error("Missing %s - this is a custom file containing resources from the\n"
			"Lure of the Temptress executable. See the documentation for creating it.",
			SUPPORT_FILENAME);

	for (uint8 fileNum = 1; fileNum <= 4; ++fileNum)
	{
		char sFilename[10];
		sprintf(sFilename, "disk%d.vga", fileNum);

		if (!Common::File::exists(sFilename))
			error("Missing disk%d.vga", fileNum);
	}

	// Check the version of the lure.dat file
	Common::File f;
	if (!f.open(SUPPORT_FILENAME)) {
		error("Error opening %s for validation", SUPPORT_FILENAME);
	} else {
		f.seek(0xbf * 8);
		VersionStructure version;
		f.read(&version, sizeof(VersionStructure));
		f.close();

		if (READ_LE_UINT16(&version.id) != 0xffff)
			error("Error validating %s - file is invalid or out of date", SUPPORT_FILENAME);
		else if ((version.vMajor != LURE_DAT_MAJOR) || (version.vMinor != LURE_DAT_MINOR))
			error("Incorrect version of %s file - expected %d.%d but got %d.%d",
				SUPPORT_FILENAME, LURE_DAT_MAJOR, LURE_DAT_MINOR, 
				version.vMajor, version.vMinor);
	}

	// Do an md5 check 

	char md5str[32 + 1];
	const GameSettings *g;
	bool found = false;

	*md5str = 0;

	for (g = lure_games; g->gameid; g++) {
		if (!Common::File::exists(g->checkFile))
			continue;

		if (!Common::md5_file_string(g->checkFile, md5str, kMD5FileSizeLimit))
			continue;

		if (strcmp(g->md5sum, (char *)md5str) == 0) {
			_features = g->features;
			_game = g->id;
			_language = g->language;

			if (g->description)
				g_system->setWindowCaption(g->description);

			found = true;
			break;
		}
	}

	if (!found) {
		debug("Unknown MD5 (%s)! Please report the details (language, platform, etc.) of this game to the ScummVM team", md5str);
		_features = GF_LNGUNK || GF_FLOPPY;
		_game = GI_LURE;
	}
}

int LureEngine::init() {
	_system->beginGFXTransaction();
		initCommonGFX(false);
		_system->initSize(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT);
	_system->endGFXTransaction();

	detectGame();
	_sys = new System(_system);
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
	delete _sys;
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

	Room::getReference().saveToStream(f);
	Resources::getReference().saveToStream(f);

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
	Room::getReference().loadFromStream(f);
	Resources::getReference().loadFromStream(f);

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
