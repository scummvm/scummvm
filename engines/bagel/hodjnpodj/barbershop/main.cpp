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
#include "bagel/hodjnpodj/hnplibs/mainmenu.h"
#include "bagel/hodjnpodj/hnplibs/cmessbox.h"
#include "bagel/boflib/misc.h"
#include "bagel/hodjnpodj/hnplibs/rules.h"
#include "bagel/boflib/error.h"
#include "bagel/hodjnpodj/barbershop/init.h"
#include "bagel/hodjnpodj/barbershop/main.h"
#include "bagel/hodjnpodj/barbershop/undo.h"
#include "bagel/hodjnpodj/barbershop/usercfg.h"

namespace Bagel {
namespace HodjNPodj {
namespace Barbershop {

//
// This mini-game's main screen bitmap
//
#define MINI_GAME_MAP   ".\\ART\\barb.BMP"

//
// This mini-game's sound file
//
#define MIDI_SOUND      ".\\SOUND\\barber.mid"

#define RULES_TXT       "barb.txt"
#define RULES_WAV       ".\\sound\\rlbq.wav"


//
// Win/Lose conditions
//
#define WIN             1
#define LOSE            0

//
// Button ID constants
//
#define IDC_MENU     100

#define TIMER_ID 10

// Local Prototypes
//
void CALLBACK GetGameParams(CWnd *);

//
// Globals
//
CPalette    *pGamePalette;
LPGAMESTRUCT pGameParams;
int         g_nCardBack;
extern HWND ghParentWnd;


CMainWindow::CMainWindow() {
	CString  WndClass;
	CRect    tmpRect;
	CDC     *pDC = nullptr;
	CDibDoc *pDibDoc;
	ERROR_CODE errCode;
	bool bSuccess;

	// assume no error
	errCode = ERR_NONE;

	// Initialize members
	//
	m_pScrollSprite = nullptr;
	m_pGamePalette = nullptr;
	m_bPause = false;
	m_bGameActive = false;
	m_bInMenu = false;
	m_bMIDIPlaying = false;
	m_pBarb = nullptr;
	m_pMIDISound = nullptr;

	// Initialize score to LOSE condition in case the user bails out
	//
	pGameParams->lScore = LOSE;


	// Set the coordinates for the "Start New Game" button
	//
	m_rNewGameButton.SetRect(15, 4, 233, 20);

	// Define a special window class which traps double-clicks, is byte aligned
	// to maximize BITBLT performance, and creates "owned" DCs rather than sharing
	// the five system defined DCs which are not guaranteed to be available;
	// this adds a bit to our app size but avoids hangs/freezes/lockups.
	WndClass = AfxRegisterWndClass(CS_DBLCLKS | CS_BYTEALIGNWINDOW | CS_OWNDC, nullptr, nullptr, nullptr);

	// can't play this game if the background art is not available
	//
	if (FileExists(MINI_GAME_MAP)) {

		// Acquire the shared palette for our game from the splash screen art
		//
		if ((pDibDoc = new CDibDoc()) != nullptr) {
			if (pDibDoc->OpenDocument(MINI_GAME_MAP) != false)
				pGamePalette = m_pGamePalette = pDibDoc->DetachPalette();
			else
				errCode = ERR_UNKNOWN;
			delete pDibDoc;
		} else {
			errCode = ERR_MEMORY;
		}
	} else {
		errCode = ERR_FFIND;
	}

	// Center our window on the screen
	//
	tmpRect.SetRect(0, 0, GAME_WIDTH, GAME_HEIGHT);

	if ((pDC = GetDC()) != nullptr) {
		tmpRect.left = (pDC->GetDeviceCaps(HORZRES) - GAME_WIDTH) >> 1;
		tmpRect.top = (pDC->GetDeviceCaps(VERTRES) - GAME_HEIGHT) >> 1;
		tmpRect.right = tmpRect.left + GAME_WIDTH;
		tmpRect.bottom = tmpRect.top + GAME_HEIGHT;
		ReleaseDC(pDC);
	} else {
		errCode = ERR_UNKNOWN;
	}

	// Create the window as a POPUP so no boarders, title, or menu are present;
	// this is because the game's background art will fill the entire 640x480 area.
	//
	Create(WndClass, "Boffo Games -- Barbershop Quintet", WS_POPUP, tmpRect, nullptr, 0);

	BeginWaitCursor();
	ShowWindow(SW_SHOWNORMAL);
	PaintScreen();
	EndWaitCursor();

	// only continue if there was no error
	//
	if (errCode == ERR_NONE) {

		if ((pDC = GetDC()) != nullptr) {

			//
			// build our main menu button
			//
			if ((m_pScrollSprite = new CSprite) != nullptr) {
				m_pScrollSprite->SharePalette(m_pGamePalette);
				bSuccess = m_pScrollSprite->LoadSprite(pDC, ".\\ART\\SCROLBTN.BMP");
				assert(bSuccess);
				if (bSuccess) {
					m_pScrollSprite->SetMasked(true);
					m_pScrollSprite->SetMobile(true);
				} else {
					errCode = ERR_UNKNOWN;
				}
			} else {
				errCode = ERR_MEMORY;
			}

			// only continue if there was no error
			//
			if (errCode == ERR_NONE) {

				// seed the random number generator
				//srand((unsigned)time(nullptr));
			}

			ReleaseDC(pDC);
		}
	}

	if (errCode == ERR_NONE) {
		//
		// Initialize midi music
		//
		if ((m_pMIDISound = new CSound((CWnd *) this, MIDI_SOUND, SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END)) == nullptr) {
			errCode = ERR_MEMORY;
		}
	}

	if (errCode == ERR_NONE) {
		//
		// Initialize wav sound
		//
		if ((m_pWavSound = new CSound) == nullptr) {
			errCode = ERR_MEMORY;
		}
	}

	if (errCode == ERR_NONE) {
		//
		// Initialize Barbershop Quintet!
		//
		if ((m_pBarb = new CBarber(pDC, m_pWavSound)) == nullptr) {
			errCode = ERR_MEMORY;
		} else {
			g_nCardBack = m_pBarb->m_cPnt->m_nCardBack;
		}   // end if
	}

	if (errCode == ERR_NONE) {
		//
		// Initialize animations
		//
		if ((m_pAnim = new CAnimate(m_pWavSound)) == nullptr) {
			errCode = ERR_MEMORY;
		}
	}

	if (errCode == ERR_NONE) {
		// Start the game theme song
		//
		if (pGameParams->bMusicEnabled) {
			m_bMIDIPlaying = true;
			m_pMIDISound->midiLoopPlaySegment(2000L, 33560L, 00L, FMT_MILLISEC);
		}

		// Automatically bring up the main menu if in stand alone mode
		//
		if (pGameParams->bPlayingMetagame != false) {
			PostMessage(WM_COMMAND, IDC_OPTIONS_NEWGAME, BN_CLICKED);
		} else {
			PostMessage(WM_COMMAND, IDC_MENU, BN_CLICKED);
		}
	}

	HandleError(errCode);
}

void CMainWindow::HandleError(ERROR_CODE errCode) {
	//
	// Exit this application on fatal errors
	//
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
	//CSprite *pSprite;
	CDibDoc myDoc;
	CRect   rcDest;
	CRect   rcDIB;
	HDIB    hDIB;
	CDC     *pDC;

	//
	// Paint the background art and upadate any sprites
	// called by OnPaint
	//
	if (FileExists(MINI_GAME_MAP)) {

		myDoc.OpenDocument(MINI_GAME_MAP);
		hDIB = myDoc.GetHDIB();

		pDC = GetDC();
		assert(pDC != nullptr);

		if (pDC != nullptr) {

			if (hDIB && (m_pGamePalette != nullptr)) {

				GetClientRect(rcDest);
				rcDIB.top = rcDIB.left = 0;
				rcDIB.right = (int) DIBWidth(hDIB);
				rcDIB.bottom = (int) DIBHeight(hDIB);
				PaintDIB(pDC->m_hDC, &rcDest, hDIB, &rcDIB, m_pGamePalette);
			}

			// repaint the command scroll
			//
			if (!m_bInMenu && (m_pScrollSprite != nullptr)) {
				m_pScrollSprite->PaintSprite(pDC, SCROLL_BUTTON_X, SCROLL_BUTTON_Y);
			}

			if (m_pBarb != nullptr)
				m_pBarb->Refresh(pDC);  // repaint the board of cards

			ReleaseDC(pDC);
		}
	}
}


bool CMainWindow::OnCommand(WPARAM wParam, LPARAM lParam) {
	CDC *pDC;
	bool bSuccess;

	if (HIWORD(lParam) == BN_CLICKED) {
		switch (wParam) {

		//
		// must bring up our menu of controls
		//
		case IDC_MENU: {

			GamePause();

			// don't display the command scroll when in the menu
			m_bInMenu = true;

			// hide the command scroll
			//
			pDC = GetDC();
			bSuccess = m_pScrollSprite->EraseSprite(pDC);

			// Create the commands menu
			//
			CMainMenu COptionsWind(
			    (CWnd *)this,
			    m_pGamePalette,
			    //NO_OPTIONS |
			    (m_bGameActive ? 0 : NO_RETURN) |
			    (pGameParams->bPlayingMetagame ? NO_OPTIONS : 0) |
			    (pGameParams->bPlayingMetagame ? NO_NEWGAME : 0),
			    GetGameParams,
			    RULES_TXT,
			    pGameParams->bSoundEffectsEnabled ? RULES_WAV : nullptr,
			    pGameParams
			);

			CSound::waitWaveSounds();

			// Get users choice from command menu
			//
			switch (COptionsWind.DoModal()) {

			// User has chosen to play a new game
			//
			case IDC_OPTIONS_NEWGAME:
				if (m_pBarb->IsNewBack(g_nCardBack) == true) {      // need to card back?
					m_pBarb->ChangeBack(pDC, g_nCardBack);          // yes - change it
				} // end if

				PlayGame();
				break;

			// User has chosen to quit this mini-game
			//
			case IDC_OPTIONS_QUIT:
				pGameParams->lScore = LOSE;     // make sure the score is zero if they quit
				PostMessage(WM_CLOSE, 0, 0);
				break;

			default:
				if (m_pBarb->IsNewBack(g_nCardBack) == true) {      // need to card back?
					m_pBarb->ChangeBack(pDC, g_nCardBack);          // yes - change it
					Invalidate(true);                               // set up for a redraw window
				} // end if
				break;

			} // end switch

			if (!pGameParams->bMusicEnabled && m_bMIDIPlaying) {

				m_pMIDISound->stop();
				m_bMIDIPlaying = false;

			} else if (pGameParams->bMusicEnabled && !m_bMIDIPlaying) {

				m_pMIDISound->midiLoopPlaySegment(2470, 32160, 0, FMT_MILLISEC);
				m_bMIDIPlaying = true;
			}

			// show the command scroll
			//
			bSuccess = m_pScrollSprite->PaintSprite(pDC, SCROLL_BUTTON_X, SCROLL_BUTTON_Y);
			assert(bSuccess);
			ReleaseDC(pDC);

			// ok to display the command scroll now
			//
			m_bInMenu = false;
			GameResume();
			return true;
		} // end case

		case IDC_OPTIONS_NEWGAME:
			PlayGame();
			break;

		} // end switch
	} // end if

	return false;
}


void CMainWindow::GamePause() {
	m_bPause = true;
}

void CMainWindow::GameResume() {
	m_bPause = false;
}

void CMainWindow::PlayGame() {
	CDC         *pDC;
	ERROR_CODE  errCode;

	// assume no error
	errCode = ERR_NONE;

	// load the .INI settings
	//
	LoadIniSettings();

	// reset all game parameters
	//
	GameReset();

	if (errCode == ERR_NONE) {
		if ((pDC = GetDC()) != nullptr) {
			//
			// Start game
			//
			//CSprite::FlushSprites();
			CSprite::FlushSpriteChain();        // Delete cards from memory
			Invalidate(true);
			UpdateWindow();
			/*
			            if ( pGameParams->bMusicEnabled != false ) {
			                m_pMIDISound->midiLoopPlaySegment(2000L, 33560L, 00L, FMT_MILLISEC);
			            }
			*/
			m_pBarb->NewGame(pDC);
			m_bGameActive = true;
			ReleaseDC(pDC);
		} else {
			errCode = ERR_MEMORY;
		} // end if
	} // end if

	HandleError(errCode);
}

void CMainWindow::LoadIniSettings() {
}

void CMainWindow::SaveIniSettings() {
}


void CMainWindow::GameReset() {
	//sndPlaySound(nullptr, SND_SYNC);               // stop all sounds
}



void CMainWindow::OnTimer(uintptr nEvent) {
	//CDC *pDC;

	// continue as long as there is a currently active non-paused game
	//
	//if (m_bGameActive && !m_bPause) {
	//	nEvent = 0x00;          // bull shit code rids warnings
	//}
}


void CMainWindow::OnMouseMove(unsigned int nFlags, CPoint point) {
	CDC *pDC;

	SetCursor(LoadCursor(nullptr, IDC_ARROW));
	if (m_pBarb->m_pCrd != nullptr) {
		pDC = GetDC();
		m_pBarb->OnMouseMove(pDC, point);
		ReleaseDC(pDC);
	}
	//nFlags = 0x00;
}


void CMainWindow::OnRButtonDown(unsigned int nFlags, CPoint point) {
	CPoint  UndoPoint(UNDO_LEF + (UNDO_RIG - UNDO_LEF) / 2, UNDO_TOP + (UNDO_BOT - UNDO_TOP) / 2);

	if (m_pBarb->m_pCrd != nullptr)        // r we currently moving a card?
		return;                         // Yes - just quit.

	if (m_pBarb->m_bIsGameOver == false) {
		m_pBarb->OnLButtonDown(
		    (CWnd*) this,
		    m_pGamePalette,
		    UndoPoint
		);
	}
}

void CMainWindow::OnLButtonDown(unsigned int nFlags, CPoint point) {
	CDC     *pDC;
	char    buf[32];
	CRect   tmpRect;

	if (m_pBarb->m_pCrd != nullptr)        // r we currently moving a card?
		return;                         // Yes - just quit.

	if (m_pScrollSprite != nullptr)
		tmpRect = m_pScrollSprite->GetRect();

	pDC = GetDC();
	// User clicked on the Menu button
	//
	if (tmpRect.PtInRect(point)) {

		// bring up the menu
		PostMessage(WM_COMMAND, IDC_MENU, BN_CLICKED);

		// User clicked on the Title - NewGame button
		//
	} else if (m_rNewGameButton.PtInRect(point)) {

		// start a new game
		if (pGameParams->bPlayingMetagame == false)
			PlayGame();

	} else if (m_pAnim->Clown(pDC, point) == true) {
		/************************************
		* user clicked animation.           *
		* animation handled in it's call.   *
		************************************/
		FlushInputEvents();
	} else if (m_pAnim->UFO(pDC, point) == true) {
		/************************************
		* user clicked animation.           *
		* animation handled in it's call.   *
		************************************/
		FlushInputEvents();
	} else if (m_pAnim->Brat(point) == true) {
		/************************************
		* user clicked animation.           *
		* animation handled in it's call.   *
		************************************/
		FlushInputEvents();
	} else if (m_pAnim->Lollipop(point) == true) {
		/************************************
		* user clicked animation.           *
		* animation handled in it's call.   *
		************************************/
		FlushInputEvents();
	} else if (m_pAnim->Haircut(point) == true) {
		/************************************
		* user clicked animation.           *
		* animation handled in it's call.   *
		************************************/
		FlushInputEvents();
	} else if (m_pBarb->m_bIsGameOver == false) {
		m_pBarb->OnLButtonDown(
		    (CWnd*) this,
		    m_pGamePalette,
		    point
		);

		// is this needed ?
		CFrameWnd::OnLButtonDown(nFlags, point);

		if (m_pBarb->m_bIsGameOver == true) {
			if (pGameParams->bPlayingMetagame) {
				Common::sprintf_s(buf, "Score: %d", m_pBarb->Score());
				CMessageBox cGameOver(
				    (CWnd*) this,
				    m_pGamePalette,
				    "Game over.",
				    buf
				);
				pGameParams->lScore = m_pBarb->Score();
				PostMessage(WM_CLOSE, 0, 0);
			} else if (m_pBarb->m_bIsWin) {
				Common::sprintf_s(buf, "You win! Score: %d", m_pBarb->Score());
				CMessageBox cGameOver(
				    (CWnd*) this,
				    m_pGamePalette,
				    "Game over.",
				    buf
				);
				pGameParams->lScore = m_pBarb->Score();
			} else {
				Common::sprintf_s(buf, "Score: %d", m_pBarb->Score());
				CMessageBox cGameOver(
				    (CWnd*) this,
				    m_pGamePalette,
				    "Game over.",
				    buf
				);
				pGameParams->lScore = m_pBarb->Score();
			} // end if
		} // end if
	} // end if

	ReleaseDC(pDC);
}


void CMainWindow::OnLButtonDblClk(unsigned int nFlags, CPoint point) {
	char    buf[32];
	CRect   tmpRect;

	if (m_pScrollSprite != nullptr)
		tmpRect = m_pScrollSprite->GetRect();

	// User clicked on the Menu button
	//
	if (tmpRect.PtInRect(point)) {

		// bring up the menu
		PostMessage(WM_COMMAND, IDC_MENU, BN_CLICKED);

		// User clicked on the Title - NewGame button
		//
	} else if (m_rNewGameButton.PtInRect(point)) {

		// start a new game
		if (pGameParams->bPlayingMetagame == false)
			PlayGame();

	} else if (m_pBarb->m_bIsGameOver == false) {
		m_pBarb->OnLButtonDblClk(
		    (CWnd*) this,
		    m_pGamePalette,
		    point
		);

		if (m_pBarb->m_bIsGameOver == true) {
			if (pGameParams->bPlayingMetagame) {
				Common::sprintf_s(buf, "Score: %d", m_pBarb->Score());
				CMessageBox cGameOver(
				    (CWnd*) this,
				    m_pGamePalette,
				    "Game Over.",
				    buf
				);
				pGameParams->lScore = m_pBarb->Score();
				PostMessage(WM_CLOSE, 0, 0);
			} else if (m_pBarb->m_bIsWin) {
				Common::sprintf_s(buf, "You win! Score: %d", m_pBarb->Score());
				CMessageBox cGameOver(
				    (CWnd*) this,
				    m_pGamePalette,
				    "Game Over.",
				    buf
				);
				pGameParams->lScore = m_pBarb->Score();
			} else {
				Common::sprintf_s(buf, "Score: %d", m_pBarb->Score());
				CMessageBox cGameOver(
				    (CWnd*) this,
				    m_pGamePalette,
				    "Game Over.",
				    buf
				);
				pGameParams->lScore = m_pBarb->Score();
			} // end if
		} // end if
	} // end if
}


void CMainWindow::OnLButtonUp(unsigned int nFlags, CPoint point) {
	char    buf[32];

	if (m_pBarb->m_bIsGameOver == false) {

		m_pBarb->OnLButtonUp((CWnd*) this);

		if (m_pBarb->m_bIsGameOver == true) {
			if (pGameParams->bPlayingMetagame) {
				Common::sprintf_s(buf, "Score: %d", m_pBarb->Score());
				CMessageBox cGameOver(
				    (CWnd*) this,
				    m_pGamePalette,
				    "Game Over.",
				    buf
				);
				pGameParams->lScore = m_pBarb->Score();
				PostMessage(WM_CLOSE, 0, 0);
			} else if (m_pBarb->m_bIsWin) {
				Common::sprintf_s(buf, "You win! Score: %d", m_pBarb->Score());
				CMessageBox cGameOver(
				    (CWnd*) this,
				    m_pGamePalette,
				    "Game Over.",
				    buf
				);
				pGameParams->lScore = m_pBarb->Score();
			} else {
				Common::sprintf_s(buf, "Score: %d", m_pBarb->Score());
				CMessageBox cGameOver(
				    (CWnd*) this,
				    m_pGamePalette,
				    "Game Over.",
				    buf
				);
				pGameParams->lScore = m_pBarb->Score();
			} // end if
		} // end if
	} // end if

	//nFlags  = 0x00;             // bull shit code to get rid of warning messages
	point   = CPoint(0, 0);
}

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
	// terminate app on ALT_Q
	//
	if ((nChar == 'q') && (nFlags & 0x2000)) {
		pGameParams->lScore = LOSE;                 // make sure the score is zero if they quit
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
		pGameParams->lScore = LOSE;                 // make sure the score is zero if they quit
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
		if (m_pBarb->m_pCrd != nullptr) {      // user holding a card?
			break;                          // yeap, can't allow this
		}

		GamePause();
		CSound::waitWaveSounds();
		CRules  RulesDlg(this, RULES_TXT, m_pGamePalette, pGameParams->bSoundEffectsEnabled ? RULES_WAV : nullptr);
		RulesDlg.DoModal();
		GameResume();
	}
	break;

	//
	// Bring up the options menu
	//
	case VK_F2:
		if (m_pBarb->m_pCrd != nullptr) {      // user holding a card?
			break;                          // yeap, can't allow this
		}

		SendMessage(WM_COMMAND, IDC_MENU, BN_CLICKED);
		break;

	default:
		CFrameWnd::OnKeyDown(nChar, nRepCnt, nFlags);
		break;
	}
}

void CMainWindow::FlushInputEvents() {
	MSG msg;

	while (true) {                                      // find and remove all keyboard events
		if (!PeekMessage(&msg, nullptr, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE))
			break;
	} // end while

	while (true) {                                      // find and remove all mouse events
		if (!PeekMessage(&msg, nullptr, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE))
			break;
	} // end while
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
}


LRESULT CMainWindow::OnMCINotify(WPARAM wParam, LPARAM lParam) {
	//CSound  *pSound;

	CSound::OnMCIStopped(wParam, lParam);
//      if (pSound != nullptr)
//          OnSoundNotify(pSound);
	return 0;
}

LRESULT CMainWindow::OnMMIONotify(WPARAM wParam, LPARAM lParam) {
	//CSound  *pSound;

	CSound::OnMMIOStopped(wParam, lParam);
	//if (pSound != nullptr)
	//  OnSoundNotify(pSound);
	return 0;
}

void CMainWindow::OnClose() {
	CDC     *pDC = GetDC();
	CRect   rctFillRect(0, 0, GAME_WIDTH, GAME_HEIGHT);
	CBrush  Brush(RGB(0, 0, 0));

	pDC->FillRect(&rctFillRect, &Brush);
	ReleaseDC(pDC);

	// perform cleanup
	//
	GameReset();

	if (m_pAnim != nullptr) {
		delete m_pAnim;
		m_pAnim = nullptr;
	}

	if (m_pMIDISound != nullptr || m_pWavSound != nullptr) {
		CSound::clearSounds();      // turn all sounds totally off and delete the objects
		m_pMIDISound = nullptr;
		m_pWavSound = nullptr;
	}

	if (m_pBarb != nullptr) {
		delete m_pBarb;
		m_pBarb = nullptr;
	}

	//
	// de-allocate any controls that we used
	//
	assert(m_pScrollSprite != nullptr);
	if (m_pScrollSprite != nullptr)
		delete m_pScrollSprite;

	//
	// need to de-allocate the game palette
	//
	assert(m_pGamePalette != nullptr);
	if (m_pGamePalette != nullptr) {
		//m_pGamePalette->DeleteObject();
		delete m_pGamePalette;
	}

	CFrameWnd::OnClose();

	MFC::PostMessage(ghParentWnd, WM_PARENTNOTIFY, WM_DESTROY, (LPARAM)pGameParams);
}

//
// CMainWindow message map:
// Associate messages with member functions.
//
BEGIN_MESSAGE_MAP(CMainWindow, CFrameWnd)
	ON_WM_PAINT()
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_WM_MOUSEMOVE()
	ON_WM_SYSCHAR()
	ON_WM_KEYDOWN()
	ON_WM_SYSKEYDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONUP()
	ON_MESSAGE(MM_MCINOTIFY, CMainWindow::OnMCINotify)
	ON_MESSAGE(MM_WOM_DONE, CMainWindow::OnMMIONotify)
END_MESSAGE_MAP()

void CALLBACK GetGameParams(CWnd *pParentWnd) {
	//
	// Our user preference dialog box is self contained in this object
	//
	CUserCfgDlg dlgUserCfg(pParentWnd, pGamePalette, IDD_USERCFG);
}

} // namespace Barbershop
} // namespace HodjNPodj
} // namespace Bagel
