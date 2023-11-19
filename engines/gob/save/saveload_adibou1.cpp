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
#include "gob/save/saveconverter.h"
#include "gob/inter.h"
#include "gob/variables.h"

namespace Gob {

SaveLoad_Adibou1::SaveFile SaveLoad_Adibou1::_saveFiles[] = {
	{ "bou.inf", kSaveModeSave, nullptr, "adibou1"},
	{ "dessin.inf", kSaveModeSave, nullptr, "paint game drawing"},
	{ "const.inf", kSaveModeSave, nullptr, "construction game"},
	{ "menu.inf", kSaveModeSave, nullptr, "temporary sprite"},
	{ "dessin1.inf", kSaveModeSave, nullptr, "paint game drawing"},
	{ "dessin2.inf", kSaveModeSave, nullptr, "paint game drawing"},
	{ "dessin3.inf", kSaveModeSave, nullptr, "paint game drawing"},
	{ "dessin4.inf", kSaveModeSave, nullptr, "paint game drawing"},
	{ "dessin5.inf", kSaveModeSave, nullptr, "paint game drawing"},
	{ "dessin6.inf", kSaveModeSave, nullptr, "paint game drawing"},
	{ "dessin7.inf", kSaveModeSave, nullptr, "paint game drawing"},
	{ "dessin8.inf", kSaveModeSave, nullptr, "paint game drawing"},
	{ "a:\\bou.inf", kSaveModeIgnore, nullptr, "copy of the savegame to be sent "
											   "to Coktel Vision on a floppy"
											   "when completing the game"},
};

SaveLoad_Adibou1::SaveLoad_Adibou1(GobEngine *vm, const char *targetName) :
	SaveLoad(vm) {

	uint32 index = 0;
	_saveFiles[index++].handler = _bouHandler = new GameFileHandler(vm, targetName, "bouinf");
	_saveFiles[index++].handler = _drawingHandler = new SpriteHandler(vm, targetName, "drawing");
	_saveFiles[index++].handler = _constructionHandler = new GameFileHandler(vm, targetName, "construction");
	_saveFiles[index++].handler = _menuHandler = new TempSpriteHandler(vm);;
	for (int i = 0; i < kAdibou1NbrOfDrawings; i++) {
		_saveFiles[index++].handler = _drawingWithThumbnailHandler[i] = new DrawingWithThumbnailHandler(vm,
																									   targetName,
																									   Common::String::format("drawing%02d", i + 1));
	}

}

SaveLoad_Adibou1::~SaveLoad_Adibou1() {
	delete _bouHandler;
	delete _drawingHandler;
	delete _constructionHandler;
	delete _menuHandler;
	for (int i = 0; i < kAdibou1NbrOfDrawings; i++)
		delete _drawingWithThumbnailHandler[i];
}

SaveLoad_Adibou1::SpriteHandler::File::File(GobEngine *vm, const Common::String &base, const Common::String &ext) :
	SlotFileStatic(vm, base, ext) {
}

SaveLoad_Adibou1::SpriteHandler::File::~File() {
}

SaveLoad_Adibou1::SpriteHandler::SpriteHandler(GobEngine *vm, const Common::String &target, const Common::String &ext)
	: TempSpriteHandler(vm), _file(vm, target, ext) {
}

SaveLoad_Adibou1::SpriteHandler::~SpriteHandler() {
}

int32 SaveLoad_Adibou1::SpriteHandler::getSize() {
	Common::String fileName = _file.build();

	if (fileName.empty())
		return -1;;

	SaveReader reader(1, 0, fileName);
	SaveHeader header;

	if (!reader.load())
		return -1;

	if (!reader.readPartHeader(0, &header))
		return -1;

	// Return the part's size
	return header.getSize();
}

bool SaveLoad_Adibou1::SpriteHandler::load(int16 dataVar, int32 size, int32 offset) {
	if (!TempSpriteHandler::createFromSprite(dataVar, size, offset))
		return false;

	Common::String fileName = _file.build();
	if (fileName.empty())
		return false;

	SaveReader reader(1, 0, fileName);
	if (!reader.load())
		return false;

	if (!reader.readPart(0, _sprite))
		return false;

	return TempSpriteHandler::load(dataVar, size, offset);
}

bool SaveLoad_Adibou1::SpriteHandler::save(int16 dataVar, int32 size, int32 offset) {
	if (!TempSpriteHandler::save(dataVar, size, offset))
		return false;

	Common::String fileName = _file.build();
	if (fileName.empty())
		return false;

	SaveWriter writer(1, 0, fileName);
	return writer.writePart(0, _sprite);
}

SaveLoad_Adibou1::DrawingWithThumbnailHandler::File::File(GobEngine *vm, const Common::String &base, const Common::String &ext) :
	SlotFileStatic(vm, base, ext) {
}

SaveLoad_Adibou1::DrawingWithThumbnailHandler::File::~File() {
}

SaveLoad_Adibou1::DrawingWithThumbnailHandler::DrawingWithThumbnailHandler(GobEngine *vm,
																		 const Common::String &target,
																		 const Common::String &ext)
	: TempSpriteHandler(vm), _file(vm, target, ext) {

	Common::String fileName = _file.build();
	_reader = new SaveReader(2, 0, fileName); // Two parts: the thumbnail + the actual drawing
	_writer = new SaveWriter(2, 0, fileName);
}

SaveLoad_Adibou1::DrawingWithThumbnailHandler::~DrawingWithThumbnailHandler() {
	delete _reader;
	delete _writer;
}

int32 SaveLoad_Adibou1::DrawingWithThumbnailHandler::getSize() {
	if (!_reader)
		return -1;;

	if (!_reader->load())
		return -1;

	SaveHeader header1, header2;
	if (!_reader->readPartHeader(0, &header1))
		return -1;

	if (!_reader->readPartHeader(1, &header2))
		return -1;

	return header1.getSize() + header2.getSize();
}

bool SaveLoad_Adibou1::DrawingWithThumbnailHandler::load(int16 dataVar, int32 size, int32 offset) {
	if (!TempSpriteHandler::createFromSprite(dataVar, size, offset))
		return false;

	Common::String fileName = _file.build();
	if (fileName.empty())
		return false;

	if (!_reader->load())
		return false;

	uint32 part = (offset == 0) ? 0 : 1;
	if (!_reader->readPart(part, _sprite))
		return false;

	return TempSpriteHandler::load(dataVar, size, offset);
}

bool SaveLoad_Adibou1::DrawingWithThumbnailHandler::save(int16 dataVar, int32 size, int32 offset) {
	if (!TempSpriteHandler::save(dataVar, size, offset))
		return false;

	Common::String fileName = _file.build();
	if (fileName.empty())
		return false;

	uint32 part = (offset == 0) ? 0 : 1;
	return _writer->writePart(part, _sprite);
}


SaveLoad_Adibou1::GameFileHandler::File::File(GobEngine *vm, const Common::String &base, const Common::String &ext) :
	SlotFileStatic(vm, base, ext) {
}

SaveLoad_Adibou1::GameFileHandler::File::~File() {
}


SaveLoad_Adibou1::GameFileHandler::GameFileHandler(GobEngine *vm, const Common::String &target, const Common::String &ext) :
	SaveHandler(vm), _file(vm, target, ext) {
}

SaveLoad_Adibou1::GameFileHandler::~GameFileHandler() {
}

int32 SaveLoad_Adibou1::GameFileHandler::getSize() {
	Common::String fileName = _file.build();
	if (fileName.empty())
		return -1;

	SaveReader reader(1, 0, fileName);
	SaveHeader header;

	if (!reader.load())
		return -1;

	if (!reader.readPartHeader(0, &header))
		return -1;

	// Return the part's size
	return header.getSize();
}

bool SaveLoad_Adibou1::GameFileHandler::load(int16 dataVar, int32 size, int32 offset) {
	Common::String fileName = _file.build();
	if (fileName.empty())
		return false;

	if (size < 0) {
		// Hack in original game, using a bitmap memory as a temporary buffer to make
		// a copy the savegame. We do not need this copy, so we can just ignore it.
		debugC(1, kDebugSaveLoad, "Ignoring bou.inf save with negative size");
		return true;
	}

	if (size == 0) {
		uint32 varSize = SaveHandler::getVarSize(_vm);
		// Indicator to load all variables
		dataVar = 0;
		size = (int32) varSize;
	}

	int32 fileSize = getSize();
	if (fileSize < 0)
		return false;

	SaveReader reader(1, 0, fileName);
	SavePartVars vars(_vm, fileSize);

	if (!reader.load()) {
		return false;
	}

	if (!reader.readPart(0, &vars)) {
		return false;
	}

	if (!vars.writeInto((uint16) dataVar, offset, size)) {
		return false;
	}

	return true;
}

bool SaveLoad_Adibou1::GameFileHandler::save(const byte *ptrRaw, int16 dataVar, int32 size, int32 offset) {
	Common::String fileName = _file.build();
	if (fileName.empty())
		return false;

	if (size == 0) {
		// Indicator to save all variables
		dataVar = 0;
		uint32 varSize = SaveHandler::getVarSize(_vm);
		size = (int32) varSize;
	}

	int32 fileSize = getSize();
	int32 newFileSize = size;
	if (fileSize > 0) {
		newFileSize = MAX<int32>(fileSize, size + offset);
	}

	SavePartVars vars(_vm, newFileSize);
	if (fileSize > 0
		&&
		(offset > 0 || size < fileSize)) {
		// Load data from file, as some of it will not be overwritten
		SaveReader reader(1, 0, fileName);
		if (!reader.load()) {
			return false;
		}

		if (fileSize == newFileSize) {
			// We can use the same SavePartVars object
			if (!reader.readPart(0, &vars)) {
				return false;
			}
		} else {
			// We need to use a temporary SavePartVars object to load data
			SavePartVars vars_from_file(_vm, fileSize);
			if (!reader.readPart(0, &vars_from_file)) {;
				return false;
			}

			// Copy data from temporary SavePartVars object to the real one
			vars.readFromRaw(vars_from_file.data(), 0, fileSize);
		}
	}

	SaveWriter writer(1, 0, fileName);
	if (ptrRaw) {
		// Write data from raw pointer
		vars.readFromRaw(ptrRaw, offset, size);
	} else {
		// Write data from variables
		if (!vars.readFrom((uint16) dataVar, offset, size))
			return false;
	}

	return writer.writePart(0, &vars);
}

bool SaveLoad_Adibou1::GameFileHandler::save(int16 dataVar, int32 size, int32 offset) {
	return save(nullptr, dataVar, size, offset);
}

bool SaveLoad_Adibou1::GameFileHandler::deleteFile() {
	Common::String fileName = _file.build();
	if (fileName.empty())
		return false;

	SaveWriter writer(1, 0, fileName);
	return writer.deleteFile();
}

const SaveLoad_Adibou1::SaveFile *SaveLoad_Adibou1::getSaveFile(const char *fileName) const {
	for (int i = 0; i < ARRAYSIZE(_saveFiles); i++)
		if (!scumm_stricmp(fileName, _saveFiles[i].sourceName))
			return &_saveFiles[i];

	return 0;
}

SaveLoad_Adibou1::SaveFile *SaveLoad_Adibou1::getSaveFile(const char *fileName) {
	fileName = stripPath(fileName);

	for (int i = 0; i < ARRAYSIZE(_saveFiles); i++)
		if (!scumm_stricmp(fileName, _saveFiles[i].sourceName))
			return &_saveFiles[i];

	return 0;
}

SaveHandler *SaveLoad_Adibou1::getHandler(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->handler;

	return 0;
}

const char *SaveLoad_Adibou1::getDescription(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->description;

	return 0;
}

SaveLoad::SaveMode SaveLoad_Adibou1::getSaveMode(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->mode;

	return kSaveModeNone;
}

} // End of namespace Gob
