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

#include "bagel/hodjnpodj/hnplibs/stdafx.h"
#include "bagel/hodjnpodj/hnplibs/cmessbox.h"
#include "bagel/boflib/misc.h"
#include "bagel/hodjnpodj/peggle/resource.h"
#include "bagel/hodjnpodj/hnplibs/gamedll.h"
#include "bagel/hodjnpodj/hnplibs/dibdoc.h"
#include "bagel/hodjnpodj/hnplibs/sprite.h"
#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/peggle/game.h"
#include "bagel/hodjnpodj/peggle/game2.h"
#include "bagel/hodjnpodj/peggle/options.h"
#include "bagel/hodjnpodj/hnplibs/rules.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/boflib/sound.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {
namespace Peggle {

#define SHOW_CURSOR true


// Board definitions:

static const int8 Board_Triangle[GRID_SIZE][GRID_SIZE] = {
	{PEGGED, PEGGED, PEGGED, PEGGED, PEGGED, NO_HOLE, NO_HOLE},
	{PEGGED, EMPTY, PEGGED, PEGGED, NO_HOLE, NO_HOLE, NO_HOLE},
	{PEGGED, PEGGED, PEGGED, NO_HOLE, NO_HOLE, NO_HOLE, NO_HOLE},
	{PEGGED, PEGGED, NO_HOLE, NO_HOLE, NO_HOLE, NO_HOLE, NO_HOLE},
	{PEGGED, NO_HOLE, NO_HOLE, NO_HOLE, NO_HOLE, NO_HOLE, NO_HOLE},
	{NO_HOLE, NO_HOLE, NO_HOLE, NO_HOLE, NO_HOLE, NO_HOLE, NO_HOLE},
	{NO_HOLE, NO_HOLE, NO_HOLE, NO_HOLE, NO_HOLE, NO_HOLE, NO_HOLE},
};
static const int8 Board_TrianglePlus[GRID_SIZE][GRID_SIZE] = {
	{NO_HOLE, PEGGED, NO_HOLE, NO_HOLE, NO_HOLE, NO_HOLE, PEGGED},
	{PEGGED, PEGGED, PEGGED, PEGGED, PEGGED, PEGGED, PEGGED},
	{NO_HOLE, PEGGED, EMPTY, PEGGED, PEGGED, NO_HOLE, NO_HOLE},
	{NO_HOLE, PEGGED, PEGGED, PEGGED, NO_HOLE, NO_HOLE, NO_HOLE},
	{NO_HOLE, PEGGED, PEGGED, NO_HOLE, NO_HOLE, NO_HOLE, NO_HOLE},
	{NO_HOLE, PEGGED, NO_HOLE, NO_HOLE, NO_HOLE, NO_HOLE, NO_HOLE},
	{PEGGED, PEGGED, NO_HOLE, NO_HOLE, NO_HOLE, NO_HOLE, NO_HOLE},
};
static const int8 Board_CrossPlus[GRID_SIZE][GRID_SIZE] = {
	{NO_HOLE, NO_HOLE, PEGGED, PEGGED, PEGGED, NO_HOLE, NO_HOLE},
	{NO_HOLE, PEGGED, PEGGED, PEGGED, PEGGED, PEGGED, NO_HOLE},
	{PEGGED, PEGGED, PEGGED, PEGGED, PEGGED, PEGGED, PEGGED},
	{PEGGED, PEGGED, PEGGED, EMPTY, PEGGED, PEGGED, PEGGED},
	{PEGGED, PEGGED, PEGGED, PEGGED, PEGGED, PEGGED, PEGGED},
	{NO_HOLE, PEGGED, PEGGED, PEGGED, PEGGED, PEGGED, NO_HOLE},
	{NO_HOLE, NO_HOLE, PEGGED, PEGGED, PEGGED, NO_HOLE, NO_HOLE},
};
static const int8 Board_Cross[GRID_SIZE][GRID_SIZE] = {
	{NO_HOLE, NO_HOLE, PEGGED, PEGGED, PEGGED, NO_HOLE, NO_HOLE},
	{NO_HOLE, NO_HOLE, PEGGED, PEGGED, PEGGED, NO_HOLE, NO_HOLE},
	{PEGGED, PEGGED, PEGGED, PEGGED, PEGGED, PEGGED, PEGGED},
	{PEGGED, PEGGED, PEGGED, EMPTY, PEGGED, PEGGED, PEGGED},
	{PEGGED, PEGGED, PEGGED, PEGGED, PEGGED, PEGGED, PEGGED},
	{NO_HOLE, NO_HOLE, PEGGED, PEGGED, PEGGED, NO_HOLE, NO_HOLE},
	{NO_HOLE, NO_HOLE, PEGGED, PEGGED, PEGGED, NO_HOLE, NO_HOLE},
};


extern  LPGAMESTRUCT    pGameInfo;

void setup_cursor();
void set_wait_cursor();
void reset_wait_cursor();

CBmpButton  *pScrollButton = nullptr;

int             sprite_count = 0,
                counter = 0 ;

CPalette        *pGamePalette = nullptr ;
static CSound   *pGameSound = nullptr;                 // Game theme song

// Board Selection stuff
bool bRandomBoard = false;

int8 BoardSelected = CROSS;

//static char MaxPegs[2][4] = {
//	{CROSS, CROSS_PLUS, TRIANGLE,   TRIANGLE_PLUS},
//	{32,    36,         14,         20},
//} ;
char fState [GRID_SIZE][GRID_SIZE];
const char *BoardSpec[BOARD_COUNT] = {
	".\\ART\\CROSS.BMP",
	".\\ART\\CROSSX.BMP",
	".\\ART\\TRIANGLE.BMP",
	".\\ART\\TRIANGLX.BMP"
};

// Move co-ordinates. 1 peg can be removed per move.
static POINT Moves[70];

CSprite *pCursorSprite = nullptr;

CSprite *pShotGlass = nullptr;
CSprite *pTableSlot = nullptr;
CSprite *pInvalidSlot = nullptr;

static int nBoard_DX = TRI_BOARD_DX,
           nBoard_DY = TRI_BOARD_DY;

static bool bIgnoreScroll = false;
static bool bPegMoving = false;

/////////////////////////////////////////////////////////////////////////////

// CMainWindow constructor:
// Create the window with the appropriate style, size, menu, etc.;
// it will be later revealed by CTheApp::InitInstance().  Then
// create our splash screen object by opening and loading its DIB.
//
CMainWindow::CMainWindow(HWND hCallingApp) {
	CString WndClass;
	CDC     *pDC;
	CPalette    *pPalOld;
	CDibDoc *pDibDoc;
	CRect   MainRect;
	bool    bSuccess;


	BeginWaitCursor();
	initStatics();

	m_hCallAppWnd = hCallingApp;

// select a type!
// Define a special window class which traps double-clicks, is byte aligned
// to maximize BITBLT performance, and creates "owned" DCs rather than sharing
// the five system defined DCs which are not guaranteed to be available;
// this adds a bit to our app size but avoids hangs/freezes/lockups.
	WndClass = AfxRegisterWndClass(CS_DBLCLKS | CS_BYTEALIGNWINDOW | CS_OWNDC,
	                               nullptr, nullptr, nullptr);

// Center our window on the screen

	pDC = GetDC();
	MainRect.left = (pDC->GetDeviceCaps(HORZRES) - GAME_WIDTH) >> 1;
	MainRect.top = (pDC->GetDeviceCaps(VERTRES) - GAME_HEIGHT) >> 1;
	MainRect.right = MainRect.left + GAME_WIDTH;
	MainRect.bottom = MainRect.top + GAME_HEIGHT;
	ReleaseDC(pDC);

//MainRect.SetRect(10,10,GAME_WIDTH + 10,GAME_HEIGHT + 10);

// Create the window as a POPUP so no boarders, title, or menu are present;
// this is because the game's background art will fill the entire 640x40 area.
	Create(WndClass, "Boffo Games -- Peggleboz", WS_POPUP, MainRect, nullptr, 0);

	SplashScreen();

	setup_cursor();

// Acquire the shared palete for our game from the splash screen art
	pDibDoc = new CDibDoc();
	bSuccess = (*pDibDoc).OpenDocument(BoardSpec[BoardSelected - BOARD_BASE]);
	ASSERT(bSuccess);

	pGamePalette = (*pDibDoc).DetachPalette();
	delete pDibDoc;

	pDC = GetDC();
	pPalOld = (*pDC).SelectPalette(pGamePalette, false);

	pScrollButton = new CBmpButton;             // build a bitmapped OKAY button for resetting
	ASSERT(pScrollButton);                            // ... the sprites, again as an example
	ScrollRect.SetRect(SCROLL_BUTTON_X,
	                   SCROLL_BUTTON_Y,
	                   SCROLL_BUTTON_X + SCROLL_BUTTON_DX - 1,
	                   SCROLL_BUTTON_Y + SCROLL_BUTTON_DY - 1);
	bSuccess = (*pScrollButton).Create(nullptr, BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, ScrollRect, this, IDC_SCROLL);
	ASSERT(bSuccess);
	bSuccess = (*pScrollButton).LoadBitmaps("SCROLLUP", "SCROLLDOWN", "SCROLLFOCUS", "SCROLLDISABLED");
	ASSERT(bSuccess);

	pShotGlass = new CSprite;

	(*pShotGlass).SharePalette(pGamePalette);
	bSuccess = (*pShotGlass).LoadResourceSprite(pDC, "SHOTGLASS");
	ASSERT(bSuccess);
	(*pShotGlass).SetMasked(true);
	(*pShotGlass).SetMobile(true);
	(*pShotGlass).SetTypeCode(SPRITE_GLASS);

	pTableSlot = new CSprite;

	(*pTableSlot).SharePalette(pGamePalette);
	bSuccess = (*pTableSlot).LoadResourceSprite(pDC, "TABLESLOT");
	ASSERT(bSuccess);
	(*pTableSlot).SetMasked(true);
	(*pTableSlot).SetMobile(true);
	(*pTableSlot).SetTypeCode(SPRITE_HOLE);

	pInvalidSlot = new CSprite;

	(*pInvalidSlot).SharePalette(pGamePalette);
	bSuccess = (*pInvalidSlot).LoadResourceSprite(pDC, "INVALIDSLOT");
	ASSERT(bSuccess);
	(*pInvalidSlot).SetMasked(true);
	(*pInvalidSlot).SetMobile(true);
	(*pInvalidSlot).SetTypeCode(SPRITE_INVALID);

	pCursorSprite = new CSprite;

	(*pCursorSprite).SharePalette(pGamePalette);
	bSuccess = (*pCursorSprite).LoadResourceSprite(pDC, "SHOTGLASS");
	ASSERT(bSuccess);
	(*pCursorSprite).SetMasked(true);
	(*pCursorSprite).SetMobile(true);

	//srand((unsigned)time(nullptr));

	if ((*pGameInfo).bPlayingMetagame)
		bRandomBoard = true;
//		BoardSelected = (brand() % BOARD_COUNT) + BOARD_BASE;

	SetUpBoard(pDC);

	(*pDC).SelectPalette(pPalOld, false);
	ReleaseDC(pDC);

	if ((*pGameInfo).bMusicEnabled) {
		if ((pGameSound = new CSound(this, GAME_THEME,
		                            SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END))) {
			(*pGameSound).midiLoopPlaySegment(1000, 30000, 0, FMT_MILLISEC);
		} // end if pGameSound
	}

	if ((*pGameInfo).bPlayingMetagame == false) {
		PostMessage(WM_COMMAND, IDC_SCROLL, BN_CLICKED);
	}

	#ifndef SHOW_CURSOR
	::ShowCursor(false);
	#endif
	EndWaitCursor();
}

void CMainWindow::initStatics() {
	pScrollButton = nullptr;
	sprite_count = counter = 0;
	pGamePalette = nullptr;
	pGameSound = nullptr;

	bRandomBoard = false;
	BoardSelected = g_engine->isDemo() ? TRIANGLE_PLUS : CROSS;
	Common::fill(&fState[0][0], &fState[0][0] + GRID_SIZE * GRID_SIZE, 0);

	for (POINT &pt : Moves)
		pt.x = pt.y = 0;

	pCursorSprite = nullptr;
	pShotGlass = nullptr;
	pTableSlot = nullptr;
	pInvalidSlot = nullptr;

	nBoard_DX = TRI_BOARD_DX;
	nBoard_DY = TRI_BOARD_DY;

	bIgnoreScroll = false;
	bPegMoving = false;
}

void CMainWindow::OnActivateApp(bool bActive, HTASK hTask) {
	m_bProgramActive = bActive;
}


// OnPaint:
// This is called whenever Windows sends a WM_PAINT message.
// Note that creating a CPaintDC automatically does a BeginPaint and
// an EndPaint call is done when it is destroyed at the end of this
// function.  CPaintDC's constructor needs the window (this).
//
void CMainWindow::OnPaint() {
	PAINTSTRUCT lpPaint;

	InvalidateRect(nullptr, false);
	BeginPaint(&lpPaint);
	SplashScreen();
	EndPaint(&lpPaint);
}


// Paint the background art (splash screen) in the client area;
// called by both OnPaint and InitInstance.
void CMainWindow::SplashScreen() {
	CRect   rcDest;
	CRect   rcDIB;
	CDC     *pDC;
	CDibDoc myDoc;
	HDIB    hDIB;
	CSprite *pSprite;
	bool    bSuccess;

	bSuccess = myDoc.OpenDocument(BoardSpec[BoardSelected - BOARD_BASE]);
	ASSERT(bSuccess);
	hDIB = myDoc.GetHDIB();

	pDC = GetDC();
	if (pDC && hDIB) {
		GetClientRect(rcDest);

		int cxDIB = (int) DIBWidth(hDIB);
		int cyDIB = (int) DIBHeight(hDIB);

		rcDIB.top = rcDIB.left = 0;
		rcDIB.right = cxDIB;
		rcDIB.bottom = cyDIB;
		PaintDIB((*pDC).m_hDC, &rcDest, hDIB, &rcDIB, pGamePalette);
	}

	pSprite = CSprite::GetSpriteChain();
	while (pSprite) {
		(*pSprite).ClearBackground();
		bSuccess = (*pSprite).RefreshSprite(pDC);
		ASSERT(bSuccess);
		pSprite = (*pSprite).GetNextSprite();
	}

	ReleaseDC(pDC);
}


void SetUpBoard(CDC *pDC) {
	int     i, j;
	CSprite *pNewSprite;
	CPoint  cPoint;

	CSprite::EraseSprites(pDC);
	CSprite::FlushSpriteChain();

	if (bRandomBoard)                                           // want a random board selected
		BoardSelected = (brand() % BOARD_COUNT) + BOARD_BASE;

	switch (BoardSelected) {
	case CROSS_PLUS:
		nBoard_DX = CROSS_PLUS_BOARD_DX;
		nBoard_DY = CROSS_PLUS_BOARD_DY;
		for (i = 0; i < GRID_SIZE; i++) {
			for (j = 0; j < GRID_SIZE; j++) {
				fState [i][j] = Board_CrossPlus [i][j] ;
			}
		}
		break ;
	case CROSS:
		nBoard_DX = CROSS_BOARD_DX;
		nBoard_DY = CROSS_BOARD_DY;
		for (i = 0; i < GRID_SIZE; i++) {
			for (j = 0; j < GRID_SIZE; j++) {
				fState [i][j] = Board_Cross [i][j] ;
			}
		}
		break ;
	case TRIANGLE:
		nBoard_DX = TRI_BOARD_DX;
		nBoard_DY = TRI_BOARD_DY;
		for (i = 0; i < GRID_SIZE; i++) {
			for (j = 0; j < GRID_SIZE; j++) {
				fState [i][j] = Board_Triangle [i][j] ;
			}
		}
		break ;
	case TRIANGLE_PLUS:
		nBoard_DX = TRI_PLUS_BOARD_DX;
		nBoard_DY = TRI_PLUS_BOARD_DY;
		for (i = 0; i < GRID_SIZE; i++) {
			for (j = 0; j < GRID_SIZE; j++) {
				fState [i][j] = Board_TrianglePlus [i][j] ;
			}
		}
		break ;
	}

	for (j = 0; j < GRID_SIZE; j++) {
		for (i = 0; i < GRID_SIZE; i++) {
			pNewSprite = nullptr;
			if (fState[i][j] == PEGGED)
				pNewSprite = (*pShotGlass).DuplicateSprite(pDC);
			else if (fState[i][j] == EMPTY)
				pNewSprite = (*pTableSlot).DuplicateSprite(pDC);
			else
//		if ((fState[i][j] == NO_HOLE) && ((BoardSelected == CROSS) || (BoardSelected == CROSS_PLUS)))
				pNewSprite = (*pInvalidSlot).DuplicateSprite(pDC);
			ASSERT(pNewSprite != nullptr);
			cPoint = CMainWindow::GridToPoint(i, j);
			if ((*pNewSprite).GetTypeCode() == SPRITE_HOLE)
				cPoint.x -= (SPRITE_SIZE_DX >> 1);
			(*pNewSprite).SetPosition(cPoint);
			(*pNewSprite).LinkSprite();
		}
	}

	pNewSprite = nullptr;

	counter = 0;

}

/////////////////////////////////////////////////////////////////////////////
//
// Process messages and controls
//
/////////////////////////////////////////////////////////////////////////////

// OnCommand
// This function is called when a WM_COMMAND message is issued,
// typically in order to process control related activities.
//

bool CMainWindow::OnCommand(WPARAM wParam, LPARAM lParam) {
	CDC         *pDC;
	CPoint      sprite_loc;
	CRules      RulesDlg((CWnd *)this, RULESSPEC, pGamePalette,
	                     ((*pGameInfo).bSoundEffectsEnabled ? NARRATIVESPEC : nullptr));
	COptions    COptionsWind((CWnd *)this, pGamePalette, IDD_OPTIONS_DIALOG) ;


	if (HIWORD(lParam) == BN_CLICKED) {

		if (counter & 0x01) {
			pDC = GetDC();
			UndoMove(pDC);
			ReleaseDC(pDC);
		}

		switch (wParam) {

		case IDC_RULES:
			bIgnoreScroll = true;
			(*pScrollButton).SendMessage(BM_SETSTATE, true, 0L);
			RulesDlg.DoModal();                          // invoke the help dialog box
			break;

		case IDC_NEWGAME:
			if (!(*pGameInfo).bPlayingMetagame) {
				pDC = GetDC();
				SetUpBoard(pDC);
				ReleaseDC(pDC);
				InvalidateRect(nullptr, false);
			}
			break;

		case IDC_SCROLL:
			if (bIgnoreScroll) {
				(*pScrollButton).SendMessage(BM_SETSTATE, true, 0L);
				break;
			}
			bIgnoreScroll = true;
			(*pScrollButton).SendMessage(BM_SETSTATE, true, 0L);
			SendDlgItemMessage(IDC_SCROLL, BM_SETSTATE, true, 0L);

			switch (COptionsWind.DoModal()) {

			case IDC_RETURN:
				(*pScrollButton).SendMessage(BM_SETSTATE, false, 0L);
				bIgnoreScroll = false;
				break;

			case IDC_RESTART:
			case IDC_NEWGAME:
				(*pScrollButton).SendMessage(BM_SETSTATE, false, 0L);
				bIgnoreScroll = false;
				pDC = GetDC();
				SetUpBoard(pDC);
				ReleaseDC(pDC);
				InvalidateRect(nullptr, false);
				break;

			case IDC_QUIT:
				PostMessage(WM_CLOSE, 0, 0);
				return false;

			default:
				(*pScrollButton).SendMessage(BM_SETSTATE, false, 0L);
				bIgnoreScroll = false;

			} //end switch(ComDlg.DoModal())

			if ((*pGameInfo).bMusicEnabled) {
				if (pGameSound == nullptr) {
					pGameSound = new CSound(this, GAME_THEME,
					                        SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END);
					if (pGameSound != nullptr)
						(*pGameSound).midiLoopPlaySegment(6370, 33000, 0, FMT_MILLISEC);
				}
			} // end if pGameSound
			else {
				if (pGameSound != nullptr) {
					pGameSound->stop();
					delete pGameSound;
					pGameSound = nullptr;
				}
			}

		} //end switch(wParam)

	}

	(*this).SetFocus();                     // Reset focus back to the main window
	return true;
}


// OnChar and OnSysChar
// These functions are called when keyboard input generates a character.
//

void CMainWindow::OnChar(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	// TODO: Add your message handler code here and/or call default

	CFrameWnd ::OnChar(nChar, nRepCnt, nFlags);     // default action
}


void CMainWindow::OnSysChar(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	// TODO: Add your message handler code here and/or call default

	if ((nChar == 'q') && (nFlags & 0x2000)) {      // terminate app on ALT-q
		CSprite::FlushSpriteChain();
		PostMessage(WM_CLOSE, 0, 0);
	}                   // *** remove later ***
	else
		CFrameWnd ::OnChar(nChar, nRepCnt, nFlags); // default action
}


void CMainWindow::OnKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	if (nChar == VK_F1) {                                  // F1 key is hit
		SendMessage(WM_COMMAND, IDC_RULES, BN_CLICKED);  // Activate the Options dialog
		(*pScrollButton).SendMessage(BM_SETSTATE, false, 0L);
		bIgnoreScroll = false;
	} else if (nChar == VK_F2) {                                 // F2 key is hit
		SendMessage(WM_COMMAND, IDC_SCROLL, BN_CLICKED);     // Activate the Options dialog
		(*pScrollButton).SendMessage(BM_SETSTATE, false, 0L);
		bIgnoreScroll = false;
	}
}


// OnMouseMove and OnButtonXXX:
// These functions are called whenever the corresponding WM_ mouse
// related message occurs.
//

void CMainWindow::OnMouseMove(unsigned int nFlags, CPoint point) {
	CDC    *pDC;
	CPoint real_loc;

	if (counter % 2) {
		real_loc.x = point.x - (SPRITE_SIZE_DX / 2);
		real_loc.y = point.y - (SPRITE_SIZE_DY / 2);
		pDC = GetDC();
		(*pCursorSprite).PaintSprite(pDC, real_loc);
		ReleaseDC(pDC);
	}

	setup_cursor();
}


void CMainWindow::OnLButtonDown(unsigned int nFlags, CPoint myPoint) {
// if you want to do click and click, just check the counter % 2
// if true, then store oldx, oldy, and first click case ("click")
// if false, then do newx, newy, and second click case ("drag")
	CSprite *pSprite;
	int score = 0;
	int num_left ;
	int moves_left = 0;
	int i, j;
	char score_string[40];
	const char *score_blurb;
	CDC     *pDC;
	CPoint  sprite_loc;
	CPoint  grid_loc;
	CSound  *pEffect = nullptr;
	int newx, newy ;
	int oldx, oldy, neighborx, neighbory;
	bool    bSuccess;
	CRect   bottleRect,
	        titleRect,
	        dartRect,
	        kegRect,
	        stoolRect,
	        netRect,
	        oarRect,
	        signRect,
	        tableRect,
	        candlenrRect,
	        candlefrRect;

	bottleRect.SetRect(BOTTLE_X, BOTTLE_Y, BOTTLE_X + BOTTLE_DX, BOTTLE_Y + BOTTLE_DY);
	titleRect.SetRect(TITLE_X, TITLE_Y, TITLE_X + TITLE_DX, TITLE_Y + TITLE_DY);
	dartRect.SetRect(EE_DART_X, EE_DART_Y, EE_DART_X + EE_DART_DX, EE_DART_Y + EE_DART_DY);
	kegRect.SetRect(KEG_X, KEG_Y, KEG_X + KEG_DX, KEG_Y + KEG_DY);
	stoolRect.SetRect(STOOL_X, STOOL_Y, STOOL_X + STOOL_DX, STOOL_Y + STOOL_DY);
	netRect.SetRect(NET_X, NET_Y, NET_X + NET_DX, NET_Y + NET_DY);
	oarRect.SetRect(OAR_X, OAR_Y, OAR_X + OAR_DX, OAR_Y + OAR_DY);
	signRect.SetRect(SIGN_X, SIGN_Y, SIGN_X + SIGN_DX, SIGN_Y + SIGN_DY);
	tableRect.SetRect(TABLE_X, TABLE_Y, TABLE_X + TABLE_DX, TABLE_Y + TABLE_DY);
	candlenrRect.SetRect(CANDLENR_X, CANDLENR_Y, CANDLENR_X + CANDLENR_DX, CANDLENR_Y + CANDLENR_DY);
	candlefrRect.SetRect(CANDLEFR_X, CANDLEFR_Y, CANDLEFR_X + CANDLEFR_DX, CANDLEFR_Y + CANDLEFR_DY);


	if (((*pGameInfo).bPlayingMetagame == false) && titleRect.PtInRect(myPoint)) {
		if (!(counter & 0x01))
			UndoTurn();
		else {
			pDC = GetDC();
			UndoMove(pDC);
			ReleaseDC(pDC);
		}
		SendMessage(WM_COMMAND, IDC_NEWGAME, BN_CLICKED);
	} else if (((*pGameInfo).bPlayingMetagame == false) && bottleRect.PtInRect(myPoint)) {
		if (!(counter & 0x01))
			UndoTurn();
		else {
			pDC = GetDC();
			UndoMove(pDC);
			ReleaseDC(pDC);
		}
	} else if (dartRect.PtInRect(myPoint)) {
		CSound::waitWaveSounds();
		sndPlaySound(nullptr, 0);
		pDC = GetDC();
		pSprite = new CSprite;
		(*pSprite).SharePalette(pGamePalette);
		bSuccess = (*pSprite).LoadCels(pDC, ".\\art\\dart.bmp", NUM_DART_CELS);
		if (!bSuccess) {
			delete pSprite;
			ReleaseDC(pDC);
			return;
		}
		(*pSprite).SetMasked(false);
		(*pSprite).SetMobile(false);

		if ((*pGameInfo).bSoundEffectsEnabled) {
			pEffect = new CSound((CWnd *)this, ".\\sound\\darts.wav",                // Load up the sound file as a
			                     SOUND_WAVE | SOUND_QUEUE | SOUND_ASYNCH | SOUND_AUTODELETE);    //...Wave file, to delete itself
		}
		for (i = 0; i < NUM_DART_CELS; i++) {
			(*pSprite).PaintSprite(pDC, DART_X, DART_Y);
			Sleep(DART_SLEEP - (i));     // * 2
			if ((i == 0) && (pEffect != nullptr)) {
				bSuccess = (*pEffect).play();                                                       //...play the narration
				if (!bSuccess)
					delete pEffect;
			}
		}
		if (pSprite != nullptr)
			delete pSprite;

		ReleaseDC(pDC);
	} else if (kegRect.PtInRect(myPoint)) {
		CSound::waitWaveSounds();
		sndPlaySound(nullptr, 0);
		pDC = GetDC();
		pSprite = new CSprite;
		(*pSprite).SharePalette(pGamePalette);
		bSuccess = (*pSprite).LoadCels(pDC, ".\\art\\keg.bmp", NUM_KEG_CELS);
		if (!bSuccess) {
			delete pSprite;
			ReleaseDC(pDC);
			return;
		}
		(*pSprite).SetMasked(false);
		(*pSprite).SetMobile(false);

		if ((*pGameInfo).bSoundEffectsEnabled) {
			pEffect = new CSound((CWnd *)this, ".\\sound\\barglass.wav",             // Load up the sound file as a
			                     SOUND_WAVE | SOUND_QUEUE | SOUND_ASYNCH | SOUND_AUTODELETE);                //...Wave file, to delete itself
		}
		if (pEffect != nullptr) {
			bSuccess = (*pEffect).play();
			if (!bSuccess)
				delete pEffect;
		}
		(*pSprite).SetCel(NUM_KEG_CELS - KEG_CEL_OFFSET);
		for (i = 0; i < NUM_KEG_CELS; i++) {
			(*pSprite).PaintSprite(pDC, KEG_X, KEG_Y);
			Sleep(KEG_SLEEP);
		}
		if (pSprite != nullptr)
			delete pSprite;

		ReleaseDC(pDC);
	} else if (stoolRect.PtInRect(myPoint)) {
		CSound::waitWaveSounds();
		sndPlaySound(nullptr, 0);
		pDC = GetDC();
		pSprite = new CSprite;
		(*pSprite).SharePalette(pGamePalette);
		bSuccess = (*pSprite).LoadCels(pDC, ".\\art\\stool.bmp", NUM_STOOL_CELS);
		if (!bSuccess) {
			delete pSprite;
			ReleaseDC(pDC);
			return;
		}
		(*pSprite).SetMasked(false);
		(*pSprite).SetMobile(false);

		if ((*pGameInfo).bSoundEffectsEnabled) {
			pEffect = new CSound((CWnd *)this, ".\\sound\\chrdance.wav",             // Load up the sound file as a
			                     SOUND_WAVE | SOUND_QUEUE |  SOUND_ASYNCH | SOUND_AUTODELETE);               //...Wave file, to delete itself
		}
		if (pEffect != nullptr) {
			bSuccess = (*pEffect).play();
			if (!bSuccess)
				delete pEffect;
		}
		for (i = 0; i < NUM_STOOL_CELS; i++) {
			(*pSprite).PaintSprite(pDC, STOOL_X, STOOL_Y);
			Sleep(STOOL_SLEEP);
		}

		delete pSprite;
		ReleaseDC(pDC);

	} else if (oarRect.PtInRect(myPoint) && (*pGameInfo).bSoundEffectsEnabled) {
		CSound::waitWaveSounds();
		sndPlaySound(nullptr, 0);
		pEffect = new CSound((CWnd *)this, OAR_SOUND,                                // Load up the sound file as a
		                     SOUND_WAVE | SOUND_QUEUE |  SOUND_ASYNCH | SOUND_AUTODELETE);   //...Wave file, to delete itself
		if (pEffect != nullptr) {
			bSuccess = (*pEffect).play();
			if (!bSuccess)
				delete pEffect;
		}
	} else if (netRect.PtInRect(myPoint) && (*pGameInfo).bSoundEffectsEnabled) {
		CSound::waitWaveSounds();
		sndPlaySound(nullptr, 0);
		pEffect = new CSound((CWnd *)this, NET_SOUND,                                // Load up the sound file as a
		                     SOUND_WAVE | SOUND_QUEUE |  SOUND_ASYNCH | SOUND_AUTODELETE);   //...Wave file, to delete itself
		if (pEffect != nullptr) {
			bSuccess = (*pEffect).play();
			if (!bSuccess)
				delete pEffect;
		}
	} else if (signRect.PtInRect(myPoint) && (*pGameInfo).bSoundEffectsEnabled) {
		CSound::waitWaveSounds();
		sndPlaySound(nullptr, 0);
		pEffect = new CSound((CWnd *)this, SIGN_SOUND,                               // Load up the sound file as a
		                     SOUND_WAVE | SOUND_QUEUE |  SOUND_ASYNCH | SOUND_AUTODELETE);   //...Wave file, to delete itself
		if (pEffect != nullptr) {
			bSuccess = (*pEffect).play();
			if (!bSuccess)
				delete pEffect;
		}
	} else if (tableRect.PtInRect(myPoint) && (*pGameInfo).bSoundEffectsEnabled) {
		CSound::waitWaveSounds();
		sndPlaySound(nullptr, 0);
		pEffect = new CSound((CWnd *)this, TABLE_SOUND,                              // Load up the sound file as a
		                     SOUND_WAVE | SOUND_QUEUE |  SOUND_ASYNCH | SOUND_AUTODELETE);   //...Wave file, to delete itself
		if (pEffect != nullptr) {
			bSuccess = (*pEffect).play();
			if (!bSuccess)
				delete pEffect;
		}
	} else if ((candlenrRect.PtInRect(myPoint) || candlefrRect.PtInRect(myPoint)) && (*pGameInfo).bSoundEffectsEnabled) {
		CSound::waitWaveSounds();
		sndPlaySound(nullptr, 0);
		pEffect = new CSound((CWnd *)this, CANDLE_SOUND,                                 // Load up the sound file as a
		                     SOUND_WAVE | SOUND_QUEUE |  SOUND_ASYNCH | SOUND_AUTODELETE);   //...Wave file, to delete itself
		if (pEffect != nullptr) {
			bSuccess = (*pEffect).play();
			if (!bSuccess)
				delete pEffect;
		}
	} else if (!(counter & 0x01)) {
		pSprite = CSprite::Touched(myPoint);
		if (pSprite != nullptr) {
			sprite_loc = (*pSprite).GetPosition();
			grid_loc = PointToGrid(sprite_loc);
			if (fState [grid_loc.x][grid_loc.y] == PEGGED) {
				counter += 1;
				Moves[counter].x = grid_loc.x;
				Moves[counter].y = grid_loc.y;
				pDC = GetDC();
				UpdatePegPosition(pDC, pTableSlot, grid_loc.x, grid_loc.y);
				if (counter % 2)
					(*pCursorSprite).PaintSprite(pDC, sprite_loc);
				ReleaseDC(pDC);
				bPegMoving = true;
			} else {             // illegal move
//			    MessageBeep(-1);
				if ((*pGameInfo).bSoundEffectsEnabled)
					sndPlaySound(nullptr, 0);
				sndPlaySound(WAV_NOMOVE, SND_ASYNC);
			}
		}
	} else { // this is the second click
		oldx = Moves[counter].x;
		oldy = Moves[counter].y;
		pSprite = CSprite::Touched(myPoint);
		if (pSprite != nullptr) {
			sprite_loc = (*pSprite).GetPosition();
			if ((*pSprite).GetTypeCode() == SPRITE_HOLE)
				sprite_loc.x += (SPRITE_SIZE_DX >> 1);
			grid_loc = PointToGrid(sprite_loc);
			newx = grid_loc.x;
			newy = grid_loc.y;
			if (fState [newx][newy] == EMPTY) {
				neighborx = ((oldx + newx) / 2);
				neighbory = ((oldy + newy) / 2);
				if ((((newx == oldx - 2) && (newy == oldy + 2)) && (BoardSelected == TRIANGLE || BoardSelected == TRIANGLE_PLUS)) ||
				        ((newx == oldx)     && (newy == oldy + 2)) ||
				        ((newx == oldx + 2) && (newy == oldy))     ||
				        ((newx == oldx - 2) && (newy == oldy))     ||
				        (((newx == oldx + 2) && (newy == oldy - 2)) && (BoardSelected == TRIANGLE || BoardSelected == TRIANGLE_PLUS)) ||
				        ((newx == oldx)     && (newy == oldy - 2))) {
					if (fState [neighborx][neighbory] == PEGGED) {
						fState [oldx][oldy] = EMPTY;
						fState [neighborx][neighbory] = EMPTY;
						fState [newx][newy] = PEGGED ;
						counter += 1;
						bPegMoving = false;
						Moves[counter].x = newx ;
						Moves[counter].y = newy ;
						pDC = GetDC();

						(*pCursorSprite).EraseSprite(pDC);

						UpdatePegPosition(pDC, pShotGlass, newx, newy);

						UpdatePegPosition(pDC, pTableSlot, neighborx, neighbory);

						ReleaseDC(pDC);

						if ((*pGameInfo).bSoundEffectsEnabled) {
							sndPlaySound(nullptr, 0);
							sndPlaySound(WAV_MOVE, SND_ASYNC);
						}

						// check for neighbors and hole after neighbors

						for (i = 0; i < GRID_SIZE; i++) {
							for (j = 0; j < GRID_SIZE; j++) {
								if (fState[i][j] == PEGGED) {
									if (((i <= 4) && (fState [i + 1][j] == PEGGED) && (fState [i + 2][j] == EMPTY)) ||
									        ((i >= 2) && (fState [i - 1][j] == PEGGED) && (fState [i - 2][j] == EMPTY)) ||
									        ((j <= 4) && (fState [i][j + 1] == PEGGED) && (fState [i][j + 2] == EMPTY)) ||
									        ((j <= 4) && (i >= 2) && (fState [i - 1][j + 1] == PEGGED) && (fState [i - 2][j + 2] == EMPTY) && (BoardSelected == TRIANGLE || BoardSelected == TRIANGLE_PLUS)) ||
									        ((i <= 4) && (j >= 2) && (fState [i + 1][j - 1] == PEGGED) && (fState [i + 2][j - 2] == EMPTY) && (BoardSelected == TRIANGLE || BoardSelected == TRIANGLE_PLUS)) ||
									        ((j >= 2) && (fState [i][j - 1] == PEGGED) && (fState [i][j - 2] == EMPTY))) {
										moves_left++;
									}
								}
							}
						}

						if (moves_left == 0) {
							// check for end conditions
							// is counter == maxmoves ?   i.e. only one peg left

							if (((counter == 62) && (BoardSelected == CROSS)) ||
							        ((counter == 70) && (BoardSelected == CROSS_PLUS)) ||
							        ((counter == 26) && (BoardSelected == TRIANGLE)) ||
							        ((counter == 38) && (BoardSelected == TRIANGLE_PLUS))) {

								// if so --> is fState [center] == PEGGED?

								if ((((BoardSelected == CROSS)      || (BoardSelected == CROSS_PLUS))       && (fState [3][3] == PEGGED)) ||
								        ((BoardSelected == TRIANGLE) && (fState [1][1] == PEGGED)) ||
								        ((BoardSelected == TRIANGLE_PLUS) && (fState [2][2] == PEGGED))) {

									// if both true, then WIN!!!!
									score = 25;
								}
								// one left but not in center
								else {
									score = 10;
								}
							}
							// else, just score...  # left = maxchips - ((counter+1)/2) .. counter starts at 0, not 1...
							num_left = (((BoardSelected == CROSS) * 64) +
							            ((BoardSelected == CROSS_PLUS) * 72) +
							            ((BoardSelected == TRIANGLE) * 28) +
							            ((BoardSelected == TRIANGLE_PLUS) * 40) -
							            counter) / 2;
							if (num_left == 2) {
								score = 5;
							} else if (num_left == 3) {
								score = 4;
							} else if (num_left == 4) {
								score = 3;
							} else if (num_left == 5) {
								score = 2;
							} else if ((num_left > 5) && (num_left < 11)) {
								score = 1;
							}
							// display the score
							// display two buttons, Quit or Again?
							#ifndef SHOW_CURSOR
							::ShowCursor(true);
							#endif

							if (score == 25) {
								if ((*pGameInfo).bSoundEffectsEnabled) {
									sndPlaySound(nullptr, 0);
									sndPlaySound(WAV_WON, SND_ASYNC);
								}
								score_blurb = "You have won!";
							} else {
								if ((*pGameInfo).bSoundEffectsEnabled) {
									sndPlaySound(nullptr, 0);
									sndPlaySound(WAV_DONE, SND_ASYNC);
								}
								score_blurb = "Game over.";
							}

							if (score == 1)
								Common::sprintf_s(score_string, "Score:  %d point.", score) ;
							else
								Common::sprintf_s(score_string, "Score:  %d points.", score) ;

							CMessageBox GameOverDlg((CWnd *)this, pGamePalette, score_blurb, score_string);
//						UpdateWindow();
//						GameOverDlg.DoModal();

							if ((*pGameInfo).bPlayingMetagame) {
								#ifndef SHOW_CURSOR
								::ShowCursor(false);
								#endif
								(*pGameInfo).lScore += score;
								PostMessage(WM_CLOSE, 0, 0);
							} else {
								pDC = GetDC();
								SetUpBoard(pDC);
								ReleaseDC(pDC);
								InvalidateRect(nullptr, false);
								// if Restart --> IDC_RESTART
//		            PostMessage(WM_COMMAND, IDC_RESTART, BN_CLICKED);
								#ifndef SHOW_CURSOR
								::ShowCursor(false);
								#endif
							}
						}

						else {
							// do nothing, it's not the last move...
						}
					} else { // Wrong move, ... must jump over a peg!!!!!
						pDC = GetDC();
						UndoMove(pDC);
						ReleaseDC(pDC);
					}
				} else {
					pDC = GetDC();
					UndoMove(pDC);
					ReleaseDC(pDC);

					// Wrong move ... must be 2 away
					// restore hole to a peg
				}
			} else {
				pDC = GetDC();
				UndoMove(pDC);
				ReleaseDC(pDC);

				// Wrong move ... must jump to a hole
				// must restore hole [old] to a peg
			}
		} else {
			counter -= 1;
//			MessageBeep(-1);
			if ((*pGameInfo).bSoundEffectsEnabled) {
				sndPlaySound(nullptr, 0);
				sndPlaySound(WAV_NOMOVE, SND_ASYNC);
			}
			pDC = GetDC();
			(*pCursorSprite).EraseSprite(pDC);

			UpdatePegPosition(pDC, pShotGlass, oldx, oldy);

			ReleaseDC(pDC);
		}
		// Wrong move ... must jump within the board
	}
}


void CMainWindow::OnLButtonUp(unsigned int nFlags, CPoint point) {
	if (bPegMoving)
		OnLButtonDown(nFlags, point);
}


void CMainWindow::OnLButtonDblClk(unsigned int nFlags, CPoint point) {
//  insert mouse button processing code here
}


void CMainWindow::OnMButtonDown(unsigned int nFlags, CPoint point) {
//  insert mouse button processing code here
}


void CMainWindow::OnMButtonUp(unsigned int nFlags, CPoint point) {
//  insert mouse button processing code here
}

void CMainWindow::OnMButtonDblClk(unsigned int nFlags, CPoint point) {
//  insert mouse button processing code here
}


void CMainWindow::OnRButtonDown(unsigned int nFlags, CPoint point) {
//  insert mouse button processing code here
}


void CMainWindow::OnRButtonUp(unsigned int nFlags, CPoint point) {
//  insert mouse button processing code here
}

void CMainWindow::OnRButtonDblClk(unsigned int nFlags, CPoint point) {
//  insert mouse button processing code here
}

//////////// Additional Sound Notify routines //////////////

LRESULT CMainWindow::OnMCINotify(WPARAM wParam, LPARAM lParam) {
	CSound  *pSound;

	pSound = CSound::OnMCIStopped(wParam, lParam);
	if (pSound != nullptr)
		OnSoundNotify(pSound);
	return 0;
}


LRESULT CMainWindow::OnMMIONotify(WPARAM wParam, LPARAM lParam) {
	CSound  *pSound;

	pSound = CSound::OnMMIOStopped(wParam, lParam);
	if (pSound != nullptr)
		OnSoundNotify(pSound);
	return 0;
}

void CMainWindow::OnSoundNotify(CSound *pSound) {
	//
	// Add your code to process explicit notification of a sound "done" event here.
	// pSound is a pointer to a CSound object for which you requested SOUND_NOTIFY.
	//
}



void CMainWindow::UpdatePegPosition(CDC *pDC, CSprite *pBaseSprite, int x, int y) {
	CSprite *pSprite;
	CPoint  sprite_loc,
	        hotspot_loc;

	sprite_loc = GridToPoint(x, y);
	hotspot_loc.x = sprite_loc.x + (SPRITE_SIZE_DX / 2);
	hotspot_loc.y = sprite_loc.y + (SPRITE_SIZE_DY / 2);
	pSprite = CSprite::Touched(hotspot_loc);
	ASSERT(pSprite != nullptr);
	(*pSprite).EraseSprite(pDC);
	(*pSprite).UnlinkSprite();
	delete pSprite;
	pSprite = (*pBaseSprite).DuplicateSprite(pDC);
	ASSERT(pSprite != nullptr);
	(*pSprite).LinkSprite();
	if ((*pSprite).GetTypeCode() == SPRITE_HOLE)
		sprite_loc.x -= (SPRITE_SIZE_DX >> 1);
	(*pSprite).PaintSprite(pDC, sprite_loc.x, sprite_loc.y);
}


void CMainWindow::UndoTurn() {
	CDC     *pDC;
	int     newx, newy, oldx, oldy,
	        neighborx, neighbory;

	if (counter > 0) {
		if ((*pGameInfo).bSoundEffectsEnabled) {
			sndPlaySound(nullptr, 0);
			sndPlaySound(WAV_UNDO, SND_ASYNC);
		}
		bPegMoving = false;
		newx = Moves[counter].x;
		newy = Moves[counter].y;
		counter -= 1;
		pDC = GetDC();
		(*pCursorSprite).EraseSprite(pDC);
		oldx = Moves[counter].x;
		oldy = Moves[counter].y;
		neighborx = (oldx + newx) / 2 ;
		neighbory = (oldy + newy) / 2 ;
		fState [neighborx][neighbory] = PEGGED ;
		fState [oldx][oldy] = PEGGED ;
		fState [newx][newy] = EMPTY ;
		counter -= 1;

		UpdatePegPosition(pDC, pShotGlass, neighborx, neighbory);

		UpdatePegPosition(pDC, pShotGlass, oldx, oldy);

		UpdatePegPosition(pDC, pTableSlot, newx, newy);

		ReleaseDC(pDC);
	}
}


void CMainWindow::UndoMove(CDC *pDC) {
// CSprite  *pSprite;
	CPoint  sprite_loc;
	int     oldx, oldy;

	oldx = Moves[counter].x;
	oldy = Moves[counter].y;

	counter -= 1;
	bPegMoving = false;

//MessageBeep(-1);
	if ((*pGameInfo).bSoundEffectsEnabled)  {
		sndPlaySound(nullptr, 0);
		sndPlaySound(WAV_NOMOVE, SND_ASYNC);
	}

	(*pCursorSprite).EraseSprite(pDC);

	UpdatePegPosition(pDC, pShotGlass, oldx, oldy);
}


CPoint CMainWindow::GridToPoint(int i, int j) {
	CPoint  sprite_loc;

	if (BoardSelected == CROSS) {
		sprite_loc.x = - (CROSS_SHOTGLASS_DX * j) +
		               ((CROSS_SHOTGLASS_DDX + (CROSS_SHOTGLASS_DDDX * j)) * i) +
		               j - 1 + i;
		sprite_loc.y = (CROSS_SHOTGLASS_DY * (j + 1)) + j;
	} else if (BoardSelected == CROSS_PLUS) {
		sprite_loc.x = - (CROSS_PLUS_SHOTGLASS_DX * j) +
		               ((CROSS_PLUS_SHOTGLASS_DDX + (CROSS_PLUS_SHOTGLASS_DDDX * j)) * i) +
		               j - 1 + i;
		sprite_loc.y = (CROSS_PLUS_SHOTGLASS_DY * (j + 1)) + j;
	} else if (BoardSelected == TRIANGLE) {
		sprite_loc.x = (TRI_SHOTGLASS_DX * (i - j));
		sprite_loc.y = (TRI_SHOTGLASS_DY * (i + j + 1));
	} else if (BoardSelected == TRIANGLE_PLUS)  {
		if ((i + j) == 1) {
			sprite_loc.x = (TRI_PLUS_SHOTGLASS_DX * (i - j));
			sprite_loc.y = TRI_PLUS_SHOTGLASS_DY;
		} else if ((i + j) == 2) {
			if ((i != 0) && (j != 0)) {
				sprite_loc.x = 0;
				sprite_loc.y = TRI_PLUS_SHOTGLASS_DY * 2;
			}
		} else if ((i + j) > 2) {
			sprite_loc.x = (TRI_PLUS_SHOTGLASS_DX * (i - j));
			sprite_loc.y = (TRI_PLUS_SHOTGLASS_DY * (i + j));
		}
	}

	sprite_loc.x += nBoard_DX;
	sprite_loc.y += nBoard_DY;

	return (sprite_loc);
}


CPoint CMainWindow::PointToGrid(CPoint myPoint) {
	int     x, y;
	CPoint  sprite_loc;
	CPoint  point;

	point.x = myPoint.x - nBoard_DX;
	point.y = myPoint.y - nBoard_DY;

	if (BoardSelected == CROSS) {
		sprite_loc.y = (point.y - CROSS_SHOTGLASS_DY) /
		               (CROSS_SHOTGLASS_DY + 1);
		sprite_loc.x = (point.x + 1 + (sprite_loc.y * (CROSS_SHOTGLASS_DX - 1))) /
		               (CROSS_SHOTGLASS_DDX + (sprite_loc.y * CROSS_SHOTGLASS_DDDX) + 1);
	} else if (BoardSelected == CROSS_PLUS) {
		sprite_loc.y = (point.y - CROSS_PLUS_SHOTGLASS_DY) /
		               (CROSS_PLUS_SHOTGLASS_DY + 1);
		sprite_loc.x = (point.x + 1 + (sprite_loc.y * (CROSS_PLUS_SHOTGLASS_DX - 1))) /
		               (CROSS_PLUS_SHOTGLASS_DDX + (sprite_loc.y * CROSS_PLUS_SHOTGLASS_DDDX) + 1);
	} else if (BoardSelected == TRIANGLE) {
		x = (point.x / TRI_SHOTGLASS_DX) ; // x1 = int (i-j)
		y = ((point.y / TRI_SHOTGLASS_DY) - 1) ; // y1 = int(i+j)
		sprite_loc.x = ((x + y) / 2) ;
		sprite_loc.y = ((y - x + 1) / 2) ; // +1 because of the size of the sprite
	} else { // Triangle Plus case
		y = (point.y / TRI_PLUS_SHOTGLASS_DY) ; // y1 = int(i+j)
		if (y == 1) {
			if ((point.x < TRI_PLUS_SHOTGLASS_DX) && (point.x >= - TRI_PLUS_SHOTGLASS_DX)) {
				sprite_loc.x = 0;
				sprite_loc.y = 1;
			} else if ((point.x < TRI_PLUS_SHOTGLASS_DX * 2) && (point.x >= TRI_PLUS_SHOTGLASS_DX)) {
				sprite_loc.x = 1;
				sprite_loc.y = 0;
			} else {
				sprite_loc.x = 0;
				sprite_loc.y = 0; // error case (didn't click on a shotglass)
			}
		} else if (y == 2) {
			if ((point.x >= 0) && (point.x <= TRI_PLUS_SHOTGLASS_DX * 2)) {
				sprite_loc.x = 1;
				sprite_loc.y = 1;
			} else {
				sprite_loc.x = 0;
				sprite_loc.y = 0; // error case (didn't click on a shotglass)
			}
		} else if (y > 2) {
			x = (point.x / TRI_PLUS_SHOTGLASS_DX) ; // int(i-j)
			sprite_loc.x = ((x + y) / 2);
			sprite_loc.y = ((y - x + 1) / 2);
		}
	}

	return (sprite_loc);
}

void CMainWindow::OnDestroy() {
//  send a message to the calling app to tell it the user has quit the game
	MFC::PostMessage(m_hCallAppWnd, WM_PARENTNOTIFY, WM_DESTROY, 0L);
	CFrameWnd::OnDestroy();
}

// CMainWindow message map:
// Associate messages with member functions.
//
BEGIN_MESSAGE_MAP(CMainWindow, CFrameWnd)
	//{{AFX_MSG_MAP( CMainWindow )
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_MBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_MOUSEMOVE()
	ON_WM_CHAR()
	ON_WM_SYSCHAR()
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_WM_KEYDOWN()
	ON_WM_ACTIVATEAPP()
	ON_WM_DESTROY()
	ON_MESSAGE(MM_MCINOTIFY, CMainWindow::OnMCINotify)
	ON_MESSAGE(MM_WOM_DONE, CMainWindow::OnMMIONotify)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTheApp

// InitInstance:
// When any CTheApp object is created, this member function is automatically
// called.  Any data may be set up at this point.
//
// Also, the main window of the application should be created and shown here.
// Return true if the initialization is successful.
//
/*
bool CTheApp::InitInstance()
{
CMainWindow *pMyMain;

TRACE( "Boffo Games\n" );

SetDialogBkColor();         // hook gray dialogs (was default in MFC V1)

m_pMainWnd = pMyMain = new CMainWindow();
m_pMainWnd->ShowWindow( m_nCmdShow );

m_pMainWnd->UpdateWindow();

return true;
}

bool CTheApp::ExitInstance()
{

CSprite::FlushSpriteChain();

if (pShotGlass != nullptr)
    delete pShotGlass;
if (pTableSlot != nullptr)
    delete pTableSlot;
if (pInvalidSlot != nullptr)
    delete pInvalidSlot;
if (pCursorSprite != nullptr)
    delete pCursorSprite;

if (pScrollButton != nullptr)
    delete pScrollButton;

if (pGamePalette != nullptr ) {
    pGamePalette->DeleteObject ;
    delete pGamePalette;
    }

// don't forget to set the Cursor back to normal!

#ifndef SHOW_CURSOR
::ShowCursor(true);
#endif

return(true);
}
*/

void CMainWindow::OnClose() {
	CDC     *pDC;
	CBrush  myBrush;
	CRect   myRect;

//	CTheApp::ExitInstance();

	pDC = GetDC();
	myRect.SetRect(0, 0, GAME_WIDTH, GAME_HEIGHT);
	myBrush.CreateStockObject(BLACK_BRUSH);
	(*pDC).FillRect(&myRect, &myBrush);
	ReleaseDC(pDC);

	if ((*pGameInfo).bMusicEnabled) {
		CSound::clearSounds();
	}

	CFrameWnd ::OnClose();
}


void setup_cursor() {
	HCURSOR hNewCursor;
	CWinApp *pMyApp;

	pMyApp = AfxGetApp();

	hNewCursor = (*pMyApp).LoadStandardCursor(IDC_ARROW);

	ASSERT(hNewCursor != nullptr);
	MFC::SetCursor(hNewCursor);
}


void set_wait_cursor() {
	CWinApp *pMyApp;

	pMyApp = AfxGetApp();

	(*pMyApp).BeginWaitCursor();
}


void reset_wait_cursor() {
	CWinApp *pMyApp;

	pMyApp = AfxGetApp();

	(*pMyApp).EndWaitCursor();
}

} // namespace Peggle
} // namespace HodjNPodj
} // namespace Bagel
