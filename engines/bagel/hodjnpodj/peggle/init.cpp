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
#include "bagel/hodjnpodj/hnplibs/stdafx.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/hodjnpodj/hnplibs/gamedll.h"
#include "bagel/hodjnpodj/peggle/resource.h"
#include "bagel/hodjnpodj/peggle/init.h"
#include "bagel/hodjnpodj/peggle/game.h"

namespace Bagel {
namespace HodjNPodj {
namespace Peggle {

HINSTANCE   hDLLInst;
HINSTANCE   hExeInst;

extern CSprite      *pCursorSprite;
extern CSprite      *pShotGlass;
extern CSprite      *pTableSlot;
extern CSprite      *pInvalidSlot;
extern CPalette     *pGamePalette;
extern CBmpButton   *pScrollButton;

CMainWindow     *pcwndPeggle = nullptr;
LPGAMESTRUCT    pGameInfo;

/////////////////////////////////////////////////////////////////////////////
// Public C interface

/*****************************************************************
 *
 * RunPeggle
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

HWND FAR PASCAL RunPeggle(HWND hParentWnd, LPGAMESTRUCT lpGameInfo) {
	pGameInfo = lpGameInfo;

// if the pointer has garbage in it, the clean it out
	if (pcwndPeggle != nullptr) {
		pcwndPeggle = nullptr;
	}
// create a my window and show it
	pcwndPeggle = new CMainWindow(hParentWnd);
	pcwndPeggle->ShowWindow(SW_SHOWNORMAL);
	pcwndPeggle->UpdateWindow();
	pcwndPeggle->SetActiveWindow();
// return the handle to this window
	hDLLInst = (HINSTANCE)GetWindowWord(pcwndPeggle->m_hWnd, GWW_HINSTANCE);
	hExeInst = (HINSTANCE)GetWindowWord(hParentWnd, GWW_HINSTANCE);
	return pcwndPeggle->m_hWnd;
}

} // namespace Peggle
} // namespace HodjNPodj
} // namespace Bagel
