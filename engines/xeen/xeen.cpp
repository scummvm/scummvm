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

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "engines/util.h"
#include "graphics/scaler.h"
#include "graphics/thumbnail.h"
#include "xeen/xeen.h"
#include "xeen/files.h"
#include "xeen/resources.h"

namespace Xeen {

XeenEngine *g_vm = nullptr;

XeenEngine::XeenEngine(OSystem *syst, const XeenGameDescription *gameDesc)
		: Engine(syst), _gameDescription(gameDesc), _randomSource("Xeen") {
	// Set up debug channels
	DebugMan.addDebugChannel(kDebugPath, "Path", "Pathfinding debug level");
	DebugMan.addDebugChannel(kDebugScripts, "scripts", "Game scripts");
	DebugMan.addDebugChannel(kDebugGraphics, "graphics", "Graphics handling");
	DebugMan.addDebugChannel(kDebugSound, "sound", "Sound processing");

	_combat = nullptr;
	_debugger = nullptr;
	_events = nullptr;
	_files = nullptr;
	_interface = nullptr;
	_locations = nullptr;
	_map = nullptr;
	_party = nullptr;
	_resources = nullptr;
	_saves = nullptr;
	_screen = nullptr;
	_scripts = nullptr;
	_sound = nullptr;
	_spells = nullptr;
	_windows = nullptr;
	_eventData = nullptr;
	_noDirectionSense = false;
	_startupWindowActive = false;
	_quitMode = 0;
	_mode = MODE_0;
	_endingScore = 0;
	_loadSaveSlot = -1;
	g_vm = this;
}

XeenEngine::~XeenEngine() {
	delete _combat;
	delete _debugger;
	delete _events;
	delete _interface;
	delete _locations;
	delete _map;
	delete _party;
	delete _saves;
	delete _screen;
	delete _scripts;
	delete _sound;
	delete _spells;
	delete _windows;
	delete _eventData;
	delete _resources;
	delete _files;
	g_vm = nullptr;
}

void XeenEngine::initialize() {
	// Create sub-objects of the engine
	_files = new FileManager(this);
	_resources = Resources::init(this);
	_combat = new Combat(this);
	_debugger = new Debugger(this);
	_events = new EventsManager(this);
	_interface = new Interface(this);
	_locations = new LocationManager();
	_map = new Map(this);
	_party = new Party(this);
	_saves = new SavesManager(this, *_party);
	_screen = new Screen(this);
	_scripts = new Scripts(this);
	_sound = new Sound(this, _mixer);
	_spells = new Spells(this);
	_windows = new Windows();

	File f("029.obj");
	_eventData = f.readStream(f.size());

	// Set graphics mode
	initGraphics(320, 200);

	// If requested, load a savegame instead of showing the intro
	if (ConfMan.hasKey("save_slot")) {
		int saveSlot = ConfMan.getInt("save_slot");
		if (saveSlot >= 0 && saveSlot <= 999)
			_loadSaveSlot = saveSlot;
	}
}

Common::Error XeenEngine::run() {
	initialize();

	outerGameLoop();

	return Common::kNoError;
}

int XeenEngine::getRandomNumber(int maxNumber) {
	return _randomSource.getRandomNumber(maxNumber);
}

int XeenEngine::getRandomNumber(int minNumber, int maxNumber) {
	return getRandomNumber(maxNumber - minNumber) + minNumber;
}

Common::Error XeenEngine::saveGameState(int slot, const Common::String &desc) {
	Common::OutSaveFile *out = g_system->getSavefileManager()->openForSaving(
		generateSaveName(slot));
	if (!out)
		return Common::kCreatingFileFailed;

	XeenSavegameHeader header;
	header._saveName = desc;
	writeSavegameHeader(out, header);

	Common::Serializer s(nullptr, out);
	synchronize(s);

	out->finalize();
	delete out;

	return Common::kNoError;
}

Common::Error XeenEngine::loadGameState(int slot) {
	Common::InSaveFile *saveFile = g_system->getSavefileManager()->openForLoading(
		generateSaveName(slot));
	if (!saveFile)
		return Common::kReadingFailed;

	Common::Serializer s(saveFile, nullptr);

	// Load the savaegame header
	XeenSavegameHeader header;
	if (!readSavegameHeader(saveFile, header))
		error("Invalid savegame");

	if (header._thumbnail) {
		header._thumbnail->free();
		delete header._thumbnail;
	}

	// Load most of the savegame data
	synchronize(s);
	delete saveFile;

	return Common::kNoError;
}

Common::String XeenEngine::generateSaveName(int slot) {
	return Common::String::format("%s.%03d", _targetName.c_str(), slot);
}

bool XeenEngine::canLoadGameStateCurrently() {
	return true;
}

bool XeenEngine::canSaveGameStateCurrently() {
	return true;
}

void XeenEngine::synchronize(Common::Serializer &s) {
	// TODO
}

const char *const SAVEGAME_STR = "XEEN";
#define SAVEGAME_STR_SIZE 6

bool XeenEngine::readSavegameHeader(Common::InSaveFile *in, XeenSavegameHeader &header) {
	char saveIdentBuffer[SAVEGAME_STR_SIZE + 1];
	header._thumbnail = nullptr;

	// Validate the header Id
	in->read(saveIdentBuffer, SAVEGAME_STR_SIZE + 1);
	if (strncmp(saveIdentBuffer, SAVEGAME_STR, SAVEGAME_STR_SIZE))
		return false;

	header._version = in->readByte();
	if (header._version > XEEN_SAVEGAME_VERSION)
		return false;

	// Read in the string
	header._saveName.clear();
	char ch;
	while ((ch = (char)in->readByte()) != '\0')
		header._saveName += ch;

	// Get the thumbnail
	header._thumbnail = Graphics::loadThumbnail(*in);
	if (!header._thumbnail)
		return false;

	// Read in save date/time
	header._year = in->readSint16LE();
	header._month = in->readSint16LE();
	header._day = in->readSint16LE();
	header._hour = in->readSint16LE();
	header._minute = in->readSint16LE();
	header._totalFrames = in->readUint32LE();

	return true;
}

void XeenEngine::writeSavegameHeader(Common::OutSaveFile *out, XeenSavegameHeader &header) {
	// Write out a savegame header
	out->write(SAVEGAME_STR, SAVEGAME_STR_SIZE + 1);

	out->writeByte(XEEN_SAVEGAME_VERSION);

	// Write savegame name
	out->writeString(header._saveName);
	out->writeByte('\0');

	// Write a thumbnail of the screen
/*
	uint8 thumbPalette[768];
	_screen->getPalette(thumbPalette);
	Graphics::Surface saveThumb;
	::createThumbnail(&saveThumb, (const byte *)_screen->getPixels(),
		_screen->w, _screen->h, thumbPalette);
	Graphics::saveThumbnail(*out, saveThumb);
	saveThumb.free();
*/
	// Write out the save date/time
	TimeDate td;
	g_system->getTimeAndDate(td);
	out->writeSint16LE(td.tm_year + 1900);
	out->writeSint16LE(td.tm_mon + 1);
	out->writeSint16LE(td.tm_mday);
	out->writeSint16LE(td.tm_hour);
	out->writeSint16LE(td.tm_min);
//	out->writeUint32LE(_events->getFrameCounter());
}

void XeenEngine::playGame() {
	_saves->reset();
	_files->setGameCc(0);
	_sound->stopAllAudio();

	play();
}

void XeenEngine::play() {
	// TODO: Init variables
	_quitMode = 0;

	_interface->setup();
	_screen->loadBackground("back.raw");
	_screen->loadPalette("mm4.pal");

	if (getGameID() != GType_WorldOfXeen && !_map->_loadDarkSide) {
		_map->_loadDarkSide = true;
		_party->_mazeId = 29;
		_party->_mazeDirection = DIR_NORTH;
		_party->_mazePosition.x = 25;
		_party->_mazePosition.y = 21;
	}

	_map->load(_party->_mazeId);

	_interface->startup();
	if (_mode == MODE_0) {
//		_screen->fadeOut();
	}

	(*_windows)[0].update();
	_interface->mainIconsPrint();
	(*_windows)[0].update();
	_events->setCursor(0);

	_combat->_moveMonsters = true;
	if (_mode == MODE_0) {
		_mode = MODE_1;
		_screen->fadeIn();
	}

	_combat->_moveMonsters = true;

	gameLoop();
}

void XeenEngine::gameLoop() {
	// Main game loop
	while (!shouldQuit()) {
		_map->cellFlagLookup(_party->_mazePosition);
		if (_map->_currentIsEvent) {
			_quitMode = _scripts->checkEvents();
			if (shouldQuit() || _quitMode)
				return;
		}
		_party->giveTreasure();

		// Main user interface handler for waiting for and processing user input
		_interface->perform();
	}
}

Common::String XeenEngine::printMil(uint value) {
	return (value >= 1000000) ? Common::String::format("%u mil", value / 1000000) :
		Common::String::format("%u", value);
}

Common::String XeenEngine::printK(uint value) {
	return (value > 9999) ? Common::String::format("%uk", value / 1000) :
		Common::String::format("%u", value);
}

Common::String XeenEngine::printK2(uint value) {
	return (value > 999) ? Common::String::format("%uk", value / 1000) :
		Common::String::format("%u", value);
}

} // End of namespace Xeen
