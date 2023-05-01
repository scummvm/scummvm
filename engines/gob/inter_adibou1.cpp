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

#include "common/str.h"

#include "gob/inter.h"
#include "gob/game.h"
#include "gob/script.h"
#include "gob/save/saveload.h"


namespace Gob {

#define OPCODEVER Inter_Adibou1
#define OPCODEDRAW(i, x)  _opcodesDraw[i]._OPCODEDRAW(OPCODEVER, x)
#define OPCODEFUNC(i, x)  _opcodesFunc[i]._OPCODEFUNC(OPCODEVER, x)
#define OPCODEGOB(i, x)   _opcodesGob[i]._OPCODEGOB(OPCODEVER, x)

Inter_Adibou1::Inter_Adibou1(GobEngine *vm) : Inter_v2(vm) {
}

void Inter_Adibou1::setupOpcodesDraw() {
	Inter_v2::setupOpcodesDraw();
	OPCODEDRAW(0x0A, o1_setRenderFlags);
}

void Inter_Adibou1::setupOpcodesFunc() {
	Inter_v2::setupOpcodesFunc();
}

void Inter_Adibou1::setupOpcodesGob() {
	OPCODEGOB(0, oAdibou1_writeSprite);
	OPCODEGOB(1, oAdibou1_readSprite);
	OPCODEGOB(2, oAdibou1_fillAreaAtPoint);
	OPCODEGOB(10, oAdibou1_getAppliNameFromId);
	OPCODEGOB(11, oAdibou1_listApplications);
}

void Inter_Adibou1::oAdibou1_writeSprite(OpGobParams &params) {
	int16 spriteIndex = _vm->_game->_script->readInt16();
	uint16 varResult = _vm->_game->_script->readUint16();
	uint16 varFileName = _vm->_game->_script->readUint16();

	WRITE_VAR(varResult, 0);

	const char *filename = GET_VAR_STR(varFileName);

	SaveLoad::SaveMode mode = _vm->_saveLoad ? _vm->_saveLoad->getSaveMode(filename) : SaveLoad::kSaveModeNone;
	if (mode == SaveLoad::kSaveModeSave) {
		if (_vm->_saveLoad->save(filename, 0, -spriteIndex - 1, 0)) {
			WRITE_VAR(varResult, 1);
		}
	} else if (mode == SaveLoad::kSaveModeIgnore)
		return;
	else if (mode == SaveLoad::kSaveModeNone)
		warning("Attempted to write to file \"%s\"", filename);
}

void Inter_Adibou1::oAdibou1_readSprite(OpGobParams &params) {
	int16 spriteIndex = _vm->_game->_script->readInt16();
	uint16 varResult = _vm->_game->_script->readUint16();
	uint16 varFileName = _vm->_game->_script->readUint16();

	WRITE_VAR(varResult, 0);

	const char *filename = GET_VAR_STR(varFileName);

	SaveLoad::SaveMode mode = _vm->_saveLoad ? _vm->_saveLoad->getSaveMode(filename) : SaveLoad::kSaveModeNone;
	if (mode == SaveLoad::kSaveModeSave) {
		if (_vm->_saveLoad->load(filename, 0, -spriteIndex - 1, 0)) {
			WRITE_VAR(varResult, 1);
		}
	} else if (mode == SaveLoad::kSaveModeIgnore)
		return;
	else if (mode == SaveLoad::kSaveModeNone)
		warning("Attempted to write to file \"%s\"", filename);
}


void Inter_Adibou1::oAdibou1_fillAreaAtPoint(OpGobParams &params) {
	uint16 varX = _vm->_game->_script->readUint16();
	uint16 varY = _vm->_game->_script->readUint16();
	uint16 varSpriteIndex = _vm->_game->_script->readUint16();
	uint16 varColor = _vm->_game->_script->readUint16();

	int16 x = VAR(varX);
	int16 y = VAR(varY);
	if (_vm->_draw->_needAdjust != 2) {
		x *= 2;
		y *= 2;
	}
	int16 spriteIndex = VAR(varSpriteIndex);
	int16 color = VAR(varColor);

	SurfacePtr destSprite = _vm->_draw->_spritesArray[spriteIndex];
	if (!destSprite) {
		warning("oAdibou1_fillAreaAtPoint(): Sprite %d does not exist", spriteIndex);
		return;
	}

	destSprite->fillAreaAtPoint(x, y, color);
}

void Inter_Adibou1::oAdibou1_getAppliNameFromId(OpGobParams &params) {
	uint16 resultAppliNameVarOffset = 4 * _vm->_game->_script->readUint16();
	uint16 wantedAppliIdVar = _vm->_game->_script->readUint16();
	byte wantedAppliId = READ_VAR_UINT8(wantedAppliIdVar);

	WRITE_VARO_STR(resultAppliNameVarOffset, "");

	Common::ArchiveMemberList files;
	SearchMan.listMatchingMembers(files, "*.BOU");
	bool firstFile = true;
	for (Common::ArchiveMemberPtr &file : files) {
		Common::SeekableReadStream *stream = file->createReadStream();
		char appliName[4];
		stream->read(appliName, 3); // Application name, e.g. "C51"
		appliName[3] = '\0';
		stream->seek(31);
		byte appliId = stream->readByte(); // Application numeric ID
		Common::String appliFilename = Common::String(appliName);
		appliFilename.toUppercase();
		appliFilename += ".BOU";

		if (file->getName() == appliFilename) {
			// First .BOU file is taken as default if the wanted appli is not found
			if (firstFile || appliId == wantedAppliId)
				WRITE_VARO_STR(resultAppliNameVarOffset, appliName);
			if (appliId == wantedAppliId)
				break;
		}

		firstFile = false;
		delete stream;
	}
}

void Inter_Adibou1::oAdibou1_listApplications(OpGobParams &params) {
	uint16 listAppliIdsVarOffset = 4 * _vm->_game->_script->readUint16();
	uint16 listAppliNamesVarOffset = 4 * _vm->_game->_script->readUint16();

	for (int i = 0; i < 40; i++)
		WRITE_VARO_UINT8(listAppliNamesVarOffset + i, '\0');

	WRITE_VARO_UINT8(listAppliIdsVarOffset, 0);

	// Look for installed applications .BOU files (C51.BOU, L51.BOU...)
	Common::ArchiveMemberList files;
	SearchMan.listMatchingMembers(files, "*.BOU");
	for (Common::ArchiveMemberPtr &file : files) {
		Common::SeekableReadStream *stream = file->createReadStream();
		char appliName[4];
		stream->read(appliName, 3); // Application name, e.g. "C51"
		appliName[3] = '\0';
		stream->seek(31);
		byte idAppli = stream->readByte(); // Application numeric ID

		Common::String appliFilename = Common::String(appliName);
		appliFilename.toUppercase();
		appliFilename += ".BOU";

		WRITE_VARO_UINT8(listAppliIdsVarOffset, idAppli);
		WRITE_VARO_UINT8(listAppliNamesVarOffset, appliFilename[0]);
		WRITE_VARO_UINT8(listAppliNamesVarOffset + 1, appliFilename[1]);

		if (file->getName() == appliFilename) {
			++listAppliIdsVarOffset;
			listAppliNamesVarOffset += 2;
		}

		delete stream;
	}
}

} // End of namespace Gob
