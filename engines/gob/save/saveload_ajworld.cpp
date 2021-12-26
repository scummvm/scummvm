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

SaveLoad_AJWorld::SaveFile SaveLoad_AJWorld::_saveFiles[] = {
	{ "menu.inf", kSaveModeSave, nullptr, "temporary sprite"}
};


SaveLoad_AJWorld::SaveLoad_AJWorld(GobEngine *vm, const char *targetName) :
		SaveLoad(vm) {

	_tempSpriteHandler = new TempSpriteHandler(vm);

	_saveFiles[0].handler = _tempSpriteHandler;
}

SaveLoad_AJWorld::~SaveLoad_AJWorld() {
	delete _tempSpriteHandler;
}

const SaveLoad_AJWorld::SaveFile *SaveLoad_AJWorld::getSaveFile(const char *fileName) const {
	fileName = stripPath(fileName);

	for (int i = 0; i < ARRAYSIZE(_saveFiles); i++)
		if (!scumm_stricmp(fileName, _saveFiles[i].sourceName))
			return &_saveFiles[i];

	return nullptr;
}

SaveLoad_AJWorld::SaveFile *SaveLoad_AJWorld::getSaveFile(const char *fileName) {
	fileName = stripPath(fileName);

	for (int i = 0; i < ARRAYSIZE(_saveFiles); i++)
		if (!scumm_stricmp(fileName, _saveFiles[i].sourceName))
			return &_saveFiles[i];

	return nullptr;
}

SaveHandler *SaveLoad_AJWorld::getHandler(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->handler;

	return nullptr;
}

const char *SaveLoad_AJWorld::getDescription(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->description;

	return nullptr;
}

SaveLoad::SaveMode SaveLoad_AJWorld::getSaveMode(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->mode;

	return kSaveModeNone;
}

} // End of namespace Gob
