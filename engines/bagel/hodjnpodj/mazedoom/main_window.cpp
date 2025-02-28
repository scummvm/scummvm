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

#include "common/str.h"
#include "bagel/hodjnpodj/mazedoom/main_window.h"
#include "bagel/hodjnpodj/mazedoom/maze_doom.h"
#include "bagel/hodjnpodj/mazedoom/globals.h"
#include "bagel/hodjnpodj/mazedoom/option_dialog.h"
#include "bagel/hodjnpodj/hodjnpodj.h"
#include "bagel/hodjnpodj/boflib/bitmaps.h"
#include "bagel/hodjnpodj/libs/dib_doc.h"
#include "bagel/hodjnpodj/libs/types.h"
#include "bagel/hodjnpodj/libs/rules.h"
#include "bagel/hodjnpodj/libs/main_menu.h"
#include "bagel/boflib/rect.h"

namespace Bagel {
namespace HodjNPodj {
namespace MazeDoom {

CMainWindow::CMainWindow() {
	CString WndClass;
	CRect MainRect, tmpRect;
	CBitmap *pPartsBitmap = nullptr;
	CDC *pDC = nullptr;
	int i;

	BeginWaitCursor();

	if (pGameInfo->bPlayingHodj) {
		_upBmp = IDB_HODJ_UP;
		_downBmp = IDB_HODJ_DOWN;
		_leftBmp = IDB_HODJ_LEFT;
		_rightBmp = IDB_HODJ_RIGHT;
	} else {
		_upBmp = IDB_PODJ_UP;
		_downBmp = IDB_PODJ_DOWN;
		_leftBmp = IDB_PODJ_LEFT;
		_rightBmp = IDB_PODJ_RIGHT;
	}

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

	// Create the window as a POPUP so no boarders, title, or menu are present;
	// this is because the game's background art will fill the entire 640x480 area.
	Create(WndClass, "Boffo Games -- Maze o' Doom", WS_POPUP, MainRect, nullptr, nullptr);

	ShowWindow(SW_SHOWNORMAL);
	SplashScreen();

	// Build Scroll Command button
	m_pScrollButton = new CBmpButton;
	assert(m_pScrollButton != nullptr);
	tmpRect.SetRect(SCROLL_BUTTON_X, SCROLL_BUTTON_Y,
		SCROLL_BUTTON_X + SCROLL_BUTTON_DX - 1,
		SCROLL_BUTTON_Y + SCROLL_BUTTON_DY - 1);
	bSuccess = (*m_pScrollButton).Create(nullptr, BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, tmpRect, this, IDC_SCROLL);
	assert(bSuccess);
	bSuccess = (*m_pScrollButton).LoadBitmaps(SCROLLUP_BMP, SCROLLDOWN_BMP, nullptr, nullptr);
	assert(bSuccess);
	m_bIgnoreScrollClick = false;

	pMazeBitmap = new CBitmap();
	pMazeDC = new CDC();

	pMazeBitmap->CreateCompatibleBitmap(pDC, NUM_COLUMNS * SQ_SIZE_X, NUM_ROWS * SQ_SIZE_Y);  // Set up MazeBitmap
	pMazeDC->CreateCompatibleDC(pDC);                                 //...and DC
	pOldBmp = pMazeDC->SelectObject(pMazeBitmap);                     // select the bitmap in
	pOldPal = pMazeDC->SelectPalette(pGamePalette, false);            // select the game palette
	pMazeDC->RealizePalette();                                          //...and realize it


	// Load up the various bitmaps for wall, edge, booby traps, etc.
	//
	pPartsBitmap = FetchResourceBitmap(pDC, nullptr, IDB_PARTS);

	pWallBitmap = ExtractBitmap(pDC, pPartsBitmap, pGamePalette, WALL_X, WALL_Y, PATH_WIDTH, PATH_HEIGHT);
	pPathBitmap = ExtractBitmap(pDC, pPartsBitmap, pGamePalette, PATH_X, WALL_Y, PATH_WIDTH, PATH_HEIGHT);
	pStartBitmap = ExtractBitmap(pDC, pPartsBitmap, pGamePalette, START_X, WALL_Y, PATH_WIDTH, PATH_HEIGHT);

	pLeftEdgeBmp = ExtractBitmap(pDC, pPartsBitmap, pGamePalette,
		0, EDGE_Y, EDGE_WIDTH, EDGE_HEIGHT);
	pRightEdgeBmp = ExtractBitmap(pDC, pPartsBitmap, pGamePalette,
		EDGE_WIDTH, EDGE_Y, EDGE_WIDTH, EDGE_HEIGHT);
	pTopEdgeBmp = ExtractBitmap(pDC, pPartsBitmap, pGamePalette,
		EDGE_WIDTH * 2, EDGE_Y, EDGE_HEIGHT, EDGE_WIDTH);
	pBottomEdgeBmp = ExtractBitmap(pDC, pPartsBitmap, pGamePalette,
		(EDGE_WIDTH * 2) + EDGE_HEIGHT, EDGE_Y, EDGE_HEIGHT, EDGE_WIDTH);

	for (i = 0; i < NUM_TRAP_MAPS; i++) {
		TrapBitmap[i] = ExtractBitmap(pDC, pPartsBitmap, pGamePalette,
			TRAP_WIDTH * i, 0, TRAP_WIDTH, TRAP_HEIGHT);
	}

	if (pPartsBitmap != nullptr) {
		pPartsBitmap->DeleteObject();
		delete pPartsBitmap;
	}

	pPlayerSprite = new CSprite;
	(*pPlayerSprite).SharePalette(pGamePalette);
	bSuccess = (*pPlayerSprite).LoadResourceCels(pDC, _leftBmp, NUM_CELS);
	assert(bSuccess);
	(*pPlayerSprite).SetMasked(TRUE);
	(*pPlayerSprite).SetMobile(TRUE);

	pLocaleBitmap = FetchResourceBitmap(pDC, nullptr, "IDB_LOCALE_BMP");
	assert(pLocaleBitmap != nullptr);
	pBlankBitmap = FetchResourceBitmap(pDC, nullptr, "IDB_BLANK_BMP");
	assert(pBlankBitmap != nullptr);

	tmpRect.SetRect(TIME_LOCATION_X, TIME_LOCATION_Y,
		TIME_LOCATION_X + TIME_WIDTH, TIME_LOCATION_Y + TIME_HEIGHT);
	if ((m_pTimeText = new CText()) != nullptr) {
		(*m_pTimeText).SetupText(pDC, pGamePalette, &tmpRect, JUSTIFY_CENTER);
	}

	ReleaseDC(pDC);

	if (pGameInfo->bPlayingMetagame) {
		if (pGameInfo->nSkillLevel == SKILLLEVEL_LOW) {
			m_nDifficulty = MIN_DIFFICULTY;              // Total Wussy
			m_nTime = 60;
		} else if (pGameInfo->nSkillLevel == SKILLLEVEL_MEDIUM) {
			m_nDifficulty = 2;              // Big Sissy
			m_nTime = 60;
		} else {  //if (pGameInfo->nSkillLevel == SKILLLEVEL_HIGH) 
			m_nDifficulty = 4;              // Minor Whimp
			m_nTime = 60;
		}
	} // end if
	else {                                      // Use Defaults 
		m_nDifficulty = 6;                      // Miner
		m_nTime = 180;
	} // end else

	tempDifficulty = m_nDifficulty;
	tempTime = m_nTime;
	nSeconds = m_nTime % 60;
	nMinutes = m_nTime / 60;
	initialize_maze();         // draw the surrounding wall and start/end squares 
	create_maze();             // create a maze layout given the intiialized maze 
	SetUpMaze();                // "translate" from the created maze into uniform grid of doom
	PaintMaze(pMazeDC);       // draw it in the MazeBitmap 
	bPlaying = TRUE;
	SetTimer(GAME_TIMER, CLICK_TIME, nullptr);   // Reset ticker

	if (pGameInfo->bMusicEnabled) {
		pGameSound = new CSound(this, GAME_THEME, SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END);
		if (pGameSound != nullptr) {
			(*pGameSound).midiLoopPlaySegment(3000, 32980, 0, FMT_MILLISEC);
		} // end if pGameSound
	}

	EndWaitCursor();

	if (!pGameInfo->bPlayingMetagame)
		PostMessage(WM_COMMAND, IDC_SCROLL, BN_CLICKED);       // Activate the Options dialog

} // End of CMainWindow

// OnPaint:
// This is called whenever Windows sends a WM_PAINT message.
// Note that creating a CPaintDC automatically does a BeginPaint and
// an EndPaint call is done when it is destroyed at the end of this
// function.  CPaintDC's constructor needs the window (this).
//
void CMainWindow::OnPaint() {
	SplashScreen();
}

// Paint the background art (splash screen) in the client area;
// called by both OnPaint and InitInstance.
void CMainWindow::SplashScreen() {
	CRect rcDest;
	CRect rcDIB;
	CDC *pDC;
	CPalette *pPalOld = nullptr;                                                    // Old palette holder
	CDibDoc myDoc;
	HDIB hDIB;
	char msg[64];

	pDC = GetDC();

	myDoc.OpenDocument(MAIN_SCREEN);
	pGamePalette = myDoc.DetachPalette();				// Acquire the shared palette for our game from the art
	pPalOld = pDC->SelectPalette(pGamePalette, false);	// Select Game Palette
	pDC->RealizePalette();								// Realize the palette to prevent palette shifting

	hDIB = myDoc.GetHDIB();

	if (hDIB) {
		GetClientRect(rcDest);
		rcDIB = CRect(0, 0, hDIB->w, hDIB->h);

		PaintDIB((*pDC).m_hDC, &rcDest, hDIB, &rcDIB, pGamePalette);

		pDC->BitBlt(SIDE_BORDER, TOP_BORDER, ART_WIDTH, ART_HEIGHT, pMazeDC, 0, SQ_SIZE_Y / 2, SRCCOPY);    // Draw Maze
		if ((pPlayerSprite != nullptr) && bPlaying)
			(*pPlayerSprite).PaintSprite(pDC, (m_PlayerPos.x * SQ_SIZE_X) + SIDE_BORDER,
				(m_PlayerPos.y * SQ_SIZE_Y) + TOP_BORDER - SQ_SIZE_Y / 2); // Update PLAYER
	}

	if (bPlaying) {                       // only false when the options are displayed
		PaintBitmap(pDC, pGamePalette, pBlankBitmap, TIME_LOCATION_X, TIME_LOCATION_Y);
		if (m_nTime == 0)
			Common::sprintf_s(msg, "Time Used: %02d:%02d", nMinutes, nSeconds);
		else {
			Common::sprintf_s(msg, "Time Left: %02d:%02d", nMinutes, nSeconds);
		}
		(*m_pTimeText).DisplayString(pDC, msg, 16, FW_SEMIBOLD, OPTIONS_COLOR);
	} else {
		if (pLocaleBitmap != nullptr)
			PaintBitmap(pDC, pGamePalette, pLocaleBitmap, TIME_LOCATION_X, TIME_LOCATION_Y);
	}

	(*pDC).SelectPalette(pPalOld, false);                                         // Select back old palette
	ReleaseDC(pDC);
}

BOOL CMainWindow::OnCommand(WPARAM wParam, LPARAM lParam) {
	if (HIWORD(lParam) == BN_CLICKED) {
		CDC *pDC;
		CRules  RulesDlg((CWnd *)this, RULES_TEXT, pGamePalette,
			pGameInfo->bSoundEffectsEnabled ? RULES_WAV : nullptr);
		CMainMenu COptionsWind((CWnd *)this, pGamePalette,
			pGameInfo->bPlayingMetagame ? (NO_NEWGAME | NO_OPTIONS) : 0,
			GetSubOptions, RULES_TEXT,
			pGameInfo->bSoundEffectsEnabled ? RULES_WAV : nullptr, pGameInfo);       // Construct option dialog

		pDC = GetDC();
		PaintBitmap(pDC, pGamePalette, pLocaleBitmap, TIME_LOCATION_X, TIME_LOCATION_Y);

		switch (wParam) {
		case IDC_RULES:
			KillTimer(GAME_TIMER);
			CBofSound::waitWaveSounds();
			m_bIgnoreScrollClick = TRUE;
			(*m_pScrollButton).SendMessage(BM_SETSTATE, TRUE, 0L);

			(void)RulesDlg.DoModal();
			m_bIgnoreScrollClick = false;
			(*m_pScrollButton).SendMessage(BM_SETSTATE, false, 0L);
			if (!m_bGameOver)
				SetTimer(GAME_TIMER, CLICK_TIME, nullptr);                   // Reset ticker
			break;

		case IDC_SCROLL:
			KillTimer(GAME_TIMER);
			if (m_bIgnoreScrollClick) {
				(*m_pScrollButton).SendMessage(BM_SETSTATE, TRUE, 0L);
				break;
			}

			m_bIgnoreScrollClick = TRUE;
			(*m_pScrollButton).SendMessage(BM_SETSTATE, TRUE, 0L);
			SendDlgItemMessage(IDC_SCROLL, BM_SETSTATE, TRUE, 0L);

			CBofSound::waitWaveSounds();

			switch (COptionsWind.DoModal()) {

			case IDC_OPTIONS_NEWGAME:                           // Selected New Game
				(*m_pScrollButton).SendMessage(BM_SETSTATE, false, 0L);
				m_bIgnoreScrollClick = false;
				if (!pGameInfo->bPlayingMetagame)
					NewGame();
				break;

			case IDC_OPTIONS_RETURN:
				(*m_pScrollButton).SendMessage(BM_SETSTATE, false, 0L);
				m_bIgnoreScrollClick = false;
				if (!m_bGameOver)
					SetTimer(GAME_TIMER, CLICK_TIME, nullptr);   // Reset ticker
				break;

			case IDC_OPTIONS_QUIT:                      // Quit button was clicked
				if (pGameInfo->bPlayingMetagame)
					pGameInfo->lScore = 0;
				PostMessage(WM_CLOSE, 0, 0);            // and post a program exit
				ReleaseDC(pDC);
				return(false);

			} //end switch(ComDlg.DoModal())

			if (!pGameInfo->bMusicEnabled && (pGameSound != nullptr)) {

				pGameSound->stop();
				delete pGameSound;
				pGameSound = nullptr;

			} else if (pGameInfo->bMusicEnabled && (pGameSound == nullptr)) {
				pGameSound = new CBofSound(this, GAME_THEME, SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END);
				if (pGameSound != nullptr) {
					pGameSound->midiLoopPlaySegment(3000, 32980, 0, FMT_MILLISEC);
				}
			}

			m_bIgnoreScrollClick = false;
			(*m_pScrollButton).SendMessage(BM_SETSTATE, false, 0L);
			break;
		} //end switch(wParam)
		ReleaseDC(pDC);
	} // end if

	(*this).SetFocus();                     // Reset focus back to the main window

	return TRUE;
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
 *      UINT nFlags     Virtual key info
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
void CMainWindow::OnLButtonDown(UINT nFlags, CPoint point) {
#ifdef TODO
	CRect   rectTitle;

	rectTitle.setRect(NEWGAME_LOCATION_X, NEWGAME_LOCATION_Y,
		NEWGAME_LOCATION_X + NEWGAME_WIDTH,
		NEWGAME_LOCATION_Y + NEWGAME_HEIGHT);

	if (rectTitle.ptInRect(point) && (pGameInfo->bPlayingMetagame == false))
		NewGame();                              // Activate New Game

	if (bPlaying && InArtRegion(point)) {
		MovePlayer(point);
	}

	CFrameWnd::OnLButtonDown(nFlags, point);
#endif
}

/*****************************************************************
 *
 *  OnLButtonUp
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Standard Left mouse button processing function
 *
 *  FORMAL PARAMETERS:
 *
 *      UINT nFlags     Virtual key info
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
void CMainWindow::OnLButtonUp(UINT nFlags, CPoint point) {
	CFrameWnd::OnLButtonUp(nFlags, point);
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
 *      UINT nFlags     Virtual key info
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
void CMainWindow::OnMouseMove(UINT nFlags, CPoint point) {
#ifdef TODO
	if (InArtRegion(point) && bPlaying) {               // If the cursor is within the border
		GetNewCursor();                                     //...and we're playing, update the cursor
		if (nFlags & MK_LBUTTON) {                        // If the Left mouse button is down,
			MovePlayer(point);                            //...have the player follow the mouse
		}
	} else SetCursor(LoadCursor(nullptr, IDC_ARROW));        // Refresh cursor object to arrow
	//...when outside the maze area
	CFrameWnd::OnMouseMove(nFlags, point);
#endif
}

/*****************************************************************
 *
 *  OnRButtonDown
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Right mouse button processing function:  Undo last move
 *
 *  FORMAL PARAMETERS:
 *
 *      UINT nFlags     Virtual key info
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
void CMainWindow::OnRButtonDown(UINT nFlags, CPoint point) {

	CFrameWnd::OnRButtonDown(nFlags, point);

} // End OnRButtonDown

// OnChar and OnSysChar
// These functions are called when keyboard input generates a character.
//

void CMainWindow::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) {
	CFrameWnd::OnChar(nChar, nRepCnt, nFlags);     // default action
}

void CMainWindow::OnSysChar(UINT nChar, UINT nRepCnt, UINT nFlags) {
#ifdef TODO
	if ((nChar == 'q') && (nFlags & 0x2000)) {      // terminate app on ALT-q
		if (pGameInfo->bPlayingMetagame)
			pGameInfo->lScore = 0;
		PostMessage(WM_CLOSE, 0, 0);                   // *** remove later ***
	} else
		CFrameWnd::OnChar(nChar, nRepCnt, nFlags);  // default action
#endif
}

void CMainWindow::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
#ifdef TODO
	switch (nChar) {

		// User has hit ALT_F4 so close down this App
		//
	case VK_F4:
		if (pGameInfo->bPlayingMetagame)
			pGameInfo->lScore = 0;
		PostMessage(WM_CLOSE, 0, 0);
		break;

	default:
		CFrameWnd::OnSysKeyDown(nChar, nRepCnt, nFlags);
		break;
	}
#endif
}

void CMainWindow::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
#ifdef TODO
	CPoint  NewPosition;
	NewPosition = (*pPlayerSprite).GetPosition();

	switch (nChar) {
	case VK_F1:                                             // F1 key is hit
		SendMessage(WM_COMMAND, IDC_RULES, BN_CLICKED);    // Activate the Rules dialog
		break;

	case VK_F2:                                             // F2 key is hit
		SendMessage(WM_COMMAND, IDC_SCROLL, BN_CLICKED);   // Activate the Options dialog
		break;

	case 'h':
	case 'H':
	case VK_LEFT:
	case VK_NUMPAD4:
		NewPosition.x -= SQ_SIZE_X;
		MovePlayer(NewPosition);
		break;

	case 'k':
	case 'K':
	case VK_UP:
		//      case VK_NUMPAD8:
		NewPosition.y -= SQ_SIZE_Y;
		MovePlayer(NewPosition);
		break;

	case 'l':
	case 'L':
	case VK_RIGHT:
	case VK_NUMPAD6:
		NewPosition.x += SQ_SIZE_X;
		MovePlayer(NewPosition);
		break;

	case 'j':
	case 'J':
	case VK_DOWN:
	case VK_NUMPAD2:
		NewPosition.y += SQ_SIZE_Y;
		MovePlayer(NewPosition);
		break;
	}
#endif
}


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
 *      UINT nIDEvent   The ID of the timer event activated
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
void CMainWindow::OnTimer(UINT nIDEvent) {
#ifdef TODO
	CDC *pDC;
	CSound *pEffect = nullptr;
	char    msg[64];

	pDC = GetDC();

	switch (nIDEvent) {

	case GAME_TIMER:
		if (m_nTime == 0) {                       // No time limit, increment 
			nSeconds++;
			if (nSeconds == 60) {
				nMinutes++;
				nSeconds = 0;
			}
		}

		else {                                      // Count down time left
			if (nSeconds == 0 && nMinutes != 0) {
				nMinutes--;
				nSeconds = 60;
			}
			nSeconds--;
		}

		if (m_nTime == 0)
			Common::sprintf_s(msg, "Time Used: %02d:%02d", nMinutes, nSeconds);
		else {
			Common::sprintf_s(msg, "Time Left: %02d:%02d", nMinutes, nSeconds);
		}
		(*m_pTimeText).DisplayString(pDC, msg, 16, FW_SEMIBOLD, OPTIONS_COLOR);

		if (nMinutes == 0 && nSeconds == 0) {           // No time left on the clock!! 
			KillTimer(nIDEvent);                      // Stop the Display timer
			bPlaying = false;
			m_bGameOver = TRUE;
			if (pGameInfo->bSoundEffectsEnabled) {
				pEffect = new CSound((CWnd *)this, LOSE_SOUND,
					SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE);  //...Wave file, to delete itself
				(*pEffect).Play();                                                      //...play the narration
			}
			MSG lpmsg;
			while (PeekMessage(&lpmsg, m_hWnd, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE));

			CMessageBox GameOverDlg((CWnd *)this, pGamePalette, "Game over.", "Time ran out!");
			CBofSound::WaitWaveSounds();
			if (pGameInfo->bPlayingMetagame) {
				pGameInfo->lScore = 0;
				PostMessage(WM_CLOSE, 0, 0);            // and post a program exit
			}
		}

		break;

	default:
		CFrameWnd::OnTimer(nIDEvent);
		break;
	}

	ReleaseDC(pDC);
#endif
}

/**********************************************************
Other functions:
***********************************************************/

/*****************************************************************
 *
 *  NewGame
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Set up and start a new game
 *
 *  FORMAL PARAMETERS:
 *
 *      none
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
void CMainWindow::NewGame() {
#ifdef TODO
	CDC *pDC;
	char    msg[64];

	pDC = GetDC();

	m_nTime = tempTime;;                            // get new time limit,
	m_nDifficulty = tempDifficulty;                 //...new Difficulty

	if (pPlayerSprite != nullptr)                    // Refresh PLAYER
		(*pPlayerSprite).EraseSprite(pDC);        // Erase PlayerSprite

	if (m_nTime != 0) {                             // If we've got a time limit
		nMinutes = m_nTime / 60;                    //...get the minutes and seconds
		nSeconds = m_nTime % 60;
	} else {
		nMinutes = 0;
		nSeconds = 0;
	}

	initialize_maze();                              // draw the surrounding wall and start/end squares 
	create_maze();                                  // create a maze layout given the intiialized maze 
	SetUpMaze();                                    // translate maze data to grid layout for display
	PaintMaze(pMazeDC);                           // paint that sucker to the offscreen bitmap
	pDC->BitBlt(SIDE_BORDER, TOP_BORDER, ART_WIDTH, ART_HEIGHT, pMazeDC, 0, SQ_SIZE_Y / 2, SRCCOPY);    // Draw Maze
	if (pPlayerSprite != nullptr)
		(*pPlayerSprite).PaintSprite(pDC, (m_PlayerPos.x * SQ_SIZE_X) + SIDE_BORDER,
			(m_PlayerPos.y * SQ_SIZE_Y) + TOP_BORDER - SQ_SIZE_Y / 2); // Display PLAYER
	bPlaying = TRUE;                                // Game is started
	m_bGameOver = false;

	PaintBitmap(pDC, pGamePalette, pBlankBitmap, TIME_LOCATION_X, TIME_LOCATION_Y);
	if (m_nTime == 0)
		Common::sprintf_s(msg, "Time Used: %02d:%02d", nMinutes, nSeconds);
	else {
		Common::sprintf_s(msg, "Time Left: %02d:%02d", nMinutes, nSeconds);
	}
	(*m_pTimeText).DisplayString(pDC, msg, 16, FW_SEMIBOLD, OPTIONS_COLOR);

	SetTimer(GAME_TIMER, CLICK_TIME, nullptr);       // Reset ticker

	ReleaseDC(pDC);
#endif
} // end NewGame

/*****************************************************************
 *
 *  MovePlayer
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Mouse movement processing function
 *
 *  FORMAL PARAMETERS:
 *
 *      UINT nFlags     Virtual key info
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
void CMainWindow::MovePlayer(CPoint point) {
#ifdef TODO
	CDC *pDC;
	CSound *pEffect = nullptr;
	CPoint  NewPosition;
	CPoint  TileLocation;
	CPoint  Hit;
	BOOL    bCollision = false;
	POINT   Delta;
	POINT   Step;
	UINT    nBmpID = IDB_HODJ_RIGHT;

	pDC = GetDC();

	Hit = ScreenToTile(point);

	Step.x = 0;
	Step.y = 0;

	NewPosition.x = m_PlayerPos.x;
	NewPosition.y = m_PlayerPos.y;

	Delta.x = m_PlayerPos.x - Hit.x;            // Get x distance from mouse click to player in Tile spaces 
	Delta.y = m_PlayerPos.y - Hit.y;            // Get y distance from mouse click to player in Tile spaces 

	if (ABS(Delta.x) > ABS(Delta.y)) {     // Moving horizontally:
		if (Delta.x < 0) {                    // To the RIGHT
			Step.x = 1;                             // move one tile at a time
			nBmpID = _rightBmp;  // use the Bitmap of the player moving Right
		} else if (Delta.x > 0) {               // To the LEFT
			Step.x = -1;                            // move one tile at a time
			nBmpID = _leftBmp;   // use Bitmap of player moving Left
		}
	} else if (ABS(Delta.y) > ABS(Delta.x)) {
		if (Delta.y > 0) {                    // Going UPward
			Step.y = -1;                            // move one tile at a time                                         
			nBmpID = _upBmp;     // use Bitmap of player moving Up
		} else if (Delta.y < 0) {               // Going DOWNward
			Step.y = 1;                             // move one tile at a time
			nBmpID = _downBmp;   // use Bitmap of player moving Down
		}
	}

	if ((Step.x != 0) || (Step.y != 0)) {      // If the click is not in the Player's Tile
		bSuccess = (*pPlayerSprite).LoadResourceCels(pDC, nBmpID, NUM_CELS);
		assert(bSuccess);
		if (pPlayerSprite != nullptr)                                            // Refresh PLAYER
			(*pPlayerSprite).PaintSprite(pDC, (m_PlayerPos.x * SQ_SIZE_X) + SIDE_BORDER,
				(m_PlayerPos.y * SQ_SIZE_Y) + TOP_BORDER - SQ_SIZE_Y / 2);   //...in new direction

		while (!bCollision) {

			NewPosition.Offset(Step);

			if (mazeTile[NewPosition.x][NewPosition.y].m_nWall == PATH ||      // Either a pathway 
				((mazeTile[NewPosition.x][NewPosition.y].m_nWall == TRAP &&     //...or a 
					mazeTile[NewPosition.x][NewPosition.y].m_bHidden == false) ||  //...revealed trap
					mazeTile[NewPosition.x][NewPosition.y].m_nWall == EXIT)) {   //...or exit is a GO
				int i, x, y;
				x = (m_PlayerPos.x * SQ_SIZE_X) + SIDE_BORDER;                  // Get player's position
				y = (m_PlayerPos.y * SQ_SIZE_Y) + TOP_BORDER - SQ_SIZE_Y / 2;     //...in screen coords
				for (i = 0; i < 4; i++) {                                     // Go through three cels  
					x += Step.x * i * (SQ_SIZE_X / 4);                            //...per tile moved
					y += Step.y * i * (SQ_SIZE_Y / 4);
					if (pPlayerSprite != nullptr)
						(*pPlayerSprite).PaintSprite(pDC, x, y);               // Update PLAYER
				} // end for
				m_PlayerPos.x = NewPosition.x;
				m_PlayerPos.y = NewPosition.y;
				if (pPlayerSprite != nullptr)                                    // Refresh PLAYER
					(*pPlayerSprite).PaintSprite(pDC, (m_PlayerPos.x * SQ_SIZE_X) + SIDE_BORDER,
						(m_PlayerPos.y * SQ_SIZE_Y) + TOP_BORDER - SQ_SIZE_Y / 2);   //...in new direction
			} // end if

			if ((mazeTile[NewPosition.x][NewPosition.y].m_bHidden) &&
				(mazeTile[NewPosition.x][NewPosition.y].m_nWall == WALL)) {

				if (pGameInfo->bSoundEffectsEnabled) {
					pEffect = new CSound((CWnd *)this, HIT_SOUND,
						SOUND_WAVE | SOUND_AUTODELETE); //| SOUND_ASYNCH ...Wave file, to delete itself
					(*pEffect).Play();                                                      //...play the narration
				}
				mazeTile[NewPosition.x][NewPosition.y].m_bHidden = false;

				if (pPlayerSprite != nullptr)                                    // Refresh PLAYER
					(*pPlayerSprite).EraseSprite(pDC);                        // Erase PlayerSprite
				PaintBitmap(pDC, pGamePalette, pWallBitmap,                    // Paint wall on screen 
					mazeTile[NewPosition.x][NewPosition.y].m_nStart.x + SIDE_BORDER,
					mazeTile[NewPosition.x][NewPosition.y].m_nStart.y + TOP_BORDER - SQ_SIZE_Y / 2);
				AddEdges(pDC, NewPosition.x, NewPosition.y, SIDE_BORDER, TOP_BORDER - SQ_SIZE_Y / 2);
				PaintBitmap(pMazeDC, pGamePalette, pWallBitmap,                // Paint wall in Maze Bitmap
					mazeTile[NewPosition.x][NewPosition.y].m_nStart.x,
					mazeTile[NewPosition.x][NewPosition.y].m_nStart.y);
				AddEdges(pMazeDC, NewPosition.x, NewPosition.y, 0, 0);
				if (pPlayerSprite != nullptr)                                    // Refresh PLAYER
					(*pPlayerSprite).PaintSprite(pDC, (m_PlayerPos.x * SQ_SIZE_X) + SIDE_BORDER,
						(m_PlayerPos.y * SQ_SIZE_Y) + TOP_BORDER - SQ_SIZE_Y / 2);   //...in new direction
				bCollision = TRUE;
			}

			if (mazeTile[NewPosition.x][NewPosition.y].m_nWall == TRAP &&
				(mazeTile[NewPosition.x][NewPosition.y].m_bHidden)) {      // Traps are only good once 

				mazeTile[NewPosition.x][NewPosition.y].m_bHidden = false;
				m_PlayerPos.x = mazeTile[NewPosition.x][NewPosition.y].m_nDest.x;
				m_PlayerPos.y = mazeTile[NewPosition.x][NewPosition.y].m_nDest.y;
				if (pPlayerSprite != nullptr)
					(*pPlayerSprite).EraseSprite(pDC);                    // Erase PlayerSprite
				PaintBitmap(pDC, pGamePalette,                             // Paint trap on screen 
					TrapBitmap[mazeTile[NewPosition.x][NewPosition.y].m_nTrap],
					mazeTile[NewPosition.x][NewPosition.y].m_nStart.x + SIDE_BORDER,
					mazeTile[NewPosition.x][NewPosition.y].m_nStart.y + TOP_BORDER - SQ_SIZE_Y / 2);

				if (pGameInfo->bSoundEffectsEnabled) {
					pEffect = new CSound((CWnd *)this, TRAP_SOUND,
						SOUND_WAVE | SOUND_AUTODELETE); //| SOUND_ASYNCH ...Wave file, to delete itself
					(*pEffect).Play();                                                      //...play the narration
				}
				PaintBitmap(pMazeDC, pGamePalette,                         // Paint trap in Maze Bitmap
					TrapBitmap[mazeTile[NewPosition.x][NewPosition.y].m_nTrap],
					mazeTile[NewPosition.x][NewPosition.y].m_nStart.x,
					mazeTile[NewPosition.x][NewPosition.y].m_nStart.y);
				if (pPlayerSprite != nullptr)
					(*pPlayerSprite).PaintSprite(pDC, (m_PlayerPos.x * SQ_SIZE_X) + SIDE_BORDER,
						(m_PlayerPos.y * SQ_SIZE_Y) + TOP_BORDER - SQ_SIZE_Y / 2); // Update PLAYER
				bCollision = TRUE;
			}

			if ((mazeTile[NewPosition.x][NewPosition.y].m_nWall == WALL) ||
				(mazeTile[NewPosition.x][NewPosition.y].m_nWall == START)) {
				bCollision = TRUE;
			}

			if (mazeTile[NewPosition.x][NewPosition.y].m_nWall == EXIT) {
				bPlaying = false;
				m_bGameOver = TRUE;
				KillTimer(GAME_TIMER);

				if (pGameInfo->bSoundEffectsEnabled) {
					pEffect = new CSound((CWnd *)this, WIN_SOUND,
						SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE);  //...Wave file, to delete itself
					(*pEffect).Play();                                                      //...play the narration
				}
				MSG lpmsg;
				while (PeekMessage(&lpmsg, m_hWnd, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE));
				CMessageBox GameOverDlg((CWnd *)this, pGamePalette, "Game over.", "He's free!");
				CBofSound::WaitWaveSounds();
				if (pGameInfo->bPlayingMetagame) {
					pGameInfo->lScore = 1;                  // A victorious maze solving
					PostMessage(WM_CLOSE, 0, 0);            // and post a program exit
				}
				bCollision = TRUE;
			}

			if ((NewPosition.x == Hit.x) && (NewPosition.y == Hit.y))
				bCollision = TRUE;
		} // end while
		GetNewCursor();
	} // end if       

	ReleaseDC(pDC);
#endif
}

/*****************************************************************
 *
 *  GetNewCursor
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Loads up a new cursor with regard to the current cursor position, and the player position
 *
 *  FORMAL PARAMETERS:
 *
 *      none
 *
 *  IMPLICIT INPUT PARAMETERS:
 *
 *      CPoint m_PlayerPos      The player's current location in Grid coordinates
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
void CMainWindow::GetNewCursor() {
#ifdef TODO
	CPoint  Hit, Delta;
	POINT   pCursorLoc;

	HCURSOR hNewCursor = nullptr;
	CWinApp *pMyApp;

	pMyApp = AfxGetApp();

	::GetCursorPos(&pCursorLoc);
	::ScreenToClient(m_hWnd, &pCursorLoc);
	Delta.x = pCursorLoc.x;
	Delta.y = pCursorLoc.y;
	Hit = ScreenToTile(Delta);

	Delta.x = m_PlayerPos.x - Hit.x;
	Delta.y = m_PlayerPos.y - Hit.y;

	if ((m_PlayerPos.x == Hit.x) && (m_PlayerPos.y == Hit.y)) {     // Directly over player
		hNewCursor = (*pMyApp).LoadCursor(IDC_MOD_NOARROW);
	}

	else if (ABS(Delta.x) >= ABS(Delta.y)) {                   // Moving horizontally:
		if (Delta.x <= 0)                                         // To the RIGHT
			hNewCursor = (*pMyApp).LoadCursor(IDC_MOD_RTARROW);
		else if (Delta.x > 0)                                     // To the LEFT
			hNewCursor = (*pMyApp).LoadCursor(IDC_MOD_LFARROW);
	} else if (ABS(Delta.y) > ABS(Delta.x)) {
		if (Delta.y >= 0)                                         // Going UPward
			hNewCursor = (*pMyApp).LoadCursor(IDC_MOD_UPARROW);
		else if (Delta.y < 0)                                     // Going DOWNward
			hNewCursor = (*pMyApp).LoadCursor(IDC_MOD_DNARROW);
	}

	if (hNewCursor != nullptr);
	::SetCursor(hNewCursor);
#endif
}

void CMainWindow::OnClose() {
#ifdef TODO
	CDC *pDC;
	CBrush  myBrush;
	CRect   myRect;

	pDC = GetDC();
	myRect.setRect(0, 0, GAME_WIDTH, GAME_HEIGHT);
	myBrush.CreateStockObject(BLACK_BRUSH);
	(*pDC).FillRect(&myRect, &myBrush);
	ReleaseDC(pDC);

	SetCursor(LoadCursor(nullptr, IDC_ARROW));        // Refresh cursor object to arrow

	// delete the game theme song
	//
	if (pGameSound != nullptr) {
		delete pGameSound;
		pGameSound = nullptr;
	}

	CBofSound::ClearSounds();                              // Clean up sounds before returning

	if (m_pScrollButton != nullptr)
		delete m_pScrollButton;

	if (m_pTimeText != nullptr)
		delete m_pTimeText;

	if (pBlankBitmap != nullptr) {
		pBlankBitmap->DeleteObject();
		delete pBlankBitmap;
	}

	if (pLocaleBitmap != nullptr) {
		pLocaleBitmap->DeleteObject();
		delete pLocaleBitmap;
	}

	if (pPlayerSprite != nullptr)
		delete pPlayerSprite;

	if (pOldBmp != nullptr)                    // Get rid of Scratch1
		pMazeDC->SelectObject(pOldBmp);
	if (pOldPal != nullptr)
		pMazeDC->SelectPalette(pOldPal, false);
	if (pMazeDC->m_hDC != nullptr) {
		pMazeDC->DeleteDC();
		delete pMazeDC;
	}

	if (pMazeBitmap != nullptr) {
		pMazeBitmap->DeleteObject();
		delete pMazeBitmap;
	}
	if (pPathBitmap != nullptr) {
		pPathBitmap->DeleteObject();
		delete pPathBitmap;
	}
	if (pTopEdgeBmp != nullptr) {
		pTopEdgeBmp->DeleteObject();
		delete pTopEdgeBmp;
	}
	if (pRightEdgeBmp != nullptr) {
		pRightEdgeBmp->DeleteObject();
		delete pRightEdgeBmp;
	}
	if (pBottomEdgeBmp != nullptr) {
		pBottomEdgeBmp->DeleteObject();
		delete pBottomEdgeBmp;
	}
	if (pLeftEdgeBmp != nullptr) {
		pLeftEdgeBmp->DeleteObject();
		delete pLeftEdgeBmp;
	}
	if (pWallBitmap != nullptr) {
		pWallBitmap->DeleteObject();
		delete pWallBitmap;
	}
	if (pStartBitmap != nullptr) {
		pStartBitmap->DeleteObject();
		delete pStartBitmap;
	}
	for (int i = 0; i < NUM_TRAP_MAPS; i++) {
		if (TrapBitmap[i] != nullptr) {
			TrapBitmap[i]->DeleteObject();
			delete TrapBitmap[i];
		}
	}
	if (pGamePalette != nullptr) {
		pGamePalette->DeleteObject();
		delete pGamePalette;
	}

	CFrameWnd::OnClose();
#ifdef  _USRDLL
	::PostMessage(ghParentWnd, WM_PARENTNOTIFY, WM_DESTROY, 0L);
#endif  //_USRDLL
#endif
}

//////////// Additional Sound Notify routines //////////////

long CMainWindow::OnMCINotify(WPARAM wParam, LPARAM lParam) {
#ifdef TODO
	CSound *pSound;

	pSound = CBofSound::OnMCIStopped(wParam, lParam);
	if (pSound != nullptr)
		OnSoundNotify(pSound);
#endif
	return 0L;
}


long CMainWindow::OnMMIONotify(WPARAM wParam, LPARAM lParam) {
#ifdef TODO
	CSound *pSound;

	pSound = CBofSound::OnMMIOStopped(wParam, lParam);
	if (pSound != nullptr)
		OnSoundNotify(pSound);
#endif
	return 0L;
}

} // namespace MazeDoom
} // namespace HodjNPodj
} // namespace Bagel
