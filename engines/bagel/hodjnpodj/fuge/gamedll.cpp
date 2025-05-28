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
#include "bagel/hodjnpodj/hnplibs/gamedll.h"
#include "bagel/hodjnpodj/fuge/fuge.h"

namespace Bagel {
namespace HodjNPodj {
namespace Fuge {

HWND FAR PASCAL RunFuge(HWND, LPGAMESTRUCT);

HINSTANCE   hDLLInst;
HINSTANCE   hExeInst;
extern LPGAMESTRUCT pGameParams;

// global the pointer to the your game's main window
HWND ghParentWnd;

/////////////////////////////////////////////////////////////////////////////
// Public C interface

/*****************************************************************
 *
 * RunFuge
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *          This is the API function for the DLL. It is what the calling app
 *          calls to invoke poker
 *
 * FORMAL PARAMETERS:
 *
 *      hParentWnd, lpGameInfo
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      n/a
 *
 * RETURN VALUE:
 *
 *      n/a
 *
 ****************************************************************/
//extern "C"
HWND FAR PASCAL RunFuge(HWND hParentWnd, LPGAMESTRUCT lpGameInfo) {
	CFugeWindow *pMain;

	pGameParams = lpGameInfo;

	// invoke your game here by creating a pGame for your main window
	// look at the InitInstance for your game for this

	ghParentWnd = hParentWnd;

	if ((pMain = new CFugeWindow) != nullptr) {

		pMain->ShowWindow(SW_SHOWNORMAL);

		pMain->UpdateWindow();

		pMain->SetActiveWindow();

		// these are used by Rules
		hDLLInst = (HINSTANCE)GetWindowWord(pMain->m_hWnd, GWW_HINSTANCE);
		hExeInst = (HINSTANCE)GetWindowWord(hParentWnd, GWW_HINSTANCE);

		if (pGameParams->bPlayingMetagame)
			pMain->PlayGame();
	}

	return pMain->m_hWnd;   // return the m_hWnd of your main game window
}

} // namespace Fuge
} // namespace HodjNPodj
} // namespace Bagel
