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
#include "bagel/hodjnpodj/hnplibs/sprite.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/hnplibs/bitmaps.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/hodjnpodj/hnplibs/mainmenu.h"
#include "bagel/hodjnpodj/hnplibs/cmessbox.h"
#include "bagel/hodjnpodj/hnplibs/rules.h"
#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/life/resource.h"
#include "bagel/hodjnpodj/life/game.h"
#include "bagel/hodjnpodj/life/life.h"
#include "bagel/hodjnpodj/life/usercfg.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {
namespace Life {

#define RULES_TXT   "life.txt"                  // rules text file
#define RULES_WAV   ".\\sound\\rllf.wav"        // rules narration
#define MIDI_SOUND  ".\\SOUND\\life.mid"        // background music

//
// Win/Lose conditions
//
#define WIN             1
#define LOSE            0

// Globals!
int         nSpeed,                 // Speed between evolutions
            nCountDown,             // Counts before an evolution
            nLife,                  // Num of lives given at game start
            nPlace;                 // Profile holder for PrePlace bool
int         nTurnCounter,           // Counts num of years before end of game
            nLifeCounter;           // Counts num of lives left to place
bool        bPrePlaceColonies,      // Whether they want to pre-place some (default for meta)
            bIsInfiniteLife,        // Tells if num of lives is set to infi
            bIsInfiniteTurns;       // Tells if num of turns is set to infi

bool        gbNewGame;              // only used from options dialog

CColorButton    *pEvolveButton;

extern      CMainWindow *gMainWnd;  // Allows me to pass main window New_Game
// messages from GameOver.cpp

CPalette    *pGamePalette = nullptr;   // Used everywhere for ref
CSprite     *pScrollSprite = nullptr;  // Commands button
CSprite     *pCalendarSprite = nullptr;    // Blank calendar
HWND        hWndParent;

void CALLBACK GetGameParams(CWnd *pParentWnd);

/*****************************************************************
 *
 * CMainWindow
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * CMainWindow constructor:
 * Create the window with the appropriate style, size, menu, etc.;
 * it will be later revealed by CTheApp::InitInstance().  Then
 * create our splash screen object by opening and loading its DIB
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
CMainWindow::CMainWindow(HWND hParentWnd, LPGAMESTRUCT lpGameInfo) {
	CString WndClass;
	CDC     *pDC;
	CRect   MainRect,
	        QuitRect,
	        CommandRect,
	        OkayRect,
	        EvolveRect;
	CDibDoc *pDibDoc;           // Used to get game palette
	bool    bAssertCheck;       // Assertion check when getting game pal


	/*******************************************
	* Handle the meta-game structure settings. *
	*******************************************/

	m_lpGameStruct = lpGameInfo;

	initStatics();

	/********************************
	* Set the global window handle. *
	********************************/
	m_hCallAppWnd = hParentWnd;

	/********************************************************************
	* Get palette from background art that will be used for all related *
	* game art work.                                                    *
	********************************************************************/
	pDibDoc = new CDibDoc();
	bAssertCheck = (*pDibDoc).OpenDocument(SPLASHSPEC);
	ASSERT(bAssertCheck);

	pGamePalette = (*pDibDoc).DetachPalette();
	ASSERT(pGamePalette);
	delete pDibDoc;

	/********************************************************************
	* Define a special window class which traps double-clicks, is byte  *
	* aligned to maximize BITBLT performance, and creates "owned" DCs   *
	* rather than sharing the five system defined DCs which are not     *
	* guaranteed to be available; this adds a bit to our app size but   *
	* avoids hangs/freezes/lockups.                                     *
	********************************************************************/
	WndClass = AfxRegisterWndClass(CS_DBLCLKS | CS_BYTEALIGNWINDOW | CS_OWNDC,
	                               LoadCursor(nullptr, IDC_ARROW),    //put in my own cursor
	                               nullptr,
	                               nullptr);

	/***********************************
	* Center our window on the screen. *
	***********************************/
	pDC = GetDC();
	MainRect.left = (pDC->GetDeviceCaps(HORZRES) - GAME_WIDTH) >> 1;
	MainRect.top = (pDC->GetDeviceCaps(VERTRES) - GAME_HEIGHT) >> 1;
	MainRect.right = MainRect.left + GAME_WIDTH;
	MainRect.bottom = MainRect.top + GAME_HEIGHT;
	ReleaseDC(pDC);

	/********************************************************************
	* Create the window as a POPUP so no boarders, title, or menu are   *
	* present; this is because the game's background art will fill the  *
	* entire 640x40 area.                                               *
	********************************************************************/
	Create(WndClass, "Boffo Games -- Life", WS_POPUP, MainRect, nullptr, 0);

	BeginWaitCursor();
	ShowWindow(SW_SHOWNORMAL);
	SplashScreen();
	EndWaitCursor();

	// allocation of CLife must happen before DisplayStats
	pDC = GetDC();
	m_cLife = new CLife(pDC);

	// this must be called before RefreshStats (which will happen in OnPaint)
	DisplayStats();

	/*********************************************
	* Life settings. These are global variables. *
	*********************************************/
	if (m_lpGameStruct->bPlayingMetagame == true) {      // playing meta game?
		switch (m_lpGameStruct->nSkillLevel) {          // yes - what skill level?
		case SKILLLEVEL_LOW:
			nLife = VILLAGES_HI;                    // All levels start with 20
			break;

		case SKILLLEVEL_MEDIUM:
			nLife = VILLAGES_HI;
			break;

		case SKILLLEVEL_HIGH:
			nLife = VILLAGES_HI;
			break;

		} // end switch

		nTurnCounter        = ROUNDS_META;
		nSpeed              = SPEED_DEF;
		nCountDown          = nSpeed * MONTHS;
		nLifeCounter        = nLife;
		bIsInfiniteTurns    = false;
		bIsInfiniteLife     = false;
		bPrePlaceColonies   = true;
	} else {                                            // playing stand alone
		nSpeed = GetPrivateProfileInt(INI_SECTION, "Speed", SPEED_DEF, INI_FNAME);
		nCountDown = nSpeed * MONTHS;

		nTurnCounter = GetPrivateProfileInt(INI_SECTION, "Rounds", ROUNDS_DEF, INI_FNAME);
		if (nTurnCounter == TURN_HI_BOUND) {             // fastest turn counter?
			bIsInfiniteTurns = true;
		} else {
			bIsInfiniteTurns = false;
		} // end if

		nLife = GetPrivateProfileInt(INI_SECTION, "Villages", VILLAGES_DEF, INI_FNAME);
		if (nLife == LIFE_HI_BOUND) {                // infinite life?
			bIsInfiniteLife = true;                 // yes
			nLifeCounter    = 0;
		} else {
			bIsInfiniteLife = false;
			nLifeCounter    = nLife;
		} // end if

		nPlace = GetPrivateProfileInt(INI_SECTION, "PrePlace", PLACE_DEF, INI_FNAME);
		if (nPlace > 0) {            // fastest turn counter?
			bPrePlaceColonies = true;
		} else {
			bPrePlaceColonies = false;
		} // end if

	} // end if

	/*****************************************
	* Reports whether game is active or not. *
	*****************************************/
	m_bGameActive       = false;

	/********************************
	* Used with options dialog box. *
	********************************/
	gbNewGame           = false;

	/************************
	* Set up Evolve button. *
	************************/
	EvolveRect.SetRect((GAME_WIDTH >> 1) - (EVOLVE_BUTTON_WIDTH >> 1) - EVOLVE_BUTTON_OFFSET_X,
	                   EVOLVE_BUTTON_OFFSET_Y,
	                   (GAME_WIDTH >> 1) + (EVOLVE_BUTTON_WIDTH >> 1) - EVOLVE_BUTTON_OFFSET_X,
	                   EVOLVE_BUTTON_HEIGHT + EVOLVE_BUTTON_OFFSET_Y);
	pEvolveButton = new CColorButton();
	if (pEvolveButton != nullptr) {
		(*pEvolveButton).Create("Evolve", BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
		                        EvolveRect, this, IDC_EVOLVE);
		(*pEvolveButton).SetPalette(pGamePalette);
	}

	/***********************************
	* Initialize Sprite Scroll Button. *
	***********************************/
	pScrollSprite = new CSprite;
	(*pScrollSprite).SharePalette(pGamePalette);
	bAssertCheck = (*pScrollSprite).LoadSprite(pDC, SCROLL_BUTTON);
	ASSERT(bAssertCheck);
	(*pScrollSprite).SetMasked(true);
	(*pScrollSprite).SetMobile(true);

	/******************************
	* Initialize Sprite Calendar  *
	******************************/
	pCalendarSprite = new CSprite;
	(*pCalendarSprite).SharePalette(pGamePalette);
	bAssertCheck = (*pCalendarSprite).LoadSprite(pDC, CALENDAR_BMP);
	ASSERT(bAssertCheck);
	(*pCalendarSprite).SetMasked(true);
	(*pCalendarSprite).SetMobile(false);

	ReleaseDC(pDC);

	/*******************************************************
	* Set the coordinates for the "Start New Game" button. *
	*******************************************************/
	m_rNewGameButton.SetRect(15, 4, 233, 20);

	/*************************
	* Initialize MIDI sound. *
	*************************/
	if (m_lpGameStruct->bMusicEnabled != false) {
		if ((m_pSound = new CSound((CWnd *) this, MIDI_SOUND, SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END)) != nullptr) {
			m_pSound->midiLoopPlaySegment(1500L, 31000L, 00L, FMT_MILLISEC);
		} else {
			PostMessage(WM_CLOSE, 0, 0);
		}
	}

	/**************************************
	* Initialize random number generator. *
	**************************************/
	//srand((unsigned) time(nullptr));         // seed the random number generator

	/**************************
	* New game or popup menu? *
	**************************/
	if (m_lpGameStruct->bPlayingMetagame == true) {
		bPrePlaceColonies = true;                                   // in meta, pre-place ten
		PostMessage(WM_COMMAND, IDC_OPTIONS_NEWGAME, BN_CLICKED);
	} else {
		PostMessage(WM_COMMAND, IDC_COMMAND, BN_CLICKED);
	} // end if
}

/*****************************************************************
 *
 * ~CMainWindow
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
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
CMainWindow::~CMainWindow() {
	if (pGamePalette != nullptr)
		delete pGamePalette;

	if (pCalendarSprite != nullptr)
		delete pCalendarSprite;

	if (pScrollSprite != nullptr)
		delete pScrollSprite;
}

void CMainWindow::initStatics() {
	nSpeed = nCountDown = nLife = nPlace = 0;
	nTurnCounter = nLifeCounter = 0;
	bPrePlaceColonies = bIsInfiniteLife = bIsInfiniteTurns = false;
	gbNewGame = false;
	pEvolveButton = nullptr;
	pGamePalette = nullptr;
	pScrollSprite = nullptr;
	pCalendarSprite = nullptr;
	hWndParent = nullptr;
}
/*****************************************************************
 *
 * OnPaint
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * OnPaint:
 * This is called whenever Windows sends a WM_PAINT message.
 * Note that creating a CPaintDC automatically does a BeginPaint and
 * an EndPaint call is done when it is destroyed at the end of this
 * function.  CPaintDC's constructor needs the window (this).
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
void CMainWindow::OnPaint() {
	PAINTSTRUCT lpPaint;

	InvalidateRect(nullptr, false);
	BeginPaint(&lpPaint);
	SplashScreen();
	RefreshStats();
	EndPaint(&lpPaint);
}

/*****************************************************************
 *
 * NewGame
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
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
void CMainWindow::NewGame() {
	CDC     *pDC;
	CPoint  point;
	int     i,
	        row,
	        col;

	if ((*m_cLife).m_bIsEvolving == true) {      // Evolve timer on?
		KillTimer(EVOLVE_TIMER_ID);             // yes - turn it off.
		(*m_cLife).m_bIsEvolving = false;
	} // end if

	pDC = GetDC();
	(*m_cLife).NewGame(pDC);

	if (bPrePlaceColonies) {                                     // want ten colonies preplaced?
		// Randomly place 10 of their villages for them!
		i = 0;                                                      // initialize counter
		while (i < VILLAGES_PLACED) {                                // until we place all preset villages
			point.x = (((brand() % PLACE_COLS) + OFFSET_X) * CURLY_X) + BOARD_START_COL;
			point.y = (((brand() % PLACE_ROWS) + OFFSET_Y) * CURLY_Y) + BOARD_START_ROW;
			// track down cell of board that was clicked
			row = (point.y - BOARD_START_ROW) / (BOARD_SPACING_TIMES_TWO + CURLY_Y);
			col = (point.x - BOARD_START_COL) / (BOARD_SPACING_TIMES_TWO + CURLY_X);

			if (!m_cLife->pColony->islife(row, col)) {                // life at this cell already?
				(*m_cLife).change_board(
				    0,                                  // nFlags is not used by change_board
				    point,
				    pDC,
				    m_lpGameStruct->bPlayingMetagame
				);
				i++;                                               // another one placed successfully
			} // end if
		} // end for
	}

	ReleaseDC(pDC);

	SetDlgItemText(IDC_EVOLVE, "Evolve");        // Make sure button is reset to read Evolve

	m_bGameActive = true;
}

/*****************************************************************
 *
 * GamePause
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
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
void CMainWindow::GamePause() {
	if ((*m_cLife).m_bIsEvolving == true) {      // Evolve timer on?
		KillTimer(EVOLVE_TIMER_ID);             // yes - turn it off.
	} // end if
}

/*****************************************************************
 *
 * GameResume
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
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
void CMainWindow::GameResume() {
	if ((*m_cLife).m_bIsEvolving == true) {
		RefreshStats();
		SetTimer(EVOLVE_TIMER_ID, EVOLVE_INTERVAL, nullptr);
	}
}

/*****************************************************************
 *
 * OnCommand
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * Called when a WM_COMMAND message is issued,
 * typically in order to process control related activities.
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
bool CMainWindow::OnCommand(WPARAM wParam, LPARAM lParam) {
	bool bAssertCheck;
	CDC *pDC;

	if (HIWORD(lParam) != BN_CLICKED)
		return true;

	if (wParam == IDC_COMMAND) {             // user click scroll sprite?
		GamePause();

		pDC = GetDC();
		bAssertCheck = (*pScrollSprite).EraseSprite(pDC);
		ReleaseDC(pDC);
		ASSERT(bAssertCheck);

		CMainMenu COptionsWind(    // Create the commands menu
		    (CWnd *)this,
		    pGamePalette,
		    (m_bGameActive ? 0 : NO_RETURN) |
		    (m_lpGameStruct->bPlayingMetagame ? (NO_NEWGAME | NO_OPTIONS) : 0),
		    GetGameParams,
		    RULES_TXT,                      // mini - game rules file
		    RULES_WAV,                      // naration sound file
		    m_lpGameStruct
		);

		CSound::waitWaveSounds();

		switch (COptionsWind.DoModal()) {

		case IDC_OPTIONS_NEWGAME:       // new game?
			NewGame();
			break;

		case IDC_OPTIONS_QUIT:          // quit game?
			if (m_lpGameStruct->bPlayingMetagame != false)
				m_lpGameStruct->lScore = LOSE;

			PostMessage(WM_CLOSE, 0, 0);
			break;

		default:                        // continue?
			break;
		}  // end switch

		if (!m_lpGameStruct->bMusicEnabled && (m_pSound != nullptr)) {

			m_pSound->stop();
			delete m_pSound;
			m_pSound = nullptr;

		} else if (m_lpGameStruct->bMusicEnabled && (m_pSound == nullptr)) {

			if ((m_pSound = new CSound((CWnd *) this, MIDI_SOUND, SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END)) != nullptr)
				m_pSound->midiLoopPlaySegment(1500L, 31000L, 00L, FMT_MILLISEC);
		}

		// show the command scroll
		pDC = GetDC();
		bAssertCheck = (*pScrollSprite).PaintSprite(pDC, SCROLL_BUTTON_X, SCROLL_BUTTON_Y);
		ReleaseDC(pDC);
		ASSERT(bAssertCheck);
		GameResume();

		//if ( gbNewGame == true ) {
		//  PostMessage(WM_COMMAND, IDC_OPTIONS_NEWGAME, BN_CLICKED);
		//  gbNewGame = false;
		//}

	} else if (wParam == IDC_EVOLVE) {                           // Evolve button was clicked
		if ((*m_cLife).m_bIsEvolving == true) {                  // currently evolving?
			if (m_lpGameStruct->bPlayingMetagame == false) {     // playing meta game?
				(*m_cLife).m_bIsEvolving = false;               // no - can turn evolve off, then
				KillTimer(EVOLVE_TIMER_ID);
				SetDlgItemText(IDC_EVOLVE, "Evolve");            // so give them the Suspend option
			}
		} else {
			(*m_cLife).m_bIsEvolving = true;                    // no - turn it on
			nCountDown = nSpeed * MONTHS;
			if (m_lpGameStruct->bPlayingMetagame == false) {     // not playing metagame
				SetDlgItemText(IDC_EVOLVE, "Suspend");       // so give them the Suspend option
			} else {                                            // Only works once in metagame,
				(*pEvolveButton).EnableWindow(false);            //...so disable evolve button
			}

			pDC = GetDC();                                      // Update visual calendars
			(*m_cLife).ResetMonths(pDC);
			ReleaseDC(pDC);

			RefreshStats();
			SetTimer(EVOLVE_TIMER_ID, EVOLVE_INTERVAL, nullptr);
		} // end if
	} else if (wParam == IDC_OPTIONS_NEWGAME) {
		NewGame();
	} // end if

	return true;
}

// OnChar and OnSysChar
// These functions are called when keyboard input generates a character.
//

/*****************************************************************
 *
 * OnSysChar
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
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
void CMainWindow::OnSysKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	switch (nChar) {

	// User has hit ALT_F4 so close down this App
	//
	case VK_F4:
		if (m_lpGameStruct->bPlayingMetagame != false)
			m_lpGameStruct->lScore = LOSE;
		PostMessage(WM_CLOSE, 0, 0);
		break;

	default:
		CFrameWnd::OnSysKeyDown(nChar, nRepCnt, nFlags);
		break;
	} // end switch
}

void CMainWindow::OnSysChar(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	if ((nChar == 'e') && (nFlags & 0x2000)) {          // ALT-e - evolve
		PostMessage(WM_COMMAND, IDC_EVOLVE, BN_CLICKED);
	} else if ((nChar == 'q') && (nFlags & 0x2000)) {    // Alt-q - quit
		if (m_lpGameStruct->bPlayingMetagame != false)
			m_lpGameStruct->lScore = LOSE;
		PostMessage(WM_CLOSE, 0, 0);
	} else {
		CFrameWnd ::OnChar(nChar, nRepCnt, nFlags);     // default action
	} // end if
}

void CMainWindow::OnKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	// Handle keyboard input
	//
	switch (nChar) {

	case VK_F1: {                   // Bring up the Rules
		GamePause();
		CSound::waitWaveSounds();
		CRules  RulesDlg(
		    this,
		    RULES_TXT,
		    pGamePalette,
		    RULES_WAV           // naration sound file
		);
		RulesDlg.DoModal();
		GameResume();
		break;
	} // end case

	case VK_F2:                     // Bring up the options menu
		PostMessage(WM_COMMAND, IDC_COMMAND, BN_CLICKED);
		break;

	default:
		CFrameWnd::OnKeyDown(nChar, nRepCnt, nFlags);
		break;
	} // end switch
}

/*****************************************************************
 *
 * OnLButtonDown
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
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
void CMainWindow::OnLButtonDown(unsigned int nFlags, CPoint point) {
	CDC     *pDC;
	CRect   cTestRect;

	cTestRect = (*pScrollSprite).GetRect();

	if (cTestRect.PtInRect(point) == true) {
		SendMessage(WM_COMMAND, IDC_COMMAND, BN_CLICKED);
	} else if (
	    m_rNewGameButton.PtInRect(point) == true &&
	    m_lpGameStruct->bPlayingMetagame == false
	) {             // playing meta game?
		NewGame();          // no - new game ok
	} else {
		pDC = GetDC();
		(*m_cLife).change_board(
		    nFlags,
		    point,
		    pDC,
		    m_lpGameStruct->bPlayingMetagame
		);
		ReleaseDC(pDC);
	} // end if
}


/*****************************************************************
 *
 * OnLButtonDblClk
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
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
void CMainWindow::OnLButtonDblClk(unsigned int nFlags, CPoint point) {
//  insert mouse button processing code here
	CRect   cTestRect;
	CDC     *pDC;

	cTestRect = (*pScrollSprite).GetRect();

	if (cTestRect.PtInRect(point)) {
		SendMessage(WM_COMMAND, IDC_COMMAND, BN_CLICKED);
	} else if (m_rNewGameButton.PtInRect(point)  &&
	           m_lpGameStruct->bPlayingMetagame == false
	          ) {
		NewGame();
	} else {
		pDC = GetDC();
		(*m_cLife).change_board(
		    nFlags,
		    point,
		    pDC,
		    m_lpGameStruct->bPlayingMetagame
		);
		ReleaseDC(pDC);
	}
}

/*****************************************************************
 *
 * OnTimer
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
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
void CMainWindow::OnTimer(uintptr nIDEvent) {
	CDC *pDC;

	switch (nIDEvent) {
	case EVOLVE_TIMER_ID: {

		// Are there no colonies left to place, and
		//          nothing left on the board?
		if (
		    !nLifeCounter &&
		    bIsInfiniteLife != true &&
		    !(*m_cLife).ColonyPlaced() &&
		    m_lpGameStruct->bPlayingMetagame == false
		) {
			KillTimer(EVOLVE_TIMER_ID);
//b
			if (m_lpGameStruct->bSoundEffectsEnabled != false) {
				sndPlaySound(WAV_DEATH, SND_SYNC);
				sndPlaySound(WAV_GAMEOVER, SND_ASYNC);  // When first starts up so
				//  that u get to see the screen
			} // end if

			CMessageBox GameOver(
			    (CWnd*) this,
			    pGamePalette,
			    "Game over.",
			    "No villages left."
			);

			(*m_cLife).m_bIsEvolving = false;

			if (m_lpGameStruct->bSoundEffectsEnabled != false) {
				sndPlaySound(nullptr, SND_SYNC);
			}

			PostMessage(WM_COMMAND, IDC_OPTIONS_NEWGAME, BN_CLICKED);
			break;
		}

		// Have we evolved proper number of turns or
		//  is evolution infinite?
		if ((*m_cLife).m_nYears != nTurnCounter ||
		        bIsInfiniteTurns) {
			// No - evolve again
			//  has timer run down to zero?
			if (nCountDown <= 0)  {
				// Yes - then evolve
				KillTimer(EVOLVE_TIMER_ID);
				pDC = GetDC();

				(*m_cLife).evolution(pDC);

				// Restart timer and display it
				nCountDown = nSpeed * MONTHS;

				if (nSpeed != FASTEST) {   // Evolve speed long enuf to play sound?
					// Yes - play the tune
					if (m_lpGameStruct->bSoundEffectsEnabled != false)
						sndPlaySound(WAV_EVOLVE, SND_ASYNC);
					// Update visual calendar
					RefreshStats();
				}

				ReleaseDC(pDC);
				SetTimer(EVOLVE_TIMER_ID, EVOLVE_INTERVAL, nullptr);
			} else {  // No, timer not zero, decrement
				// Reduce counter by 1
				nCountDown -= 1;

				// Update visual calendar
				pDC = GetDC();
				(*m_cLife).DisplayMonth(nCountDown, pDC);
				ReleaseDC(pDC);
			}
		} else {  // Yes -- shut this thing down
			KillTimer(EVOLVE_TIMER_ID);
			(*m_cLife).m_bIsEvolving = false;
			char buf[64];
			Common::sprintf_s(buf, "Score:  %ld", (long)(*m_cLife).m_dScore);
			if (m_lpGameStruct->bSoundEffectsEnabled != false)
				sndPlaySound(WAV_GAMEOVER, SND_SYNC);   // When first starts up so
			//  that u get to see the screen
			CMessageBox GameOver(
			    (CWnd*) this,
			    pGamePalette,
			    "Game over.",
			    buf
			);

			if (m_lpGameStruct->bPlayingMetagame != false) {
				m_lpGameStruct->lScore = (long)(*m_cLife).m_dScore;
				PostMessage(WM_CLOSE, 0, 0);
			} else {
				PostMessage(WM_COMMAND, IDC_OPTIONS_NEWGAME, BN_CLICKED);
			}
		}
		break;
	}

	default:
		CFrameWnd ::OnTimer(nIDEvent);
	}
}

/*****************************************************************
 *
 * OnClose
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
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
void CMainWindow::OnClose() {
	CDC     *pDC = GetDC();
	CRect   rctFillRect(0, 0, 640, 480);
	CBrush  Brush(RGB(0, 0, 0));

	pDC->FillRect(&rctFillRect, &Brush);
	ReleaseDC(pDC);
	// TODO: Add your message handler code here and/or call default

	if (pEvolveButton != nullptr) {
		delete pEvolveButton;
		pEvolveButton = nullptr;
	}

	delete m_cLife;

	if (m_pSound != nullptr) {
		m_pSound->stop();
		delete m_pSound;
		m_pSound = nullptr;
	} // end if

	CSound::clearSounds();

	CFrameWnd ::OnClose();
}

/*****************************************************************
 *
 * OnDestroy
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
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
void CMainWindow::OnDestroy() {
//  send a message to the calling app to tell it the user has quit the game
	MFC::PostMessage(m_hCallAppWnd, WM_PARENTNOTIFY, WM_DESTROY, (LPARAM)m_lpGameStruct);

//        delete m_lpGameStruct;
	CFrameWnd::OnDestroy();
}


/*****************************************************************
 *
 * SplashScreen
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * Paint the background art (splash screen) in the client area;
 * called by both OnPaint and InitInstance.
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
void CMainWindow::SplashScreen() {
	CRect   rcDest;
	CRect   rcDIB;
	CDC     *pDC;
	CDibDoc myDoc;
	HDIB    hDIB;
	CSprite *pSprite;
	bool    bCheck;
	int     nMonthIndex;

	bCheck = myDoc.OpenDocument(SPLASHSPEC);
	ASSERT(bCheck);
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
		if (pScrollSprite != nullptr)
			pScrollSprite->PaintSprite(pDC, SCROLL_BUTTON_X, SCROLL_BUTTON_Y);
		if (pCalendarSprite != nullptr)
			pCalendarSprite->PaintSprite(pDC, MONTH_COL_POS, MONTH_ROW_POS);
	}

	pSprite = CSprite::GetSpriteChain();
	while (pSprite) {
		(*pSprite).ClearBackground();
		bCheck = (*pSprite).RefreshSprite(pDC);
		ASSERT(bCheck);
		pSprite = (*pSprite).GetNextSprite();
	}

	// Refresh visual calendar
	if (nSpeed == FASTEST) {
		nMonthIndex = 0;
	} else {
		nMonthIndex = (MONTHS - ((int) nCountDown / nSpeed) - 1);

		if (nMonthIndex < 0)
			nMonthIndex = 0;
	}

	if (m_cLife != nullptr)
		m_cLife->m_cCalendar.pMonthSprite[nMonthIndex]->ClearBackground();

	ReleaseDC(pDC);
}


/*****************************************************************
 *
 * DisplayStats
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
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
void CMainWindow::DisplayStats() {
	CDC         *pDC;
	bool        bAssertCheck;
	CRect       statsRect;
	char        buf[256];

	pDC = GetDC();

	// Colony placed count box
	statsRect.SetRect(CURRENT_LEFT_COL,
	                  CURRENT_LEFT_ROW,
	                  CURRENT_RIGHT_COL,
	                  CURRENT_RIGHT_ROW);

	if (((*m_cLife).pColonyPlaced = new CText()) != nullptr) {
		bAssertCheck = m_cLife->pColonyPlaced->SetupText(pDC, pGamePalette, &statsRect, JUSTIFY_LEFT);
		ASSERT(bAssertCheck);   // initialize the text objext

		Common::sprintf_s(buf, "Current Villages: %d", (*m_cLife).ColonyPlaced());
		bAssertCheck = m_cLife->pColonyPlaced->DisplayString(pDC, buf, STATS_FONT_SIZE, FW_BOLD, STATS_COLOR);
		ASSERT(bAssertCheck);   // paint the text
	}

	// Score box
	statsRect.SetRect(SCORE_LEFT_COL,
	                  SCORE_LEFT_ROW,
	                  SCORE_RIGHT_COL,
	                  SCORE_RIGHT_ROW);

	if (((*m_cLife).pScore = new CText()) != nullptr) {
		bAssertCheck = m_cLife->pScore->SetupText(pDC, pGamePalette, &statsRect, JUSTIFY_LEFT);
		ASSERT(bAssertCheck);   // initialize the text objext

		Common::sprintf_s(buf, "Score: %.1f", (*m_cLife).m_dScore);
		bAssertCheck = m_cLife->pScore->DisplayString(pDC, buf, STATS_FONT_SIZE, FW_BOLD, STATS_COLOR);
		ASSERT(bAssertCheck);   // paint the text
	}

	// Round box
	statsRect.SetRect(ROUND_LEFT_COL,
	                  ROUND_LEFT_ROW,
	                  ROUND_RIGHT_COL,
	                  ROUND_RIGHT_ROW);

	if (((*m_cLife).pYears = new CText()) != nullptr) {
		bAssertCheck = m_cLife->pYears->SetupText(pDC, pGamePalette, &statsRect);
		ASSERT(bAssertCheck);   // initialize the text objext

		Common::sprintf_s(buf, "%d", (*m_cLife).m_nYears);
		bAssertCheck = m_cLife->pYears->DisplayString(pDC, buf, STATS_FONT_SIZE, FW_BOLD, STATS_COLOR);
		ASSERT(bAssertCheck);   // paint the text
	}

	statsRect.SetRect(ROUND_TEXT1_LEFT_COL,
	                  ROUND_TEXT1_LEFT_ROW,
	                  ROUND_TEXT1_RIGHT_COL,
	                  ROUND_TEXT1_RIGHT_ROW);

	if (((*m_cLife).pYearsText1 = new CText()) != nullptr) {
		bAssertCheck = m_cLife->pYearsText1->SetupText(pDC, pGamePalette, &statsRect);
		ASSERT(bAssertCheck);   // initialize the text objext

		Common::sprintf_s(buf, "years");
		bAssertCheck = m_cLife->pYearsText1->DisplayString(pDC, buf, STATS_FONT_SIZE, FW_BOLD, STATS_COLOR);
		ASSERT(bAssertCheck);   // paint the text
	}

	statsRect.SetRect(ROUND_TEXT2_LEFT_COL,
	                  ROUND_TEXT2_LEFT_ROW,
	                  ROUND_TEXT2_RIGHT_COL,
	                  ROUND_TEXT2_RIGHT_ROW);

	if (((*m_cLife).pYearsText2 = new CText()) != nullptr) {
		bAssertCheck = m_cLife->pYearsText2->SetupText(pDC, pGamePalette, &statsRect);
		ASSERT(bAssertCheck);   // initialize the text objext

		Common::sprintf_s(buf, "past");
		bAssertCheck = m_cLife->pYearsText2->DisplayString(pDC, buf, STATS_FONT_SIZE, FW_BOLD, STATS_COLOR);
		ASSERT(bAssertCheck);   // paint the text
	}

	// Colony left count box
	statsRect.SetRect(VILLAGE_LEFT_COL,
	                  VILLAGE_LEFT_ROW,
	                  VILLAGE_RIGHT_COL,
	                  VILLAGE_RIGHT_ROW);

	if (((*m_cLife).pColonyStat = new CText()) != nullptr) {
		bAssertCheck = m_cLife->pColonyStat->SetupText(pDC, pGamePalette, &statsRect);
		ASSERT(bAssertCheck);   // initialize the text objext

		Common::sprintf_s(buf, "%d", nLifeCounter);
		bAssertCheck = m_cLife->pColonyStat->DisplayString(pDC, buf, STATS_FONT_SIZE, FW_BOLD, STATS_COLOR);
		ASSERT(bAssertCheck);   // paint the text
	}

	statsRect.SetRect(VILLAGE_TEXT1_LEFT_COL,
	                  VILLAGE_TEXT1_LEFT_ROW,
	                  VILLAGE_TEXT1_RIGHT_COL,
	                  VILLAGE_TEXT1_RIGHT_ROW);

	if (((*m_cLife).pColonyStatText1 = new CText()) != nullptr) {
		bAssertCheck = m_cLife->pColonyStatText1->SetupText(pDC, pGamePalette, &statsRect);
		ASSERT(bAssertCheck);   // initialize the text objext

		Common::sprintf_s(buf, "left");
		bAssertCheck = m_cLife->pColonyStatText1->DisplayString(pDC, buf, STATS_FONT_SIZE, FW_BOLD, STATS_COLOR);
		ASSERT(bAssertCheck);   // paint the text
	}

	// Update visual calendar
	(*m_cLife).DisplayMonth(nCountDown, pDC);

	ReleaseDC(pDC);
}


/*****************************************************************
 *
 * RefreshStats
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
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
void CMainWindow::RefreshStats() {
	CDC         *pDC;
	bool        bAssertCheck;
	char        buf[256];

	pDC = GetDC();

	// Colony placed count box
	Common::sprintf_s(buf, "Current Villages: %d", (*m_cLife).ColonyPlaced());
	bAssertCheck = m_cLife->pColonyPlaced->DisplayString(pDC, buf, STATS_FONT_SIZE, FW_BOLD, STATS_COLOR);
	ASSERT(bAssertCheck);   // paint the text

	// Score box
	Common::sprintf_s(buf, "Score: %.1f", (*m_cLife).m_dScore);
	bAssertCheck = m_cLife->pScore->DisplayString(pDC, buf, STATS_FONT_SIZE, FW_BOLD, STATS_COLOR);
	ASSERT(bAssertCheck);   // paint the text

	// Round box
	if (bIsInfiniteTurns != true) {
		Common::sprintf_s(buf, "%d", (*m_cLife).m_nYears);

		bAssertCheck = m_cLife->pYears->DisplayString(pDC, buf, STATS_FONT_SIZE, FW_BOLD, STATS_COLOR);
		ASSERT(bAssertCheck);   // paint the text

		Common::sprintf_s(buf, "years");
		bAssertCheck = m_cLife->pYearsText1->DisplayString(pDC, buf, STATS_FONT_SIZE, FW_BOLD, STATS_COLOR);
		ASSERT(bAssertCheck);   // paint the text


		Common::sprintf_s(buf, "past");
		bAssertCheck = m_cLife->pYearsText2->DisplayString(pDC, buf, STATS_FONT_SIZE, FW_BOLD, STATS_COLOR);
		ASSERT(bAssertCheck);   // paint the text
	} else {
		Common::strcat_s(buf, "");

		bAssertCheck = m_cLife->pYears->DisplayString(pDC, buf, STATS_FONT_SIZE, FW_BOLD, STATS_COLOR);
		ASSERT(bAssertCheck);   // paint the text

		bAssertCheck = m_cLife->pYearsText1->DisplayString(pDC, buf, STATS_FONT_SIZE, FW_BOLD, STATS_COLOR);
		ASSERT(bAssertCheck);   // paint the text

		bAssertCheck = m_cLife->pYearsText2->DisplayString(pDC, buf, STATS_FONT_SIZE, FW_BOLD, STATS_COLOR);
		ASSERT(bAssertCheck);   // paint the text
	}

	// Colony left count box
	// update colony count stat
	if (bIsInfiniteLife == true) {
		Common::sprintf_s(buf, "Unlim");
		bAssertCheck = m_cLife->pColonyStat->DisplayString(pDC, buf, STATS_FONT_SIZE, FW_BOLD, STATS_COLOR);
		ASSERT(bAssertCheck);   // paint the text

		Common::strcat_s(buf, "");
		bAssertCheck = m_cLife->pColonyStatText1->DisplayString(pDC, buf, STATS_FONT_SIZE, FW_BOLD, STATS_COLOR);
		ASSERT(bAssertCheck);   // paint the text
	} else {
		Common::sprintf_s(buf, "%d", nLifeCounter);
		bAssertCheck = m_cLife->pColonyStat->DisplayString(pDC, buf, STATS_FONT_SIZE, FW_BOLD, STATS_COLOR);
		ASSERT(bAssertCheck);   // paint the text

		Common::sprintf_s(buf, "left");
		bAssertCheck = m_cLife->pColonyStatText1->DisplayString(pDC, buf, STATS_FONT_SIZE, FW_BOLD, STATS_COLOR);
		ASSERT(bAssertCheck);   // paint the text
	}


	// Update visual calendar
	(*m_cLife).DisplayMonth(nCountDown, pDC);

	ReleaseDC(pDC);
}

LRESULT CMainWindow::OnMCINotify(WPARAM wParam, LPARAM lParam) {
	//CSound  *pSound;

	CSound::OnMCIStopped(wParam, lParam);
	return 0;
}

LRESULT CMainWindow::OnMMIONotify(WPARAM wParam, LPARAM lParam) {
	//CSound  *pSound;

	CSound::OnMMIOStopped(wParam, lParam);
	return 0;
}


/////////////////////////////////////////////////////////////////////////////
// CWindowMain message handlers

// CMainWindow message map:
// Associate messages with member functions.
//
BEGIN_MESSAGE_MAP(CMainWindow, CFrameWnd)
	//{{AFX_MSG_MAP( CMainWindow )
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_SYSKEYDOWN()
	ON_WM_KEYDOWN()
	ON_WM_SYSCHAR()
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_MESSAGE(MM_MCINOTIFY, CMainWindow::OnMCINotify)
	ON_MESSAGE(MM_WOM_DONE, CMainWindow::OnMMIONotify)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

IMPLEMENT_DYNCREATE(CWindowMain, CFrameWnd)

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
    //COLORREF clrCtlBk = RGB(0,0,0);
    //COLORREF clrCtlText = STATS_COLOR;
    CMainWindow *pMyMain;

    TRACE( "Boffo Games\n" );

    //SetDialogBkColor(clrCtlBk,clrCtlText);
    //SetDialogBkColor();       // hook gray dialogs (was default in MFC V1)

    m_pMainWnd = gMainWnd = pMyMain = new CMainWindow();
    m_pMainWnd->ShowWindow( m_nCmdShow );

    // this is required stuff
    pMyMain->SplashScreen();    // Force immediate display to minimize repaint delay
    pMyMain->DisplayStats();    // Initializes the stats

    m_pMainWnd->UpdateWindow();

    return true;
}
*/
/////////////////////////////////////////////////////////////////////////////
// CWindowMain

CWindowMain::CWindowMain() {
}

CWindowMain::~CWindowMain() {
}

BEGIN_MESSAGE_MAP(CWindowMain, CFrameWnd)
	//{{AFX_MSG_MAP(CWindowMain)
	// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*****************************************************************
*
* GetGameParams
*
* FUNCTIONAL DESCRIPTION:
*
*      [Description of function]
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
void CALLBACK GetGameParams(CWnd *pParentWnd) {
	gbNewGame = false;
	CUserCfgDlg dlgUserCfg(pParentWnd, pGamePalette, IDD_USERCFG);

	if (dlgUserCfg.DoModal() == IDOK) {  // user click ok?
		gbNewGame = true;                   // restart game with new settings
	}
} // GetGameParams

} // namespace Life
} // namespace HodjNPodj
} // namespace Bagel
