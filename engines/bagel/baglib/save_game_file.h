
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
 * aint32 with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BAGEL_BAGLIB_SAVE_GAME_FILE_H
#define BAGEL_BAGLIB_SAVE_GAME_FILE_H

#include "common/serializer.h"
#include "bagel/boflib/dat_file.h"

namespace Bagel {

#define MAX_SAVEDGAMES 40
#define MAX_SAVETITLE 128
#define MAX_USERNAME 64

struct ST_SAVEDGAME_HEADER {
	char m_szTitle[MAX_SAVETITLE] = { '\0' };
	char m_szUserName[MAX_USERNAME] = { '\0' };
	uint32 m_bUsed = 0;
};

#define MAX_SDEV_NAME 40
#define MAX_CLOSEUP_DEPTH 4

#define MAX_VARS 1000
#define MAX_VAR_NAME 40
#define MAX_VAR_VALUE 60

struct ST_VAR {
	char m_szName[MAX_VAR_NAME];
	char m_szValue[MAX_VAR_VALUE];
	uint16 m_nType;

	byte m_bGlobal;
	byte m_bConstant;
	byte m_bReference;
	byte m_bTimer;

	byte m_bRandom;
	byte m_bNumeric;
	byte m_bAttached;

	byte m_bUsed; // If this entry is used or not

	void synchronize(Common::Serializer &s);
	void clear();
};

#define MAX_OBJ_NAME 40
#define MAX_SDEV_NAME 40
#define MAX_OBJS 1000

struct ST_OBJ {
	char m_szName[MAX_OBJ_NAME];
	char m_szSDev[MAX_SDEV_NAME];
	uint32 m_lState;

	uint32 m_lProperties;
	uint32 m_lType;

	uint32 m_lLoop;
	uint32 m_lSpeed;

	byte m_bAttached;
	byte m_bUsed;

	uint16 m_nFlags; // Flags for kicks...

	void synchronize(Common::Serializer &s);
	void clear();
};

// Flags for the st_obj strucuture

#define mIsMsgWaiting 0x0001

/**
 * Savegame data structure
 */
struct ST_BAGEL_SAVE {
	ST_VAR m_stVarList[MAX_VARS];
	ST_OBJ m_stObjList[MAX_OBJS];
	ST_OBJ m_stObjListEx[MAX_OBJS];
	char m_szScript[MAX_FNAME];                          // Name of current world file (no path)
	uint32 m_nLocType;                                    // TYPE_PAN, TYPE_CLOSUP, etc...
	char m_szLocStack[MAX_CLOSEUP_DEPTH][MAX_SDEV_NAME]; // Your storage device stack
	uint16 m_nLocX;                                      // X Location in PAN
	uint16 m_nLocY;                                      // Y Location in PAN
	uint16 m_bUseEx;
	uint16 m_nFiller; // Make structs align

	void synchronize(Common::Serializer &s);
	void clear();
};

/**
 * Handles reading/writing the original savegame file that contains
 * all the saves. Used by the console save/load commands.
*/
class CBagSaveGameFile : public CBofDataFile {
public:
	CBagSaveGameFile(const char *pszFileName);

	int32 GetNumSavedGames() const {
		return GetNumberOfRecs();
	}
	int32 GetActualNumSaves();
	bool AnySavedGames();

	/**
	 * Saves a BAGEL game to current save game file
	 */
	ErrorCode WriteSavedGame(int32 lSaveGamePos, ST_SAVEDGAME_HEADER *pSavedGame, void *pDataBuf, int32 lDataSize);

	/**
	 * Restore a BAGEL saved game
	 */
	ErrorCode ReadSavedGame(int32 lSaveGamePos, ST_SAVEDGAME_HEADER *pSavedGame, void *pDataBuf, int32 lDataSize);

	/**
	 * Reads a BAGEL saved game title
	 */
	ErrorCode ReadTitle(int32 lSlot, ST_SAVEDGAME_HEADER *pSavedGame);

	ErrorCode ReadTitleOnly(int32 lSlot, char *pGameTitle);
};

} // namespace Bagel

#endif
