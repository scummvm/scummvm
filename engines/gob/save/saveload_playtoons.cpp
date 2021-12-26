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

#include "gob/save/saveload.h"
#include "gob/inter.h"
#include "gob/variables.h"

namespace Gob {

SaveLoad_Playtoons::SaveFile SaveLoad_Playtoons::_saveFiles[] = {
	{   "did.inf", kSaveModeSave,   nullptr, nullptr}, // Purpose ignored at the moment, intensively used to save things.
	{   "dan.itk", kSaveModeNone,   nullptr, nullptr}, // Playtoons CK detection file
	{   "cat.inf", kSaveModeNone,   nullptr, nullptr},
	{ "titre.009", kSaveModeIgnore, nullptr, nullptr}, // Playtoons theoritical title files that are checked for nothing
	{ "titre.010", kSaveModeIgnore, nullptr, nullptr},
	{ "titre.011", kSaveModeIgnore, nullptr, nullptr},
	{ "titre.012", kSaveModeIgnore, nullptr, nullptr},
	{ "titre.013", kSaveModeIgnore, nullptr, nullptr},
	{ "titre.014", kSaveModeIgnore, nullptr, nullptr},
	{ "titre.015", kSaveModeIgnore, nullptr, nullptr},
	{ "titre.016", kSaveModeIgnore, nullptr, nullptr},
	{ "titre.017", kSaveModeIgnore, nullptr, nullptr},
	{ "titre.018", kSaveModeIgnore, nullptr, nullptr},
	{ "titre.019", kSaveModeIgnore, nullptr, nullptr},
	{ "titre.020", kSaveModeIgnore, nullptr, nullptr},
	{ "titre.021", kSaveModeIgnore, nullptr, nullptr},
	{ "titre.022", kSaveModeIgnore, nullptr, nullptr},
	{ "titre.023", kSaveModeIgnore, nullptr, nullptr},
	{ "titre.024", kSaveModeIgnore, nullptr, nullptr},
	{ "titre.025", kSaveModeIgnore, nullptr, nullptr},
	{ "titre.026", kSaveModeIgnore, nullptr, nullptr},
	{ "titre.027", kSaveModeIgnore, nullptr, nullptr},
	{ "titre.028", kSaveModeIgnore, nullptr, nullptr},
	{ "titre.029", kSaveModeIgnore, nullptr, nullptr},
	{ "titre.030", kSaveModeIgnore, nullptr, nullptr},
	{ "titre.031", kSaveModeIgnore, nullptr, nullptr},
	{ "titre.032", kSaveModeIgnore, nullptr, nullptr},
	{ "titre.033", kSaveModeIgnore, nullptr, nullptr},
	{ "titre.034", kSaveModeIgnore, nullptr, nullptr},
	{ "titre.035", kSaveModeIgnore, nullptr, nullptr},
	{ "titre.036", kSaveModeIgnore, nullptr, nullptr},
	{ "titre.037", kSaveModeIgnore, nullptr, nullptr},
	{ "titre.038", kSaveModeIgnore, nullptr, nullptr},
	{ "titre.039", kSaveModeIgnore, nullptr, nullptr},
};

SaveLoad_Playtoons::GameHandler::File::File(GobEngine *vm, const char *base) :
	SlotFileIndexed(vm, SaveLoad_Playtoons::kSlotCount, base, "s") {
}

SaveLoad_Playtoons::GameHandler::File::~File() {
}

int SaveLoad_Playtoons::GameHandler::File::getSlot(int32 offset) const {
	uint32 varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return -1;

	return ((offset - (kPropsSize + kIndexSize)) / varSize);
}

int SaveLoad_Playtoons::GameHandler::File::getSlotRemainder(int32 offset) const {
	uint32 varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return -1;

	return ((offset - (kPropsSize + kIndexSize)) % varSize);
}


SaveLoad_Playtoons::GameHandler::GameHandler(GobEngine *vm, const char *target) : SaveHandler(vm) {
	memset(_props, 0, kPropsSize);
	memset(_index, 0, kIndexSize);

	_tempSpriteHandler = new TempSpriteHandler(vm);
	_slotFile = new File(vm, target);
}

SaveLoad_Playtoons::GameHandler::~GameHandler() {
	delete _slotFile;
	delete _tempSpriteHandler;
}

int32 SaveLoad_Playtoons::GameHandler::getSize() {
	uint32 varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return -1;

	return _slotFile->tallyUpFiles(varSize, kPropsSize + kIndexSize);
}

bool SaveLoad_Playtoons::GameHandler::load(int16 dataVar, int32 size, int32 offset) {
	uint32 varSize;

	if (size < 0) {
		// Load a temporary sprite
		debugC(2, kDebugSaveLoad, "Loading temporary sprite %d at pos %d", size, offset);
		_tempSpriteHandler->load(dataVar, size, offset);
		return true;
	}

	varSize = SaveHandler::getVarSize(_vm);
	if (varSize == 0)
		return false;

	if (size == 0) {
		// Indicator to load all variables
		dataVar = 0;
		size = varSize;
	}

	if (((uint32) offset) < kPropsSize) {
		// Properties

		if (((uint32) (offset + size)) > kPropsSize) {
			warning("Wrong index size (%d, %d)", size, offset);
			return false;
		}

		_vm->_inter->_variables->copyFrom(dataVar, _props + offset, size);

	} else if (((uint32) offset) < kPropsSize + kIndexSize) {
		// Save index

		if (((uint32) size) != kIndexSize) {
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

		SaveReader *reader = nullptr;

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

bool SaveLoad_Playtoons::GameHandler::save(int16 dataVar, int32 size, int32 offset) {
	uint32 varSize;

	if (size < 0) {
		// Save a temporary sprite
		debugC(2, kDebugSaveLoad, "Saving temporary sprite %d at pos %d", size, offset);
		_tempSpriteHandler->save(dataVar, size, offset);
		return true;
	}

	varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return false;

	if (size == 0) {
		// Indicator to save all variables
		dataVar = 0;
		size = varSize;
	}

	if (((uint32) offset) < kPropsSize) {
		// Properties

		if (((uint32) (offset + size)) > kPropsSize) {
			warning("Wrong index size (%d, %d)", size, offset);
			return false;
		}

		_vm->_inter->_variables->copyTo(dataVar, _props + offset, size);

	}  else if (((uint32) offset) < kPropsSize + kIndexSize) {
		// Save index

		if (((uint32) size) != kIndexSize) {
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

void SaveLoad_Playtoons::GameHandler::buildIndex(byte *buffer) const {
	uint32 varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return;

	SavePartInfo info(kSlotNameLength, (uint32) _vm->getGameType(),
			0, _vm->getEndianness(), varSize);

	_slotFile->buildIndex(buffer, info, nullptr);
}


SaveLoad_Playtoons::SaveLoad_Playtoons(GobEngine *vm, const char *targetName) :
		SaveLoad(vm) {

	_gameHandler = new GameHandler(vm, targetName);

	_saveFiles[0].handler = _gameHandler;
}

SaveLoad_Playtoons::~SaveLoad_Playtoons() {
	delete _gameHandler;
}

const SaveLoad_Playtoons::SaveFile *SaveLoad_Playtoons::getSaveFile(const char *fileName) const {
	fileName = stripPath(fileName);

	for (int i = 0; i < ARRAYSIZE(_saveFiles); i++)
		if (!scumm_stricmp(fileName, _saveFiles[i].sourceName))
			return &_saveFiles[i];

	return nullptr;
}

SaveLoad_Playtoons::SaveFile *SaveLoad_Playtoons::getSaveFile(const char *fileName) {
	fileName = stripPath(fileName);

	for (int i = 0; i < ARRAYSIZE(_saveFiles); i++)
		if (!scumm_stricmp(fileName, _saveFiles[i].sourceName))
			return &_saveFiles[i];

	return nullptr;
}

SaveHandler *SaveLoad_Playtoons::getHandler(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->handler;

	return nullptr;
}

const char *SaveLoad_Playtoons::getDescription(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->description;

	return nullptr;
}

SaveLoad::SaveMode SaveLoad_Playtoons::getSaveMode(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->mode;

	return kSaveModeNone;
}

} // End of namespace Gob
