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
	{   "debil.tmp", kSaveModeSave, nullptr, nullptr        },
	{  "config.inf", kSaveModeSave, nullptr, "configuration"},
	// Adibou environment (flowers, vegetables...)
	{"enviro01.inf", kSaveModeSave, nullptr, "environment" }, // Child 01
	{"enviro02.inf", kSaveModeSave, nullptr, "environment" }, // Child 02
	{"enviro03.inf", kSaveModeSave, nullptr, "environment" }, // Child 03
	{"enviro04.inf", kSaveModeSave, nullptr, "environment" }, // Child 04
	{"enviro05.inf", kSaveModeSave, nullptr, "environment" }, // Child 05
	{"enviro06.inf", kSaveModeSave, nullptr, "environment" }, // Child 06
	{"enviro07.inf", kSaveModeSave, nullptr, "environment" }, // Child 07
	{"enviro08.inf", kSaveModeSave, nullptr, "environment" }, // Child 08
    {"enviro09.inf", kSaveModeSave, nullptr, "environment" }, // Child 09
	{"enviro10.inf", kSaveModeSave, nullptr, "environment" }, // Child 10
	{"enviro11.inf", kSaveModeSave, nullptr, "environment" }, // Child 11
	{"enviro12.inf", kSaveModeSave, nullptr, "environment" }, // Child 12
    {"enviro13.inf", kSaveModeSave, nullptr, "environment" }, // Child 13
	{"enviro14.inf", kSaveModeSave, nullptr, "environment" }, // Child 14
	{"enviro15.inf", kSaveModeSave, nullptr, "environment" }, // Child 15
	{"enviro16.inf", kSaveModeSave, nullptr, "environment" }, // Child 16
	// Adibou Applications 1-5
    {"Gsa01_01.inf", kSaveModeSave, 0, "app progress" }, // Child 01
    {"Gsa02_01.inf", kSaveModeSave, 0, "app progress" },
    {"Gsa03_01.inf", kSaveModeSave, 0, "app progress" },
    {"Gsa04_01.inf", kSaveModeSave, 0, "app progress" },
    {"Gsa05_01.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa01_02.inf", kSaveModeSave, 0, "app progress" }, // Child 02
	{"Gsa02_02.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa03_02.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa04_02.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa05_02.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa01_03.inf", kSaveModeSave, 0, "app progress" }, // Child 03
	{"Gsa02_03.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa03_03.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa04_03.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa05_03.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa01_04.inf", kSaveModeSave, 0, "app progress" }, // Child 04
	{"Gsa02_04.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa03_04.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa04_04.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa05_04.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa01_05.inf", kSaveModeSave, 0, "app progress" }, // Child 05
	{"Gsa02_05.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa03_05.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa04_05.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa05_05.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa01_06.inf", kSaveModeSave, 0, "app progress" }, // Child 06
	{"Gsa02_06.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa03_06.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa04_06.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa05_06.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa01_07.inf", kSaveModeSave, 0, "app progress" }, // Child 07
	{"Gsa02_07.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa03_07.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa04_07.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa05_07.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa01_08.inf", kSaveModeSave, 0, "app progress" }, // Child 08
	{"Gsa02_08.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa03_08.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa04_08.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa05_08.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa01_09.inf", kSaveModeSave, 0, "app progress" }, // Child 09
	{"Gsa02_09.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa03_09.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa04_09.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa05_09.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa01_10.inf", kSaveModeSave, 0, "app progress" }, // Child 10
	{"Gsa02_10.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa03_10.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa04_10.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa05_10.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa01_11.inf", kSaveModeSave, 0, "app progress" }, // Child 11
	{"Gsa02_11.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa03_11.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa04_11.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa05_11.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa01_12.inf", kSaveModeSave, 0, "app progress" }, // Child 12
	{"Gsa02_12.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa03_12.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa04_12.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa05_12.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa01_13.inf", kSaveModeSave, 0, "app progress" }, // Child 13
	{"Gsa02_13.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa03_13.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa04_13.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa05_13.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa01_14.inf", kSaveModeSave, 0, "app progress" }, // Child 14
	{"Gsa02_14.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa03_14.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa04_14.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa05_14.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa01_15.inf", kSaveModeSave, 0, "app progress" }, // Child 15
	{"Gsa02_15.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa03_15.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa04_15.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa05_15.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa01_16.inf", kSaveModeSave, 0, "app progress" }, // Child 16
	{"Gsa02_16.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa03_16.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa04_16.inf", kSaveModeSave, 0, "app progress" },
	{"Gsa05_16.inf", kSaveModeSave, 0, "app progress" },
	{"memo_01.inf", kSaveModeSave, 0, "memo" }, // Child 01
	{"memo_02.inf", kSaveModeSave, 0, "memo" }, // Child 02
	{"memo_03.inf", kSaveModeSave, 0, "memo" }, // Child 03
	{"memo_04.inf", kSaveModeSave, 0, "memo" }, // Child 04
	{"memo_05.inf", kSaveModeSave, 0, "memo" }, // Child 05
	{"memo_06.inf", kSaveModeSave, 0, "memo" }, // Child 06
	{"memo_07.inf", kSaveModeSave, 0, "memo" }, // Child 07
	{"memo_08.inf", kSaveModeSave, 0, "memo" }, // Child 08
	{"memo_09.inf", kSaveModeSave, 0, "memo" }, // Child 09
	{"memo_10.inf", kSaveModeSave, 0, "memo" }, // Child 10
	{"memo_11.inf", kSaveModeSave, 0, "memo" }, // Child 11
	{"memo_12.inf", kSaveModeSave, 0, "memo" }, // Child 12
	{"memo_13.inf", kSaveModeSave, 0, "memo" }, // Child 13
	{"memo_14.inf", kSaveModeSave, 0, "memo" }, // Child 14
	{"memo_15.inf", kSaveModeSave, 0, "memo" }, // Child 15
	{"memo_16.inf", kSaveModeSave, 0, "memo" }, // Child 16
	{"diplo_01.inf", kSaveModeSave, 0, "diplo" }, // Child 01
	{"diplo_02.inf", kSaveModeSave, 0, "diplo" }, // Child 02
	{"diplo_03.inf", kSaveModeSave, 0, "diplo" }, // Child 03
	{"diplo_04.inf", kSaveModeSave, 0, "diplo" }, // Child 04
	{"diplo_05.inf", kSaveModeSave, 0, "diplo" }, // Child 05
	{"diplo_06.inf", kSaveModeSave, 0, "diplo" }, // Child 06
	{"diplo_07.inf", kSaveModeSave, 0, "diplo" }, // Child 07
	{"diplo_08.inf", kSaveModeSave, 0, "diplo" }, // Child 08
	{"diplo_09.inf", kSaveModeSave, 0, "diplo" }, // Child 09
	{"diplo_10.inf", kSaveModeSave, 0, "diplo" }, // Child 10
	{"diplo_11.inf", kSaveModeSave, 0, "diplo" }, // Child 11
	{"diplo_12.inf", kSaveModeSave, 0, "diplo" }, // Child 12
	{"diplo_13.inf", kSaveModeSave, 0, "diplo" }, // Child 13
	{"diplo_14.inf", kSaveModeSave, 0, "diplo" }, // Child 14
	{"diplo_15.inf", kSaveModeSave, 0, "diplo" }, // Child 15
	{"diplo_16.inf", kSaveModeSave, 0, "diplo" }, // Child 16
	{"appli_01.inf", kSaveModeSave, 0, "app info" },
	{"appli_02.inf", kSaveModeSave, 0, "app info" },
	{"appli_03.inf", kSaveModeSave, 0, "app info" },
	{"appli_04.inf", kSaveModeSave, 0, "app info" },
	{"appli_05.inf", kSaveModeSave, 0, "app info" },
	{"crite_01.inf", kSaveModeSave, 0, "app info" },
	{"crite_02.inf", kSaveModeSave, 0, "app info" },
	{"crite_03.inf", kSaveModeSave, 0, "app info" },
	{"crite_04.inf", kSaveModeSave, 0, "app info" },
	{"crite_05.inf", kSaveModeSave, 0, "app info" },
	{"exo_01.inf", kSaveModeSave, 0, "app info" },
	{"exo_02.inf", kSaveModeSave, 0, "app info" },
	{"exo_03.inf", kSaveModeSave, 0, "app info" },
	{"exo_04.inf", kSaveModeSave, 0, "app info" },
	{"exo_05.inf", kSaveModeSave, 0, "app info" },
	{"ico_01.inf", kSaveModeSave, 0, "app info" },
	{"ico_02.inf", kSaveModeSave, 0, "app info" },
	{"ico_03.inf", kSaveModeSave, 0, "app info" },
	{"ico_04.inf", kSaveModeSave, 0, "app info" },
	{"ico_05.inf", kSaveModeSave, 0, "app info" },
	{"applis.inf", kSaveModeSave, 0, "app info" },
	{"lance.inf", kSaveModeSave, 0, "app info" },
	{"retour.inf", kSaveModeSave, 0, "app info" },

    // Addy 4 Base
	{"config00.inf", kSaveModeSave, nullptr, nullptr        },
	{"statev00.inf", kSaveModeSave, nullptr, nullptr        },
	// Addy 4 Grundschule
	{ "premier.dep", kSaveModeSave, nullptr, nullptr        },
	{ "quitter.dep", kSaveModeSave, nullptr, nullptr        },
	{   "appel.dep", kSaveModeSave, nullptr, nullptr        },
	{  "parole.dep", kSaveModeSave, nullptr, nullptr        },
	{    "ado4.inf", kSaveModeSave, nullptr, nullptr        },
	{"mcurrent.inf", kSaveModeSave, nullptr, nullptr        },
	{   "perso.dep", kSaveModeSave, nullptr, nullptr        },
	{ "nouveau.dep", kSaveModeSave, nullptr, nullptr        },
	{     "adi.tmp", kSaveModeSave, nullptr, nullptr        },
	{     "adi.inf", kSaveModeSave, nullptr, nullptr        },
	{    "adi4.tmp", kSaveModeSave, nullptr, nullptr        }
};

SaveLoad_v7::SpriteHandler::File::File(GobEngine *vm, const Common::String &base, const Common::String &ext) :
SlotFileStatic(vm, base, ext) {
}

SaveLoad_v7::SpriteHandler::File::~File() {
}


SaveLoad_v7::SpriteHandler::SpriteHandler(GobEngine *vm, const Common::String &target, const Common::String &ext)
	: TempSpriteHandler(vm), _file(vm, target, ext) {
}

SaveLoad_v7::SpriteHandler::~SpriteHandler() {
}

int32 SaveLoad_v7::SpriteHandler::getSize() {
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

bool SaveLoad_v7::SpriteHandler::load(int16 dataVar, int32 size, int32 offset)
{
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

bool SaveLoad_v7::SpriteHandler::save(int16 dataVar, int32 size, int32 offset)
{
	if (!TempSpriteHandler::save(dataVar, size, offset))
		return false;

	Common::String fileName = _file.build();
	if (fileName.empty())
		return false;

	SaveWriter writer(1, 0, fileName);
	return writer.writePart(0, _sprite);
}

SaveLoad_v7::GameFileHandler::File::File(GobEngine *vm, const Common::String &base, const Common::String &ext) :
SlotFileStatic(vm, base, ext) {
}

SaveLoad_v7::GameFileHandler::File::~File() {
}


SaveLoad_v7::GameFileHandler::GameFileHandler(GobEngine *vm, const Common::String &target, const Common::String &ext) :
SaveHandler(vm), _file(vm, target, ext) {
}

SaveLoad_v7::GameFileHandler::~GameFileHandler() {
}

int32 SaveLoad_v7::GameFileHandler::getSize() {
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

bool SaveLoad_v7::GameFileHandler::load(int16 dataVar, int32 size, int32 offset) {
	Common::String fileName = _file.build();
	if (fileName.empty())
		return false;

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

bool SaveLoad_v7::GameFileHandler::save(int16 dataVar, int32 size, int32 offset) {
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
			vars.partialReadFromRaw(vars_from_file.data(), fileSize);
		}
	}

	SaveWriter writer(1, 0, fileName);
	if (!vars.readFrom((uint16) dataVar, offset, size))
		return false;

	return writer.writePart(0, &vars);
}

SaveLoad_v7::SaveLoad_v7(GobEngine *vm, const char *targetName) :
		SaveLoad(vm) {

	uint32 index = 0;
	for (uint32 i = 0; i < kChildrenCount; i++) {
		_saveFiles[index++].handler = _faceHandler[i] = new SpriteHandler(_vm,
																		targetName,
																		Common::String::format("vsg%02d", i + 1));
	}

	_saveFiles[index++].handler = _childrenHandler = new FakeFileHandler(_vm);
	_saveFiles[index++].handler = _debilHandler    = new FakeFileHandler(_vm);
	_saveFiles[index++].handler = _configHandler   = new GameFileHandler(_vm, targetName, "cfg");

	//const Common::Array<int> environmentOffsets = {0, 1764, 2564, 3284, 6684, 7192};
	for (uint32 i = 0; i < kChildrenCount; i++)
		_saveFiles[index++].handler = _adibou2EnvHandler[i] = new GameFileHandler(_vm,
																				  targetName,
																				  Common::String::format("env%02d", i + 1));

	for (uint32 i = 0; i < kChildrenCount; i++)
	{
		for (uint32 j = 0; j < kAdibou2NbrOfApplications; j++)
		{
			Common::String ext = Common::String::format("gsa%02d_%02d", j + 1, i + 1);
			_saveFiles[index++].handler = _adibou2AppProgressHandler[i][j]  = new GameFileHandler(_vm,
																								 targetName,
																								 ext);
		}
	}

	for (uint32 i = 0; i < kChildrenCount; i++)
	{
		_saveFiles[index++].handler = _adibou2MemoHandler[i] = new GameFileHandler(_vm, targetName, Common::String::format("memo%02d", i + 1));
		_saveFiles[index++].handler = _adibou2DiploHandler[i] = new GameFileHandler(_vm, targetName, Common::String::format("diplo%02d", i + 1));
	}


	for (uint32 i = 0; i < kAdibou2NbrOfApplications; i++) {
		_saveFiles[index++].handler = _adibou2AppliHandler[i] = new GameFileHandler(_vm, targetName, Common::String::format("appli%02d", i + 1));
		_saveFiles[index++].handler = _adibou2CriteHandler[i] = new GameFileHandler(_vm, targetName, Common::String::format("crite%02d", i + 1));
		_saveFiles[index++].handler = _adibou2ExoHandler[i] = new GameFileHandler(_vm, targetName, Common::String::format("exo%02d", i + 1));
	}

	for (uint32 i = 0; i < kAdibou2NbrOfApplications; i++) {
		_saveFiles[index++].handler = _adibou2AppIcoHandler[i] = new SpriteHandler(_vm, targetName, Common::String::format("app_ico%02d", i + 1));
	}

	const Common::Array<int> applisOffsets = {0, 4, 8, 12, 16, 20};
	_saveFiles[index++].handler = _adibou2ApplicationsInfoHandler = new GameFileHandler(_vm, targetName, "applis");
	_saveFiles[index++].handler = _adibou2RetourHandler = new FakeFileHandler(_vm);
	_saveFiles[index++].handler = _adibou2LanceHandler = new FakeFileHandler(_vm);

	for (int i = 0; i < 2; i++)
		_saveFiles[index++].handler = _addy4BaseHandler[i] = new FakeFileHandler(_vm);

	for (int i = 0; i < 11; i++)
		_saveFiles[index++].handler = _addy4GrundschuleHandler[i] = new FakeFileHandler(_vm);
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
	{
		delete _adibou2EnvHandler[i];

		for (uint32 j = 0; j < kAdibou2NbrOfApplications; j++)
			delete _adibou2AppProgressHandler[i][j];
	}

	for (uint32 i = 0; i < kChildrenCount; i++)
	{
		delete _faceHandler[i];
		delete _adibou2MemoHandler[i];
		delete _adibou2DiploHandler[i];
	}

	for (uint32 i = 0; i < kAdibou2NbrOfApplications; i++) {
		delete _adibou2AppliHandler[i];
		delete _adibou2CriteHandler[i];
		delete _adibou2ExoHandler[i];
	}

	for (uint32 i = 0; i < kAdibou2NbrOfApplications; i++) {
		delete _adibou2AppIcoHandler[i];
	}

	delete _adibou2ApplicationsInfoHandler;
	delete _adibou2RetourHandler;
	delete _adibou2LanceHandler;
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
