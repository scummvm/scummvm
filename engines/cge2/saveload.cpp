/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * This code is based on original Sfinx source code
 * Copyright (c) 1994-1997 Janusz B. Wisniewski and L.K. Avalon
 */

#include "common/memstream.h"
#include "common/savefile.h"
#include "common/system.h"
#include "graphics/thumbnail.h"
#include "graphics/surface.h"
#include "graphics/paletteman.h"
#include "graphics/scaler.h"
#include "cge2/events.h"
#include "cge2/snail.h"
#include "cge2/hero.h"
#include "cge2/text.h"

namespace CGE2 {

#define kSavegameCheckSum (1997 + _now + _sex + kWorldHeight)
#define kBadSVG           99

bool CGE2Engine::canSaveGameStateCurrently(Common::U32String *msg) {
	return (_gamePhase == kPhaseInGame) && _mouse->_active &&
		_commandHandler->idle() && (_soundStat._wait == nullptr);
}

Common::Error CGE2Engine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	storeHeroPos();
	saveGame(slot, desc);
	sceneUp(_now);
	return Common::kNoError;
}

void CGE2Engine::saveGame(int slotNumber, const Common::String &desc) {
	// Set up the serializer
	Common::String slotName = getSaveStateName(slotNumber);
	Common::OutSaveFile *saveFile = g_system->getSavefileManager()->openForSaving(slotName);

	// Write out the ScummVM savegame header
	SavegameHeader header;
	header.saveName = desc;
	header.version = kSavegameVersion;
	writeSavegameHeader(saveFile, header);

	// Write out the data of the savegame
	sceneDown();
	syncGame(nullptr, saveFile);

	// Finish writing out game data
	saveFile->finalize();
	delete saveFile;
}

bool CGE2Engine::canLoadGameStateCurrently(Common::U32String *msg) {
	return (_gamePhase == kPhaseInGame) && _mouse->_active;
}

Common::Error CGE2Engine::loadGameState(int slot) {
	_commandHandler->clear();
	_commandHandlerTurbo->clear();
	sceneDown();
	if (!loadGame(slot))
		return Common::kReadingFailed;
	sceneUp(_now);
	initToolbar();
	return Common::kNoError;
}

bool CGE2Engine::loadGame(int slotNumber) {
	Common::MemoryReadStream *readStream;

	// Open up the savegame file
	Common::String slotName = getSaveStateName(slotNumber);
	Common::InSaveFile *saveFile = g_system->getSavefileManager()->openForLoading(slotName);

	// Read the data into a data buffer
	int size = saveFile->size();
	byte *dataBuffer = (byte *)malloc(size);
	saveFile->read(dataBuffer, size);
	readStream = new Common::MemoryReadStream(dataBuffer, size, DisposeAfterUse::YES);
	delete saveFile;

	// Check to see if it's a ScummVM savegame or not
	char buffer[kSavegameStrSize + 1];
	readStream->read(buffer, kSavegameStrSize + 1);

	if (strncmp(buffer, kSavegameStr, kSavegameStrSize + 1) != 0) {
		delete readStream;
		return false;
	} else {
		SavegameHeader saveHeader;

		if (!readSavegameHeader(readStream, saveHeader)) {
			delete readStream;
			return false;
		}

		g_engine->setTotalPlayTime(saveHeader.playTime * 1000);
	}

	resetGame();

	// Get in the savegame
	syncGame(readStream, nullptr);
	delete readStream;

	loadHeroes();

	return true;
}

void CGE2Engine::resetGame() {
	_busyPtr = nullptr;
	busy(false);
	_spare->clear();
	_vga->_showQ->clear();
	loadScript("CGE.INI", true);
	delete _infoLine;
	_infoLine = new InfoLine(this, kInfoW);
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

	// Stop the heroes from moving and redraw them before taking the picture.
	for (int i = 0; i < 2; i++)
		_heroTab[i]->_ptr->park();
	_vga->show();

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

	out->writeUint32LE(g_engine->getTotalPlayTime() / 1000);
}

WARN_UNUSED_RESULT bool CGE2Engine::readSavegameHeader(Common::InSaveFile *in, SavegameHeader &header, bool skipThumbnail) {
	header.version     = 0;
	header.saveName.clear();
	header.thumbnail   = nullptr;
	header.saveYear    = 0;
	header.saveMonth   = 0;
	header.saveDay     = 0;
	header.saveHour    = 0;
	header.saveMinutes = 0;
	header.playTime    = 0;

	// Get the savegame version
	header.version = in->readByte();
	if (header.version > kSavegameVersion)
		return false;

	// Read in the string
	char ch;
	while ((ch = (char)in->readByte()) != '\0')
		header.saveName += ch;

	// Get the thumbnail
	if (!Graphics::loadThumbnail(*in, header.thumbnail, skipThumbnail)) {
		return false;
	}

	// Read in save date/time
	header.saveYear    = in->readSint16LE();
	header.saveMonth   = in->readSint16LE();
	header.saveDay     = in->readSint16LE();
	header.saveHour    = in->readSint16LE();
	header.saveMinutes = in->readSint16LE();

	if (header.version >= 2) {
		header.playTime = in->readUint32LE();
	}


	return true;
}

void CGE2Engine::syncGame(Common::SeekableReadStream *readStream, Common::WriteStream *writeStream) {
	Common::Serializer s(readStream, writeStream);

	// Synchronise header data
	syncHeader(s);

	// Synchronise _spare
	_spare->sync(s);

	if (s.isSaving()) {
		// Save the references of the items in the heroes pockets:
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < kPocketMax; j++) {
				int ref = _heroTab[i]->_downPocketId[j];
				s.syncAsSint16LE(ref);
			}
		}
	} else {
		// Load items to the pockets
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < kPocketMax; j++) {
				int ref = 0;
				s.syncAsSint16LE(ref);
				_heroTab[i]->_downPocketId[j] = ref;
			}
		}
	}

	// Heroes' _posTabs
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < kSceneMax; j++) {
			s.syncAsSint16LE(_heroTab[i]->_posTab[j]->x);
			s.syncAsSint16LE(_heroTab[i]->_posTab[j]->y);
		}
	}
}

void CGE2Engine::syncHeader(Common::Serializer &s) {
	s.syncAsUint16LE(_now);
	s.syncAsUint16LE(_sex);
	s.syncAsUint16LE(_vga->_rot._len);
	s.syncAsUint16LE(_waitSeq);
	s.syncAsUint16LE(_waitRef);

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

} // End of namespace CGE2
