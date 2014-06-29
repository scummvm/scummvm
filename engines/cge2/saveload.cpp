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

/*
 * This code is based on original Sfinx source code
 * Copyright (c) 1994-1997 Janus B. Wisniewski and L.K. Avalon
 */

#include "cge2/detection.h"
#include "common/config-manager.h"
#include "common/savefile.h"
#include "common/system.h"
#include "graphics/thumbnail.h"
#include "graphics/surface.h"
#include "graphics/palette.h"
#include "graphics/scaler.h"
#include "cge2/events.h"
#include "cge2/snail.h"
#include "cge2/hero.h"
#include "cge2/text.h"

namespace CGE2 {

#define kSavegameCheckSum (1997 + _now + _music + kWorldHeight)
#define kBadSVG           99

struct SavegameHeader {
	uint8 version;
	Common::String saveName;
	Graphics::Surface *thumbnail;
	int saveYear, saveMonth, saveDay;
	int saveHour, saveMinutes;
};

int CGE2MetaEngine::getMaximumSaveSlot() const {
	return 99;
}

SaveStateList CGE2MetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern = target;
	pattern += ".???";

	filenames = saveFileMan->listSavefiles(pattern);
	sort(filenames.begin(), filenames.end());   // Sort (hopefully ensuring we are sorted numerically..)

	SaveStateList saveList;
	for (Common::StringArray::const_iterator filename = filenames.begin(); filename != filenames.end(); ++filename) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(filename->c_str() + filename->size() - 3);

		if (slotNum >= 0 && slotNum <= 99) {

			Common::InSaveFile *file = saveFileMan->openForLoading(*filename);
			if (file) {
				CGE2::SavegameHeader header;

				// Check to see if it's a ScummVM savegame or not
				char buffer[kSavegameStrSize + 1];
				file->read(buffer, kSavegameStrSize + 1);

				if (!strncmp(buffer, kSavegameStr, kSavegameStrSize + 1)) {
					// Valid savegame
					if (CGE2::CGE2Engine::readSavegameHeader(file, header)) {
						saveList.push_back(SaveStateDescriptor(slotNum, header.saveName));
						if (header.thumbnail) {
							header.thumbnail->free();
							delete header.thumbnail;
						}
					}
				} else {
					// Must be an original format savegame
					saveList.push_back(SaveStateDescriptor(slotNum, "Unknown"));
				}

				delete file;
			}
		}
	}

	return saveList;
}

SaveStateDescriptor CGE2MetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(fileName);

	if (f) {
		CGE2::SavegameHeader header;

		// Check to see if it's a ScummVM savegame or not
		char buffer[kSavegameStrSize + 1];
		f->read(buffer, kSavegameStrSize + 1);

		bool hasHeader = !strncmp(buffer, kSavegameStr, kSavegameStrSize + 1) &&
			CGE2::CGE2Engine::readSavegameHeader(f, header);
		delete f;

		if (!hasHeader) {
			// Original savegame perhaps?
			SaveStateDescriptor desc(slot, "Unknown");
			return desc;
		} else {
			// Create the return descriptor
			SaveStateDescriptor desc(slot, header.saveName);
			desc.setThumbnail(header.thumbnail);
			desc.setSaveDate(header.saveYear, header.saveMonth, header.saveDay);
			desc.setSaveTime(header.saveHour, header.saveMinutes);

			return desc;
		}
	}

	return SaveStateDescriptor();
}

bool CGE2Engine::readSavegameHeader(Common::InSaveFile *in, SavegameHeader &header) {
	header.thumbnail = nullptr;

	// Get the savegame version
	header.version = in->readByte();
	if (header.version > kSavegameVersion)
		return false;

	// Read in the string
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

void CGE2MetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	g_system->getSavefileManager()->removeSavefile(fileName);
}

bool CGE2Engine::canSaveGameStateCurrently() {
	bool isHeroVisible = false;
	for (int i = 0; i < 2; i++) {
		isHeroVisible = !_heroTab[i]->_ptr->_flags._hide;
		if (isHeroVisible)
			break;
	}
	return (_startupMode == 0) && _mouse->_active &&
		_commandHandler->idle() && isHeroVisible;
}

bool CGE2Engine::canLoadGameStateCurrently() {
	return (_startupMode == 0) && _mouse->_active;
}

Common::Error CGE2Engine::saveGameState(int slot, const Common::String &desc) {
	// Set up the serializer
	Common::String slotName = generateSaveName(slot);
	Common::OutSaveFile *saveFile = g_system->getSavefileManager()->openForSaving(slotName);

	// Write out the ScummVM savegame header
	SavegameHeader header;
	header.saveName = desc;
	header.version = kSavegameVersion;
	writeSavegameHeader(saveFile, header);

	// Write out the data of the savegame
	syncGame(NULL, saveFile);

	// Finish writing out game data
	saveFile->finalize();
	delete saveFile;

	return Common::kNoError;
}

/**
* Support method that generates a savegame name
* @param slot		Slot number
*/
Common::String CGE2Engine::generateSaveName(int slot) {
	return Common::String::format("%s.%03d", _targetName.c_str(), slot);
}

void CGE2Engine::writeSavegameHeader(Common::OutSaveFile *out, SavegameHeader &header) {
	// Write out a savegame header
	out->write(kSavegameStr, kSavegameStrSize + 1);

	out->writeByte(kSavegameVersion);

	// Write savegame name
	out->write(header.saveName.c_str(), header.saveName.size() + 1);

	// Get the active palette
	uint8 thumbPalette[256 * 3];
	g_system->getPaletteManager()->grabPalette(thumbPalette, 0, 256);

	// Create a thumbnail and save it
	Graphics::Surface *thumb = new Graphics::Surface();
	Graphics::Surface *s = _vga->_page[0];
	::createThumbnail(thumb, (const byte *)s->getPixels(), kScrWidth, kScrHeight, thumbPalette);
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

void CGE2Engine::syncGame(Common::SeekableReadStream *readStream, Common::WriteStream *writeStream) {
	Common::Serializer s(readStream, writeStream);

	// Synchronise header data
	syncHeader(s);

	if (s.isSaving()) {
		for (int i = 0; i < kSceneMax; i++)
			_eyeTab[i]->sync(s);

		_spare->sync(s);

		// The references of the items in the heroes pockets:
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < kPocketMax; j++) {
				Sprite *spr = _heroTab[i]->_pocket[j];
				int ref = (spr) ? spr->_ref : -1;
				s.syncAsSint16LE(ref);
			}
		}
		
		for (int i = 0; i < kMaxPoint; i++)
			_point[i]->sync(s);
	} else {
		// Loading game
	}
}

void CGE2Engine::syncHeader(Common::Serializer &s) {
	s.syncAsUint16LE(_now);
	s.syncAsUint16LE(_sex);
	s.syncAsUint16LE(_music);
	s.syncAsUint16LE(_waitSeq);
	s.syncAsUint16LE(_waitRef);
	s.syncAsUint16LE(_sayCap);
	s.syncAsUint16LE(_sayVox);
	for (int i = 0; i < 4; i++)
		s.syncAsUint16LE(_flag[i]);

	if (s.isLoading()) {
		// Reset scene values
		//initSceneValues();
	}

	if (s.isSaving()) {
		// Write checksum
		int checksum = kSavegameCheckSum;
		s.syncAsUint16LE(checksum);
	} else {
		// Read checksum and validate it
		uint16 checksum = 0;
		s.syncAsUint16LE(checksum);
		if (checksum != kSavegameCheckSum)
			error("%s", _text->getText(kBadSVG));
	}
}

Common::Error CGE2Engine::loadGameState(int slot) {
	warning("STUB: CGE2Engine::loadGameState()");
	return Common::kNoError;
}

} // End of namespace CGE2
