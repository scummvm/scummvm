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

#include "bagel/hodjnpodj/hnplibs/dibapi.h"
#include "bagel/hodjnpodj/hnplibs/stdafx.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/hodjnpodj/hnplibs/rules.h"
#include "bagel/hodjnpodj/hnplibs/sprite.h"
#include "bagel/hodjnpodj/hnplibs/cmessbox.h"
#include "bagel/boflib/misc.h"
#include "bagel/boflib/sound.h"
#include "bagel/hodjnpodj/poker/poker.h"
#include "bagel/hodjnpodj/poker/dialogs.h"
#include "bagel/hodjnpodj/poker/c1btndlg.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {
namespace Poker {


// Game theme song
#define     GAME_THEME      ".\\SOUND\\VIDEOPO.MID"

// Rules File Identifiers
#define     RULESFILE       "POKER.TXT"
#define     RULES_WAV       ".\\SOUND\\POKER.WAV"

#define     BETBUTTONS          "ART\\BETBTNS.BMP"
#define     BET1UP              "ART\\BET1U.BMP"

#define     CLEARBETUP          "ART\\CLRBETU.BMP"
#define     CLEARBETDOWN        "ART\\CLRBETD.BMP"
#define     CLEARBETDISABLED    "ART\\CLRBETG.BMP"

#define     DEALUP              "ART\\DEALU.BMP"
#define     DEALDOWN            "ART\\DEALD.BMP"
#define     DEALDISABLED        "ART\\DEALG.BMP"

#define     DRAWUP              "ART\\DRAWU.BMP"
#define     DRAWDOWN            "ART\\DRAWD.BMP"
#define     DRAWDISABLED        "ART\\DRAWG.BMP"

#define     HOLDBUTTONS             "ART\\HOLDBTNS.BMP"

#define     MAXBET               10000

extern CMainPokerWindow *pcwndPoker;

CPalette        *pGamePalette = nullptr;           // Palette to be used throughout the game
CPalette        *pBtnPalette = nullptr;            // Palette to be used throughout the game
CBmpButton      *pOptionButton = nullptr;      // Option button object for getting to the options dialog


// gtb
//static bool bIgnoreButtonClick = false;
int roundOfPlay;

CSprite             *apCard[5];             // Bitmap buttons for displaying the cards

CBitmap             *pBetButtons = nullptr;
CBmpButton  *apBet[7];
CBitmap     *pUpBmp = nullptr;
CBitmap     *pDnBmp = nullptr;
CBitmap     *pDsBmp = nullptr;
static const RECT arDstBet[7] = {
	{ BET1_LEFT, BET1_TOP, BET1_LEFT + BET1_WIDTH, BET1_TOP + BET1_HEIGHT },
	{ BET5_LEFT, BET5_TOP, BET5_LEFT + BET5_WIDTH, BET5_TOP + BET5_HEIGHT },
	{ BET10_LEFT, BET10_TOP, BET10_LEFT + BET10_WIDTH, BET10_TOP + BET10_HEIGHT },
	{ BET25_LEFT, BET25_TOP, BET25_LEFT + BET25_WIDTH, BET25_TOP + BET25_HEIGHT },
	{ BET100_LEFT, BET100_TOP, BET100_LEFT + BET100_WIDTH, BET100_TOP + BET100_HEIGHT },
	{ BET1000_LEFT, BET1000_TOP, BET1000_LEFT + BET1000_WIDTH, BET1000_TOP + BET1000_HEIGHT },
	{ BETALL_LEFT, BETALL_TOP, BETALL_LEFT + BETALL_WIDTH, BETALL_TOP + BETALL_HEIGHT }
};


CBmpButton  *apHold[5];       // Bitmap buttons for the Hold card buttons
CBitmap     *pHoldButtons = nullptr;

static const RECT arHoldRect[5] = {
	{ HOLD1_LEFT, HOLD_TOP, HOLD1_LEFT + HOLD_WIDTH, HOLD_TOP + HOLD_HEIGHT },
	{ HOLD2_LEFT, HOLD_TOP, HOLD2_LEFT + HOLD_WIDTH, HOLD_TOP + HOLD_HEIGHT },
	{ HOLD3_LEFT, HOLD_TOP, HOLD3_LEFT + HOLD_WIDTH, HOLD_TOP + HOLD_HEIGHT },
	{ HOLD4_LEFT, HOLD_TOP, HOLD4_LEFT + HOLD_WIDTH, HOLD_TOP + HOLD_HEIGHT },
	{ HOLD5_LEFT, HOLD_TOP, HOLD5_LEFT + HOLD_WIDTH, HOLD_TOP + HOLD_HEIGHT },
};

CBmpButton          *pClearBet = nullptr;      // Button for betting Clearing out the bet
CBmpButton          *pDealButton = nullptr;    // Button for deal all five cards, starts a hand
CBmpButton          *pDrawButton = nullptr;  // Button for drawing cards not held, after initial deal

static  CSound  *pGameSound = nullptr;                             // Game theme song

static const char *sBitmaps[53] = {
	"ART\\PKR1.BMP",
	"ART\\PKR2.BMP",
	"ART\\PKR3.BMP",
	"ART\\PKR4.BMP",
	"ART\\PKR5.BMP",
	"ART\\PKR6.BMP",
	"ART\\PKR7.BMP",
	"ART\\PKR8.BMP",
	"ART\\PKR9.BMP",
	"ART\\PKR10.BMP",
	"ART\\PKR11.BMP",
	"ART\\PKR12.BMP",
	"ART\\PKR13.BMP",
	"ART\\PKR14.BMP",
	"ART\\PKR15.BMP",
	"ART\\PKR16.BMP",
	"ART\\PKR17.BMP",
	"ART\\PKR18.BMP",
	"ART\\PKR19.BMP",
	"ART\\PKR20.BMP",
	"ART\\PKR21.BMP",
	"ART\\PKR22.BMP",
	"ART\\PKR23.BMP",
	"ART\\PKR24.BMP",
	"ART\\PKR25.BMP",
	"ART\\PKR26.BMP",
	"ART\\PKR27.BMP",
	"ART\\PKR28.BMP",
	"ART\\PKR29.BMP",
	"ART\\PKR30.BMP",
	"ART\\PKR31.BMP",
	"ART\\PKR32.BMP",
	"ART\\PKR33.BMP",
	"ART\\PKR34.BMP",
	"ART\\PKR35.BMP",
	"ART\\PKR36.BMP",
	"ART\\PKR37.BMP",
	"ART\\PKR38.BMP",
	"ART\\PKR39.BMP",
	"ART\\PKR40.BMP",
	"ART\\PKR41.BMP",
	"ART\\PKR42.BMP",
	"ART\\PKR43.BMP",
	"ART\\PKR44.BMP",
	"ART\\PKR45.BMP",
	"ART\\PKR46.BMP",
	"ART\\PKR47.BMP",
	"ART\\PKR48.BMP",
	"ART\\PKR49.BMP",
	"ART\\PKR50.BMP",
	"ART\\PKR51.BMP",
	"ART\\PKR52.BMP",
	"ART\\CARDBACK.BMP"
};

int     nPayOff;
bool    bFirstTime;

/////////////////////////////////////////////////////////////////////////////

/*****************************************************************
 *
 * CMainPokerWindow
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

CMainPokerWindow::CMainPokerWindow(HWND hCallingWnd, LPGAMESTRUCT lpGameStruct) :
		NewGameRect(21, 4, 225, 21) {
	CDC     *pDC = nullptr;                        // device context for the screen
	CString WndClass;
	CDibDoc *pDibDoc = nullptr;            // pointer to the background art DIB
	CSize   mySize;
	int     nCounter1;
	bool    bTestCreate,                        // bool for testing the creation of each button
	        bTestLoadBmp,                       // bool for testing the loading of bitmaps
	        bTestDibDoc;                        // bool for testing the creation of the splash screen
	int     i;
	CPalette    *pOldPal = nullptr;

	BeginWaitCursor();
	initStatics();

	// Define a special window class which traps double-clicks, is byte aligned
	// to maximize BITBLT performance, and creates "owned" DCs rather than sharing
	// the five system defined DCs which are not guaranteed to be available;
	// this adds a bit to our app size but avoids hangs/freezes/lockups.
	WndClass = AfxRegisterWndClass(CS_DBLCLKS | CS_BYTEALIGNWINDOW | CS_OWNDC,
	                               nullptr, nullptr, nullptr);

	for (nCounter1 = 0; nCounter1 < 10; ++nCounter1)    {
		aDealtArray[nCounter1][0] = 0;
		aDealtArray[nCounter1][1] = 0;
	}

	// initialize private members
	m_lpGameStruct = lpGameStruct;

	// Make sure user has money before the game starts:
	if (m_lpGameStruct->bPlayingMetagame) {
		m_nRound = NUMBEROFROUNDS;
		m_lStartingAmount = m_lpGameStruct->lCrowns;
		m_lUserAmount = m_lpGameStruct->lCrowns;
		m_bPlayRounds = true;
	} else {
		m_nRound = 0;
		m_lStartingAmount = 1000;
		m_lUserAmount = 1000;
		m_bPlayRounds = false;
	}

	roundOfPlay = 0;
	m_hCallAppWnd = hCallingWnd;
	m_bPlaySounds = m_lpGameStruct->bSoundEffectsEnabled;

	m_lUserBet = 0;

// load splash screen
	pDC = GetDC();                                  // get a device context for our window

//()FetchBitmap( pDC, &pGamePalette, SPLASHSPEC );


	pDibDoc = new CDibDoc();                        // create an object to hold our splash screen
	ASSERT(pDibDoc);                                // ... and verify we got it
	bTestDibDoc = pDibDoc->OpenDocument(SPLASHSPEC);    // next load in the actual DIB based artwork
	ASSERT(bTestDibDoc);
	pGamePalette = (*pDibDoc).DetachPalette();      // grab its palette and save it for later use
	delete pDibDoc;                                 // now discard the splash screen

// set window coordinates to center game on screeen
	MainRect.left = (pDC->GetDeviceCaps(HORZRES) - GAME_WIDTH) >> 1;
	MainRect.top = (pDC->GetDeviceCaps(VERTRES) - GAME_HEIGHT) >> 1;
	MainRect.right = MainRect.left + GAME_WIDTH;    // determine where to place the game window
	MainRect.bottom = MainRect.top + GAME_HEIGHT;   // ... so it is centered on the screen

	ReleaseDC(pDC);                                 // release our window context

// Create the window as a POPUP so that no boarders, title, or menu are present;
// this is because the game's background art will fill the entire 640x40 area.
	Create(WndClass, "Boffo Games -- Poker", WS_POPUP, MainRect, nullptr, 0);

	ShowWindow(SW_SHOWNORMAL);
	UpdateWindow();

	// create buttons
	pOptionButton = new CBmpButton;         // create the Options button
	ASSERT(pOptionButton);
	OptionRect.SetRect(OPTION_LEFT,
	                   OPTION_TOP,
	                   OPTION_LEFT + OPTION_WIDTH,
	                   OPTION_TOP + OPTION_HEIGHT);
	bTestCreate = pOptionButton->Create(nullptr, BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, OptionRect, this, IDC_OPTION);
	ASSERT(bTestCreate);                // test for button's creation
	bTestLoadBmp = pOptionButton->LoadBitmaps((IDB_OPTIONSCROLL),
	               (IDB_OPTIONSCROLL),
	               (IDB_OPTIONSCROLL),
	               (IDB_OPTIONSCROLL));
	ASSERT(bTestLoadBmp);                 // test for button's bitmap loading

	pDealButton = new CBmpButton;   // create the Deal button
	ASSERT(pDealButton);
	DealRect.SetRect(DEAL_LEFT,
	                 DEAL_TOP,
	                 DEAL_LEFT + DEAL_WIDTH,
	                 DEAL_TOP + DEAL_HEIGHT);
	bTestCreate = pDealButton->Create(nullptr, BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, DealRect, this, IDC_DEAL);
	ASSERT(bTestCreate);                // test for button's creation
	bTestLoadBmp = pDealButton->LoadBmpBitmaps(DEALUP, DEALDOWN, nullptr, DEALDISABLED);
	ASSERT(bTestLoadBmp);                 // test for button's bitmap loading

	pDrawButton = new CBmpButton;   // create the Draw button
	ASSERT(pDrawButton);
	DrawRect.SetRect(DRAW_LEFT,
	                 DRAW_TOP,
	                 DRAW_LEFT + DRAW_WIDTH,
	                 DRAW_TOP + DRAW_HEIGHT);
	bTestCreate = pDrawButton->Create(nullptr, BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, DrawRect, this, IDC_DRAW);
	ASSERT(bTestCreate);              // test for button's creation
	bTestLoadBmp = pDrawButton->LoadBmpBitmaps(DRAWUP, DRAWDOWN, nullptr, DRAWDISABLED);
	ASSERT(bTestLoadBmp);                 // test for button's bitmap loading

	pDC = nullptr;
	pDC = GetDC();
	pOldPal = pDC->SelectPalette(pGamePalette, false);
	pDC->RealizePalette();

	pBetButtons = FetchBitmap(pDC, nullptr, BETBUTTONS);

	for (i = 0; i < 7; i++) {

		pUpBmp = ExtractBitmap(pDC, pBetButtons, pGamePalette,
		                       (i * BET_BMP_GRID_WITDH), BET_BMP_GRID_TOP1,
		                       arDstBet[i].right - arDstBet[i].left,
		                       arDstBet[i].bottom - arDstBet[i].top);
		pDnBmp = ExtractBitmap(pDC, pBetButtons, pGamePalette,
		                       (i * BET_BMP_GRID_WITDH), BET_BMP_GRID_TOP2,
		                       arDstBet[i].right - arDstBet[i].left,
		                       arDstBet[i].bottom - arDstBet[i].top);
		pDsBmp = ExtractBitmap(pDC, pBetButtons, pGamePalette,
		                       (i * BET_BMP_GRID_WITDH), BET_BMP_GRID_TOP3,
		                       arDstBet[i].right - arDstBet[i].left,
		                       arDstBet[i].bottom - arDstBet[i].top);

		apBet[i] = nullptr;
		apBet[i] = new CBmpButton;          // create the Bet 1 Crown button
		ASSERT(apBet[i] != nullptr);

		bTestCreate = apBet[i]->Create(nullptr, BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, arDstBet[i], this, IDC_BET1 + i);
		ASSERT(bTestCreate);      // test for button's creation

		bTestLoadBmp = apBet[i]->LoadBitmaps(pGamePalette, pUpBmp, pDnBmp, nullptr, pDsBmp);
		ASSERT(bTestLoadBmp);                 // test for button's bitmap loading

		pUpBmp = nullptr;
		pDnBmp = nullptr;
		pDsBmp = nullptr;
		pBtnPalette = nullptr;
	}

	pHoldButtons = FetchBitmap(pDC, nullptr, HOLDBUTTONS);

	for (i = 0; i < 5; i++) {

		arCardRect[i].SetRect(CARD_LEFT + (i * CARD_OFFSET) - 1, CARD_TOP - 1,
		                      CARD_LEFT + CARD_WIDTH + (i * CARD_OFFSET) - 1,  CARD_TOP + CARD_HEIGHT - 1);
		apCard[i] = nullptr;

		pUpBmp = ExtractBitmap(pDC, pHoldButtons, pGamePalette,
		                       (i * HOLD_BMP_GRID_WITDH), HOLD_BMP_GRID_TOP2,
		                       arHoldRect[i].right - arHoldRect[i].left,
		                       arHoldRect[i].bottom - arHoldRect[i].top);
		pDnBmp = ExtractBitmap(pDC, pHoldButtons, pGamePalette,
		                       (i * HOLD_BMP_GRID_WITDH), HOLD_BMP_GRID_TOP3,
		                       arHoldRect[i].right - arHoldRect[i].left,
		                       arHoldRect[i].bottom - arHoldRect[i].top);
		pDsBmp = ExtractBitmap(pDC, pHoldButtons, pGamePalette,
		                       (i * HOLD_BMP_GRID_WITDH), HOLD_BMP_GRID_TOP4,
		                       arHoldRect[i].right - arHoldRect[i].left,
		                       arHoldRect[i].bottom - arHoldRect[i].top);

		apHold[i] = nullptr;
		apHold[i] = new CBmpButton;         // create the Bet 1 Crown button
		ASSERT(apHold[i] != nullptr);

		bTestCreate = apHold[i]->Create(nullptr, BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, arHoldRect[i], this, (IDC_HOLD1 + i));
		ASSERT(bTestCreate);      // test for button's creation

		bTestLoadBmp = apHold[i]->LoadBitmaps(pGamePalette, pUpBmp, pDnBmp, nullptr, pDsBmp);
		ASSERT(bTestLoadBmp);                 // test for button's bitmap loading

		pUpBmp = nullptr;
		pDnBmp = nullptr;
		pDsBmp = nullptr;
		pBtnPalette = nullptr;
	}

	pDC->SelectPalette(pOldPal, false);
	pDC->RealizePalette();
	pOldPal = nullptr;
	ReleaseDC(pDC);

	pClearBet = new CBmpButton;  // create the Clear Bet button
	ASSERT(pClearBet);
	ClearBetRect.SetRect(CLEARBET_LEFT,
	                     CLEARBET_TOP,
	                     CLEARBET_LEFT + CLEARBET_WIDTH,
	                     CLEARBET_TOP + CLEARBET_HEIGHT);
	bTestCreate = pClearBet->Create(nullptr, BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, ClearBetRect, this, IDC_CLEARBET);
	ASSERT(bTestCreate);      // test for button's creation
	bTestLoadBmp = pClearBet->LoadBmpBitmaps(CLEARBETUP, CLEARBETDOWN, nullptr, CLEARBETDISABLED);
	ASSERT(bTestLoadBmp);                 // test for button's bitmap loading

// initialize payoffs to Poporrean
	m_nPayOffRoyalFlush = 500;
	m_nPayOffStraightFlush = 100;
	m_nPayOffFourofaKind = 50;
	m_nPayOffFullHouse = 15;
	m_nPayOffFlush = 10;
	m_nPayOffStraight = 8;
	m_nPayOffThreeofaKind = 5;
	m_nPayOffTwoPair = 3;
	m_nPayOffPairJackorHigher = 2;
	m_nPayOffPair = 1;
	nPayOff = IDC_POPO;

// initialize BOOLS
	m_bMiddleOfHand = false;
	m_bMouseCaptured = false;
	m_bEndHand = true;

	if (m_lpGameStruct->bMusicEnabled) {
		pGameSound = new CSound(this, GAME_THEME,
		                        SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END);
		if (pGameSound != nullptr)
			(*pGameSound).midiLoopPlaySegment(6370, 33000, 0, FMT_MILLISEC);
	} // end if pGameSound

	bFirstTime = true;

	EndWaitCursor();

}

void CMainPokerWindow::initStatics() {
	pGamePalette = nullptr;
	pBtnPalette = nullptr;
	pOptionButton = nullptr;

	roundOfPlay = 0;
	Common::fill(apCard, apCard + 5, nullptr);
	pBetButtons = nullptr;
	Common::fill(apBet, apBet + 7, nullptr);
	pUpBmp = nullptr;
	pDnBmp = nullptr;
	pDsBmp = nullptr;

	Common::fill(apHold, apHold + 5, nullptr);
	pHoldButtons = nullptr;

	pClearBet = nullptr;
	pDealButton = nullptr;
	pDrawButton = nullptr;

	pGameSound = nullptr;
	nPayOff = 0;
	bFirstTime = false;
}

/*****************************************************************
 *
 * EnableBets
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  Enables and disables the Bet buttons dependent upon if the user
 *  has that much to bet
 *
 * FORMAL PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  UserAmount
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
void CMainPokerWindow::EnableBets() {

// if not in Round play or have played less than m_nRound rounds
	if ((m_bPlayRounds == false) || (roundOfPlay < m_nRound)) {
// disable/enable Bet buttons accoringly

		if (m_lUserAmount >= 1) {
			apBet[0]->EnableWindow(true);
			apBet[6]->EnableWindow(true);
		} else {
			apBet[0]->EnableWindow(false);
			apBet[6]->EnableWindow(false);
		}

		if (m_lUserAmount >= 5)
			apBet[1]->EnableWindow(true);
		else
			apBet[1]->EnableWindow(false);

		if (m_lUserAmount >= 10)
			apBet[2]->EnableWindow(true);
		else
			apBet[2]->EnableWindow(false);

		if (m_lUserAmount >= 25)
			apBet[3]->EnableWindow(true);
		else
			apBet[3]->EnableWindow(false);

		if (m_lUserAmount >= 100)
			apBet[4]->EnableWindow(true);
		else
			apBet[4]->EnableWindow(false);

		if (m_lUserAmount >= 1000)
			apBet[5]->EnableWindow(true);
		else
			apBet[5]->EnableWindow(false);

		if (m_lUserBet > 0)
			pClearBet->EnableWindow(true);
		else
			pClearBet->EnableWindow(false);

		if (m_lUserBet >= MAXBET) {
			apBet[0]->EnableWindow(false);
			apBet[1]->EnableWindow(false);
			apBet[2]->EnableWindow(false);
			apBet[3]->EnableWindow(false);
			apBet[4]->EnableWindow(false);
			apBet[5]->EnableWindow(false);
			apBet[6]->EnableWindow(false);
		}
	}

	return;
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

void CMainPokerWindow::OnPaint() {
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

void CMainPokerWindow::SplashScreen() {
	CRect           rcDest;                                 // defines where the art gets painted
	CRect           rcDIB;                                  // defines where the art comes from
	CDC             *pDC;                                       // pointer to the window device context
	CDibDoc     myDoc;                  // contains the artwork's DIB information
	HDIB            hDIB;                   // a handle to the DIB itself
	char            cBet[7];                                // empty strings for displaying the user's bet
	char            cUser[12];                          // empty strings for displaying the user's amount
// COLORREF clrOldColorRef;         // a place holder for the old text color in the DC
// int              nOldBkMode;             // a place holder for the old Background Mode in the DC
	bool            bTestDibDoc;                        // a bool for testing the return on DibDoc

	char            cUserDisplay[32];       // the character array to display for the user's amount
	char            cBetDisplay[12];                // the character array to display for the bet amount
	int             nCharIndex;             // keeps track of the current element in the old array
	int             nDisplayIndex;          // keeps track of the current element in the new array
	int             nCounter1;              // counter
	int             i, j;
	CText           *ptxtUserDisplay = nullptr;
	CText           *ptxtBetDisplay = nullptr;
	CRect           rectDisplayUser;
	CRect           rectDisplayBet;

	bTestDibDoc = myDoc.OpenDocument(SPLASHSPEC);
	ASSERT(bTestDibDoc);

	hDIB = myDoc.GetHDIB();           // ... get a handle to its DIB
	ASSERT(hDIB);

	pDC = GetDC();                                      // get a device context for the window
	ASSERT(pDC);

	GetClientRect(rcDest);                   // get the rectangle to where we paint

	int cxDIB = (int) DIBWidth(hDIB);
	int cyDIB = (int) DIBHeight(hDIB);

	rcDIB.top = rcDIB.left = 0;             // setup the source rectangle from which
	rcDIB.right = cxDIB;                // ... we'll do the painting
	rcDIB.bottom = cyDIB;
	PaintDIB((*pDC).m_hDC, &rcDest, hDIB, &rcDIB, pGamePalette);    // transfer the image to the screen

	rectDisplayUser.SetRect(USER_AMOUNT_X, USER_AMOUNT_Y, USER_AMOUNT_X + AMOUNT_WIDTH, USER_AMOUNT_Y + AMOUNT_HEIGHT);
	rectDisplayBet.SetRect(POT_AMOUNT_X, POT_AMOUNT_Y, POT_AMOUNT_X + AMOUNT_WIDTH, POT_AMOUNT_Y + AMOUNT_HEIGHT);

	ptxtUserDisplay = new CText;
	ptxtBetDisplay = new CText;

// create the display strings for the bet and user amount
	Common::sprintf_s((char *)cBet, 10, "%li", m_lUserBet);
	Common::sprintf_s((char *)cUser, 10, "%li", m_lUserAmount);

	j = 0;
	for (nCharIndex = strlen((char *)cUser) - 1, nDisplayIndex = nCharIndex + (nCharIndex / 3), nCounter1 = 1;
	        nCharIndex >= 0;
	        nCharIndex--, nDisplayIndex--, nCounter1++) {

		if ((nCounter1 % 4) == 0) {
			cUserDisplay[nDisplayIndex] = ',';
			nCharIndex++;
			j++;
		} else {
			cUserDisplay[nDisplayIndex] = cUser[nCharIndex];
		}
	}
	nDisplayIndex = strlen((char *)cUser);
	cUserDisplay[nDisplayIndex + j] = 0;

	ptxtUserDisplay->SetupText(pDC, pGamePalette, &rectDisplayUser, JUSTIFY_CENTER);
	ptxtUserDisplay->DisplayString(pDC, cUserDisplay, 16, FW_BOLD, (COLORREF)RGB(0, 0, 255));

	for (nCharIndex = strlen((char *)cBet) - 1, nDisplayIndex = nCharIndex + (nCharIndex / 3), nCounter1 = 1;
	        nCharIndex >= 0;
	        nCharIndex--, nDisplayIndex--, nCounter1++) {

		if ((nCounter1 % 4) == 0) {
			cBetDisplay[nDisplayIndex] = ',';
			nCharIndex++;
		} else {
			cBetDisplay[nDisplayIndex] = cBet[nCharIndex];
		}
	}
	nDisplayIndex = strlen((char *)cBet);
	cBetDisplay[nDisplayIndex + (nDisplayIndex / 4)] = 0;

	ptxtBetDisplay->SetupText(pDC, pGamePalette, &rectDisplayBet, JUSTIFY_CENTER);
	ptxtBetDisplay->DisplayString(pDC, cBetDisplay, 16, FW_BOLD, RGB(0, 0, 255));

	for (i = 0; i < 5; i++) {
		CPoint  ptTemp(arCardRect[i].left + 1, arCardRect[i].top + 1);

		if (apCard[i] != nullptr)
			apCard[i]->PaintSprite(pDC, ptTemp.x, ptTemp.y);
	}

	ReleaseDC(pDC);                                     // release the window's context
	delete ptxtUserDisplay;
	delete ptxtBetDisplay;

}


/*****************************************************************
 *
 * ResetGame
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * Start a new game, and reset all arrays and buttons
 *
 * FORMAL PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  aDealtArray, apHold
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

void CMainPokerWindow::ResetGame(long lAmount) {
	int nLoop;
	CDC     *pDC = nullptr;

	// set arrays to 0
	for (nLoop = 0; nLoop < 10; ++nLoop)    {
		aDealtArray[nLoop][0] = 0;
		aDealtArray[nLoop][1] = 0;
	}
	for (nLoop = 0; nLoop < 5; ++nLoop) {
		apHold[nLoop]->EnableWindow(false);
		if (bFirstTime == false) {
			abHoldArray[nLoop] = true;
			SetHoldList(nLoop);
		} else {
			abHoldArray[nLoop] = false;
		}
	}

	if (bFirstTime)
		bFirstTime = false;

// disable deal and draw buttons
	pDealButton->EnableWindow(false);
	pDrawButton->EnableWindow(false);

// clear out any bet
	if (lAmount == 0L)
		SetBet(-1);
	else {
		m_lUserAmount = lAmount;
		m_lUserBet = 0L;
		SetBet(0);
	}
	for (nLoop = 0; nLoop < 5; ++nLoop) {

		if (apCard[nLoop] != nullptr) {
			pDC = GetDC();
			apCard[nLoop]->EraseSprite(pDC);
			delete apCard[nLoop];
			apCard[nLoop] = nullptr;
			ReleaseDC(pDC);
		}

	}
	return;
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
void CALLBACK lpfnOptionCallback(CWnd * pWnd) {                          // do the mini options dialog
	int     nOption = 0;         // return from the Options dialog
	int     nSetAmount = 0;      // return from the Set User Amount dialog
	int     nSetPayOff = 0;      // return from the Set Payoffs dialog
//	unsigned int x = IDD_MINIOPTIONS_DIALOG;

	COptionsDlg     dlgOptions(pWnd, pGamePalette, IDD_OPTIONS);
	CSetAmountDlg   dlgSetAmt(pWnd, pGamePalette, IDD_SETAMOUNT);
	CSetPayoffsDlg  dlgPayOff(pWnd, pGamePalette, IDD_WINRATIO, nPayOff);

	dlgOptions.SetInitialOptions((pcwndPoker->m_bMiddleOfHand || pcwndPoker->m_bPlayRounds),
	                             pcwndPoker->m_bPlaySounds);

	nOption = dlgOptions.DoModal();

	switch (nOption) {

	case ID_SETPAYOFFS:                     // if Winning Ratio's button was hit, popup the Payoff dialog
		(*pWnd).UpdateWindow();
		nSetPayOff = dlgPayOff.DoModal();
		nPayOff = nSetPayOff;
		pcwndPoker->SetPayOffs(nSetPayOff);              // the return is what set of payoffs was selected was clicked
		pcwndPoker->FlagResetGame();
		break;

	case ID_SETUSERAMT:                     // if Set Amount button was hit, popup the Set Amount dialog
		dlgSetAmt.SetInitialOptions(pcwndPoker->m_lUserAmount);
		(*pWnd).UpdateWindow();
		nSetAmount = dlgSetAmt.DoModal();
		if (nSetAmount != 0)   {             // the return is what the amount was set to, set m_lUserAmount to it
			pcwndPoker->m_lUserAmount = nSetAmount;
			pcwndPoker->m_lStartingAmount = nSetAmount;
		}
		pcwndPoker->FlagResetGame();
		break;
	}
	return;
}


bool CMainPokerWindow::OnCommand(WPARAM wParam, LPARAM lParam) {
	int         nHoldLoop, nLoop1;  // loop variables
	int         nMainOption = 0;       // return from the Options dialog
	CMainMenu       dlgMainOpts((CWnd *)this, pGamePalette,
	                            (m_lpGameStruct->bPlayingMetagame ? (NO_NEWGAME | NO_OPTIONS) : 0),
	                            lpfnOptionCallback, RULESFILE,
	                            (m_lpGameStruct->bSoundEffectsEnabled ? RULES_WAV : nullptr), m_lpGameStruct);

	CDC     *pDC = nullptr;
	int         nTemp = 0;

	if (HIWORD(lParam) == BN_CLICKED)           // only want to look at button clicks
		switch (wParam) {

		case IDC_OPTION:                                // Option button clicked, then put up the Options dialog
			pOptionButton->ShowWindow(SW_HIDE);
			CSound::waitWaveSounds();
			sndPlaySound(nullptr, 0);
			_flagResetGame = false;

			nMainOption = dlgMainOpts.DoModal();

			// If the minigame options were changed, we need to reset the game
			if (_flagResetGame && nMainOption == IDC_OPTIONS_RETURN)
				nMainOption = IDC_OPTIONS_NEWGAME;
			_flagResetGame = false;

			switch (nMainOption) {
			case IDC_OPTIONS_QUIT:                      // if Quit buttons was hit, quit
				PostMessage(WM_CLOSE, 0, 0);
				break;

			case IDC_OPTIONS_NEWGAME:                   // reset the game and start a new hand
				if ((m_bPlayRounds == false) && (m_bMiddleOfHand == false)) {
					ResetGame(m_lStartingAmount);
				}
				break;

			default:
				break;
			}

			m_bPlaySounds = m_lpGameStruct->bSoundEffectsEnabled;
			if (m_lpGameStruct->bMusicEnabled) {
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
			pOptionButton->ShowWindow(SW_SHOWNORMAL);
			EnableBets();
			for (nTemp = 0; nTemp < 7; nTemp++) {
				apBet[nTemp]->RedrawWindow();
			}
			//          PostMessage( WM_COMMAND, wParam, lParam);
			break;

		// if the Hols buttons are hit then set then accordingly
		case IDC_HOLD1:
			SetHoldList(0);
			break;

		case IDC_HOLD2:
			SetHoldList(1);
			break;

		case IDC_HOLD3:
			SetHoldList(2);
			break;

		case IDC_HOLD4:
			SetHoldList(3);
			break;

		case IDC_HOLD5:
			SetHoldList(4);
			break;

		// the Draw button is hit
		case IDC_DRAW:

			m_bMiddleOfHand = false;

			// increment the number of rounds
			roundOfPlay++;
			// for all cards not held, draw a new card
			for (nHoldLoop = 0; nHoldLoop < 5; ++nHoldLoop) {
				if (abHoldArray[nHoldLoop] == false) {
					ShowNewCard(DealNewCard(), nHoldLoop);
					aDealtArray[nHoldLoop][1] = 0;
				}
			}
			// check to see if the hand won
			CheckWinningHand();
			m_bEndHand = true;
			for (nLoop1 = 0; nLoop1 < 5; ++nLoop1) {

				pDC = GetDC();
				apCard[nLoop1]->EraseSprite(pDC);
				delete apCard[nLoop1];
				apCard[nLoop1] = nullptr;

				ReleaseDC(pDC);

			}
			// reset all arrays and buttons and start a new hand
			for (nLoop1 = 0; nLoop1 < 10; ++nLoop1) {
				aDealtArray[nLoop1][0] = 0;
				aDealtArray[nLoop1][1] = 0;
			}

			pDealButton->EnableWindow(false);
			pDrawButton->EnableWindow(false);
			for (nHoldLoop = 0; nHoldLoop < 5; ++nHoldLoop) {
				apHold[nHoldLoop]->EnableWindow(false);
				abHoldArray[nHoldLoop] = true;
				SetHoldList(nHoldLoop);
			}

			if ((m_lpGameStruct->bPlayingMetagame) && (m_lUserAmount == 0)) {
				C1ButtonDialog  cMsgBox((CWnd *)this, pGamePalette, "&Okay", "You Lose.", "You have 0", "crowns left.");
				cMsgBox.DoModal();
				PostMessage(WM_CLOSE);
			} else {
				SetBet(0);
				if (m_lpGameStruct->bPlayingMetagame) {
					if (roundOfPlay >= m_nRound) {
						CMessageBox cMsgBox((CWnd *)this, pGamePalette, "Your 4 rounds", "are over.");
						PostMessage(WM_CLOSE);
					}
				}
			}
			break;                          // ... to force a repaint

		// the Deal button is hit
		case IDC_DEAL:

			m_bMiddleOfHand = true;
			// make sure that the Dealt array is reset
			for (nLoop1 = 0; nLoop1 < 10; ++nLoop1) {
				aDealtArray[nLoop1][0] = 0;
				aDealtArray[nLoop1][1] = 0;
			}

			// for all 5 cards shown deal a new card
			for (nLoop1 = 0; nLoop1 < 5; ++nLoop1)
				ShowNewCard(DealNewCard(), nLoop1);

			// enable the Draw, and Hold buttons
			pDrawButton->EnableWindow(true);
			for (nHoldLoop = 0; nHoldLoop < 5; ++nHoldLoop)
				apHold[nHoldLoop]->EnableWindow(true);

			// disable all other buttons
			pDealButton->EnableWindow(false);

			apBet[0]->EnableWindow(false);
			apBet[1]->EnableWindow(false);
			apBet[2]->EnableWindow(false);
			apBet[3]->EnableWindow(false);
			apBet[4]->EnableWindow(false);
			apBet[5]->EnableWindow(false);
			apBet[6]->EnableWindow(false);

			pClearBet->EnableWindow(false);
			break;

		// if the Bet buttons are hit then set the bet accordingly
		case IDC_BET1:
			SetBet(1);
			pDealButton->EnableWindow(true);
			break;
		case IDC_BET5:
			SetBet(5);
			pDealButton->EnableWindow(true);
			break;
		case IDC_BET10:
			SetBet(10);
			pDealButton->EnableWindow(true);
			break;
		case IDC_BET25:
			SetBet(25);
			pDealButton->EnableWindow(true);
			break;
		case IDC_BET100:
			SetBet(100);
			pDealButton->EnableWindow(true);
			break;
		case IDC_BET1000:
			SetBet(1000);
			pDealButton->EnableWindow(true);
			break;
		case IDC_BETALL:
			SetBet(min(m_lUserAmount, (long)MAXBET));
			pDealButton->EnableWindow(true);
			break;
		case IDC_CLEARBET:
			if (m_lpGameStruct->bSoundEffectsEnabled) {
				sndPlaySound(nullptr, 0);
				sndPlaySound(WAV_CLEAR, SND_ASYNC);
			}
			SetBet(-1);
			pDealButton->EnableWindow(false);
			break;
		}


	(*this).SetFocus();                         // Reset focus back to the main window
	return true;
}

void CMainPokerWindow::OnRButtonDown(unsigned int nFlags, CPoint point) {
	CSetPayoffsDlg  dlgPayOff((CWnd *)this, pGamePalette, IDD_WINRATIO, nPayOff, true);

	dlgPayOff.DoModal();

	CWnd::OnLButtonDown(nFlags, point);
}

void CMainPokerWindow::OnLButtonDown(unsigned int nFlags, CPoint point) {
	CDC     *pDC;
	CRect   rClock,
	        rBoom,
	        rPencil,
	        rChair,
	        rWindow,
	        rLight;
	CSprite *pSprite = nullptr;
	CSound  *pEffect = nullptr;
	bool    bSuccess;
	int     i;


	rClock.SetRect(CLOCK_X, CLOCK_Y, CLOCK_X + CLOCK_DX, CLOCK_Y + CLOCK_DY);
	rBoom.SetRect(BOOM_X, BOOM_Y, BOOM_X + BOOM_DX, BOOM_Y + BOOM_DY);
	rPencil.SetRect(PENCIL_X, PENCIL_Y, PENCIL_X + PENCIL_DX, PENCIL_Y + PENCIL_DY);
	rChair.SetRect(CHAIR_X, CHAIR_Y, CHAIR_X + CHAIR_DX, CHAIR_Y + CHAIR_DY);
	rWindow.SetRect(WINDOW_X, WINDOW_Y, WINDOW_X + WINDOW_DX, WINDOW_Y + WINDOW_DY);
	rLight.SetRect(LIGHT_X, LIGHT_Y, LIGHT_X + LIGHT_DX, LIGHT_Y + LIGHT_DY);

	pDC = GetDC();

	if (NewGameRect.PtInRect(point) && !m_lpGameStruct->bPlayingMetagame)  {
		SetCapture();
		m_bMouseCaptured = true;
	} else if (rClock.PtInRect(point))  {
		CSound::waitWaveSounds();
		sndPlaySound(nullptr, 0);
		PlayEasterEgg(pDC, (CWnd *)this, pGamePalette, CLOCK_ANIM, WAV_CLOCK, CLOCK_FRAMES,
		              CLOCK_X, CLOCK_Y, CLOCK_SLEEP, m_bPlaySounds);
	} else if (rBoom.PtInRect(point))  {
		pSprite = new CSprite;
		(*pSprite).SharePalette(pGamePalette);
		bSuccess = (*pSprite).LoadCels(pDC, BOOM_ANIM, BOOM_FRAMES);
		if (!bSuccess) {
			delete pSprite;
			return;
		}
		(*pSprite).SetMasked(false);
		(*pSprite).SetMobile(false);

		CSound::waitWaveSounds();
		sndPlaySound(nullptr, 0);

		if (m_bPlaySounds) {
			pEffect = new CSound((CWnd *)this, WAV_BOOM,                                 // Load up the sound file as a
			                     SOUND_WAVE | SOUND_ASYNCH | SOUND_QUEUE | SOUND_AUTODELETE);    //...Wave file, to delete itself
		}
		if (pEffect != nullptr) {
			bSuccess = (*pEffect).play();
			if (!bSuccess)
				delete pEffect;
		}
		(*pSprite).SetCel(BOOM_FRAMES);
		for (i = 0; i < 13; i++) {
			(*pSprite).PaintSprite(pDC, BOOM_X, BOOM_Y);
			Sleep(BOOM_SLEEP);
		}

		if (m_bPlaySounds) {
			pEffect = new CSound((CWnd *)this, WAV_STAR,                                 // Load up the sound file as a
			                     SOUND_WAVE | SOUND_ASYNCH | SOUND_QUEUE | SOUND_AUTODELETE);    //...Wave file, to delete itself
		}
		if (pEffect != nullptr) {
			bSuccess = (*pEffect).play();
			if (!bSuccess)
				delete pEffect;
		}
//		(*pSprite).SetCel( nNumCels );
		while (i < BOOM_FRAMES) {
			(*pSprite).PaintSprite(pDC, BOOM_X, BOOM_Y);
			Sleep(BOOM_SLEEP);
			i++;
		}

		delete pSprite;

	} else if (rPencil.PtInRect(point) && m_lpGameStruct->bSoundEffectsEnabled)  {
		CSound::waitWaveSounds();
		sndPlaySound(nullptr, 0);
		if (m_bPlaySounds) {
			pEffect = new CSound((CWnd *)this, WAV_PENCIL,                               // Load up the sound file as a
			                     SOUND_WAVE | SOUND_ASYNCH | SOUND_QUEUE | SOUND_AUTODELETE);    //...Wave file, to delete itself
		}
		if (pEffect != nullptr) {
			bSuccess = (*pEffect).play();
			if (!bSuccess)
				delete pEffect;
		}
//		sndPlaySound( WAV_PENCIL, SND_ASYNC );
	} else if (rChair.PtInRect(point) && m_lpGameStruct->bSoundEffectsEnabled)  {
		CSound::waitWaveSounds();
		sndPlaySound(nullptr, 0);
		if (m_bPlaySounds) {
			pEffect = new CSound((CWnd *)this, WAV_CHAIR,                                // Load up the sound file as a
			                     SOUND_WAVE | SOUND_ASYNCH | SOUND_QUEUE | SOUND_AUTODELETE);    //...Wave file, to delete itself
		}
		if (pEffect != nullptr) {
			bSuccess = (*pEffect).play();
			if (!bSuccess)
				delete pEffect;
		}
//		sndPlaySound( WAV_CHAIR, SND_ASYNC );
	} else if (rWindow.PtInRect(point) && m_lpGameStruct->bSoundEffectsEnabled)  {
		CSound::waitWaveSounds();
		sndPlaySound(nullptr, 0);
		if (m_bPlaySounds) {
			pEffect = new CSound((CWnd *)this, WAV_WINDOW,                               // Load up the sound file as a
			                     SOUND_WAVE | SOUND_ASYNCH | SOUND_QUEUE | SOUND_AUTODELETE);    //...Wave file, to delete itself
		}
		if (pEffect != nullptr) {
			bSuccess = (*pEffect).play();
			if (!bSuccess)
				delete pEffect;
		}
//		sndPlaySound( WAV_WINDOW, SND_ASYNC );
	} else if (rLight.PtInRect(point) && m_lpGameStruct->bSoundEffectsEnabled)  {
		CSound::waitWaveSounds();
		sndPlaySound(nullptr, 0);
		if (m_bPlaySounds) {
			pEffect = new CSound((CWnd *)this, WAV_LIGHT,                                // Load up the sound file as a
			                     SOUND_WAVE | SOUND_ASYNCH | SOUND_QUEUE | SOUND_AUTODELETE);    //...Wave file, to delete itself
		}
		if (pEffect != nullptr) {
			bSuccess = (*pEffect).play();
			if (!bSuccess)
				delete pEffect;
		}
//		sndPlaySound( WAV_LIGHT, SND_ASYNC );
	}

	ReleaseDC(pDC);

	CWnd::OnLButtonDown(nFlags, point);
}

void CMainPokerWindow::OnLButtonUp(unsigned int nFlags, CPoint point) {
	if (m_bMouseCaptured) {
		m_bMouseCaptured = false;
		ReleaseCapture();
		if (NewGameRect.PtInRect(point) && !m_lpGameStruct->bPlayingMetagame)
			ResetGame(m_lStartingAmount);
	}
	CWnd::OnLButtonUp(nFlags, point);
}

void CMainPokerWindow::OnKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	CRules  dlgRules((CWnd *)this, RULESFILE, pGamePalette,
	                 (m_lpGameStruct->bSoundEffectsEnabled ? RULES_WAV : nullptr));

	if (nChar == VK_F1) {
		pOptionButton->ShowWindow(SW_HIDE);
//		UpdateWindow();
		dlgRules.DoModal();      // invoke the help dialog box
		pOptionButton->ShowWindow(SW_SHOWNORMAL);
		return;
	} else {
		if (nChar == VK_F2) {
			SendMessage(WM_COMMAND, IDC_OPTION, BN_CLICKED);
			return;
		}
	}
	CFrameWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}
/*****************************************************************
 *
 * SetPayOffs
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *          Sets the PayOff Ratios
 *
 * FORMAL PARAMETERS:
 *
 *      nPayOffType = what set of payoffs to use, this corresponds
 *                                      to exactly the same set of odds as displayed
 *                                      in the "Set Payoffs" dialog
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *    m_nPayOffPair
 *      m_nPayOffPairJackorHigher
 *      m_nPayOffTwoPair
 *      m_nPayOffThreeofaKind
 *      m_nPayOffStraight
 *      m_nPayOffFlush
 *      m_nPayOffFullHouse
 *      m_nPayOffFourofaKind
 *      m_nPayOffStraightFlush
 *      m_nPayOffRoyalFlush
 *
 * RETURN VALUE:
 *
 *      n/a
 *
 ****************************************************************/
void CMainPokerWindow::SetPayOffs(int nPayOffType) {

//  set the payoffs according to what id was is sent in
	switch (nPayOffType) {
	case IDC_KLINGON:
		m_nPayOffPair = 0;
		m_nPayOffPairJackorHigher = 0;
		m_nPayOffTwoPair = 1;
		m_nPayOffThreeofaKind = 2;
		m_nPayOffStraight = 3;
		m_nPayOffFlush = 4;
		m_nPayOffFullHouse = 5;
		m_nPayOffFourofaKind = 10;
		m_nPayOffStraightFlush = 20;
		m_nPayOffRoyalFlush = 50;
		break;

	case IDC_VEGAS:
		m_nPayOffPair = 0;
		m_nPayOffPairJackorHigher = 1;
		m_nPayOffTwoPair = 2;
		m_nPayOffThreeofaKind = 3;
		m_nPayOffStraight = 4;
		m_nPayOffFlush = 6;
		m_nPayOffFullHouse = 9;
		m_nPayOffFourofaKind = 25;
		m_nPayOffStraightFlush = 50;
		m_nPayOffRoyalFlush = 250;
		break;

	case IDC_POPO:
		m_nPayOffPair = 1;
		m_nPayOffPairJackorHigher = 2;
		m_nPayOffTwoPair = 3;
		m_nPayOffThreeofaKind = 5;
		m_nPayOffStraight = 8;
		m_nPayOffFlush = 10;
		m_nPayOffFullHouse = 15;
		m_nPayOffFourofaKind = 50;
		m_nPayOffStraightFlush = 100;
		m_nPayOffRoyalFlush = 500;
		break;

	case IDC_KUWAITI:
		m_nPayOffPair = 2;
		m_nPayOffPairJackorHigher = 3;
		m_nPayOffTwoPair = 5;
		m_nPayOffThreeofaKind = 10;
		m_nPayOffStraight = 15;
		m_nPayOffFlush = 20;
		m_nPayOffFullHouse = 50;
		m_nPayOffFourofaKind = 100;
		m_nPayOffStraightFlush = 250;
		m_nPayOffRoyalFlush = 1000;
		break;

	case IDC_MARTIAN:
		m_nPayOffPair = 2;
		m_nPayOffPairJackorHigher = 2;
		m_nPayOffTwoPair = 13;
		m_nPayOffThreeofaKind = 3;
		m_nPayOffStraight = 89;
		m_nPayOffFlush = 98;
		m_nPayOffFullHouse = 177;
		m_nPayOffFourofaKind = 4;
		m_nPayOffStraightFlush = 23;
		m_nPayOffRoyalFlush = 11;
		break;
	}
	return;
}
/*****************************************************************
 *
 * CheckWinningHand
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *          Checks the hand to see if it won
 *
 * FORMAL PARAMETERS:
 *
 *      n/a
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      UserBet, UserAmount, aDealtArray
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      UserBet, UserAmount
 *
 * RETURN VALUE:
 *
 *      n/a
 *
 ****************************************************************/
void CMainPokerWindow::CheckWinningHand() {
	int     anShownCards[5];        // an array to contain the currently shown cards
	int     nSortSwap;          // used in the sorting array to swap to numbers
	int     nPairMatch;                 // used to keep track of the number matches found,
	//when looking for a pair, two pair, .. four of a kind
	int     nIndex, nLoop;      // looping variables
	int     nCard = 0;          // card in a pair, used to tell if a pair is Jacks or higher
	CRect   potRect;  // a rect of screen coordinates
	// that contain the pot and user amounts
	CUserWonDlg dlgWon((CWnd *)this, pGamePalette);

// find out what cards are currently shown
	for (nLoop = 0; nLoop < 5; nLoop++) {
		anShownCards[nLoop] = 0;
	}

	for (nLoop = 0, nIndex = 0; nLoop <= 9; nLoop++, nIndex++) {
		if (aDealtArray[nLoop][1] == 1)
			anShownCards[nIndex] = aDealtArray[nLoop][0];
		else
			nIndex--;
	}


// sort anShownCards
	for (nLoop = 0; nLoop < 5; nLoop++) {
		for (nIndex = 1; nIndex < 5; nIndex++) {

			if (anShownCards[nIndex] < anShownCards[nIndex - 1]) {
				nSortSwap = anShownCards[nIndex];
				anShownCards[nIndex] = anShownCards[nIndex - 1];
				anShownCards[nIndex - 1] = nSortSwap;
			}
		}
	}

// check royal flush
	if (((Mod(anShownCards[1], 13)) == 10) &&
	        (anShownCards[4] == anShownCards[1] + 3) &&
	        (anShownCards[0] == anShownCards[1] - 9)) {
		if (m_bPlaySounds) {
			sndPlaySound(nullptr, 0);
			sndPlaySound(WAV_PAY, SND_ASYNC);
		}

//      MessageBox("You got a Royal Flush", nullptr, MB_OK);
		dlgWon.SetInitialOptions(10);
		dlgWon.DoModal();

		sndPlaySound(nullptr, 0);
//	Check for overflow, if so then the user broken the bank, reset to zero
		if (m_lUserAmount > m_lUserAmount + m_lUserBet * m_nPayOffRoyalFlush) {
			m_lUserAmount = 2147483647;
			potRect.SetRect(POT_AMOUNT_X, POT_AMOUNT_Y, POT_AMOUNT_X + 137, POT_AMOUNT_Y + 101);
			InvalidateRect(&potRect, true);
			MessageBox("Congratulations! You just broke the bank", "YOU WIN!", MB_OK);
			m_lUserAmount = 0L;
		} else
			m_lUserAmount += m_lUserBet * m_nPayOffRoyalFlush;
		return;
	}

// check straight flush
	if (anShownCards[4] == anShownCards[0] + 4) {

		if (m_bPlaySounds) {
			sndPlaySound(nullptr, 0);
			sndPlaySound(WAV_PAY, SND_ASYNC);
		}
//      MessageBox("You got a Straight Flush", nullptr, MB_OK);
		dlgWon.SetInitialOptions(9);
		dlgWon.DoModal();
		sndPlaySound(nullptr, 0);
//	Check for overflow, if so then the user broken the bank, reset to zero
		if (m_lUserAmount > m_lUserAmount + m_lUserBet * m_nPayOffRoyalFlush) {
			m_lUserAmount = 2147483647;
			potRect.SetRect(POT_AMOUNT_X, POT_AMOUNT_Y, POT_AMOUNT_X + 137, POT_AMOUNT_Y + 101);
			InvalidateRect(&potRect, true);
			MessageBox("Congratulations! You just broke the bank", "YOU WIN!", MB_OK);
			m_lUserAmount = 0L;
		} else
			m_lUserAmount += m_lUserBet * m_nPayOffStraightFlush;
		return;
	}

// check for these, four of a kind, full house, three of a kind, two pair, one pair
	nPairMatch = 0;
	for (nLoop = 0; nLoop < 4; nLoop++) {
		for (nIndex = nLoop + 1; nIndex < 5; nIndex++) {
			if (Mod(anShownCards[nLoop], 13) == Mod(anShownCards[nIndex], 13)) {
				nCard = Mod(anShownCards[nLoop], 13);
				nPairMatch++;
			}
		}
	}
// nPairMatch = 6 : four of a kind
// nPairMatch = 4 : full house
// nPairMatch = 3 : three of a kind
// nPairMatch = 2 : two pair
// nPairMatch = 1 : one pair
	if (nPairMatch == 0)
		nCard = 0;
	if (nPairMatch == 6) {

		if (m_bPlaySounds) {
			sndPlaySound(nullptr, 0);
			sndPlaySound(WAV_PAY, SND_ASYNC);
		}
//      MessageBox("You got a Four of a Kind", nullptr, MB_OK);
		dlgWon.SetInitialOptions(8);
		dlgWon.DoModal();
		sndPlaySound(nullptr, 0);
//	Check for overflow, if so then the user broken the bank, reset to zero
		if (m_lUserAmount > m_lUserAmount + m_lUserBet * m_nPayOffRoyalFlush) {
			m_lUserAmount = 2147483647;
			potRect.SetRect(POT_AMOUNT_X, POT_AMOUNT_Y, POT_AMOUNT_X + 137, POT_AMOUNT_Y + 101);
			InvalidateRect(&potRect, true);
			MessageBox("Congratulations! You just broke the bank", "YOU WIN!", MB_OK);
			m_lUserAmount = 0L;
		} else
			m_lUserAmount += m_lUserBet * m_nPayOffFourofaKind;
		return;
	}
	if (nPairMatch == 4) {

		if (m_bPlaySounds) {
			sndPlaySound(nullptr, 0);
			sndPlaySound(WAV_PAY, SND_ASYNC);
		}
//      MessageBox("You got a Full House", nullptr, MB_OK);
		dlgWon.SetInitialOptions(7);
		dlgWon.DoModal();
		sndPlaySound(nullptr, 0);
//	Check for overflow, if so then the user broken the bank, reset to zero
		if (m_lUserAmount > m_lUserAmount + m_lUserBet * m_nPayOffRoyalFlush) {
			m_lUserAmount = 2147483647;
			potRect.SetRect(POT_AMOUNT_X, POT_AMOUNT_Y, POT_AMOUNT_X + 137, POT_AMOUNT_Y + 101);
			InvalidateRect(&potRect, true);
			MessageBox("Congratulations! You just broke the bank", "YOU WIN!", MB_OK);
			m_lUserAmount = 0L;
		} else
			m_lUserAmount += m_lUserBet * m_nPayOffFullHouse;
		return;
	}
	if (nPairMatch == 3) {

		if (m_bPlaySounds) {
			sndPlaySound(nullptr, 0);
			sndPlaySound(WAV_PAY, SND_ASYNC);
		}
//      MessageBox("You got a Three of a Kind", nullptr, MB_OK);
		dlgWon.SetInitialOptions(4);
		dlgWon.DoModal();
		sndPlaySound(nullptr, 0);
//	Check for overflow, if so then the user broken the bank, reset to zero
		if (m_lUserAmount > m_lUserAmount + m_lUserBet * m_nPayOffRoyalFlush) {
			m_lUserAmount = 2147483647;
			potRect.SetRect(POT_AMOUNT_X, POT_AMOUNT_Y, POT_AMOUNT_X + 137, POT_AMOUNT_Y + 101);
			InvalidateRect(&potRect, true);
			MessageBox("Congratulations! You just broke the bank", "YOU WIN!", MB_OK);
			m_lUserAmount = 0L;
		} else {
			m_lUserAmount += m_lUserBet * m_nPayOffThreeofaKind;
		}

		return;
	}
	if (nPairMatch == 2) {

		if (m_bPlaySounds) {
			sndPlaySound(nullptr, 0);
			sndPlaySound(WAV_PAY, SND_ASYNC);
		}
//      MessageBox("You got a Two Pair", nullptr, MB_OK);
		dlgWon.SetInitialOptions(3);
		dlgWon.DoModal();
		sndPlaySound(nullptr, 0);
//	Check for overflow, if so then the user broken the bank, reset to zero
		if (m_lUserAmount > m_lUserAmount + m_lUserBet * m_nPayOffRoyalFlush) {
			m_lUserAmount = 2147483647;
			potRect.SetRect(POT_AMOUNT_X, POT_AMOUNT_Y, POT_AMOUNT_X + 137, POT_AMOUNT_Y + 101);
			InvalidateRect(&potRect, true);
			
			MessageBox("Congratulations! You just broke the bank", "YOU WIN!", MB_OK);
			m_lUserAmount = 0L;
		} else
			m_lUserAmount += m_lUserBet * m_nPayOffTwoPair;
		return;
	}
	if (nPairMatch == 1) {
		if ((nCard > 10) || (nCard == 1)) {

			if (m_nPayOffPairJackorHigher == 0) {
				if (m_bPlaySounds)
					sndPlaySound(WAV_NOPAY, SND_ASYNC);
				dlgWon.SetInitialOptions();
				dlgWon.DoModal();
				sndPlaySound(nullptr, 0);
			} else {
				if (m_bPlaySounds) {
					sndPlaySound(nullptr, 0);
					sndPlaySound(WAV_PAY, SND_ASYNC);
				}
//      MessageBox("You got a Pair ( Jacks or Higher", nullptr, MB_OK);
				dlgWon.SetInitialOptions(2);
				dlgWon.DoModal();
				sndPlaySound(nullptr, 0);
			}
//	Check for overflow, if so then the user broken the bank, reset to zero
			if (m_lUserAmount > m_lUserAmount + m_lUserBet * m_nPayOffRoyalFlush) {
				m_lUserAmount = 2147483647;
				potRect.SetRect(POT_AMOUNT_X, POT_AMOUNT_Y, POT_AMOUNT_X + 137, POT_AMOUNT_Y + 101);
				InvalidateRect(&potRect, true);
				
				MessageBox("Congratulations! You just broke the bank", "YOU WIN!", MB_OK);
				m_lUserAmount = 0L;
			} else
				m_lUserAmount += m_lUserBet * m_nPayOffPairJackorHigher;
		} else {
			if (m_nPayOffPair == 0) {
				if (m_bPlaySounds)
					sndPlaySound(WAV_NOPAY, SND_ASYNC);
				dlgWon.SetInitialOptions();
				dlgWon.DoModal();
				sndPlaySound(nullptr, 0);
			} else {
				if (m_bPlaySounds) {
					sndPlaySound(nullptr, 0);
					sndPlaySound(WAV_PAY, SND_ASYNC);
				}
//      MessageBox("You got a Pair", nullptr, MB_OK);
				dlgWon.SetInitialOptions(1);
				dlgWon.DoModal();
				sndPlaySound(nullptr, 0);
			}

//	Check for overflow, if so then the user broken the bank, reset to zero
			if (m_lUserAmount > m_lUserAmount + m_lUserBet * m_nPayOffRoyalFlush) {
				m_lUserAmount = 2147483647;
				potRect.SetRect(POT_AMOUNT_X, POT_AMOUNT_Y, POT_AMOUNT_X + 137, POT_AMOUNT_Y + 101);
				InvalidateRect(&potRect, true);
				
				MessageBox("Congratulations! You just broke the bank", "YOU WIN!", MB_OK);
				m_lUserAmount = 0L;
			} else
				m_lUserAmount += m_lUserBet * m_nPayOffPair;
		}
		return;
	}

// check for flush
	for (nLoop = 0; nLoop < 53; nLoop += 13) {
		if ((anShownCards[0] <= (13 + nLoop)) && (anShownCards[0] >= (nLoop)) &&
		        (anShownCards[4] <= (13 + nLoop)) && (anShownCards[4] >= (nLoop))) {

			if (m_bPlaySounds) {
				sndPlaySound(nullptr, 0);
				sndPlaySound(WAV_PAY, SND_ASYNC);
			}
//      MessageBox("You got a Flush", nullptr, MB_OK);
			dlgWon.SetInitialOptions(6);
			dlgWon.DoModal();
			sndPlaySound(nullptr, 0);
//	Check for overflow, if so then the user broken the bank, reset to zero
			if (m_lUserAmount > m_lUserAmount + m_lUserBet * m_nPayOffRoyalFlush) {
				m_lUserAmount = 2147483647;
				potRect.SetRect(POT_AMOUNT_X, POT_AMOUNT_Y, POT_AMOUNT_X + 137, POT_AMOUNT_Y + 101);
				InvalidateRect(&potRect, true);
				
				MessageBox("Congratulations! You just broke the bank", "YOU WIN!", MB_OK);
				m_lUserAmount = 0L;
			} else
				m_lUserAmount += m_lUserBet * m_nPayOffFlush;
			return;
		}
	}

// check for straight
	for (nLoop = 0; nLoop < 5; nLoop++)
		anShownCards[nLoop] = Mod(anShownCards[nLoop], 13);


// sort anShownCards
	for (nLoop = 0; nLoop < 5; nLoop++) {
		for (nIndex = 1; nIndex < 5; nIndex++) {

			if (anShownCards[nIndex] < anShownCards[nIndex - 1]) {
				nSortSwap = anShownCards[nIndex];
				anShownCards[nIndex] = anShownCards[nIndex - 1];
				anShownCards[nIndex - 1] = nSortSwap;
			}
		}
	}

	if ((anShownCards[4] == anShownCards[0] + 4) ||
	        ((anShownCards[0] == 1) &&
	         (anShownCards[4] == anShownCards[1] + 3) &&
	         (anShownCards[1] == 10))) {

		if (m_bPlaySounds) {
			sndPlaySound(nullptr, 0);
			sndPlaySound(WAV_PAY, SND_ASYNC);
		}
//      MessageBox("You got a Straight", nullptr, MB_OK);
		dlgWon.SetInitialOptions(5);
		dlgWon.DoModal();
		sndPlaySound(nullptr, 0);
//	Check for overflow, if so then the user broken the bank, reset to zero
		if (m_lUserAmount > m_lUserAmount + m_lUserBet * m_nPayOffRoyalFlush) {
			m_lUserAmount = 2147483647;
			potRect.SetRect(POT_AMOUNT_X, POT_AMOUNT_Y, POT_AMOUNT_X + 137, POT_AMOUNT_Y + 101);
			InvalidateRect(&potRect, true);
			
			MessageBox("Congratulations! You just broke the bank", "YOU WIN!", MB_OK);
			m_lUserAmount = 0L;
		} else
			m_lUserAmount += m_lUserBet * m_nPayOffStraight;
		return;
	}

	if (m_bPlaySounds)
		sndPlaySound(WAV_NOPAY, SND_ASYNC);
	dlgWon.SetInitialOptions();
	dlgWon.DoModal();

	sndPlaySound(nullptr, 0);
	

	return;
}
/*****************************************************************
 *
 * SetBet
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *          Sets the bet and invaldates the the display, so that the
 *          bet gets shown to the user
 *
 * FORMAL PARAMETERS:
 *
 *      size of the bet ( 1, 5, 10, 25, 100, 1000, min(ALL, 10000) )
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      UserBet, UserAmount
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      UserBet, UserAmount
 *
 * RETURN VALUE:
 *
 *      n/a
 *
 ****************************************************************/
void CMainPokerWindow::SetBet(long lBet) {
	CRect   potRect;  // a pointer to the rect of screen coordinates
	// that contain the pot and user amounts

// if lBet = 0, then reset the bet to zero
	if (lBet == 0) {
		m_lUserBet = 0;
		m_lUserAmount -= lBet;
	} else {
// if lBet > 0, then add it to the current bet
		if (lBet > 0) {
			if (m_lpGameStruct->bSoundEffectsEnabled) {
				sndPlaySound(nullptr, 0);
				sndPlaySound(WAV_PLACE, SND_ASYNC);
			}
//      if lBet > 10000, then max bet at 10000
			if ((m_lUserBet + lBet) > MAXBET) {
				lBet = MAXBET - m_lUserBet;
				m_lUserBet = MAXBET;
				m_lUserAmount -= lBet;
			} else {
				m_lUserBet += lBet;
				m_lUserAmount -= lBet;
			}
		} else {
			m_lUserAmount += m_lUserBet;
			m_lUserBet = 0;
		}
	}

// redraw the pot and user amounts
	potRect.SetRect(POT_AMOUNT_X, POT_AMOUNT_Y, POT_AMOUNT_X + 137, POT_AMOUNT_Y + 101);
	InvalidateRect(&potRect, true);
	

// adjust the bet buttons
	EnableBets();
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

bool CMainPokerWindow::OnEraseBkgnd(CDC *pDC) {
// eat this
	return true;
}

void CMainPokerWindow::OnActivate(unsigned int nState, CWnd *pWndOther, bool bMinimized) {
	if (!bMinimized)
		switch (nState) {
		case WA_ACTIVE:
		case WA_CLICKACTIVE:
			InvalidateRect(nullptr, false);
			break;
		}
	return;
}
/*****************************************************************
 *
 * SetHoldList
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *
 * FORMAL PARAMETERS:
 *
 *      index   =   index in the Hold list, and of the button
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      abHoldArray
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      abHoldArray
 *
 * RETURN VALUE:
 *
 *          n/a
 *
 ****************************************************************/
void CMainPokerWindow::SetHoldList(int nIndex) {
	bool    bTestLoadBmp = false; // bool for testing to make sure that bitmaps were loaded
	CDC         *pDC = GetDC();
	CPalette    *pOldPal = pDC->SelectPalette(pGamePalette, false);

	pDC->RealizePalette();

	if (abHoldArray[nIndex]) {
		abHoldArray[nIndex] = false;
		pUpBmp = ExtractBitmap(pDC, pHoldButtons, pGamePalette,
		                       (nIndex * HOLD_BMP_GRID_WITDH), HOLD_BMP_GRID_TOP2,
		                       arHoldRect[nIndex].right - arHoldRect[nIndex].left,
		                       arHoldRect[nIndex].bottom - arHoldRect[nIndex].top);
	} else {
		if (m_lpGameStruct->bSoundEffectsEnabled) {
			sndPlaySound(nullptr, SND_ASYNC);
			sndPlaySound(WAV_HOLD, SND_ASYNC);
		}
		abHoldArray[nIndex] = true;
		pUpBmp = ExtractBitmap(pDC, pHoldButtons, pGamePalette,
		                       (nIndex * HOLD_BMP_GRID_WITDH), HOLD_BMP_GRID_TOP1,
		                       arHoldRect[nIndex].right - arHoldRect[nIndex].left,
		                       arHoldRect[nIndex].bottom - arHoldRect[nIndex].top);
	}

	pDnBmp = ExtractBitmap(pDC, pHoldButtons, pGamePalette,
	                       (nIndex * HOLD_BMP_GRID_WITDH), HOLD_BMP_GRID_TOP3,
	                       arHoldRect[nIndex].right - arHoldRect[nIndex].left,
	                       arHoldRect[nIndex].bottom - arHoldRect[nIndex].top);
	pDsBmp = ExtractBitmap(pDC, pHoldButtons, pGamePalette,
	                       (nIndex * HOLD_BMP_GRID_WITDH), HOLD_BMP_GRID_TOP4,
	                       arHoldRect[nIndex].right - arHoldRect[nIndex].left,
	                       arHoldRect[nIndex].bottom - arHoldRect[nIndex].top);

	bTestLoadBmp = apHold[nIndex]->LoadBitmaps(pGamePalette, pUpBmp, pDnBmp, nullptr, pDsBmp);
	ASSERT(bTestLoadBmp);                 // test for button's bitmap loading

	pUpBmp = nullptr;
	pDnBmp = nullptr;
	pDsBmp = nullptr;

	pDC->SelectPalette(pOldPal, false);
	ReleaseDC(pDC);
}

/*****************************************************************
 *
 * DealNewCard
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *          Using a random number generator, select a number (card)
 *          from 1-52. Then check to make sure that this number (card)
 *          has not already been dealt. If not, then add it to bDealtArray
 *          and return it.
 *
 * FORMAL PARAMETERS:
 *
 *      None
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      bDealtArray
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      bDealtArray
 *
 * RETURN VALUE:
 *
 *      The value [1-52] of the new card select (and added to the
 *          dealt array)
 *
 ****************************************************************/
int CMainPokerWindow::DealNewCard() {

	int     nNewCard;           // contains the new card number
	int     nCounter;     // loop counter
	bool    bDealAgain;   // bool to test that the card has not already been dealt

// generate a new number 1-52
	do {
		bDealAgain = false;
		nNewCard = (brand() % 52) + 1;
		for (nCounter = 0; nCounter < 9; ++nCounter) {
			if (aDealtArray[nCounter][0] == nNewCard)
				bDealAgain = true;
		}
// until we get one that hs not been dealt yet
	} while (bDealAgain);

// find the last entry into the dealt list
	nCounter = 0;
	while ((nCounter < 9) && (aDealtArray[nCounter][0] != 0))
		++nCounter;

// put this new card into it
	aDealtArray[nCounter][0] = nNewCard;
	aDealtArray[nCounter][1] = 1;

// return the new card dealt
	return nNewCard;
}

/*****************************************************************
 *
 * ShowNewCard
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *          Show the card inputted in the position inputted, by setting the
 *          that cardbutton bitmaps to the approriate card.
 *
 * FORMAL PARAMETERS:
 *
 *      card        = the card number (1-52) to be displayed
 *          cardPos = the position to display the new card
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      apCard = the array of card buttons
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
void CMainPokerWindow::ShowNewCard(int nCard, int nCardPos)    {
	CPoint  ptTemp(arCardRect[nCardPos].left + 1, arCardRect[nCardPos].top + 1);
	bool    bTestCreate = false;
	CDC     *pDC = GetDC();

	pDC = GetDC();
	if (apCard[nCardPos] != nullptr) {
		apCard[nCardPos]->EraseSprite(pDC);
		delete apCard[nCardPos];
		apCard[nCardPos] = nullptr;
	}
	apCard[nCardPos] = new CSprite;
	apCard[nCardPos]->SharePalette(pGamePalette);
	bTestCreate = apCard[nCardPos]->LoadSprite(pDC, sBitmaps[nCard - 1]);
	ASSERT(bTestCreate);                    // test for sprite's creation
	apCard[nCardPos]->SetMasked(false);
	apCard[nCardPos]->SetMobile(false);
	apCard[nCardPos]->SetOptimizeSpeed(true);
	ReleaseDC(pDC);

	apCard[nCardPos]->PaintSprite(pDC, ptTemp.x, ptTemp.y);

// if i'm playing sound, play a card dealt sound
	if (m_bPlaySounds)
		sndPlaySound(CARDSOUND, SND_SYNC);
	return ;
}
/*****************************************************************
 *
 * OnDrawItem
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *          OnDrawItem is the OnPaint for my controls. I capture the
 *          controls I specifically want to draw myself, and send the
 *          rest to their default paints.
 *
 * FORMAL PARAMETERS:
 *
 *      itemType = identifier of the control sending the message
 *
 *          lpdiDrawItemStruct = sent by windows it contains everything
 *          I need to draw like the HDC, the control ID, etc. For exact
 *          details see SDK on DRAWITEMSTRUCT.
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

void CMainPokerWindow::OnDrawItem(int itemType, LPDRAWITEMSTRUCT lpdiDrawItemStruct) {

	CWnd::OnDrawItem(itemType, lpdiDrawItemStruct);
	return;
}

/*
****************************************************************
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

void CMainPokerWindow::OnClose() {
	CDC *pDC = GetDC();
	CRect       rctFillRect(0, 0, 640, 480);
	CBrush  Brush(RGB(0, 0, 0));

	pDC->FillRect(&rctFillRect, &Brush);
	ReleaseResources();
	ReleaseDC(pDC);
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
void CMainPokerWindow::OnDestroy() {
//  send a message to the calling app to tell it the user has quit the game
	m_lpGameStruct->lCrowns = m_lUserAmount;
	MFC::PostMessage(m_hCallAppWnd, WM_PARENTNOTIFY, WM_DESTROY, (LPARAM)m_lpGameStruct);
//	m_lpGameStruct = nullptr;
//	delete m_lpGameStruct;
	CFrameWnd::OnDestroy();
}

/*****************************************************************
 *
 * Mod
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *          This is a modified % that takes moves the 0 mod to the divisor
 *
 * FORMAL PARAMETERS:
 *
 *      Dividend, Divisor
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
int CMainPokerWindow::Mod(int Dividend, int Divisor) {
	int returnvalue = 0;                                    // is the remainder of the mod

// do the standard modulo
	returnvalue = Dividend % Divisor;

// but if there is no remainder, then set the remainder to the divisor
	if (returnvalue == 0)
		returnvalue = Divisor;
	return returnvalue;
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

void CMainPokerWindow::ReleaseResources() {
	int i;

	if (pGameSound != nullptr) {
		delete pGameSound;                      // delete the game theme song
		pGameSound = nullptr;
	}

	for (i = 0; i < 7; i++) {
		if (apBet[i] != nullptr)
			delete apBet[i];
	}

	if (pUpBmp != nullptr)
		delete pUpBmp;

	if (pDnBmp != nullptr)
		delete pDnBmp;

	if (pDsBmp != nullptr)
		delete pDsBmp;

	if (pHoldButtons != nullptr)
		delete pHoldButtons;

	if (pBetButtons != nullptr)
		delete pBetButtons;

	delete pOptionButton;                                       // release the buttons we used
	delete pClearBet;
	delete pDrawButton;
	delete pDealButton;

	for (i = 0; i < 5; i++) {
		if (apCard[i] != nullptr)
			delete apCard[i];
		if (apHold[i] != nullptr)
			delete apHold[i];
	}

	(*pGamePalette).DeleteObject();         // release the game color palette
	delete pGamePalette;

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

void CMainPokerWindow::FlushInputEvents() {
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

LRESULT CMainPokerWindow::OnMCINotify(WPARAM wParam, LPARAM lParam) {
	CSound  *pSound;

	pSound = CSound::OnMCIStopped(wParam, lParam);
	if (pSound != nullptr)
		OnSoundNotify(pSound);
	return 0;
}


LRESULT CMainPokerWindow::OnMMIONotify(WPARAM wParam, LPARAM lParam) {
	CSound  *pSound;

	pSound = CSound::OnMMIOStopped(wParam, lParam);
	if (pSound != nullptr)
		OnSoundNotify(pSound);
	return 0;
}

void CMainPokerWindow::OnSoundNotify(CSound *pSound) {
	//
	// Add your code to process explicit notification of a sound "done" event here.
	// pSound is a pointer to a CSound object for which you requested SOUND_NOTIFY.
	//
}

// CMainPokerWindow message map:
// Associate messages with member functions.
//
BEGIN_MESSAGE_MAP(CMainPokerWindow, CFrameWnd)
	//{{AFX_MSG_MAP( CMainPokerWindow )
	ON_WM_PAINT()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_DRAWITEM()
	ON_WM_ERASEBKGND()
	ON_WM_KEYDOWN()
	ON_WM_ACTIVATE()
	ON_MESSAGE(MM_MCINOTIFY, CMainPokerWindow::OnMCINotify)
	ON_MESSAGE(MM_WOM_DONE, CMainPokerWindow::OnMMIONotify)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//
// Routine to play easter egg animations and sounds
void PlayEasterEgg(CDC *pDC, CWnd *pWnd, CPalette *pPalette,
                   const char *pszAnimFile, const char *pszSoundFile,
                   int nNumCels, int nXLoc, int nYLoc, int nSleep, bool bPlaySound) {
	CSprite *pSprite = nullptr;
	CSound  *pEffect = nullptr;
	bool    bSuccess;
	int     i;

	pSprite = new CSprite;
	(*pSprite).SharePalette(pPalette);
	bSuccess = (*pSprite).LoadCels(pDC, pszAnimFile, nNumCels);
	if (!bSuccess) {
		delete pSprite;
		return;
	}
	(*pSprite).SetMasked(false);
	(*pSprite).SetMobile(false);

	if (bPlaySound) {
		pEffect = new CSound(pWnd, pszSoundFile,                                 // Load up the sound file as a
		                     SOUND_WAVE | SOUND_ASYNCH | SOUND_QUEUE | SOUND_AUTODELETE);    //...Wave file, to delete itself
	}
	if (pEffect != nullptr) {
		bSuccess = (*pEffect).play();
		if (!bSuccess)
			delete pEffect;
	}
	(*pSprite).SetCel(nNumCels);
	for (i = 0; i < nNumCels; i++) {
		(*pSprite).PaintSprite(pDC, nXLoc, nYLoc);
		Sleep(nSleep);
	}

	delete pSprite;
}

} // namespace Poker
} // namespace HodjNPodj
} // namespace Bagel
