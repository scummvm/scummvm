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
#include "bagel/hodjnpodj/hnplibs/stdafx.h"
#include "bagel/boflib/misc.h"
#include "bagel/hodjnpodj/hnplibs/sprite.h"
#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/hnplibs/mainmenu.h"
#include "bagel/hodjnpodj/hnplibs/cmessbox.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/hnplibs/gamedll.h"
#include "bagel/hodjnpodj/hnplibs/rules.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/hodjnpodj/garfunkle/resource.h"
#include "bagel/hodjnpodj/garfunkle/garfunkle.h"
#include "bagel/hodjnpodj/garfunkle/optndlg.h"
#include "bagel/hodjnpodj/garfunkle/note.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {
namespace Garkfunkle {

void CALLBACK GetSubOptions(CWnd* pParentWind);
void add_note_to_series(int nNewValue);

CBmpButton  *m_pScrollButton;               // Scroll button
CPalette    *pGamePalette = nullptr;           // Palette of current artwork
bool        bSuccess = false;
bool        m_bIgnoreScrollClick;
bool        bLDown = false;                 // Make sure we only act on LUp if LDown was on a musician
bool        bPlayingBackSeries = false;

#define     FIRST_MUSICIAN      100
#define     NOT_PLAYING         0
#define     NOT_THERE           1

//
// Musician-related stuff:
//
CSound      *pMusic = nullptr;                     // The sound object for the musician's music
CSprite     *pAnimSprite[MAX_BUTTONS];          // Pointer for the animating musician
CBitmap     *pMusicians[(MAX_BUTTONS * 2)];     // Bitmap for Not_playing and Not_there
int         m_nButID = 0;                       // button which is animating

CText       *m_pSignText = nullptr;                // The current series length display on the sign
CBitmap     *pRibbon = nullptr;                    // The blue ribbon to be put on sign at win condition

bool    m_bPlaying;
bool    m_bNewGame = false;

bool    m_bPlayGame;                            // Options variables
int     m_nSpeed;                               // Speed is in MILLISECONDS
int     m_nNumButtons;
int     m_nWinCondition = 0;                    // Number needed to win the game (in meta mode only)

// Temporary variables for new Options
bool    tempPlayGame;
int     tempSpeed;          // Speed is in MILLISECONDS
int     tempNumButtons;

int     nNoteCount = 0;
int     nCheckCount = 0;
unsigned int    nSButFlag = MAX_BUTTONS;
CNote   *pNoteMarker;

static const char *cSoundName[MAX_BUTTONS] = {
	VIOLIN_SOUND, CELLO_SOUND, DRUM_SOUND,
	SAX_SOUND, HARP_SOUND, CLARINET_SOUND
};

static const char *cAnimName[MAX_BUTTONS] = {
	VIOLIN_ANIM, CELLO_ANIM, DRUM_ANIM, SAX_ANIM,
	HARP_ANIM, CLARINET_ANIM
};

static const int nNumCels[MAX_BUTTONS] = { VIOLIN_CELS, CELLO_CELS, DRUM_CELS, SAX_CELS,
                                  HARP_CELS, CLARINET_CELS
                                };

static const POINT Offset[MAX_BUTTONS] = {
	{ VIOLIN_OFFSET_X, VIOLIN_OFFSET_Y },
	{ CELLO_OFFSET_X, CELLO_OFFSET_Y },
	{ DRUM_OFFSET_X, DRUM_OFFSET_Y },
	{ SAX_OFFSET_X, SAX_OFFSET_Y },
	{ HARP_OFFSET_X, HARP_OFFSET_Y },
	{ CLARINET_OFFSET_X, CLARINET_OFFSET_Y }
};

extern  LPGAMESTRUCT pGameInfo;
extern HWND ghParentWnd;

/////////////////////////////////////////////////////////////////////////////

// CMainWindow constructor:
// Create the window with the appropriate style, size, menu, etc.;
// it will be later revealed by CTheApp::InitInstance().  Then
// create our splash screen object by opening and loading its DIB.
//
CMainWindow::CMainWindow() {
	CDC     *pDC;
	CString WndClass;
	CRect   MainRect, StartRect;
	CBitmap *pBackBitmap = nullptr;
	CPalette *pOldPal = nullptr;
	int     i, j;

	BeginWaitCursor();
	initStatics();

	pGameInfo->lScore = 0L;

	// Define a special window class which traps double-clicks, is byte aligned
	// to maximize BITBLT performance, and creates "owned" DCs rather than sharing
	// the five system defined DCs which are not guaranteed to be available;
	// this adds a bit to our app size but avoids hangs/freezes/lockups.
	WndClass = AfxRegisterWndClass(CS_BYTEALIGNWINDOW | CS_OWNDC,
	                               nullptr,
	                               nullptr,
	                               nullptr);

	// Center our window on the screen
	pDC = GetDC();

	MainRect.left = (pDC->GetDeviceCaps(HORZRES) - GAME_WIDTH) >> 1;
	MainRect.top = (pDC->GetDeviceCaps(VERTRES) - GAME_HEIGHT) >> 1;
	MainRect.right = MainRect.left + GAME_WIDTH;
	MainRect.bottom = MainRect.top + GAME_HEIGHT;

	ReleaseDC(pDC);

	// Create the window as a POPUP so no boarders, title, or menu are present;
	// this is because the game's background art will fill the entire 640x480 area.
	Create(WndClass, "Boffo Games -- Garfunkel", WS_POPUP, MainRect, nullptr, 0);

	pDC = GetDC();
	pBackBitmap = FetchBitmap(pDC, &pGamePalette, MAINSCREEN);
	pOldPal = pDC->SelectPalette(pGamePalette, false);           // select the game palette
	pDC->RealizePalette();                                          //...and realize it

	pBackBitmap->DeleteObject();
	delete pBackBitmap;

	ShowWindow(SW_SHOWNORMAL);                           // Give 'em something to look at
	SplashScreen();                                         // Paint the backdrop with no musicians

	// Build Scroll Command button
	m_pScrollButton = new CBmpButton;
	ASSERT(m_pScrollButton != nullptr);
	StartRect.SetRect(SCROLL_BUTTON_X, SCROLL_BUTTON_Y,
	                  SCROLL_BUTTON_X + SCROLL_BUTTON_DX - 1,
	                  SCROLL_BUTTON_Y + SCROLL_BUTTON_DY - 1);
	bSuccess = (*m_pScrollButton).Create(nullptr, BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, StartRect, this, IDC_SCROLL);
	ASSERT(bSuccess);
	bSuccess = (*m_pScrollButton).LoadBitmaps(SCROLLUP, SCROLLDOWN, 0, 0);
	ASSERT(bSuccess);
	m_bIgnoreScrollClick = false;

	if (pGameInfo->bPlayingMetagame)                         // only in metamode
		pRibbon = FetchBitmap(pDC, nullptr, RIBBON);            // load bitmap for ribbon

	// Initialize the musician bitmaps and locations
	rectMusic[0].SetRect(VIOLIN_LOCATION_X, VIOLIN_LOCATION_Y,
	                     VIOLIN_LOCATION_X + VIOLIN_WIDTH, VIOLIN_LOCATION_Y + VIOLIN_HEIGHT);
	rectMusic[1].SetRect(CELLO_LOCATION_X, CELLO_LOCATION_Y,
	                     CELLO_LOCATION_X + CELLO_WIDTH, CELLO_LOCATION_Y + CELLO_HEIGHT);
	rectMusic[2].SetRect(DRUM_LOCATION_X, DRUM_LOCATION_Y,
	                     DRUM_LOCATION_X + DRUM_WIDTH, DRUM_LOCATION_Y + DRUM_HEIGHT);
	rectMusic[3].SetRect(SAX_LOCATION_X, SAX_LOCATION_Y,
	                     SAX_LOCATION_X + SAX_WIDTH, SAX_LOCATION_Y + SAX_HEIGHT);
	rectMusic[4].SetRect(HARP_LOCATION_X, HARP_LOCATION_Y,
	                     HARP_LOCATION_X + HARP_WIDTH, HARP_LOCATION_Y + HARP_HEIGHT);
	rectMusic[5].SetRect(CLARINET_LOCATION_X, CLARINET_LOCATION_Y,
	                     CLARINET_LOCATION_X + CLARINET_WIDTH, CLARINET_LOCATION_Y + CLARINET_HEIGHT);

	for (i = 0; i < MAX_BUTTONS; i++) {
		for (j = 0; j < 2; j++) {
			pMusicians[(i * 2) + j] = FetchResourceBitmap(pDC, nullptr, FIRST_MUSICIAN + (i * 2) + j);
		}
		pAnimSprite[i] = new CSprite;
		(*pAnimSprite[i]).SharePalette(pGamePalette);
		bSuccess = (*pAnimSprite[i]).LoadCels(pDC, cAnimName[i], nNumCels[i]);
		ASSERT(bSuccess);
		(*pAnimSprite[i]).SetMasked(false);
		(*pAnimSprite[i]).SetMobile(false);
	}

	// Set up the text on the sign to display the current series length
	StartRect.SetRect(SIGN_LOCATION_X, SIGN_LOCATION_Y,
	                  SIGN_LOCATION_X + SIGN_WIDTH, SIGN_LOCATION_Y + SIGN_HEIGHT);
	if ((m_pSignText = new CText()) != nullptr) {
		(*m_pSignText).SetupText(pDC, pGamePalette, &StartRect, JUSTIFY_CENTER);
	}

	(*pDC).SelectPalette(pOldPal, false);           // Select back the old palette
	ReleaseDC(pDC);

	//srand((unsigned) time(nullptr));                 // seed the random number generator

	m_bPlayGame = true;

	if (pGameInfo->bPlayingMetagame) {
		switch (pGameInfo->nSkillLevel) {
		case SKILLLEVEL_LOW:
			m_nNumButtons = 4;
			m_nSpeed = 6;           // Allegretto
			m_nWinCondition = LOW_WIN;
			break;
		case SKILLLEVEL_MEDIUM:
			m_nNumButtons = 5;
			m_nSpeed = 8;           // Vivace
			m_nWinCondition = MEDIUM_WIN;
			break;
		case SKILLLEVEL_HIGH:
			m_nNumButtons = 6;
			m_nSpeed = 10;          // Prestissimo
			m_nWinCondition = HIGH_WIN;
			break;
		} // end switch
	} else {
		m_nNumButtons = MAX_BUTTONS;    // 6
		m_nSpeed = 8;                   // Vivace
	}

	tempPlayGame = m_bPlayGame;
	tempNumButtons = m_nNumButtons;
	tempSpeed = m_nSpeed;

	m_bPlaying = false;

	GetNewSequence(MAX_SEQUENCE);                    // Get a random sequence
	nNoteCount = 1;                                 // Set the count to the first note
	nCheckCount = 0;                                // Haven't checked any input yet

	MSG lpmsg;                                      // Remove any messages in the queue before starting
	while (PeekMessage(&lpmsg, m_hWnd, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE)) ;     // prevent random 'hits'

	EndWaitCursor();

	if (pGameInfo->bPlayingMetagame == false) {
		ActivateButtons(m_nNumButtons, NOT_PLAYING);     // Activate the buttons
		PostMessage(WM_COMMAND, IDC_SCROLL, BN_CLICKED);         // Activate the Options dialog
	}

} //End of CMainWindow

void CMainWindow::initStatics() {
	m_pScrollButton = nullptr;
	pGamePalette = nullptr;
	bSuccess = false;
	m_bIgnoreScrollClick = false;
	bLDown = false;
	bPlayingBackSeries = false;
	pMusic = nullptr;
	Common::fill(pAnimSprite, pAnimSprite + MAX_BUTTONS, nullptr);
	Common::fill(pMusicians, pMusicians + (MAX_BUTTONS * 2), nullptr);
	m_nButID = 0;

	m_pSignText = nullptr;
	pRibbon = nullptr;
	m_bPlaying = false;
	m_bNewGame = false;
	m_bPlayGame = false;

	m_nSpeed = 0;
	m_nNumButtons = 0;
	m_nWinCondition = 0;

	tempPlayGame = false;
	tempSpeed = 0;
	tempNumButtons = 0;
	nNoteCount = 0;
	nCheckCount = 0;
	nSButFlag = MAX_BUTTONS;
	pNoteMarker = nullptr;
}

// OnPaint:
// This is called whenever Windows sends a WM_PAINT message.
// Note that creating a CPaintDC automatically does a BeginPaint and
// an EndPaint call is done when it is destroyed at the end of this
// function.  CPaintDC's constructor needs the window (this).
//
void CMainWindow::OnPaint() {
	PAINTSTRUCT lpPaint;

	InvalidateRect(nullptr, false);                    // invalidate the entire window
	BeginPaint(&lpPaint);
	SplashScreen();                                 // Paint the backdrop and scroll button
	ActivateButtons(m_nNumButtons, NOT_PLAYING);     // Activate the buttons
	EndPaint(&lpPaint);
}


// Paint the background art (splash screen) in the client area;
// and repaint the scroll button at the top
// called by both OnPaint and InitInstance.
void CMainWindow::SplashScreen() {
	CRect   rcDest;
	CRect   rcDIB;
	CDC     *pDC;
	CDibDoc myDoc;
	HDIB    hDIB;
	char    msg[4];

	pDC = GetDC();

	myDoc.OpenDocument(MAINSCREEN);

	hDIB = myDoc.GetHDIB();

	if (pDC && hDIB) {
		GetClientRect(rcDest);

		int cxDIB = (int) DIBWidth(hDIB);
		int cyDIB = (int) DIBHeight(hDIB);

		rcDIB.top = rcDIB.left = 0;
		rcDIB.right = cxDIB;
		rcDIB.bottom = cyDIB;
		PaintDIB((*pDC).m_hDC, &rcDest, hDIB, &rcDIB, pGamePalette);
	}

	if (m_bPlayGame) {
		Common::sprintf_s(msg, "%d", nNoteCount - 1);
		(*m_pSignText).DisplayString(pDC, msg, 32, FW_NORMAL, SIGN_COLOR);
		if (pGameInfo->bPlayingMetagame && (nNoteCount > m_nWinCondition)) {
			PaintMaskedBitmap(pDC, pGamePalette, pRibbon, RIBBON_X, RIBBON_Y);
		}
	}

	ReleaseDC(pDC);
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
	CDC     *pDC;
	CNote   *pNewNote;
	CSound  *pEffect = nullptr;

	KillTimer(PLAYER_TIMER);

	pDC = GetDC();

	if (HIWORD(lParam) == BN_CLICKED) {
		CRules  RulesDlg((CWnd *)this, RULES_TEXT, pGamePalette, pGameInfo->bSoundEffectsEnabled ? RULES_SOUND : nullptr);

		CMainMenu COptionsWind((CWnd *)this, pGamePalette,
		                       pGameInfo->bPlayingMetagame ? (NO_NEWGAME | NO_OPTIONS) : 0,
		                       GetSubOptions, RULES_TEXT, pGameInfo->bSoundEffectsEnabled ? RULES_SOUND : nullptr, pGameInfo) ;       // Construct Option dialog

		switch (wParam) {

		case IDC_START:                                 // And we're off!
			if (pGameInfo->bPlayingMetagame)
				wait_awhile(PAUSE_TIME);                 // Give the resources, etc. time to load
			m_bPlaying = true;
			m_bNewGame = true;
			pNewNote = CNote::GetNoteHead();            // Get the first note
			if (pNewNote) {                              // if list is not empty
				PlayBackSeries(nNoteCount);                  // Play the first note
				pNoteMarker = CNote::GetNoteHead();     // set checking pointer
				pNewNote = nullptr;                            // Stop pointing at the note list
				SetTimer(PLAYER_TIMER, TIME_LIMIT, nullptr);    // Give the player TIME_LIMIT to respond
			}                                           //...to head of list
			else                                            // no note chain,
				m_bPlaying = false;                         //...so we can't play
			break;

		case IDC_A:                                     // If a musician button was hit...
		case IDC_B:
		case IDC_C:
		case IDC_D:
		case IDC_E:
		case IDC_F:
			unsigned int    nButID, nHitID;
			char    msg[4];

			nHitID = wParam - IDC_A;                        // The musician buttons are consecutive
			//...get the index of the one pressed
			if (m_bPlayGame) {                           // If they're not just playing music
				if (pNoteMarker)                                 // If we're playing a game
					nButID = pNoteMarker->GetValue();           // Get the Index of the correct note
				else break;

				if (nButID == nHitID) {                          // If the one pressed is the correct note
					pNoteMarker = pNoteMarker->GetNextNote();   // Move to the next
					nCheckCount++;                              // Increment the number correct so far
				} else {                                        // They hit the wrong button :-(
					char buf[64];
					Common::sprintf_s(buf, "Longest series:  %d", nNoteCount - 1);
					if (pGameInfo->bSoundEffectsEnabled) {
						pEffect = new CSound((CWnd *)this, WRONG_SOUND,
						                     SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE);  //...Wave file, to delete itself
						(*pEffect).play();                                                      //...play the narration
					}
					MSG lpmsg;                                  // Clear out any extraneous mouse clicks
					while (PeekMessage(&lpmsg, m_hWnd, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE)) ;

					CMessageBox GameOverDlg((CWnd *)this, pGamePalette, "Wrong musician!", buf);
					CSound::waitWaveSounds();

					pNoteMarker = nullptr;
					m_bPlaying = false;
					m_bNewGame = false;
					CNote::FlushNoteList();                     // Flush the last series of notes
					if (pGameInfo->bPlayingMetagame)
						PostMessage(WM_CLOSE, 0, 0);         // and post a program exit
					break;
				}
				if (nCheckCount == nNoteCount) {                 // If they completed the series
					pNoteMarker = nullptr;
					if (nNoteCount == MAX_SEQUENCE) {
						nNoteCount++;                           // Hitting MAX increments NoteCount to MAX + 1
						Common::sprintf_s(msg, "%d", nNoteCount - 1);    //...so return is correct
						(*m_pSignText).DisplayString(pDC, msg, 32, FW_NORMAL, SIGN_COLOR);

						if (pGameInfo->bPlayingMetagame && (nNoteCount > m_nWinCondition)) {
							PaintMaskedBitmap(pDC, pGamePalette, pRibbon, RIBBON_X, RIBBON_Y);
						}

						if (pGameInfo->bSoundEffectsEnabled) {
							pEffect = new CSound((CWnd *)this, WIN_SOUND,
							                     SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE);  //...Wave file, to delete itself
							(*pEffect).play();                                                      //...play the narration
						}
						MSG lpmsg;
						while (PeekMessage(&lpmsg, m_hWnd, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE)) ;

						CMessageBox GameOverDlg((CWnd *)this, pGamePalette, "Game over", "You have won!");
						CSound::waitWaveSounds();
						m_bPlaying = false;
						m_bNewGame = false;
						CNote::FlushNoteList();
						if (pGameInfo->bPlayingMetagame)
							PostMessage(WM_CLOSE, 0, 0);         // and post a program exit
					} else {
						nNoteCount++;
						Common::sprintf_s(msg, "%d", nNoteCount - 1);
						(*m_pSignText).DisplayString(pDC, msg, 32, FW_NORMAL, SIGN_COLOR);

						if (pGameInfo->bPlayingMetagame && (nNoteCount > m_nWinCondition)) {
							PaintMaskedBitmap(pDC, pGamePalette, pRibbon, RIBBON_X, RIBBON_Y);
						}

						wait_awhile(PAUSE_TIME);                 // Pause before playing sequence
						nCheckCount = 0;                        // Reset checking counter
						if ((nNoteCount % INCREMENT_RATE == 0) && (m_nSpeed != MAX_SPEED))
							m_nSpeed++;                         // on multiples of the increment rate
						//...increase the speed by one

						PlayBackSeries(nNoteCount);
						MSG lpmsg;
						while (PeekMessage(&lpmsg, m_hWnd, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE)) ;
						SetTimer(PLAYER_TIMER, TIME_LIMIT, nullptr);    //Reset time limit
					}
					pNoteMarker = CNote::GetNoteHead();         //set checking pointer to head of list
				} else
					SetTimer(PLAYER_TIMER, TIME_LIMIT, nullptr);    //Reset time limit
			}

			break;

		case IDC_RULES:
			m_bIgnoreScrollClick = true;
			(*m_pScrollButton).SendMessage(BM_SETSTATE, true, 0L);

			CSound::waitWaveSounds();
			RulesDlg.DoModal();
			m_bIgnoreScrollClick = false;
			(*m_pScrollButton).SendMessage(BM_SETSTATE, false, 0L);
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

			case IDC_OPTIONS_NEWGAME:
				if (!pGameInfo->bPlayingMetagame)
					NewGame();
				(*m_pScrollButton).SendMessage(BM_SETSTATE, false, 0L);
				m_bIgnoreScrollClick = false;
				break;

			case IDC_OPTIONS_RETURN:
				(*m_pScrollButton).SendMessage(BM_SETSTATE, false, 0L);
				m_bIgnoreScrollClick = false;
				if (m_bPlayGame && m_bNewGame) {                         // playing repeat game & already
					wait_awhile(PAUSE_TIME);                             // started...pause for a second
					PostMessage(WM_COMMAND, IDC_START, BN_CLICKED);  // Activate the Options dialog
				}
				break;

			case IDC_OPTIONS_QUIT:                      // Quit button was clicked
				PostMessage(WM_CLOSE, 0, 0);         // and post a program exit
				return false;

			} //end switch(ComDlg.DoModal())

		} //end switch(wParam)
	} // end if

	ReleaseDC(pDC);
	(*this).SetFocus();                     // Reset focus back to the main window
	return true;
}

/*****************************************************************
 *
 *  OnLButtonDown
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Left mouse button processing function
 *
 *  FORMAL PARAMETERS:
 *
 *      unsigned int nFlags     Virtual key info
 *      CPoint point    Location of cursor
 *
 *  IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 *  IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 *  RETURN VALUE:
 *
 *      void
 *
 ****************************************************************/
void CMainWindow::OnLButtonDown(unsigned int nFlags, CPoint point) {
	CRect   rectTitle,
	        rectWoodsRight,
	        rectWoodsLeft,
	        rectSign,
	        rectBench;
	CDC     *pDC;
	CSound  *pEffect = nullptr;
	int     i,
	        nPick = 0;
	char    bufName[64];

	pDC = GetDC();

	rectTitle.SetRect(NEWGAME_LOCATION_X, NEWGAME_LOCATION_Y,
	                  NEWGAME_LOCATION_X + NEWGAME_WIDTH,
	                  NEWGAME_LOCATION_Y + NEWGAME_HEIGHT);

	rectWoodsRight.SetRect(WOODRIGHT_LOCATION_X, WOODRIGHT_LOCATION_Y,
	                       WOODRIGHT_LOCATION_X + WOODRIGHT_WIDTH,
	                       WOODRIGHT_LOCATION_Y + WOODRIGHT_HEIGHT);

	rectWoodsLeft.SetRect(WOODLEFT_LOCATION_X, WOODLEFT_LOCATION_Y,
	                      WOODLEFT_LOCATION_X + WOODLEFT_WIDTH,
	                      WOODLEFT_LOCATION_Y + WOODLEFT_HEIGHT);

	rectSign.SetRect(SIGN_LOCATION_X, SIGN_LOCATION_Y,
	                 SIGN_LOCATION_X + SIGN_WIDTH,
	                 SIGN_LOCATION_Y + SIGN_HEIGHT);

	rectBench.SetRect(BENCH_LOCATION_X, BENCH_LOCATION_Y,
	                  BENCH_LOCATION_X + BENCH_WIDTH,
	                  BENCH_LOCATION_Y + BENCH_HEIGHT);

	if (rectTitle.PtInRect(point) && (!pGameInfo->bPlayingMetagame))
		NewGame();                                                                  // Activate New Game
	else if (m_bNewGame && m_bPlaying) {
		for (i = 0; i < m_nNumButtons; i++) {
			if (rectMusic[i].PtInRect(point)) {
				bLDown = true;
				KillTimer(PLAYER_TIMER);                                             // don't time out
				m_nButID = i;
				StartAnimation();
				wait_awhile(10 * ((NUM_SPEEDS - MAX_SPEED) + SLOW_DOWN));
				return;
				//break;
			} // end if ptinrect
		} // end for
	} // end else if m_bPlaying
	else if (!m_bNewGame) {
//		if (pGameInfo->bPlayingMetagame)
		PostMessage(WM_COMMAND, IDC_START, BN_CLICKED);      // Activate the imaginary 'start' button
		return;
	}



	if (rectWoodsRight.PtInRect(point) || rectWoodsLeft.PtInRect(point)) {
		if (pGameInfo->bSoundEffectsEnabled) {
			KillTimer(PLAYER_TIMER);                                             // so it doesn't run out
			pEffect = new CSound((CWnd *)this, TREES_SOUND,
			                     SOUND_WAVE | SOUND_AUTODELETE); // Wave file, sync, to delete itself
			(*pEffect).play();                                                      // Play the sound
			if (m_bPlayGame && m_bNewGame)
				SetTimer(PLAYER_TIMER, TIME_LIMIT, nullptr);                        // Reset response time limit
		}
	} else if (rectSign.PtInRect(point)) {
		if (pGameInfo->bSoundEffectsEnabled) {
			KillTimer(PLAYER_TIMER);                                             // so it doesn't run out
			nPick = brand() % NUM_SIGN_SOUNDS;
			switch (nPick) {
			case 0:
				Common::sprintf_s(bufName, SIGN_1_SOUND);
				break;
			case 1:
				Common::sprintf_s(bufName, SIGN_2_SOUND);
				break;
			case 2:
				Common::sprintf_s(bufName, SIGN_3_SOUND);
				break;
			case 3:
				Common::sprintf_s(bufName, SIGN_4_SOUND);
				break;
			default:
				Common::sprintf_s(bufName, SIGN_5_SOUND);
				break;
			}
			pEffect = new CSound((CWnd *)this, bufName,
			                     SOUND_WAVE | SOUND_AUTODELETE);             // Wave file, to delete itself
			(*pEffect).play();                                                  // play the sound
			if (m_bPlayGame && m_bNewGame)
				SetTimer(PLAYER_TIMER, TIME_LIMIT, nullptr);                            // Reset response time limit
		}
	} else if (rectBench.PtInRect(point)) {
		if (pGameInfo->bSoundEffectsEnabled) {
			KillTimer(PLAYER_TIMER);                                             // so it doesn't run out
			pEffect = new CSound((CWnd *)this, BENCH_SOUND,
			                     SOUND_WAVE | SOUND_AUTODELETE);             // Wave file, to delete itself
			(*pEffect).play();                                                  // Play the sound
			if (m_bPlayGame && m_bNewGame)
				SetTimer(PLAYER_TIMER, TIME_LIMIT, nullptr);                            // Reset response time limit
		}
	}

	ReleaseDC(pDC);

}


void CMainWindow::OnLButtonUp(unsigned int nFlags, CPoint point) {
	if (m_bNewGame) {
		if (m_bPlaying && bLDown) {
			if (pAnimSprite[m_nButID] != nullptr) {
				StopAnimation();
				MFC::SendMessage(m_hWnd, WM_COMMAND, m_nButID + IDC_A, BN_CLICKED);          // Activate hit logic
			}
			bLDown = false;
		}
	}
}

void CMainWindow::OnRButtonDown(unsigned int nFlags, CPoint point) {

	if (!m_bNewGame) {
		PostMessage(WM_COMMAND, IDC_START, BN_CLICKED);      // Activate the imaginary 'start' button
	}

}

/*****************************************************************
 *
 *  OnMouseMove
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Mouse movement processing function
 *
 *  FORMAL PARAMETERS:
 *
 *      unsigned int nFlags     Virtual key info
 *      CPoint point    Location of cursor
 *
 *  IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 *  IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 *  RETURN VALUE:
 *
 *      void
 *
 ****************************************************************/
void CMainWindow::OnMouseMove(unsigned int nFlags, CPoint point) {
	if (bPlayingBackSeries) {
		SetCursor(LoadCursor(nullptr, IDC_WAIT));            // Refresh cursor object
	} else {
		SetCursor(LoadCursor(nullptr, IDC_ARROW));           // Refresh cursor object
	}

	CFrameWnd ::OnMouseMove(nFlags, point);
}


// OnChar and OnSysChar
// These functions are called when keyboard input generates a character.
//

void CMainWindow::OnChar(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	CFrameWnd::OnChar(nChar, nRepCnt, nFlags);  // default action
}

void CMainWindow::OnSysChar(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	if ((nChar == 'q') && (nFlags & 0x2000))        // terminate app on ALT-q
		PostMessage(WM_CLOSE, 0, 0);                 // *** remove later ***
	else
		CFrameWnd::OnChar(nChar, nRepCnt, nFlags);  // default action
}

void CMainWindow::OnSysKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	if ((nChar == VK_F4) && (nFlags & 0x2000))      // terminate app on ALT-q
		PostMessage(WM_CLOSE, 0, 0);                 // *** remove later ***
	else
		CFrameWnd::OnSysKeyDown(nChar, nRepCnt, nFlags);    // default action
}

void CMainWindow::OnKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	if (!bPlayingBackSeries) {
		if (nChar == VK_F1) {                                  // F1 key is hit
			SendMessage(WM_COMMAND, IDC_RULES, BN_CLICKED);      // Activate the Rules dialog
		} else if (nChar == VK_F2) {                         // F2 key is hit
			SendMessage(WM_COMMAND, IDC_SCROLL, BN_CLICKED);         // Activate the Options dialog
		}
	}
	/* one-plus of low priority
	    else if ((nFlags & 0x40) == 0) {                            // if the key was previously up

	        switch (nChar) {

	            case '1':
	                i = 4;
	                break;

	            case '2':
	                i = 0;
	                break;

	            case '3':
	                i = 1;
	                break;

	            case '4':
	                i = 2;
	                break;

	            case '5':
	                i = 3;
	                break;

	            case '6':
	                i = 5;
	                break;

	            default:
	                break;
	        } // end switch

	        if ((i < MAX_BUTTONS) && (m_bPlaying)) {
	            KillTimer( PLAYER_TIMER );                                          // don't time out
	            m_nButID = i;
	            StartAnimation();
	        }
	    } // end else switch
	*/
}
/* one-plus of low priority
void CMainWindow::OnKeyUp(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags)
{
    bool    bMusician = false;

        switch (nChar) {

            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
                bMusician = true;
                break;

            default:
                break;
        } // end switch

        if (bMusician && m_bPlaying) {
            if (pAnimSprite[m_nButID] != nullptr) {
                StopAnimation();
                ::SendMessage( m_hWnd, WM_COMMAND, m_nButID + IDC_A, BN_CLICKED );      // Activate hit logic
            }
        }

}
*/
/*****************************************************************
 *
 *  OnTimer
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Processes Timer events
 *
 *  FORMAL PARAMETERS:
 *
 *      unsigned int nIDEvent   The ID of the timer event activated
 *
 *  IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 *  IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 *  RETURN VALUE:
 *
 *      void
 *
 ****************************************************************/
void CMainWindow::OnTimer(uintptr nIDEvent) {
	CDC     *pDC = nullptr;
	CSound  *pEffect = nullptr;

	pDC = GetDC();

	switch (nIDEvent) {

	case PLAYER_TIMER: {
		char buf[64];
		Common::sprintf_s(buf, "Longest series:  %d", nNoteCount - 1);
		if (pAnimSprite[m_nButID] != nullptr)                                  // If there's an animation
			StopAnimation();                                        //...running, stop it
		KillTimer(nIDEvent);                                         // Stop this timer
		if (pGameInfo->bSoundEffectsEnabled) {
			pEffect = new CSound((CWnd *)this, SLOW_SOUND,
			                     SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE);  //...Wave file, to delete itself
			(*pEffect).play();                                                      //...play the narration
		}
		MSG lpmsg;
		while (PeekMessage(&lpmsg, m_hWnd, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE)) ;

		CMessageBox GameOverDlg((CWnd *)this, pGamePalette, "Time ran out!", buf);
		CSound::waitWaveSounds();
		pNoteMarker = nullptr;
		m_bPlaying = false;
		CNote::FlushNoteList();
		if (pGameInfo->bPlayingMetagame)
			PostMessage(WM_CLOSE, 0, 0);                         // and post a program exit
	}
	KillTimer(nIDEvent);                                         // Stop the timer
	break;

	case ANIM_TIMER:
		(*pAnimSprite[m_nButID]).PaintSprite(pDC, rectMusic[m_nButID].TopLeft().x - Offset[m_nButID].x,
		                                     rectMusic[m_nButID].TopLeft().y - Offset[m_nButID].y);
		break;

	default:
		CFrameWnd ::OnTimer(nIDEvent);
		KillTimer(nIDEvent);                         //Stop the timer
		break;
	} // end switch

	ReleaseDC(pDC);

}

void CMainWindow::OnActivate(unsigned int nState, CWnd *pWndOther, bool bMinimized) {

	if (!bMinimized) {
		switch (nState) {
		case WA_ACTIVE:
		case WA_CLICKACTIVE:
			//InvalidateRect(nullptr, false);
			break;

		default:
			break;
		}
	}

}

/**********************************************************
The following functions handle operations on the list of notes:
    Getting a new sequence by either:
        reading in a list from a file, or
        generating a random list,
    Adding a new member to the list, and
    Playing back the entire sequence.
***********************************************************/
void CMainWindow::NewGame() {
	CDC     *pDC;
	char    msg[4];

	pDC = GetDC();

	(*m_pScrollButton).SendMessage(BM_SETSTATE, false, 0L);
	m_bIgnoreScrollClick = false;

	m_bPlayGame = tempPlayGame;
	m_nNumButtons = tempNumButtons;
	m_nSpeed = tempSpeed;

	m_bNewGame = false;

	CNote::FlushNoteList();
	ActivateButtons(m_nNumButtons, NOT_PLAYING);             // Bring on the players!!

	if (m_bPlayGame) {
		GetNewSequence(MAX_SEQUENCE);
		nNoteCount = 1;
		nCheckCount = 0;
		Common::sprintf_s(msg, "%d", nNoteCount - 1);
		(*m_pSignText).DisplayString(pDC, msg, 32, FW_NORMAL, SIGN_COLOR);
	} else {
		m_bPlaying = true;                                  // Make sure we can play music
		Common::strcpy_s(msg, "");
		(*m_pSignText).DisplayString(pDC, msg, 32, FW_NORMAL, SIGN_COLOR);
	}

	ReleaseDC(pDC);
}

void CMainWindow::StartAnimation() {
	CDC     *pDC;

	pDC = GetDC();

	(*pAnimSprite[m_nButID]).SetCel(nNumCels[m_nButID]);
	SetTimer(ANIM_TIMER, ANIM_SLEEP, nullptr);

	if (pGameInfo->bSoundEffectsEnabled) {
		pMusic = new CSound((CWnd *)this, cSoundName[m_nButID],
		                    SOUND_MIDI | SOUND_ASYNCH |
		                    SOUND_LOOP | SOUND_NOTIFY);         //...Midi file, looping | SOUND_NOTIFY
		(*pMusic).play();                                           //...play the sound
	}

	(*pAnimSprite[m_nButID]).PaintSprite(pDC,
	                                     rectMusic[m_nButID].TopLeft().x - Offset[m_nButID].x,
	                                     rectMusic[m_nButID].TopLeft().y - Offset[m_nButID].y);

	ReleaseDC(pDC);
}


void CMainWindow::StopAnimation() {
	CDC     *pDC;

	pDC = GetDC();

	KillTimer(ANIM_TIMER);

	if (pAnimSprite[m_nButID] != nullptr) {
		pAnimSprite[m_nButID]->EraseSprite(pDC);
	}

	if (pMusic != nullptr) {
		(*pMusic).stop();
		delete pMusic;
		pMusic = nullptr;
	}

	CSound::clearSounds();

	PaintBitmap(pDC, pGamePalette, pMusicians[(m_nButID * 2) + NOT_PLAYING],
	            rectMusic[m_nButID].TopLeft().x, rectMusic[m_nButID].TopLeft().y);

	ReleaseDC(pDC);
}


bool CMainWindow::GetNewSequence(const char *pszFileName) {
	int  nNote;
	char note[5];

	ifstream IFStream(pszFileName);
	if (IFStream.fail()) {
		return false;
	}
	while (!IFStream.eof()) {
		IFStream.getline(note, sizeof(note));
		nNote = atoi(note);
		add_note_to_series(nNote);
	}
	return true;
}//end GetNewSequence()

bool CMainWindow::GetNewSequence(int nLength) {
	int i;
	for (i = nLength; i > 0; --i) {
		add_note_to_series(brand() % m_nNumButtons);
	}
	return true;
}//end GetNewSequence()

void CMainWindow::ActivateButtons(unsigned int nNumActive, bool bState) {
	CDC         *pDC;
	CPalette    *pOldPal = nullptr;
	unsigned int i;

	pDC = GetDC();
	pOldPal = pDC->SelectPalette(pGamePalette, false);           // select the game palette
	pDC->RealizePalette();                                          //...and realize it

	for (i = 0; i < nNumActive; i++) {
		PaintBitmap(pDC, pGamePalette, pMusicians[(i * 2) + bState],
		            rectMusic[i].TopLeft().x, rectMusic[i].TopLeft().y);
	}
	while (i < MAX_BUTTONS) {
		PaintBitmap(pDC, pGamePalette, pMusicians[(i * 2) + NOT_THERE],
		            rectMusic[i].TopLeft().x, rectMusic[i].TopLeft().y);
		i++;
	}

	(*pDC).SelectPalette(pOldPal, false);                        // Select back the old palette
	ReleaseDC(pDC);

}//end ActivateButtons()


void add_note_to_series(int nNewValue) {
	CNote *pNewNote;

	if ((pNewNote = new CNote()) == 0) {
		MessageBox(nullptr, "Could not create note!!", nullptr, MB_ICONEXCLAMATION);
	}
	(*pNewNote).SetValue(nNewValue);
	(*pNewNote).LinkNote();                 //Add the new note to the bottom of the list
	pNewNote = nullptr;
}//end add_note_to_series

void CMainWindow::PlayBackSeries(int nNumNotes) {
	CDC *pDC;
	int i;
	CNote *pNewNote;

	pDC = GetDC();

	bPlayingBackSeries = true;

//	HCURSOR HOldCursor = MFC::SetCursor( AfxGetApp ()->LoadStandardCursor( nullptr ) ); // Make cursor go away
	HCURSOR HOldCursor = MFC::SetCursor(LoadCursor(nullptr, IDC_WAIT));          // Refresh cursor object
	MFC::ShowCursor(true);

	pNewNote = CNote::GetNoteHead();
	for (i = nNumNotes; i > 0; --i) {
		if (pNewNote) {                                                              // If this isn't the end
			m_nButID = pNewNote->GetValue();                                        // Get the new note

			StartAnimation();
			wait_awhile(10 * ((NUM_SPEEDS - m_nSpeed) + SLOW_DOWN));
			StopAnimation();

			pNewNote = pNewNote->GetNextNote();                                     // And move to the next

			if (i != 1) wait_awhile(5 * (NUM_SPEEDS - m_nSpeed));                     // Process events while-you-wait!
		}
	}
	pNewNote = nullptr;

	MFC::ShowCursor(false);
	MFC::SetCursor(HOldCursor);

	bPlayingBackSeries = false;

	ReleaseDC(pDC);

}//end PlayBackSeries


bool CMainWindow::wait_awhile(int nHundSecs) {                      // Given time is in hundreths of sec
	uint32   goal;
	MSG     msg;

	goal = (nHundSecs * 10) + GetTickCount();                                // time is in millisecs
	while (goal > GetTickCount()) {
		pause();

		if (PeekMessage(&msg, m_hWnd, 0, WM_MOUSEMOVE, PM_REMOVE)) {    // Remove any messages except
			if (msg.message == WM_CLOSE || msg.message == WM_QUIT)      //...quit & close, which get
				break;                                                  //...sent to the queue
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (PeekMessage(&msg, m_hWnd, WM_PARENTNOTIFY, 0xFFFF, PM_REMOVE)) {
			if (msg.message == WM_CLOSE || msg.message == WM_QUIT)
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

	}                                   // spin yer wheels 'til nSecs pass

	return true;
}//end wait_awhile()

void CMainWindow::OnClose() {

	CDC     *pDC;
	CBrush  myBrush;
	CRect   myRect;
	int     i;

	if (pGameInfo->bPlayingMetagame)
		pGameInfo->lScore = (long)(nNoteCount - 1);     // Hitting MAX increments NoteCount to MAX + 1
	//...so return is correct

	pDC = GetDC();
	myRect.SetRect(0, 0, GAME_WIDTH, GAME_HEIGHT);
	myBrush.CreateStockObject(BLACK_BRUSH);
	(*pDC).FillRect(&myRect, &myBrush);
	ReleaseDC(pDC);

	CNote::FlushNoteList();                     // Delete list from memory
	CSound::clearSounds();                      // Make sure there's no sounds on return

	if (m_pSignText != nullptr)
		delete m_pSignText;

	for (i = 0; i < (MAX_BUTTONS * 2); i++) {
		if (pMusicians[i] != nullptr) {
			//pMusicians[i]->DeleteObject;
			delete pMusicians[i];               // Bitmap for Not_playing and Not_there
		}
	}

	for (i = 0; i < MAX_BUTTONS; i++) {
		if (pAnimSprite[i] != nullptr) {
			pAnimSprite[i]->EraseSprite(pDC);
			delete pAnimSprite[i];
			pAnimSprite[i] = nullptr;
		}
	}

	if (m_pScrollButton != nullptr)
		delete m_pScrollButton;

	if (pGamePalette != nullptr) {
		pGamePalette->DeleteObject();
		delete pGamePalette;
	}

	CFrameWnd::OnClose();

	MFC::PostMessage(ghParentWnd, WM_PARENTNOTIFY, WM_DESTROY, 0L);
}

void CALLBACK GetSubOptions(CWnd* pParentWind) {
	COptnDlg OptionsDlg(pParentWind, pGamePalette);      // Call Specific Game

	OptionsDlg.m_bPlayGame = m_bPlayGame;
	OptionsDlg.m_nNumButtons = m_nNumButtons;
	OptionsDlg.m_nSpeed = tempSpeed;

	if (OptionsDlg.DoModal() == IDOK) {          // save values set in dialog box
		tempPlayGame = OptionsDlg.m_bPlayGame;;          // get new time limit,
		tempNumButtons = OptionsDlg.m_nNumButtons;           //...new rows, and cols
		tempSpeed = OptionsDlg.m_nSpeed;
	}
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

// CMainWindow message map:
// Associate messages with member functions.
//
BEGIN_MESSAGE_MAP(CMainWindow, CFrameWnd)
	//{{AFX_MSG_MAP( CMainWindow )
	ON_WM_PAINT()
	ON_WM_CHAR()
	ON_WM_SYSCHAR()
	ON_WM_SYSKEYDOWN()
	ON_WM_KEYDOWN()
//	ON_WM_KEYUP()
	ON_WM_TIMER()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_CLOSE()
	ON_WM_ACTIVATE()
	ON_MESSAGE(MM_MCINOTIFY, CMainWindow::OnMCINotify)
	ON_MESSAGE(MM_WOM_DONE, CMainWindow::OnMMIONotify)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

} // namespace Garfunkle
} // namespace HodjNPodj
} // namespace Bagel
