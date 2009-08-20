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

SaveLoad_v6::SaveFile SaveLoad_v6::_saveFiles[] = {
	{  "cat.inf", kSaveModeSave,   0, "savegame"}, // Save file
	{  "mdo.def", kSaveModeExists, 0, 0},
	{"no_cd.txt", kSaveModeExists, 0, 0},
};


SaveLoad_v6::GameHandler::File::File(GobEngine *vm, const char *base) :
	SlotFileIndexed(vm, SaveLoad_v6::kSlotCount, base, "s") {
}

SaveLoad_v6::GameHandler::File::~File() {
}

int SaveLoad_v6::GameHandler::File::getSlot(int32 offset) const {
	uint32 varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return -1;

	return ((offset - (kPropsSize + kIndexSize)) / varSize);
}

int SaveLoad_v6::GameHandler::File::getSlotRemainder(int32 offset) const {
	uint32 varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return -1;

	return ((offset - (kPropsSize + kIndexSize)) % varSize);
}


SaveLoad_v6::GameHandler::GameHandler(GobEngine *vm, const char *target) : SaveHandler(vm) {
	memset(_props, 0, kPropsSize);
	memset(_index, 0, kIndexSize);

	_slotFile = new File(vm, target);
}

SaveLoad_v6::GameHandler::~GameHandler() {
	delete _slotFile;
}

int32 SaveLoad_v6::GameHandler::getSize() {
	uint32 varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return -1;

	return _slotFile->tallyUpFiles(varSize, kPropsSize + kIndexSize);
}

bool SaveLoad_v6::GameHandler::load(int16 dataVar, int32 size, int32 offset) {
	uint32 varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return false;

	if (size == 0) {
		// Indicator to load all variables
		dataVar = 0;
		size = varSize;
	}

	if (offset < kPropsSize) {
		// Properties

		refreshProps();

		if ((offset + size) > kPropsSize) {
			warning("Wrong index size (%d, %d)", size, offset);
			return false;
		}

		_vm->_inter->_variables->copyFrom(dataVar, _props + offset, size);

	} else if (offset < kPropsSize + kIndexSize) {
		// Save index

		if (size != kIndexSize) {
			warning("Wrong index size (%d, %d)", size, offset);
			return false;
		}

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
		SaveConverter_v6 converter(_vm, slotFile);

		if (converter.isOldSave()) {
			// Old save, plug the converter in
			if (!converter.load())
				return false;

			reader = new SaveReader(2, slot, converter);

		} else
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

bool SaveLoad_v6::GameHandler::save(int16 dataVar, int32 size, int32 offset) {
	uint32 varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return false;

	if (size == 0) {
		// Indicator to save all variables
		dataVar = 0;
		size = varSize;
	}

	if (offset < kPropsSize) {
		// Properties

		if ((offset + size) > kPropsSize) {
			warning("Wrong index size (%d, %d)", size, offset);
			return false;
		}

		_vm->_inter->_variables->copyTo(dataVar, _props + offset, size);

		refreshProps();

	}  else if (offset < kPropsSize + kIndexSize) {
		// Save index

		if (size != kIndexSize) {
			warning("Wrong index size (%d, %d)", size, offset);
			return false;
		}

		// Just copy the index into our buffer
		_vm->_inter->_variables->copyTo(dataVar, _index, kIndexSize);

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

void SaveLoad_v6::GameHandler::buildIndex(byte *buffer) const {
	uint32 varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return;

	SavePartInfo info(kSlotNameLength, (uint32) _vm->getGameType(),
			0, _vm->getEndianness(), varSize);

	SaveConverter_v6 converter(_vm);

	_slotFile->buildIndex(buffer, info, &converter);
}

void SaveLoad_v6::GameHandler::refreshProps() {
	uint32 maxSlot = _slotFile->getSlotMax();

	memset(_props + 40, 0xFF, 40);          // Joker
	_props[159] = 0x03;                     // # of joker unused
	WRITE_LE_UINT32(_props + 160, maxSlot); // # of saves
}


SaveLoad_v6::SaveLoad_v6(GobEngine *vm, const char *targetName) :
		SaveLoad(vm) {

	_gameHandler = new GameHandler(vm, targetName);

	_saveFiles[0].handler = _gameHandler;
}

SaveLoad_v6::~SaveLoad_v6() {
	delete _gameHandler;
}

const SaveLoad_v6::SaveFile *SaveLoad_v6::getSaveFile(const char *fileName) const {
	fileName = stripPath(fileName);

	for (int i = 0; i < ARRAYSIZE(_saveFiles); i++)
		if (!scumm_stricmp(fileName, _saveFiles[i].sourceName))
			return &_saveFiles[i];

	return 0;
}

SaveLoad_v6::SaveFile *SaveLoad_v6::getSaveFile(const char *fileName) {
	fileName = stripPath(fileName);

	for (int i = 0; i < ARRAYSIZE(_saveFiles); i++)
		if (!scumm_stricmp(fileName, _saveFiles[i].sourceName))
			return &_saveFiles[i];

	return 0;
}

SaveHandler *SaveLoad_v6::getHandler(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->handler;

	return 0;
}

const char *SaveLoad_v6::getDescription(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->description;

	return 0;
}

SaveLoad::SaveMode SaveLoad_v6::getSaveMode(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->mode;

	return kSaveModeNone;
}

} // End of namespace Gob
