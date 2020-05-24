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

#include "prince/prince.h"
#include "prince/graphics.h"
#include "prince/flags.h"
#include "prince/script.h"
#include "prince/hero.h"

#include "common/savefile.h"
#include "common/system.h"
#include "common/config-manager.h"
#include "common/memstream.h"
#include "common/translation.h"

#include "graphics/thumbnail.h"
#include "graphics/surface.h"
#include "graphics/palette.h"
#include "graphics/scaler.h"

#include "gui/saveload.h"

namespace Prince {

#define kSavegameVersion 1

class InterpreterFlags;
class Interpreter;

bool PrinceEngine::scummVMSaveLoadDialog(bool isSave) {
	GUI::SaveLoadChooser *dialog;
	Common::String desc;
	int slot;

	if (isSave) {
		dialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"), true);

		slot = dialog->runModalWithCurrentTarget();
		desc = dialog->getResultString();

		if (desc.empty()) {
			desc = dialog->createDefaultSaveDescription(slot);
		}
	} else {
		dialog = new GUI::SaveLoadChooser(_("Restore game:"), _("Restore"), false);
		slot = dialog->runModalWithCurrentTarget();
	}

	delete dialog;

	if (slot < 0)
		return false;

	if (isSave) {
		return saveGameState(slot, desc).getCode() == Common::kNoError;
	} else {
		return loadGameState(slot).getCode() == Common::kNoError;
	}
}

WARN_UNUSED_RESULT bool PrinceEngine::readSavegameHeader(Common::InSaveFile *in, SavegameHeader &header, bool skipThumbnail) {
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
	header.playTime    = in->readUint32LE();

	return true;
}

bool PrinceEngine::canSaveGameStateCurrently() {
	if (_mouseFlag && _mouseFlag != 3) {
		if (_mainHero->_visible) {
			// 29 - Basement
			if (_locationNr != 29) {
				// No dialog box and not in inventory
				if (!_dialogFlag && !_showInventoryFlag) {
					return true;
				}
			}
		}
	}
	return false;
}

bool PrinceEngine::canLoadGameStateCurrently() {
	if (_mouseFlag && _mouseFlag != 3) {
		if (_mainHero->_visible) {
			// 29 - Basement
			if (_locationNr != 29) {
				// No dialog box and not in inventory
				if (!_dialogFlag && !_showInventoryFlag) {
					return true;
				}
			}
		}
	}
	return false;
}

Common::Error PrinceEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	// Set up the serializer
	Common::String slotName = getSaveStateName(slot);
	Common::OutSaveFile *saveFile = g_system->getSavefileManager()->openForSaving(slotName);

	// Write out the ScummVM savegame header
	SavegameHeader header;
	header.saveName = desc;
	header.version = kSavegameVersion;
	writeSavegameHeader(saveFile, header);

	// Write out the data of the savegame
	syncGame(nullptr, saveFile);

	// Finish writing out game data
	saveFile->finalize();
	delete saveFile;

	return Common::kNoError;
}

void PrinceEngine::writeSavegameHeader(Common::OutSaveFile *out, SavegameHeader &header) {
	// Write out a savegame header
	out->write(kSavegameStr, kSavegameStrSize + 1);

	out->writeByte(kSavegameVersion);

	// Write savegame name
	out->write(header.saveName.c_str(), header.saveName.size() + 1);

	// Get the active palette
	uint8 thumbPalette[256 * 3];
	_system->getPaletteManager()->grabPalette(thumbPalette, 0, 256);

	// Create a thumbnail and save it
	Graphics::Surface *thumb = new Graphics::Surface();
	Graphics::Surface *s = _graph->_frontScreen; // check inventory / map etc..
	::createThumbnail(thumb, (const byte *)s->getPixels(), s->w, s->h, thumbPalette);
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

void PrinceEngine::syncGame(Common::SeekableReadStream *readStream, Common::WriteStream *writeStream) {
	int emptyRoom = 0x00;
	int normRoom = 0xFF;
	byte endInv = 0xFF;

	Common::Serializer s(readStream, writeStream);

	if (s.isSaving()) {
		// Flag values
		for (int i = 0; i < _flags->kMaxFlags; i++) {
			uint32 value = _flags->getFlagValue((Flags::Id)(_flags->kFlagMask + i));
			s.syncAsUint32LE(value);
		}

		// Dialog data
		for (uint32 i = 0; i < _dialogDatSize; i++) {
			byte value = _dialogDat[i];
			s.syncAsByte(value);
		}

		// Location number
		s.syncAsUint16LE(_locationNr);

		// Rooms
		for (int roomId = 0; roomId < _script->kMaxRooms; roomId++) {
			Room *room = new Room();
			room->loadRoom(_script->getRoomOffset(roomId));

			if (room->_mobs) {
				s.syncAsByte(normRoom);
			} else {
				s.syncAsByte(emptyRoom);
				delete room;
				continue;
			}

			// Mobs
			for (int mobId = 0; mobId < kMaxMobs; mobId++) {
				byte value = _script->getMobVisible(room->_mobs, mobId);
				s.syncAsByte(value);
			}

			// Background animations
			for (int backAnimSlot = 0; backAnimSlot < kMaxBackAnims; backAnimSlot++) {
				uint32 value = _script->getBackAnimId(room->_backAnim, backAnimSlot);
				s.syncAsUint32LE(value);
			}

			// Objects
			for (int objectSlot = 0; objectSlot < kMaxObjects; objectSlot++) {
				byte value = _script->getObjId(room->_obj, objectSlot);
				s.syncAsByte(value);
			}

			delete room;
		}

		// Main hero
		s.syncAsUint16LE(_mainHero->_visible);
		s.syncAsUint16LE(_mainHero->_middleX);
		s.syncAsUint16LE(_mainHero->_middleY);
		s.syncAsUint16LE(_mainHero->_lastDirection);
		s.syncAsUint32LE(_mainHero->_color);
		s.syncAsUint16LE(_mainHero->_maxBoredom);
		s.syncAsUint32LE(_mainHero->_animSetNr);

		for (uint inv1Slot = 0; inv1Slot < _mainHero->_inventory.size(); inv1Slot++) {
			s.syncAsByte(_mainHero->_inventory[inv1Slot]);
		}
		s.syncAsByte(endInv);

		for (uint inv2Slot = 0; inv2Slot < _mainHero->_inventory2.size(); inv2Slot++) {
			s.syncAsByte(_mainHero->_inventory2[inv2Slot]);
		}
		s.syncAsByte(endInv);

		// Second hero
		s.syncAsUint16LE(_secondHero->_visible);
		s.syncAsUint16LE(_secondHero->_middleX);
		s.syncAsUint16LE(_secondHero->_middleY);
		s.syncAsUint16LE(_secondHero->_lastDirection);
		s.syncAsUint32LE(_secondHero->_color);
		s.syncAsUint16LE(_secondHero->_maxBoredom);
		s.syncAsUint32LE(_secondHero->_animSetNr);

		for (uint inv1Slot = 0; inv1Slot < _secondHero->_inventory.size(); inv1Slot++) {
			s.syncAsByte(_secondHero->_inventory[inv1Slot]);
		}
		s.syncAsByte(endInv);

		for (uint inv2Slot = 0; inv2Slot < _secondHero->_inventory2.size(); inv2Slot++) {
			s.syncAsByte(_secondHero->_inventory2[inv2Slot]);
		}
		s.syncAsByte(endInv);

	} else {
		// Cursor reset
		changeCursor(1);
		_currentPointerNumber = 1;

		// Flag values
		for (int i = 0; i < _flags->kMaxFlags; i++) {
			uint32 value = 0;
			s.syncAsUint32LE(value);
			_flags->setFlagValue((Flags::Id)(_flags->kFlagMask + i), value);
		}

		// Dialog data
		for (uint32 i = 0; i < _dialogDatSize; i++) {
			byte value = 0;
			s.syncAsByte(value);
			_dialogDat[i] = value;
		}

		// Location number
		int restoreRoom = 0;
		s.syncAsUint16LE(restoreRoom);
		_flags->setFlagValue(Flags::RESTOREROOM, restoreRoom);

		// Rooms
		for (int roomId = 0; roomId < _script->kMaxRooms; roomId++) {
			Room *room = new Room();
			room->loadRoom(_script->getRoomOffset(roomId));

			byte roomType = emptyRoom;
			s.syncAsByte(roomType);
			if (roomType == emptyRoom) {
				delete room;
				continue;
			}

			// Mobs
			for (int mobId = 0; mobId < kMaxMobs; mobId++) {
				byte value = 0;
				s.syncAsByte(value);
				_script->setMobVisible(room->_mobs, mobId, value);
			}

			// Background animations
			for (int backAnimSlot = 0; backAnimSlot < kMaxBackAnims; backAnimSlot++) {
				uint32 value = 0;
				s.syncAsUint32LE(value);
				_script->setBackAnimId(room->_backAnim, backAnimSlot, value);
			}

			// Objects
			for (int objectSlot = 0; objectSlot < kMaxObjects; objectSlot++) {
				byte value = 0;
				s.syncAsByte(value);
				_script->setObjId(room->_obj, objectSlot, value);
			}

			delete room;
		}

		// Main hero
		s.syncAsUint16LE(_mainHero->_visible);
		s.syncAsUint16LE(_mainHero->_middleX);
		s.syncAsUint16LE(_mainHero->_middleY);
		s.syncAsUint16LE(_mainHero->_lastDirection);
		s.syncAsUint32LE(_mainHero->_color);
		s.syncAsUint16LE(_mainHero->_maxBoredom);
		s.syncAsUint32LE(_mainHero->_animSetNr);
		_mainHero->loadAnimSet(_mainHero->_animSetNr);

		_mainHero->_inventory.clear();
		byte invId = endInv;
		while (1) {
			s.syncAsByte(invId);
			if (invId == endInv) {
				break;
			}
			_mainHero->_inventory.push_back(invId);
		}

		_mainHero->_inventory2.clear();
		invId = endInv;
		while (1) {
			s.syncAsByte(invId);
			if (invId == endInv) {
				break;
			}
			_mainHero->_inventory2.push_back(invId);
		}

		// Second hero
		s.syncAsUint16LE(_secondHero->_visible);
		s.syncAsUint16LE(_secondHero->_middleX);
		s.syncAsUint16LE(_secondHero->_middleY);
		s.syncAsUint16LE(_secondHero->_lastDirection);
		s.syncAsUint32LE(_secondHero->_color);
		s.syncAsUint16LE(_secondHero->_maxBoredom);
		s.syncAsUint32LE(_secondHero->_animSetNr);
		_secondHero->loadAnimSet(_secondHero->_animSetNr);

		_secondHero->_inventory.clear();
		invId = endInv;
		while (1) {
			s.syncAsByte(invId);
			if (invId == endInv) {
				break;
			}
			_secondHero->_inventory.push_back(invId);
		}

		_secondHero->_inventory2.clear();
		invId = endInv;
		while (1) {
			s.syncAsByte(invId);
			if (invId == endInv) {
				break;
			}
			_secondHero->_inventory2.push_back(invId);
		}

		// Script
		_interpreter->setBgOpcodePC(0);
		_interpreter->setFgOpcodePC(_script->_scriptInfo.restoreGame);

	}
}

Common::Error PrinceEngine::loadGameState(int slot) {
	if (!loadGame(slot)) {
		return Common::kReadingFailed;
	}
	return Common::kNoError;
}

bool PrinceEngine::loadGame(int slotNumber) {
	Common::MemoryReadStream *readStream;

	// Open up the savegame file
	Common::String slotName = getSaveStateName(slotNumber);
	Common::InSaveFile *saveFile = g_system->getSavefileManager()->openForLoading(slotName);

	if (!saveFile)
		return false;

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

	// Get in the savegame
	syncGame(readStream, nullptr);
	delete readStream;

	return true;
}

} // End of namespace Prince
