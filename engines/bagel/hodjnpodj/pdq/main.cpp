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
#include "bagel/hodjnpodj/hnplibs/sprite.h"
#include "bagel/hodjnpodj/hnplibs/mainmenu.h"
#include "bagel/hodjnpodj/hnplibs/cmessbox.h"
#include "bagel/hodjnpodj/hnplibs/rules.h"
#include "bagel/hodjnpodj/hnplibs/gamedll.h"
#include "bagel/boflib/misc.h"
#include "bagel/hodjnpodj/pdq/main.h"
#include "bagel/hodjnpodj/pdq/game.h"
#include "bagel/hodjnpodj/pdq/guess.h"
#include "bagel/metaengine.h"

namespace Bagel {
namespace HodjNPodj {
namespace PDQ {

//
// This mini-game's main screen bitmap
//
#define MINI_GAME_MAP  ".\\ART\\CORRAL.BMP"

//
// Button ID constants
//
#define IDC_MENU    100

#define WAV_NARRATION   ".\\SOUND\\TGG.WAV"

STATIC const char *pszCategoryBitmaps[N_CATEGORIES] = {
	".\\ART\\PERSON.BMP",
	".\\ART\\PLACE.BMP",
	".\\ART\\PHRASE.BMP",
	".\\ART\\TITLE.BMP"
};


//
// Edit control ID constants
//
#define IDE_GUESS    101
#define IDE_SCORE    102
#define IDE_SCOREAVG 103

#define KEY_ENTER 0x000D
#define KEY_ESC   0x001B

/*
* Local prototypes
*/
void UpdateScore(unsigned int, unsigned int, unsigned int, unsigned int);

/*
* Globals
*/
CPalette     *pMyGamePalette;
CMainWindow  *gMain;
bool       bInGame;
CText        *txtScore, *txtTotalScore, *txtTitle;
LPGAMESTRUCT  pGameParams;

extern HWND ghParentWnd;

CMainWindow::CMainWindow() {
	CString  WndClass;
	CRect    tmpRect;
	CDC     *pDC;
	CDibDoc *pDibDoc;
	ERROR_CODE errCode;
	bool bSuccess;

	// the game structure must be valid
	assert(pGameParams != nullptr);

	// assume no error
	errCode = ERR_NONE;

	// Inits
	//
	pMyGamePalette = nullptr;
	m_pScrollSprite = nullptr;
	m_pSoundTrack = nullptr;
	m_bInGuess = false;
	m_bInMenu = false;
	m_iLastType = -1;
	gMain = this;
	pGameParams->lScore = 0;

	// Set the coordinates for the "Start New Game" button
	//
	m_rNewGameButton.SetRect(15, 4, 233, 20);

	// Define a special window class which traps double-clicks, is byte aligned
	// to maximize BITBLT performance, and creates "owned" DCs rather than sharing
	// the five system defined DCs which are not guaranteed to be available;
	// this adds a bit to our app size but avoids hangs/freezes/lockups.
	WndClass = AfxRegisterWndClass(CS_DBLCLKS | CS_BYTEALIGNWINDOW | CS_OWNDC, nullptr, nullptr, nullptr);

	// Acquire the shared palette for our game from the splash screen art
	if ((pDibDoc = new CDibDoc()) != nullptr) {
		if (pDibDoc->OpenDocument(MINI_GAME_MAP) != false)
			pMyGamePalette = pDibDoc->DetachPalette();
		else
			errCode = ERR_FFIND;
		delete pDibDoc;
	} else {
		errCode = ERR_MEMORY;
	}

	// Center our window on the screen
	//
	tmpRect.SetRect(0, 0, GAME_WIDTH, GAME_HEIGHT);
	#ifndef DEBUG
	if ((pDC = GetDC()) != nullptr) {
		tmpRect.left = (pDC->GetDeviceCaps(HORZRES) - GAME_WIDTH) >> 1;
		tmpRect.top = (pDC->GetDeviceCaps(VERTRES) - GAME_HEIGHT) >> 1;
		tmpRect.right = tmpRect.left + GAME_WIDTH;
		tmpRect.bottom = tmpRect.top + GAME_HEIGHT;
		ReleaseDC(pDC);
	}
	#endif

	// Create the window as a POPUP so no boarders, title, or menu are present;
	// this is because the game's background art will fill the entire 640x480 area.
	Create(WndClass, "Boffo Games -- ThGesngGme", WS_POPUP, tmpRect, nullptr, 0);

	BeginWaitCursor();
	ShowWindow(SW_SHOWNORMAL);
	PaintScreen();
	EndWaitCursor();

	if ((pDC = GetDC()) != nullptr) {
		//
		// build our main menu button
		//
		if ((m_pScrollSprite = new CSprite) != nullptr) {
			m_pScrollSprite->SharePalette(pMyGamePalette);
			bSuccess = m_pScrollSprite->LoadResourceSprite(pDC, IDB_SCROLBTN);
			assert(bSuccess);
			m_pScrollSprite->SetMasked(true);
			m_pScrollSprite->SetMobile(true);
		}

		//
		// set up the score controls
		//
		tmpRect.SetRect(401, 143, 585, 163);
		if ((txtTitle = new CText) != nullptr)
			txtTitle->SetupText(pDC, pMyGamePalette, &tmpRect, JUSTIFY_LEFT);

		tmpRect.SetRect(383, 162, 493, 187);
		if ((txtScore = new CText) != nullptr)
			txtScore->SetupText(pDC, pMyGamePalette, &tmpRect, JUSTIFY_CENTER);

		tmpRect.SetRect(515, 162, 555, 187);
		if ((txtTotalScore = new CText) != nullptr)
			txtTotalScore->SetupText(pDC, pMyGamePalette, &tmpRect, JUSTIFY_CENTER);

		ReleaseDC(pDC);
	} else {
		errCode = ERR_MEMORY;
	}

	BeginWaitCursor();

	if (pGameParams->bMusicEnabled) {
		if ((m_pSoundTrack = new CSound) != nullptr) {
			m_pSoundTrack->initialize(this, MID_SOUNDTRACK, SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END);
			m_pSoundTrack->midiLoopPlaySegment(1580, 32600, 0, FMT_MILLISEC);
		}
	} // end if m_pSoundTrack

	LoadCategoryNames();

	EndWaitCursor();

	SetFocus();

	// if we are not playing from the metagame
	//
	if (!pGameParams->bPlayingMetagame) {

		// Automatically bring up the main menu
		//
		PostMessage(WM_COMMAND, IDC_MENU, BN_CLICKED);

	} else {
		m_nTurnCount = 0;                               // Count up to three turns in meta-game mode
	}

	HandleError(errCode);
}

ERROR_CODE CMainWindow::LoadCategoryNames() {
	CDC *pDC;
	int i;
	ERROR_CODE errCode;

	// assume no error
	errCode = ERR_NONE;

	if ((pDC = GetDC()) != nullptr) {

		for (i = 0; i < N_CATEGORIES; i++) {

			if ((m_pCategories[i] = new CSprite) != nullptr) {
				if (m_pCategories[i]->LoadSprite(pDC, pszCategoryBitmaps[i]) != false) {

					m_pCategories[i]->SharePalette(pMyGamePalette);
					m_pCategories[i]->SetMasked(true);
					m_pCategories[i]->SetMobile(true);

				} else {
					errCode = ERR_UNKNOWN;
				}

			} else {
				errCode = ERR_MEMORY;
			}
		}

	} else {
		errCode = ERR_MEMORY;
	}

	return errCode;
}

void CMainWindow::ReleaseCategoryNames() {
	int i;

	for (i = N_CATEGORIES - 1; i >= 0; i--) {
		if (m_pCategories[i] != nullptr) {
			delete m_pCategories[i];
			m_pCategories[i] = nullptr;
		}
	}
}


void CMainWindow::PaintCategory(int iType) {
	CDC *pDC;

	assert((iType >= 0) && (iType < N_CATEGORIES));

	// save this ID for later (EraseCategory)
	m_iLastType = iType;

	m_pCategories[iType]->LinkSprite();

	pDC = GetDC();
	m_pCategories[iType]->PaintSprite(pDC, (GAME_WIDTH - m_pCategories[iType]->GetSize().cx) / 2, 76);
	ReleaseDC(pDC);
}


void CMainWindow::EraseCategory() {
	CDC *pDC;

	assert((m_iLastType >= 0) && (m_iLastType < N_CATEGORIES));

	pDC = GetDC();
	m_pCategories[m_iLastType]->EraseSprite(pDC);
	ReleaseDC(pDC);

	m_pCategories[m_iLastType]->UnlinkSprite();
	m_iLastType = -1;
}


void CMainWindow::HandleError(ERROR_CODE errCode) {
	//
	// Exit this application on fatal errors
	//
	if (errCode != ERR_NONE) {

		// pause the current game (if any)
		GamePauseTimer();

		// Display Error Message to the user
		MessageBox(errList[errCode], "Fatal Error!", MB_OK | MB_ICONSTOP);

		// Force this application to terminate
		PostMessage(WM_CLOSE, 0, 0);

		// Don't allow a repaint (remove all WM_PAINT messages)
		ValidateRect(nullptr);
	}
}


// OnPaint:
// This is called whenever Windows sends a WM_PAINT message.
// Note that creating a CPaintDC automatically does a BeginPaint and
// an EndPaint call is done when it is destroyed at the end of this
// function.  CPaintDC's constructor needs the window (this).
//
void CMainWindow::OnPaint() {
	PAINTSTRUCT lpPaint;

	Invalidate(false);
	BeginPaint(&lpPaint);
	PaintScreen();
	EndPaint(&lpPaint);
}


// Paint the background art and upadate any sprites
// called by OnPaint
void CMainWindow::PaintScreen() {
	CRect   rcDest;
	CRect   rcDIB;
	CDC     *pDC;
	CDibDoc myDoc;
	HDIB    hDIB;
	unsigned int nLeft, nTotal, nLeftAvg, nTotalAvg;
	bool bSuccess;

	myDoc.OpenDocument(MINI_GAME_MAP);
	hDIB = myDoc.GetHDIB();

	pDC = GetDC();
	if (pDC && hDIB) {
		GetClientRect(rcDest);

		rcDIB.top = rcDIB.left = 0;
		rcDIB.right = (int) DIBWidth(hDIB);
		rcDIB.bottom = (int) DIBHeight(hDIB);
		
		PaintDIB(pDC->m_hDC, &rcDest, hDIB, &rcDIB, pMyGamePalette);
	}

	if (!m_bInMenu) {
		if (m_pScrollSprite != nullptr) {
			bSuccess = m_pScrollSprite->PaintSprite(pDC, SCROLL_BUTTON_X, SCROLL_BUTTON_Y);
			assert(bSuccess);
		}
	}

	/* update the on-screen sprites */
	RepaintSpriteList(pDC);

	if (txtTitle != nullptr)
		txtTitle->DisplayString(pDC, "Score       Total Score", 21, FW_BOLD, RGB(0, 0, 0));

	GameGetScore(&nLeft, &nTotal, &nLeftAvg, &nTotalAvg);
	UpdateScore(nLeft, nTotal, nLeftAvg, nTotalAvg);

	ReleaseDC(pDC);
}


// OnCommand
// This function is called when a WM_COMMAND message is issued,
// typically in order to process control related activities.
//
bool CMainWindow::OnCommand(WPARAM wParam, LPARAM lParam) {
	CMainMenu COptionsWind((CWnd *)this,
	                       pMyGamePalette,
	                       (pGameParams->bPlayingMetagame ? (NO_NEWGAME | NO_OPTIONS) : 0) | (bInGame ? 0 : NO_RETURN),
	                       GetGameParams, "tggrules.txt", (pGameParams->bSoundEffectsEnabled ? WAV_NARRATION : nullptr), pGameParams);
	CDC *pDC;
	bool bSuccess;

	if (HIWORD(lParam) == BN_CLICKED) {
		switch (wParam) {

		/*
		* must bring up our menu of controls
		*/
		case IDC_MENU:

			GamePauseTimer();

			// indicate a state of being in the command menu
			m_bInMenu = true;

			pDC = GetDC();
			assert(m_pScrollSprite != nullptr);
			if (m_pScrollSprite != nullptr) {
				bSuccess = m_pScrollSprite->EraseSprite(pDC);
				assert(bSuccess);
			}
			//PaintScreen();

			CSound::waitWaveSounds();

			switch (COptionsWind.DoModal()) {

			case IDC_OPTIONS_NEWGAME:
				PlayGame();
				break;

			case IDC_OPTIONS_QUIT:
				pGameParams->lScore = 0;
				PostMessage(WM_CLOSE, 0, 0);
				break;

			default:
				break;
			}


			assert(m_pScrollSprite != nullptr);
			if (m_pScrollSprite != nullptr) {
				bSuccess = m_pScrollSprite->PaintSprite(pDC, SCROLL_BUTTON_X, SCROLL_BUTTON_Y);
				assert(bSuccess);
			}
			ReleaseDC(pDC);
			SetFocus();

			// not in command menu any more
			m_bInMenu = false;

			if (!pGameParams->bMusicEnabled && (m_pSoundTrack != nullptr)) {

				m_pSoundTrack->stop();
				delete m_pSoundTrack;
				m_pSoundTrack = nullptr;

			} else if (pGameParams->bMusicEnabled && (m_pSoundTrack == nullptr)) {

				if ((m_pSoundTrack = new CSound) != nullptr) {
					m_pSoundTrack->initialize(this, MID_SOUNDTRACK, SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END);
					m_pSoundTrack->midiLoopPlaySegment(1580, 32600, 0, FMT_MILLISEC);
				}
			}

			GameResumeTimer();

			return true;
		}
	}

	return false;
}

void CMainWindow::PlayGame() {
	CDC *pDC;
	unsigned int nLeft, nTotal, nLeftAvg, nTotalAvg;
	ERROR_CODE errCode;

	pDC = GetDC();

	bInGame = false;
	if ((errCode = InitGame(m_hWnd, pDC)) == ERR_NONE) {
		bInGame = true;
		if (pGameParams->bPlayingMetagame)
			m_nTurnCount++;

		/* erase current score */
		GameGetScore(&nLeft, &nTotal, &nLeftAvg, &nTotalAvg);
		UpdateScore(nLeft, nTotal, nLeftAvg, nTotalAvg);

		if ((errCode = StartGame(pDC)) != ERR_NONE) {

			/* cleanup on error */
			EndGame(pDC);
			bInGame = false;
		}
	}

	HandleError(errCode);

	ReleaseDC(pDC);
}

void CMainWindow::OnMouseMove(unsigned int, CPoint) {
	SetCursor(LoadCursor(nullptr, IDC_ARROW));
}

//
// Hooked when we get a WM_QUIT (or WM_CLOSE ?) message
//
void CMainWindow::OnClose() {
	CBrush myBrush;
	CRect myRect;
	CDC *pDC;

	ReleaseCategoryNames();

	// delete the game theme song
	//
	if (m_pSoundTrack != nullptr) {
		assert(pGameParams->bMusicEnabled);
		delete m_pSoundTrack;
		m_pSoundTrack = nullptr;
	}

	CSound::clearSounds();

	if ((pDC = GetDC()) != nullptr) {

		if (bInGame) {
			EndGame(pDC);
			bInGame = false;
		}
		ReleaseDC(pDC);
	}

	assert(txtTotalScore != nullptr);
	if (txtTotalScore != nullptr) {
		delete txtTotalScore;
		txtTotalScore = nullptr;
	}

	assert(txtScore != nullptr);
	if (txtScore != nullptr) {
		delete txtScore;
		txtScore = nullptr;
	}

	assert(txtTitle != nullptr);
	if (txtTitle != nullptr) {
		delete txtTitle;
		txtTitle = nullptr;
	}

	//
	// de-allocate any controls that we used
	//
	assert(m_pScrollSprite != nullptr);
	if (m_pScrollSprite != nullptr) {
		delete m_pScrollSprite;
		m_pScrollSprite = nullptr;
	}

	//
	// need to de-allocate the game palette
	//
	assert(pMyGamePalette != nullptr);
	if (pMyGamePalette != nullptr) {
		pMyGamePalette->DeleteObject();
		delete pMyGamePalette;
		pMyGamePalette = nullptr;
	}

	if ((pDC = GetDC()) != nullptr) {              // paint black

		myRect.SetRect(0, 0, GAME_WIDTH, GAME_HEIGHT);
		myBrush.CreateStockObject(BLACK_BRUSH);
		pDC->FillRect(&myRect, &myBrush);
		ReleaseDC(pDC);
	}

	CFrameWnd::OnClose();

	MFC::PostMessage(ghParentWnd, WM_PARENTNOTIFY, WM_DESTROY, 0L);
}

void CMainWindow::OnSysChar(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	// terminate app on ALT_Q
	//
	if ((nChar == 'q') && (nFlags & 0x2000)) {

		pGameParams->lScore = 0;
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
		pGameParams->lScore = 0;
		PostMessage(WM_CLOSE, 0, 0);
		break;

	default:
		CFrameWnd::OnSysKeyDown(nChar, nRepCnt, nFlags);
		break;
	}
}

void CMainWindow::OnKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	CDC *pDC;
	unsigned int nLeft, nTotal, nLeftAvg, nTotalAvg;

	// Handle keyboard input
	//
	switch (nChar) {

	//
	// Bring up the Rules
	//
	case VK_F1: {
		GamePauseTimer();
		CSound::waitWaveSounds();
		CRules  RulesDlg(this, "tggrules.txt", pMyGamePalette, (pGameParams->bSoundEffectsEnabled ? WAV_NARRATION : nullptr));
		RulesDlg.DoModal();
		SetFocus();

		GameResumeTimer();
	}
	break;

	//
	// Bring up the options menu
	//
	case VK_F2:
		SendMessage(WM_COMMAND, IDC_MENU, BN_CLICKED);
		break;

	default:

		if (bInGame && ((nChar == KEY_ENTER) || (toupper(nChar) >= 'A' && toupper(nChar) <= 'Z'))) {
			m_pDlgGuess = new CGuessDlg(this, pMyGamePalette);
			assert(m_pDlgGuess);

			GamePauseTimer();

			m_pDlgGuess->text = "";
			if (nChar != KEY_ENTER)
				m_pDlgGuess->text = CString((char)nChar);

			m_bInGuess = true;
			m_pDlgGuess->DoModal();
			m_bInGuess = false;
			SetFocus();

			// Check user's guess with the actual phrase
			if (CheckUserGuess(m_pDlgGuess->text)) {
				// User WON - now we need to shut off the timer
				GameStopTimer();

				WinGame();

				/*
				* Calculate score
				*/
				GameGetScore(&nLeft, &nTotal, &nLeftAvg, &nTotalAvg);
				UpdateScore(nLeft, nTotal, nLeftAvg, nTotalAvg);

				if (pGameParams->bSoundEffectsEnabled)
					sndPlaySound(WAV_YOUWIN, SND_ASYNC);

				CMessageBox dlgYouWin((CWnd *)this, pMyGamePalette, "You are correct!", "You have won.");
				SetFocus();

				pDC = gMain->GetDC();
				EndGame(pDC);
				gMain->ReleaseDC(pDC);
				bInGame = false;

				pGameParams->lScore += (100 * nLeft) / nTotal;

				// if in metagame then close dll when game is ended
				//
				if ((pGameParams->bPlayingMetagame) && (m_nTurnCount == MAX_TURNS)) {
					PostMessage(WM_CLOSE, 0, 0);
				} else {
					// start a new game instantly
					PlayGame();
				}
			} else {
				// User guessed incorrectly
				if (pGameParams->bSoundEffectsEnabled)
					sndPlaySound(WAV_BADGUESS, SND_ASYNC);
			}

			delete m_pDlgGuess;
			m_pDlgGuess = nullptr;
			GameResumeTimer();

		} else {
			CFrameWnd::OnKeyDown(nChar, nRepCnt, nFlags);
		}
		break;
	}
}


void CMainWindow::OnChar(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	CWnd::OnChar(nChar, nRepCnt, nFlags);
}

void UpdateScore(unsigned int nLeft, unsigned int nTotal, unsigned int nLeftAvg, unsigned int nTotalAvg) {
	char buf[40];
	CDC *pDC;

	if ((pDC = gMain->GetDC()) != nullptr) {

		if ((txtScore != nullptr) && (txtTotalScore != nullptr)) {

			/*
			* update the current score
			*/
			if (nTotal == 0)
				Common::sprintf_s(buf, "%d/%d = %d%%", nLeft, nTotal, 0);
			else
				Common::sprintf_s(buf, "%d/%d = %d%%", nLeft, nTotal, (100 * nLeft) / nTotal);

			txtScore->DisplayString(pDC, buf, 21, FW_BOLD, RGB(0, 0, 0));

			/*
			* update the cumulative score
			*/
			if (nTotalAvg == 0)
				Common::sprintf_s(buf, "0%%");
			else
				Common::sprintf_s(buf, "%d%%", (100 * nLeftAvg) / nTotalAvg);
			txtTotalScore->DisplayString(pDC, buf, 21, FW_BOLD, RGB(0, 0, 0));
		}
		gMain->ReleaseDC(pDC);
	}
}

void CMainWindow::OnLButtonDown(unsigned int nFlags, CPoint point) {
	CDC     *pDC;
	CSprite *pSprite = nullptr;
	CSound  *pEffect = nullptr;
	CRect   tmpRect,
	        birdRect,
	        horse1Rect,
	        horse2Rect,
	        flowerRect;
	bool    bSuccess;
	int     i;

	birdRect.SetRect(BIRD_X, BIRD_Y, BIRD_X + BIRD_DX, BIRD_Y + BIRD_DY);
	horse1Rect.SetRect(HORSE1_X, HORSE1_Y, HORSE1_X + HORSE1_DX, HORSE1_Y + HORSE1_DY);
	horse2Rect.SetRect(HORSE2_X, HORSE2_Y, HORSE2_X + HORSE2_DX, HORSE2_Y + HORSE2_DY);
	flowerRect.SetRect(FLOWER_X, FLOWER_Y, FLOWER_X + FLOWER_DX, FLOWER_Y + FLOWER_DY);

	tmpRect = m_pScrollSprite->GetRect();

	CSound::waitWaveSounds();

	if (tmpRect.PtInRect(point)) {

		PostMessage(WM_COMMAND, IDC_MENU, BN_CLICKED);

		// User clicked on the Title - NewGame button
		//
	} else if (m_rNewGameButton.PtInRect(point)) {

		// if we are not playing from the metagame
		//
		if (!pGameParams->bPlayingMetagame) {

			// start a new game
			PlayGame();
		}

	} else if (birdRect.PtInRect(point)) {
		pDC = GetDC();
		pSprite = new CSprite;
		(*pSprite).SharePalette(pMyGamePalette);
		bSuccess = (*pSprite).LoadCels(pDC, BIRD_ANIM, NUM_BIRD_CELS);
		if (!bSuccess) {
			delete pSprite;
			ReleaseDC(pDC);
			return;
		}
		(*pSprite).SetMasked(false);
		(*pSprite).SetMobile(false);

		if ((*pGameParams).bSoundEffectsEnabled) {
			if ((pEffect = new CSound((CWnd *)this, WAV_BIRD, SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE | SOUND_QUEUE)) != nullptr) {
				pEffect->play();
			}
		}
		(*pSprite).SetCel(NUM_BIRD_CELS);
		for (i = 0; i < NUM_BIRD_CELS; i++) {
			(*pSprite).PaintSprite(pDC, BIRD_X, BIRD_Y);
			CSound::handleMessages();
			Sleep(BIRD_SLEEP);
		}

		delete pSprite;
		ReleaseDC(pDC);

	} else if (horse1Rect.PtInRect(point)) {
		pDC = GetDC();
		pSprite = new CSprite;
		(*pSprite).SharePalette(pMyGamePalette);
		bSuccess = (*pSprite).LoadCels(pDC, HORSE1_ANIM, NUM_HORSE1_CELS);
		if (!bSuccess) {
			delete pSprite;
			ReleaseDC(pDC);
			return;
		}
		(*pSprite).SetMasked(false);
		(*pSprite).SetMobile(false);

		if ((*pGameParams).bSoundEffectsEnabled) {
			if ((pEffect = new CSound((CWnd *)this, WAV_HORSE1, SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE | SOUND_QUEUE)) != nullptr) {
				pEffect->play();
			}
		}
		(*pSprite).SetCel(NUM_HORSE1_CELS);
		for (i = 0; i < NUM_HORSE1_CELS; i++) {
			(*pSprite).PaintSprite(pDC, HORSE1_X, HORSE1_Y);
			CSound::handleMessages();
			Sleep(HORSE1_SLEEP);
		}
		if (pSprite != nullptr)
			delete pSprite;

		ReleaseDC(pDC);

	} else if (horse2Rect.PtInRect(point)) {
		pDC = GetDC();
		pSprite = new CSprite;
		(*pSprite).SharePalette(pMyGamePalette);
		bSuccess = (*pSprite).LoadCels(pDC, HORSE2_ANIM, NUM_HORSE2_CELS);
		if (!bSuccess) {
			delete pSprite;
			ReleaseDC(pDC);
			return;
		}
		(*pSprite).SetMasked(false);
		(*pSprite).SetMobile(false);

		if ((*pGameParams).bSoundEffectsEnabled) {
			if ((pEffect = new CSound((CWnd *)this, WAV_HORSE2, SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE | SOUND_QUEUE)) != nullptr) {
				pEffect->play();
			}
		}
		(*pSprite).SetCel(NUM_HORSE2_CELS);
		for (i = 0; i < NUM_HORSE2_CELS; i++) {
			(*pSprite).PaintSprite(pDC, HORSE2_X, HORSE2_Y);
			CSound::handleMessages();
			Sleep(HORSE2_SLEEP);
		}
		if (pSprite != nullptr)
			delete pSprite;

		ReleaseDC(pDC);

	} else if (flowerRect.PtInRect(point)) {
		pDC = GetDC();
		pSprite = new CSprite;
		(*pSprite).SharePalette(pMyGamePalette);
		bSuccess = (*pSprite).LoadCels(pDC, FLOWER_ANIM, NUM_FLOWER_CELS);
		if (!bSuccess) {
			delete pSprite;
			ReleaseDC(pDC);
			return;
		}
		(*pSprite).SetMasked(false);
		(*pSprite).SetMobile(false);

		if ((*pGameParams).bSoundEffectsEnabled) {
			if ((pEffect = new CSound((CWnd *)this, WAV_FLOWER, SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE | SOUND_QUEUE)) != nullptr) {
				pEffect->play();
			}
		}
		(*pSprite).SetCel(NUM_FLOWER_CELS);
		for (i = 0; i < NUM_FLOWER_CELS; i++) {
			(*pSprite).PaintSprite(pDC, FLOWER_X, FLOWER_Y);
			CSound::handleMessages();
			Sleep(FLOWER_SLEEP);
		}
		if (pSprite != nullptr)
			delete pSprite;

		ReleaseDC(pDC);

	} else {

		CFrameWnd::OnLButtonDown(nFlags, point);
	}
}

void CMainWindow::OnSetFocus(CWnd *) {
	if (m_bInGuess)
		m_pDlgGuess->SetFocus();
	else {
		SetFocus();
	}
}

void CMainWindow::OnActivate(unsigned int nState, CWnd *, bool bMinimized) {
	if (!bMinimized) {
		switch (nState) {
		case WA_ACTIVE:
		case WA_CLICKACTIVE:
			// Restrict the keybinding to minimal
			BagelMetaEngine::setKeybindingMode(KBMODE_MINIMAL);
			break;

		case WA_INACTIVE:
			BagelMetaEngine::setKeybindingMode(KBMODE_NORMAL);
			break;

		default:
			break;
		}
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

void CMainWindow::OnSoundNotify(CSound *) {
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
	ON_WM_PAINT()
	ON_WM_CHAR()
	ON_WM_SYSCHAR()
	ON_WM_KEYDOWN()
	ON_WM_SYSKEYDOWN()
	ON_WM_CLOSE()
	ON_WM_MOUSEMOVE()
	ON_WM_SETFOCUS()
	ON_WM_LBUTTONDOWN()
	ON_WM_ACTIVATE()
	ON_MESSAGE(MM_MCINOTIFY, CMainWindow::OnMCINotify)
	ON_MESSAGE(MM_WOM_DONE, CMainWindow::OnMMIONotify)
END_MESSAGE_MAP()

} // namespace PDQ
} // namespace HodjNPodj
} // namespace Bagel
