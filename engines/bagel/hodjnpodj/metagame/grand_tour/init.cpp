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
#include "bagel/hodjnpodj/metagame/grand_tour/resource.h"
#include "bagel/hodjnpodj/metagame/grand_tour/grand_tour.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace GrandTour {

HINSTANCE   hDLLInst;
HINSTANCE   hExeInst;

CMainGTWindow   *pMainGameWnd = nullptr;    // pointer to the poker's main window
CPalette        *pTestPalette = nullptr;
HCURSOR         hGameCursor;

/////////////////////////////////////////////////////////////////////////////
// Public C interface

HWND RunGrandTour(HWND hParentWnd, LPGRANDTRSTRUCT pgtGrandTourStruct) {
	// if the pointer has garbage in it, the clean it out
	if (pMainGameWnd != nullptr) {
		pMainGameWnd = nullptr;
	}
// create a my poker window and show it
	pMainGameWnd = new CMainGTWindow(hParentWnd, pgtGrandTourStruct);
	pMainGameWnd->ShowWindow(SW_SHOWNORMAL);
	pMainGameWnd->UpdateWindow();
	pMainGameWnd->SetActiveWindow();

// return the handle to this window
	hDLLInst = (HINSTANCE)GetWindowWord(pMainGameWnd->m_hWnd, GWW_HINSTANCE);
	hExeInst = (HINSTANCE)GetWindowWord(hParentWnd, GWW_HINSTANCE);

	return pMainGameWnd->m_hWnd;
}

} // namespace GrandTour
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
