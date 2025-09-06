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
#include "bagel/boflib/misc.h"
#include "bagel/hodjnpodj/dfa/dfa.h"
#include "bagel/hodjnpodj/dfa/dialogs.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {
namespace DFA {

#define SPLASHSPEC      ".\\ART\\DFA.BMP"

#ifdef BACKWARDSTIMER
	#define TIMERSPRITE     ".\\ART\\WATCHES2.BMP"
#else
	#define TIMERSPRITE     ".\\ART\\WATCHES.BMP"
#endif

//
// Cursor and mallet animation
//
#define MALLET          ".\\ART\\MALLET.BMP"
#define MALLETCELS      5
#define MALLET_WIDTH    32
#define MALLET_HEIGHT   32

#define RULESFILE       "DFA.TXT"

#define TIMERSPRITECELS 13
#define GAMETIMER       99

extern CMainDFAWindow   *pMainGameWnd;

CPalette    *pGamePalette = nullptr;       // Palette to be used throughout the game
CBmpButton  *pOptionButton = nullptr;      // Option button object for getting to the options dialog
CSprite     *pMalletSprite = nullptr;      // The cursor will be a sprite that animates on LButtonDown

CSprite     *pTimerSprite = nullptr;
int         nCurrentCel;

CSprite *apBeaverSprite[NUM_BEAVERS];
int     anBeaverShown[NUM_BEAVERS];

static  CSound  *pGameSound = nullptr;                             // Game theme song
int     nCurrentTimer;
bool    bEndGame;
bool    bResetGame;
bool    bStart;

/*****************************************************************
 *
 * CMainDFAWindow
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

CMainDFAWindow::CMainDFAWindow(HWND hCallingWnd, LPGAMESTRUCT lpGameStruct) :
		rNewGame(21, 3, 227, 20) {
	CDC         *pDC = nullptr;                        // device context for the screen
//	CPalette *pOldPalette = nullptr;
	CPalette    *pOldPal = nullptr;
	CString     WndClass;
	CSize       mySize;
	bool        bTestCreate, bTestDibDoc;           // bool for testing the creation of each button
	CText       atxtDisplayRow[NUMBEROFROWS];
	CDibDoc     *pDibDoc = nullptr;
	int         x, i;
	CString     BeaverFiles[NUM_BEAVERS];


	BeginWaitCursor();


	// Define a special window class which traps double-clicks, is byte aligned
	// to maximize BITBLT performance, and creates "owned" DCs rather than sharing
	// the five system defined DCs which are not guaranteed to be available;
	// this adds a bit to our app size but avoids hangs/freezes/lockups.

	WndClass = AfxRegisterWndClass(CS_DBLCLKS | CS_BYTEALIGNWINDOW | CS_OWNDC,
	                               nullptr, nullptr, nullptr);

	// set the seed for the random number generator
	//srand( (unsigned)time( nullptr ));

	// initialize private members
	m_lpGameStruct = lpGameStruct;
	m_lpGameStruct->lScore = 0L;

	m_hCallAppWnd = hCallingWnd;

	// load splash screen
	pDC = GetDC();                                      // get a device context for our window

	pDibDoc = new CDibDoc();                            // create an object to hold our splash screen
	ASSERT(pDibDoc);                                    // ... and verify we got it
	bTestDibDoc = pDibDoc->OpenDocument(SPLASHSPEC);    // next load in the actual DIB based artwork
	ASSERT(bTestDibDoc);
	pGamePalette = (*pDibDoc).DetachPalette();          // grab its palette and save it for later use
	delete pDibDoc;                                     // now discard the splash screen

	// set window coordinates to center game on screeen
	MainRect.left = (pDC->GetDeviceCaps(HORZRES) - GAME_WIDTH) >> 1;
	MainRect.top = (pDC->GetDeviceCaps(VERTRES) - GAME_HEIGHT) >> 1;
	MainRect.right = MainRect.left + GAME_WIDTH;    // determine where to place the game window
	MainRect.bottom = MainRect.top + GAME_HEIGHT;   // ... so it is centered on the screen

	pDC->SelectPalette(pOldPal, false);             // replace old palette
	ReleaseDC(pDC);                                 // release our window context


	// Create the window as a POPUP so that no boarders, title, or menu are present;
	// this is because the game's background art will fill the entire 640x40 area.
	Create(WndClass, "Boffo Games - Dam Furry Animals", WS_POPUP, MainRect, nullptr, 0);

	ClipCursor(&MainRect);

	//
	// create button
	//
	pOptionButton = new CBmpButton;         // create the Options button
	ASSERT(pOptionButton);
	OptionRect.SetRect(OPTION_LEFT,
	                   OPTION_TOP,
	                   OPTION_LEFT + OPTION_WIDTH,
	                   OPTION_TOP + OPTION_HEIGHT);
	bTestCreate = pOptionButton->Create("Options", BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, OptionRect, this, IDC_OPTION);
	ASSERT(bTestCreate != 0);                 // test for button's creation
	bTestCreate = pOptionButton->LoadBitmaps(SCROLLUP, SCROLLDOWN, SCROLLUP, SCROLLUP);
	ASSERT(bTestCreate != 0);                 // test for button's creation

	// Since we have the screen & button, let's put it up to amuse the player-in-waiting
	m_nTimeForGame = 0;                 // will be checked in SplashScreen(), so initialize
	ShowWindow(SW_SHOWNORMAL);
	UpdateWindow();

	aHitFile[0] = "OwOneSound";
	aHitFile[1] = "OwTwoSound";
	aHitFile[2] = "OwThreeSound";
	aHitFile[3] = "OwFourSound";
	aHitFile[4] = "OwFiveSound";
	aHitFile[5] = "OwSixSound";
	aHitFile[6] = "OwSevenSound";
	aHitFile[7] = "OwEightSound";
	aHitFile[8] = "OwNineSound";
	aHitFile[9] = "OwTenSound";
	aHitFile[10] = "OwElevenSound";

	aMissFile[0] = "MissOneSound";
	aMissFile[1] = "MissTwoSound";
	aMissFile[2] = "MissTwoSound";

	for (i = 0; i < NUM_HIT_SOUNDS; i++) {
		m_pHitSound[i] = nullptr;
		m_hHitRes[i] = nullptr;
	}

	for (i = 0; i < NUM_MISS_SOUNDS; i++) {
		m_pMissSound[i] = nullptr;
		m_hMissRes[i] = nullptr;
	}

	// WORKAROUND: Original uses pDC after calling ReleaseDC
	pDC = GetDC();

	pTimerSprite = new CSprite;
	pTimerSprite->SharePalette(pGamePalette);
	bTestCreate = pTimerSprite->LoadCels(pDC, TIMERSPRITE, TIMERSPRITECELS);
	ASSERT(bTestCreate);                    // test for sprite's creation
	pTimerSprite->SetMasked(true);
	pTimerSprite->SetMobile(true);
	pTimerSprite->SetOptimizeSpeed(true);

	//
	//  Set up the Mallet
	//
	pMalletSprite = new CSprite;
	pMalletSprite->SharePalette(pGamePalette);

	bTestCreate = pMalletSprite->LoadCels(pDC, MALLET, MALLETCELS);
	ASSERT(bTestCreate);                    // test for sprite's creation

	pMalletSprite->SetMasked(true);
	pMalletSprite->SetMobile(true);
	pMalletSprite->SetOptimizeSpeed(true);
	pMalletSprite->SetHotspot(0, 12);            // Set HotSpot to center of Sprite
	pMalletSprite->LinkSprite();
	pMalletSprite->SetZOrder(SPRITE_TOPMOST);

	BeaverFiles[0] = BEAVER1;
	BeaverFiles[1] = BEAVER2;
	BeaverFiles[2] = BEAVER3;
	BeaverFiles[3] = BEAVER4;
	BeaverFiles[4] = BEAVER5;
	BeaverFiles[5] = BEAVER6;
	BeaverFiles[6] = BEAVER7;

	for (x = 0; x < NUM_BEAVERS; x++) {
		apBeaverSprite[x] = nullptr;
		apBeaverSprite[x] = new CSprite;
		apBeaverSprite[x]->SharePalette(pGamePalette);
		bTestCreate = apBeaverSprite[x]->LoadSprite(pDC, BeaverFiles[x]);
		ASSERT(bTestCreate);                    // test for sprite's creation
		apBeaverSprite[x]->SetMasked(true);
		apBeaverSprite[x]->SetMobile(true);
		apBeaverSprite[x]->SetOptimizeSpeed(true);
		apBeaverSprite[x]->LinkSprite();
		apBeaverSprite[x]->SetZOrder(SPRITE_BACKGROUND);
		anBeaverShown[x] = 0;
	}

	arBeaver[0].SetRect(93, 190, 179, 2276);
	arBeaver[1].SetRect(47, 289, 133, 375);
	arBeaver[2].SetRect(198, 242, 284, 328);
	arBeaver[3].SetRect(331, 196, 417, 282);
	arBeaver[4].SetRect(421, 265, 507, 351);
	arBeaver[5].SetRect(471, 154, 557, 240);
	arBeaver[6].SetRect(558, 216, 623, 302);

	//
	// Set up artwork-area
	//
	ArtRect.SetRect(0, TOP_BORDER, GAME_WIDTH,  GAME_HEIGHT - TOP_BORDER);

	bResetGame = false;
	bEndGame = false;
	m_lScore = 0L;

	if (m_lpGameStruct->bPlayingMetagame == true) {
		switch (m_lpGameStruct->nSkillLevel) {

		case SKILLLEVEL_LOW:
			m_nTimeForGame = 30;
			m_nBeaverDuration = 4;
			break;
		case SKILLLEVEL_MEDIUM:
			m_nTimeForGame = 30;
			m_nBeaverDuration = 3;
			break;
		case SKILLLEVEL_HIGH:
			m_nTimeForGame = 30;
			m_nBeaverDuration = 2;
			break;
		}

		nCurrentCel = -1;
		bStart = true;
	} else {
		m_nTimeForGame = 30;
		m_nBeaverDuration = 3;
		nCurrentCel = -1;
		bStart = false;
	}

	if (LoadBeaverSounds() == false) {
		MessageBox("Could not load sound files.", "Error in Dam Furry Animals", MB_ICONINFORMATION | MB_OK);
	}

	pGameSound = new CSound(this, GAME_THEME, SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END);
	if (m_lpGameStruct->bMusicEnabled) {
		if (pGameSound != nullptr)
			(*pGameSound).midiLoopPlaySegment(1480, 30700, 0, FMT_MILLISEC);
	} // end if pGameSound


	EndWaitCursor();
	if (m_lpGameStruct->bPlayingMetagame)
		SetTimer(GAMETIMER, 500, nullptr);

	// WORKAROUND: Free re-allocated DC
	ReleaseDC(pDC);
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
void CMainDFAWindow::OnPaint() {
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
void CMainDFAWindow::SplashScreen() {
	CDC         *pDC = GetDC();                                                                          // get a device context for the window
	CPalette    *pOldPalette = pDC->SelectPalette(pGamePalette, false);      // load game palette;
	CDibDoc     myDoc;
	bool        bTestDibDoc;
	CRect       rcDest;         // defines where the art gets painted
	CRect       rcDIB;          // defines where the art comes from
	HDIB        hDIB;           // a handle to the DIB itself

	ASSERT(pDC);

	pDC->RealizePalette();                                          // realize game palette

	bTestDibDoc = myDoc.OpenDocument(SPLASHSPEC);
	ASSERT(bTestDibDoc);

	hDIB = myDoc.GetHDIB();                                         // ... get a handle to its DIB
	ASSERT(hDIB);

	pDC = GetDC();                                                  // get a device context for the window
	ASSERT(pDC);

	GetClientRect(rcDest);                                           // get the rectangle to where we paint

	int cxDIB = (int) DIBWidth(hDIB);
	int cyDIB = (int) DIBHeight(hDIB);
	
	rcDIB.top = rcDIB.left = 0;                                     // setup the source rectangle from which
	rcDIB.right = cxDIB;                                            // ... we'll do the painting
	rcDIB.bottom = cyDIB;
	PaintDIB((*pDC).m_hDC, &rcDest, hDIB, &rcDIB, pGamePalette);  // transfer the image to the screen

	if (m_nTimeForGame > 0) {
		pTimerSprite->SetCel(nCurrentCel);
		pTimerSprite->PaintSprite(pDC, WATCH_X, WATCH_Y);
	}

	pDC->SelectPalette(pOldPalette, false);                      // replace old palette
	ReleaseDC(pDC);                                                 // release the window's context

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

void CMainDFAWindow::ResetGame() {
	CDC *pDC = GetDC();
	int i;

	KillTimer(GAMETIMER);
	pTimerSprite->EraseSprite(pDC);
	for (i = 0; i < NUM_BEAVERS; i++) {
		if (anBeaverShown[i] != 0) {
			apBeaverSprite[i]->EraseSprite(pDC);
			anBeaverShown[i] = 0;
		}
	}
	nCurrentTimer = 0;
	bEndGame = false;
	m_lScore = 0L;
	ReleaseDC(pDC);
	nCurrentCel = -1;
	if (m_nTimeForGame > 0) {
		#ifdef BACKWARDSTIMER
		switch (m_nTimeForGame) {
		case 15:
			nCurrentCel = 8;
			break;
		case 30:
			nCurrentCel = 5;
			break;
		case 45:
			nCurrentCel = 2;
			break;
		case 60:
			nCurrentCel = -1;
			break;
		}
		#else
		nCurrentCel = -1;
		#endif
		pTimerSprite->SetCel(nCurrentCel);
		pTimerSprite->PaintSprite(pDC, WATCH_X, WATCH_Y);
	}
	SetTimer(GAMETIMER, 500, nullptr);

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

void CALLBACK lpfnOptionCallback(CWnd * pWnd) {
// do the mini options dialog
	int             nOption = 0;       // return from the Options dialog
//unsigned int          x = IDD_MINIOPTIONS_DIALOG;
	CDFAOptDlg      dlgMiniOptDlg(pWnd, pGamePalette, IDD_MINIOPTIONS_DIALOG);

	dlgMiniOptDlg.SetInitialOptions(pMainGameWnd->m_nTimeForGame, pMainGameWnd->m_nBeaverDuration);

	nOption = dlgMiniOptDlg.DoModal();

	if (nOption > 0) {
		pMainGameWnd->m_nBeaverDuration =  nOption / 1000;

		switch (nOption % 1000) {
		case 1:
			pMainGameWnd->m_nTimeForGame = 15;
			#ifdef BACKWARDSTIMER
			nCurrentCel = 8;
			#else
			nCurrentCel = -1;
			#endif
			break;
		case 2:
			pMainGameWnd->m_nTimeForGame = 30;
			#ifdef BACKWARDSTIMER
			nCurrentCel = 5;
			#else
			nCurrentCel = -1;
			#endif
			break;
		case 3:
			pMainGameWnd->m_nTimeForGame = 45;
			#ifdef BACKWARDSTIMER
			nCurrentCel = 2;
			#else
			nCurrentCel = -1;
			#endif
			break;
		case 4:
			pMainGameWnd->m_nTimeForGame = 60;
			#ifdef BACKWARDSTIMER
			nCurrentCel = -1;
			#else
			nCurrentCel = -1;
			#endif
			break;
		//      case 8:
		default:
			pMainGameWnd->m_nTimeForGame = 0;
			#ifdef BACKWARDSTIMER
			nCurrentCel = -1;
			#else
			nCurrentCel = -1;
			#endif
			break;
		}
	} // end if nOption > 0

	return;
}

bool CMainDFAWindow::OnCommand(WPARAM wParam, LPARAM lParam) {
	int nMainOption = 0;                        // return from the Options dialog

	if (HIWORD(lParam) == BN_CLICKED)   {       // only want to look at button clicks
		switch (wParam) {

		// Option button clicked, then put up the Options dialog
		case IDC_OPTION:
			KillTimer(GAMETIMER);

			CDC *pDC;
			pDC = GetDC();
			pMalletSprite->EraseSprite(pDC);
			SetCursor(LoadCursor(nullptr, IDC_ARROW));
			ReleaseDC(pDC);

			pOptionButton->EnableWindow(false);
			bResetGame = false;
			if (m_lpGameStruct->bPlayingMetagame == true) {
				CMainMenu       dlgMainOpts((CWnd *)this, pGamePalette, (NO_NEWGAME | NO_OPTIONS),
				                            lpfnOptionCallback, RULESFILE,
				                            (m_lpGameStruct->bSoundEffectsEnabled ? RULES_WAV : nullptr), m_lpGameStruct);
				nMainOption = dlgMainOpts.DoModal();
				switch (nMainOption) {
				case IDC_OPTIONS_QUIT:
					//      if Quit buttons was hit, quit
					PostMessage(WM_CLOSE, 0, 0);
					break;
				}
			} else {
				CMainMenu       dlgMainOpts((CWnd *)this, pGamePalette, 0,
				                            lpfnOptionCallback, RULESFILE,
				                            (m_lpGameStruct->bSoundEffectsEnabled ? RULES_WAV : nullptr), m_lpGameStruct);
				nMainOption = dlgMainOpts.DoModal();
				switch (nMainOption) {
				case IDC_OPTIONS_QUIT:
					// if Quit buttons was hit, quit
					PostMessage(WM_CLOSE, 0, 0);
					break;
				case IDC_OPTIONS_NEWGAME:
					// reset the game and start a new hand
					bResetGame = true;
					break;
				}
			}
			pOptionButton->EnableWindow(true);
			SetFocus();

			if ((m_lpGameStruct->bMusicEnabled == false) && (pGameSound != nullptr)) {
				if (pGameSound->playing())
					(*pGameSound).stop();
			} else if (m_lpGameStruct->bMusicEnabled == true) {
				if (pGameSound == nullptr) {
					pGameSound = new CSound(this, GAME_THEME,
					                        SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END);
				}
				if (pGameSound != nullptr) {
					(*pGameSound).midiLoopPlaySegment(1480, 30700, 0, FMT_MILLISEC);
				} // end if pGameSound
			}

			if (bResetGame && !m_lpGameStruct->bPlayingMetagame) {
				ResetGame();
			} else {
				if (m_nTimeForGame > 0) {
					pDC = GetDC();

					pTimerSprite->SetCel(nCurrentCel);
					pTimerSprite->PaintSprite(pDC, WATCH_X, WATCH_Y);
					ReleaseDC(pDC);
				}

				SetTimer(GAMETIMER, 500, nullptr);
			}
			break;
		}
	}

	(*this).SetFocus();                         // Reset focus back to the main window
	return true;
}

void CMainDFAWindow::OnRButtonDown(unsigned int nFlags, CPoint point) {
	if (bStart) {
		bStart = false;
	}
	CWnd::OnRButtonDown(nFlags, point);
}

void CMainDFAWindow::OnLButtonDown(unsigned int nFlags, CPoint point) {
	CSound  *pEffect = nullptr;
	CRect   rWatch,
	        rLake,
	        rMount,
	        rFlowers;
	int     x,
	        nPick;

	rWatch.SetRect(WATCH_X, WATCH_Y, WATCH_X + WATCH_DX, WATCH_Y + WATCH_DY);
	rLake.SetRect(LAKE_X, LAKE_Y, LAKE_X + LAKE_DX, LAKE_Y + LAKE_DY);
	rMount.SetRect(MOUNT_X, MOUNT_Y, MOUNT_X + MOUNT_DX, MOUNT_Y + MOUNT_DY);
	rFlowers.SetRect(FLOWERS_X, FLOWERS_Y, FLOWERS_X + FLOWERS_DX, FLOWERS_Y + FLOWERS_DY);

	if (rNewGame.PtInRect(point) && !m_lpGameStruct->bPlayingMetagame) {
		ResetGame();
		return;
	} else if (bStart) {
		bStart = false;
		return;
	}

	else if (!bEndGame && ArtRect.PtInRect(point)) {
		CDC *pDC;

		pDC = GetDC();

		pMalletSprite->SetCel(1);
		pMalletSprite->PaintSprite(pDC, pMalletSprite->GetPosition());

		ReleaseDC(pDC);
	}

	if (rWatch.PtInRect(point) && m_lpGameStruct->bSoundEffectsEnabled)   {
		CSound::clearWaveSounds();
		sndPlaySound(nullptr, 0);
		pEffect = new CSound((CWnd *)this, WATCH_WAV,
		                     SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE);  //...Wave file, to delete itself
		(*pEffect).play();                                                      //...play the narration
	}

	else if (rLake.PtInRect(point) && m_lpGameStruct->bSoundEffectsEnabled)   {
		CSound::clearWaveSounds();
		sndPlaySound(nullptr, 0);
		pEffect = new CSound((CWnd *)this, LAKE_WAV,
		                     SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE);  //...Wave file, to delete itself
		(*pEffect).play();                                                      //...play the narration
	}

	else if (rMount.PtInRect(point) && m_lpGameStruct->bSoundEffectsEnabled)  {
		CSound::clearWaveSounds();
		sndPlaySound(nullptr, 0);
		pEffect = new CSound((CWnd *)this, MOUNT_WAV,
		                     SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE);  //...Wave file, to delete itself
		(*pEffect).play();                                                      //...play the narration
	}

	else if (rFlowers.PtInRect(point) && m_lpGameStruct->bSoundEffectsEnabled)    {
		CSound::clearWaveSounds();
		sndPlaySound(nullptr, 0);
		pEffect = new CSound((CWnd *)this, BEE_WAV,
		                     SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE);  //...Wave file, to delete itself
		(*pEffect).play();                                                      //...play the narration
	}

	if (bEndGame)
		return;

	for (x = 0; x < NUM_BEAVERS; x ++) {                                 // for each beaver
		if (arBeaver[x].PtInRect(point)) {                        // see if it was hit

			if (anBeaverShown[x] > 0) {                          // if it was there at hit-time
				CDC *pDC = GetDC();
				if (m_lpGameStruct->bSoundEffectsEnabled) {          // Play the hit sound
					nPick = (brand() % NUM_HIT_SOUNDS);
					CSound::clearWaveSounds();
					sndPlaySound(m_pHitSound[nPick], SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
				} // end if

				apBeaverSprite[x]->EraseSprite(pDC);             // erase beaver
				anBeaverShown[x] = 0;
				ReleaseDC(pDC);
				m_lScore++;
			} // end if Shown

			else {                                              // no beaver there at hit-time
				if (m_lpGameStruct->bSoundEffectsEnabled) {          // Play the hit sound
					nPick = (brand() % NUM_MISS_SOUNDS);                // randomly select a miss sound
					CSound::clearWaveSounds();
					sndPlaySound(m_pMissSound[nPick], SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
				} // end if bSoundEffectsEnabled
			} // end else
		} // end if PtInRect
	}

	CWnd::OnLButtonDown(nFlags, point);
}

void CMainDFAWindow::OnMouseMove(unsigned int nFlags, CPoint point) {
	CDC     *pDC;

	pDC = GetDC();

	if (ArtRect.PtInRect(point) && !bEndGame) {
		SetCursor(LoadCursor(nullptr, nullptr));
		pMalletSprite->SetCel(-1);
		pMalletSprite->PaintSprite(pDC, point);
	} else {
		pMalletSprite->EraseSprite(pDC);
		SetCursor(LoadCursor(nullptr, IDC_ARROW));
	}

	ReleaseDC(pDC);

	CWnd::OnMouseMove(nFlags, point);
}

void CMainDFAWindow::OnLButtonUp(unsigned int nFlags, CPoint point) {

	if (!bEndGame && ArtRect.PtInRect(point)) {
		CDC *pDC;

		pDC = GetDC();

		pMalletSprite->SetCel(-1);
		pMalletSprite->PaintSprite(pDC, pMalletSprite->GetPosition());

		ReleaseDC(pDC);
	}

	CWnd::OnLButtonUp(nFlags, point);
}

void CMainDFAWindow::OnSysKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	// terminate app on ALT_F4
	//
	if ((nChar == VK_F4) && (nFlags & 0x2000)) {

		PostMessage(WM_CLOSE, 0, 0);

	} else {

		// default action
		CWnd::OnSysKeyDown(nChar, nRepCnt, nFlags);
	}
}


void CMainDFAWindow::OnKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	CRules  dlgRules((CWnd *)this, RULESFILE, pGamePalette,
	                 (m_lpGameStruct->bSoundEffectsEnabled ? RULES_WAV : nullptr));

	switch (nChar) {
	case VK_F1:
		KillTimer(GAMETIMER);                                        // stop the beavers for now
		CSound::waitWaveSounds();
		pOptionButton->ShowWindow(SW_HIDE);                          // hide furled scroll
		dlgRules.DoModal();                                  // invoke the help dialog box
		pOptionButton->ShowWindow(SW_SHOWNORMAL);                    // show furled scroll
		SetFocus();
		if (m_nTimeForGame > 0) {
			CDC *pDC = GetDC();

			pTimerSprite->SetCel(nCurrentCel);
			pTimerSprite->PaintSprite(pDC, WATCH_X, WATCH_Y);
			ReleaseDC(pDC);
		}

		SetTimer(GAMETIMER, 500, nullptr);
		break;
	case VK_F2:
		SendMessage(WM_COMMAND, IDC_OPTION, BN_CLICKED);
		break;
	default:
		CFrameWnd::OnKeyDown(nChar, nRepCnt, nFlags);
		break;
	}
	return;
}

void CMainDFAWindow::OnTimer(uintptr nWhichTimer) {
	CSound  *pEffect = nullptr;

	if (nWhichTimer == GAMETIMER) {
		CDC *pDC = GetDC();
		int x = brand() % NUM_BEAVERS;

		if (bStart) {
			ReleaseDC(pDC);
			return;
		}

		if (bEndGame) {
			ReleaseDC(pDC);
			return;
		}

		if (m_nTimeForGame > 0)
			nCurrentTimer++;
		else
			nCurrentTimer = 0;

		if (((nCurrentTimer % 10) == 0) && (m_nTimeForGame != 0)) {
			pDC = GetDC();

			nCurrentCel++;
			pTimerSprite->SetCel(nCurrentCel);
			/*          if (m_lpGameStruct->bSoundEffectsEnabled) {
			                pEffect = new CSound( (CWnd *)this, TICK_WAV,
			                                        SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE);  //...Wave file, to delete itself
			                (*pEffect).play();                                                      //...play the narration
			            }
			*/
			pTimerSprite->PaintSprite(pDC, WATCH_X, WATCH_Y);
			ReleaseDC(pDC);
		}

		if (nCurrentTimer > (int)(2 * m_nTimeForGame)) {
			CMsgDlg msgBox((CWnd*)this, pGamePalette);

			KillTimer(GAMETIMER);
			bEndGame = true;
			for (x = 0; x < NUM_BEAVERS; x++) {
				apBeaverSprite[x]->EraseSprite(pDC);
				anBeaverShown[x] = 0;
			}
			pMalletSprite->EraseSprite(pDC);                         // Get rid of color mallet cursor
			ReleaseDC(pDC);

			MSG lpmsg;                                      // Remove any messages in the queue before doing msgbox
			while (PeekMessage(&lpmsg, m_hWnd, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE)) ;     // prevent random 'hits'

			if (m_lpGameStruct->bSoundEffectsEnabled) {
				CSound::clearWaveSounds();
				sndPlaySound(nullptr, 0);
				pEffect = new CSound((CWnd *)this, TIME_WAV,
				                     SOUND_WAVE | SOUND_ASYNCH /*| SOUND_QUEUE*/ | SOUND_AUTODELETE);    //...Wave file, to delete itself
				(*pEffect).play();                                                      //...play the narration
			}

			msgBox.SetInitialOptions(1, m_lScore);

			while (PeekMessage(&lpmsg, m_hWnd, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE)) ;     // prevent random 'hits'

			msgBox.DoModal();
			if (m_lpGameStruct->bPlayingMetagame)
				PostMessage(WM_CLOSE, 0, 0);

			ReleaseDC(pDC);
			return;
		}

		if (anBeaverShown[x] == 0) {
			apBeaverSprite[x]->PaintSprite(pDC, arBeaver[x].left, arBeaver[x].top);
			anBeaverShown[x]++;
		}
		for (x = 0; x < NUM_BEAVERS; x++) {
			if (anBeaverShown[x] > m_nBeaverDuration) {
				apBeaverSprite[x]->EraseSprite(pDC);
				anBeaverShown[x] = 0;
			} else {
				if (anBeaverShown[x] > 0)
					anBeaverShown[x]++;
			}
		}

		ReleaseDC(pDC);
	}
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

bool CMainDFAWindow::OnEraseBkgnd(CDC *pDC) {
// eat this
	return true;
}

void CMainDFAWindow::OnActivate(unsigned int nState, CWnd   *pWndOther, bool bMinimized) {
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

void CMainDFAWindow::OnClose() {
	CDC *pDC = GetDC();
	CRect   rctFillRect(0, 0, 640, 480);
	CBrush  Brush(RGB(0, 0, 0));

	ClipCursor(nullptr);

	pDC->FillRect(&rctFillRect, &Brush);
	ReleaseDC(pDC);
	ReleaseResources();
	ReleaseBeaverSounds();

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
void CMainDFAWindow::OnDestroy() {
//  send a message to the calling app to tell it the user has quit the game
	m_lpGameStruct->lScore = m_lScore;
	MFC::PostMessage(m_hCallAppWnd, WM_PARENTNOTIFY, WM_DESTROY, (LPARAM)m_lpGameStruct);
	m_lpGameStruct = nullptr;
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

void CMainDFAWindow::ReleaseResources() {
	int x;

	if (pGameSound != nullptr) {
		delete pGameSound;                      // delete the game theme song
		pGameSound = nullptr;
	}

	CSound::clearSounds();

	if (pTimerSprite != nullptr) {
		delete pTimerSprite;
	}

	if (pMalletSprite != nullptr) {
		delete pMalletSprite;
	}

	for (x = 0; x < NUM_BEAVERS; x++) {
		if (apBeaverSprite[x] != nullptr)
			delete apBeaverSprite[x];
	}

	if (pGamePalette != nullptr) {
		pGamePalette->DeleteObject();         // release the game color palette
		delete pGamePalette;
	}

	delete pOptionButton;                                       // release the buttons we used
	return;
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

void CMainDFAWindow::FlushInputEvents() {
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

//////////// Additional Sound Notify routines //////////////

LRESULT CMainDFAWindow::OnMCINotify(WPARAM wParam, LPARAM lParam) {
	CSound  *pSound;

	pSound = CSound::OnMCIStopped(wParam, lParam);
	if (pSound != nullptr)
		OnSoundNotify(pSound);
	return 0;
}


LRESULT CMainDFAWindow::OnMMIONotify(WPARAM wParam, LPARAM lParam) {
	CSound  *pSound;

	pSound = CSound::OnMMIOStopped(wParam, lParam);
	if (pSound != nullptr)
		OnSoundNotify(pSound);
	return 0;
}

void CMainDFAWindow::OnSoundNotify(CSound *pSound) {
	//
	// Add your code to process explicit notification of a sound "done" event here.
	// pSound is a pointer to a CSound object for which you requested SOUND_NOTIFY.
	//
}

bool CMainDFAWindow::LoadBeaverSounds() {
	HANDLE      hResInfo;
	HINSTANCE   hInst;
	bool        bSuccess;
	int         i;
	// assume no error
	bSuccess = true;

	hInst = (HINSTANCE)GetWindowWord(m_hWnd, GWW_HINSTANCE);

	for (i = 0; i < NUM_HIT_SOUNDS; i++) {
		// Load and lock
		//
		if ((hResInfo = FindResource(hInst, aHitFile[i], "WAVE")) != nullptr) {
			if ((m_hHitRes[i] = LoadResource(hInst, (HRSRC)hResInfo)) != nullptr) {
				if ((m_pHitSound[i] = (char *)LockResource((HGLOBAL)m_hHitRes[i])) != nullptr) {
					// we have now loaded at least one of the master sounds
				} else bSuccess = false;
			} else bSuccess = false;
		} else bSuccess = false;
	}

	for (i = 0; i < NUM_MISS_SOUNDS; i++) {
		// Load and lock
		//
		if ((hResInfo = FindResource(hInst, aMissFile[i], "WAVE")) != nullptr) {
			if ((m_hMissRes[i] = LoadResource(hInst, (HRSRC)hResInfo)) != nullptr) {
				if ((m_pMissSound[i] = (char *)LockResource((HGLOBAL)m_hMissRes[i])) != nullptr) {
					// we have now loaded at least one of the master sounds
				} else bSuccess = false;
			} else bSuccess = false;
		} else bSuccess = false;
	}

	return bSuccess;

} // end LoadBeaverSounds

void CMainDFAWindow::ReleaseBeaverSounds() {
	int i;

	for (i = 0; i < NUM_HIT_SOUNDS; i++) {
		if (m_hHitRes[i] != nullptr) {
			FreeResource(m_hHitRes[i]);
			m_hHitRes[i] = nullptr;
		}
	}

	for (i = 0; i < NUM_MISS_SOUNDS; i++) {
		if (m_hMissRes[i] != nullptr) {
			FreeResource(m_hMissRes[i]);
			m_hMissRes[i] = nullptr;
		}
	}
}



// CMainDFAWindow message map:
// Associate messages with member functions.
//
BEGIN_MESSAGE_MAP(CMainDFAWindow, CFrameWnd)
	//{{AFX_MSG_MAP( CMainDFAWindow )
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	ON_WM_SYSKEYDOWN()
	ON_WM_KEYDOWN()
	ON_WM_ACTIVATE()
	ON_MESSAGE(MM_MCINOTIFY, CMainDFAWindow::OnMCINotify)
	ON_MESSAGE(MM_WOM_DONE, CMainDFAWindow::OnMMIONotify)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

} // namespace DFA
} // namespace HodjNPodj
} // namespace Bagel
