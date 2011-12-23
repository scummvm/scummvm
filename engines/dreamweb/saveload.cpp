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
	if (data.byte(kCommandtype) != 246) {
		data.byte(kCommandtype) = 246;
		commandOnly(41);
	}
	if (data.word(kMousebutton) == data.word(kOldbutton))
		return; // "noload"
	if (data.word(kMousebutton) == 1)
		doLoad(-1);
}

// if -1, open menu to ask for slot to load
// if >= 0, directly load from that slot
void DreamBase::doLoad(int savegameId) {
	data.byte(kLoadingorsave) = 1;

	if (ConfMan.getBool("dreamweb_originalsaveload") && savegameId == -1) {
		showOpBox();
		showLoadOps();
		data.byte(kCurrentslot) = 0;
		showSlots();
		showNames();
		data.byte(kPointerframe) = 0;
		workToScreenM();
		namesToOld();
		data.byte(kGetback) = 0;

		while (data.byte(kGetback) == 0) {
			if (quitRequested())
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
			if (data.byte(kGetback) == 2)
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
			data.byte(kGetback) = 0;
			return;
		}

		loadPosition(savegameId);

		data.byte(kGetback) = 1;
	}

	// If we reach this point, loadPosition() has just been called.
	// Among other things, it will have filled g_MadeUpRoomDat.

	// kTempgraphics might not have been allocated if we bypassed all menus
	if (data.word(kTempgraphics) != 0xFFFF)
		getRidOfTemp();

	startLoading(g_madeUpRoomDat);
	loadRoomsSample();
	data.byte(kRoomloaded) = 1;
	data.byte(kNewlocation) = 255;
	clearSprites();
	initMan();
	initRain();
	data.word(kTextaddressx) = 13;
	data.word(kTextaddressy) = 182;
	data.byte(kTextlen) = 240;
	startup();
	workToScreenCPP();
	data.byte(kGetback) = 4;
}


void DreamBase::saveGame() {
	if (data.byte(kMandead) == 2) {
		blank();
		return;
	}

	if (data.byte(kCommandtype) != 247) {
		data.byte(kCommandtype) = 247;
		commandOnly(44);
	}
	if (data.word(kMousebutton) != 1)
		return;

	data.byte(kLoadingorsave) = 2;

	if (ConfMan.getBool("dreamweb_originalsaveload")) {
		showOpBox();
		showSaveOps();
		data.byte(kCurrentslot) = 0;
		showSlots();
		showNames();
		workToScreenM();
		namesToOld();
		data.word(kBufferin) = 0;
		data.word(kBufferout) = 0;
		data.byte(kGetback) = 0;

		while (data.byte(kGetback) == 0) {
			if (quitRequested())
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
			data.byte(kGetback) = 0;
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
		data.word(kTextaddressx) = 13;
		data.word(kTextaddressy) = 182;
		data.byte(kTextlen) = 240;
		redrawMainScrn();
		workToScreenCPP();	// show the main screen without the mouse pointer

		// We need to save after the scene has been redrawn, to capture the
		// correct screen thumbnail
		savePosition(savegameId, descbuf);

		workToScreenM();
		data.byte(kGetback) = 4;
	}
}

void DreamBase::namesToOld() {
	memcpy(_saveNamesOld, _saveNames, 17*7);
}

void DreamBase::oldToNames() {
	memcpy(_saveNames, _saveNamesOld, 17*7);
}

void DreamBase::saveLoad() {
	if (data.word(kWatchingtime) || (data.byte(kPointermode) == 2)) {
		blank();
		return;
	}
	if (data.byte(kCommandtype) != 253) {
		data.byte(kCommandtype) = 253;
		commandOnly(43);
	}
	if ((data.word(kMousebutton) != data.word(kOldbutton)) && (data.word(kMousebutton) & 1))
		doSaveLoad();
}

void DreamBase::doSaveLoad() {
	data.byte(kPointerframe) = 0;
	data.word(kTextaddressx) = 70;
	data.word(kTextaddressy) = 182-8;
	data.byte(kTextlen) = 181;
	data.byte(kManisoffscreen) = 1;
	clearWork();
	createPanel2();
	underTextLine();
	getRidOfAll();
	loadSaveBox();
	showOpBox();
	showMainOps();
	workToScreenCPP();

	RectWithCallback<DreamGenContext> opsList[] = {
		{ kOpsx+59,kOpsx+114,kOpsy+30,kOpsy+76,&DreamBase::getBackFromOps },
		{ kOpsx+10,kOpsx+77,kOpsy+10,kOpsy+59,&DreamBase::DOSReturn },
		{ kOpsx+128,kOpsx+190,kOpsy+16,kOpsy+100,&DreamGenContext::discOps },
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
		data.byte(kGetback) = 0;

		do {	// wait ops
			if (data.byte(kQuitrequested)) {
				data.byte(kManisoffscreen) = 0;
				return;
			}

			readMouse();
			showPointer();
			vSync();
			dumpPointer();
			dumpTextLine();
			delPointer();
			checkCoords(opsList);
		} while (!data.byte(kGetback));
	} while (data.byte(kGetback) == 2);

	data.word(kTextaddressx) = 13;
	data.word(kTextaddressy) = 182;
	data.byte(kTextlen) = 240;
	if (data.byte(kGetback) != 4) {
		getRidOfTemp();
		restoreAll();
		redrawMainScrn();
		workToScreenM();
		data.byte(kCommandtype) = 200;
	}
	data.byte(kManisoffscreen) = 0;
}

void DreamBase::getBackFromOps() {
	if (data.byte(kMandead) == 2)
		blank();
	else
		getBack1();
}

void DreamBase::getBackToOps() {
	if (data.byte(kCommandtype) != 201) {
		data.byte(kCommandtype) = 201;
		commandOnly(42);
	}

	if (data.word(kMousebutton) != data.word(kOldbutton)) {
		if (data.word(kMousebutton) & 1) {
			oldToNames();
			data.byte(kGetback) = 2;
		}
	}
}

void DreamBase::showMainOps() {
	showFrame(tempGraphics(), kOpsx+10, kOpsy+10, 8, 0);
	showFrame(tempGraphics(), kOpsx+59, kOpsy+30, 7, 0);
	showFrame(tempGraphics(), kOpsx+128+4, kOpsy+12, 1, 0);
}

void DreamBase::showDiscOps() {
	showFrame(tempGraphics(), kOpsx+128+4, kOpsy+12, 1, 0);
	showFrame(tempGraphics(), kOpsx+10, kOpsy+10, 9, 0);
	showFrame(tempGraphics(), kOpsx+59, kOpsy+30, 10, 0);
	showFrame(tempGraphics(), kOpsx+176+2, kOpsy+60-4, 5, 0);
}

void DreamBase::discOps() {
	if (data.byte(kCommandtype) != 249) {
		data.byte(kCommandtype) = 249;
		commandOnly(43);
	}

	if (data.word(kMousebutton) == data.word(kOldbutton) || !(data.word(kMousebutton) & 1))
		return;

	scanForNames();
	data.byte(kLoadingorsave) = 2;
	showOpBox();
	showDiscOps();
	data.byte(kCurrentslot) = 0;
	workToScreenM();
	data.byte(kGetback) = 0;

	RectWithCallback<DreamGenContext> discOpsList[] = {
		{ kOpsx+59,kOpsx+114,kOpsy+30,kOpsy+76,&DreamBase::loadGame },
		{ kOpsx+10,kOpsx+79,kOpsy+10,kOpsy+59,&DreamBase::saveGame },
		{ kOpsx+176,kOpsx+192,kOpsy+60,kOpsy+76,&DreamBase::getBackToOps },
		{ 0,320,0,200,&DreamBase::blank },
		{ 0xFFFF,0,0,0,0 }
	};

	do {
		if (data.byte(kQuitrequested) != 0)
			return; // quitdiscops

		delPointer();
		readMouse();
		showPointer();
		vSync();
		dumpPointer();
		dumpTextLine();
		checkCoords(discOpsList);
	} while (!data.byte(kGetback));
}

void DreamBase::actualSave() {
	if (data.byte(kCommandtype) != 222) {
		data.byte(kCommandtype) = 222;
		commandOnly(44);
	}

	if (!(data.word(kMousebutton) & 1))
		return;

	unsigned int slot = data.byte(kCurrentslot);

	const char *desc = &_saveNames[17*slot];
	if (desc[1] == 0) // The actual description string starts at desc[1]
		return;

	savePosition(slot, desc);

	getRidOfTemp();
	restoreAll(); // reels
	data.word(kTextaddressx) = 13;
	data.word(kTextaddressy) = 182;
	data.byte(kTextlen) = 240;
	redrawMainScrn();
	workToScreenM();
	data.byte(kGetback) = 4;
}

void DreamBase::actualLoad() {
	if (data.byte(kCommandtype) != 221) {
		data.byte(kCommandtype) = 221;
		commandOnly(41);
	}

	if (data.word(kMousebutton) == data.word(kOldbutton) || data.word(kMousebutton) != 1)
		return;

	unsigned int slot = data.byte(kCurrentslot);

	const char *desc = &_saveNames[17*slot];
	if (desc[1] == 0) // The actual description string starts at desc[1]
		return;

	loadPosition(data.byte(kCurrentslot));
	data.byte(kGetback) = 1;
}

void DreamBase::savePosition(unsigned int slot, const char *descbuf) {

	const Room &currentRoom = g_roomData[data.byte(kLocation)];

	Room madeUpRoom = currentRoom;
	madeUpRoom.roomsSample = data.byte(kRoomssample);
	madeUpRoom.mapX = data.byte(kMapx);
	madeUpRoom.mapY = data.byte(kMapy);
	madeUpRoom.liftFlag = data.byte(kLiftflag);
	madeUpRoom.b21 = data.byte(kManspath);
	madeUpRoom.facing = data.byte(kFacing);
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
	                  4*kNumchanges, 48, kNumReelRoutines*8+1 };
	for (int i = 0; i < 6; ++i)
		header.setLen(i, len[i]);

	// Write a new section with data that we need for ScummVM (version,
	// thumbnail, played time etc). We don't really care for its size,
	// so we just set it to a magic number.
	header.setLen(6, SCUMMVM_BLOCK_MAGIC_SIZE);

	outSaveFile->write((const uint8 *)&header, sizeof(FileHeader));
	outSaveFile->write(descbuf, len[0]);
	outSaveFile->write(data.ptr(kStartvars, len[1]), len[1]);
	outSaveFile->write(getSegment(data.word(kExtras)).ptr(kExframedata, len[2]), len[2]);
	outSaveFile->write(_listOfChanges, len[3]);

	// len[4] == 48, which is sizeof(Room) plus 16 for 'Roomscango'
	outSaveFile->write((const uint8 *)&madeUpRoom, sizeof(Room));
	outSaveFile->write(data.ptr(kRoomscango, 16), 16);

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
	data.word(kTimecount) = 0;
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
	inSaveFile->read(getSegment(data.word(kExtras)).ptr(kExframedata, len[2]), len[2]);
	inSaveFile->read(_listOfChanges, len[3]);

	// len[4] == 48, which is sizeof(Room) plus 16 for 'Roomscango'
	// Note: the values read into g_madeUpRoomDat are only used in actualLoad,
	// which is (almost) immediately called after this function
	inSaveFile->read((uint8 *)&g_madeUpRoomDat, sizeof(Room));
	inSaveFile->read(data.ptr(kRoomscango, 16), 16);

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
	if (data.byte(kCommandtype) != 252) {
		data.byte(kCommandtype) = 252;
		commandOnly(48);
	}

	if (!(data.word(kMousebutton) & 1))
		return;

	doLoad(-1);

	if (data.byte(kGetback) == 4 || quitRequested())
		return;

	showDecisions();
	workToScreenM();
	data.byte(kGetback) = 0;
}

void DreamBase::showDecisions() {
	createPanel2();
	showOpBox();
	showFrame(tempGraphics(), kOpsx + 17, kOpsy + 13, 6, 0);
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

		if (slot != data.byte(kCurrentslot)) {
			printDirect((const uint8 *)name.c_str(), kOpsx + 21, kOpsy + 10*slot + 10, 200, false);
			continue;
		}
		if (data.byte(kLoadingorsave) != 2) {
			data.word(kCharshift) = 91;
			printDirect((const uint8 *)name.c_str(), kOpsx + 21, kOpsy + 10*slot + 10, 200, false);
			data.word(kCharshift) = 0;
			continue;
		}

		int pos = name.size();
		data.byte(kCursorpos) = pos;
		name += '/'; // cursor character
		printDirect((const uint8 *)name.c_str(), kOpsx + 21, kOpsy + 10*slot + 10, 200, false);
	}
}

void DreamBase::checkInput() {
	if (data.byte(kLoadingorsave) == 3)
		return;

	readKey();

	// The first character of the savegame name is unused
	char *name = &_saveNames[17*data.byte(kCurrentslot) + 1];

	if (data.byte(kCurrentkey) == 0) {
		return;
	} else if (data.byte(kCurrentkey) == 13) {
		data.byte(kLoadingorsave) = 3;
	} else if (data.byte(kCurrentkey) == 8) {
		if (data.byte(kCursorpos) == 0)
			return;

		--data.byte(kCursorpos);
		name[data.byte(kCursorpos)] = 0;
		name[data.byte(kCursorpos)+1] = 1;
	} else {
		if (data.byte(kCursorpos) == 14)
			return;

		name[data.byte(kCursorpos)] = data.byte(kCurrentkey);
		name[data.byte(kCursorpos)+1] = 0;
		name[data.byte(kCursorpos)+2] = 1;
		++data.byte(kCursorpos);
	}

	showOpBox();
	showNames();
	showSlots();
	showSaveOps();
	workToScreenM();
}

void DreamBase::selectSlot() {
	if (data.byte(kCommandtype) != 244) {
		data.byte(kCommandtype) = 244;
		commandOnly(45);
	}

	if (data.word(kMousebutton) != 1 || data.word(kMousebutton) == data.word(kOldbutton))
		return; // noselslot
	if (data.byte(kLoadingorsave) == 3)
		data.byte(kLoadingorsave)--;

	oldToNames();
	int y = data.word(kMousey) - (kOpsy + 4);
	if (y < 11)
		data.byte(kCurrentslot) = 0;
	else
		data.byte(kCurrentslot) = y / 11;

	delPointer();
	showOpBox();
	showSlots();
	showNames();
	if (data.byte(kLoadingorsave) == 1)
		showLoadOps();
	else
		showSaveOps();
	readMouse();
	showPointer();
	workToScreenCPP();
	delPointer();
}

void DreamBase::showSlots() {
	showFrame(tempGraphics(), kOpsx + 7, kOpsy + 8, 2, 0);

	uint16 y = kOpsy + 11;

	for (int slot = 0; slot < 7; slot++) {
		if (slot == data.byte(kCurrentslot))
			showFrame(tempGraphics(), kOpsx + 10, y, 3, 0);

		y += 10;
	}
}

void DreamBase::showOpBox() {
	showFrame(tempGraphics(), kOpsx, kOpsy, 0, 0);

	// CHECKME: There seem to be versions of dreamweb in which this call
	// should be removed. It displays a red dot on the ops dialogs if left in.
	showFrame(tempGraphics(), kOpsx, kOpsy + 55, 4, 0);
}

void DreamBase::showLoadOps() {
	showFrame(tempGraphics(), kOpsx + 128 + 4, kOpsy + 12, 1, 0);
	showFrame(tempGraphics(), kOpsx + 176 + 2, kOpsy + 60 - 4, 5, 0);
	printMessage(kOpsx + 104, kOpsy + 14, 55, 101, (101 & 1));
}

void DreamBase::showSaveOps() {
	showFrame(tempGraphics(), kOpsx + 128 + 4, kOpsy + 12, 1, 0);
	showFrame(tempGraphics(), kOpsx + 176 + 2, kOpsy + 60 - 4, 5, 0);
	printMessage(kOpsx + 104, kOpsy + 14, 54, 101, (101 & 1));
}

} // End of namespace DreamGen
