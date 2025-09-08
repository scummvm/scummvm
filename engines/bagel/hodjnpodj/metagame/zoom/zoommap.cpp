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

#include "bagel/hodjnpodj/metagame/zoom/zoommap.h"
#include "bagel/hodjnpodj/metagame/bgen/mgstat.h"
#include "bagel/boflib/misc.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Zoom {

#define SPLASHSPEC  ".\\ART\\ZOOMMAP.BMP"

#define IDC_LEAVE   999

extern HCURSOR          hGameCursor;

CPalette    *pGamePalette = nullptr;        // Palette to be used throughout the game

int     nReturnValue = -1;       // the values to return to the main EXE to tell it what

static const RECT MINIGAME_RECTS[21] = {
	{  24, 243,  63, 263 },
	{ 495, 373, 529, 406 },
	{ 412, 185, 432, 201 },
	{ 526, 310, 597, 342 },
	{ 568, 376, 590, 417 },
	{ 201, 329, 254, 357 },
	{ 375, 136, 405, 153 },
	{  77, 253, 114, 302 },
	{ 314, 181, 337, 203 },
	{ 450,  10, 479,  34 },
	{ 482, 300, 505, 329 },
	{ 171,  75, 203,  94 },
	{ 255,  50, 271,  72 },
	{ 126, 254, 191, 278 },
	{ 467, 334, 519, 360 },
	{ 510, 189, 555, 226 },
	{ 292, 315, 341, 335 },
	{ 278,  70, 296,  88 },
	{ 349, 301, 382, 338 },
	{ 402, 203, 433, 250 }
};

static const int16 GAME_VALUES[21] = {
	// set the game values to return
	MG_GAME_ARCHEROIDS,
	MG_GAME_ARTPARTS,
	MG_GAME_BARBERSHOP,
	MG_GAME_BATTLEFISH,
	MG_GAME_BEACON,
	MG_GAME_CRYPTOGRAMS,
	MG_GAME_DAMFURRY,
	MG_GAME_FUGE,
	MG_GAME_GARFUNKEL,
	MG_GAME_LIFE,
	MG_GAME_MANKALA,
	MG_GAME_MAZEODOOM,
	MG_GAME_NOVACANCY,
	MG_GAME_PACRAT,
	MG_GAME_PEGGLEBOZ,
	MG_GAME_RIDDLES,
	MG_GAME_THGESNGGME,
	MG_GAME_VIDEOPOKER,
	MG_GAME_WORDSEARCH,
	-1
};

static const char *MINIGAME_DESC[21] = {       // set the display names for when the cursor passes over a game rect
	"Click Here To Play Archeroids",
	"Click Here To Play Art Parts",
	"Click Here To Play Barbershop Quintet",
	"Click Here To Play Battlefish",
	"Click Here To Play Beacon",
	"Click Here To Play Cryptograms",
	"Click Here To Play Dam Furry Animals",
	"Click Here To Play Fuge",
	"Click Here To Play Garfunkel",
	"Click Here To Play Life",
	"Click Here To Play Mankala",
	"Click Here To Play Maze O' Doom",
	"Click Here To Play No Vacancy",
	"Click Here To Play Pack-Rat",
	"Click Here To Play Peggleboz",
	"Click Here To Play Riddles",
	"Click Here To Play TH GESNG GAM",
	"Click Here To Play Poker",
	"Click Here To Play Word Search",
	"Click Here To Go To Main Menu"
};

static const RECT DEMO_RECTS[30] = {
	{  24,    243,     63,    263 },
	{ 495,    373,    529,    406 },
	{ 412,    185,    432,    201 },
	{ 526,    310,    597,    342 },
	{ 568,    376,    590,    417 },
	{ 201,    329,    254,    357 },
	{ 375,    136,    405,    153 },
	{  77,    253,    114,    302 },
	{ 314,    181,    337,    203 },
	{ 450,      6,    479,     34 },
	{ 482,    300,    505,    329 },
	{ 171,     75,    203,     94 },
	{ 255,     50,    271,     72 },
	{ 126,    254,    191,    278 },
	{ 467,    334,    519,    360 },
	{ 510,    189,    555,    226 },
	{ 292,    315,    341,    335 },
	{ 278,     70,    296,     88 },
	{ 349,    301,    382,    338 },
	{ 402,    203,    433,    250 },
	{  33,     17,     87,     56 },
	{  29,    403,     79,    441 },
	{ 244,    385,    303,    430 },
	{ 425,    412,    455,    432 },
	{ 581,      7,    612,     40 },
	{ 336,     56,    385,     91 },
	{ 142,    293,    198,    337 },
	{ 315,    346,    347,    379 },
	{ 368,    195,    392,    222 },
	{ 572,     85,    611,    121 },
};

static const char *DEMO_DESC[30] = {
	"Archeroids Would Be Here",
	"Click Here To Try Art Parts",
	"Barbershop Quintet Would Be Here",
	"Battlefish Would Be Here",
	"Beacon Would Be Here",
	"Click Here To Try Cryptograms",
	"Dam Furry Animals Would Be Here",
	"Fuge Would Be Here",
	"Garfunkel Would Be Here",
	"Life Would Be Here",
	"Mankala Would Be Here",
	"Click Here To Try Maze O' Doom",
	"No Vacancy Would Be Here",
	"Pack-Rat Would Be Here",
	"Click Here To Try Peggleboz",
	"Click Here To Try Riddles",
	"TH GESNG GAM Would Be Here",
	"Poker Would Be Here",
	"Word Search Would Be Here",
	"Click Here To See the Slide Show",
	"Tell me more about these mini-games.",
	"How does this board-game work?",
	"What's the story of Hodj 'n' Podj?",
	"Would my children like this game?",
	"How long does it take to play Hodj 'n' Podj?",
	"Who wrote Hodj 'n' Podj?",
	"Is this game gonna make me laugh, or what?",
	"Will Hodj 'n' Podj run on my computer?",
	"What have reviewers said about it?",
	"I'm convinced!  Where can I buy Hodj 'n Podj"
};

int             nLastRect;      // the last gaem rect passed over
CText           *pText = nullptr;   // the game name display

static  bool    bActiveWindow = false;          // whether our window is activesho
CBitmap         *pSplashScreen = nullptr;

CColorButton    *pReturnButton = nullptr;

BEGIN_MESSAGE_MAP(CMainZoomWindow, CFrameWnd)
	//{{AFX_MSG_MAP( CMainZoomWindow )
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	ON_WM_KEYDOWN()
	ON_WM_ACTIVATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/*****************************************************************
 *
 * CMainZoomWindow
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * Create the window with the appropriate style, size, menu, etc.;
 * it will be later revealed by CTheApp::InitInstance().  Then
 * create our splash screen object by opening and loading its DIB.
 *
 * FORMAL PARAMETERS:
 *
 *  lUserAmount = initial amount of money that user starts with
 *                              defaults to zero
 *  nRounds         = the number of rounds to play, if 0 then not playing rounds
 *                          = defaults to zero
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  n/a
 *
 * RETURN VALUE:
 *
 *  n/a
 *
 ****************************************************************/

CMainZoomWindow::CMainZoomWindow(HWND hCallingWnd, bool bShowExit) :
		rText(0, 428, 640, 450) {
	CDC         *pDC = nullptr;                     // device context for the screen
	CString     WndClass;
	bool        bSuccess = false;

	BeginWaitCursor();
// Define a special window class which traps double-clicks, is byte aligned
// to maximize BITBLT performance, and creates "owned" DCs rather than sharing
// the five system defined DCs which are not guaranteed to be available;
// this adds a bit to our app size but avoids hangs/freezes/lockups.

	WndClass = AfxRegisterWndClass(CS_DBLCLKS | CS_BYTEALIGNWINDOW | CS_OWNDC,
	    hGameCursor, nullptr, nullptr);

	m_hCallAppWnd = hCallingWnd;
	m_bShowExit = bShowExit;

	// get a device context for our window
	pDC = GetDC();

// set window coordinates to center game on screeen
	MainRect.left = (pDC->GetDeviceCaps(HORZRES) - GAME_WIDTH) >> 1;
	MainRect.top = (pDC->GetDeviceCaps(VERTRES) - GAME_HEIGHT) >> 1;
	MainRect.right = MainRect.left + GAME_WIDTH;    // determine where to place the game window
	MainRect.bottom = MainRect.top + GAME_HEIGHT;   // ... so it is centered on the screen

	rLeaveRect.SetRect((GAME_WIDTH / 2) - 50, 450, (GAME_WIDTH / 2) + 50, 470);

	pText = new CText(pDC, pGamePalette, &rText, JUSTIFY_CENTER);    // Set up the Text Object to show the game names

	ReleaseDC(pDC);                                 // release our window context
	pDC = nullptr;

// Create the window as a POPUP so that no boarders, title, or menu are present;
// this is because the game's background art will fill the entire 640x40 area.
	Create(WndClass, "Boffo Games - ZOOM MAP", WS_POPUP, MainRect, nullptr, 0);

	pDC = GetDC();
	pSplashScreen = FetchBitmap(pDC, &pGamePalette, SPLASHSPEC);
	ReleaseDC(pDC);

	if (bShowExit) {
		pReturnButton = new CColorButton();
		ASSERT(pReturnButton);
		bSuccess = pReturnButton->Create("Main Menu", BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, rLeaveRect, this, IDC_LEAVE);
		ASSERT(bSuccess);
		pReturnButton->SetPalette(pGamePalette);
	}

	nLastRect = -1; // contains the number of the last rect that the mouse ran thru
	_isDemo = g_engine->isDemo();

	EndWaitCursor();
}

/*****************************************************************
 *
 * OnPaint
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * Repaint the screen whenever needed; e.g. when uncovered by an
 * overlapping window, when maximized from an icon, and when it the
 * window is initially created.  Ensures that the entire client area
 * of the main screen window is repainted, not just the portion in the
 * update region; see SplashScreen();
 *
 * This routine is called whenever Windows sends a WM_PAINT message.
 * Note that creating a CPaintDC automatically does a BeginPaint and
 * an EndPaint call is done when it is destroyed at the end of this
 * function.  CPaintDC's constructor needs the window (this).
 *
 * FORMAL PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  n/a
 *
 * RETURN VALUE:
 *
 *  n/a
 *
 ****************************************************************/
void CMainZoomWindow::OnPaint() {
	PAINTSTRUCT lpPaint;

	BeginPaint(&lpPaint);                           // bracket start of window update
	SplashScreen();                                 // repaint our window's content
	EndPaint(&lpPaint);                             // bracket end of window update
}

/*****************************************************************
 *
 * SplashScreen
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * Repaint the background artwork, together with all sprites in the
 * sprite chain queue.  The entire window is redrawn, rather than just
 * the updated area, to ensure that the sprites end up with the correct
 * background bitmaps saved for their image areas.
 *
 * FORMAL PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  n/a
 *
 * RETURN VALUE:
 *
 *  n/a
 *
 ****************************************************************/
void CMainZoomWindow::SplashScreen() {
	CDC         *pDC = GetDC();                                                                          // get a device context for the window
	CPalette    *pOldPalette = pDC->SelectPalette(pGamePalette, false);      // load game palette;

	ASSERT(pDC);

	pDC->RealizePalette();                      // realize game palette

	PaintBitmap(pDC, pGamePalette, pSplashScreen);

	pDC->SelectPalette(pOldPalette, false);     // replace old palette
	ReleaseDC(pDC);                             // release the window's context

}

/*****************************************************************
 *
 * OnCommand
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * Process the QUIT and OKAY buttons when they are clicked.
 *
 * This function is called when a WM_COMMAND message is issued,
 * typically in order to process control related activities.
 *
 * FORMAL PARAMETERS:
 *
 *  wParam      identifier for the button to be processed
 *  lParam      type of message to be processed
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  n/a
 *
 * RETURN VALUE:
 *
 *  n/a
 *
 ****************************************************************/

// OnCommand
//

void CALLBACK lpfnOptionCallback(CWnd * pWnd) {
// do the mini options dialog
	return;
}

bool CMainZoomWindow::OnCommand(WPARAM wParam, LPARAM lParam) {
	if (HIWORD(lParam) == BN_CLICKED) {
		if (wParam == IDC_LEAVE) {
			nReturnValue = -1;
			PostMessage(WM_CLOSE);
		}
	}
	(*this).SetFocus();                         // Reset focus back to the main window
	return true;
}

void CMainZoomWindow::OnLButtonDown(unsigned int nFlags, CPoint point) {
	int x = rectIndexOf(point);

	if (x != -1) {
		// Check to see if player clicked on a game
		if (_isDemo || ((GAME_VALUES[x] == -1) && (m_bShowExit == false))) {
			CWnd::OnLButtonDown(nFlags, point);
			return;
		}
		nReturnValue = GAME_VALUES[x];	// if so then dispatch to game
		PostMessage(WM_CLOSE);
		return;
	}

	CWnd::OnLButtonDown(nFlags, point);
}

void CMainZoomWindow::OnMouseMove(unsigned int nFlags, CPoint point) {
	int x = rectIndexOf(point);

	// If cursor passes over a game rect
	if (x != -1) {
		if (!_isDemo && (GAME_VALUES[x] == -1) && (m_bShowExit == false)) {
			CWnd::OnMouseMove(nFlags, point);
			return;
		}

		if (x != nLastRect) {
			// then highlight it
			CDC *pDC = GetDC();
			CBrush brshCyanBrush(RGB(0, 255, 255));
			CBrush brshBlackBrush(RGB(0, 0, 0));
			const RECT &oldRect = (_isDemo ? DEMO_RECTS : MINIGAME_RECTS)[nLastRect];
			const RECT &newRect = (_isDemo ? DEMO_RECTS : MINIGAME_RECTS)[x];

			CRect rTemp1((newRect.left - 5 + 2), (newRect.top - 5 + 2),
				(newRect.right + 5 + 2), (newRect.bottom + 5 + 2));
			CRect rTemp2((newRect.left - 5), (newRect.top - 5),
				(newRect.right + 5), (newRect.bottom + 5));

			if (nLastRect != -1) {
				CRect rTemp(oldRect.left - 10, oldRect.top - 10,
					oldRect.right + 10, oldRect.bottom + 10);
				RedrawWindow(&rTemp);
			}

			pDC->FrameRect(&rTemp1, &brshBlackBrush);
			pDC->FrameRect(&rTemp2, &brshCyanBrush);

			const char *desc = (_isDemo ? DEMO_DESC : MINIGAME_DESC)[x];
			pText->DisplayShadowedString(pDC, desc, 16, FW_BOLD, RGB(0, 255, 255));
			nLastRect = x;
			ReleaseDC(pDC);
		}

		return;
	}

	if (nLastRect != -1) {
		const RECT &oldRect = (_isDemo ? DEMO_RECTS : MINIGAME_RECTS)[nLastRect];
		CRect rTemp(oldRect.left - 10, MAX<int>(oldRect.top - 10, 0),
			oldRect.right + 10, oldRect.bottom + 10);

		RedrawWindow(&rTemp);
		RedrawWindow(&rText);
		nLastRect = -1;
	}

	CWnd::OnMouseMove(nFlags, point);
}

void CMainZoomWindow::OnLButtonUp(unsigned int nFlags, CPoint point) {
	CWnd::OnLButtonUp(nFlags, point);
}

void CMainZoomWindow::OnKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {

	switch (nChar) {
	case VK_ESCAPE:
		if (m_bShowExit == true) {
			nReturnValue = -1;
			PostMessage(WM_CLOSE);
		}
		break;
	default:
		CFrameWnd::OnKeyDown(nChar, nRepCnt, nFlags);
		break;
	}
	return;
}

void CMainZoomWindow::OnTimer(uintptr nWhichTimer) {
	return;
}

/*****************************************************************
 *
 * OnXXXXXX
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      These functions are called when ever the corresponding WM_
 *      event message is generated for the mouse.
 *
 *      (Add game-specific processing)
 *
 * FORMAL PARAMETERS:
 *
 *      [Show arguments]
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 * RETURN VALUE:
 *
 *      [Discuss return value]
 *
 ****************************************************************/

bool CMainZoomWindow::OnEraseBkgnd(CDC *pDC) {
// eat this
	return true;
}


void CMainZoomWindow::OnActivate(unsigned int nState, CWnd  *pWndOther, bool bMinimized) {
	bool    bUpdateNeeded;

	switch (nState) {
	case WA_INACTIVE:
		bActiveWindow = false;
		break;
	case WA_ACTIVE:
	case WA_CLICKACTIVE:
		bActiveWindow = true;
		bUpdateNeeded = GetUpdateRect(nullptr, false);
		if (bUpdateNeeded)
			InvalidateRect(nullptr, false);
	}
}


/*****************************************************************
 *
 * OnClose
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  This function is called when a Close event is generated.  For
 *  this sample application we need only kill our event timer;
 *  The ExitInstance will handle releasing resources.
 *
 * FORMAL PARAMETERS:
 *
 *      n/a
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

void CMainZoomWindow::OnClose() {
	CDC *pDC = GetDC();
	CRect       rctFillRect(0, 0, 640, 480);
	CBrush  Brush(RGB(0, 0, 0));

	pDC->FillRect(&rctFillRect, &Brush);
	ReleaseDC(pDC);
	ReleaseResources();
	CFrameWnd ::OnClose();
}

/*****************************************************************
 *
 * OnDestroy
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  This function is called when after the window has been destroyed.
 *  For poker, we post a message bak to the calling app to tell it
 * that the user has quit the game, and therefore the app can unload
 * this DLLL
 *
 * FORMAL PARAMETERS:
 *
 *      n/a
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
void CMainZoomWindow::OnDestroy() {
//  send a message to the calling app to tell it the user has quit the game
	MFC::PostMessage(m_hCallAppWnd, WM_PARENTNOTIFY, WM_DESTROY, (LPARAM)nReturnValue);
	CFrameWnd::OnDestroy();
}


/*****************************************************************
 *
 * ReleaseResources
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  Release all resources that were created and retained during the
 *  course of the game.  This includes sprites in the sprite chain,
 *  the game color palette, and button controls.
 *
 * FORMAL PARAMETERS:
 *
 *      n/a
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

void CMainZoomWindow::ReleaseResources() {
	delete pText;
	pText = nullptr;

	delete pSplashScreen;
	pSplashScreen = nullptr;

	if (pGamePalette != nullptr) {
		pGamePalette->DeleteObject();         // release the game color palette
		delete pGamePalette;
		pGamePalette = nullptr;
	}

	delete pReturnButton;                                       // release the buttons we used
	pReturnButton = nullptr;
}


/*****************************************************************
 *
 * FlushInputEvents
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  Remove all keyboard and mouse related events from the message
 *  so that they will not be sent to us for processing; i.e. this
 *  flushes keyboard type ahead and extra mouse clicks and movement.
 *
 * FORMAL PARAMETERS:
 *
 *      n/a
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

void CMainZoomWindow::FlushInputEvents() {
	MSG msg;

	while (true) {                                      // find and remove all keyboard events
		if (!PeekMessage(&msg, nullptr, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE))
			break;
	}

	while (true) {                                      // find and remove all mouse events
		if (!PeekMessage(&msg, nullptr, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE))
			break;
	}
}

int CMainZoomWindow::rectIndexOf(const CPoint &pt) {
	const RECT *r = _isDemo ? DEMO_RECTS : MINIGAME_RECTS;
	const int count = _isDemo ? 30 : 21;

	for (int i = 0; i < count; ++i, ++r) {
		if (PtInRect(r, pt))
			return i;
	}

	return -1;
}

} // namespace Zoom
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
