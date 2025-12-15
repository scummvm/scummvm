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

#include "bagel/hodjnpodj/metagame/demo/hodjpodj_demo.h"
#include "bagel/hodjnpodj/metagame/zoom/init.h"
#include "bagel/hodjnpodj/hnplibs/bitmaps.h"
#include "bagel/hodjnpodj/hodjnpodj.h"
#include "bagel/boflib/sound.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Demo {

#define IDC_SPLASH1				1001
#define IDC_SPLASH2				1002
#define IDC_PLAY_DEMO_MOVIE		1003
#define IDC_ZOOM				888

#define TIMER_SPLASH1			1
#define TIMER_SPLASH2			2
#define SPLASH_DURATION			5000

#define MOVIE_ID_INTRO			1
#define MOVIE_INTRO				"art/demovid.avi"
#define BITMAP_VIRGIN			"art/virgin.bmp"
#define BITMAP_BOFFO			"art/boffo.bmp"

#define LOGO_MIDI				"sound/maintitl.mid"

BEGIN_MESSAGE_MAP(CHodjPodjDemoWindow, CFrameWnd)
ON_WM_TIMER()
ON_WM_LBUTTONDOWN()
ON_WM_CLOSE()
ON_WM_KEYDOWN()
ON_WM_SYSCHAR()
ON_WM_PARENTNOTIFY()
END_MESSAGE_MAP()

CHodjPodjDemoWindow::CHodjPodjDemoWindow() {
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
	blackScreen();
	EndWaitCursor();

	pDC = GetDC();                                  // get a device context for our window

	pBitmap = FetchBitmap(pDC, &pGamePalette, "art/oscroll.bmp");
	delete pBitmap;
	ReleaseDC(pDC);

	// Show the Virgin splash screen
	PostMessage(WM_COMMAND, IDC_SPLASH1);
}

bool CHodjPodjDemoWindow::OnCommand(WPARAM wParam, LPARAM lParam) {
	_currentCommand = wParam;

	switch (wParam) {
	case IDC_SPLASH1:
		drawBitmap(BITMAP_VIRGIN);
		startBackgroundMidi();
		CWnd::SetTimer(TIMER_SPLASH1, SPLASH_DURATION, nullptr);
		break;

	case IDC_SPLASH2:
		drawBitmap(BITMAP_BOFFO);
		CWnd::SetTimer(TIMER_SPLASH2, SPLASH_DURATION, nullptr);
		break;

	case IDC_PLAY_DEMO_MOVIE:
		playMovie(MOVIE_ID_INTRO, MOVIE_INTRO, false);
		break;

	case MOVIE_OVER:
		// Switch to Zoom map
		ShowCursor(true);
		PostMessage(WM_COMMAND, IDC_ZOOM);
		break;

	case IDC_ZOOM:
		blackScreen();
		stopBackgroundMidi();

		Metagame::Zoom::RunZoomMap(m_hWnd, true);
		break;

	default:
		break;
	}

	return true;
}

void CHodjPodjDemoWindow::blackScreen() {
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

void CHodjPodjDemoWindow::drawBitmap(const char *filename) {
	CDC *dc = GetDC();
	CBitmap *bitmap = FetchBitmap(dc, &pGamePalette, filename);

	PaintBitmap(dc, nullptr, bitmap, 0, 0);

	delete bitmap;
	ReleaseDC(dc);
}

void CHodjPodjDemoWindow::playMovie(const int nMovieId, const char *pszMovie, bool bScroll) {
	POINT   ptMovie;

	blackScreen();

	pMovie.nMovieId = nMovieId;

	ptMovie.x = (GAME_WIDTH / 2) - (MOVIE_WIDTH / 2);
	ptMovie.y = (GAME_HEIGHT / 2) - (MOVIE_HEIGHT / 2);

	pMovie.BlowWindow((CWnd *)this, bScroll, pszMovie, ptMovie.x, ptMovie.y);
}


void CHodjPodjDemoWindow::startBackgroundMidi() {
	if (pBackgroundMidi == nullptr) {
		pBackgroundMidi = new CSound(this, LOGO_MIDI, SOUND_MIDI | SOUND_LOOP /* | SOUND_DONT_LOOP_TO_END */);
		(*pBackgroundMidi).play();
	}
}

void CHodjPodjDemoWindow::stopBackgroundMidi() {
	if (pBackgroundMidi != nullptr) {
		(*pBackgroundMidi).stop();
		delete pBackgroundMidi;
		pBackgroundMidi = nullptr;
	}
}

void CHodjPodjDemoWindow::OnTimer(uintptr nEventID) {
	switch (nEventID) {
	case TIMER_SPLASH1:
		KillTimer(TIMER_SPLASH1);
		PostMessage(WM_COMMAND, IDC_SPLASH2);
		break;
	case TIMER_SPLASH2:
		KillTimer(TIMER_SPLASH2);
		PostMessage(WM_COMMAND, IDC_PLAY_DEMO_MOVIE);
		break;
	default:
		break;
	}
}

void CHodjPodjDemoWindow::OnLButtonDown(uint nFlags, CPoint point) {
	skipSplash();
}


void CHodjPodjDemoWindow::OnKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	skipSplash();
}

void CHodjPodjDemoWindow::OnSysChar(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	skipSplash();
}

void CHodjPodjDemoWindow::skipSplash() {
	switch (_currentCommand) {
	case IDC_SPLASH1:
		OnTimer(TIMER_SPLASH1);
		break;
	case IDC_SPLASH2:
		OnTimer(TIMER_SPLASH2);
		break;
	default:
		break;
	}
}

void CHodjPodjDemoWindow::OnClose() {
	stopBackgroundMidi();

	KillTimer(TIMER_SPLASH1);
	KillTimer(TIMER_SPLASH2);

	CFrameWnd::OnClose();
}

void CHodjPodjDemoWindow::OnParentNotify(unsigned int msg, LPARAM lParam) {
	LPARAM nGameReturn;

	// Ignore messages during app shutdown
	if (AfxGetApp()->shouldQuit())
		return;

	switch (msg) {
	case WM_DESTROY:
		nGameReturn = lParam;

		if (_minigame != -1) {
			// Finished a minigame
			const auto &game = CMgStatic::cGameTable[_minigame];
			_minigame = -1;

			AfxGetApp()->removeResources(game._dllName);
			PostMessage(WM_COMMAND, IDC_ZOOM);

		} else if (nGameReturn < 0) {
			// Restart intro video
			PostMessage(WM_COMMAND, IDC_PLAY_DEMO_MOVIE);

		} else {
			loadNewDLL(nGameReturn);
		}
		break;

	default:
		break;
	}

	CWnd::OnParentNotify(msg, lParam);
}

void CHodjPodjDemoWindow::loadNewDLL(LPARAM lParam) {
	uint nWhichDLL;
	CWinApp *pMyApp;

	pMyApp = AfxGetApp();
	nWhichDLL = lParam - MG_GAME_BASE;
	assert(nWhichDLL < MG_GAME_COUNT);
	_minigame = nWhichDLL;

	blackScreen();

	stopBackgroundMidi();

	const auto &game = CMgStatic::cGameTable[nWhichDLL];
	assert(game._initFn != nullptr);

	GAMESTRUCT *lpGameStruct = new GAMESTRUCT;
	lpGameStruct->lCrowns = 1000;
	lpGameStruct->lScore = 0;
	lpGameStruct->nSkillLevel = SKILLLEVEL_MEDIUM;
	lpGameStruct->bSoundEffectsEnabled = true;
	lpGameStruct->bMusicEnabled = true;
	lpGameStruct->bPlayingMetagame = false;
	lpGameStruct->bPlayingHodj = true;

	// Load the minigame's dll for resources, and launch it
	pMyApp->addResources(game._dllName);
	(*game._initFn)(m_hWnd, lpGameStruct);
}

} // namespace Demo
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
