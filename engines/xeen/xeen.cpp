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
#include "xeen/dialogs_options.h"
#include "xeen/files.h"

namespace Xeen {

XeenEngine::XeenEngine(OSystem *syst, const XeenGameDescription *gameDesc)
		: Engine(syst), ButtonContainer(), _gameDescription(gameDesc), _randomSource("Xeen") {
	_debugger = nullptr;
	_events = nullptr;
	_saves = nullptr;
	_screen = nullptr;
	_sound = nullptr;
	_eventData = nullptr;
	Common::fill(&_activeRoster[0], &_activeRoster[MAX_ACTIVE_PARTY], nullptr);
	Common::fill(&_partyFaces[0], &_partyFaces[MAX_ACTIVE_PARTY], nullptr);
	_isEarlyGame = false;
	_loadDarkSide = 1;
	_buttonsLoaded = false;
}

XeenEngine::~XeenEngine() {
	delete _debugger;
	delete _events;
	delete _saves;
	delete _screen;
	delete _sound;
	delete _eventData;
}

void XeenEngine::initialize() {
	// Set up debug channels
	DebugMan.addDebugChannel(kDebugPath, "Path", "Pathfinding debug level");
	DebugMan.addDebugChannel(kDebugScripts, "scripts", "Game scripts");
	DebugMan.addDebugChannel(kDebugGraphics, "graphics", "Graphics handling");
	DebugMan.addDebugChannel(kDebugSound, "sound", "Sound and Music handling");

	// Create sub-objects of the engine
	FileManager::init(this);
	_debugger = new Debugger(this);
	_events = new EventsManager(this);
	_saves = new SavesManager(this, _party, _roster);
	_screen = new Screen(this);
	_screen->setupWindows();
	_sound = new SoundManager(this);

	File f("029.obj");
	_eventData = f.readStream(f.size());

	// Set graphics mode
	initGraphics(320, 200, false);

	// If requested, load a savegame instead of showing the intro
	if (ConfMan.hasKey("save_slot")) {
		int saveSlot = ConfMan.getInt("save_slot");
		if (saveSlot >= 0 && saveSlot <= 999)
			_loadSaveSlot = saveSlot;
	}
}

Common::Error XeenEngine::run() {
	initialize();

	showIntro();
	if (shouldQuit())
		return Common::kNoError;

	showMainMenu();
	if (shouldQuit())
		return Common::kNoError;

	playGame();

	return Common::kNoError;
}

int XeenEngine::getRandomNumber(int maxNumber) {
	return _randomSource.getRandomNumber(maxNumber);
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

void XeenEngine::showIntro() {

}

void XeenEngine::showMainMenu() {
	//OptionsMenu::show(this);
}

void XeenEngine::playGame() {
	_saves->reset();
	setupUI(true);
}

/*
 * Lots of stuff in this method.
 * TODO: Consider renaming method when better understood
 */
void XeenEngine::setupUI(bool soundPlayed) {
	SpriteResource sprites1("global.icn"), borderSprites("border.icn"),
		uiSprites("inn.icn");

	// Get mappings to the active characters in the party
	Common::fill(&_activeRoster[0], &_activeRoster[MAX_ACTIVE_PARTY], nullptr);
	for (int i = 0; i < _party._partyCount; ++i) {
		_activeRoster[i] = &_roster[_party._partyMembers[i]];
	}

	_isEarlyGame = _party._minutes >= 300;
	
	if (_party._mazeId == 0) {
		if (!soundPlayed) {
			warning("TODO: loadSound?");
		}

		if (!_partyFaces[0]) {
			// Xeen only uses 24 of possible 30 character slots
			loadCharIcons(XEEN_TOTAL_CHARACTERS);

			for (int i = 0; i < _party._partyCount; ++i)
				_partyFaces[i] = &_charFaces[_party._partyMembers[i]];
		}

		_mode = MODE_1;
		Common::Array<int> xeenSideChars;

		// Build up a list of characters on the same Xeen side being loaded
		for (int i = 0; i < XEEN_TOTAL_CHARACTERS; ++i) {
			PlayerStruct &player = _roster[i];
			if (player._name.empty() || player._xeenSide != _loadDarkSide)
				continue;

			xeenSideChars.push_back(i);
		}

		// Add in buttons for the UI
		_buttonsLoaded = true;
		addButton(Common::Rect(16, 100, 40, 120), 242, &uiSprites, true);
		addButton(Common::Rect(52, 100, 76, 120), 243, &uiSprites, true);
		addButton(Common::Rect(87, 100, 111, 120), 68, &uiSprites, true);
		addButton(Common::Rect(122, 100, 146, 120), 82, &uiSprites, true);
		addButton(Common::Rect(157, 100, 181, 120), 67, &uiSprites, true);
		addButton(Common::Rect(192, 100, 216, 120), 88, &uiSprites, true);
		addButton(Common::Rect(), 27, &uiSprites, false);
		addButton(Common::Rect(16, 16, 48, 48), 49, &uiSprites, false);
		addButton(Common::Rect(117, 16, 139, 48), 50, &uiSprites, false);
		addButton(Common::Rect(16, 59, 48, 81), 51, &uiSprites, false);
		addButton(Common::Rect(117, 59, 149, 81), 52, &uiSprites, false);

		setupGameBackground();
	}
}

void XeenEngine::loadCharIcons(int numChars) {
	for (int i = 0; i < numChars; ++i) {
		// Load new character resource
		Common::String name = Common::String::format("char%02d.fac", i);
		_charFaces[i].load(name);
	}

	_dseFace.load("dse.fac");
}

void XeenEngine::setupGameBackground() {
	
}

} // End of namespace Xeen
