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

#include "bagel/baglib/save_game_file.h"
#include "bagel/boflib/misc.h"

namespace Bagel {

#define WORLDDIR "$SBARDIR\\WLD\\%s"

CBagSaveGameFile::CBagSaveGameFile(const CHAR *pszFileName) {
	SetFile(pszFileName, (CDF_MEMORY | CDF_ENCRYPT | CDF_KEEPOPEN | CDF_CREATE));
}

ERROR_CODE CBagSaveGameFile::WriteSavedGame(LONG lSlot, ST_SAVEDGAME_HEADER *pSavedGame, VOID *pDataBuf, LONG lDataSize) {
	Assert(IsValidObject(this));

	// validate input
	Assert(lSlot >= 0 && lSlot < MAX_SAVEDGAMES);
	Assert(pSavedGame != nullptr);
	Assert(lDataSize >= 0);

	UBYTE *pBuf;
	LONG lSize, lRecNum;

#if BOF_MAC
	PCizeSavedGame(pDataBuf, lDataSize);
#endif

	lSize = sizeof(ST_SAVEDGAME_HEADER) + lDataSize;
	if ((pBuf = (UBYTE *)BofAlloc(lSize)) != nullptr) {

		BofMemCopy(pBuf, pSavedGame, sizeof(ST_SAVEDGAME_HEADER));

		if (lDataSize > 0) {
			BofMemCopy(pBuf + sizeof(ST_SAVEDGAME_HEADER), pDataBuf, lDataSize);
		}

		if ((lRecNum = FindRecord(lSlot)) == -1) {
			AddRecord(pBuf, lSize, TRUE, lSlot);

		} else {
			//DeleteRecord(lRecNum, TRUE);
			//AddRecord(pBuf, lSize, TRUE, lSlot);
			WriteRecord(lRecNum, pBuf, lSize, TRUE, lSlot);
		}

		BofFree(pBuf);
	} else {
		ReportError(ERR_MEMORY, "Could not allocate %ld bytes for saved game", lSize);
	}

	return (m_errCode);
}

ERROR_CODE CBagSaveGameFile::ReadSavedGame(LONG lSlot, ST_SAVEDGAME_HEADER *pSavedGame, VOID *pDataBuf, LONG lDataSize) {
	Assert(IsValidObject(this));

	// Validate input
	Assert(lSlot >= 0 && lSlot < MAX_SAVEDGAMES);
	Assert(pSavedGame != nullptr);
	Assert(lDataSize >= 0);

	UBYTE *pBuf;
	LONG lSize, lRecNum;

	if ((lRecNum = FindRecord(lSlot)) != -1) {

		lSize = GetRecSize(lRecNum);

		if ((pBuf = (UBYTE *)BofAlloc(lSize)) != nullptr) {

			ReadRecord(lRecNum, pBuf);

			// fill ST_SAVEDGAME_HEADER structure with this game's saved info
			BofMemCopy(pSavedGame, pBuf, sizeof(ST_SAVEDGAME_HEADER));

			// NOTE: pDataBuf must point to a buffer that has already been
			// allocated to hold the actual saved game data.
			//
			if (pDataBuf != nullptr) {
				BofMemCopy(pDataBuf, pBuf + sizeof(ST_SAVEDGAME_HEADER), lDataSize);
#if BOF_MAC
				MacintizeSavedGame(pDataBuf, lDataSize);
#endif
			}

			BofFree(pBuf);

		} else {
			ReportError(ERR_MEMORY, "Could not allocate %ld bytes to restore a saved game", lSize);
		}

	} else {
		ReportError(ERR_UNKNOWN, "Unable to find saved game #%ld in %s", lSlot, m_szFileName);
	}

	return (m_errCode);
}

ERROR_CODE CBagSaveGameFile::ReadTitle(LONG lSlot, ST_SAVEDGAME_HEADER *pSavedGame) {
	Assert(IsValidObject(this));

	// validate input
	Assert(lSlot >= 0 && lSlot < MAX_SAVEDGAMES);
	Assert(pSavedGame != nullptr);

	UBYTE *pBuf;
	LONG lSize, lRecNum;

	if ((lRecNum = FindRecord(lSlot)) != -1) {

		lSize = GetRecSize(lRecNum);

		if ((pBuf = (UBYTE *)BofAlloc(lSize)) != nullptr) {

			ReadRecord(lRecNum, pBuf);

			// Fill ST_SAVEDGAME_HEADER structure with this game's saved info
			BofMemCopy(pSavedGame, pBuf, sizeof(ST_SAVEDGAME_HEADER));

			BofFree(pBuf);

		} else {
			ReportError(ERR_MEMORY, "Could not allocate %ld bytes to read a saved game title", lSize);
		}

	} else {
		ReportError(ERR_UNKNOWN, "Unable to find saved game #%ld in %s", lSlot, m_szFileName);
	}

	return (m_errCode);
}

ERROR_CODE CBagSaveGameFile::ReadTitleOnly(LONG lSlot, CHAR *pGameTitle) {
	Assert(IsValidObject(this));

	// Validate input
	Assert(lSlot >= 0 && lSlot < MAX_SAVEDGAMES);
	Assert(pGameTitle != nullptr);

	UBYTE pBuf[MAX_SAVETITLE + 1];
	LONG lSize, lRecNum;

	if ((lRecNum = FindRecord(lSlot)) != -1) {
		lSize = MAX_SAVETITLE;
		ReadFromFile(lRecNum, pBuf, lSize);

		// Fill with current game title
		BofMemCopy(pGameTitle, pBuf, lSize);

	} else {
		ReportError(ERR_UNKNOWN, "Unable to find saved game #%ld in %s", lSlot, m_szFileName);
	}

	return (m_errCode);
}

LONG CBagSaveGameFile::GetActualNumSaves() {
	Assert(IsValidObject(this));

	ST_SAVEDGAME_HEADER stGameInfo;
	LONG i, lNumRecs, lNumSaves;

	lNumSaves = 0;
	lNumRecs = GetNumSavedGames();
	for (i = 0; i < lNumRecs; i++) {

		if (ReadTitle(i, &stGameInfo) == ERR_NONE) {

			if (stGameInfo.m_bUsed) {
				lNumSaves++;
			}
		} else {
			break;
		}
	}

	return lNumSaves;
}

BOOL CBagSaveGameFile::AnySavedGames() {
	Assert(IsValidObject(this));

	ST_SAVEDGAME_HEADER stGameInfo;
	LONG i, lNumRecs;

	lNumRecs = GetNumSavedGames();
	for (i = 0; i < lNumRecs; i++) {
		if (ReadTitle(i, &stGameInfo) == ERR_NONE) {
			if (stGameInfo.m_bUsed) {
				return TRUE;
			}
		} else {
			break;
		}
	}

	return FALSE;
}

#if BOF_MAC
// Translate any PC specific info in the game file to mac land
VOID CBagSaveGameFile::MacintizeSavedGame(VOID *pBuf, LONG pBufLen) {
	ST_BAGEL_SAVE *pstBagGame = (ST_BAGEL_SAVE *)pBuf;
	CHAR *p;
	CHAR            szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString      sStr(szLocalBuff, 256);

	SwapSavedGame(pBuf, pBufLen);

	// Fix the filename... don't bother if there is none.

	if (strlen(pstBagGame->m_szScript) == 0) {
		return;
	}

	p = strchr(pstBagGame->m_szScript, '\\');
	while (p) {
		*p = ':';
		p = strchr(p, '\\');
	}

	// Gotta figure out the name of the world script.  Try this three
	// tier approach:
	//
	// If file exists, we're done.
	// If not, do a macro replace and check if that file exists, if it does, we're done
	// otherwise, find the name of the .wld file and build a valid script name out of that

	if (FileExists(pstBagGame->m_szScript)) {
		return;
	}

	sStr = pstBagGame->m_szScript;
	MACROREPLACE(sStr);
	if (FileExists(sStr.GetBuffer())) {
		strcpy(pstBagGame->m_szScript, sStr.GetBuffer());
		return;
	}

	// Nope, still not there... work back from the end of the file path until we
	// hit the delimeter then tack a $SBARDIR:WLD on the front of that.

	p = &szLocalBuff[sStr.GetLength() - 1];
	while ((*p != *PATH_DELIMETER) && p != szLocalBuff) {
		p--;
	}

	if (*p == *PATH_DELIMETER) {
		sprintf(szLocalBuff, "$SBARDIR%sWLD%s%s", PATH_DELIMETER, PATH_DELIMETER, &p[1]);
		MACROREPLACE(sStr);

		strcpy(pstBagGame->m_szScript, sStr.GetBuffer());
	}
}

VOID CBagSaveGameFile::PCizeSavedGame(VOID *pBuf, LONG pBufLen) {
	ST_BAGEL_SAVE *pstBagGame = (ST_BAGEL_SAVE *)pBuf;
	CHAR *p;
	CHAR            szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString      sGameStr(pstBagGame->m_szScript, MAX_FNAME);

	if (pBuf == nullptr || pBufLen == 0) {
		return;
	}

	SwapSavedGame(pBuf, pBufLen);

	if (sGameStr.GetLength() == 0) {
		return;
	}

	// Parse this bad baby to be sbardir\wld\script name

	p = sGameStr.GetBuffer() + sGameStr.GetLength() - 1;
	while ((*p != *PATH_DELIMETER) && p != szLocalBuff) {
		p--;
	}

	if (*p == *PATH_DELIMETER) {
		sprintf(szLocalBuff, WORLDDIR, &p[1]);
		sGameStr = szLocalBuff;
	}
}

// This will swap all the shorts and longs in the buffer.
VOID CBagSaveGameFile::SwapSavedGame(VOID *pBuf, LONG pBufLen) {
	ST_BAGEL_SAVE *p = (ST_BAGEL_SAVE *)pBuf;
	INT     i;

	Assert(pBufLen >= sizeof(ST_BAGEL_SAVE));

	//  Here's the structure of the game file that we'll be receiving...
	//  typedef struct {
	//      ULONG   m_lStructSize;                  // sizeof(ST_BAGEL_SAVE)
	//      ST_VAR  m_stVarList[MAX_VARS];
	//      ST_OBJ  m_stObjList[MAX_OBJS];
	//      ST_OBJ  m_stObjListEx[MAX_OBJS];
	//      CHAR    m_szScript[MAX_FNAME];          // Name of current world file (no path)
	//      ULONG   m_nLocType;                     // TYPE_PAN, TYPE_CLOSUP, etc...
	//      CHAR    m_szLocStack[MAX_CLOSEUP_DEPTH][MAX_SDEV_NAME]; // Your storage device stack
	//      USHORT  m_nLocX;                        // X Location in PAN
	//      USHORT  m_nLocY;                        // Y Location in PAN
	//      USHORT  m_bUseEx;
	//  } ST_BAGEL_SAVE;

	//  ULONG   m_lStructSize;                  // sizeof(ST_BAGEL_SAVE)
	p->m_lStructSize = SWAPLONG(p->m_lStructSize);

	for (i = 0; i < MAX_VARS; i++) {
		p->m_stVarList[i].m_nType = SWAPWORD(p->m_stVarList[i].m_nType);
	}

	//      ST_OBJ  m_stObjList[MAX_OBJS];
	for (i = 0; i < MAX_OBJS; i++) {

		// Swap all longs in the structure.
		p->m_stObjList[i].m_lState = SWAPLONG(p->m_stObjList[i].m_lState);
		p->m_stObjList[i].m_lProperties = SWAPLONG(p->m_stObjList[i].m_lProperties);
		p->m_stObjList[i].m_lType = SWAPLONG(p->m_stObjList[i].m_lType);
		p->m_stObjList[i].m_lLoop = SWAPLONG(p->m_stObjList[i].m_lLoop);
		p->m_stObjList[i].m_lSpeed = SWAPLONG(p->m_stObjList[i].m_lSpeed);
		p->m_stObjList[i].m_nFlags = SWAPWORD(p->m_stObjList[i].m_nFlags);
	}

	//      ST_OBJ  m_stObjListEx[MAX_OBJS];
	for (i = 0; i < MAX_OBJS; i++) {

		// Swap all longs in the structure.
		p->m_stObjListEx[i].m_lState = SWAPLONG(p->m_stObjListEx[i].m_lState);
		p->m_stObjListEx[i].m_lProperties = SWAPLONG(p->m_stObjListEx[i].m_lProperties);
		p->m_stObjListEx[i].m_lType = SWAPLONG(p->m_stObjListEx[i].m_lType);
		p->m_stObjListEx[i].m_lLoop = SWAPLONG(p->m_stObjListEx[i].m_lLoop);
		p->m_stObjListEx[i].m_lSpeed = SWAPLONG(p->m_stObjListEx[i].m_lSpeed);
		p->m_stObjListEx[i].m_nFlags = SWAPWORD(p->m_stObjListEx[i].m_nFlags);
	}

	//      CHAR    m_szScript[MAX_FNAME];          // Name of current world file (no path)
	//      ULONG   m_nLocType;                     // TYPE_PAN, TYPE_CLOSUP, etc...
	p->m_nLocType = SWAPLONG(p->m_nLocType);

	//      CHAR    m_szLocStack[MAX_CLOSEUP_DEPTH][MAX_SDEV_NAME]; // Your storage device stack
	//      USHORT  m_nLocX;                        // X Location in PAN
	p->m_nLocX = SWAPWORD(p->m_nLocX);

	//      USHORT  m_nLocY;                        // Y Location in PAN
	p->m_nLocY = SWAPWORD(p->m_nLocY);

	//      USHORT  m_bUseEx;
	p->m_bUseEx = SWAPWORD(p->m_bUseEx);
}

#endif

} // namespace Bagel
