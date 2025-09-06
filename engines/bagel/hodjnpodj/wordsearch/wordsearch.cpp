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

#include "bagel/boflib/misc.h"
#include "bagel/hodjnpodj/hnplibs/stdafx.h"
#include "bagel/hodjnpodj/wordsearch/wordsearch.h"
#include "bagel/hodjnpodj/wordsearch/dialogs.h"
#include "bagel/hodjnpodj/wordsearch/wordlist.h"
#include "bagel/hodjnpodj/wordsearch/clongdlg.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {
namespace WordSearch {

extern CMainWSWindow    *pMainGameWnd;
extern HCURSOR          hGameCursor;

extern void PlayEasterEgg(CDC *pDC, CWnd *pWnd, CPalette *pPalette,
                          const char *pszAnimFile, const char *pszSoundFile,
                          int nNumCels, int nXLoc, int nYLoc, int nSleep, bool bPlaySound);

CPalette    *pGamePalette = nullptr;       // Palette to be used throughout the game
CBmpButton  *pOptionButton = nullptr;      // Option button object for getting to the options dialog


char    acWordChosen[WORDSPERLIST];
char    acWordBack[WORDSPERLIST];
POINT   ptCurrPosInGrid;
POINT   ptLastPosInGrid;
POINT   ptOrigPosInGrid;

bool    bResetGame;

CDC         *pOffScreenDC = nullptr;
CPalette    *pOldOffScreenPal = nullptr;
CBitmap     *pOffScreenBmp = nullptr;
CBitmap     *pOldOffScreenBmp = nullptr;
CBitmap     *pbmpSplashScreen = nullptr;

CBitmap     *pbmpAllLetters;
int         nCurrentLetter;

CText       *ptxtScore = nullptr;

CString *astrGameList[WORDSPERLIST];
CString *astrGameListDisplay[WORDSPERLIST];
CText *atxtDisplayWord[WORDSPERLIST];

static  CSound  *pGameSound = nullptr;                             // Game theme song

CSprite *pTimerSprite = nullptr;
int     nLastCell;
unsigned int    nTimerRes;
long    lCurrentTimer;

int     nWordsLeft;
int     nWordList;

char    acAlpha[26] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
                        'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'
                      };

extern const char *astrWLCat[NUMBEROFLISTS];
extern char acList[NUMBEROFLISTS][WORDSPERLIST][20];

/*****************************************************************
 *
 * CMainWSWindow
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

CMainWSWindow::CMainWSWindow(HWND hCallingWnd, LPGAMESTRUCT lpGameStruct) :
		rNewGame(21, 3, 227, 20),
		rRefreshRect(0, 0, GAME_WIDTH, 385),
		rScore(190, 340, 610, 365) {
	CDC     *pDC = nullptr;                        // device context for the screen
	CString WndClass;
	CSize   mySize;
	bool    bTestCreate;                        // bool for testing the creation of each button
	int     x, y;
	CText   atxtDisplayRow[NUMBEROFROWS];

	BeginWaitCursor();

	CPalette    *pOldPal = nullptr;

	// Define a special window class which traps double-clicks, is byte aligned
	// to maximize BITBLT performance, and creates "owned" DCs rather than sharing
	// the five system defined DCs which are not guaranteed to be available;
	// this adds a bit to our app size but avoids hangs/freezes/lockups.

	WndClass = AfxRegisterWndClass(CS_DBLCLKS | CS_BYTEALIGNWINDOW | CS_OWNDC,
	                               hGameCursor, nullptr, nullptr);

	// set the seed for the random number generator
	//srand( (unsigned)time( nullptr ));

	// initialize private members
	m_lpGameStruct = lpGameStruct;
	m_lpGameStruct->lScore = (WORDSPERLIST);

	m_hCallAppWnd = hCallingWnd;

	m_bWordsForwardOnly = false;

	// Initialize globals to point to class fields
	for (int i = 0; i < WORDSPERLIST; ++i) {
		astrGameList[i] = &_strGameList[i];
		astrGameListDisplay[i] = &_strGameListDisplay[i];
		atxtDisplayWord[i] = &_txtDisplayWord[i];
	}

	// load splash screen
	pDC = GetDC();                                  // get a device context for our window

	pbmpSplashScreen = new CBitmap();
	pbmpSplashScreen = FetchBitmap(pDC, &pGamePalette, SPLASHSPEC);      // get splash screen and game palettte

	pOldPal = pDC->SelectPalette(pGamePalette, false);   // load game palette
	pDC->RealizePalette();                                  // realize game palette

	// set window coordinates to center game on screeen
	MainRect.left = (pDC->GetDeviceCaps(HORZRES) - GAME_WIDTH) >> 1;
	MainRect.top = (pDC->GetDeviceCaps(VERTRES) - GAME_HEIGHT) >> 1;
	MainRect.right = MainRect.left + GAME_WIDTH;    // determine where to place the game window
	MainRect.bottom = MainRect.top + GAME_HEIGHT;   // ... so it is centered on the screen

	pDC->SelectPalette(pOldPal, false);             // replace old palette
	ReleaseDC(pDC);                                 // release our window context

	// Create the window as a POPUP so that no boarders, title, or menu are present;
	// this is because the game's background art will fill the entire 640x40 area.
	Create(WndClass, "Boffo Games - Word Search", WS_POPUP, MainRect, nullptr, 0);

	pDC = GetDC();
	pOldPal = pDC->SelectPalette(pGamePalette, false);   // load game palette
	pDC->RealizePalette();                                  // realize game palette

	pOptionButton = new CBmpButton;         // create the Options button
	ASSERT(pOptionButton != 0);
	OptionRect.SetRect(OPTION_LEFT,
	                   OPTION_TOP,
	                   OPTION_LEFT + OPTION_WIDTH,
	                   OPTION_TOP + OPTION_HEIGHT);
	bTestCreate = pOptionButton->Create("Options", BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, OptionRect, this, IDC_OPTION);
	ASSERT(bTestCreate != 0);                 // test for button's creation
	bTestCreate = pOptionButton->LoadBitmaps(SCROLLUP, SCROLLDOWN, SCROLLUP, SCROLLUP);
	ASSERT(bTestCreate != 0);                 // test for button's creation

	ShowWindow(SW_SHOWNORMAL);
	PaintBitmap(pDC, pGamePalette, pbmpSplashScreen, 0, 0, GAME_WIDTH, GAME_HEIGHT);

	pOffScreenBmp = new CBitmap();
	pOffScreenDC = new CDC();

	pOffScreenBmp->CreateCompatibleBitmap(pDC, GAME_WIDTH, GAME_HEIGHT);     // create offscreen bitmap
	pOffScreenDC->CreateCompatibleDC(pDC);               // create offscreen DC

	pOldOffScreenPal = pOffScreenDC->SelectPalette(pGamePalette, false);    // load in game palette
	pOldOffScreenBmp = pOffScreenDC->SelectObject(pOffScreenBmp);            // load in offscreen bitmap
	pOffScreenDC->RealizePalette();                                         // realize palette

	pbmpAllLetters = new CBitmap;
	//pbmpAllLetters = FetchBitmap( pDC, nullptr, ALLLETTERS );
	pbmpAllLetters = FetchBitmap(pOffScreenDC, nullptr, ALLLETTERS);
	ASSERT(pbmpAllLetters != 0);

	ptxtScore = new CText(pOffScreenDC, pGamePalette, &rScore, JUSTIFY_CENTER);

	pTimerSprite = new CSprite;
	pTimerSprite->SharePalette(pGamePalette);
	bTestCreate = pTimerSprite->LoadCels(pDC, TIMERSPRITE, TIMERSPRITECELS);
	ASSERT(bTestCreate);                    // test for sprite's creation
	pTimerSprite->SetPosition(FLOWER_X, FLOWER_Y);
	pTimerSprite->SetMasked(true);
	pTimerSprite->SetMobile(false);

	for (y = 0; y < WORDSPERLIST; y++) {
		arWordDisplay[y].SetRect(20, 50 + (y * 15), 170, 50 + (y * 15) + 14);
		_txtDisplayWord[y].SetupText(pOffScreenDC, pGamePalette, &arWordDisplay[y], JUSTIFY_RIGHT);
	}

	// paint splash screen to offscreen DC
	PaintBitmap(pOffScreenDC, pGamePalette, pbmpSplashScreen, 0, 0, GAME_WIDTH, GAME_HEIGHT);

	pDC->SelectPalette(pOldPal, false);             // replace old palette
	ReleaseDC(pDC);                                 // release our window context

	for (y = 0; y < NUMBEROFROWS; y++) {
		for (x = 0; x < NUMBEROFCOLS; x++) {
			arScreenGrid[y][x].SetRect(190 + (x * 21),      40 + (y * 21),
			                           190 + (x * 21) + 20,    40 + (y * 21) + 21);
		}
	}

	if (m_lpGameStruct->bPlayingMetagame == true) {
		pDC = GetDC();
		m_bShowWordList = false;
		if (m_lpGameStruct->nSkillLevel == SKILLLEVEL_LOW) {
			m_nTimeForGame = 75;
			m_bWordsForwardOnly = true;
		}
		if (m_lpGameStruct->nSkillLevel == SKILLLEVEL_MEDIUM) {
			m_nTimeForGame = 60;
			m_bWordsForwardOnly = true;
		}
		if (m_lpGameStruct->nSkillLevel == SKILLLEVEL_HIGH) {
			m_nTimeForGame = 60;
			m_bWordsForwardOnly = false;
		}

//	SetTimer( GAMETIMER, nTimerRes, nullptr );
		long    lTemp = (long)m_nTimeForGame * 1000;

//	nTimerRes = (unsigned int)( lTemp / ( TIMERSPRITECELS - 1) );
		nTimerRes = (unsigned int)(lTemp / (TIMERSPRITECELS - 2));
		nLastCell = -1;
		pTimerSprite->SetCel(nLastCell);
		ReleaseDC(pDC);
	} else {
		m_nTimeForGame = 300;
//	nTimerRes = (unsigned int)( (m_nTimeForGame * 1000) / ( TIMERSPRITECELS - 1) );
		nTimerRes = (unsigned int)((m_nTimeForGame * 1000) / (TIMERSPRITECELS - 2));
		nLastCell = -1;
		m_bShowWordList = true;
		m_bWordsForwardOnly  = false;
	}

//CreateNewGrid();

//SetWindowPos( &wndTopMost, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE );
// create buttons

	m_bMouseCaptured = false;
	bResetGame = false;
	lCurrentTimer = 1;

	nWordsLeft = WORDSPERLIST;

	if (m_lpGameStruct->bMusicEnabled) {
		pGameSound = new CSound(this, GAME_THEME,
		                        SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END);
		(*pGameSound).midiLoopPlaySegment(500, 31500, 0, FMT_MILLISEC);
	} // end if pGameSound

	EndWaitCursor();

	if (m_lpGameStruct->bPlayingMetagame) {
		CreateNewGrid();
		SplashScreen();
		SetTimer(GAMETIMER, nTimerRes, nullptr);
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
void CMainWSWindow::OnPaint() {
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
void CMainWSWindow::SplashScreen() {
	CPalette    *pOldPalette;
	CDC         *pDC = GetDC();                                                      // get a device context for the window

	ASSERT(pDC);

	pOldPalette = pDC->SelectPalette(pGamePalette, false);                   // load game palette
	pDC->RealizePalette();                                                      // realize game palette

	pTimerSprite->SetCel(nLastCell);
	pTimerSprite->PaintSprite(pOffScreenDC, FLOWER_X, FLOWER_Y);

	pDC->BitBlt(0, 0, GAME_WIDTH, GAME_HEIGHT, pOffScreenDC, 0, 0, SRCCOPY);     // Draw Word grid from offscreen

	pDC->SelectPalette(pOldPalette, false);     // replace old palette
	ReleaseDC(pDC);                             // release the window's context

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

void CMainWSWindow::ResetGame() {
	CDC *pDC;

	pDC = GetDC();
	nLastCell = -1;

	KillTimer(GAMETIMER);
	lCurrentTimer = 1;
	CreateNewGrid();
	SplashScreen();

	if (m_nTimeForGame > 0) {
		long    lTemp = (long)m_nTimeForGame * 1000;

		nTimerRes = (unsigned int)(lTemp / (TIMERSPRITECELS - 2));
		SetTimer(GAMETIMER, nTimerRes, nullptr);
	}

	ReleaseDC(pDC);
	return;
}

void CMainWSWindow::CreateNewGrid() {
	int     x, y, z;
	POINT   ptSPos;
	int     nDirection;
	char    cSpace = ' ';
	char    cTemp1[21];
	char    cTemp2[21];
	bool    bTemp1;
	bool    bTemp2;
	int     nWordLen;
	CText   atxtDisplayRow[NUMBEROFROWS];
	CString strTemp;
	int     nIterations1 = 0;
	int     nIterations2;
	CRect   rDisplayCat(190, 365, 610, 385);
	CText   txtDisplayCat(pOffScreenDC, pGamePalette, &rDisplayCat, JUSTIFY_CENTER);


	BeginWaitCursor();

	nWordsLeft = WORDSPERLIST;
	nWordList = brand() % NUMBEROFLISTS;
	nIterations2 = 0;

	for (y = 0; y < WORDSPERLIST; y++) {

		if (nIterations2 > 500) {
			nWordList++;
			nWordList %= NUMBEROFLISTS;         // keep nWordList in range of 0 to 49
			nIterations2 = 0;
		}

		if (y == 0) {
			for (x = 0; x < NUMBEROFROWS; x++) {
				for (z = 0; z < NUMBEROFCOLS; z++) {
					acGameGrid[x][z] = '\0';
				}
			}
			for (x = 0; x < WORDSPERLIST; x++) {
				_strGameList[x].Empty();
				_strGameListDisplay[x].Empty();
			}
		}

		for (x = 0; x < 21; x++) {
			cTemp1[x] = '\0';
			cTemp2[x] = '\0';
		}

		for (x = 0, z = 0; x < 20; x++, z++) {
			if (acList[nWordList][y][x] != cSpace) {
				cTemp1[z] = acList[nWordList][y][x];
			} else
				z--;
			cTemp2[x] = acList[nWordList][y][x];
		}

		_strGameList[y] = cTemp1;
		_strGameListDisplay[y] = cTemp2;

		nWordLen = strlen(cTemp1);
		bTemp1 = true;
		if (m_bWordsForwardOnly)
			nDirection = brand() % 4;
		else
			nDirection = brand() % 8;

		while (bTemp1) {
			nIterations1++;
			if (nIterations1 > 500) {
				nIterations1 = 0;
				nIterations2++;
				y = -1;
				bTemp1 = false;
				continue;
			}
			ptSPos.x = brand() % NUMBEROFCOLS;
			ptSPos.y = brand() % NUMBEROFROWS;
			if (m_bWordsForwardOnly)
				switch (nDirection) {
				case 0:
					nDirection = 0;
					break;
				case 1:
					nDirection = 1;
					break;
				case 2:
					nDirection = 2;
					break;
				case 3:
					nDirection = 7;
					break;
				} else {
				switch (nDirection) {
				case 0:
					nDirection = 2;
					break;
				case 1:
					nDirection = 3;
					break;
				case 2:
					nDirection = 4;
					break;
				case 3:
					nDirection = 5;
					break;
				case 4:
					nDirection = 6;
					break;
				case 5:
					nDirection = 7;
					break;
				case 6:
					nDirection = 0;
					break;
				case 7:
					nDirection = 1;
					break;
				}
			}
			switch (nDirection) {
			case 0: // going right
				if (ptSPos.x + nWordLen > NUMBEROFCOLS)
					continue;
				bTemp2 = true;
				for (z = 0; z < nWordLen; z++) {
					if ((acGameGrid[ptSPos.y][ptSPos.x + z] != '\0') &&
					        (acGameGrid[ptSPos.y][ptSPos.x + z] != cTemp1[z])) {
						bTemp2 = false;
					}
				}
				if (bTemp2 == false)
					continue;
				for (z = 0; z < nWordLen; z++) {
					acGameGrid[ptSPos.y][ptSPos.x + z] = cTemp1[z];
					bTemp1 = false;
				}
				break;
			case 1: // going down and right
				if (ptSPos.x + nWordLen > NUMBEROFCOLS)
					continue;
				if (ptSPos.y + nWordLen > NUMBEROFROWS)
					continue;
				bTemp2 = true;
				for (z = 0; z < nWordLen; z++) {
					if ((acGameGrid[ptSPos.y + z][ptSPos.x + z] != '\0') &&
					        (acGameGrid[ptSPos.y + z][ptSPos.x + z] != cTemp1[z])) {
						bTemp2 = false;
					}
				}
				if (bTemp2 == false)
					continue;
				for (z = 0; z < nWordLen; z++) {
					acGameGrid[ptSPos.y + z][ptSPos.x + z] = cTemp1[z];
					bTemp1 = false;
				}
				break;
			case 2: // going down
				if (ptSPos.y + nWordLen > NUMBEROFROWS)
					continue;
				bTemp2 = true;
				for (z = 0; z < nWordLen; z++) {
					if ((acGameGrid[ptSPos.y + z][ptSPos.x] != '\0') &&
					        (acGameGrid[ptSPos.y + z][ptSPos.x] != cTemp1[z])) {
						bTemp2 = false;
					}
				}
				if (bTemp2 == false)
					continue;
				for (z = 0; z < nWordLen; z++) {
					acGameGrid[ptSPos.y + z][ptSPos.x] = cTemp1[z];
					bTemp1 = false;
				}
				break;
			case 3: // going down and left
				if (ptSPos.y + nWordLen > NUMBEROFROWS)
					continue;
				if (ptSPos.x - nWordLen < 0)
					continue;
				bTemp2 = true;
				for (z = 0; z < nWordLen; z++) {
					if ((acGameGrid[ptSPos.y + z][ptSPos.x - z] != '\0') &&
					        (acGameGrid[ptSPos.y + z][ptSPos.x - z] != cTemp1[z])) {
						bTemp2 = false;
					}
				}
				if (bTemp2 == false)
					continue;
				for (z = 0; z < nWordLen; z++) {
					acGameGrid[ptSPos.y + z][ptSPos.x - z] = cTemp1[z];
					bTemp1 = false;
				}
				break;
			case 4: // going left
				if (ptSPos.x - nWordLen < 0)
					continue;
				bTemp2 = true;
				for (z = 0; z < nWordLen; z++) {
					if ((acGameGrid[ptSPos.y][ptSPos.x - z] != '\0') &&
					        (acGameGrid[ptSPos.y][ptSPos.x - z] != cTemp1[z])) {
						bTemp2 = false;
					}
				}
				if (bTemp2 == false)
					continue;
				for (z = 0; z < nWordLen; z++) {
					acGameGrid[ptSPos.y][ptSPos.x - z] = cTemp1[z];
					bTemp1 = false;
				}
				break;
			case 5: // going up and left
				if (ptSPos.x - nWordLen < 0)
					continue;
				if (ptSPos.y - nWordLen < 0)
					continue;
				bTemp2 = true;
				for (z = 0; z < nWordLen; z++) {
					if ((acGameGrid[ptSPos.y - z][ptSPos.x - z] != '\0') &&
					        (acGameGrid[ptSPos.y - z][ptSPos.x - z] != cTemp1[z])) {
						bTemp2 = false;
					}
				}
				if (bTemp2 == false)
					continue;
				for (z = 0; z < nWordLen; z++) {
					acGameGrid[ptSPos.y - z][ptSPos.x - z] = cTemp1[z];
					bTemp1 = false;
				}
				break;
			case 6: // going up
				if (ptSPos.y - nWordLen < 0)
					continue;
				bTemp2 = true;
				for (z = 0; z < nWordLen; z++) {
					if ((acGameGrid[ptSPos.y - z][ptSPos.x] != '\0') &&
					        (acGameGrid[ptSPos.y - z][ptSPos.x] != cTemp1[z])) {
						bTemp2 = false;
					}
				}
				if (bTemp2 == false)
					continue;
				for (z = 0; z < nWordLen; z++) {
					acGameGrid[ptSPos.y - z][ptSPos.x] = cTemp1[z];
					bTemp1 = false;
				}
				break;
			case 7: // going up and right
				if (ptSPos.x + nWordLen > NUMBEROFCOLS)
					continue;
				if (ptSPos.y - nWordLen < 0)
					continue;
				bTemp2 = true;
				for (z = 0; z < nWordLen; z++) {
					if ((acGameGrid[ptSPos.y - z][ptSPos.x + z] != '\0') &&
					        (acGameGrid[ptSPos.y - z][ptSPos.x + z] != cTemp1[z])) {
						bTemp2 = false;
					}
				}
				if (bTemp2 == false)
					continue;
				for (z = 0; z < nWordLen; z++) {
					acGameGrid[ptSPos.y - z][ptSPos.x + z] = cTemp1[z];
					bTemp1 = false;
				}
				break;
			}
		}
	}
	for (y = 0; y < NUMBEROFROWS; y++) {
		for (x = 0; x < NUMBEROFCOLS; x++) {
			if (acGameGrid[y][x] == '\0')
				acGameGrid[y][x] = acAlpha[(brand() % 26)];
		}
	}

	PaintBitmap(pOffScreenDC, pGamePalette, pbmpSplashScreen, 0, 0, GAME_WIDTH, GAME_HEIGHT);


	for (y = 0; y < NUMBEROFROWS; y++) {
		for (x = 0; x < NUMBEROFCOLS; x++) {
			int     a, b;
			CRect   rTemp;

			a = ((int)(acGameGrid[y][x]) - 65);
			b = 0;  // 21 if grey letter
			rTemp.SetRect((a * 21), (b), ((a * 21) + 20), (b + 20));

			BltMaskedBitmap(pOffScreenDC, pGamePalette, pbmpAllLetters,
			                &rTemp, arScreenGrid[y][x].left, arScreenGrid[y][x].top);

		}
	}

	strTemp = "Category: ";
	strTemp += astrWLCat[nWordList];
	txtDisplayCat.DisplayString(pOffScreenDC, strTemp, 16, FW_BOLD, DK_CYAN);   //Shadowed

	char    cDisplayTemp[32];
	Common::sprintf_s(cDisplayTemp, "Words Left: %i", nWordsLeft);
	ptxtScore->DisplayString(pOffScreenDC, cDisplayTemp, 16, FW_BOLD, DK_CYAN);    //Shadowed

	m_bNoGrid = false;
	EndWaitCursor();

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
	CWSOptDlg   dlgMiniOptDlg(pWnd, pGamePalette, IDD_MINIOPTIONS_DIALOG);

	dlgMiniOptDlg.SetInitialOptions(pMainGameWnd->m_nTimeForGame,
	                                pMainGameWnd->m_bShowWordList,
	                                pMainGameWnd->m_bWordsForwardOnly);

	nOption = dlgMiniOptDlg.DoModal();
	if (nOption > 0) {

		if (nOption >= 20000) {
			pMainGameWnd->m_bWordsForwardOnly = false;
			nOption -= 20000;
		} else {
			pMainGameWnd->m_bWordsForwardOnly = true;
			nOption -= 10000;
		}

		if (nOption >= 2000) {
			pMainGameWnd->m_bShowWordList = false;
			nOption -= 2000;
		} else {
			pMainGameWnd->m_bShowWordList = true;
			nOption -= 1000;
		}

		pMainGameWnd->m_nTimeForGame = nOption;
		bResetGame = true;
	}
	return;
}

bool CMainWSWindow::OnCommand(WPARAM wParam, LPARAM lParam) {
	int             nMainOption = 0;       // return from the Options dialog

	if (HIWORD(lParam) == BN_CLICKED)   {       // only want to look at button clicks
		switch (wParam) {

// Option button clicked, then put up the Options dialog
		case IDC_OPTION:
			pOptionButton->EnableWindow(false);
			bResetGame = false;
			KillTimer(GAMETIMER);
			if (m_lpGameStruct->bPlayingMetagame == true) {
				CMainMenu       dlgMainOpts((CWnd *)this, pGamePalette, (NO_NEWGAME | NO_OPTIONS),
				                            lpfnOptionCallback, RULESFILE,
				                            (m_lpGameStruct->bSoundEffectsEnabled ? RULES_WAV : nullptr),
				                            m_lpGameStruct);
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
				                            (m_lpGameStruct->bSoundEffectsEnabled ? RULES_WAV : nullptr),
				                            m_lpGameStruct);
				nMainOption = dlgMainOpts.DoModal();
				switch (nMainOption) {
				case IDC_OPTIONS_QUIT:
					//      if Quit buttons was hit, quit
					PostMessage(WM_CLOSE, 0, 0);
					break;
				case IDC_OPTIONS_NEWGAME:
					// reset the game and start a new hand
					bResetGame = true;
					break;
				}
			}
			pOptionButton->EnableWindow(true);
			//
			// Check to see if the music state was changed and adjust to match it
			//
			if ((m_lpGameStruct->bMusicEnabled == false) && (pGameSound != nullptr)) {
				if (pGameSound->playing())
					pGameSound->stop();
			} else if (m_lpGameStruct->bMusicEnabled) {
				if (pGameSound == nullptr) {
					pGameSound = new CSound(this, GAME_THEME,
					                        SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END);
				}
				if (pGameSound != nullptr) {
					if (!pGameSound->playing())
						(*pGameSound).midiLoopPlaySegment(500, 31500, 0, FMT_MILLISEC);
				}
			}
			if (bResetGame && !m_lpGameStruct->bPlayingMetagame)
				ResetGame();
			else if ((m_nTimeForGame > 0) && (m_bNoGrid == false)) {
				long    lTemp = (long)m_nTimeForGame * 1000;

				nTimerRes = (unsigned int)(lTemp / (TIMERSPRITECELS - 1));
//					nTimerRes = ( ( m_nTimeForGame * 1000 ) / TIMERSPRITECELS );
				SetTimer(GAMETIMER, nTimerRes, nullptr);
			}
			break;
		}
	}

	(*this).SetFocus();                         // Reset focus back to the main window
	return true;
}

void CMainWSWindow::OnLButtonDown(unsigned int nFlags, CPoint point) {
	CDC         *pDC = nullptr;
	CPalette    *pTempPal = nullptr;
	CRect       rChicken,
	            rPig,
	            rCow,
	            rFlower;
	int         x, y;

	rChicken.SetRect(CHICKEN_X, CHICKEN_Y, CHICKEN_X + CHICKEN_DX, CHICKEN_Y + CHICKEN_DY);
	rPig.SetRect(PIG_X, PIG_Y, PIG_X + PIG_DX, PIG_Y + PIG_DY);
	rCow.SetRect(COW_X, COW_Y, COW_X + COW_DX, COW_Y + COW_DY);
	rFlower.SetRect(FLOWER_X, FLOWER_Y, FLOWER_X + FLOWER_DX, FLOWER_Y + FLOWER_DY);

	if (m_lpGameStruct->bPlayingMetagame == false) {
		if (rNewGame.PtInRect(point))  {
			SetCapture();
			m_bMouseCaptured = true;
		}
	}

	pDC = GetDC();

	if (rChicken.PtInRect(point)) {
		CSound::waitWaveSounds();
		sndPlaySound(nullptr, 0);
		PlayEasterEgg(pDC, (CWnd *)this, pGamePalette, CHICKEN_ANIM, CHICKEN_WAV, NUM_CHICKEN_CELS,
		              CHICKEN_X, CHICKEN_Y, CHICKEN_SLEEP, (*m_lpGameStruct).bSoundEffectsEnabled);
	} else if (rCow.PtInRect(point)) {
		CSound::waitWaveSounds();
		sndPlaySound(nullptr, 0);
		PlayEasterEgg(pDC, (CWnd *)this, pGamePalette, COW_ANIM, COW_WAV, NUM_COW_CELS,
		              COW_X, COW_Y, COW_SLEEP, (*m_lpGameStruct).bSoundEffectsEnabled);
	} else if (rPig.PtInRect(point) && (*m_lpGameStruct).bSoundEffectsEnabled) {
		CSound::waitWaveSounds();
		sndPlaySound(nullptr, 0);
		sndPlaySound(PIG_WAV, SND_ASYNC);
	} else if (rFlower.PtInRect(point) && (*m_lpGameStruct).bSoundEffectsEnabled) {
		CSound::waitWaveSounds();
		sndPlaySound(nullptr, 0);
		sndPlaySound(FLOWER_WAV, SND_ASYNC);
	}

	ReleaseDC(pDC);

	if (m_bNoGrid)
		return;

	for (y = 0; y < NUMBEROFROWS; y++) {
		for (x = 0; x < NUMBEROFCOLS; x++) {
			if (arScreenGrid[y][x].PtInRect(point)) {
				CBrush cBrush((COLORREF)RGB(0, 255, 255));
				pDC = GetDC();
				pTempPal =  pDC->SelectPalette(pGamePalette, false);

				SetCapture();
				pDC->RealizePalette();
				m_bMouseCaptured = true;
				ptCurrPosInGrid.x = x;
				ptCurrPosInGrid.y = y;
				ptOrigPosInGrid.x = x;
				ptOrigPosInGrid.y = y;
				ptOrigPosInGrid.x = x;
				ptOrigPosInGrid.y = y;
				ptLastPosInGrid.x = x;
				ptLastPosInGrid.y = y;
				pDC->FrameRect(&arScreenGrid[y][x], &cBrush);
				pDC->SelectPalette(pTempPal, false);
				ReleaseDC(pDC);
			}
		}
	}

	CWnd::OnLButtonDown(nFlags, point);
}

void CMainWSWindow::OnMouseMove(unsigned int nFlags, CPoint point) {
	CDC     *pDC = nullptr;
	CBrush  cBrush((COLORREF)RGB(0, 255, 255));
	CPalette    *pTempPal = nullptr;
	CRgn    FrameRgn;
	POINT   aptFrame[6];

	int x, y;
	int nCurrentX, nCurrentY;

	SetCursor(LoadCursor(nullptr, IDC_ARROW));           // Refresh cursor object

	if (m_bNoGrid)
		return;

	if (m_bMouseCaptured) {
		for (int i = 0; i < 6; ++i)
			aptFrame[i].x = aptFrame[i].y = 0;

		for (y = 0; y < NUMBEROFROWS; y++) {
			for (x = 0; x < NUMBEROFCOLS; x++) {
				if (arScreenGrid[y][x].PtInRect(point)) {
					ptCurrPosInGrid.x = x;
					ptCurrPosInGrid.y = y;
				}
			}
		}
		if ((ptCurrPosInGrid.x != ptLastPosInGrid.x) ||
		        (ptCurrPosInGrid.y != ptLastPosInGrid.y)) {

			for (x = 0; x < WORDSPERLIST; x++)
				acWordChosen[x] = '\0';

			// draw vertical box
			if (ptCurrPosInGrid.x == ptOrigPosInGrid.x) {
				if (ptCurrPosInGrid.y >= ptOrigPosInGrid.y) {
					// start from the top and go down
					aptFrame[0].x = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].left;
					aptFrame[0].y = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].top;
					aptFrame[1].x = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].left;
					aptFrame[1].y = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].top;
					aptFrame[2].x = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].left;
					aptFrame[2].y = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].top;
					aptFrame[3].x = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].right;
					aptFrame[3].y = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].top;
					aptFrame[4].x = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].right;
					aptFrame[4].y = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].bottom;
					aptFrame[5].x = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].left;
					aptFrame[5].y = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].bottom;
					// put the selected word into acWordChosen
					for (y = ptOrigPosInGrid.y; y <= ptCurrPosInGrid.y; y++) {
						acWordChosen[ y - ptOrigPosInGrid.y ] = acGameGrid[y][ptOrigPosInGrid.x];
					}
				} else {
					// start from the bottom and go up
					aptFrame[0].x = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].left;
					aptFrame[0].y = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].top;
					aptFrame[1].x = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].left;
					aptFrame[1].y = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].top;
					aptFrame[2].x = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].left;
					aptFrame[2].y = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].top;
					aptFrame[3].x = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].right;
					aptFrame[3].y = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].top;
					aptFrame[4].x = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].right;
					aptFrame[4].y = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].bottom;
					aptFrame[5].x = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].left;
					aptFrame[5].y = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].bottom;
					// put the selected word into acWordChosen
					for (y = ptCurrPosInGrid.y; y <= ptOrigPosInGrid.y; y++) {
						acWordChosen[ y - ptCurrPosInGrid.y ] = acGameGrid[y][ptCurrPosInGrid.x];
					}
				}
			}

			// draw horizontal box
			if (ptCurrPosInGrid.y == ptOrigPosInGrid.y) {
				if (ptCurrPosInGrid.x >= ptOrigPosInGrid.x) {
					// start from the left and go right
					aptFrame[0].x = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].left;
					aptFrame[0].y = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].top;
					aptFrame[1].x = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].left;
					aptFrame[1].y = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].top;
					aptFrame[2].x = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].left;
					aptFrame[2].y = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].top;
					aptFrame[3].x = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].right;
					aptFrame[3].y = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].top;
					aptFrame[4].x = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].right;
					aptFrame[4].y = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].bottom;
					aptFrame[5].x = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].left;
					aptFrame[5].y = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].bottom;
					// put the selected word into acWordChosen
					for (x = ptOrigPosInGrid.x; x <= ptCurrPosInGrid.x; x++) {
						acWordChosen[ x - ptOrigPosInGrid.x ] = acGameGrid[ptOrigPosInGrid.y][x];
					}
				} else {
					// start from the right and go left
					aptFrame[0].x = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].left;
					aptFrame[0].y = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].top;
					aptFrame[1].x = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].left;
					aptFrame[1].y = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].top;
					aptFrame[2].x = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].left;
					aptFrame[2].y = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].top;
					aptFrame[3].x = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].right;
					aptFrame[3].y = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].top;
					aptFrame[4].x = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].right;
					aptFrame[4].y = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].bottom;
					aptFrame[5].x = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].left;
					aptFrame[5].y = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].bottom;
					// put the selected word into acWordChosen
					for (x = ptCurrPosInGrid.x; x <= ptOrigPosInGrid.x; x++) {
						acWordChosen[ x - ptCurrPosInGrid.x ] = acGameGrid[ptCurrPosInGrid.y][x];
					}
				}
			}

			nCurrentX = ptCurrPosInGrid.x - ptOrigPosInGrid.x;
			nCurrentY = ptCurrPosInGrid.y - ptOrigPosInGrid.y;

			if ((nCurrentX - nCurrentY) == 0) {  // diagnol with slope -1
				if (ptCurrPosInGrid.x > ptOrigPosInGrid.x) {
					aptFrame[0].x = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].left;
					aptFrame[0].y = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].top;
					aptFrame[1].x = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].right;
					aptFrame[1].y = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].top;
					aptFrame[2].x = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].right;
					aptFrame[2].y = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].top;
					aptFrame[3].x = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].right;
					aptFrame[3].y = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].bottom;
					aptFrame[4].x = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].left;
					aptFrame[4].y = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].bottom;
					aptFrame[5].x = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].left;
					aptFrame[5].y = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].bottom;
					for (x = ptOrigPosInGrid.x, y = ptOrigPosInGrid.y; x <= ptCurrPosInGrid.x; x++, y++) {
						acWordChosen[ x - ptOrigPosInGrid.x ] = acGameGrid[y][x];
					}
				} else {
					aptFrame[0].x = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].left;
					aptFrame[0].y = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].top;
					aptFrame[1].x = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].right;
					aptFrame[1].y = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].top;
					aptFrame[2].x = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].right;
					aptFrame[2].y = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].top;
					aptFrame[3].x = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].right;
					aptFrame[3].y = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].bottom;
					aptFrame[4].x = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].left;
					aptFrame[4].y = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].bottom;
					aptFrame[5].x = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].left;
					aptFrame[5].y = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].bottom;
					for (x = ptCurrPosInGrid.x, y = ptCurrPosInGrid.y; x <= ptOrigPosInGrid.x; x++, y++) {
						acWordChosen[ x - ptCurrPosInGrid.x ] = acGameGrid[y][x];
					}
				}
			} else {
				if ((nCurrentX + nCurrentY) == 0) {  // diagnol with slope 1
					if (ptCurrPosInGrid.x > ptOrigPosInGrid.x) {
						aptFrame[0].x = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].left;
						aptFrame[0].y = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].top;
						aptFrame[1].x = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].left;
						aptFrame[1].y = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].top;
						aptFrame[2].x = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].right;
						aptFrame[2].y = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].top;
						aptFrame[3].x = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].right;
						aptFrame[3].y = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].bottom;
						aptFrame[4].x = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].right;
						aptFrame[4].y = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].bottom;
						aptFrame[5].x = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].left;
						aptFrame[5].y = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].bottom;
						for (x = ptOrigPosInGrid.x, y = ptOrigPosInGrid.y; x <= ptCurrPosInGrid.x; x++, y--) {
							acWordChosen[ x - ptOrigPosInGrid.x ] = acGameGrid[y][x];
						}
					} else {
						aptFrame[0].x = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].left;
						aptFrame[0].y = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].top;
						aptFrame[1].x = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].left;
						aptFrame[1].y = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].top;
						aptFrame[2].x = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].right;
						aptFrame[2].y = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].top;
						aptFrame[3].x = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].right;
						aptFrame[3].y = arScreenGrid[ptOrigPosInGrid.y][ptOrigPosInGrid.x].bottom;
						aptFrame[4].x = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].right;
						aptFrame[4].y = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].bottom;
						aptFrame[5].x = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].left;
						aptFrame[5].y = arScreenGrid[ptCurrPosInGrid.y][ptCurrPosInGrid.x].bottom;
						for (x = ptCurrPosInGrid.x, y = ptCurrPosInGrid.y; x <= ptOrigPosInGrid.x; x++, y--) {
							acWordChosen[ x - ptCurrPosInGrid.x ] = acGameGrid[y][x];
						}
					}
				}
			}
//			redraw only what's necessary
			RedrawWindow(&rRefreshRect);

			pDC = GetDC();
			pTempPal = pDC->SelectPalette(pGamePalette, false);
			pDC->RealizePalette();
			FrameRgn.CreatePolygonRgn(aptFrame, 6, WINDING);
			pDC->FrameRgn(&FrameRgn, &cBrush, 1, 1);

		}
		ptLastPosInGrid.x = ptCurrPosInGrid.x;
		ptLastPosInGrid.y = ptCurrPosInGrid.y;
	}
	if (pDC != nullptr) {
		pDC->SelectPalette(pTempPal, false);
		ReleaseDC(pDC);
	}
	CWnd::OnMouseMove(nFlags, point);
}

void CMainWSWindow::OnLButtonUp(unsigned int nFlags, CPoint point) {
	int     x, y;
	POINT   ptTemp;
	bool    bFoundWord = false;
	int     nWordNum = 0;
	int     a, b;
	CRect   rTemp;


	if (m_bMouseCaptured) {
		m_bMouseCaptured = false;
		ReleaseCapture();
		if (rNewGame.PtInRect(point) && !m_lpGameStruct->bPlayingMetagame) {
			ResetGame();
			return;
		}

		if (m_bNoGrid)
			return;

		for (x = 0; x < WORDSPERLIST; x++)
			acWordBack[x] = '\0';

		if (acWordChosen[0] != '\0') {

			for (x = 0, y = (strlen(acWordChosen) - 1); y >= 0 ; x++, y--)
				acWordBack[x] = acWordChosen[y];

			for (x = 0; x < WORDSPERLIST; x++) {
				bFoundWord = false;
				nWordNum = 0;
				if (_strGameList[x].IsEmpty() == false) {
					if (_strGameList[x] == acWordChosen) {
						bFoundWord = true;
						nWordNum = x;
						break;
					} else {
						if (_strGameList[x] == acWordBack) {
							bFoundWord = true;
							nWordNum = x;
							break;
						}
					}
				}
			}

			if (bFoundWord) {
				if ((*m_lpGameStruct).bSoundEffectsEnabled) {
					sndPlaySound(nullptr, 0);
					sndPlaySound(FIND_WAV, SND_ASYNC);
				}
				_strGameList[nWordNum].Empty();
				nWordsLeft--;
				char    cDisplayTemp[32];
				Common::sprintf_s(cDisplayTemp, "Words Left: %i", nWordsLeft);
				ptxtScore->DisplayString(pOffScreenDC, cDisplayTemp, 16, FW_BOLD, DK_CYAN);

				ptTemp.x = ptOrigPosInGrid.x;
				ptTemp.y = ptOrigPosInGrid.y;

				a = ((int)(acGameGrid[ptTemp.y][ptTemp.x]) - 65);
				b = 22;
				rTemp.SetRect((a * 21), (b), ((a * 21) + 20), (b + 20));

				BltMaskedBitmap(pOffScreenDC, pGamePalette, pbmpAllLetters,
				                &rTemp,
				                arScreenGrid[ptTemp.y][ptTemp.x].left,
				                arScreenGrid[ptTemp.y][ptTemp.x].top);

				do {
					if ((ptCurrPosInGrid.y == ptOrigPosInGrid.y) && (ptCurrPosInGrid.x > ptOrigPosInGrid.x)) {
						//  word direction right
						ptTemp.x++;
					} else {
						if ((ptCurrPosInGrid.y > ptOrigPosInGrid.y) && (ptCurrPosInGrid.x > ptOrigPosInGrid.x)) {
							//  word direction down and right
							ptTemp.x++;
							ptTemp.y++;
						} else {
							if ((ptCurrPosInGrid.y > ptOrigPosInGrid.y) && (ptCurrPosInGrid.x == ptOrigPosInGrid.x)) {
								//  word direction down
								ptTemp.y++;
							} else {
								if ((ptCurrPosInGrid.y > ptOrigPosInGrid.y) && (ptCurrPosInGrid.x < ptOrigPosInGrid.x)) {
									//  word direction down and left
									ptTemp.x--;
									ptTemp.y++;
								} else {
									if ((ptCurrPosInGrid.y == ptOrigPosInGrid.y) && (ptCurrPosInGrid.x < ptOrigPosInGrid.x)) {
										//  word direction left
										ptTemp.x--;
									} else {
										if ((ptCurrPosInGrid.y < ptOrigPosInGrid.y) && (ptCurrPosInGrid.x < ptOrigPosInGrid.x)) {
											//  word direction up and left
											ptTemp.x--;
											ptTemp.y--;
										} else {
											if ((ptCurrPosInGrid.y < ptOrigPosInGrid.y) && (ptCurrPosInGrid.x == ptOrigPosInGrid.x)) {
												//  word direction up
												ptTemp.y--;
											} else {
												if ((ptCurrPosInGrid.y < ptOrigPosInGrid.y) && (ptCurrPosInGrid.x > ptOrigPosInGrid.x)) {
													//  word direction up and right
													ptTemp.x++;
													ptTemp.y--;
												}
											}
										}
									}
								}
							}
						}
					}

					a = ((int)(acGameGrid[ptTemp.y][ptTemp.x]) - 65);
					b = 22;
					rTemp.SetRect((a * 21), (b), ((a * 21) + 20), (b + 20));

					BltMaskedBitmap(pOffScreenDC, pGamePalette, pbmpAllLetters,
					                &rTemp,
					                arScreenGrid[ptTemp.y][ptTemp.x].left,
					                arScreenGrid[ptTemp.y][ptTemp.x].top);

				} while ((ptTemp.x != ptCurrPosInGrid.x) || (ptTemp.y != ptCurrPosInGrid.y));
			} // end if bWordFound
			else {
				if ((*m_lpGameStruct).bSoundEffectsEnabled) {
					sndPlaySound(nullptr, 0);
					sndPlaySound(NOPE_WAV, SND_SYNC);
					sndPlaySound(TRYAGAIN_WAV, SND_SYNC);
				}
			} // end else
		}

		RedrawWindow(&rRefreshRect);
		if (nWordsLeft == 0) {
			CMsgDlg     msgGameOver((CWnd *)this, pGamePalette);
			KillTimer(GAMETIMER);
			m_bNoGrid = true;
			if ((*m_lpGameStruct).bSoundEffectsEnabled) {
				sndPlaySound(nullptr, 0);
				sndPlaySound(ALLFOUND_WAV, SND_SYNC);            // play sound
			}
			msgGameOver.SetInitialOptions(1, nWordsLeft);
			msgGameOver.DoModal();
			if (m_lpGameStruct->bPlayingMetagame)
				PostMessage(WM_CLOSE, 0, 0);
		}

	}

	CWnd::OnLButtonUp(nFlags, point);
}

void CMainWSWindow::OnRButtonDown(unsigned int nFlags, CPoint point) {
	if (m_bShowWordList) {
		CLongDialog dlgWordlist((CWnd *)this, pGamePalette, "OK", "Word", "list");
		dlgWordlist.DoModal();
	}
}

void CMainWSWindow::OnSysKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	// terminate app on ALT_F4
	//
	if ((nChar == VK_F4) && (nFlags & 0x2000)) {

		PostMessage(WM_CLOSE, 0, 0);

	} else {

		// default action
		CWnd::OnSysKeyDown(nChar, nRepCnt, nFlags);
	}
}


void CMainWSWindow::OnKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	CRules  dlgRules((CWnd *)this, RULESFILE, pGamePalette, nullptr);

	switch (nChar) {
	case VK_F1:
		pOptionButton->ShowWindow(SW_HIDE);
//			UpdateWindow();
		CSound::waitWaveSounds();
		dlgRules.DoModal();      // invoke the help dialog box
		pOptionButton->ShowWindow(SW_SHOWNORMAL);
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

void CMainWSWindow::OnTimer(uintptr nWhichTimer) {
	if (nWhichTimer == GAMETIMER) {
		if (lCurrentTimer == (TIMERSPRITECELS - 1)) {        //> ( TIMERSPRITECELS - 1 ) ) {
			CDC *pDC = GetDC();
			CMsgDlg     msgGameOver((CWnd *)this, pGamePalette);

			KillTimer(GAMETIMER);
			nLastCell++;
			pTimerSprite->SetCel(nLastCell);
			pTimerSprite->PaintSprite(pDC, FLOWER_X, FLOWER_Y);

			pTimerSprite->SetCel(nLastCell);
			pTimerSprite->PaintSprite(pOffScreenDC, FLOWER_X, FLOWER_Y);

			ReleaseDC(pDC);
			m_bNoGrid = true;
			if ((*m_lpGameStruct).bSoundEffectsEnabled) {
				sndPlaySound(nullptr, 0);
				sndPlaySound(TIMEOUT_WAV, SND_SYNC);             // play sound
			}
			msgGameOver.SetInitialOptions(2, nWordsLeft);
			msgGameOver.DoModal();
			if (m_lpGameStruct->bPlayingMetagame)
				PostMessage(WM_CLOSE, 0, 0);
			else
				RedrawWindow();
		} else {
			CDC *pDC = GetDC();
			lCurrentTimer++;
			nLastCell++;
			if ((*m_lpGameStruct).bSoundEffectsEnabled) {
				sndPlaySound(nullptr, 0);
				sndPlaySound(TICK_WAV, SND_ASYNC);
			}
			pTimerSprite->SetCel(nLastCell);
			pTimerSprite->PaintSprite(pDC, FLOWER_X, FLOWER_Y);

			pTimerSprite->SetCel(nLastCell);
			pTimerSprite->PaintSprite(pOffScreenDC, FLOWER_X, FLOWER_Y);

			ReleaseDC(pDC);
		}
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

bool CMainWSWindow::OnEraseBkgnd(CDC *pDC) {
// eat this
	return true;
}

void CMainWSWindow::OnActivate(unsigned int nState, CWnd    *pWndOther, bool bMinimized) {
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

void CMainWSWindow::OnClose() {
	CDC *pDC = GetDC();
	CRect       rctFillRect(0, 0, GAME_WIDTH, GAME_HEIGHT);
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
void CMainWSWindow::OnDestroy() {
//  send a message to the calling app to tell it the user has quit the game
	m_lpGameStruct->lScore = (WORDSPERLIST - nWordsLeft);
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

void CMainWSWindow::ReleaseResources() {

	if (pGameSound != nullptr) {
		delete pGameSound;                      // delete the game theme song
		pGameSound = nullptr;
	}

	CSound::clearSounds();

	if (pTimerSprite != nullptr)
		delete pTimerSprite;

	if (pOldOffScreenBmp != nullptr)
		pOffScreenDC->SelectObject(pOldOffScreenBmp);

	if (pOldOffScreenPal != nullptr)
		pOffScreenDC->SelectPalette(pOldOffScreenPal, false);

	if (pOffScreenDC->m_hDC != nullptr) {
		pOffScreenDC->DeleteDC();
		delete pOffScreenDC;
	}

	if (pOffScreenBmp != nullptr) {
		pOffScreenBmp->DeleteObject();
		delete pOffScreenBmp;
	}

	if (pbmpSplashScreen != nullptr) {
		pbmpSplashScreen->DeleteObject();
		delete pbmpSplashScreen;
	}

	if (pbmpAllLetters != nullptr) {
		pbmpAllLetters->DeleteObject();
		delete pbmpAllLetters;
	}

	if (ptxtScore != nullptr)
		delete ptxtScore;

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

void CMainWSWindow::FlushInputEvents() {
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

LRESULT CMainWSWindow::OnMCINotify(WPARAM wParam, LPARAM lParam) {
	CSound  *pSound;

	pSound = CSound::OnMCIStopped(wParam, lParam);
	if (pSound != nullptr)
		OnSoundNotify(pSound);
	return 0;
}


LRESULT CMainWSWindow::OnMMIONotify(WPARAM wParam, LPARAM lParam) {
	CSound  *pSound;

	pSound = CSound::OnMMIOStopped(wParam, lParam);
	if (pSound != nullptr)
		OnSoundNotify(pSound);
	return 0;
}

void CMainWSWindow::OnSoundNotify(CSound *pSound) {
	//
	// Add your code to process explicit notification of a sound "done" event here.
	// pSound is a pointer to a CSound object for which you requested SOUND_NOTIFY.
	//
}

// CMainWSWindow message map:
// Associate messages with member functions.
//
BEGIN_MESSAGE_MAP(CMainWSWindow, CFrameWnd)
	//{{AFX_MSG_MAP( CMainWSWindow )
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	ON_WM_KEYDOWN()
	ON_WM_SYSKEYDOWN()
	ON_WM_ACTIVATE()
	ON_MESSAGE(MM_MCINOTIFY, CMainWSWindow::OnMCINotify)
	ON_MESSAGE(MM_WOM_DONE, CMainWSWindow::OnMMIONotify)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

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
	(*pSprite).SetCel(-1);           // nNumCels
	for (i = 0; i < nNumCels; i++) {
		(*pSprite).PaintSprite(pDC, nXLoc, nYLoc);
		Sleep(nSleep);
	}

	if (pSprite != nullptr)
		delete pSprite;

} // end PlayEasterEgg

} // namespace WordSearch
} // namespace HodjNPodj
} // namespace Bagel
