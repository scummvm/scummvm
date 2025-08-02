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

#include "bagel/afxwin.h"
#include "bagel/boflib/misc.h"
#include "bagel/boflib/error.h"
#include "bagel/hodjnpodj/metagame/frame/resource.h"
#include "bagel/hodjnpodj/metagame/bgen/bfc.h"
#include "bagel/hodjnpodj/metagame/bgen/item.h"
#include "bagel/hodjnpodj/metagame/bgen/invent.h"
#include "bagel/hodjnpodj/metagame/saves/restgame.h"
#include "bagel/hodjnpodj/metagame/saves/restdlg.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Saves {

STATIC CHAR *gpszSaveGameFileName;

STATIC CHAR szDescriptions[MAX_SAVEGAMES][40];
STATIC CHAR *pszDesc[MAX_SAVEGAMES];


// Local prototypes
//
ERROR_CODE GetSaveGameDescriptions(VOID);
ERROR_CODE RestoreSlot(INT, CBfcMgr *);
ERROR_CODE ReadSavedGame(void *, INT, SAVEGAME_INFO *);

ERROR_CODE GetSaveGameDescriptions() {
	#ifdef TODO
	SAVEGAME_INFO *pSaveGameInfo;
	FILE *pFile;
	INT i;
	ERROR_CODE errCode;

	// assume no error
	errCode = ERR_NONE;

	// can't restore a game unless the .SAV file exists
	//
	if (FileExists(gpszSaveGameFileName)) {

		if ((pSaveGameInfo = (SAVEGAME_INFO *)malloc(sizeof(SAVEGAME_INFO))) != nullptr) {

			// open the .SAV file
			//
			if ((pFile = fopen(gpszSaveGameFileName, "rb")) != nullptr) {

				for (i = 0; i < MAX_SAVEGAMES; i++) {

					if ((errCode = ReadSavedGame(pFile, i, pSaveGameInfo)) == ERR_NONE) {

						assert(strlen(pSaveGameInfo->m_szSaveGameDescription) < 40);
						Common::strcpy_s(pszDesc[i] = szDescriptions[i], pSaveGameInfo->m_szSaveGameDescription);

						if (!pSaveGameInfo->m_bUsed) {
							szDescriptions[i][0] = '\0';
							pszDesc[i] = nullptr;
						}

					} else {
						break;
					}
				}
				fclose(pFile);
			} else {
				errCode = ERR_FOPEN;
			}

			free(pSaveGameInfo);
		} else {
			errCode = ERR_MEMORY;
		}

	} else {
		ErrorLog("ERROR.LOG", "%s not found.  Can't restore game.", gpszSaveGameFileName);
	}

	return (errCode);
	#else
	error("TODO: GetSaveGameDescriptions");
	#endif
}


/*****************************************************************************
*
*  RestoreSlot      - one line discription
*
*  DESCRIPTION:
*
*
*  SAMPLE USAGE:
*  errCode = RestoreSlot(iGameNum, pBfcMgr);
*  INT iGameNum;                            Game slot to retore game from
*  CBfcMgr *pBfcMgr;                        destination object of restored info
*
*  RETURNS:  ERROR_CODE = Error return code.
*
*****************************************************************************/
ERROR_CODE RestoreSlot(INT iGameNum, CBfcMgr *pBfcMgr) {
	#ifdef TODO
	SAVEGAME_INFO *pSaveGameInfo;
	FILE *pFile;
	ERROR_CODE errCode;

	// assume no error
	errCode = ERR_NONE;

	assert(iGameNum >= 0 && iGameNum < MAX_SAVEGAMES);
	assert(pBfcMgr != nullptr);

	assert(FileExists(gpszSaveGameFileName));

	if ((pSaveGameInfo = (SAVEGAME_INFO *)malloc(sizeof(SAVEGAME_INFO))) != nullptr) {

		if ((pFile = fopen(gpszSaveGameFileName, "rb")) != nullptr) {

			#ifdef _DEBUG
			memset(pSaveGameInfo, 0, sizeof(SAVEGAME_INFO));
			#endif

			if ((errCode = ReadSavedGame(pFile, iGameNum, pSaveGameInfo)) == ERR_NONE) {

				if (ValidateSGInfo(pSaveGameInfo)) {

					errCode = ConvertFromSGI(pBfcMgr, pSaveGameInfo);
				} else {
					errCode = ERR_FTYPE;
				}
			}

			fclose(pFile);
		} else {
			errCode = ERR_FOPEN;
		}

		free(pSaveGameInfo);
	} else {
		errCode = ERR_MEMORY;
	}

	#ifdef _DEBUG
	if (errCode != ERR_NONE) {
		ErrorLog("ERROR.LOG", "errCode = %d in %s at line %d", errCode, __FILE__, __LINE__);
	}
	#endif

	return (errCode);
	#else
	error("TODO: RestoreSlot");
	#endif
}


ERROR_CODE ReadSavedGame(void *pFile, INT iGameNum, SAVEGAME_INFO *pSaveGameInfo) {
	#ifdef TODO
	ERROR_CODE errCode;

	// asume no error
	errCode = ERR_NONE;

	// validate explicit input
	assert(pFile != nullptr);
	assert(iGameNum >= 0 && iGameNum < MAX_SAVEGAMES);
	assert(pSaveGameInfo != nullptr);

	// seek to correct save game number (slot)
	//
	if (fseek(pFile, iGameNum * sizeof(SAVEGAME_INFO), SEEK_SET) == 0) {

		// write the save game to that slot
		//
		if (fread(pSaveGameInfo, sizeof(SAVEGAME_INFO), 1, pFile) == 1) {

			Decrypt(pSaveGameInfo, sizeof(SAVEGAME_INFO));

			// if this fails, then we are trying to restore an old saved game file
			// and the SAVEGAME_INFO structure has changed. (i.e. the .SAV file is
			// incompatable with this version of the game)
			//
			assert(pSaveGameInfo->m_nFixedRecordSize == sizeof(SAVEGAME_INFO));

		} else {
			errCode = ERR_FREAD;
		}

	} else {
		errCode = ERR_FSEEK;
	}

	return (errCode);
	#else
	error("TODO: ReadSavedGame");
	#endif
}

} // namespace Saves
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
