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
#include "bagel/hodjnpodj/hnplibs/dibdoc.h"
#include "bagel/hodjnpodj/hnplibs/stdinc.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/hnplibs/gamedll.h"
#include "bagel/hodjnpodj/hnplibs/sprite.h"
#include "bagel/hodjnpodj/hnplibs/mainmenu.h"
#include "bagel/hodjnpodj/hnplibs/cmessbox.h"
#include "bagel/hodjnpodj/hnplibs/bitmaps.h"
#include "bagel/boflib/misc.h"
#include "bagel/hodjnpodj/hnplibs/rules.h"
#include "bagel/boflib/error.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/hodjnpodj/novacancy/main.h"
#include "bagel/hodjnpodj/novacancy/dimens.h"
#include "bagel/boflib/sound.h"
#include "bagel/hodjnpodj/libs/macros.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {
namespace NoVacancy {

//
// Button ID constants
//
#define IDC_MENU     100
#define TIMER_ID 10

//
// Globals
//
static CBitmap *pCLDieBmp[7];
static CBitmap *pCRDieBmp[7];

static CSprite *pCDoorBmp[10];

CPalette    *pGamePalette;
LPGAMESTRUCT pGameParams;
const char *INI_SECTION = "No Vacancy";

static int gnLDieLeftFinal,    // final positions of dice.
       gnLDieTopFinal,
       gnRDieTopFinal,
       gnRDieLeftFinal;                   //these are set in AnimateDice

extern HWND ghParentWnd;

CMainWindow::CMainWindow() {
	CString  WndClass;
	CRect    tmpRect;
	CDibDoc *pDibDoc;
	ERROR_CODE errCode;
	bool bSuccess;
	short i;
	char szMapFile[256];

	// assume no error
	errCode = ERR_NONE;
	MFC::SetCursor(LoadCursor(nullptr, IDC_WAIT));     //hourglass cursor (wait!)

	/* Initialize members   */
	m_pScrollButton = nullptr;
	m_pGamePalette = nullptr;
	m_bIgnoreScrollClick = false;
	m_bSound = true;
	m_pCLRollingDie = nullptr;
	m_pCRRollingDie = nullptr;


	m_bGameLoadUp = true;           //this flag is reset to false as soon as the first paint message is processed.
	//It is used so that at the first paint message (as invoked from the UpdateWindow() call
	// in gamedll.cpp) whether or not Meta Game is being played,
	// the doors won't be painted when not a single door is closed as in the case of
	// game load up. (The splash screen has doors open all the way.)
	m_bPause = false;
	m_bGameActive = false;
	m_cActiveDoor = OPEN;           //  the game has just begun and no door is active. The 0-th door is always open.
	m_cDoorCount = 0;               //  no door is closed.

	m_cUnDoableThrows = 0;          //  no undoable throw has been registered yet.
	m_bDiceJustThrown = false;      // dice haven't been thrown yet
	m_bOneDieCase = false;          // always start w/ two dice on floor.

	// Set the coordinates for the "Start New Game" button
	//
	m_rNewGameButton.SetRect(15, 4, 233, 20);

	// Define a special window class which traps double-clicks, is byte aligned
	// to maximize BITBLT performance, and creates "owned" DCs rather than sharing
	// the five system defined DCs which are not guaranteed to be available;
	// this adds a bit to our app size but avoids hangs/freezes/lockups.
	WndClass = AfxRegisterWndClass(CS_DBLCLKS | CS_BYTEALIGNWINDOW | CS_OWNDC, nullptr, nullptr, nullptr);
	gWndClass = WndClass;

	// play this game if the background art is available
	Common::strcpy_s(szMapFile, GetStringFromResource(IDS_MINI_GAME_MAP));
	if (FileExists(szMapFile)) {
		// Acquire the shared palette for our game from the splash screen art
		if ((pDibDoc = new CDibDoc()) != nullptr) {
			if (pDibDoc->OpenDocument(szMapFile) != false)
				pGamePalette = m_pGamePalette = pDibDoc->DetachPalette();
			else
				errCode = ERR_UNKNOWN;
			delete pDibDoc;
			pDibDoc = nullptr;
		} else {
			errCode = ERR_MEMORY;
		}
	} else {
		errCode = ERR_FFIND;
	}

	// Center our window on the screen in the RELEASE mode.
	m_rectGameArea.SetRect(0, 0, GAME_WIDTH, GAME_HEIGHT);

	// Create the window as a POPUP so no boarders, title, or menu are present;
	// this is because the game's background art will fill the entire 640x480 area.
	if (!Create(WndClass, "Boffo Games -- No Vacancy", WS_POPUP | WS_CLIPCHILDREN, m_rectGameArea, nullptr, 0)) {
		errCode = ERR_UNKNOWN;
		MFC::SetCursor(LoadCursor(nullptr, IDC_ARROW));              //reset cursor
		HandleError(errCode);
		return ;
	}

	// only continue if there was no error
	if (errCode == ERR_NONE) {

		if ((m_pScrollButton = new CBmpButton) != nullptr) {

			m_bIgnoreScrollClick = false;
			tmpRect.SetRect(SCROLL_BUTTON_X, SCROLL_BUTTON_Y, SCROLL_BUTTON_X + SCROLL_BUTTON_DX, SCROLL_BUTTON_Y + SCROLL_BUTTON_DY);
			bSuccess = m_pScrollButton->Create(nullptr, BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, tmpRect, this, IDC_MENU);
			assert(bSuccess);
			if (bSuccess) {
				bSuccess = m_pScrollButton->LoadBitmaps(SCROLLUP, SCROLLDOWN, SCROLLUP, SCROLLUP);
				assert(bSuccess);
			} else {
				errCode = ERR_UNKNOWN;
			}
		} else {
			errCode = ERR_MEMORY;
		}
	}

	// Automatically bring up the main menu
	if ((errCode == ERR_NONE) && !pGameParams->bPlayingMetagame) {
		PostMessage(WM_COMMAND, IDC_MENU, BN_CLICKED);
	}//ELSE WAIT until UpdateWindow and SetActiveWindow, etc, are executed in GameDLL.cpp

	//
	// Give them something to look at while loading stuff
	//
	ShowWindow(SW_SHOWNORMAL);                           // Make window visible
	UpdateWindow();                                         // Send an OnPaint message to it

	// Set the coordinates for the Dice Locations.
	m_rLDie.SetRect(LDIE_LEFT, LDIE_TOP, LDIE_RIGHT, LDIE_BOTTOM);
	m_rRDie.SetRect(RDIE_LEFT, RDIE_TOP, RDIE_RIGHT, RDIE_BOTTOM);

	// Set the coordinates for the Doors.
	m_rDoor1.SetRect(DOOR1_LEFT, DOOR1_TOP, DOOR1_RIGHT, DOOR1_BOTTOM);
	m_rDoor[1].SetRect(DOOR1_LEFT, DOOR1_TOP, DOOR1_RIGHT, DOOR1_BOTTOM);
	m_rDoor[2].SetRect(DOOR2_LEFT, DOOR2_TOP, DOOR2_RIGHT, DOOR2_BOTTOM);
	m_rDoor[3].SetRect(DOOR3_LEFT, DOOR3_TOP, DOOR3_RIGHT, DOOR3_BOTTOM);
	m_rDoor[4].SetRect(DOOR4_LEFT, DOOR4_TOP, DOOR4_RIGHT, DOOR4_BOTTOM);
	m_rDoor[5].SetRect(DOOR5_LEFT, DOOR5_TOP, DOOR5_RIGHT, DOOR5_BOTTOM);
	m_rDoor[6].SetRect(DOOR6_LEFT, DOOR6_TOP, DOOR6_RIGHT, DOOR6_BOTTOM);
	m_rDoor[7].SetRect(DOOR7_LEFT, DOOR7_TOP, DOOR7_RIGHT, DOOR7_BOTTOM);
	m_rDoor[8].SetRect(DOOR8_LEFT, DOOR8_TOP, DOOR8_RIGHT, DOOR8_BOTTOM);
	m_rDoor[9].SetRect(DOOR9_LEFT, DOOR9_TOP, DOOR9_RIGHT, DOOR9_BOTTOM);

	//set coordinates for paper, bottle,etc.
	Paper.SetRect(PAPER_L, PAPER_T, PAPER_R, PAPER_B);
	aBrShoes.SetRect(aBRSHOES_L, aBRSHOES_T, aBRSHOES_R, aBRSHOES_B);
	bBrShoes.SetRect(bBRSHOES_L, bBRSHOES_T, bBRSHOES_R, bBRSHOES_B);
	BluShoes.SetRect(BLSHOES_L, BLSHOES_T, BLSHOES_R, BLSHOES_B);
	Bottle.SetRect(BOTTLE_L, BOTTLE_T, BOTTLE_R, BOTTLE_B);
	Glass.SetRect(GLASS_L, GLASS_T, GLASS_R, GLASS_B);    //  ... not used.
	UmbrStand.SetRect(STAND_L, STAND_T, STAND_R, STAND_B);
	Cat.SetRect(HAT4_L, HAT4_T, HAT4_R, HAT4_B);
	Hat6.SetRect(HAT6_L, HAT6_T, HAT6_R, HAT6_B);


	for (i = 0; i < 10; i++) {
		m_iDoorStatus[i] = OPEN; // every door is preset to open, the 0-th door is ALWAYS open.
	}

	/* preload dice cell sprites */
#pragma warning(disable: 4706)

	m_bDiceBmpsLoaded = false;

	memset(m_bDoorBmpLoaded, false, 10 * sizeof(bool));
	for (i = 1; i < 10; pCDoorBmp[i] = new CSprite(), i++);

#pragma warning(default: 4706)


	// only continue if there was no error
	if (errCode == ERR_NONE) {

		// seed the random number generator
		//srand((unsigned int)timeGetTime());

		//       begin playing bckgnd music when the game begins
		if (pGameParams->bMusicEnabled) {
			if ((m_psndBkgndMusic = new CSound(this, GetStringFromResource(IDS_MIDI_FILE), SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END))) {
				m_psndBkgndMusic->midiLoopPlaySegment(4000L, 31030L, 0L, FMT_MILLISEC);
			}//end if m_psndBkgndMusic=new...
		} else {
			m_psndBkgndMusic = nullptr;
		}

	}

	MFC::SetCursor(LoadCursor(nullptr, IDC_ARROW));
	HandleError(errCode);
}

void CMainWindow::HandleError(ERROR_CODE errCode) {
	// Exit this application on fatal errors
	if (errCode != ERR_NONE) {
		// pause the current game (if any)
		GamePause();

		// Display Error Message to the user
		MessageBox(errList[errCode], "Fatal Error!", MB_OK | MB_ICONSTOP);

		// Force this application to terminate
		PostMessage(WM_CLOSE, 0, 0);

		// Don't allow a repaint (remove all WM_PAINT messages)
		ValidateRect(nullptr);
	}
}


void CMainWindow::OnPaint() {
	PAINTSTRUCT lpPaint;

	Invalidate(false);
	BeginPaint(&lpPaint);

	PaintScreen();
	EndPaint(&lpPaint);
}


void CMainWindow::PaintScreen() {
	CDibDoc myDoc;
	CRect   rcDest;
	char szMapFile[256];

	/*nish adds*/
	CRect   rcDoor,
	        rcDie;
	CDibDoc dieDoc,
	        doorDoc;
	short ii;
	/*end nish add*/

	CRect   rcDIB;
	HDIB    hDIB;
	CDC     *pDC;


	//
	// Paint the background art and upadate any sprites
	// called by OnPaint
	//
	Common::strcpy_s(szMapFile, GetStringFromResource(IDS_MINI_GAME_MAP));
	if (FileExists(szMapFile)) {
		myDoc.OpenDocument(szMapFile);
		hDIB = myDoc.GetHDIB();

		pDC = GetDC();
		assert(pDC != nullptr);

		if (pDC && m_pGamePalette) {

			if (hDIB) {
				GetClientRect(rcDest);

				rcDIB.top = rcDIB.left = 0;
				rcDIB.right = (int) DIBWidth(hDIB);
				rcDIB.bottom = (int) DIBHeight(hDIB);

				PaintDIB(pDC->m_hDC, &rcDest, hDIB, &rcDIB, m_pGamePalette);
			}               //end-if (hDIB)

			if (m_bGameActive) {

				/* dont animate dice in the paint message, just paint 'em*/

				PaintMaskedBitmap(pDC, m_pGamePalette, pCLDieBmp[m_LDie], \
				                  m_rLDie.left, m_rLDie.top, (int) m_rLDie.Width(), (int) m_rLDie.Height());
				if (!m_bOneDieCase) {
					PaintMaskedBitmap(pDC, m_pGamePalette, pCRDieBmp[m_RDie], \
					                  m_rRDie.left, m_rRDie.top, (int) m_rRDie.Width(), (int) m_rRDie.Height());
				}

				/* paint doors except when open or upon GameLoadUp. */
				for (ii = 1; !m_bGameLoadUp && ii < 10; ii++) {                        //repaint all doors, except on start up.
					if (m_iDoorStatus[ii] == OPEN) continue;         //don't need to paint an open door.
					if (!m_bDoorBmpLoaded[ii]) {
						if (!pCDoorBmp[ii])                              // the door bmp might have been freed, because the door was locked.
							if ((pCDoorBmp[ii] = new CSprite())) // ...in which case load a new CSprite.
								pCDoorBmp[ii]->LoadCels(pDC, GetStringFromResource(IDS_D1 + ii - 1), NUM_DOOR_CELS);
						//else errCode=ERR_MEMORY;
						m_bDoorBmpLoaded[ii] = true;
					}
					if (m_iDoorStatus[ii] != OPEN) pCDoorBmp[ii]->SetCel(-1);                                     //paint the 1st cel.
					// /* bypassed because of line #1 in the for loop*/ else pCDoorBmp[ii]->SetCel(NUM_DOOR_CELS-2);   //paint the last cel.

					pCDoorBmp[ii]->PaintSprite(pDC, m_rDoor[ii].left, m_rDoor[ii].top);

				}//end for(ii)

				m_bGameJustBegun = false;
				m_bGameActive = true;
				ReleaseDC(pDC);
			}  //END -if (m_bGameActive)

			m_bGameLoadUp = false;                     // the next paint message is not a game load up paint.
		}  //end if(pDC && GamePalette)
	} //end if FileExists(MINIGAME_MAP)
}// end of function.


bool CMainWindow::OnCommand(WPARAM wParam, LPARAM lParam) {
	CMainMenu COptionsWind((CWnd *)this, m_pGamePalette, \
	                       (pGameParams->bPlayingMetagame ? NO_NEWGAME : 0X00) | (m_bGameActive ? 0X0 : NO_RETURN) | NO_OPTIONS, \
	                       nullptr, RULES,
	                       pGameParams->bSoundEffectsEnabled ? GetStringFromResource(IDS_RULES_WAV) : nullptr,
	                       pGameParams);           //DLL CHNG.

	if (HIWORD(lParam) == BN_CLICKED) {
		switch (wParam) {
		//
		// must bring up our menu of controls
		//
		case IDC_MENU:

			// hide the command scroll
			//
			m_pScrollButton->SendMessage(BM_SETSTATE, true, 0);

			if (!m_bIgnoreScrollClick) {

				m_bIgnoreScrollClick = true;

				GamePause();

				// Get users choice from command menu
				//
				switch (COptionsWind.DoModal()) {

				// User has chosen to play a new game; this is possible only in StandAlone mode.
				//
				case IDC_OPTIONS_NEWGAME:
					if (!pGameParams->bPlayingMetagame) PlayGame();

					break;

				// User has chosen to quit this mini-game
				//
				case IDC_OPTIONS_QUIT:
					PostMessage(WM_CLOSE, 0, 0);
					break;

				default:
					break;
				}

				// show the command scroll
				//
				m_pScrollButton->SendMessage(BM_SETSTATE, false, 0);
				m_bIgnoreScrollClick = false;

				//
				// Check to see if the music state was changed and adjust to match it
				//
				if ((pGameParams->bMusicEnabled == false) && (m_psndBkgndMusic != nullptr)) {
					if (m_psndBkgndMusic->playing())
						m_psndBkgndMusic->stop();
				} else if (pGameParams->bMusicEnabled) {
					if (m_psndBkgndMusic == nullptr) {
						m_psndBkgndMusic = new CSound(this, GetStringFromResource(IDS_MIDI_FILE), SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END);
					}
					if (m_psndBkgndMusic != nullptr) {
						if (!m_psndBkgndMusic->playing())
							m_psndBkgndMusic->midiLoopPlaySegment(4000L, 31030L, 0L, FMT_MILLISEC);
					}
				}

				GameResume();
			}

			return true;
		}
	}

	return false;
}


void CMainWindow::GamePause() {
	m_bPause = true;
};


void CMainWindow::GameResume() {
	m_bPause = false;
};

void CMainWindow::PlayGame() {
	ERROR_CODE errCode;
	CDibDoc myDoc;
	CRect   rcDest;
	CRect   rcDIB;
	HDIB    hDIB;
	CDC     *pDC;
	// assume no error
	errCode = ERR_NONE;
	HCURSOR hOldCursor;
	char szMapFile[256];
	CBitmap *pCMonolithDiceBmp = nullptr;

	//used to ExtractBitmap from the monolithic DiceBmp.
	int xDice[RIGHT + 1][7] = {{0, 60, 120, 180, 240, 300, 360}, {0, 58, 116, 174, 232, 290, 348}};
	int yDice[RIGHT + 1][7] = {{0, 0, 0, 0, 0, 0, 0}, {62, 62, 62, 62, 62, 62, 62}};
	int dxDice[RIGHT + 1][7] = {{60, 60, 60, 60, 60, 60, 60}, {57, 58, 58, 58, 58, 58, 58}};
	int dyDice[RIGHT + 1][7] = {{62, 62, 62, 62, 62, 62, 62}, {60, 61, 61, 61, 61, 61, 61}};


	TRACE("Starting New Game...");

	// reset all game parameters
	//
	GameReset();

	if (!m_bDiceBmpsLoaded) {
		hOldCursor = MFC::SetCursor(LoadCursor(nullptr, IDC_WAIT));
		if ((pDC = GetDC()) != nullptr) {
			if ((m_pCLRollingDie = new CSprite()) != nullptr) {
				m_pCLRollingDie->SetMobile(true);
				m_pCLRollingDie->SetMasked(false);
				if (m_pCLRollingDie->LoadCels(pDC, GetStringFromResource(IDS_ROLLING_LDIE_ANIMATION), NUM_LDIE_CELS))
					m_pCLRollingDie->LinkSprite();
				else {
					errCode = ERR_MEMORY;
					MFC::SetCursor(LoadCursor(nullptr, IDC_ARROW));              //reset cursor
					HandleError(errCode);
					return ;
				}
			} else {
				errCode = ERR_MEMORY;
				MFC::SetCursor(LoadCursor(nullptr, IDC_ARROW));              //reset cursor
				HandleError(errCode);
				return ;
			}  // end if m_pCLRollingDie

			if ((m_pCRRollingDie = new CSprite)) {
				m_pCRRollingDie->SetMobile(true);
				m_pCRRollingDie->SetMasked(false);
				if (m_pCRRollingDie->LoadCels(pDC, GetStringFromResource(IDS_ROLLING_RDIE_ANIMATION), NUM_RDIE_CELS))
					m_pCRRollingDie->LinkSprite();
				else {
					errCode = ERR_MEMORY;
					MFC::SetCursor(LoadCursor(nullptr, IDC_ARROW));              //reset cursor
					HandleError(errCode);
					return ;
				}
			} else {
				errCode = ERR_MEMORY;
				MFC::SetCursor(LoadCursor(nullptr, IDC_ARROW));              //reset cursor
				HandleError(errCode);
				return ;
			}   // end if m_pCRRollingDie


			if ((pCMonolithDiceBmp = FetchBitmap(pDC, nullptr, GetStringFromResource(IDS_DICE_MONOLITHE))) != nullptr) {
				pCLDieBmp[0] = ExtractBitmap(pDC, pCMonolithDiceBmp, m_pGamePalette, xDice[LEFT][0], yDice[LEFT][0], dxDice[LEFT][0], dyDice[LEFT][0]);     //flr under Ldie
				pCRDieBmp[0] = ExtractBitmap(pDC, pCMonolithDiceBmp, m_pGamePalette, xDice[RIGHT][0], yDice[RIGHT][0], dxDice[RIGHT][0], dyDice[RIGHT][0]); //flr under Rdie
				for (int i = 1; i < 7; i++) {
					pCLDieBmp[i] = ExtractBitmap(pDC, pCMonolithDiceBmp, m_pGamePalette, xDice[LEFT][i], yDice[LEFT][i], dxDice[LEFT][i], dyDice[LEFT][i]);
					pCRDieBmp[i] = ExtractBitmap(pDC, pCMonolithDiceBmp, m_pGamePalette, xDice[RIGHT][i], yDice[RIGHT][i], dxDice[RIGHT][i], dyDice[RIGHT][i]);
				}//end for i
				if (pCMonolithDiceBmp)  delete pCMonolithDiceBmp;
				pCMonolithDiceBmp = nullptr;

			} else {
				errCode = ERR_MEMORY;
				MFC::SetCursor(LoadCursor(nullptr, IDC_ARROW));               //reset cursor
				HandleError(errCode);
				return ;
			}             // end if pCMonolithDiceBmp




			ReleaseDC(pDC);
			pDC = nullptr;

		} else {  // pDC
			errCode = ERR_MEMORY;
			MFC::SetCursor(hOldCursor);
			HandleError(errCode);
		}//end if pDC
		MFC::SetCursor(hOldCursor);
		m_bDiceBmpsLoaded = true;
	}    // m_bDiceBmpsLoaded

	Common::strcpy_s(szMapFile, GetStringFromResource(IDS_MINI_GAME_MAP));
	if (FileExists(szMapFile)) {
		myDoc.OpenDocument(szMapFile);
		hDIB = myDoc.GetHDIB();

		pDC = GetDC();
		if (hDIB) {
			GetClientRect(rcDest);

			rcDIB.top = rcDIB.left = 0;
			rcDIB.right = (int) DIBWidth(hDIB);
			rcDIB.bottom = (int) DIBHeight(hDIB);

			PaintDIB(pDC->m_hDC, &rcDest, hDIB, &rcDIB, m_pGamePalette);
		}//end if (hDIB)

		AnimateDice();
#pragma warning(disable: 4135)
		m_LDie = (byte)(((uint32)(unsigned int)brand() * 6L) / ((uint32)(unsigned int)RAND_MAX + 1L)) + 1;
		m_RDie = (byte)(((uint32)(unsigned int)brand() * 6L) / ((uint32)(unsigned int)RAND_MAX + 1L)) + 1;
#pragma warning(default: 4135)
		PaintMaskedBitmap(pDC, m_pGamePalette, pCRDieBmp[m_RDie], \
		                  m_rRDie.left, m_rRDie.top, (int) m_rRDie.Width(), (int) m_rRDie.Height());

		PaintMaskedBitmap(pDC, m_pGamePalette, pCLDieBmp[m_LDie], \
		                  m_rLDie.left, m_rLDie.top, (int) m_rLDie.Width(), (int) m_rLDie.Height());

		m_bDiceJustThrown = true;
		m_bGameActive = true;

		ReleaseDC(pDC);

	} //end if(FileExists())
	HandleError(errCode);
}



void CMainWindow::GameReset() {
	m_bGameActive = false;                      // there is no currently active game

	for (short i = 0; i < 10; i++) {
		m_bDoorBmpLoaded[i] = false;
		m_iDoorStatus[i] = OPEN; //every door is open.
	}// end i

	m_bGameJustBegun = true;
	m_cActiveDoor = OPEN;                // the game has just begun and no door is active. The 0-th door is always open.
	m_cDoorCount = 0;                       //  no door is closed.
	m_cUnDoableThrows = 0;             //   no undoable throw has been registered yet.

	m_bDiceJustThrown = false;                   //dice haven't been thrown yet
	m_bPause = false;                           // the game is not paused

	m_bOneDieCase = false;
	m_iMoveValid = 0;
	//srand(LOWORD(timeGetTime()));

}


/*
*******************************************************************************************************************
*
*   OnMouseMove
*
*   FUNCTIONAL DESCRIPTION
*   Handles the Window Message WM_MOUSEMOVE. In _Debug mode it is used to
*   reset recursion count of the midi looping.
*
*   CALLING SEQUENCE:
*   Called By MFC/Windows;
*
*   FORMAL PARAMETERS:
*   Refer to MFC doc.
*
*   IMPLICIT INPUT PARAMETERS:
*   Refer to MFC doc.
*
*   IMPLICIT OUTPUT PARAMETERS:
*   Refer to MFC doc.
*
*   RETURN VALUE:
*   void
*
***********************************************************************************************************************************
*/
void CMainWindow::OnMouseMove(unsigned int, CPoint) {
	SetCursor(LoadCursor(nullptr, IDC_ARROW));
}


/*
*******************************************************************************************************************
*
*   OnLButtonDown
*
*   FUNCTIONAL DESCRIPTION
*   Handles the Window Message WM_LBUTTONDOWN.
*   For the game it processes all easter egg animations,
*   determines if doors/dice have been clicked and initiates appropriates
*   logical steps to that effect using procedures such as CMainWindow::LegalizeMove() and others.
*   It also checks if the game is over and computes score.
*
*   CALLING SEQUENCE:
*   Called By MFC/Windows;
*
*   FORMAL PARAMETERS:
*   Refer to MFC doc.
*
*   IMPLICIT INPUT PARAMETERS:
*   Refer to MFC doc.
*   m_r?Die                                 CRect       rectangle spanned by each die.
*    m_bGameActive                  bool        is game on?
*   m_bOneDieCase                   bool        is just one die to be rolled (from now on)?
*   m_pC?RollingDie                 CSprite     cell animation for each die.
*   gn?DieLeft, gn?DieTop       int             Final positions for dice.
*   m_?Die                                   int            The face value of the throw for each die.
*   pGameParams                     LPGAMESTRUCT    defined by Meta game.
*
*   IMPLICIT OUTPUT PARAMETERS:
*   Refer to MFC doc.
*
*   RETURN VALUE:
*   void
*
***********************************************************************************************************************************
*/

void CMainWindow::OnLButtonDown(unsigned int nFlags, CPoint point) {
	short i,
	      iDoor;
	short jj;
	bool AllFxd,                       //flag to indicate all fixed (locked) doors.
	     AllClosed;                //flag to indicate all closed doors.
	uint32 V;
	CDC* pDC;
	short int cel;
	CSprite* pBottleSprite,
	         *pCatSprite;
	HLOCAL hlocScore;
	char *npszScore;            //to display score in case of SA mode
	const int iMaxScore = 45; //1+2+3+4+5+6+7+8+9       to be used for score computation.
	ERROR_CODE errCode = ERR_NONE;

	//reset recursion count of midi music loop

	// User clicked on the Title - NewGame button
	//
	if (m_rNewGameButton.PtInRect(point) && !pGameParams->bPlayingMetagame) {

		// start a new game only if not in meta game mode.
		PlayGame();

	} else {
		if (pGameParams->bSoundEffectsEnabled) {            //play sound effects (Easter Eggs) only if this flag is enabled
			pDC = GetDC();

			if (Paper.PtInRect(point)) {                                    //NewsPaper EasterEgg,
				sndPlaySound(GetStringFromResource(IDS_EXTRA), SND_SYNC);
			}

			if (Glass.PtInRect(point)) {                                    //ShotGlass EasterEgg,
				sndPlaySound(GetStringFromResource(IDS_HICCUPS), SND_SYNC);
			}


			if (Bottle.PtInRect(point)) {                                         //AINMATE bottle when door is closed and just play "GHOST.WAV" otherwise.
				if (m_iDoorStatus[3] != OPEN) {
					sndPlaySound(GetStringFromResource(IDS_GHOST), SND_ASYNC);
					if ((pBottleSprite = new CSprite()) != nullptr) {
						if (!pBottleSprite->LoadCels(pDC, GetStringFromResource(IDS_BOTTLE_STRIP_door_closed), BOTTLE_CELS)) {
							errCode = ERR_MEMORY;
							MFC::SetCursor(LoadCursor(nullptr, IDC_ARROW));              //reset cursor
							HandleError(errCode);
							return ;
						} else {
							pBottleSprite->LinkSprite();
							for (cel = 0; cel < BOTTLE_CELS; cel ++) {
								pBottleSprite->SetCel(cel - 1);
								pBottleSprite->PaintSprite(pDC, BOTTLE_SPRITE_L, BOTTLE_SPRITE_T);
								Sleep(3 * 1810 / BOTTLE_CELS);
							}
							pBottleSprite->EraseSprite(pDC);
						}
						delete pBottleSprite;
						pBottleSprite = nullptr;
					}//end if pBottleSprite
				} else {
					sndPlaySound(GetStringFromResource(IDS_HICCUPS), SND_SYNC);
				}//end if m_iDoorStatus
			}//end if Bottle.PtInRect

			if (aBrShoes.PtInRect(point) || bBrShoes.PtInRect(point) ||  BluShoes.PtInRect(point)) {
				sndPlaySound(GetStringFromResource(IDS_FOOTSTEP), SND_SYNC);                //BROWN SHOES EasterEgg.
			}

			if (Cat.PtInRect(point)) {            // Hat4 is the CAT.
				if ((pCatSprite = new CSprite()) != nullptr) {
					sndPlaySound(GetStringFromResource(IDS_MEOW), SND_ASYNC);
					if (m_iDoorStatus[4] != OPEN) {
						pCatSprite->LoadCels(pDC, GetStringFromResource(IDS_HAT4_STRIP_door_closed), HAT4_CELS);
						pCatSprite->LinkSprite();
						for (cel = 0; cel < HAT4_CELS; cel ++) {
							pCatSprite->SetCel(cel - 1);
							pCatSprite->PaintSprite(pDC, HAT4_SPRITE_L, HAT4_SPRITE_T);
							Sleep(2020 / HAT4_CELS);
						}
					} else {
						pCatSprite->LoadCels(pDC, GetStringFromResource(IDS_HAT4_STRIP_door_open), HAT4_CELS);
						for (cel = 0; cel < HAT4_CELS; cel ++) {
							pCatSprite->SetCel(cel - 1);
							pCatSprite->PaintSprite(pDC, HAT4_SPRITE_L, HAT4_SPRITE_T);
							Sleep(2020 / HAT4_CELS);
						}
					}
					pCatSprite->EraseSprite(pDC);
					delete pCatSprite;
					pCatSprite = nullptr;
				}
			}
			if (Hat6.PtInRect(point)) {             //Hat EasterEgg.
				sndPlaySound(GetStringFromResource(IDS_HAT), SND_SYNC);
			}
			if (UmbrStand.PtInRect(point)) { //Umbrella Stand EasterEgg.
				sndPlaySound(GetStringFromResource(IDS_SINGRAIN), SND_SYNC);
			}


			ReleaseDC(pDC);
		}

		/*
		Detect if dice were clicked, if Game is active.
		*/
		if (((m_rLDie.PtInRect(point)  ||  m_rRDie.PtInRect(point)) && m_bGameActive && !m_bOneDieCase)
		        || (m_rLDie.PtInRect(point) && m_bOneDieCase && m_bGameActive)) {

			if (m_iMoveValid > 0) {                                         //dice can be clicked only if doors are correctly opened.When
				// the game has just begun, everything is taken care of in the WM_PAINT message..
				m_bDiceJustThrown = true;                       //set flag if click is to be accepted.

				for (i = 1; i < 10; i++) {
					if (m_iDoorStatus[i] == CLOSED) {
						m_iDoorStatus[i] = FIXED;  // lock all closed doors, before permitting player to click on door(s),
						if (pCDoorBmp[i]) {
							delete pCDoorBmp[i];                                // and free up those sprites.
							pCDoorBmp[i] = nullptr;
							m_bDoorBmpLoaded[i] = false;
						}
					}      // end if m_iDoorStatus[i]==CLOSED
				}// end for i


				/*
				determine if this throw is to be done with a single die. (ie. if throws 7 and above are all undoable)
				*/
				for (i = 7, AllFxd = true; i < 13; i++) {            // i<13 because i==12 is the max doable throw with two dice.
					/***** note that this was a bug in versions 1.0 thru 1.5 ******/
					if (AllFxd) AllFxd = !IsThrowDoable((byte)i);
					else  break;
				}
				m_bOneDieCase = AllFxd;

				/* randomise throws */
#pragma warning(disable: 4135)
				V = (uint32)((unsigned int)RAND_MAX + 1);
				m_LDie = (byte)(((uint32)(unsigned int)brand() * 6L) / V) +1;                                    //  left Die
				m_RDie = m_bOneDieCase ? 0 : (byte)(((uint32)(unsigned int)brand() * 6) / V) +1;       //    right Die
#pragma warning(default: 4135)

				pDC = GetDC();


				/* animate dice with audio FX*/
				AnimateDice();

				/*
				Paint dice/ (die and flr).
				*/
				PaintMaskedBitmap(pDC, m_pGamePalette, pCLDieBmp[m_LDie], \
				                  m_rLDie.left, m_rLDie.top, (int) m_rLDie.Width(), (int) m_rLDie.Height());

				PaintMaskedBitmap(pDC, m_pGamePalette, pCRDieBmp[m_RDie], \
				                  m_rRDie.left, m_rRDie.top, (int) m_rRDie.Width(), (int) m_rRDie.Height());
				ReleaseDC(pDC);

				/*
				    see if current throw is doable. If not increment count of undoable throws.
				    Bring up MessageBox to that effect and in case it's 3 throws terminate current
				    game. If the throw is doable reset the count of undoable throws to 0.
				 */
				if (!IsThrowDoable((byte)(m_LDie + m_RDie))) {
					/*
					if the #of successive undoable throws is 3 then you 've lost game.
					Else: display message box and continue
					*/

					switch (++m_cUnDoableThrows) {
					default:
					case 3:
						if (pGameParams->bSoundEffectsEnabled) sndPlaySound(GetStringFromResource(IDS_SOSORRY), SND_ASYNC);
						CMessageBox(this, m_pGamePalette, "Game over.", "You have lost!");

						pDC = GetDC();
						if (!m_bOneDieCase) {
							m_pCLRollingDie->SetCel(-1);                                        //repaint floor with no dice.
							m_pCRRollingDie->SetCel(-1);
							m_pCLRollingDie->PaintSprite(pDC, gnLDieLeftFinal, gnLDieTopFinal);
							m_pCRRollingDie->PaintSprite(pDC, gnRDieLeftFinal, gnRDieTopFinal);
						} else {

							m_pCLRollingDie->SetCel(-1);          /*single*/
							m_pCLRollingDie->PaintSprite(pDC, gnLDieLeftFinal, gnLDieTopFinal);/*single*/
						}
						ReleaseDC(pDC);
						/*
						Compute Score as the number of open doors
						*/
						for (iDoor = 1, pGameParams->lScore = 0x00L; iDoor < 10; iDoor++) {
							if (m_iDoorStatus[iDoor] == OPEN) pGameParams->lScore += iDoor;
						}
						/*
						    Display Score if not in Meta mode
						*/
						//if(!pGameParams->bPlayingMetagame){
						if ((hlocScore = LocalAlloc(LHND, 32)) != nullptr) {
							npszScore = (char *)LocalLock(hlocScore);
							Common::sprintf_s(npszScore, 32, "%lu point%c out of 45.", iMaxScore - pGameParams->lScore, ((iMaxScore - pGameParams->lScore) == 1) ? ' ' : 's'); //imaxScore is 45.
							CMessageBox(this, m_pGamePalette, "Your score is", npszScore);
							LocalUnlock(hlocScore);
							LocalFree(hlocScore);
						}
						//}
						m_bGameActive = false;                      //set game over flag.
						if (pGameParams->bPlayingMetagame) PostMessage(WM_CLOSE, 0, 0L);        //quit if in Meta game mode.

						break;

					case 1:
					case 2:
						char *npszInfo;
						HLOCAL hInfo;

						if ((hInfo = LocalAlloc(LHND, 32)) == nullptr)
							error("TODO: Memory alloc");

						npszInfo = (char *)LocalLock(hInfo);
						Common::sprintf_s(npszInfo, 32, "Only %d throw%c left.", (3 - m_cUnDoableThrows), (m_cUnDoableThrows == 1) ? 's' : ' ');

						GamePause();
						sndPlaySound(GetStringFromResource(IDS_SORRY), SND_ASYNC);
						CMessageBox(this, m_pGamePalette, "Undoable throw!", npszInfo ? (char *)npszInfo : "");

						LocalUnlock(hInfo);
						LocalFree(hInfo);

						pDC = GetDC();
						if (!m_bOneDieCase) {
							m_pCLRollingDie->SetCel(-1);                                        //repaint floor with no dice.
							m_pCRRollingDie->SetCel(-1);
							m_pCLRollingDie->PaintSprite(pDC, gnLDieLeftFinal, gnLDieTopFinal);
							m_pCRRollingDie->PaintSprite(pDC, gnRDieLeftFinal, gnRDieTopFinal);
						} else {
							m_pCLRollingDie->SetCel(-1);/*Single*/
							m_pCLRollingDie->PaintSprite(pDC, gnLDieLeftFinal, gnLDieTopFinal);/*Single*/
						}
						ReleaseDC(pDC);
						GameResume();
						Sleep(200);                                //wait for a while
						break;

					}   //end switch(m_cUnDoableThrows)

					m_iMoveValid = 1;       //this is an invalid throw; so reset indicators and roll dice, then exit the func.
					//  m_iMoveValid is set to 1, because the move is valid even though the throw is invalid.

					if (!m_bOneDieCase) PostMessage(WM_LBUTTONDOWN, 0, MAKELPARAM(LDIE_MIDPOINT_X, LDIE_MIDPOINT_Y));
					else    PostMessage(WM_LBUTTONDOWN, 0, MAKELPARAM(SINGLE_DIE_MIDPOINT_X, SINGLE_DIE_MIDPOINT_Y));
					PostMessage(WM_LBUTTONUP, 0, 0L);
					return;

				} else {
					m_cUnDoableThrows = 0;
				} //end if(!IsThrowdoable...)


				m_iMoveValid = 0;                                // indicators.

			} else {
				sndPlaySound(GetStringFromResource(IDS_NOPE), SND_ASYNC);
			}//end !Ism_iMoveValid

		} //end if(m_rLDie.PtInRect(point)...

		if (m_bDiceJustThrown || (m_iMoveValid >= 0)) {                  // can doors be clicked upon?
			pDC = GetDC();
			for (i = 1; i < 10; i++) {
				if (m_rDoor[i].PtInRect(point)  && m_bGameActive) {
					/*first:
					 if the corresponding cel strip is not loaded, then load it
					 */
					if (!m_bDoorBmpLoaded[i]) {
						if (!pCDoorBmp[i])
							pCDoorBmp[i] = new CSprite;     // the sprite might have been deleted in prev game if that door was locked @end of game.
						if (pCDoorBmp[i]) pCDoorBmp[i]->LoadCels(pDC, GetStringFromResource(IDS_D1 + i - 1), NUM_DOOR_CELS);
						m_bDoorBmpLoaded[i] = true;
					}

					/*
					if door is not FIXED( LOCKED) check for validity of move and/or throw.
					*/
					if (m_iDoorStatus[i] != FIXED) {                        //you can click only on an unlocked door.
						m_iMoveValid = LegalizeMove(i);
						if (m_iMoveValid != -1) {           //valid move.
							m_iDoorStatus[i] = !(m_iDoorStatus[i]);                                  //m_idoorStatus reflects the new door status.

							if (m_iDoorStatus[i] == OPEN) {          //open a closed door.
								if (pGameParams->bSoundEffectsEnabled) sndPlaySound(GetStringFromResource(IDS_CREAKING_DOOR_OPENING), SND_ASYNC);

								/*animate doorsprite.*/
								for (jj = 0; jj < NUM_DOOR_CELS; jj++) {
									pCDoorBmp[i]->SetCel(jj - 1);
									pCDoorBmp[i]->PaintSprite(pDC, m_rDoor[i].left, m_rDoor[i].top);
									Sleep(SLEEP_OPENING_TIME);                   //sync the audio and video of animation.
								}  //end for jj
							} else {                                                  //shut the open door.
								if (pGameParams->bSoundEffectsEnabled) sndPlaySound(GetStringFromResource(IDS_CREAKING_DOOR_CLOSING), SND_ASYNC);
								for (jj = NUM_DOOR_CELS; jj > 0;  jj--) {
									pCDoorBmp[i]->SetCel(jj - 2);                   // because PaintSprite automatically advances to the next cel.
									pCDoorBmp[i]->PaintSprite(pDC, m_rDoor[i].left, m_rDoor[i].top);
									if (jj < 5) Sleep(SLEEP_CLOSING_TIME);  //sync the audio and video of animation.
								}//end for jj
							} //end if m_iDoorStatus==Open.


							m_cActiveDoor = (byte)i;         //set active door.     //this flag is now obsolete, but retained only for compatibility.
						} else {   //if m_iMoveValid ==-1 i.e. invalid move
							sndPlaySound(GetStringFromResource(IDS_NOPE), SND_ASYNC);
							if ((m_iMoveValid = LegalizeMove(0)) == 1);
							/* Consider the following case:
							All Doors are open;
							You Roll 9, Click on Door 9 to close it, m_iMoveValid is 1.
							click on door 1,(it beeps; door sum is still 9), but m_iMoveValid is -1.
							Thus the move is erroneously interpreted as invalid.
							To take care of such cases, re-check the status of the move,
							by passing in the 0-th door (this door is always open). It does not
							afffect the door sum and hence is a beautiful verification mechanism.
							*/
							else m_iMoveValid = 0;           //the move is genuinely invalid.
						}      //end if (m_iMoveValid...)
					}       //end if(m_iDoorStatus...)
					break;      //breaks the for loop cos there's only one mouse strike for a given loop.
				}       //end if(m_rDoor[i]...)
			}       // end -for(i)
			ReleaseDC(pDC);
		} else {
			//indicate that you are clicking on doors one too many, or you have an incorrect sum !
			sndPlaySound(GetStringFromResource(IDS_NOPE), SND_ASYNC);
		}       //end if (m_bDiceJustThrown ...)

		if (m_iMoveValid > 0) {                            //if valid move, check for the YOU WIN case.
			for (AllClosed = true, i = 1; i < 10; i++) {
				if (AllClosed) AllClosed = (m_iDoorStatus[i] != OPEN);
				else break;
			}

			if (AllClosed) {                                                                                   //you have won!
				if (pGameParams->bSoundEffectsEnabled) sndPlaySound(GetStringFromResource(IDS_APPLAUSE), SND_ASYNC);
				CMessageBox(this, m_pGamePalette, "Game over.", "You have won!");
				pGameParams->lScore = 0x00L;        //make game result available to meta game.; no door open

				m_iMoveValid = 0;
				m_bDiceJustThrown = false;
				m_bGameActive = false;
				/* you have to quit the mini game if in MetaGame Mode */
				if (pGameParams->bPlayingMetagame) PostMessage(WM_CLOSE, 0, 0L);
			}//end if AllClosed
		}//end if (m_iMoveValid>0)

	}//end else if not in  new button rect.
	(void)nFlags;
}




/*
*******************************************************************************************************************
*
*   OnRButtonDown
*
*   FUNCTIONAL DESCRIPTION
*   Handles the Window Message WM_RBUTTONDOWN.
*   For the game it undoes the last sequence of closing/opening of doors,
*   and restores'em to a state just prior to the last roll of dice.
*
*   CALLING SEQUENCE:
*   Called By MFC/Windows;
*
*   FORMAL PARAMETERS:
*   Refer to MFC doc.
*
*   IMPLICIT INPUT PARAMETERS:
*   Refer to MFC doc.
*   m_rDoor[ ]                              CRect       rectangle spanned by each door.
*    m_bGameActive                  bool        is game on?
*   m_iDoorStatus[ ]                    int             The Status of each door.
*
*   IMPLICIT OUTPUT PARAMETERS:
*   Refer to MFC doc.
*
*   RETURN VALUE:
*   void
*
***********************************************************************************************************************************
*/
void CMainWindow::OnRButtonDown(unsigned int nFlags, CPoint point) {
	short i;
	int         xx,
	            yy;
	CDC* pDC;

	/*
	    Restores (Undoes) m_iDoorStatus to that just prior to rolling of dice
	*/
	if (m_bGameActive) {

		pDC = GetDC();
		for (i = 1; i < 10; i++) {
			/*open all unlocked doors*/
			if (m_iDoorStatus[i] == CLOSED) {
				/*
				    simulate clicking (with L Mouse Button) on each closed door individually
				*/
				xx = m_rDoor[i].left + m_rDoor[i].Width() / 2;
				yy = m_rDoor[i].top + m_rDoor[i].Height() / 2;

				PostMessage(WM_LBUTTONDOWN, 0, MAKELPARAM(xx, yy));
				PostMessage(WM_LBUTTONUP, 0, MAKELPARAM(xx, yy));
			} //end if m_iDoorStatus[i]...

		}       // end -for(i)
		ReleaseDC(pDC);
		(void)nFlags;
	} else {
		CFrameWnd::OnRButtonDown(nFlags, point);
	}
}


/*
*******************************************************************************************************************
*
*   OnMCINotify
*
*   FUNCTIONAL DESCRIPTION
*   Handles the Window Message MM_MCINOTIFY.
*   For the game it calls the CSound member func OnMCIStopped.
*
*   CALLING SEQUENCE:
*   Called By MFC/Windows MMSYSTEM.DLL when sound (in this case
*       m_psndBkndMusic) is over.
*
*   FORMAL PARAMETERS:
*   Refer to MFC doc.
*
*   IMPLICIT INPUT PARAMETERS:
*   Refer to MFC doc.
*   m_psndBkgndMusic            CSound      Midi Music.
*
*   IMPLICIT OUTPUT PARAMETERS:
*
*   RETURN VALUE:
*   Refer to MMSystem doc.
*
***********************************************************************************************************************************
*/
LRESULT CMainWindow::OnMCINotify(WPARAM wParam, LPARAM lParam) {
	CSound::OnMCIStopped(wParam, lParam);
	return 0;
}

LRESULT CMainWindow::OnMMIONotify(WPARAM wParam, LPARAM lParam) {
	CSound::OnMMIOStopped(wParam, lParam);
	return 0;
}


/*
*******************************************************************************************************************
*
*   DeleteSprite
*
*   FUNCTIONAL DESCRIPTION
*   Deletes a Sprite.
*
*   CALLING SEQUENCE:
*   Called by OnClose() when deleting a CSprite  object.
*
*   FORMAL PARAMETERS:
*   pSprite     CSprite     The Sprite to be deleted.
*
*   IMPLICIT INPUT PARAMETERS:
*       n/a
*
*   IMPLICIT OUTPUT PARAMETERS:
*       n/a
*
*   RETURN VALUE:
*       void
*
***********************************************************************************************************************************
*/
void CMainWindow::DeleteSprite(CSprite *pSprite) {
	CDC *pDC;

	// can't delete a null pointer
	assert(pSprite != nullptr);

	if ((pDC = GetDC()) != nullptr) {
		pSprite->EraseSprite(pDC);                  // erase it from screen
		ReleaseDC(pDC);
	}
	pSprite->UnlinkSprite();                        // unlink it

	delete pSprite;                                 // delete it
}




void CMainWindow::OnSysChar(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	// Terminate app on ALT_Q
	if ((nChar == 'q') && (nFlags & 0x2000)) {

		PostMessage(WM_CLOSE, 0, 0);

	} else {

		// default action
		CFrameWnd ::OnSysChar(nChar, nRepCnt, nFlags);
	}
}




void CMainWindow::OnSysKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	switch (nChar) {

	// User has hit ALT_F4 so close down this App
	//
	case VK_F4:
		PostMessage(WM_CLOSE, 0, 0);
		break;

	default:
		CFrameWnd::OnSysKeyDown(nChar, nRepCnt, nFlags);
		break;
	}
}



void CMainWindow::OnKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	// Handle keyboard input
	//

	switch (nChar) {

	//
	// Bring up the Rules
	//
	case VK_F1: {
		GamePause();
		CSound::waitWaveSounds();
		CRules  RulesDlg(this, ".\\novac.txt", m_pGamePalette, \
		                 (pGameParams->bSoundEffectsEnabled) ? GetStringFromResource(IDS_RULES_WAV) : nullptr);
		RulesDlg.DoModal();
		GameResume();
	}
	break;

	//
	// Bring up the options menu
	//
	case VK_F2:
		SendMessage(WM_COMMAND, IDC_MENU, BN_CLICKED);
		break;

	case 'D':                               //use 'd' to roll the dice; Send (do NOT Post) Messages to preserve Message Queue order.
		SendMessage(WM_LBUTTONDOWN, 0, MAKELPARAM(LDIE_MIDPOINT_X, LDIE_MIDPOINT_Y));
		SendMessage(WM_LBUTTONUP, 0, 0L);
		break;

	default:                                // use the NUM_KEYpad or the number keys to close/open doors.
		if ((nChar >= VK_NUMPAD1) && (nChar <= VK_NUMPAD9)) nChar -= VK_NUMPAD1 - '1'; //map numpad keys to regular keys.
		if ((nChar >= '1') && (nChar <= '9')) {
			SendMessage(WM_LBUTTONDOWN, 0, MAKELPARAM((nChar - '1')*DOOR_SPACING + DOOR1_MIDPOINT_X, DOOR1_MIDPOINT_Y));
			SendMessage(WM_LBUTTONUP, 0, 0L);
		} else {
			CFrameWnd::OnKeyDown(nChar, nRepCnt, nFlags);
		}
		break;
	}
}


void CMainWindow::OnActivate(unsigned int nState, CWnd *pWndOther, bool bMinimized) {
	if (!bMinimized) {
		switch (nState) {
		case WA_ACTIVE:
		case WA_CLICKACTIVE:
			InvalidateRect(nullptr, false);
			break;

		default:
			break;
		}
	}
	(void)pWndOther;
}


short CMainWindow::LegalizeMove(short  j) {
	/*
	*******************************************************************************************************************
	*
	*   LegalizeMove
	*
	*   FUNCTIONAL DESCRIPTION
	*   checks if a given move is valid. A move is defined as a sequence of rolling the dice and
	*   opening or closing of doors.  To do this, it computes the roll of the dice, sums up doors as
	*   they are opened or closed adding or subracting appropriately. The variable  DoorSum is
	*   statically maintained to this end, and set to zero as soon as the dice are rolled (as indicated
	*   by m_bDiceJustThrown).
	*
	*   CALLING SEQUENCE:
	*   short LegalizeMove(short j);
	*
	*   FORMAL PARAMETERS:
	*   j   short   indicates the last door cliked on by the user.
	*
	*   IMPLICIT INPUT PARAMETERS:
	*   m_LDie, m_RDie(if applicable)   the rolls of the left and right dice.
	*   m_iDoorStatus[10]                       the open/closd/fixed status of every door just befor this func was
	*                                                                   called; i.e, before the door "j" was clicked on.
	*   m_bDiceJustThrown                   this flag indicates if "j" is the first door clicked upon after the dice
	*                                                                   were thrown.
	*   m_cDoorCount                            # of doors open at any pt of time.
	*
	*   IMPLICIT OUTPUT PARAMETERS:
	*   m_cDoorCount.                                   Updates the count of doors open.
	*
	*   RETURN VALUE:
	*    -1   if doorSum > DiceSum which implies that the player can't go anhead unless he/she opens or
	*                                               shuts doors, enough to meet the DOORSUM criterion.
	*                                               Dice can't be clicked if  no possible combinations are
	*                                               available or unless the right combination is struck.
	*    1      if doorSum==DiceSum which implies that the move is valid and dice can be clicked again, if desired.
	*
	*   0       if doorSum< DiceSum which implies that the move is in construction and could possibly be valid.
	*                                                   the player can click on any door, but not on the dice.
	*
	*   LegalizeMove keeps track of the number of open doors.
	***********************************************************************************************************************************
	*/
	static byte DoorSum,
	       DiceSum;
	short int ReadyForDiceClick = 0;

	if (m_bDiceJustThrown) {
		DoorSum = 0;                                  // reset door sum if "j" is the first open door to be clicked upon
		// ... after the dice were rolled in.
		m_bDiceJustThrown = false;
	}

	DiceSum = (byte)(m_LDie + m_RDie);
	if (m_iDoorStatus[j] == OPEN) {
		//increment  door count  if  the door is initially open (i.e. the mouse is clicked on an open door) and decrement otherwise

		if ((DoorSum += (byte)j) > DiceSum) {
			DoorSum -= (byte)j;                                     // disallow clicking on such a door so as to exceed door sum.
			return -1;
		} else {
			if (DoorSum == DiceSum)    ReadyForDiceClick = 1;
			if (!j) ++m_cDoorCount;                         //0-th door should not affect door count
			return (ReadyForDiceClick);        // return (1) if all set, 0 if door sum < dice sum.
		}
	} else {                   // if door is initially closed decrement doorsum.
		DoorSum -= (byte)j;
		if (DoorSum > DiceSum) {
			DoorSum += (byte)j;
			return -1;
		}
		if (DoorSum == DiceSum)    ReadyForDiceClick = 1;
		if (!j) --m_cDoorCount;                              //0-th door should not affect door count
		return (ReadyForDiceClick);
	}
}


bool CMainWindow::IsThrowDoable(byte DiceSum) {
	/*****************************************************************************************************************
	*   [IsThrowDoable]
	*
	*   FUNCTIONAL DESCRIPRION:
	*   the algo is to look at list of all the open doors just before the dice were rolled in and
	*   generate a set of all possible sums with an # of doors taken at a time; see if the dice
	*   combination just rolled in is a member of this set; if yes, the throw is doable, else not.
	*
	*   CALLING SEQUENCE:
	*   bool _pascal IsThrowDoable(byte DiceSum)
	*
	*   FORMAL PARAMETERS:
	*   DiceSum is the current throw of the dice.
	*
	*   IMPLICIT INPUT PARAMETERS:
	*   m_iDoorStatus[10]; indicates if a door is CLOSED, OPEN or FIXED. these constants are
	*   defined in main.h. Once a door is FIXED, it's locked and can never be opened again.
	*
	*   IMPLICIT OUTPUT PARAMETERS:
	*   N/A
	*
	*   RETURN VALUE:
	*   returns true if Throw is Doable.
	*               false if  Undoable.
	*
	*   ENVIRONMENT:
	*   n/a
	****************************************************************************************************************
	*/

#pragma warning(disable: 4135)
	byte s[9];                                                       //Open doors.
	byte Count,                                               //# of open doors.
	     i,
	     k,
	     sum,
	     i1, i2, i3, i4, i5, i6, i7, i8, i9,
	     temp, temp2, temp3, temp4, temp5, temp6, temp7, temp8, temp9;

	for (i = 1, Count = 0; i < 10; i++) {
		if (m_iDoorStatus[i] == OPEN) s[Count++] = i;       //feeds in a list of open doors.
	}
	for (k = 1; k < Count + 1; k++) {
		switch (k) {
		case 1:
			if (DiceSum < 10) if (m_iDoorStatus[DiceSum] == OPEN) return true;
			break;

		case 2:
			for (i1 = 0; i1 < Count; i1++) {
				temp = s[i1];
				for (i2 = i1 + 1; i2 < Count; i2++) {
					sum = temp;
					sum += s[i2];
					if (sum == DiceSum) return true;
				}
			}
			break;

		case 3:
			for (i3 = 0; i3 < Count; i3++) {
				temp3 = s[i3];
				for (i2 = i3 + 1; i2 < Count; i2++) {
					temp2 = temp3;
					temp2 += s[i2];
					for (i1 = i2 + 1; i1 < Count; i1++) {
						sum = temp2;
						sum += s[i1];
						if (sum == DiceSum) return true;
					}
				}
			}
			break;

		case 4:
			for (i4 = 0; i4 < Count; i4++) {
				temp4 = s[i4];
				for (i3 = i4 + 1; i3 < Count; i3++) {
					temp3 = temp4;
					temp3 += s[i3];
					for (i2 = i3 + 1; i2 < Count; i2++) {
						temp2 = temp3;
						temp2 += s[i2];
						for (i1 = i2 + 1; i1 < Count; i1++) {
							sum = temp2;
							sum += s[i1];
							if (sum == DiceSum) return true;
						}
					}
				}
			}
			break;


		case 5:


			for (i5 = 0; i5 < Count; i5++) {
				temp5 = s[i5];
				for (i4 = i5 + 1; i4 < Count; i4++) {
					temp4 = temp5;
					temp4 += s[i4];
					for (i3 = i4 + 1; i3 < Count; i3++) {
						temp3 = temp4;
						temp3 += s[i3];
						for (i2 = i3 + 1; i2 < Count; i2++) {
							temp2 = temp3;
							temp2 += s[i2];
							for (i1 = i2 + 1; i1 < Count; i1++) {
								sum = temp2;
								sum += s[i1];
								if (sum == DiceSum) return true;
							}
						}
					}
				}
			}
			break;


		case 6:

			for (i6 = 0; i6 < Count; i6++) {
				temp6 = s[i6];
				for (i5 = i6 + 1; i5 < Count; i5++) {
					temp5 = temp6;
					temp5 += s[i5];
					for (i4 = i5 + 1; i4 < Count; i4++) {
						temp4 = temp5;
						temp4 += s[i4];
						for (i3 = i4 + 1; i3 < Count; i3++) {
							temp3 = temp4;
							temp3 += s[i3];
							for (i2 = i3 + 1; i2 < Count; i2++) {
								temp2 = temp3;
								temp2 += s[i2];
								for (i1 = i2 + 1; i1 < Count; i1++) {
									sum = temp2;
									sum += s[i1];
									if (sum == DiceSum) return true;
								}
							}
						}
					}
				}
			}
			break;
		case 7:

			for (i7 = 0; i7 < Count; i7++) {
				temp7 = s[i7];
				for (i6 = i7 + 1; i6 < Count; i6++) {
					temp6 = temp7;
					temp6 += s[i6];
					for (i5 = i6 + 1; i5 < Count; i5++) {
						temp5 = temp6;
						temp5 += s[i5];
						for (i4 = i5 + 1; i4 < Count; i4++) {
							temp4 = temp5;
							temp4 += s[i4];
							for (i3 = i4 + 1; i3 < Count; i3++) {
								temp3 = temp4;
								temp3 += s[i3];
								for (i2 = i3 + 1; i2 < Count; i2++) {
									temp2 = temp3;
									temp2 += s[i2];
									for (i1 = i2 + 1; i1 < Count; i1++) {
										sum = temp2;
										sum += s[i1];
										if (sum == DiceSum) return true;
									}
								}
							}
						}
					}
				}
			}
			break;

		case 8:
			for (i8 = 0; i8 < Count; i8++) {
				temp8 = s[i8];
				for (i7 = i8 + 1; i7 < Count; i7++) {
					temp7 = temp8;
					temp7 += s[i7];
					for (i6 = i7 + 1; i6 < Count; i6++) {
						temp6 = temp7;
						temp6 += s[i6];
						for (i5 = i6 + 1; i5 < Count; i5++) {
							temp5 = temp6;
							temp5 += s[i5];
							for (i4 = i5 + 1; i4 < Count; i4++) {
								temp4 = temp5;
								temp4 += s[i4];
								for (i4 = 0; i4 < Count; i4++) {
									temp4 = s[i4];
									for (i3 = i4 + 1; i3 < Count; i3++) {
										temp3 = temp4;
										temp3 += s[i3];
										for (i2 = i3 + 1; i2 < Count; i2++) {
											temp2 = temp3;
											temp2 += s[i2];
											for (i1 = i2 + 1; i1 < Count; i1++) {
												sum = temp2;
												sum += s[i1];
												if (sum == DiceSum) return true;
											}
										}
									}
								}
							}
						}
					}
				}
			}
			break;

		case 9:
			for (i9 = 0; i9 < Count; i9++) {
				temp9 = s[i9];
				for (i8 = i9 + 1; i8 < Count; i8++) {
					temp8 = temp9;
					temp8 += s[i8];
					for (i7 = i8 + 1; i7 < Count; i7++) {
						temp7 = temp8;
						temp7 += s[i7];
						for (i6 = i7 + 1; i6 < Count; i6++) {
							temp6 = temp7;
							temp6 += s[i6];
							for (i5 = i6 + 1; i5 < Count; i5++) {
								temp5 = temp6;
								temp5 += s[i5];
								for (i4 = i5 + 1; i4 < Count; i4++) {
									temp4 = temp5;
									temp4 += s[i4];
									for (i3 = i4 + 1; i3 < Count; i3++) {
										temp3 = temp4;
										temp3 += s[i3];
										for (i2 = i3 + 1; i2 < Count; i2++) {
											temp2 = temp3;
											temp2 += s[i2];
											for (i1 = i2 + 1; i1 < Count; i1++) {
												sum = temp2;
												sum += s[i1];
												if (sum == DiceSum) return true;
											}
										}
									}
								}
							}
						}
					}
				}
			}
			break;
		default:
			break;
		} //end switch(count)
	}//end for -k
	return false;    //Undoable by default.
}
#pragma warning(default: 4135)


void CMainWindow::OnClose() {
	CBrush cbrBlack;
	CRect crectSplashScr;
	CDC *pDC;

	/*
	    if playing meta game then (re)compute score should the user choose to quit before end of play.
	    (if the user hits quit when the scroll is first brought up, then the score is 1+2+...+9 because all
	    doors are set to open in the constructor.
	*/
	pGameParams->lScore = 0x00L;
	for (int iDoor = 1; pGameParams->bPlayingMetagame && iDoor < 10; iDoor++) {
		if (m_iDoorStatus[iDoor] == OPEN) pGameParams->lScore += iDoor;
	}

	GameReset();

	CSound::clearSounds();   //ONLY A TEST; this works.

	for (int i = 0; i < 7; i++) {
		if (pCLDieBmp[i]) {
			delete pCLDieBmp[i];            //clear dice bmps.
			pCLDieBmp[i] = nullptr;
		}
		if (pCRDieBmp[i]) {
			delete pCRDieBmp[i];
			pCRDieBmp[i] = nullptr;
		}
	}

	//
	// de-allocate the main menu scroll button
	//
	assert(m_pScrollButton != nullptr);
	if (m_pScrollButton != nullptr) {
		delete m_pScrollButton;
		m_pScrollButton = nullptr;
	}


	//
	// need to de-allocate the game palette
	//
	assert(m_pGamePalette != nullptr);
	if (m_pGamePalette != nullptr) {
		m_pGamePalette->DeleteObject();
		delete m_pGamePalette;
		m_pGamePalette = nullptr;
	}

	for (int i = 1; i < 10; i++) {
		if (pCDoorBmp[i]) {
			DeleteSprite(pCDoorBmp[i]); /* clear door sprites */
			pCDoorBmp[i] = nullptr;
		}
	}

	/*clear dice sprites*/
	if (m_pCRRollingDie) {
		DeleteSprite(m_pCRRollingDie);
		m_pCRRollingDie = nullptr;
	}

	if (m_pCLRollingDie) {
		DeleteSprite(m_pCLRollingDie);
		m_pCLRollingDie = nullptr;
	}

	if ((pDC = GetDC()) != nullptr) {              // paint black  screen after all's over.
		crectSplashScr.SetRect(0, 0, GAME_WIDTH, GAME_HEIGHT);
		cbrBlack.CreateStockObject(BLACK_BRUSH);
		pDC->FillRect(&crectSplashScr, &cbrBlack);
		ReleaseDC(pDC);
	}

	CFrameWnd::OnClose();

	MFC::PostMessage(ghParentWnd, WM_PARENTNOTIFY, WM_DESTROY, 0L);
}


//
// CMainWindow message map:
// Associate messages with member functions.
//
BEGIN_MESSAGE_MAP(CMainWindow, CFrameWnd)
	ON_WM_PAINT()
	ON_WM_CLOSE()
	ON_WM_MOUSEMOVE()
	ON_WM_SYSCHAR()
	ON_WM_KEYDOWN()
	ON_WM_SYSKEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()

	ON_MESSAGE(MM_MCINOTIFY, CMainWindow::OnMCINotify)
	ON_MESSAGE(MM_WOM_DONE, CMainWindow::OnMMIONotify)
END_MESSAGE_MAP()

/*****************************************************************************************************************
*   [AnimateDice]
*
*   FUNCTIONAL DESCRIPRION:
*   Animates dice by using cell sprites.
*
*   CALLING SEQUENCE:
*   void AnimateDice()
*
*   FORMAL PARAMETERS:
*   none.
*
*   IMPLICIT INPUT PARAMETERS:
*   m_pC?RollingDie, the sprite animation for  ?die (?=L, R, single).
*
*   IMPLICIT OUTPUT PARAMETERS:
*   N/A
*
*   RETURN VALUE:
*   none
*
*   ENVIRONMENT:
*   n/a
****************************************************************************************************************
*/
void CMainWindow::AnimateDice() {
	CDC* pDC;

	// Coordinates of each cel from cell strip on splash screen.
	// make these static so as not to encroach upon the limited stack.
	static const int16 LDieLeft[NUM_LDIE_CELS] = {234, 442, 400, 384, 365, 346, 310, 264, 242, 235, 235, 236};
	static const int16 LDieTop[NUM_LDIE_CELS] = {344, 395, 340, 323, 323, 379, 372, 336, 365, 365, 347, 344};
	static const int16 RDieLeft[NUM_RDIE_CELS] = {336, 595, 571, 527, 480, 442, 398, 373, 356, 341, 337};
	static const int16 RDieTop[NUM_RDIE_CELS] = {345, 374, 353, 324, 334, 387, 342, 349, 376, 357, 346};

	int ii;

	// Coordinates of where to place dice on splash screen from cell strips.
	gnLDieLeftFinal = LDieLeft[0];
	gnLDieTopFinal = LDieTop[0];
	gnRDieLeftFinal = RDieLeft[0];
	gnRDieTopFinal = RDieTop[0];

	///////// Paint sprites ////////////
	pDC = GetDC();

	m_pCLRollingDie->EraseSprite(pDC);
	m_pCRRollingDie->EraseSprite(pDC);

	if (pGameParams->bSoundEffectsEnabled)
		sndPlaySound(GetStringFromResource(IDS_SHAKE), SND_ASYNC);

	if (!m_bOneDieCase) {
		m_pCLRollingDie->SetCel(0);
		m_pCRRollingDie->SetCel(0);

		m_pCLRollingDie->PaintSprite(pDC, LDieLeft[1], LDieTop[1]);

		for (ii = 2; ii < NUM_LDIE_CELS ; ii++) {
			m_pCLRollingDie->PaintSprite(pDC, *(LDieLeft + ii), *(LDieTop + ii));
			m_pCRRollingDie->PaintSprite(pDC, *(RDieLeft + ii - 1), *(RDieTop + ii - 1));
			pause();
		}

		if (pGameParams->bSoundEffectsEnabled) {
			sndPlaySound(nullptr, 0);                           // kill rattle
			sndPlaySound(GetStringFromResource(IDS_ROLL), SND_ASYNC);    // and roll!
		}
		m_pCRRollingDie->SetCel(NUM_RDIE_CELS - 2);         //paint LAST CEL
		m_pCRRollingDie->PaintSprite(pDC, *(RDieLeft + NUM_RDIE_CELS - 1), *(RDieTop + NUM_RDIE_CELS - 1));

	} else {
		// The single die case...
		m_pCLRollingDie->SetCel(0);                     /*Single*/
		for (ii = 1; ii < NUM_SINGLE_DIE_CELS; ii++) {
			m_pCLRollingDie->PaintSprite(pDC, *(LDieLeft + ii), *(LDieTop + ii)); /*Single*/
			pause();

			if (ii < NUM_SINGLE_DIE_CELS - 1) {
				// Slow down animation, except for the last cel (this enables quick
				// repainting of actual dice throws .)
				Sleep(22);

			} else if (pGameParams->bSoundEffectsEnabled) {
				sndPlaySound(nullptr, 0);                           // kill rattle
				sndPlaySound(GetStringFromResource(IDS_ROLL), SND_ASYNC);    // and roll!
			}

		}
	}

	ReleaseDC(pDC);

}


/*****************************************************************************************************************
*   [GetStringFromResource]
*
*   FUNCTIONAL DESCRIPRION:
*   Retrieves a string in a static buffer from a resource object.
*
*   CALLING SEQUENCE:
*   char* GetStringFromResource(unsigned int nResourceID)
*
*   FORMAL PARAMETERS:
*   unsigned int    nID     The ID of the string resource from the resource file.
*
*   IMPLICIT INPUT PARAMETERS:
*   static szBuffer[ ], the buffer for  storing the string.
*
*   IMPLICIT OUTPUT PARAMETERS:
*   N/A
*
*   RETURN VALUE:
*   static &szBuffer[0]

*   ENVIRONMENT:
*   n/a
****************************************************************************************************************
*/

char *GetStringFromResource(unsigned int nID) {
	static char szBuffer[256];

	if (LoadString(AfxGetResourceHandle(), nID, szBuffer, 256))
		return szBuffer;
	else
		return nullptr;
}

} // namespace NoVacancy
} // namespace HodjNPodj
} // namespace Bagel
