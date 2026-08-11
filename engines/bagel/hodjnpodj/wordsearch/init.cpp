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
#include "bagel/hodjnpodj/wordsearch/resource.h"
#include "bagel/hodjnpodj/wordsearch/wordsearch.h"

namespace Bagel {
namespace HodjNPodj {
namespace WordSearch {

HINSTANCE   hDLLInst;
HINSTANCE   hExeInst;

CMainWSWindow   *pMainGameWnd = nullptr;   // pointer to the poker's main window
CPalette        *pTestPalette = nullptr;
HCURSOR         hGameCursor;
/////////////////////////////////////////////////////////////////////////////
// Public C interface

/*****************************************************************
 *
 * RunPoker
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

HWND FAR PASCAL RunWordSearch(HWND hParentWnd, LPGAMESTRUCT lpGameInfo) {

// if the pointer has garbage in it, the clean it out
	if (pMainGameWnd != nullptr) {
		pMainGameWnd = nullptr;
	}

// create a my poker window and show it
	pMainGameWnd = new CMainWSWindow(hParentWnd, lpGameInfo);
	pMainGameWnd->ShowWindow(SW_SHOWNORMAL);
	pMainGameWnd->UpdateWindow();
	pMainGameWnd->SetActiveWindow();
// return the handle to this window
	hDLLInst = (HINSTANCE)GetWindowWord(pMainGameWnd->m_hWnd, GWW_HINSTANCE);
	hExeInst = (HINSTANCE)GetWindowWord(hParentWnd, GWW_HINSTANCE);
	if (lpGameInfo->bPlayingMetagame == false)
		MFC::PostMessage(pMainGameWnd->m_hWnd, WM_COMMAND, IDC_OPTION, BN_CLICKED);
	return pMainGameWnd->m_hWnd;
}

} // namespace WordSearch
} // namespace HodjNPodj
} // namespace Bagel
