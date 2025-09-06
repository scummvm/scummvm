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

#ifndef BAGEL_METAGAME_SAVES_SAVEGAME_H
#define BAGEL_METAGAME_SAVES_SAVEGAME_H

#include "bagel/hodjnpodj/metagame/bgen/bfc.h"
#include "bagel/hodjnpodj/hnplibs/stdinc.h"
#include "bagel/boflib/error.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Saves {

typedef bool (CALLBACK *LPSAVEFUNC)(const char *, CBfcMgr *, CWnd *, CPalette *, ERROR_CODE *);

/**
 * Converts a CBfcMgr to a SAVEGAME_INFO
 * @param pBfcMgr		Meta Game info to be converted
 * @param pSaveGameInfo	Storage for record info
 * @return				error return code.
 */
extern void ConvertToSGI(CBfcMgr *pBfcMgr, SAVEGAME_INFO *pSaveGameInfo);

extern bool ValidateSGInfo(SAVEGAME_INFO *);
extern ERROR_CODE ConvertFromSGI(CBfcMgr *, SAVEGAME_INFO *);

/**
 * Shows a Save-Game dialog box
 * User can choose which slot they want to save a game into,
 * and can give a description of the saved game.
 */
extern bool SaveGame(CWnd *, CPalette *, ERROR_CODE *);

/**
 * Shows a Restore-Game dialog box
 * User can select which game they want to restore.
**/
extern bool RestoreGame(CWnd *pWnd, CPalette *pPalette, ERROR_CODE *pErrCode);

} // namespace Saves
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
