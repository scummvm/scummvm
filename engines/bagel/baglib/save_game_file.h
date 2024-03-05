
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

#include "bagel/boflib/dat_file.h"

namespace Bagel {

#define MAX_SAVEDGAMES 40
#define MAX_SAVETITLE 128
#define MAX_USERNAME 64

struct ST_SAVEDGAME_HEADER {
	CHAR m_szTitle[MAX_SAVETITLE];
	CHAR m_szUserName[MAX_USERNAME];
	ULONG m_bUsed;
};

#define MAX_SDEV_NAME 40
#define MAX_CLOSEUP_DEPTH 4

#define MAX_VARS 1000
#define MAX_VAR_NAME 40
#define MAX_VAR_VALUE 60

struct ST_VAR {
	CHAR m_szName[MAX_VAR_NAME];
	CHAR m_szValue[MAX_VAR_VALUE];
	USHORT m_nType;

	UBYTE m_bGlobal;
	UBYTE m_bConstant;
	UBYTE m_bReference;
	UBYTE m_bTimer;

	UBYTE m_bRandom;
	UBYTE m_bNumeric;
	UBYTE m_bAttached;

	UBYTE m_bUsed; // If this entry is used or not
};

#define MAX_OBJ_NAME 40
#define MAX_SDEV_NAME 40
#define MAX_OBJS 1000

struct ST_OBJ {
	CHAR m_szName[MAX_OBJ_NAME];
	CHAR m_szSDev[MAX_SDEV_NAME];
	ULONG m_lState;

	ULONG m_lProperties;
	ULONG m_lType;

	ULONG m_lLoop;
	ULONG m_lSpeed;

	UBYTE m_bAttached;
	UBYTE m_bUsed;

	USHORT m_nFlags; // Flags for kicks...
};

// Flags for the st_obj strucuture

#define mIsMsgWaiting 0x0001

// Ditto, if any fields are added, then please notify the mac guy
typedef struct {
	ULONG m_lStructSize; // sizeof(ST_BAGEL_SAVE)
	ST_VAR m_stVarList[MAX_VARS];
	ST_OBJ m_stObjList[MAX_OBJS];
	ST_OBJ m_stObjListEx[MAX_OBJS];
	CHAR m_szScript[MAX_FNAME];                          // Name of current world file (no path)
	ULONG m_nLocType;                                    // TYPE_PAN, TYPE_CLOSUP, etc...
	CHAR m_szLocStack[MAX_CLOSEUP_DEPTH][MAX_SDEV_NAME]; // Your storage device stack
	USHORT m_nLocX;                                      // X Location in PAN
	USHORT m_nLocY;                                      // Y Location in PAN
	USHORT m_bUseEx;
	USHORT m_nFiller; // Make structs align
} ST_BAGEL_SAVE;

class CBagSaveGameFile : public CBofDataFile {
public:
	CBagSaveGameFile(const CHAR *pszFileName);

	LONG GetNumSavedGames() { return GetNumberOfRecs(); }
	LONG GetActualNumSaves();
	BOOL AnySavedGames();

	/**
	 * Saves a BAGEL game to current save game file
	 * @param lSaveGamePos		Slot to save game into
	 * @param pSavedGame		Saved game description info
	 * @param pDataBuf			Actual game data
	 * @param lDataSize			Size of data buffer
	 * @return					Error return Code
	 */
	ERROR_CODE WriteSavedGame(LONG lSaveGamePos, ST_SAVEDGAME_HEADER *pSavedGame, VOID *pDataBuf, LONG lDataSize);

	/**
	 * Restore a BAGEL saved game
	 * @param lSaveGamePos		Slot to read from
	 * @param pSavedGame		Buffer to fill with saved game info
	 * @param pDataBuf			Buffer to store actual game data
	 * @param lDataSize			Size of data buffer
	 * @return					Error return Code
	 */
	ERROR_CODE ReadSavedGame(LONG lSaveGamePos, ST_SAVEDGAME_HEADER *pSavedGame, VOID *pDataBuf, LONG lDataSize);

	/**
	 * Reads a BAGEL saved game title
	 * @param lSlot				Slot to read from
	 * @param pSavedGame		Buffer to fill with saved game info
	 * @return					Error return Code
	 */
	ERROR_CODE ReadTitle(LONG lSlot, ST_SAVEDGAME_HEADER *pSavedGame);

	/**
	 * Read just the first 128 bytes of the saved game slot.
	 */
	ERROR_CODE ReadTitleOnly(LONG lSlot, CHAR *);

#if BOF_MAC
	VOID SwapSavedGame(VOID *pBuf, LONG pBufLen);
	VOID MacintizeSavedGame(VOID *pBuf, LONG pBufLen);
	VOID PCizeSavedGame(VOID *pBuf, LONG pBufLen);
#endif
};

} // namespace Bagel

#endif
