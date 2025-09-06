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
#include "bagel/hodjnpodj/hnplibs/dibdoc.h"
#include "bagel/boflib/misc.h"
#include "bagel/hodjnpodj/hnplibs/sprite.h"
#include "bagel/hodjnpodj/hnplibs/cmessbox.h"
#include "bagel/hodjnpodj/hnplibs/mainmenu.h"
#include "bagel/hodjnpodj/hnplibs/rules.h"
#include "bagel/boflib/sound.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/hodjnpodj/hnplibs/bitmaps.h"
#include "bagel/hodjnpodj/crypt/globals.h"
#include "bagel/hodjnpodj/crypt/main.h"
#include "bagel/hodjnpodj/crypt/optn.h"
#include "bagel/hodjnpodj/crypt/resource.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {
namespace Crypt {

#define IDB_HOUR        301

void CALLBACK GetGameParams(CWnd *pParentWnd);

/***********
* Globals! *
***********/
CPalette        *pGamePalette = nullptr;       // Palette to be used throughout the game
static CSound   *pGameSound = nullptr;         // Game theme song
CCryptogram     *m_cCryptograms;            // cryptogram game object
static CSprite  *aHourGlass[MAX_HOURS];
static bool     m_bPause;                   // flag to pause the timer
static int      m_nTimer;
static int      tempLetters;
static int      tempTimeLimit;

/*****************************************************************
 *
 * CMainWindow
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * Create the window with the appropriate style, size, menu, etc.;
 * it will be later revealed by CTheApp::InitInstance().  Then
 * create our splash screen object by opening and loading its DIB.
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
CMainWindow::CMainWindow(HWND hCallingWnd, LPGAMESTRUCT lpGameStruct) {
	CDC     *pDC = nullptr;                // device context for the screen
	CString WndClass;
	CDibDoc *pDibDoc = nullptr;            // pointer to the background art DIB
	CSize   mySize;
	bool    bSuccess;                   // bitmap button vars
	CRect   ScrollRect;                 // bitmap button vars
	CSprite *pSprite = nullptr;
	int     i;

	m_bPause = false;
	m_nTimer = 0;
	m_pHourGlass = nullptr;
	m_hCallAppWnd = hCallingWnd;
	m_lpGameStruct = lpGameStruct;
	if (m_lpGameStruct->bPlayingMetagame)
		m_lpGameStruct->lScore = 0L;

	BeginWaitCursor();

	/********************************************************************************
	* Define a special window class which traps double-clicks, is byte aligned      *
	* to maximize BITBLT performance, and creates "owned" DCs rather than sharing   *
	* the five system defined DCs which are not guaranteed to be available;         *
	* this adds a bit to our app size but avoids hangs/freezes/lockups.             *
	********************************************************************************/
	WndClass = AfxRegisterWndClass(CS_DBLCLKS | CS_BYTEALIGNWINDOW | CS_OWNDC,
	                               LoadCursor(nullptr, IDC_ARROW),    //put in my own cursor
	                               nullptr,
	                               nullptr);

	pDC = GetDC();                                  // get a device context for our window

	pDibDoc = new CDibDoc();                        // create an object to hold our splash screen
	ASSERT(pDibDoc);                                // ... and verify we got it
	bSuccess = (*pDibDoc).OpenDocument(SPLASHSPEC); // next load in the actual DIB based artwork
	ASSERT(bSuccess);                               // next load in the actual DIB based artwork
	pGamePalette = (*pDibDoc).DetachPalette();      // grab its palette and save it for later use
	delete pDibDoc;                                 // now discard the splash screen

	MainRect.left = (pDC->GetDeviceCaps(HORZRES) - GAME_WIDTH) >> 1;
	MainRect.top = (pDC->GetDeviceCaps(VERTRES) - GAME_HEIGHT) >> 1;
	MainRect.right = MainRect.left + GAME_WIDTH;    // determine where to place the game window
	MainRect.bottom = MainRect.top + GAME_HEIGHT;   // ... so it is centered on the screen

	/*******************************************
	* New game button area on the Main Window. *
	*******************************************/
	m_cNewGame.left     = NEW_GAME_LEFT;
	m_cNewGame.top      = NEW_GAME_TOP;
	m_cNewGame.right    = NEW_GAME_RIGHT;
	m_cNewGame.bottom   = NEW_GAME_BOTTOM;
	ReleaseDC(pDC);                                 // release our window context

	/********************************************************************************
	* Create the window as a POPUP so that no boarders, title, or menu are present; *
	* this is because the game's background art will fill the entire 640x40 area.   *
	********************************************************************************/
	Create(WndClass, "Boffo Games -- Cryptograms", WS_POPUP, MainRect, nullptr, 0);
	// SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);  // Makes window always on top
	// before every domodal must do update window
	// UpdateWindow();
	// setfocus back to game

	/***********************
	* Bitmap scroll button *
	***********************/
	pDC = GetDC();                                  // get a device context for our window
	m_pScrollButton = new CBmpButton();                     // build a bitmapped scroll button that will invoke
	ASSERT(m_pScrollButton != nullptr);                         // ... the game options dialog box when clicked
	ScrollRect.SetRect(SCROLL_BUTTON_X,                     // establish the rectangular bounds for the button
	                   SCROLL_BUTTON_Y,
	                   SCROLL_BUTTON_X + SCROLL_BUTTON_DX,
	                   SCROLL_BUTTON_Y + SCROLL_BUTTON_DY + 1);
	bSuccess = (*m_pScrollButton).Create(nullptr,              // create the actual button
	                                     BS_OWNERDRAW | WS_CHILD | WS_VISIBLE,
	                                     ScrollRect, this, IDC_SCROLL);
	ASSERT(bSuccess);
	bSuccess = (*m_pScrollButton).LoadBitmaps(    // load the artwork for the button states
	               "SCROLLUP",
	               "SCROLLDOWN",
	               "SCROLLUP",
	               "SCROLLUP"
	           );
	ASSERT(bSuccess);
	m_bIgnoreScrollClick = false;                           // set to process button clicks

	/*******************************************************
	* Put up something to look at while we load up the game*
	*******************************************************/
	ShowWindow(SW_SHOWNORMAL);
	SplashScreen();

	/*************************
	* initialize cryptograms *
	*************************/
	m_cCryptograms = new CCryptogram(pDC);
	ASSERT(m_cCryptograms != nullptr);
	if (m_lpGameStruct->bPlayingMetagame != false) {
		switch (m_lpGameStruct->nSkillLevel) {

		case SKILLLEVEL_LOW:
			m_cCryptograms->m_cStats->m_nLettersSolved = 12;
			m_cCryptograms->m_cStats->m_nTime = 90;
			break;

		case SKILLLEVEL_MEDIUM:
			m_cCryptograms->m_cStats->m_nLettersSolved = 10;
			m_cCryptograms->m_cStats->m_nTime = 80;
			break;

		default: // SKILLLEVEL_HIGH
			m_cCryptograms->m_cStats->m_nLettersSolved = 8;
			m_cCryptograms->m_cStats->m_nTime = 70;
			break;

		} // end switch

	} else {
		m_cCryptograms->m_cStats->m_nLettersSolved = 6;
		m_cCryptograms->m_cStats->m_nTime = 180;            // 3 min.
	}

	tempLetters = m_cCryptograms->m_cStats->m_nLettersSolved;
	tempTimeLimit = m_cCryptograms->m_cStats->m_nTime;
	m_bIsFirstTimeHack = true;

	// pre-load of sections of the sun dial
	//
	for (i = 0; i < MAX_HOURS; i++) {

		if ((aHourGlass[i] = new CSprite) != nullptr) {
			pSprite = aHourGlass[i];
			bSuccess = pSprite->LoadResourceSprite(pDC, IDB_HOUR + i);
			ASSERT(bSuccess);
			pSprite->SharePalette(pGamePalette);
			pSprite->SetPosition(HOUR_X, HOUR_Y);

			pSprite->SetTypeCode(IDB_HOUR + i);
			pSprite->SetMasked(true);
			pSprite->SetMobile(false);

		}
	} // end for
	ASSERT(aHourGlass[0] != nullptr);
	m_pHourGlass = aHourGlass[0];

	ReleaseDC(pDC);                                 // release our window context

	//srand((unsigned) time(nullptr));                 // seed the random number generator

	pGameSound = new CSound(this, GAME_THEME, SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END);
	if (m_lpGameStruct->bMusicEnabled) {
		if (pGameSound != nullptr) {
			(*pGameSound).midiLoopPlaySegment(1080, 32500, 0, FMT_MILLISEC);    //32750
		} // end if pGameSound
	}

	EndWaitCursor();

	if (m_lpGameStruct->bPlayingMetagame)
		PostMessage(WM_COMMAND, IDC_OPTIONS_NEWGAME, BN_CLICKED);
	else
		PostMessage(WM_COMMAND, IDC_SCROLL, BN_CLICKED);
}

CMainWindow::~CMainWindow() {
	if (pGameSound != nullptr) {
		delete pGameSound;
		pGameSound = nullptr;
	}

	CSound::clearSounds();              // clean exit to metagame

	if (m_cCryptograms != nullptr) {        // release Cryptograms
		delete m_cCryptograms;
		m_cCryptograms = nullptr;
	}

	if (m_pScrollButton != nullptr) {   // release button
		delete m_pScrollButton;
		m_pScrollButton = nullptr;
	}

	if (pGamePalette != nullptr) {          // game color palette
		//(*pGamePalette).DeleteObject();
		delete pGamePalette;
		pGamePalette = nullptr;
	}
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
void CMainWindow::OnPaint() {
	PAINTSTRUCT lpPaint;

	InvalidateRect(nullptr, false);                    // invalidate the entire window
	BeginPaint(&lpPaint);                           // bracket start of window update
	SplashScreen();                                 // repaint our window's content
	RefreshStats();                                 // repaint the stats
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
void CMainWindow::SplashScreen() {
	CRect   rcDest;                                     // defines where the art gets painted
	CRect   rcDIB;                                      // defines where the art comes from
	CDC     *pDC;                                       // pointer to the window device context
	CDibDoc myDoc;                                      // contains the artwork's DIB information
	HDIB    hDIB;                                       // a handle to the DIB itself
	CSprite *pSprite;                                   // points to sprite being processed
	bool    bSuccess;

	bSuccess = myDoc.OpenDocument(SPLASHSPEC);          // open the background art file and
	ASSERT(bSuccess);                                   // open the background art file and
	hDIB = myDoc.GetHDIB();                             // ... get a handle to its DIB
	ASSERT(hDIB);

	pDC = GetDC();                                      // get a device context for the window
	ASSERT(pDC);

	GetClientRect(rcDest);                           // get the rectangle to where we paint

	int cxDIB = (int) DIBWidth(hDIB);
	int cyDIB = (int) DIBHeight(hDIB);

	rcDIB.top = rcDIB.left = 0;                         // setup the source rectangle from which
	rcDIB.right = cxDIB;                                // ... we'll do the painting
	rcDIB.bottom = cyDIB;
	PaintDIB((*pDC).m_hDC, &rcDest, hDIB, &rcDIB, pGamePalette);    // transfer the image to the screen

	pSprite = CSprite::GetSpriteChain();                // now get a pointer to the sprite chain
	while (pSprite) {                                   // ... and for each sprite, clear its saved
		(*pSprite).ClearBackground();                   // ... background and repaint its image, thus
		bSuccess = (*pSprite).RefreshSprite(pDC);       // ... restoring the image but forcing it to
		ASSERT(bSuccess);                               // ... restoring the image but forcing it to
		pSprite = (*pSprite).GetNextSprite();
	}          // ... fetch/save the background it covers up

	if (m_bIsFirstTimeHack == true) {
		m_bIsFirstTimeHack = false;
	}

	ReleaseDC(pDC);                                     // release the window's context
}

void CMainWindow::DisplayStats(CDC *pDC) {
	bool        bSuccess;

	if (m_cCryptograms->m_cStats->m_nTime != MAX_TIME) {
		(*m_pHourGlass).ClearBackground();                  // ... background and repaint its image, thus
		bSuccess = (*m_pHourGlass).RefreshSprite(pDC);      // ... restoring the image but forcing it to
		ASSERT(bSuccess);                                   // ... restoring the image but forcing it to
	}

}

void CMainWindow::RefreshStats() {
	CDC         *pDC;
	//bool        bSuccess;

	pDC = GetDC();

	if (m_cCryptograms->m_cStats->m_nTime != MAX_TIME) {                // Game is timed
		if (m_pHourGlass != nullptr) {
			(*m_pHourGlass).ClearBackground();                  // ... background and repaint its image, thus
			(*m_pHourGlass).RefreshSprite(pDC);      // ... restoring the image but forcing it to
		}
	}
	if (m_cCryptograms->bIsGameOver == true) {
		if (m_lpGameStruct->bPlayingMetagame)
			m_lpGameStruct->lScore = m_cCryptograms->m_cStats->m_nScore;
	}

	ReleaseDC(pDC);
}

void CMainWindow::GameWin() {
	CDC     *pDC;
	char    buf[64];

	KillTimer(STAT_TIMER_ID);
	m_cCryptograms->m_cStats->m_nScore = SCORE_JACKPOT;
	m_cCryptograms->bIsGameOver = true;

	RefreshStats();
	pDC = GetDC();
	m_cCryptograms->DrawSource(pDC);
	ReleaseDC(pDC);

	Common::sprintf_s(buf, "Score:  %d", m_cCryptograms->m_cStats->m_nScore);
	CMessageBox GameOverDlg((CWnd *)this, pGamePalette,
	                        "You win!", buf, -1, 30);
}

void CMainWindow::GameLose() {
	CDC     *pDC;
	char    buf[64];

	KillTimer(STAT_TIMER_ID);
	pDC = GetDC();

	m_cCryptograms->bIsGameOver = true;
	m_cCryptograms->m_cStats->m_nCountDown = 0;
	m_cCryptograms->m_cStats->m_nScore = m_cCryptograms->LettersSolved() * SCORE_FACTOR;

	RefreshStats();
	Common::sprintf_s(buf, "Score:  %d", m_cCryptograms->m_cStats->m_nScore);
	CMessageBox GameOverDlg((CWnd *)this, pGamePalette,
	                        "Time's up!", buf,  -1, 30);

	ReleaseDC(pDC);
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
bool CMainWindow::OnCommand(WPARAM wParam, LPARAM lParam) {
	CDC         *pDC;
	long        IntervalLength = 0;
	CRules      RulesDlg((CWnd *)this, RULES_TEXT, pGamePalette,
	                     (m_lpGameStruct->bSoundEffectsEnabled ? RULES_WAV : nullptr));
	CMainMenu COptionsWind((CWnd *)this, pGamePalette,
	    m_lpGameStruct->bPlayingMetagame ? (NO_NEWGAME | NO_OPTIONS) :
			// WORKAROUND: Properly disable Continue initially
			CSprite::GetSpriteChain() ? 0 : NO_RETURN,
	    GetGameParams, RULES_TEXT,
	    (m_lpGameStruct->bSoundEffectsEnabled ? RULES_WAV : nullptr),
		m_lpGameStruct);

	m_bPause = true;

	if (HIWORD(lParam) == BN_CLICKED) {
		switch (wParam) {

		case IDC_OPTIONS_RULES:
			m_bIgnoreScrollClick = true;
			(*m_pScrollButton).SendMessage(BM_SETSTATE, true, 0L);

			CSound::waitWaveSounds();
			RulesDlg.DoModal();
			m_bPause = false;

			break;

		case IDC_OPTIONS_NEWGAME:
			pDC = GetDC();

			// Make the new settings active on a new game
			m_cCryptograms->m_cStats->m_nLettersSolved = tempLetters;
			m_cCryptograms->m_cStats->m_nTime = tempTimeLimit;

			/**********************************************************
			* Dump User out if error occured in cryptogram text file. *
			**********************************************************/
			if (m_cCryptograms->DrawGram(pDC) == false) {
				char szMsg[128];
				Common::sprintf_s(szMsg, "File error.  Check %s.", CRYPT_TXT_FILE);
				MessageBox(szMsg);
				PostMessage(WM_CLOSE, 0, 0);
				return false;
			}

			KillTimer(STAT_TIMER_ID);

			m_nTimer = 0;                                       // Go back to first hourglass
			m_pHourGlass = nullptr;
			m_pHourGlass = aHourGlass[m_nTimer];

			if (m_cCryptograms->m_cStats->m_nTime == MAX_TIME) {        // if the hourglass was up, erase it
				m_pHourGlass->EraseSprite(pDC);
			} else if (m_cCryptograms->m_cStats->m_nTime != MAX_TIME) {      // Max time is more than max
				IntervalLength = ((long)1000 * m_cCryptograms->m_cStats->m_nTime) / (MAX_HOURS - 1);
				SetTimer(STAT_TIMER_ID, (unsigned int)(IntervalLength), nullptr);
			}

			RefreshStats();

			ReleaseDC(pDC);

			m_bIgnoreScrollClick = false;
			(*m_pScrollButton).SendMessage(BM_SETSTATE, false, 0L);
			if (m_cCryptograms->IsSolved() == true) {
				GameWin();
				if (m_lpGameStruct->bPlayingMetagame)
					PostMessage(WM_CLOSE, 0, 0);
			}
			m_bPause = false;

			break;

		case IDC_SCROLL:
			if (m_bIgnoreScrollClick) {
				(*m_pScrollButton).SendMessage(BM_SETSTATE, true, 0L);
				break;
			}
			m_bIgnoreScrollClick = true;
			(*m_pScrollButton).SendMessage(BM_SETSTATE, true, 0L);
			SendDlgItemMessage(IDC_SCROLL, BM_SETSTATE, true, 0L);

			switch (COptionsWind.DoModal()) {

			case IDC_OPTIONS_RETURN:
				(*m_pScrollButton).SendMessage(BM_SETSTATE, false, 0L);
				m_bIgnoreScrollClick = false;
				m_bPause = false;
				break;

			case IDC_OPTIONS_NEWGAME:
				pDC = GetDC();
				(*m_pScrollButton).SendMessage(BM_SETSTATE, false, 0L);
				m_bIgnoreScrollClick = false;

				//UpdateWindow();
				PostMessage(WM_COMMAND, IDC_OPTIONS_NEWGAME, BN_CLICKED);
				break;

			case IDC_OPTIONS_QUIT:
				PostMessage(WM_CLOSE, 0, 0);
				return false;

			} //end switch(ComDlg.DoModal())
			if ((m_lpGameStruct->bMusicEnabled == false) && (pGameSound != nullptr)) {
				if (pGameSound->playing())
					(*pGameSound).stop();
			} else if (m_lpGameStruct->bMusicEnabled) {
				if (pGameSound == nullptr) {
					pGameSound = new CSound(this, GAME_THEME,
					                        SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END);
				}
				if (pGameSound != nullptr) {
					if (!pGameSound->playing())
						(*pGameSound).midiLoopPlaySegment(1080, 32500, 0, FMT_MILLISEC);
				} // end if pGameSound
			}
			break;

		} //end switch(wParam)

	}

	(*this).SetFocus();                     // Reset focus back to the main window
	return true;
}

/*****************************************************************
 *
 * OnChar
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  This function is called when keyboard input generates a character.
 *
 *  (Add game-specific processing)
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
void CMainWindow::OnChar(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	CDC *pDC;

	pDC = GetDC();

	if (m_cCryptograms->HandleUserUpdate(pDC, nChar) == true) {
		GameWin();
		if (m_lpGameStruct->bPlayingMetagame)
			PostMessage(WM_CLOSE, 0, 0);
	}

	ReleaseDC(pDC);

	FlushInputEvents();

	CFrameWnd ::OnChar(nChar, nRepCnt, nFlags);     // default action
}

/*****************************************************************
 *
 * OnSysChar
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  This function is called when keyboard input generates a
 *  system character.
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
void CMainWindow::OnSysChar(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	if ((nChar == 'q') && (nFlags & 0x2000))        // terminate the game on an ALT-q
		PostMessage(WM_CLOSE, 0, 0);                // ... same as clicking QUIT button
	else
		CFrameWnd ::OnChar(nChar, nRepCnt, nFlags); // default action
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
void CMainWindow::OnMouseMove(unsigned int nFlags, CPoint point) {

	CFrameWnd ::OnMouseMove(nFlags, point);         // default action
}


void CMainWindow::OnLButtonDown(unsigned int nFlags, CPoint point) {
	CDC *pDC;
	CSprite *pSprite = nullptr;                    // Easter Egg anim pointer
	CSound  *pEffect = nullptr;                    // EE & Sound pointer
	CRect   skullRect,                          // Easter Egg hotspots
	        urn1Rect,
	        urn2Rect,
	        urn3Rect,
	        gryphRect,                          // Sound hotspots
	        stepsRect,
	        torch1Rect,
	        torch2Rect,
	        torch3Rect,
	        torch4Rect,
	        hourRect;
	bool    bSuccess;
	int     i,
	        nPick = 0,
	        nSleepTime;
	char    bufName[64];

	skullRect.SetRect(SKULL_X, SKULL_Y, SKULL_X + SKULL_DX, SKULL_Y + SKULL_DY);
	urn1Rect.SetRect(URN1_X, URN1_Y, URN1_X + URN1_DX, URN1_Y + URN1_DY);
	urn2Rect.SetRect(URN2_X, URN2_Y, URN2_X + URN2_DX, URN2_Y + URN2_DY);
	urn3Rect.SetRect(URN3_X, URN3_Y, URN3_X + URN3_DX, URN3_Y + URN3_DY);
	gryphRect.SetRect(GRYPH_X, GRYPH_Y, GRYPH_X + GRYPH_DX, GRYPH_Y + GRYPH_DY);
	stepsRect.SetRect(STEPS_X, STEPS_Y, STEPS_X + STEPS_DX, STEPS_Y + STEPS_DY);
	torch1Rect.SetRect(TORCH1_X, TORCH1_Y, TORCH1_X + TORCH_DX, TORCH1_Y + TORCH_DY);
	torch2Rect.SetRect(TORCH2_X, TORCH2_Y, TORCH2_X + TORCH_DX, TORCH2_Y + TORCH_DY);
	torch3Rect.SetRect(TORCH3_X, TORCH3_Y, TORCH3_X + TORCH_DX, TORCH3_Y + TORCH_DY);
	torch4Rect.SetRect(TORCH4_X, TORCH4_Y, TORCH4_X + TORCH_DX, TORCH4_Y + TORCH_DY);
	hourRect.SetRect(HOUR_X, HOUR_Y, HOUR_X + HOUR_DX, HOUR_Y + HOUR_DY);

	pDC = GetDC();
	if ((*m_cCryptograms).HandleUserUpdate(pDC, point) == true) {
		GameWin();
		if (m_lpGameStruct->bPlayingMetagame)
			PostMessage(WM_CLOSE, 0, 0);
		return;
	}
	ReleaseDC(pDC);

	if ((m_cNewGame.PtInRect(point) == true) && (!m_lpGameStruct->bPlayingMetagame))
		PostMessage(WM_COMMAND, IDC_OPTIONS_NEWGAME, BN_CLICKED);
	else if (skullRect.PtInRect(point)) {
		pDC = GetDC();
		pSprite = new CSprite;
		(*pSprite).SharePalette(pGamePalette);
		bSuccess = (*pSprite).LoadCels(pDC, ".\\art\\skull.bmp", NUM_SKULL_CELS);
		ASSERT(bSuccess);
		(*pSprite).SetMasked(false);
		(*pSprite).SetMobile(false);

		if (m_lpGameStruct->bSoundEffectsEnabled) {
			nPick = brand() % NUM_SKULL_SOUNDS;
			switch (nPick) {
			case 0:
				Common::sprintf_s(bufName, WAV_JOKE1);
				nSleepTime = JOKE1_SLEEP;
				break;
			case 1:
				Common::sprintf_s(bufName, WAV_JOKE2);
				nSleepTime = JOKE2_SLEEP;
				break;
			case 2:
				Common::sprintf_s(bufName, WAV_JOKE3);
				nSleepTime = JOKE3_SLEEP;
				break;
			case 3:
				Common::sprintf_s(bufName, WAV_JOKE4);
				nSleepTime = JOKE4_SLEEP;
				break;
			case 4:
				Common::sprintf_s(bufName, WAV_JOKE5);
				nSleepTime = JOKE5_SLEEP;
				break;
			default:
				Common::sprintf_s(bufName, WAV_JOKE6);
				nSleepTime = JOKE6_SLEEP;
				break;
			}
			pEffect = new CSound((CWnd *)this, bufName, SOUND_QUEUE |
			                     SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE);  //...Wave file, to delete itself
			(*pEffect).play();                                                      //...play the narration
		} else      // no sound playing
			nSleepTime = SKULL_SLEEP;

		if (bSuccess) {
			(*pSprite).SetCel(NUM_SKULL_CELS);
			for (i = 0; i < NUM_SKULL_CELS; i++) {
				(*pSprite).PaintSprite(pDC, SKULL_X, SKULL_Y);
				if (i < (NUM_SKULL_CELS - 1))
					Sleep(nSleepTime);
			}
			if (nPick == 1) {
				for (i = 0; i < NUM_SKULL_CELS; i++) {
					(*pSprite).PaintSprite(pDC, SKULL_X, SKULL_Y);
					Sleep(JOKE2B_SLEEP);
				}
			}
		}
		if (pSprite != nullptr)
			delete pSprite;

		ReleaseDC(pDC);
	} else if (urn1Rect.PtInRect(point)) {
		pDC = GetDC();
		pSprite = new CSprite;
		(*pSprite).SharePalette(pGamePalette);
		bSuccess = (*pSprite).LoadCels(pDC, ".\\art\\urn01.bmp", NUM_URN1_CELS);
		ASSERT(bSuccess);
		(*pSprite).SetMasked(false);
		(*pSprite).SetMobile(false);

		if (m_lpGameStruct->bSoundEffectsEnabled) {
			pEffect = new CSound((CWnd *)this, WAV_URN1, SOUND_QUEUE |
			                     SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE);  //...Wave file, to delete itself
			(*pEffect).play();                                                      //...play the narration
		}
		if (bSuccess) {
			(*pSprite).SetCel(NUM_URN1_CELS);
			for (i = 0; i < NUM_URN1_CELS; i++) {
				(*pSprite).PaintSprite(pDC, URN1_X, URN1_Y);
				Sleep(URN1_SLEEP);
			}
		}
		if (pSprite != nullptr)
			delete pSprite;

		ReleaseDC(pDC);
	} else if (urn2Rect.PtInRect(point)) {
		pDC = GetDC();
		pSprite = new CSprite;
		(*pSprite).SharePalette(pGamePalette);
		bSuccess = (*pSprite).LoadCels(pDC, ".\\art\\urn02.bmp", NUM_URN2_CELS);
		ASSERT(bSuccess);
		(*pSprite).SetMasked(false);
		(*pSprite).SetMobile(false);

		if (m_lpGameStruct->bSoundEffectsEnabled) {
			pEffect = new CSound((CWnd *)this, WAV_URN2, SOUND_QUEUE |
			                     SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE);  //...Wave file, to delete itself
			(*pEffect).play();                                                      //...play the narration
		}
		if (bSuccess) {
			(*pSprite).SetCel(NUM_URN2_CELS);
			for (i = 0; i < NUM_URN2_CELS; i++) {
				(*pSprite).PaintSprite(pDC, URN2_X, URN2_Y);
				Sleep(URN2_SLEEP);
			}
		}
		if (pSprite != nullptr)
			delete pSprite;

		ReleaseDC(pDC);
	} else if (urn3Rect.PtInRect(point)) {
		pDC = GetDC();
		pSprite = new CSprite;
		(*pSprite).SharePalette(pGamePalette);
		bSuccess = (*pSprite).LoadCels(pDC, ".\\art\\urn03.bmp", NUM_URN3_CELS);
		ASSERT(bSuccess);
		(*pSprite).SetMasked(false);
		(*pSprite).SetMobile(false);

		if (m_lpGameStruct->bSoundEffectsEnabled) {
			pEffect = new CSound((CWnd *)this, WAV_URN3, SOUND_QUEUE |
			                     SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE);  //...Wave file, to delete itself
			(*pEffect).play();                                                      //...play the narration
		}
		if (bSuccess) {
			(*pSprite).SetCel(NUM_URN3_CELS);
			for (i = 0; i < NUM_URN3_CELS; i++) {
				(*pSprite).PaintSprite(pDC, URN3_X, URN3_Y);
				Sleep(URN3_SLEEP);
			}
		}
		if (pSprite != nullptr)
			delete pSprite;

		ReleaseDC(pDC);
	} else if (gryphRect.PtInRect(point) && (m_lpGameStruct->bSoundEffectsEnabled)) {
		pEffect = new CSound((CWnd *)this, WAV_GRYPH, SOUND_QUEUE |
		                     SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE);  //...Wave file, to delete itself
		(*pEffect).play();                                                      //...play the narration
	} else if (stepsRect.PtInRect(point) && (m_lpGameStruct->bSoundEffectsEnabled)) {
		pEffect = new CSound((CWnd *)this, WAV_STEPS, SOUND_QUEUE |
		                     SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE);  //...Wave file, to delete itself
		(*pEffect).play();                                                      //...play the narration
	} else if (m_cCryptograms->m_cStats->m_nTime < MAX_TIME &&
	           (hourRect.PtInRect(point) && (m_lpGameStruct->bSoundEffectsEnabled))) {
		pEffect = new CSound((CWnd *)this, WAV_HOUR, SOUND_QUEUE |
		                     SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE);  //...Wave file, to delete itself
		(*pEffect).play();                                                      //...play the narration
	} else if (((torch1Rect.PtInRect(point)) || (torch2Rect.PtInRect(point))) ||
	           ((torch3Rect.PtInRect(point)) || (torch4Rect.PtInRect(point)))) {
		if (m_lpGameStruct->bSoundEffectsEnabled) {
			pEffect = new CSound((CWnd *)this, WAV_TORCH, SOUND_QUEUE |
			                     SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE);  //...Wave file, to delete itself
			(*pEffect).play();                                                      //...play the narration
		}
	}
}


void CMainWindow::OnLButtonUp(unsigned int nFlags, CPoint point) {
}


void CMainWindow::OnLButtonDblClk(unsigned int nFlags, CPoint point) {
	CDC *pDC;

	pDC = GetDC();
	if ((*m_cCryptograms).HandleUserUpdate(pDC, point) == true) {
		GameWin();
		if (m_lpGameStruct->bPlayingMetagame)
			PostMessage(WM_CLOSE, 0, 0);
		ReleaseDC(pDC);
		return;
	}

	if ((m_cNewGame.PtInRect(point) == true) && (!m_lpGameStruct->bPlayingMetagame))
		PostMessage(WM_COMMAND, IDC_OPTIONS_NEWGAME, BN_CLICKED);
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

/*****************************************************************
 *
 * OnTimer
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  Update sprite positions on the screen; this is cyclic based on
 *  the interval specified when the timer was initiated.
 *
 *  This function is called when ever the interval timer generates
 *  an event message; i.e. the timer fires.
 *
 * FORMAL PARAMETERS:
 *
 *  nIDEvent    identifies the particular timer that fired
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
void CMainWindow::OnTimer(uintptr nIDEvent) {
	if ((nIDEvent == STAT_TIMER_ID) && (m_bPause == false)) {
		// move pointer to next hour glass formation
		//
		m_nTimer++;

		m_pHourGlass = nullptr;
		ASSERT(m_nTimer >= 0 && m_nTimer < MAX_HOURS);
		if (m_nTimer < MAX_HOURS)
			m_pHourGlass = aHourGlass[m_nTimer];
		ASSERT(m_pHourGlass != nullptr);

		RefreshStats();

		if (m_nTimer == (MAX_HOURS - 1)) {
			GameLose();
			if (m_lpGameStruct->bPlayingMetagame)
				PostMessage(WM_CLOSE, 0, 0);
		}

		/*
		        m_nTimer++;
		        if ( (m_nTimer == (MAX_HOURS - 1) ) ) {     //(m_cCryptograms->m_cStats->m_nCountDown <= 0) ||
		            GameLose();
		            if (m_lpGameStruct->bPlayingMetagame)
		                PostMessage( WM_CLOSE, 0, 0 );
		        }
		        else {
		            // move pointer to next hour glass formation
		            //
		            m_pHourGlass = nullptr;
		            ASSERT(m_nTimer >= 0 && m_nTimer < MAX_HOURS );
		            m_pHourGlass = aHourGlass[m_nTimer];
		            ASSERT(m_pHourGlass != nullptr);

		            RefreshStats();
		        }
		*/
	} else {
		CFrameWnd ::OnTimer(nIDEvent);
	}
}

void CMainWindow::OnSysKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	// terminate app on ALT_F4
	//
	if ((nChar == VK_F4) && (nFlags & 0x2000)) {

		PostMessage(WM_CLOSE, 0, 0);

	} else {

		// default action
		CWnd::OnSysKeyDown(nChar, nRepCnt, nFlags);
	}
}


/*****************************************************************
 *
 * OnKeyDown
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * FORMAL PARAMETERS:
 *
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *
 * RETURN VALUE:
 *
 *
 ****************************************************************/
void CMainWindow::OnKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	if (nChar == VK_F1) {                                  // F1 key is hit
		SendMessage(WM_COMMAND, IDC_OPTIONS_RULES, BN_CLICKED);  // Activate the Options dialog
		(*m_pScrollButton).SendMessage(BM_SETSTATE, false, 0L); // Reset scroll button to up state
		m_bIgnoreScrollClick = false;
	} else if (nChar == VK_F2) {                                 // F2 key is hit
		SendMessage(WM_COMMAND, IDC_SCROLL, BN_CLICKED);     // Activate the Rules dialog
		(*m_pScrollButton).SendMessage(BM_SETSTATE, false, 0L); // Reset scroll button to up state
		m_bIgnoreScrollClick = false;
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
void CMainWindow::OnClose() {
	int i;

	if (m_lpGameStruct->bPlayingMetagame)
		m_lpGameStruct->lScore = m_cCryptograms->m_cStats->m_nScore;

	// release the master sprites
	//
	m_pHourGlass = nullptr;

	for (i = 0; i < MAX_HOURS; i++) {
		if (aHourGlass[i] != nullptr) {
			delete aHourGlass[i];
			aHourGlass[i] = nullptr;
		}
	}

	CFrameWnd ::OnClose();
}

void CMainWindow::OnDestroy() {
//  send a message to the calling app to tell it the user has quit the game
	MFC::PostMessage(m_hCallAppWnd, WM_PARENTNOTIFY, WM_DESTROY, (LPARAM)m_lpGameStruct);
	CFrameWnd::OnDestroy();
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
void CMainWindow::FlushInputEvents() {
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

/*****************************************************************
 *
 * InitInstance
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  This routine is automatically called when the application is
 *  started.
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
 *      bool        Success (true) / Failure (false) status
 *
 ****************************************************************/
/*
bool CTheApp::InitInstance()
{
    CMainWindow *pMyMain;

    TRACE( "Boffo Games\n" );

    SetDialogBkColor();                                 // hook gray dialogs (was default in MFC V1)

    m_pMainWnd = pMyMain = new CMainWindow();           // make the main window visible
    m_pMainWnd->ShowWindow( m_nCmdShow );

    // this is required stuff
    pMyMain->SplashScreen();    // Force immediate display to minimize repaint delay
    pMyMain->DisplayStats();

    m_pMainWnd->UpdateWindow();

    return(true);
}

*/
/*****************************************************************
 *
 * ExitInstance
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  This routine is automatically called when the application is
 *  being terminated.
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
 *      int         Success (0) / Failure status
 *
 ****************************************************************/
/*
int CTheApp::ExitInstance()
{
    return(0);
}

*/

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

//
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
	ON_WM_DESTROY()
	ON_WM_CREATE()
	ON_WM_KEYDOWN()
	ON_WM_SYSKEYDOWN()
	ON_MESSAGE(MM_MCINOTIFY, CMainWindow::OnMCINotify)
	ON_MESSAGE(MM_WOM_DONE, CMainWindow::OnMMIONotify)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CALLBACK GetGameParams(CWnd *pParentWnd) {
	//
	// Our user preference dialog box is self contained in this object
	//
//    CUserCfgDlg dlgUserCfg(pParentWnd, pGamePalette, IDD_USERCFG);
	COptn       cOpnDlg(
	    pParentWnd,
	    pGamePalette,
	    m_cCryptograms->m_cStats->m_nLettersSolved,
	    m_cCryptograms->m_cStats->m_nTime
	);

	if (cOpnDlg.DoModal() == IDOK) {
//		m_cCryptograms->m_cStats->SaveStats(cOpnDlg.m_nLttrsSlvd, cOpnDlg.m_nTime);
//		(*pParentWnd).PostMessage(WM_COMMAND, IDC_OPTIONS_NEWGAME, BN_CLICKED);
		tempLetters = cOpnDlg.m_nLttrsSlvd;
		tempTimeLimit = cOpnDlg.m_nTime;
		(*pParentWnd).PostMessage(WM_COMMAND, IDC_SCROLL, BN_CLICKED);
	} else {
		(*pParentWnd).PostMessage(WM_COMMAND, IDC_SCROLL, BN_CLICKED);
	}

}

int CMainWindow::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CFrameWnd ::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO: Add your specialized creation code here


	return 0;
}

} // namespace Crypt
} // namespace HodjNPodj
} // namespace Bagel
