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

#include "common/algorithm.h"
#include "common/memstream.h"
#include "common/serializer.h"
#include "bagel/baglib/save_game_file.h"
#include "bagel/boflib/misc.h"
#include "bagel/bagel.h"

namespace Bagel {

#define WORLD_DIR "$SBARDIR\\WLD\\%s"

void StSavegameHeader::synchronize(Common::Serializer &s) {
	s.syncBytes((byte *)_szTitle, MAX_SAVE_TITLE);
	s.syncBytes((byte *)_szUserName, MAX_USER_NAME);
	s.syncAsUint32LE(_bUsed);
}

void StVar::synchronize(Common::Serializer &s) {
	s.syncBytes((byte *)_szName, MAX_VAR_NAME);
	s.syncBytes((byte *)_szValue, MAX_VAR_VALUE);
	s.syncAsUint16LE(_nType);

	s.syncAsByte(_bGlobal);
	s.syncAsByte(_bConstant);
	s.syncAsByte(_bReference);
	s.syncAsByte(_bTimer);

	s.syncAsByte(_bRandom);
	s.syncAsByte(_bNumeric);
	s.syncAsByte(_bAttached);

	s.syncAsByte(_bUsed);
}

void StVar::clear() {
	Common::fill(_szName, _szName + MAX_VAR_NAME, '\0');
	Common::fill(_szValue, _szValue + MAX_VAR_VALUE, '\0');

	_nType = 0;
	_bGlobal = 0;
	_bConstant = 0;
	_bReference = 0;
	_bTimer = 0;
	_bRandom = 0;
	_bNumeric = 0;
	_bAttached = 0;
	_bUsed = 0;
}

void StObj::synchronize(Common::Serializer &s) {
	s.syncBytes((byte *)_szName, MAX_OBJ_NAME);
	s.syncBytes((byte *)_szSDev, MAX_SDEV_NAME);
	s.syncAsUint32LE(_lState);

	s.syncAsUint32LE(_lProperties);
	s.syncAsUint32LE(_lType);

	s.syncAsUint32LE(_lLoop);
	s.syncAsUint32LE(_lSpeed);

	s.syncAsByte(_bAttached);
	s.syncAsByte(_bUsed);

	s.syncAsUint16LE(_nFlags);
}

void StObj::clear() {
	Common::fill(_szName, _szName + MAX_OBJ_NAME, '\0');
	Common::fill(_szSDev, _szSDev + MAX_SDEV_NAME, '\0');

	_lState = 0;
	_lProperties = 0;
	_lType = 0;
	_lLoop = 0;
	_lSpeed = 0;
	_bAttached = 0;
	_bUsed = 0;
	_nFlags = 0;
}

void StBagelSave::synchronize(Common::Serializer &s) {
	for (int i = 0; i < MAX_VARS; ++i)
		_stVarList[i].synchronize(s);
	for (int i = 0; i < MAX_OBJS; ++i)
		_stObjList[i].synchronize(s);
	for (int i = 0; i < MAX_OBJS; ++i)
		_stObjListEx[i].synchronize(s);

	s.syncBytes((byte *)_szScript, MAX_FNAME);
	s.syncAsUint32LE(_nLocType);

	for (int i = 0; i < MAX_CLOSEUP_DEPTH; ++i)
		s.syncBytes((byte *)_szLocStack[i], MAX_SDEV_NAME);

	s.syncAsUint16LE(_nLocX);
	s.syncAsUint16LE(_nLocY);
	s.syncAsUint16LE(_bUseEx);
	s.syncAsUint16LE(_nFiller);
}

void StBagelSave::clear() {
	for (int i = 0; i < MAX_VARS; ++i)
		_stVarList[i].clear();
	for (int i = 0; i < MAX_OBJS; ++i) {
		_stObjList[i].clear();
		_stObjListEx[i].clear();
	}

	Common::fill(_szScript, _szScript + MAX_FNAME, '\0');
	_nLocType = 0;

	for (int i = 0; i < MAX_CLOSEUP_DEPTH; ++i)
		Common::fill(_szLocStack[i], _szLocStack[i] + MAX_SDEV_NAME, '\0');

	_nLocX = 0;
	_nLocY = 0;
	_bUseEx = 0;
	_nFiller = 0;
}


CBagSaveGameFile::CBagSaveGameFile(bool isSaving) {
	setFile("spacebar.sav",
		isSaving ?
		(CDF_MEMORY | CDF_ENCRYPT | CDF_KEEPOPEN | CDF_CREATE | CDF_SAVEFILE) :
		(CDF_MEMORY | CDF_ENCRYPT | CDF_KEEPOPEN | CDF_SAVEFILE)
	);
}

ErrorCode CBagSaveGameFile::writeSavedGame() {
	assert(isValidObject(this));

	// Populate the save data
	StBagelSave *saveData = new StBagelSave();
	g_engine->_masterWin->fillSaveBuffer(saveData);

	Common::String str = "./" + Common::String(saveData->_szScript);
	str.replace('/', '\\');
	Common::strcpy_s(saveData->_szScript, str.c_str());

	// Set up header fields
	StSavegameHeader header;
	Common::strcpy_s(header._szTitle, "ScummVM Save");
	Common::strcpy_s(header._szUserName, "ScummVM User");
	header._bUsed = 1;

	// Create the data buffer
	Common::MemoryWriteStreamDynamic stream(DisposeAfterUse::YES);
	Common::Serializer s(nullptr, &stream);

	header.synchronize(s);
	stream.writeUint32LE(StBagelSave::size());
	saveData->synchronize(s);

	// Add the record
	addRecord(stream.getData(), stream.size(), true, 0);

	delete saveData;

	return _errCode;
}

ErrorCode CBagSaveGameFile::readSavedGame(int32 slotNum) {
	assert(isValidObject(this));

	int32 lRecNum = findRecord(slotNum);
	if (lRecNum != -1) {
		int32 lSize = getRecSize(lRecNum);

		if (lSize == StSavegameHeader::size()) {
			_errCode = ERR_FREAD;
		} else {
			byte *pBuf = (byte *)bofAlloc(lSize);
			readRecord(lRecNum, pBuf);

			// Load in the savegame
			Common::MemoryReadStream stream(pBuf, lSize);
			Common::Serializer s(&stream, nullptr);
			StSavegameHeader header;
			header.synchronize(s);
			s.skip(4);		// Skip save data structure size
			StBagelSave *saveData = new StBagelSave();
			saveData->synchronize(s);

			bofFree(pBuf);

			CBofString str(saveData->_szScript);
			fixPathName(str);
			const char *path = str.getBuffer();
			assert(!strncmp(path, "./", 2));
			Common::strcpy_s(saveData->_szScript, path + 2);

			// Restore the game
			g_engine->_masterWin->doRestore(saveData);

			delete saveData;
		}
	} else {
		_errCode = ERR_FREAD;
	}

	return _errCode;
}

ErrorCode CBagSaveGameFile::readTitle(int32 lSlot, StSavegameHeader *pSavedGame) {
	assert(isValidObject(this));

	// validate input
	assert(lSlot >= 0 && lSlot < MAX_SAVED_GAMES);
	assert(pSavedGame != nullptr);

	int32 lRecNum = findRecord(lSlot);

	if (lRecNum != -1) {
		int32 lSize = getRecSize(lRecNum);

		byte *pBuf = (byte *)bofAlloc(lSize);
		readRecord(lRecNum, pBuf);

		// Fill StSavegameHeader structure with this game's saved info
		memcpy(pSavedGame, pBuf, sizeof(StSavegameHeader));
		bofFree(pBuf);

	} else {
		reportError(ERR_UNKNOWN, "Unable to find saved game #%ld in %s", lSlot, _szFileName);
	}

	return(_errCode);
}

ErrorCode CBagSaveGameFile::readTitleOnly(int32 lSlot, char *pGameTitle) {
	assert(isValidObject(this));

	// Validate input
	assert(lSlot >= 0 && lSlot < MAX_SAVED_GAMES);
	assert(pGameTitle != nullptr);

	byte pBuf[MAX_SAVE_TITLE + 1];
	int32 lRecNum = findRecord(lSlot);
	if (lRecNum != -1) {
		int32 lSize = MAX_SAVE_TITLE;
		readFromFile(lRecNum, pBuf, lSize);

		// Fill with current game title
		memcpy(pGameTitle, pBuf, lSize);

	} else {
		reportError(ERR_UNKNOWN, "Unable to find saved game #%ld in %s", lSlot, _szFileName);
	}

	return _errCode;
}


int32 CBagSaveGameFile::getActualNumSaves() {
	assert(isValidObject(this));

	int32 lNumSaves = 0;
	int32 lNumRecs = getNumSavedGames();
	for (int32 i = 0; i < lNumRecs; i++) {
		StSavegameHeader stGameInfo;
		if (readTitle(i, &stGameInfo) == ERR_NONE) {
			if (stGameInfo._bUsed) {
				lNumSaves++;
			}
		} else {
			break;
		}
	}

	return(lNumSaves);
}

bool CBagSaveGameFile::anySavedGames() {
	assert(isValidObject(this));

	int32 lNumRecs = getNumSavedGames();
	for (int32 i = 0; i < lNumRecs; i++) {
		StSavegameHeader stGameInfo;
		if (readTitle(i, &stGameInfo) == ERR_NONE) {

			if (stGameInfo._bUsed) {
				return true;
			}
		} else {
			break;
		}
	}

	return false;
}

} // namespace Bagel
