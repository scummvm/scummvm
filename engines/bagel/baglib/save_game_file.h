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

#ifndef BAGEL_BAGLIB_SAVE_GAME_FILE_H
#define BAGEL_BAGLIB_SAVE_GAME_FILE_H

#include "common/serializer.h"
#include "bagel/boflib/dat_file.h"

namespace Bagel {

#define MAX_SAVED_GAMES 40
#define MAX_SAVE_TITLE 128
#define MAX_USER_NAME 64

struct StSavegameHeader {
	char _szTitle[MAX_SAVE_TITLE] = { '\0' };
	char _szUserName[MAX_USER_NAME] = { '\0' };
	uint32 _bUsed = 0;

	void synchronize(Common::Serializer &s);
	static int size() {
		return MAX_SAVE_TITLE + MAX_USER_NAME + 4;
	}
};

#define MAX_SDEV_NAME 40
#define MAX_CLOSEUP_DEPTH 4

#define MAX_VARS 1000
#define MAX_VAR_NAME 40
#define MAX_VAR_VALUE 60

struct StVar {
	char _szName[MAX_VAR_NAME];
	char _szValue[MAX_VAR_VALUE];
	uint16 _nType;

	byte _bGlobal;
	byte _bConstant;
	byte _bReference;
	byte _bTimer;

	byte _bRandom;
	byte _bNumeric;
	byte _bAttached;

	byte _bUsed; // If this entry is used or not

	void synchronize(Common::Serializer &s);
	void clear();
};

#define MAX_OBJ_NAME 40
#define MAX_SDEV_NAME 40
#define MAX_OBJS 1000

struct StObj {
	char _szName[MAX_OBJ_NAME];
	char _szSDev[MAX_SDEV_NAME];
	uint32 _lState;

	uint32 _lProperties;
	uint32 _lType;

	uint32 _lLoop;
	uint32 _lSpeed;

	byte _bAttached;
	byte _bUsed;

	uint16 _nFlags; // Flags for kicks...

	void synchronize(Common::Serializer &s);
	void clear();
};

// Flags for the st_obj structure

#define mIsMsgWaiting 0x0001

/**
 * Savegame data structure
 */
struct StBagelSave {
	StVar _stVarList[MAX_VARS];
	StObj _stObjList[MAX_OBJS];
	StObj _stObjListEx[MAX_OBJS];
	char _szScript[MAX_FNAME];                          // Name of current world file (no path)
	uint32 _nLocType;                                    // TYPE_PAN, TYPE_CLOSEUP, etc...
	char _szLocStack[MAX_CLOSEUP_DEPTH][MAX_SDEV_NAME]; // Your storage device stack
	uint16 _nLocX;                                      // X Location in PAN
	uint16 _nLocY;                                      // Y Location in PAN
	uint16 _bUseEx;
	uint16 _nFiller; // Make structs align

	void synchronize(Common::Serializer &s);
	void clear();

	static int size() {
		return 318432;
	}
};

/**
 * Handles reading/writing the original savegame file that contains
 * all the saves. Used by the console save/load commands.
*/
class CBagSaveGameFile : public CBofDataFile {
public:
	CBagSaveGameFile(bool isSaving);

	int32 getNumSavedGames() const {
		return getNumberOfRecs();
	}
	int32 getActualNumSaves();
	bool anySavedGames();

	/**
	 * Saves a BAGEL game to current save game file
	 */
	ErrorCode writeSavedGame();

	/**
	 * Restore a BAGEL saved game
	 */
	ErrorCode readSavedGame(int32 slotNum);

	/**
	 * Reads a BAGEL saved game title
	 */
	ErrorCode readTitle(int32 lSlot, StSavegameHeader *pSavedGame);

	ErrorCode readTitleOnly(int32 lSlot, char *pGameTitle);
};

} // namespace Bagel

#endif
