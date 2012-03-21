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
 */

/*
 * This code is based on original Mortville Manor DOS source code
 * Copyright (c) 1988-1989 Lankhor
 */

#include "common/file.h"
#include "common/system.h"
#include "mortevielle/dialogs.h"
#include "mortevielle/mor.h"
#include "mortevielle/mortevielle.h"
#include "mortevielle/mouse.h"
#include "mortevielle/ovd1.h"
#include "mortevielle/saveload.h"
#include "mortevielle/var_mor.h"

namespace Mortevielle {

static const char SAVEGAME_ID[4] = { 'M', 'O', 'R', 'T' };

Common::String SavegameManager::generateSaveName(int slotNumber) {
	return Common::String::format("sav%d.mor", slotNumber);
}

/**
 * Handle saving or loading savegame data
 */
void SavegameManager::sync_save(Common::Serializer &sz) {
	sz.syncAsSint16LE(g_s1._faithScore);
	for (int i = 0; i < 11; ++i)
		sz.syncAsByte(g_s1._pourc[i]);
	for (int i = 0; i < 43; ++i)
		sz.syncAsByte(g_s1._teauto[i]);
	for (int i = 0; i < 31; ++i)
		sz.syncAsByte(g_s1._sjer[i]);

	sz.syncAsSint16LE(g_s1._currPlace);
	sz.syncAsSint16LE(g_s1._atticBallHoleObjectId);
	sz.syncAsSint16LE(g_s1._atticRodHoleObjectId);
	sz.syncAsSint16LE(g_s1._cellarObjectId);
	sz.syncAsSint16LE(g_s1._secretPassageObjectId);
	sz.syncAsSint16LE(g_s1._wellObjectId);
	sz.syncAsSint16LE(g_s1._selectedObjectId);
	sz.syncAsSint16LE(g_s1._purpleRoomObjectId);
	sz.syncAsSint16LE(g_s1._cryptObjectId);
	sz.syncAsByte(g_s1._alreadyEnteredManor);
	sz.syncAsByte(g_s1._fullHour);

	sz.syncBytes(g_bufcha, 391);
}

/**
 * Inner code for loading a saved game
 * @remarks	Originally called 'takesav'
 */
void SavegameManager::loadSavegame(int n) {
	// -- Load the file
	Common::String filename = generateSaveName(n);

	// Try loading first from the save area
	Common::SeekableReadStream *stream = g_system->getSavefileManager()->openForLoading(filename);

	// If not present, try loading from the program folder
	Common::File f;
	if (stream == NULL) {
		if (!f.open(filename))
			error("Unable to open save file '%s'", filename.c_str());

		stream = f.readStream(f.size());
		f.close();
	}

	// Check whether it's a ScummVM saved game
	char buffer[4];
	stream->read(buffer, 4);
	if (!strncmp(&buffer[0], &SAVEGAME_ID[0], 4)) {
		// Yes, it is, so skip over the savegame header
		SavegameHeader header;
		readSavegameHeader(stream, header);
		delete header.thumbnail;
	} else {
		stream->seek(0);
	}

	// Read the game contents
	Common::Serializer sz(stream, NULL);
	sync_save(sz);

	g_s = g_s1;
	for (int i = 0; i <= 389; ++i)
		g_tabdon[i + kAcha] = g_bufcha[i];

	// Close the stream
	delete stream;	
}

/**
 * Load a saved game
 */
Common::Error SavegameManager::loadGame(int n) {
	g_vm->_mouse.hideMouse();
	maivid();
	loadSavegame(n);
	
	/* Initialization */
	g_vm->charToHour();
	g_vm->initGame();
	g_vm->gameLoaded();
	g_vm->_mouse.showMouse();
	return Common::kNoError;
}

/**
 * Save the game
 */
Common::Error SavegameManager::saveGame(int n, const Common::String &saveName) {
	Common::OutSaveFile *f;
	int i;

	g_vm->_mouse.hideMouse();
	g_vm->hourToChar();
	
	for (i = 0; i <= 389; ++i)
		g_bufcha[i] = g_tabdon[i + kAcha];
	g_s1 = g_s;
	if (g_s1._currPlace == 26)
		g_s1._currPlace = 15;
	
	Common::String filename = generateSaveName(n);
	f = g_system->getSavefileManager()->openForSaving(filename);

	// Write out the savegame header
	f->write(&SAVEGAME_ID[0], 4);

	// Write out the header
	SavegameHeader header;
	writeSavegameHeader(f, saveName);

	// Write out the savegame contents
	Common::Serializer sz(NULL, f);
	sync_save(sz);

	// Close the save file
	f->finalize();
	delete f;
	
	// Skipped: dialog asking to swap floppy

	g_vm->_mouse.showMouse();
	return Common::kNoError;
}

void SavegameManager::writeSavegameHeader(Common::OutSaveFile *out, const Common::String &saveName) {
	// Write out a savegame header
	out->writeByte(SAVEGAME_VERSION);

	// Write savegame name
	out->writeString(saveName);
	out->writeByte(0);

	// Get the active palette
	uint8 thumbPalette[256 * 3];
	g_system->getPaletteManager()->grabPalette(thumbPalette, 0, 256);

	// Create a thumbnail and save it
	Graphics::Surface *thumb = new Graphics::Surface();
	Graphics::Surface s = g_vm->_screenSurface.lockArea(Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));

	::createThumbnail(thumb, (const byte *)s.pixels, SCREEN_WIDTH, SCREEN_HEIGHT, thumbPalette);
	Graphics::saveThumbnail(*out, *thumb);
	thumb->free();
	delete thumb;

	// Write out the save date/time
	TimeDate td;
	g_system->getTimeAndDate(td);
	out->writeSint16LE(td.tm_year + 1900);
	out->writeSint16LE(td.tm_mon + 1);
	out->writeSint16LE(td.tm_mday);
	out->writeSint16LE(td.tm_hour);
	out->writeSint16LE(td.tm_min);
}

bool SavegameManager::readSavegameHeader(Common::InSaveFile *in, SavegameHeader &header) {
	header.thumbnail = NULL;

	// Get the savegame version
	header.version = in->readByte();

	// Read in the save name
	header.saveName.clear();
	char ch;
	while ((ch = (char)in->readByte()) != '\0')
		header.saveName += ch;

	// Get the thumbnail
	header.thumbnail = Graphics::loadThumbnail(*in);
	if (!header.thumbnail)
		return false;

	// Read in save date/time
	header.saveYear = in->readSint16LE();
	header.saveMonth = in->readSint16LE();
	header.saveDay = in->readSint16LE();
	header.saveHour = in->readSint16LE();
	header.saveMinutes = in->readSint16LE();

	return true;
}

SaveStateList SavegameManager::listSaves(const char *target) {
	Common::String pattern = "sav*.mor";
	Common::StringArray files = g_system->getSavefileManager()->listSavefiles(pattern);
	sort(files.begin(), files.end());	// Sort (hopefully ensuring we are sorted numerically..)

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = files.begin(); file != files.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		const Common::String &fname = *file;
		int slotNumber = atoi(fname.c_str() + 3);

		Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(fname);
		if (in) {
			// There can be two types of savegames: original interpreter savegames, and ScummVM savegames.
			// Original interpreter savegames are 497 bytes, and still need to be supported because the
			// initial game state is stored as a savegame
			bool validFlag = false;
			Common::String saveDescription;

			char buffer[4];
			in->read(buffer, 4);
			if (!strncmp(&buffer[0], &SAVEGAME_ID[0], 4)) {
				// ScummVm savegame. Read in the header to get the savegame name
				SavegameHeader header;
				validFlag = readSavegameHeader(in, header);
				
				if (validFlag) {
					delete header.thumbnail;
					saveDescription = header.saveName;
				}
			} else if (file->size() == 497) {
				// Form an appropriate savegame name
				saveDescription = (slotNumber == 0) ? "Initial game state" : 
					Common::String::format("Savegame #%d", slotNumber);
				validFlag = true;
			}
				
			if (validFlag)
				// Got a valid savegame
				saveList.push_back(SaveStateDescriptor(slotNumber, saveDescription));

			delete in;
		}
	}

	return saveList;
}

SaveStateDescriptor SavegameManager::querySaveMetaInfos(int slot) {
	Common::String fileName = Mortevielle::SavegameManager::generateSaveName(slot);
	Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(fileName);
	
	if (f) {
		// Check to see if it's a ScummVM savegame or not
		char buffer[4];
		f->read(buffer, 4);

		bool hasHeader = !strncmp(&buffer[0], &SAVEGAME_ID[0], 4);

		if (!hasHeader) {
			// Original savegame perhaps?
			delete f;

			SaveStateDescriptor desc(slot, Common::String::format("Savegame #%d", slot));
			desc.setDeletableFlag(slot != 0);
			desc.setWriteProtectedFlag(slot == 0);
			return desc;
		} else {
			// Get the savegame header information
			SavegameHeader header;
			readSavegameHeader(f, header);
			delete f;

			// Create the return descriptor
			SaveStateDescriptor desc(slot, header.saveName);
			desc.setDeletableFlag(true);
			desc.setWriteProtectedFlag(false);
			desc.setThumbnail(header.thumbnail);
			desc.setSaveDate(header.saveYear, header.saveMonth, header.saveDay);
			desc.setSaveTime(header.saveHour, header.saveMinutes);

			return desc;
		}
	}
	
	return SaveStateDescriptor();
}

} // End of namespace Mortevielle
