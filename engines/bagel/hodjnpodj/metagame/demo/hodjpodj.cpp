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

#include "bagel/hodjnpodj/metagame/demo/hodjpodj.h"
#include "bagel/hodjnpodj/metagame/zoom/init.h"
#include "bagel/hodjnpodj/hodjnpodj.h"
#include "bagel/boflib/sound.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Demo {

#define IDC_PLAY_DEMO_MOVIE		9999
#define IDC_ZOOM				888
#define MOVIE_ID_INTRO			1
#define MOVIE_INTRO				"art/demovid.avi"

BEGIN_MESSAGE_MAP(CHodjPodjWindow, CFrameWnd)
END_MESSAGE_MAP()

CHodjPodjWindow::CHodjPodjWindow() {
	CString WndClass;
	CRect MainRect;
	bool bTestDibDoc;
	CDC *pDC;
	CBitmap *pBitmap;

	WndClass = AfxRegisterWndClass(CS_DBLCLKS | CS_BYTEALIGNWINDOW | CS_OWNDC, nullptr, nullptr, nullptr);

	MainRect.left = 0;
	MainRect.top = 0;
	MainRect.right = GAME_WIDTH;
	MainRect.bottom = GAME_HEIGHT;
	bTestDibDoc = Create(WndClass, "Boffo Games - Hodj 'n' Podj Demo", WS_POPUP, MainRect, nullptr, 0);
	ASSERT(bTestDibDoc);

	BeginWaitCursor();
	ShowWindow(SW_SHOWNORMAL);
	BlackScreen();
	EndWaitCursor();

	pDC = GetDC();                                  // get a device context for our window

	pBitmap = FetchBitmap(pDC, &pGamePalette, "art/oscroll.bmp");
	delete pBitmap;
	ReleaseDC(pDC);

	// Play the demo intro movie
	PostMessage(WM_COMMAND, IDC_PLAY_DEMO_MOVIE);
}

bool CHodjPodjWindow::OnCommand(WPARAM wParam, LPARAM lParam) {
	switch (wParam) {
	case IDC_PLAY_DEMO_MOVIE:
		PlayMovie(MOVIE_ID_INTRO, MOVIE_INTRO, false);
		break;

	case MOVIE_OVER:
		// Switch to Zoom map
		ShowCursor(true);
		BlackScreen();
		PostMessage(WM_COMMAND, IDC_ZOOM);
		break;

	case IDC_ZOOM:
		BlackScreen();

		Metagame::Zoom::RunZoomMap(m_hWnd, true);
		break;

	default:
		break;
	}

	return true;
}

void CHodjPodjWindow::BlackScreen() {
	CDC *pDC;
	CBrush Brush(RGB(0, 0, 0));
	CPalette *pPalOld = nullptr;
	CRect MainRect;

	ValidateRect(nullptr);
	pDC = GetDC();

	MainRect.left = MainRect.top = 0;
	MainRect.right = GAME_WIDTH;
	MainRect.bottom = GAME_HEIGHT;
	pDC->FillRect(&MainRect, &Brush);

	if (pGamePalette != nullptr) {                                  // map in color palette to be used
		pPalOld = (*pDC).SelectPalette(pGamePalette, false);
		(*pDC).RealizePalette();
	}

	pDC->FillRect(&MainRect, &Brush);

	if (pPalOld != nullptr)                                 // relinquish the resources we built
		(*pDC).SelectPalette(pPalOld, false);

	ReleaseDC(pDC);
}


void CHodjPodjWindow::PlayMovie(const int nMovieId, const char *pszMovie, bool bScroll) {
	POINT   ptMovie;

	BlackScreen();

	pMovie.nMovieId = nMovieId;

	ptMovie.x = (GAME_WIDTH / 2) - (MOVIE_WIDTH / 2);
	ptMovie.y = (GAME_HEIGHT / 2) - (MOVIE_HEIGHT / 2);

	pMovie.BlowWindow((CWnd *)this, bScroll, pszMovie, ptMovie.x, ptMovie.y);
}

} // namespace Demo
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
