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
 */

#include "dreamweb/dreamweb.h"
#include "engines/metaengine.h"
#include "graphics/thumbnail.h"
#include "gui/saveload.h"
#include "common/config-manager.h"
#include "common/translation.h"
#include "common/serializer.h"

namespace DreamGen {

// Temporary storage for loading the room from a savegame
Room g_madeUpRoomDat;


void syncReelRoutine(Common::Serializer &s, ReelRoutine *reel) {
	s.syncAsByte(reel->reallocation);
	s.syncAsByte(reel->mapX);
	s.syncAsByte(reel->mapY);
	s.syncAsUint16LE(reel->_reelPointer);
	s.syncAsByte(reel->period);
	s.syncAsByte(reel->counter);
	s.syncAsByte(reel->b7);
}

void DreamBase::loadGame() {
	if (_commandType != 246) {
		_commandType = 246;
		commandOnly(41);
	}
	if (_mouseButton == _oldButton)
		return; // "noload"
	if (_mouseButton == 1)
		doLoad(-1);
}

// if -1, open menu to ask for slot to load
// if >= 0, directly load from that slot
void DreamBase::doLoad(int savegameId) {
	_loadingOrSave = 1;

	if (ConfMan.getBool("dreamweb_originalsaveload") && savegameId == -1) {
		showOpBox();
		showLoadOps();
		_currentSlot = 0;
		showSlots();
		showNames();
		_pointerFrame = 0;
		workToScreenM();
		namesToOld();
		_getBack = 0;

		while (_getBack == 0) {
			if (_quitRequested)
				return;
			delPointer();
			readMouse();
			showPointer();
			vSync();
			dumpPointer();
			dumpTextLine();
			RectWithCallback<DreamBase> loadlist[] = {
				{ kOpsx+176,kOpsx+192,kOpsy+60,kOpsy+76,&DreamBase::getBackToOps },
				{ kOpsx+128,kOpsx+190,kOpsy+12,kOpsy+100,&DreamBase::actualLoad },
				{ kOpsx+2,kOpsx+92,kOpsy+4,kOpsy+81,&DreamBase::selectSlot },
				{ 0,320,0,200,&DreamBase::blank },
				{ 0xFFFF,0,0,0,0 }
			};
			checkCoords(loadlist);
			if (_getBack == 2)
				return; // "quitloaded"
		}
	} else {

		if (savegameId == -1) {
			// Open dialog to get savegameId

			const EnginePlugin *plugin = NULL;
			Common::String gameId = ConfMan.get("gameid");
			EngineMan.findGame(gameId, &plugin);
			GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Restore game:"), _("Restore"));
			dialog->setSaveMode(false);
			savegameId = dialog->runModalWithPluginAndTarget(plugin, ConfMan.getActiveDomainName());
			delete dialog;
		}

		if (savegameId < 0) {
			_getBack = 0;
			return;
		}

		loadPosition(savegameId);

		_getBack = 1;
	}

	// If we reach this point, loadPosition() has just been called.
	// Among other things, it will have filled g_MadeUpRoomDat.

	getRidOfTemp();

	startLoading(g_madeUpRoomDat);
	loadRoomsSample();
	_roomLoaded = 1;
	_newLocation = 255;
	clearSprites();
	initMan();
	initRain();
	_textAddressX = 13;
	_textAddressY = 182;
	_textLen = 240;
	startup();
	workToScreen();
	_getBack = 4;
}


void DreamBase::saveGame() {
	if (data.byte(kMandead) == 2) {
		blank();
		return;
	}

	if (_commandType != 247) {
		_commandType = 247;
		commandOnly(44);
	}
	if (_mouseButton != 1)
		return;

	_loadingOrSave = 2;

	if (ConfMan.getBool("dreamweb_originalsaveload")) {
		showOpBox();
		showSaveOps();
		_currentSlot = 0;
		showSlots();
		showNames();
		workToScreenM();
		namesToOld();
		_bufferIn = 0;
		_bufferOut = 0;
		_getBack = 0;

		while (_getBack == 0) {
			if (_quitRequested)
				return;
			delPointer();
			checkInput();
			readMouse();
			showPointer();
			vSync();
			dumpPointer();
			dumpTextLine();

			RectWithCallback<DreamBase> savelist[] = {
				{ kOpsx+176,kOpsx+192,kOpsy+60,kOpsy+76,&DreamBase::getBackToOps },
				{ kOpsx+128,kOpsx+190,kOpsy+12,kOpsy+100,&DreamBase::actualSave },
				{ kOpsx+2,kOpsx+92,kOpsy+4,kOpsy+81,&DreamBase::selectSlot },
				{ 0,320,0,200,&DreamBase::blank },
				{ 0xFFFF,0,0,0,0 }
			};
			checkCoords(savelist);
		}
		return;
	} else {
		const EnginePlugin *plugin = NULL;
		Common::String gameId = ConfMan.get("gameid");
		EngineMan.findGame(gameId, &plugin);
		GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"));
		dialog->setSaveMode(true);
		int savegameId = dialog->runModalWithPluginAndTarget(plugin, ConfMan.getActiveDomainName());
		Common::String game_description = dialog->getResultString();
		if (game_description.empty())
			game_description = "Untitled";
		delete dialog;

		if (savegameId < 0) {
			_getBack = 0;
			return;
		}

		char descbuf[17] = { 2, 0 };
		Common::strlcpy((char*)descbuf + 1, game_description.c_str(), 16);
		unsigned int desclen = game_description.size();
		if (desclen > 15)
			desclen = 15;
		// zero terminate, and pad with ones
		descbuf[++desclen] = 0;
		while (desclen < 16)
			descbuf[++desclen] = 1;

		// TODO: The below is copied from actualsave
		getRidOfTemp();
		restoreAll(); // reels
		_textAddressX = 13;
		_textAddressY = 182;
		_textLen = 240;
		redrawMainScrn();
		workToScreen();	// show the main screen without the mouse pointer

		// We need to save after the scene has been redrawn, to capture the
		// correct screen thumbnail
		savePosition(savegameId, descbuf);

		workToScreenM();
		_getBack = 4;
	}
}

void DreamBase::namesToOld() {
	memcpy(_saveNamesOld, _saveNames, 17*7);
}

void DreamBase::oldToNames() {
	memcpy(_saveNames, _saveNamesOld, 17*7);
}

void DreamBase::saveLoad() {
	if (data.word(kWatchingtime) || (_pointerMode == 2)) {
		blank();
		return;
	}
	if (_commandType != 253) {
		_commandType = 253;
		commandOnly(43);
	}
	if ((_mouseButton != _oldButton) && (_mouseButton & 1))
		doSaveLoad();
}

void DreamBase::doSaveLoad() {
	_pointerFrame = 0;
	_textAddressX = 70;
	_textAddressY = 182-8;
	_textLen = 181;
	_manIsOffScreen = 1;
	clearWork();
	createPanel2();
	underTextLine();
	getRidOfAll();
	loadSaveBox();
	showOpBox();
	showMainOps();
	workToScreen();

	RectWithCallback<DreamBase> opsList[] = {
		{ kOpsx+59,kOpsx+114,kOpsy+30,kOpsy+76,&DreamBase::getBackFromOps },
		{ kOpsx+10,kOpsx+77,kOpsy+10,kOpsy+59,&DreamBase::DOSReturn },
		{ kOpsx+128,kOpsx+190,kOpsy+16,kOpsy+100,&DreamBase::discOps },
		{ 0,320,0,200,&DreamBase::blank },
		{ 0xFFFF,0,0,0,0 }
	};

	bool firstOps = true;

	do {	// restart ops
		if (firstOps) {
			firstOps = false;
		} else {
			showOpBox();
			showMainOps();
			workToScreenM();
		}
		_getBack = 0;

		do {	// wait ops
			if (_quitRequested) {
				_manIsOffScreen = 0;
				return;
			}

			readMouse();
			showPointer();
			vSync();
			dumpPointer();
			dumpTextLine();
			delPointer();
			checkCoords(opsList);
		} while (!_getBack);
	} while (_getBack == 2);

	_textAddressX = 13;
	_textAddressY = 182;
	_textLen = 240;
	if (_getBack != 4) {
		getRidOfTemp();
		restoreAll();
		redrawMainScrn();
		workToScreenM();
		_commandType = 200;
	}
	_manIsOffScreen = 0;
}

void DreamBase::getBackFromOps() {
	if (data.byte(kMandead) == 2)
		blank();
	else
		getBack1();
}

void DreamBase::getBackToOps() {
	if (_commandType != 201) {
		_commandType = 201;
		commandOnly(42);
	}

	if (_mouseButton != _oldButton) {
		if (_mouseButton & 1) {
			oldToNames();
			_getBack = 2;
		}
	}
}

void DreamBase::showMainOps() {
	showFrame(_tempGraphics, kOpsx+10, kOpsy+10, 8, 0);
	showFrame(_tempGraphics, kOpsx+59, kOpsy+30, 7, 0);
	showFrame(_tempGraphics, kOpsx+128+4, kOpsy+12, 1, 0);
}

void DreamBase::showDiscOps() {
	showFrame(_tempGraphics, kOpsx+128+4, kOpsy+12, 1, 0);
	showFrame(_tempGraphics, kOpsx+10, kOpsy+10, 9, 0);
	showFrame(_tempGraphics, kOpsx+59, kOpsy+30, 10, 0);
	showFrame(_tempGraphics, kOpsx+176+2, kOpsy+60-4, 5, 0);
}

void DreamBase::discOps() {
	if (_commandType != 249) {
		_commandType = 249;
		commandOnly(43);
	}

	if (_mouseButton == _oldButton || !(_mouseButton & 1))
		return;

	scanForNames();
	_loadingOrSave = 2;
	showOpBox();
	showDiscOps();
	_currentSlot = 0;
	workToScreenM();
	_getBack = 0;

	RectWithCallback<DreamBase> discOpsList[] = {
		{ kOpsx+59,kOpsx+114,kOpsy+30,kOpsy+76,&DreamBase::loadGame },
		{ kOpsx+10,kOpsx+79,kOpsy+10,kOpsy+59,&DreamBase::saveGame },
		{ kOpsx+176,kOpsx+192,kOpsy+60,kOpsy+76,&DreamBase::getBackToOps },
		{ 0,320,0,200,&DreamBase::blank },
		{ 0xFFFF,0,0,0,0 }
	};

	do {
		if (_quitRequested)
			return; // quitdiscops

		delPointer();
		readMouse();
		showPointer();
		vSync();
		dumpPointer();
		dumpTextLine();
		checkCoords(discOpsList);
	} while (!_getBack);
}

void DreamBase::actualSave() {
	if (_commandType != 222) {
		_commandType = 222;
		commandOnly(44);
	}

	if (!(_mouseButton & 1))
		return;

	unsigned int slot = _currentSlot;

	const char *desc = &_saveNames[17*slot];
	if (desc[1] == 0) // The actual description string starts at desc[1]
		return;

	savePosition(slot, desc);

	getRidOfTemp();
	restoreAll(); // reels
	_textAddressX = 13;
	_textAddressY = 182;
	_textLen = 240;
	redrawMainScrn();
	workToScreenM();
	_getBack = 4;
}

void DreamBase::actualLoad() {
	if (_commandType != 221) {
		_commandType = 221;
		commandOnly(41);
	}

	if (_mouseButton == _oldButton || _mouseButton != 1)
		return;

	unsigned int slot = _currentSlot;

	const char *desc = &_saveNames[17*slot];
	if (desc[1] == 0) // The actual description string starts at desc[1]
		return;

	loadPosition(_currentSlot);
	_getBack = 1;
}

void DreamBase::savePosition(unsigned int slot, const char *descbuf) {

	const Room &currentRoom = g_roomData[data.byte(kLocation)];

	Room madeUpRoom = currentRoom;
	madeUpRoom.roomsSample = _roomsSample;
	madeUpRoom.mapX = _mapX;
	madeUpRoom.mapY = _mapY;
	madeUpRoom.liftFlag = data.byte(kLiftflag);
	madeUpRoom.b21 = _mansPath;
	madeUpRoom.facing = _facing;
	madeUpRoom.b27 = 255;

	Common::String filename = engine->getSavegameFilename(slot);
	debug(1, "savePosition: slot %d filename %s", slot, filename.c_str());
	Common::OutSaveFile *outSaveFile = engine->getSaveFileManager()->openForSaving(filename);
	if (!outSaveFile)	// TODO: Do proper error handling!
		error("save could not be opened for writing");

	// Initialize new header
	FileHeader header;

	// Note: _desc is not zero-terminated
	const char *desc = "DREAMWEB DATA FILE COPYRIGHT 1992 CREATIVE REALITY";
	assert(strlen(desc) == sizeof(header._desc));
	memcpy(header._desc, desc, sizeof(header._desc));
	memset(&header._len[0], 0, sizeof(header._len));
	memset(&header._padding[0], 0, sizeof(header._padding));

	// fill length fields in savegame file header
	uint16 len[6] = { 17, kLengthofvars, kLengthofextra,
	                  4*kNumChanges, 48, kNumReelRoutines*8+1 };
	for (int i = 0; i < 6; ++i)
		header.setLen(i, len[i]);

	// Write a new section with data that we need for ScummVM (version,
	// thumbnail, played time etc). We don't really care for its size,
	// so we just set it to a magic number.
	header.setLen(6, SCUMMVM_BLOCK_MAGIC_SIZE);

	outSaveFile->write((const uint8 *)&header, sizeof(FileHeader));
	outSaveFile->write(descbuf, len[0]);
	outSaveFile->write(data.ptr(kStartvars, len[1]), len[1]);

	// the Extras segment:
	outSaveFile->write((const uint8 *)_exFrames._frames, 2080);
	outSaveFile->write((const uint8 *)_exFrames._data, kExframeslen);
	outSaveFile->write((const uint8 *)_exData, sizeof(DynObject)*kNumexobjects);
	outSaveFile->write((const uint8 *)_exText._offsetsLE, 2*(kNumExObjects+2));
	outSaveFile->write((const uint8 *)_exText._text, kExtextlen);

	outSaveFile->write(_listOfChanges, len[3]);

	// len[4] == 48, which is sizeof(Room) plus 16 for 'Roomscango'
	outSaveFile->write((const uint8 *)&madeUpRoom, sizeof(Room));
	outSaveFile->write(_roomsCanGo, 16);

	// TODO: Convert more to serializer?
	Common::Serializer s(0, outSaveFile);
	for (unsigned int i = 0; i < kNumReelRoutines; ++i) {
		syncReelRoutine(s, &_reelRoutines[i]);
	}
	// Terminator
	s.syncAsByte(_reelRoutines[kNumReelRoutines].reallocation);

	// ScummVM data block
	outSaveFile->writeUint32BE(SCUMMVM_HEADER);
	outSaveFile->writeByte(SAVEGAME_VERSION);
	TimeDate curTime;
	g_system->getTimeAndDate(curTime);
	uint32 saveDate = ((curTime.tm_mday & 0xFF) << 24) | (((curTime.tm_mon + 1) & 0xFF) << 16) | ((curTime.tm_year + 1900) & 0xFFFF);
	uint32 saveTime = ((curTime.tm_hour & 0xFF) << 16) | (((curTime.tm_min) & 0xFF) << 8) | ((curTime.tm_sec) & 0xFF);
	uint32 playTime = g_engine->getTotalPlayTime() / 1000;
	outSaveFile->writeUint32LE(saveDate);
	outSaveFile->writeUint32LE(saveTime);
	outSaveFile->writeUint32LE(playTime);
	Graphics::saveThumbnail(*outSaveFile);

	outSaveFile->finalize();
	if (outSaveFile->err()) {
		// TODO: Do proper error handling
		warning("an error occurred while writing the savegame");
	}

	delete outSaveFile;
}

void DreamBase::loadPosition(unsigned int slot) {
	_timeCount = 0;
	clearChanges();

	Common::String filename = engine->getSavegameFilename(slot);
	debug(1, "loadPosition: slot %d filename %s", slot, filename.c_str());
	Common::InSaveFile *inSaveFile = engine->getSaveFileManager()->openForLoading(filename);
	if (!inSaveFile)	// TODO: Do proper error handling!
		error("save could not be opened for reading");

	FileHeader header;

	inSaveFile->read((uint8 *)&header, sizeof(FileHeader));

	// read segment lengths from savegame file header
	int len[6];
	for (int i = 0; i < 6; ++i)
		len[i] = header.len(i);
	if (len[0] != 17)
		::error("Error loading save: description buffer isn't 17 bytes");

	if (slot < 7) {
		inSaveFile->read(&_saveNames[17*slot], len[0]);
	} else {
		// The savenames buffer only has room for 7 descriptions
		uint8 namebuf[17];
		inSaveFile->read(namebuf, 17);
	}
	inSaveFile->read(data.ptr(kStartvars, len[1]), len[1]);

	// the Extras segment:
	inSaveFile->read((uint8 *)_exFrames._frames, kExframes);
	inSaveFile->read((uint8 *)_exFrames._data, kExframeslen);
	inSaveFile->read((uint8 *)_exData, sizeof(DynObject)*kNumexobjects);
	inSaveFile->read((uint8 *)_exText._offsetsLE, 2*(kNumExObjects+2));
	inSaveFile->read((uint8 *)_exText._text, kExtextlen);

	inSaveFile->read(_listOfChanges, len[3]);

	// len[4] == 48, which is sizeof(Room) plus 16 for 'Roomscango'
	// Note: the values read into g_madeUpRoomDat are only used in actualLoad,
	// which is (almost) immediately called after this function
	inSaveFile->read((uint8 *)&g_madeUpRoomDat, sizeof(Room));
	inSaveFile->read(_roomsCanGo, 16);

	// TODO: Use serializer for more
	Common::Serializer s(inSaveFile, 0);
	for (unsigned int i = 0; i < kNumReelRoutines; ++i) {
		syncReelRoutine(s, &_reelRoutines[i]);
	}
	// Terminator
	s.syncAsByte(_reelRoutines[kNumReelRoutines].reallocation);

	// Check if there's a ScummVM data block
	if (header.len(6) == SCUMMVM_BLOCK_MAGIC_SIZE) {
		uint32 tag = inSaveFile->readUint32BE();
		if (tag != SCUMMVM_HEADER) {
			warning("ScummVM data block found, but the block header is incorrect - skipping");
			delete inSaveFile;
			return;
		}

		byte version = inSaveFile->readByte();
		if (version > SAVEGAME_VERSION) {
			warning("ScummVM data block found, but it has been saved with a newer version of ScummVM - skipping");
			delete inSaveFile;
			return;
		}

		inSaveFile->skip(4);	// saveDate
		inSaveFile->skip(4);	// saveTime
		uint32 playTime = inSaveFile->readUint32LE();
		g_engine->setTotalPlayTime(playTime * 1000);

		// The thumbnail data follows, but we don't need it here
	}

	delete inSaveFile;
}

// Count number of save files, and load their descriptions into _saveNames
uint DreamBase::scanForNames() {
	// Initialize the first 7 slots (like the original code expects)
	for (unsigned int slot = 0; slot < 7; ++slot) {
		_saveNames[17 * slot + 0] = 2;
		_saveNames[17 * slot + 1] = 0;
		for (int i = 2; i < 17; ++i)
			_saveNames[17 * slot + i] = 1;	// initialize with 1's
	}

	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray files = saveFileMan->listSavefiles("DREAMWEB.D??");
	Common::sort(files.begin(), files.end());

	SaveStateList saveList;
	for (uint i = 0; i < files.size(); ++i) {
		const Common::String &file = files[i];
		Common::InSaveFile *stream = saveFileMan->openForLoading(file);
		if (!stream)
			error("cannot open save file %s", file.c_str());
		char name[17] = {};
		stream->seek(0x61);
		stream->read(name, sizeof(name) - 1);
		delete stream;

		int slotNum = atoi(file.c_str() + file.size() - 2);
		SaveStateDescriptor sd(slotNum, name);
		saveList.push_back(sd);
		if (slotNum < 7)
			Common::strlcpy(&_saveNames[17 * slotNum + 1], name, 16);	// the first character is unused
	}

	// FIXME: Can the following be safely removed?
//	al = saveList.size() <= 7 ? (uint8)saveList.size() : 7;

	return saveList.size();
}

void DreamBase::loadOld() {
	if (_commandType != 252) {
		_commandType = 252;
		commandOnly(48);
	}

	if (!(_mouseButton & 1))
		return;

	doLoad(-1);

	if (_getBack == 4 || _quitRequested)
		return;

	showDecisions();
	workToScreenM();
	_getBack = 0;
}

void DreamBase::showDecisions() {
	createPanel2();
	showOpBox();
	showFrame(_tempGraphics, kOpsx + 17, kOpsy + 13, 6, 0);
	underTextLine();
}

void DreamBase::loadSaveBox() {
	loadIntoTemp("DREAMWEB.G08");
}

// show savegame names (original interface), and set kCursorpos
void DreamBase::showNames() {
	for (int slot = 0; slot < 7; ++slot) {
		// The first character of the savegame name is unused
		Common::String name(&_saveNames[17*slot + 1]);

		if (slot != _currentSlot) {
			printDirect((const uint8 *)name.c_str(), kOpsx + 21, kOpsy + 10*slot + 10, 200, false);
			continue;
		}
		if (_loadingOrSave != 2) {
			_charShift = 91;
			printDirect((const uint8 *)name.c_str(), kOpsx + 21, kOpsy + 10*slot + 10, 200, false);
			_charShift = 0;
			continue;
		}

		int pos = name.size();
		_cursorPos = pos;
		name += '/'; // cursor character
		printDirect((const uint8 *)name.c_str(), kOpsx + 21, kOpsy + 10*slot + 10, 200, false);
	}
}

void DreamBase::checkInput() {
	if (_loadingOrSave == 3)
		return;

	readKey();

	// The first character of the savegame name is unused
	char *name = &_saveNames[17*_currentSlot + 1];

	if (_currentKey == 0) {
		return;
	} else if (_currentKey == 13) {
		_loadingOrSave = 3;
	} else if (_currentKey == 8) {
		if (_cursorPos == 0)
			return;

		--_cursorPos;
		name[_cursorPos] = 0;
		name[_cursorPos+1] = 1;
	} else {
		if (_cursorPos == 14)
			return;

		name[_cursorPos] = _currentKey;
		name[_cursorPos+1] = 0;
		name[_cursorPos+2] = 1;
		++_cursorPos;
	}

	showOpBox();
	showNames();
	showSlots();
	showSaveOps();
	workToScreenM();
}

void DreamBase::selectSlot() {
	if (_commandType != 244) {
		_commandType = 244;
		commandOnly(45);
	}

	if (_mouseButton != 1 || _mouseButton == _oldButton)
		return; // noselslot
	if (_loadingOrSave == 3)
		_loadingOrSave--;

	oldToNames();
	int y = _mouseY - (kOpsy + 4);
	if (y < 11)
		_currentSlot = 0;
	else
		_currentSlot = y / 11;

	delPointer();
	showOpBox();
	showSlots();
	showNames();
	if (_loadingOrSave == 1)
		showLoadOps();
	else
		showSaveOps();
	readMouse();
	showPointer();
	workToScreen();
	delPointer();
}

void DreamBase::showSlots() {
	showFrame(_tempGraphics, kOpsx + 7, kOpsy + 8, 2, 0);

	uint16 y = kOpsy + 11;

	for (int slot = 0; slot < 7; slot++) {
		if (slot == _currentSlot)
			showFrame(_tempGraphics, kOpsx + 10, y, 3, 0);

		y += 10;
	}
}

void DreamBase::showOpBox() {
	showFrame(_tempGraphics, kOpsx, kOpsy, 0, 0);

	// CHECKME: There seem to be versions of dreamweb in which this call
	// should be removed. It displays a red dot on the ops dialogs if left in.
	showFrame(_tempGraphics, kOpsx, kOpsy + 55, 4, 0);
}

void DreamBase::showLoadOps() {
	showFrame(_tempGraphics, kOpsx + 128 + 4, kOpsy + 12, 1, 0);
	showFrame(_tempGraphics, kOpsx + 176 + 2, kOpsy + 60 - 4, 5, 0);
	printMessage(kOpsx + 104, kOpsy + 14, 55, 101, (101 & 1));
}

void DreamBase::showSaveOps() {
	showFrame(_tempGraphics, kOpsx + 128 + 4, kOpsy + 12, 1, 0);
	showFrame(_tempGraphics, kOpsx + 176 + 2, kOpsy + 60 - 4, 5, 0);
	printMessage(kOpsx + 104, kOpsy + 14, 54, 101, (101 & 1));
}

} // End of namespace DreamGen
