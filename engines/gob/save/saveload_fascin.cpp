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
 * $URL$
 * $Id$
 *
 */

#include "gob/save/saveload.h"
#include "gob/save/saveconverter.h"
#include "gob/inter.h"
#include "gob/variables.h"

namespace Gob {

SaveLoad_Fascination::SaveFile SaveLoad_Fascination::_saveFiles[] = {
	{ "cat.cat",    kSaveModeSave, 0, "savegame catalog"},
	{ "save0.inf",  kSaveModeSave, 0, "savegame"},
	{ "save1.inf",  kSaveModeSave, 0, "savegame"},
	{ "save2.inf",  kSaveModeSave, 0, "savegame"},
	{ "save3.inf",  kSaveModeSave, 0, "savegame"},
	{ "save4.inf",  kSaveModeSave, 0, "savegame"},
	{ "save5.inf",  kSaveModeSave, 0, "savegame"},
	{ "save6.inf",  kSaveModeSave, 0, "savegame"},
	{ "save7.inf",  kSaveModeSave, 0, "savegame"},
	{ "save8.inf",  kSaveModeSave, 0, "savegame"},
	{ "save9.inf",  kSaveModeSave, 0, "savegame"},
	{ "save10.inf", kSaveModeSave, 0, "savegame"},
	{ "save11.inf", kSaveModeSave, 0, "savegame"},
	{ "save12.inf", kSaveModeSave, 0, "savegame"},
	{ "save13.inf", kSaveModeSave, 0, "savegame"},
	{ "save14.inf", kSaveModeSave, 0, "savegame"},
};


SaveLoad_Fascination::GameHandler::File::File(GobEngine *vm, const char *base) :
	SlotFileIndexed(vm, SaveLoad_Fascination::kSlotCount, base, "s") {
}

SaveLoad_Fascination::GameHandler::File::~File() {
}

int SaveLoad_Fascination::GameHandler::File::getSlot(int32 offset) const {
	uint32 varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return -1;

	return ((offset - kIndexSize) / varSize);
}

int SaveLoad_Fascination::GameHandler::File::getSlotRemainder(int32 offset) const {
	uint32 varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return -1;

	return ((offset - kIndexSize) % varSize);
}


SaveLoad_Fascination::GameHandler::GameHandler(GobEngine *vm, const char *target) : SaveHandler(vm) {
	memset(_index, 0, kIndexSize);
	_hasIndex = false;

	_slotFile = new File(vm, target);
}

SaveLoad_Fascination::GameHandler::~GameHandler() {
	delete _slotFile;
}

int32 SaveLoad_Fascination::GameHandler::getSize() {
	uint32 varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return -1;

	return _slotFile->tallyUpFiles(varSize, kIndexSize);
}

bool SaveLoad_Fascination::GameHandler::load(int16 dataVar, int32 size, int32 offset) {
	uint32 varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return false;

	if (size == 0) {
		// Indicator to load all variables
		dataVar = 0;
		size = varSize;
	}

	if (offset == 0) {
		// Save index

		if (((uint32) size) != kIndexSize) {
			warning("Requested index has wrong size (%d)", size);
			return false;
		}

		// Create/Fake the index
		buildIndex(_vm->_inter->_variables->getAddressOff8(dataVar));

	} else {
		// Save slot, whole variable block

		uint32 slot = _slotFile->getSlot(offset);
		int slotRem = _slotFile->getSlotRemainder(offset);

		debugC(2, kDebugSaveLoad, "Loading from slot %d", slot);

		if ((slot >= kSlotCount) || (slotRem != 0) ||
		    (dataVar != 0) || (((uint32) size) != varSize)) {

			warning("Invalid loading procedure (%d, %d, %d, %d, %d)",
					dataVar, size, offset, slot, slotRem);
			return false;
		}

		Common::String slotFile = _slotFile->build(slot);

		SaveReader *reader = 0;

		// New save, load directly
		reader = new SaveReader(2, slot, slotFile);

		SavePartInfo info(kSlotNameLength, (uint32) _vm->getGameType(), 0,
				_vm->getEndianness(), varSize);
		SavePartVars vars(_vm, varSize);

		if (!reader->load()) {
			delete reader;
			return false;
		}

		if (!reader->readPart(0, &info)) {
			delete reader;
			return false;
		}
		if (!reader->readPart(1, &vars)) {
			delete reader;
			return false;
		}

		// Get all variables
		if (!vars.writeInto(0, 0, varSize)) {
			delete reader;
			return false;
		}

		delete reader;
	}

	return true;
}

bool SaveLoad_Fascination::GameHandler::save(int16 dataVar, int32 size, int32 offset) {
	uint32 varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return false;

	if (size == 0) {
		// Indicator to save all variables
		dataVar = 0;
		size = varSize;
	}

	if (offset == 0) {
		// Save index

		if (((uint32) size) != kIndexSize) {
			warning("Requested index has wrong size (%d)", size);
			return false;
		}

		// Just copy the index into our buffer
		_vm->_inter->_variables->copyTo(dataVar, _index, kIndexSize);
		_hasIndex = true;

	} else {
		// Save slot, whole variable block

		uint32 slot = _slotFile->getSlot(offset);
		int slotRem = _slotFile->getSlotRemainder(offset);

		debugC(2, kDebugSaveLoad, "Saving to slot %d", slot);

		if ((slot >= kSlotCount) || (slotRem != 0) ||
		    (dataVar != 0) || (((uint32) size) != varSize)) {

			warning("Invalid saving procedure (%d, %d, %d, %d, %d)",
					dataVar, size, offset, slot, slotRem);
			return false;
		}

		// An index is needed for the save slot description
		if (!_hasIndex) {
			warning("No index written yet");
			return false;
		}

		_hasIndex = false;

		Common::String slotFile = _slotFile->build(slot);

		SaveWriter writer(2, slot, slotFile);
		SavePartInfo info(kSlotNameLength, (uint32) _vm->getGameType(), 0,
				_vm->getEndianness(), varSize);
		SavePartVars vars(_vm, varSize);

		// Write the description
		info.setDesc(_index + (slot * kSlotNameLength), kSlotNameLength);
		// Write all variables
		if (!vars.readFrom(0, 0, varSize))
			return false;

		if (!writer.writePart(0, &info))
			return false;
		if (!writer.writePart(1, &vars))
			return false;
	}

	return true;
}

void SaveLoad_Fascination::GameHandler::buildIndex(byte *buffer) const {
	uint32 varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return;

	SavePartInfo info(kSlotNameLength, (uint32) _vm->getGameType(),
			0, _vm->getEndianness(), varSize);

	_slotFile->buildIndex(buffer, info, 0);
}


SaveLoad_Fascination::SaveLoad_Fascination(GobEngine *vm, const char *targetName) :
		SaveLoad(vm) {

	_gameHandler = new GameHandler(vm, targetName);

	_saveFiles[0].handler  = _gameHandler;
	_saveFiles[1].handler  = _gameHandler;
	_saveFiles[2].handler  = _gameHandler;
	_saveFiles[3].handler  = _gameHandler;
	_saveFiles[4].handler  = _gameHandler;
	_saveFiles[5].handler  = _gameHandler;
	_saveFiles[6].handler  = _gameHandler;
	_saveFiles[7].handler  = _gameHandler;
	_saveFiles[8].handler  = _gameHandler;
	_saveFiles[9].handler  = _gameHandler;
	_saveFiles[10].handler = _gameHandler;
	_saveFiles[11].handler = _gameHandler;
	_saveFiles[12].handler = _gameHandler;
	_saveFiles[13].handler = _gameHandler;
	_saveFiles[14].handler = _gameHandler;
	_saveFiles[15].handler = _gameHandler;
}

SaveLoad_Fascination::~SaveLoad_Fascination() {
	delete _gameHandler;
}

const SaveLoad_Fascination::SaveFile *SaveLoad_Fascination::getSaveFile(const char *fileName) const {
	fileName = stripPath(fileName);

	for (int i = 0; i < ARRAYSIZE(_saveFiles); i++)
		if (!scumm_stricmp(fileName, _saveFiles[i].sourceName))
			return &_saveFiles[i];

	return 0;
}

SaveLoad_Fascination::SaveFile *SaveLoad_Fascination::getSaveFile(const char *fileName) {
	fileName = stripPath(fileName);

	for (int i = 0; i < ARRAYSIZE(_saveFiles); i++)
		if (!scumm_stricmp(fileName, _saveFiles[i].sourceName))
			return &_saveFiles[i];

	return 0;
}

SaveHandler *SaveLoad_Fascination::getHandler(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->handler;

	return 0;
}

const char *SaveLoad_Fascination::getDescription(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->description;

	return 0;
}

SaveLoad::SaveMode SaveLoad_Fascination::getSaveMode(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->mode;

	return kSaveModeNone;
}

} // End of namespace Gob
