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
#include "bagel/hodjnpodj/metagame/zoom/resource.h"
#include "bagel/hodjnpodj/metagame/zoom/zoommap.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Zoom {

//#ifndef _DEBUG
//#error This source file must be compiled with _DEBUG defined
//#endif

HINSTANCE   hDLLInst;
HINSTANCE   hExeInst;

CMainZoomWindow *pMainGameWnd = nullptr;    // pointer to the poker's main window
CPalette        *pTestPalette = nullptr;
HCURSOR         hGameCursor;

/////////////////////////////////////////////////////////////////////////////
// Public C interface

HWND RunZoomMap(HWND hParentWnd, bool bShowExit) {

// if the pointer has garbage in it, the clean it out
	if (pMainGameWnd != nullptr) {
		pMainGameWnd = nullptr;
	}
// create a my poker window and show it
	pMainGameWnd = new CMainZoomWindow(hParentWnd, bShowExit);
	pMainGameWnd->ShowWindow(SW_SHOWNORMAL);
	pMainGameWnd->SetActiveWindow();
	pMainGameWnd->UpdateWindow();

// return the handle to this window
	hDLLInst = (HINSTANCE)GetWindowWord(pMainGameWnd->m_hWnd, GWW_HINSTANCE);
	hExeInst = (HINSTANCE)GetWindowWord(hParentWnd, GWW_HINSTANCE);

	return pMainGameWnd->m_hWnd;
}

} // namespace Zoom
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
