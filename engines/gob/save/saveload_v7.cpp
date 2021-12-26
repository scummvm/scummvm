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

SaveLoad_v7::SaveFile SaveLoad_v7::_saveFiles[] = {
	// Addy Junior Base
	{"visage01.inf", kSaveModeSave, nullptr, "face"         }, // Child 01
	{"visage02.inf", kSaveModeSave, nullptr, "face"         }, // Child 02
	{"visage03.inf", kSaveModeSave, nullptr, "face"         }, // Child 03
	{"visage04.inf", kSaveModeSave, nullptr, "face"         }, // Child 04
	{"visage05.inf", kSaveModeSave, nullptr, "face"         }, // Child 05
	{"visage06.inf", kSaveModeSave, nullptr, "face"         }, // Child 06
	{"visage07.inf", kSaveModeSave, nullptr, "face"         }, // Child 07
	{"visage08.inf", kSaveModeSave, nullptr, "face"         }, // Child 08
	{"visage09.inf", kSaveModeSave, nullptr, "face"         }, // Child 09
	{"visage10.inf", kSaveModeSave, nullptr, "face"         }, // Child 10
	{"visage11.inf", kSaveModeSave, nullptr, "face"         }, // Child 11
	{"visage12.inf", kSaveModeSave, nullptr, "face"         }, // Child 12
	{"visage13.inf", kSaveModeSave, nullptr, "face"         }, // Child 13
	{"visage14.inf", kSaveModeSave, nullptr, "face"         }, // Child 14
	{"visage15.inf", kSaveModeSave, nullptr, "face"         }, // Child 15
	{"visage16.inf", kSaveModeSave, nullptr, "face"         }, // Child 16
	{  "enfant.inf", kSaveModeSave, nullptr, "children"     },
	{   "debil.tmp", kSaveModeSave, nullptr, nullptr              },
	{  "config.inf", kSaveModeSave, nullptr, "configuration"},
// Addy 4 Base
	{"config00.inf", kSaveModeSave, nullptr, nullptr              },
	{"statev00.inf", kSaveModeSave, nullptr, nullptr              },
	// Addy 4 Grundschule
	{ "premier.dep", kSaveModeSave, nullptr, nullptr              },
	{ "quitter.dep", kSaveModeSave, nullptr, nullptr              },
	{   "appel.dep", kSaveModeSave, nullptr, nullptr              },
	{  "parole.dep", kSaveModeSave, nullptr, nullptr              },
	{    "ado4.inf", kSaveModeSave, nullptr, nullptr              },
	{"mcurrent.inf", kSaveModeSave, nullptr, nullptr              },
	{   "perso.dep", kSaveModeSave, nullptr, nullptr              },
	{ "nouveau.dep", kSaveModeSave, nullptr, nullptr              },
	{     "adi.tmp", kSaveModeSave, nullptr, nullptr              },
	{     "adi.inf", kSaveModeSave, nullptr, nullptr              },
	{    "adi4.tmp", kSaveModeSave, nullptr, nullptr              }
};


SaveLoad_v7::SaveLoad_v7(GobEngine *vm, const char *targetName) :
		SaveLoad(vm) {

	for (uint32 i = 0; i < kChildrenCount; i++)
		_saveFiles[i].handler = _faceHandler[i] = new TempSpriteHandler(_vm);

	_saveFiles[16].handler = _childrenHandler = new FakeFileHandler(_vm);
	_saveFiles[17].handler = _debilHandler    = new FakeFileHandler(_vm);
	_saveFiles[18].handler = _configHandler   = new FakeFileHandler(_vm);

	for (int i = 0; i < 2; i++)
		_saveFiles[19 + i].handler = _addy4BaseHandler[i] = new FakeFileHandler(_vm);

	for (int i = 0; i < 11; i++)
		_saveFiles[21 + i].handler = _addy4GrundschuleHandler[i] = new FakeFileHandler(_vm);
}

SaveLoad_v7::~SaveLoad_v7() {
	for (int i = 0; i < 11; i++)
		delete _addy4GrundschuleHandler[i];

	for (int i = 0; i < 2; i++)
		delete _addy4BaseHandler[i];

	delete _configHandler;
	delete _debilHandler;
	delete _childrenHandler;

	for (uint32 i = 0; i < kChildrenCount; i++)
		delete _faceHandler[i];
}

const SaveLoad_v7::SaveFile *SaveLoad_v7::getSaveFile(const char *fileName) const {
	fileName = stripPath(fileName);

	for (int i = 0; i < ARRAYSIZE(_saveFiles); i++)
		if (!scumm_stricmp(fileName, _saveFiles[i].sourceName))
			return &_saveFiles[i];

	return nullptr;
}

SaveLoad_v7::SaveFile *SaveLoad_v7::getSaveFile(const char *fileName) {
	fileName = stripPath(fileName);

	for (int i = 0; i < ARRAYSIZE(_saveFiles); i++)
		if (!scumm_stricmp(fileName, _saveFiles[i].sourceName))
			return &_saveFiles[i];

	return nullptr;
}

SaveHandler *SaveLoad_v7::getHandler(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->handler;

	return nullptr;
}

const char *SaveLoad_v7::getDescription(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->description;

	return nullptr;
}

SaveLoad::SaveMode SaveLoad_v7::getSaveMode(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->mode;

	return kSaveModeNone;
}

} // End of namespace Gob
