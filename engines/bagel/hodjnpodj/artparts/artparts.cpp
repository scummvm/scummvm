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

#include "bagel/boflib/sound.h"
#include "bagel/boflib/misc.h"
#include "bagel/hodjnpodj/hnplibs/gamedll.h"
#include "bagel/hodjnpodj/artparts/resource.h"
#include "bagel/hodjnpodj/artparts/globals.h"
#include "bagel/hodjnpodj/hnplibs/mainmenu.h"
#include "bagel/hodjnpodj/hnplibs/cmessbox.h"
#include "bagel/hodjnpodj/artparts/artparts.h"
#include "bagel/hodjnpodj/hnplibs/rules.h"
#include "bagel/hodjnpodj/hnplibs/bitmaps.h"
#include "bagel/hodjnpodj/artparts/optndlg.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {
namespace ArtParts {

bool    InArtRegion(CPoint point);
CPoint  WinToArt(CPoint point);

extern  HWND ghParentWnd;
extern  LPGAMESTRUCT pGameInfo;

CPalette *CMainWindow::pGamePalette;
int CMainWindow::nSeconds;
int CMainWindow::nMinutes;
int CMainWindow::nLastPick;
int CMainWindow::m_nTime;
int CMainWindow::m_nRows;
int CMainWindow::m_nColumns;
int CMainWindow::m_nWidth;
int CMainWindow::m_nHeight;
float CMainWindow::m_nScore;
bool CMainWindow::bFramed;
int CMainWindow::tempTime;
int CMainWindow::tempRows;
int CMainWindow::tempColumns;
bool CMainWindow::tempFramed;

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CMainWindow constructor:
// Create the window with the appropriate style, size, menu, etc.;
// it will be later revealed by CTheApp::InitInstance().  Then
// create our splash screen object by opening and loading its DIB.
//
CMainWindow::CMainWindow() {
	CString WndClass;
	CRect   MainRect, tmpRect;
	CDC     *pDC = nullptr;

	initStatics();
	BeginWaitCursor();

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
	Create(WndClass, "Boffo Games -- Art Parts", WS_POPUP, MainRect, nullptr, 0);

	pScratch1 = new CBitmap();          // Scratch1 is the source for copying
	pScratch1DC = new CDC();
	pScratch2 = new CBitmap();          // Scratch2 is the destination for copying
	pScratch2DC = new CDC();

	pScratch1->CreateCompatibleBitmap(pDC, ART_WIDTH, ART_HEIGHT);   // Set up Scratch1
	pScratch1DC->CreateCompatibleDC(pDC);                               //...bitmap and DC
	pOldBmp1 = pScratch1DC->SelectObject(pScratch1);

	pScratch2->CreateCompatibleBitmap(pDC, ART_WIDTH, ART_HEIGHT);      // Set up Scratch2
	pScratch2DC->CreateCompatibleDC(pDC);                               //...bitmap and DC
	pOldBmp2 = pScratch2DC->SelectObject(pScratch2);

	//srand((unsigned) time(nullptr));         // seed the random number generator

	InitValues();       // Set the default values of global variables
	m_bPlaying = false;

	if (LoadArtWork() == false) {            // Load first artwork & display
		PostMessage(WM_CLOSE, 0, 0);
		return;
	}

	// Build Scroll Command button
	m_pScrollButton = new CBmpButton;
	ASSERT(m_pScrollButton != nullptr);
	tmpRect.SetRect(SCROLL_BUTTON_X, SCROLL_BUTTON_Y,
	                SCROLL_BUTTON_X + SCROLL_BUTTON_DX - 1,
	                SCROLL_BUTTON_Y + SCROLL_BUTTON_DY - 1);
	bSuccess = (*m_pScrollButton).Create(nullptr, BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, tmpRect, this, IDC_SCROLL);
	ASSERT(bSuccess);
	bSuccess = (*m_pScrollButton).LoadBitmaps(SCROLLUP, SCROLLDOWN, SCROLLUP, SCROLLUP);
	ASSERT(bSuccess);
	m_bIgnoreScrollClick = false;

	// Put up something in the meantime
	//
	ShowWindow(SW_SHOWNORMAL);
	SplashScreen();

	pLocaleBitmap = FetchResourceBitmap(pDC, nullptr, "IDB_LOCALE_BMP");
	ASSERT(pLocaleBitmap != nullptr);
	pBlankBitmap = FetchResourceBitmap(pDC, nullptr, "IDB_BLANK_BMP");
	ASSERT(pBlankBitmap != nullptr);

	tmpRect.SetRect(TIME_LOCATION_X, TIME_LOCATION_Y,
	                TIME_LOCATION_X + TIME_WIDTH, TIME_LOCATION_Y + TIME_HEIGHT);
	if ((m_pTimeText = new CText()) != nullptr) {
		(*m_pTimeText).SetupText(pDC, pGamePalette, &tmpRect, JUSTIFY_CENTER);
	}

	ReleaseDC(pDC);

	MSG lpmsg;
	while (PeekMessage(&lpmsg, nullptr, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE | PM_NOYIELD)) ;

	EndWaitCursor();

	if (pGameInfo->bPlayingMetagame) {
		m_bPlaying = true;
		if (pGameInfo->bMusicEnabled) {
			pGameSound = new CSound(this, GAME_THEME, SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END);
			if (pGameSound != nullptr) {
				(*pGameSound).midiLoopPlaySegment(2300, 32000, 0, FMT_MILLISEC);
			} // end if pGameSound
		}
	} else {
		if (pGameInfo->bMusicEnabled) {
			pGameSound = new CSound(this, GAME_THEME, SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END);
			if (pGameSound != nullptr) {
				(*pGameSound).midiLoopPlaySegment(2300, 32000, 0, FMT_MILLISEC);
			} // end if pGameSound
		}
		PostMessage(WM_COMMAND, IDC_SCROLL, BN_CLICKED);         // Activate the Options dialog
	}

	bStartOkay = true;

} //End of CMainWindow

void CMainWindow::initStatics() {
	pGamePalette = nullptr;
	nSeconds = MIN_TIME;
	nMinutes = 0;
	nLastPick = 0;
	m_nTime = MIN_TIME;
	m_nRows = MIN_ROWS;
	m_nColumns = MIN_COLUMNS;
	m_nWidth = ART_WIDTH / MIN_COLUMNS;
	m_nHeight = ART_HEIGHT / MIN_ROWS;
	m_nScore = 0.0;

	bFramed = false;
	tempTime = MIN_TIME;
	tempRows = MIN_ROWS;
	tempColumns = MIN_COLUMNS;
	tempFramed = true;
}

// OnPaint:
// This is called whenever Windows sends a WM_PAINT message.
// Note that creating a CPaintDC automatically does a BeginPaint and
// an EndPaint call is done when it is destroyed at the end of this
// function.  CPaintDC's constructor needs the window (this).
//
void CMainWindow::OnPaint() {
	CDC         *pDC;                                                               // Screen device cont.
	CPalette    *pPalOld = nullptr;                                                    // Old palette holder
	PAINTSTRUCT lpPaint;
	char        msg[64];

	pDC = GetDC();                                                                  // Get screen DC
	pPalOld = (*pDC).SelectPalette(pGamePalette, false);                            // Select Game Palette
	(*pDC).RealizePalette();                                                        // Use it

	InvalidateRect(nullptr, false);            // invalidate the entire window
	BeginPaint(&lpPaint);
	SplashScreen();

	if (m_bPlaying) {            // only false when the options are displayed
		SplashScratchPaint();
		PaintBitmap(pDC, pGamePalette, pBlankBitmap, TIME_LOCATION_X, TIME_LOCATION_Y);
		if (m_nTime == 0)
			Common::sprintf_s(msg, "Time Used: %02d:%02d", nMinutes, nSeconds);
		else {
			Common::sprintf_s(msg, "Time Left: %02d:%02d", nMinutes, nSeconds);
		}
		(*m_pTimeText).DisplayString(pDC, msg, 16, FW_SEMIBOLD, OPTIONS_COLOR);
	} else {
		PaintBitmap(pDC, pGamePalette, pLocaleBitmap, TIME_LOCATION_X, TIME_LOCATION_Y);
	}

	EndPaint(&lpPaint);
	(*pDC).SelectPalette(pPalOld, false);                                            // Select back old palette
	ReleaseDC(pDC);                                                                 // Release the DC

	if (bStartOkay && (bGameStarted == false)) {
		MSG lpmsg;
		while (PeekMessage(&lpmsg, nullptr, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE | PM_NOYIELD)) ;
		bGameStarted = true;
		bStartOkay = false;
	}
}

/*****************************************************************
 *
 *  SplashScratch()
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Paints the scratch bitmap on the screen
 *
 *  FORMAL PARAMETERS:
 *
 *      none
 *
 *  IMPLICIT INPUT PARAMETERS:
 *
 *      gets Screen DC, uses pScratchDC
 *
 *  IMPLICIT OUTPUT PARAMETERS:
 *
 *      Screen DC
 *
 *  RETURN VALUE:
 *
 *      void
 *
 ****************************************************************/
void CMainWindow::SplashScratch() {
	CDC         *pDC;                                                               // Screen device cont.
	CPalette    *pPalOld = nullptr;                                                    // Old palette holder

	pDC = GetDC();                                                                  // Get screen DC
	pPalOld = (*pDC).SelectPalette(pGamePalette, false);                            // Select Game Palette
	(*pDC).RealizePalette();                                                        // Use it

	pDC->BitBlt(SIDE_BORDER, TOP_BORDER, ART_WIDTH, ART_HEIGHT, pScratch2DC, 0, 0, SRCCOPY);     // Draw Scratch2

	(*pDC).SelectPalette(pPalOld, false);                                                // Select back old palette
	ReleaseDC(pDC);                                                                     // Release the DC

	pScratch1DC->BitBlt(0, 0, ART_WIDTH, ART_HEIGHT, pScratch2DC, 0, 0, SRCCOPY);        // Copy New parts locations
	//...to the Scratch1 bitmap
}

void CMainWindow::SplashScratchPaint() {
	CRect       rcDest;
	CRect       rcDIB;
	CDC         *pDC;
	CDibDoc     *pSourceDoc;
	HDIB        hDIB;
	CPalette    *pOldPalSource = nullptr,
	             *pOldPalScreen = nullptr;
	CDC         *pSourceDC;
	CBitmap     *pSource = nullptr,
	             *pOldBmp = nullptr;

	pDC = GetDC();
	pOldPalScreen = (*pDC).SelectPalette(pGamePalette, false);                            // Select Game Palette
	(*pDC).RealizePalette();                                                        // Use it

	if (bFramed) {
		pSourceDC = new CDC();
		pSource = new CBitmap();          // Source is the original, in-tact artwork for framed mode

		pSource->CreateCompatibleBitmap(pDC,    ART_WIDTH + (2 * FRAME_WIDTH),
		                                ART_HEIGHT + (2 * FRAME_HEIGHT));   // Set up Source
		pSourceDC->CreateCompatibleDC(pDC);                                          //...bitmap and DC
		pOldBmp = pSourceDC->SelectObject(pSource);

		pSourceDoc = new CDibDoc();
		ASSERT(pSourceDoc != nullptr);
		if (pSourceDoc == nullptr) {
			return;
		}

		(*pSourceDoc).OpenDocument(szCurrentArt);

		pOldPalSource = pSourceDC->SelectPalette(pGamePalette, false);
		pSourceDC->RealizePalette();

		hDIB = (*pSourceDoc).GetHDIB();

		if (hDIB) {
			rcDest.SetRect(0, 0, ART_WIDTH + (2 * FRAME_WIDTH), ART_HEIGHT + (2 * FRAME_HEIGHT));

			int cxDIB = (int) DIBWidth(hDIB);
			int cyDIB = (int) DIBHeight(hDIB);

			if (cxDIB > ART_WIDTH) {                     // Center and crop
				rcDIB.left = (cxDIB - ART_WIDTH) / 2;   //...too wide art
				rcDIB.right = rcDIB.left + ART_WIDTH;
			} else {                                    // Stretch too thin art
				rcDIB.left = 0;
				rcDIB.right = cxDIB;
			}

			if (cyDIB > ART_HEIGHT) {                    // Center and crop
				rcDIB.top = (cyDIB - ART_HEIGHT) / 2;   //...too long art
				rcDIB.bottom = rcDIB.top + ART_HEIGHT;
			} else {                                    // Stretch too short art
				rcDIB.top = 0;
				rcDIB.bottom = cyDIB;
			}
			PaintDIB((*pSourceDC).m_hDC, &rcDest, hDIB, &rcDIB, pGamePalette);
			pSourceDC->BitBlt(FRAME_WIDTH, FRAME_HEIGHT, ART_WIDTH, ART_HEIGHT,      // Copy the Scrambled art to
			                  pScratch2DC, 0, 0, SRCCOPY);                            //... the Frame + Art bitmap
			pDC->BitBlt(SIDE_BORDER - FRAME_WIDTH, TOP_BORDER - FRAME_HEIGHT,            // Copy the Frame + Scrambled
			            ART_WIDTH + (2 * FRAME_WIDTH), ART_HEIGHT + (2 * FRAME_HEIGHT), //...art to the screen
			            pSourceDC, 0, 0, SRCCOPY);
		}

		delete pSourceDoc;
		pSourceDoc = nullptr;

		if (pOldBmp != nullptr)                        // Get rid of Source
			pSourceDC->SelectObject(pOldBmp);
		if (pOldPalSource != nullptr)
			pSourceDC->SelectPalette(pOldPalSource, false);
		if (pSourceDC->m_hDC != nullptr) {
			pSourceDC->DeleteDC();
			delete pSourceDC;
		}

		pSource->DeleteObject();
		delete pSource;
	} else {
		pDC->BitBlt(SIDE_BORDER, TOP_BORDER, ART_WIDTH, ART_HEIGHT, pScratch2DC, 0, 0, SRCCOPY);     // Draw Scratch2

	}                                                                                               //...on screen

	(*pDC).SelectPalette(pOldPalScreen, false);                                                  // Select back old palette
	ReleaseDC(pDC);

	pScratch1DC->BitBlt(0, 0, ART_WIDTH, ART_HEIGHT, pScratch2DC, 0, 0, SRCCOPY);        // Copy New parts locations
}

// Paint the background art (splash screen) in the client area;
// called by both OnPaint and InitInstance.
void CMainWindow::SplashScreen() {
	CRect   rcDest;
	CRect   rcDIB;
	CDC     *pDC;
	CDibDoc myDoc;
	HDIB    hDIB;

	pDC = GetDC();

	myDoc.OpenDocument(TEXTSCREEN);

	hDIB = myDoc.GetHDIB();

	if (pDC && hDIB) {
		GetClientRect(rcDest);
		int cxDIB = (int)DIBWidth(hDIB);
		int cyDIB = (int)DIBHeight(hDIB);

		rcDIB.top = rcDIB.left = 0;
		rcDIB.right = cxDIB;
		rcDIB.bottom = cyDIB;
		PaintDIB((*pDC).m_hDC, &rcDest, hDIB, &rcDIB, pGamePalette);
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
	CPalette    *pPalOld = nullptr;
	CBrush      *pBrushOld = nullptr,
	             *pBrushNew = nullptr;
	CDC *pDC;

	if ((HIWORD(lParam) == BN_CLICKED) && !m_bShowOutOfPlace) {

		pDC = GetDC();

		CRules  RulesDlg((CWnd *)this, "artparts.txt", pGamePalette, pGameInfo->bSoundEffectsEnabled ? RULES_WAV : nullptr);                // Construct Rules dialog
		CMainMenu COptionsWind((CWnd *)this, pGamePalette,
		                       pGameInfo->bPlayingMetagame ? (NO_NEWGAME | NO_OPTIONS) : 0,
		                       GetSubOptions, "artparts.txt", pGameInfo->bSoundEffectsEnabled ? RULES_WAV : nullptr, pGameInfo) ;         // Construct Option dialog
		PaintBitmap(pDC, pGamePalette, pLocaleBitmap, TIME_LOCATION_X, TIME_LOCATION_Y);

		switch (wParam) {

		case IDC_RULES:
			CSound::waitWaveSounds();

			m_bIgnoreScrollClick = true;
			(*m_pScrollButton).SendMessage(BM_SETSTATE, true, 0L);

			RulesDlg.DoModal();
			m_bIgnoreScrollClick = false;
			(*m_pScrollButton).SendMessage(BM_SETSTATE, false, 0L);
			break;

		case IDC_SCROLL:

			KillTimer(DISPLAY_TIMER);                                // Stop the Displayed Time timer
			if (m_bIgnoreScrollClick) {
				(*m_pScrollButton).SendMessage(BM_SETSTATE, true, 0L);
				break;
			}

			m_bIgnoreScrollClick = true;
			(*m_pScrollButton).SendMessage(BM_SETSTATE, true, 0L);
			SendDlgItemMessage(IDC_SCROLL, BM_SETSTATE, true, 0L);
			m_bPlaying = false;                                     // Not playing the game
			bSwitched = false;                                      // Prevent ability to Undo after Command is done

			CheckForWin();                                          // Get current score

			if (bGameStarted) {
				pPalOld = (*pDC).SelectPalette(pGamePalette, false);    // Select in the artwork's palette
				(*pDC).RealizePalette();                                // Use it

				pBrushNew = new CBrush();                               // Construct a new brush object
				if (pBrushNew != nullptr) {                                 // If the constructor was successful:
					pBrushNew->CreateSolidBrush(PALETTERGB(128, 0, 0));           // Create my backdrop color brush
					pBrushOld = (*pDC).SelectObject(pBrushNew);                  // Select into the DC my new brush
					pDC->SetROP2(R2_COPYPEN);                                    // Set Draw mode to use the pen color
					if (bFramed)
						(*pDC).Rectangle(SIDE_BORDER - FRAME_WIDTH, TOP_BORDER - FRAME_HEIGHT,
						                 GAME_WIDTH - (SIDE_BORDER - FRAME_WIDTH),
						                 GAME_HEIGHT - (BOTTOM_BORDER - FRAME_HEIGHT));
					else
						(*pDC).Rectangle(SIDE_BORDER, TOP_BORDER, GAME_WIDTH - SIDE_BORDER, GAME_HEIGHT - BOTTOM_BORDER);
					(*pDC).SelectObject(pBrushOld);                          // Select in the old brush
					(*pDC).SelectPalette(pPalOld, false);                        // Select in the old palette
					delete pBrushNew;                                           // Delete the new brush
				}
			} // end if bGameStarted

			CSound::clearWaveSounds();

			switch (COptionsWind.DoModal()) {

			case IDC_OPTIONS_NEWGAME:                           // Selected New Game
				(*m_pScrollButton).SendMessage(BM_SETSTATE, false, 0L);
				m_bIgnoreScrollClick = false;
				if (!pGameInfo->bPlayingMetagame)
					NewGame();
				//if m_nScore == 100, randomly put it in order, so start over
				// m_bNewGame = false; PostMessage( IDC_OPTIONS_NEWGAME ...);break;
				break;

			case IDC_OPTIONS_RETURN:
				(*m_pScrollButton).SendMessage(BM_SETSTATE, false, 0L);
				m_bIgnoreScrollClick = false;
				m_bPlaying = true;
				if (bGameStarted && (m_nTime != (nSeconds + (nMinutes * 60)))) {     // have started
					SetTimer(DISPLAY_TIMER, CLICK_TIME, nullptr);                   // Set timer
				}
				break;

			case IDC_OPTIONS_QUIT:                      // Quit button was clicked
				PostMessage(WM_CLOSE, 0, 0);         // and post a program exit
				ReleaseDC(pDC);
				return false;

			} //end switch(ComDlg.DoModal())

			//
			// Check to see if the music state was changed and adjust to match it
			//
			if ((pGameInfo->bMusicEnabled == false) && (pGameSound != nullptr)) {
				if (pGameSound->playing())
					pGameSound->stop();
			} else if (pGameInfo->bMusicEnabled) {
				if (pGameSound == nullptr) {
					pGameSound = new CSound(this, GAME_THEME, SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END);
				}
				if (pGameSound != nullptr) {
					if (!pGameSound->playing())
						(*pGameSound).midiLoopPlaySegment(2300, 32000, 0, FMT_MILLISEC);
				} // end if pGameSound
			}

			(*pDC).SelectPalette(pPalOld, false);
			InvalidateRect(nullptr, false);            // force a redraw of the entire window
			//...and stop any other WM_PAINT messages
		} //end switch(wParam)

		ReleaseDC(pDC);
	} // end if
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
	CRect   rectTitle;
	CSound  *pEffect = nullptr;

	rectTitle.SetRect(NEWGAME_LOCATION_X, NEWGAME_LOCATION_Y,
	                  NEWGAME_LOCATION_X + NEWGAME_WIDTH,
	                  NEWGAME_LOCATION_Y + NEWGAME_HEIGHT);

	if (rectTitle.PtInRect(point) && ((pGameInfo->bPlayingMetagame == false) && !m_bShowOutOfPlace)) {
		NewGame();
	} else if (bGameStarted && (m_bNewGame && !m_bShowOutOfPlace)) {
		CDC     *pDC;
		CPoint Hit;

		pDC = GetDC();

		if (!m_bFirst && InArtRegion(point)) {                   // Selecting First area

			if (m_nTime == (nSeconds + (nMinutes * 60))) {      // Very first click of the game...
				SetTimer(DISPLAY_TIMER, CLICK_TIME, nullptr);   // If it is timed. Set timer
			}

			Hit = WinToArt(point);
			First.x = Hit.x - (Hit.x % m_nWidth);
			First.y = Hit.y - (Hit.y % m_nHeight);
			Second.x = First.x + m_nWidth;
			Second.y = First.y + m_nHeight;
			BaseRect.SetRect(First.x, First.y, Second.x, Second.y); // Save the starting part to
			//...check sizing changes
			OldRect.SetRect(First.x + SIDE_BORDER, First.y + TOP_BORDER,
			                Second.x + SIDE_BORDER, Second.y + TOP_BORDER);
			MyFocusRect(pDC, OldRect, R2_NOT);
			m_bFirst = true;
			bSwitched = false;
			SetCapture();                               // Hog all the mouse events
		} // end if
		else if (m_bFirst) {                            // Selecting Second area
			ReleaseCapture();                       // Let other windows get mouse events
			DstRect.SetRect(OldRect.TopLeft().x - SIDE_BORDER,       // in case Point is outside
			                OldRect.TopLeft().y - TOP_BORDER,       //...of art area
			                OldRect.BottomRight().x - SIDE_BORDER,  // Use OldRect for
			                OldRect.BottomRight().y - TOP_BORDER);  //...copy-to area

			MyFocusRect(pDC, OldRect, R2_NOT);           // erase old rect
			SwitchAreas(SrcRect, DstRect);               // Switch the two areas
			m_bFirst = false;
			bSwitched = true;

			if (m_nScore == 100) {                   // If the Picture is all correct:
				KillTimer(DISPLAY_TIMER);            // Stop the Display timer
				if (pGameInfo->bSoundEffectsEnabled) {
					pEffect = new CSound((CWnd *)this, WIN_SOUND,
					                     SOUND_WAVE | SOUND_ASYNCH |
					                     SOUND_QUEUE | SOUND_AUTODELETE);    //...Wave file, to delete itself
					(*pEffect).play();                                                      //...play the narration
				}
				MSG lpmsg;
				while (PeekMessage(&lpmsg, nullptr, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE | PM_NOYIELD)) ;
				CMessageBox GameOverDlg((CWnd *)this, pGamePalette, "Game over.", "It's complete!");
				m_bNewGame = false;                 // Game over, but can still look
				//..at the art (Playing is true
				bGameStarted = false;
				if (pGameInfo->bPlayingMetagame) {
					pGameInfo->lScore = 1;
					PostMessage(WM_CLOSE, 0, 0);         // and post a program exit
				}
			} // end if m_nScore
			else {
				if (pGameInfo->bSoundEffectsEnabled) {
					pEffect = new CSound((CWnd *)this, SWITCH_SOUND,
					                     SOUND_WAVE | SOUND_ASYNCH |
					                     SOUND_QUEUE | SOUND_AUTODELETE);    //...Wave file, to delete itself
					(*pEffect).play();                                                      //...play the narration
				} // end if pGameInfo->bSoundEffectsEnabled
			} // end else
		}// end else if (m_bFirst)
		ReleaseDC(pDC);
	} // end else if ( m_bNewGame )

	CFrameWnd ::OnLButtonDown(nFlags, point);
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
void CMainWindow::OnLButtonUp(unsigned int nFlags, CPoint point) {
	if (bGameStarted)
		if (m_bNewGame && !m_bShowOutOfPlace) {
			CDC     *pDC;
			CSound  *pEffect = nullptr;

			ReleaseCapture();
			pDC = GetDC();

			if (m_bFirst) {
				CRect   WndRect;                                            // Selecting Source Area
				SrcRect.SetRect(OldRect.TopLeft().x - SIDE_BORDER,       // SrcRect is in Art coords
				                OldRect.TopLeft().y - TOP_BORDER,
				                OldRect.BottomRight().x - SIDE_BORDER,
				                OldRect.BottomRight().y - TOP_BORDER);
				HiLiteRect = OldRect;                                   // Highlight selected area
				MyFocusRect(pDC, HiLiteRect, R2_COPYPEN);
				if (pGameInfo->bSoundEffectsEnabled) {
//				sndPlaySound( nullptr, SND_SYNC );                       // Kill any current sound
//				sndPlaySound( PICK_SOUND, SND_ASYNC );                // Make pick noise
					pEffect = new CSound((CWnd *)this, PICK_SOUND,
					                     SOUND_WAVE | SOUND_ASYNCH |
					                     SOUND_QUEUE | SOUND_AUTODELETE);    //...Wave file, to delete itself
					(*pEffect).play();                                                      //...play the narration
				}

				Center.x = First.x + SIDE_BORDER + (OldRect.Width() / 2); // Center is in Screen coords
				Center.y = First.y + TOP_BORDER + (OldRect.Height() / 2);

				UpLeft.x = First.x + SIDE_BORDER;                       // Upper left corner of area
				UpLeft.y = First.y + TOP_BORDER;                        //...in Screen coords

//new:
				GetWindowRect(WndRect);                                      // Get the screen loc of wnd
				SetCursorPos(WndRect.TopLeft().x + Center.x, WndRect.TopLeft().y + Center.y);    //...use it to set cursor pos

				SetCapture();                                           // Hog all the mouse events
			}
			ReleaseDC(pDC);
		}
	CFrameWnd ::OnLButtonUp(nFlags, point);
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
	SetCursor(LoadCursor(nullptr, IDC_ARROW));           // Refresh cursor object

	if (bGameStarted && (m_bNewGame && !m_bShowOutOfPlace)) {
		CDC *pDC;
		pDC = GetDC();
		CRect NewRect;

		if ((nFlags & MK_LBUTTON) && m_bFirst) {                 // Selecting a region
			CPoint Hit;
			Hit = WinToArt(point);

			if (Hit.x < BaseRect.TopLeft().x) {                  // Dealing with left side
				if (point.x < (OldRect.TopLeft().x))             // Expanding to the left
					First.x -= m_nWidth;
				else if (point.x > (OldRect.TopLeft().x + m_nWidth))
					First.x += m_nWidth;                        // Shrinking from the left
			}

			else if (point.x > BaseRect.BottomRight().x) {   // Dealing with right side
				if (point.x > OldRect.BottomRight().x)       // Expanding to the right
					Second.x += m_nWidth;
				else if (point.x < (OldRect.BottomRight().x - m_nWidth))
					Second.x -= m_nWidth;                       // Shrinking from the right
			}

			else {                                              // This may be extraneous, no?
				First.x = BaseRect.TopLeft().x;
				Second.x = BaseRect.BottomRight().x;
			}

			if (Hit.y < BaseRect.TopLeft().y) {              // Dealing with Top
				if (point.y < OldRect.TopLeft().y)               // Expanding Up
					First.y -= m_nHeight;
				else if (point.y > (OldRect.TopLeft().y + m_nHeight))
					First.y += m_nHeight;                       // Shrinking from Top
			}

			else if (point.y > BaseRect.BottomRight().y) {   // Dealing with Bottom
				if (point.y > OldRect.BottomRight().y)       // Expanding Down
					Second.y += m_nHeight;
				else if (point.y < (OldRect.BottomRight().y - m_nHeight))
					Second.y -= m_nHeight;
			}

			else {                                              // This may be extraneous, no?
				First.y = BaseRect.TopLeft().y;
				Second.y = BaseRect.BottomRight().y;
			}

			NewRect.SetRect(First.x + SIDE_BORDER, First.y + TOP_BORDER,
			                Second.x + SIDE_BORDER, Second.y + TOP_BORDER);

			if (NewRect.BottomRight().x > (SIDE_BORDER + ART_WIDTH)) {
				NewRect.BottomRight().x = SIDE_BORDER + ART_WIDTH;
			} else if (NewRect.TopLeft().x < SIDE_BORDER) {
				NewRect.TopLeft().x = SIDE_BORDER;
			}
			if (NewRect.BottomRight().y > (TOP_BORDER + ART_HEIGHT)) {
				NewRect.BottomRight().y = TOP_BORDER + ART_HEIGHT;
			} else if (NewRect.TopLeft().y < TOP_BORDER) {
				NewRect.TopLeft().y = TOP_BORDER;
			}

			MyFocusRect(pDC, OldRect, R2_NOT);
			MyFocusRect(pDC, NewRect, R2_NOT);
			OldRect = NewRect;
		} else if (m_bFirst && !m_bShowOutOfPlace) {                     // Moving selected area

			if (point.x > Center.x + m_nWidth / 2) {
				Center.x += m_nWidth;       // Step by Part Widths
				UpLeft.x += m_nWidth;
			} else if (point.x < Center.x - m_nWidth / 2) {
				Center.x -= m_nWidth;
				UpLeft.x -= m_nWidth;
			}

			if (point.y > Center.y + m_nHeight / 2) {
				Center.y += m_nHeight;  // Step by Part Heights
				UpLeft.y += m_nHeight;
			} else if (point.y < Center.y - m_nHeight / 2) {
				Center.y -= m_nHeight;
				UpLeft.y -= m_nHeight;
			}
			/*
			            NewRect.SetRect( Center.x - (OldRect.Width()/2), Center.y - (OldRect.Height()/2),
			                        Center.x + (OldRect.Width()/2), Center.y + (OldRect.Height()/2) );
			*/
			NewRect.SetRect(UpLeft.x, UpLeft.y,
			                UpLeft.x + OldRect.Width(), UpLeft.y + OldRect.Height());

			if (NewRect.BottomRight().x > (SIDE_BORDER + ART_WIDTH)) {      // Keep within Art limits
				NewRect.BottomRight().x = SIDE_BORDER + ART_WIDTH;
				NewRect.TopLeft().x = NewRect.BottomRight().x - OldRect.Width();
			} else if (NewRect.TopLeft().x < SIDE_BORDER) {
				NewRect.TopLeft().x = SIDE_BORDER;
				NewRect.BottomRight().x = NewRect.TopLeft().x + OldRect.Width();
			}
			if (NewRect.BottomRight().y > (TOP_BORDER + ART_HEIGHT)) {
				NewRect.BottomRight().y = TOP_BORDER + ART_HEIGHT;
				NewRect.TopLeft().y = NewRect.BottomRight().y - OldRect.Height();
			} else if (NewRect.TopLeft().y < TOP_BORDER) {
				NewRect.TopLeft().y = TOP_BORDER;
				NewRect.BottomRight().y = NewRect.TopLeft().y + OldRect.Height();
			}

			if (NewRect.EqualRect(OldRect) == false) {            // if the Rectangle has changed
				MyFocusRect(pDC, OldRect, R2_NOT);              // Erase the old position
				MyFocusRect(pDC, NewRect, R2_NOT);               // Draw Focus rect in new position
				OldRect = NewRect;                              // Store the new focus rect
				MyFocusRect(pDC, HiLiteRect, R2_COPYPEN);    // Refresh the highlighted area
			}
		}

		ReleaseDC(pDC);
	}
	CFrameWnd ::OnMouseMove(nFlags, point);
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
void CMainWindow::OnRButtonDown(unsigned int nFlags, CPoint point) {
	if (bGameStarted) {
		if (InArtRegion(point) && (m_bNewGame && !m_bShowOutOfPlace)) {
			CDC     *pDC;
			CSound  *pEffect = nullptr;

			pDC = GetDC();

			if (pGameInfo->bSoundEffectsEnabled) {
				pEffect = new CSound((CWnd *)this, UNDO_SOUND,
				                     SOUND_WAVE | SOUND_ASYNCH |
				                     SOUND_QUEUE | SOUND_AUTODELETE);    //...Wave file, to delete itself
				(*pEffect).play();                                                      //...play the narration
			}

			if (m_bFirst) {                              // Deselect area
				MyFocusRect(pDC, OldRect, R2_NOT);       // Erase the Focus rect
				SplashScratch();                            // Repaint the artwork
			}

			else if (bSwitched) {                        // Switch back the two areas
				SwitchAreas(SrcRect, DstRect);
				bSwitched = false;                          // Don't allow second Undo
			}

			m_bFirst = false;                               // Return to beginning state
			CFrameWnd ::OnRButtonDown(nFlags, point);
			ReleaseDC(pDC);
		}
	} // end if bGameStarted

} // End OnRButtonDown

/*****************************************************************
 *
 *  OnMButtonDown
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Middle mouse button processing function
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
void CMainWindow::OnMButtonDown(unsigned int nFlags, CPoint point) {
	if (bGameStarted)
		if (!m_bShowOutOfPlace && (pGameInfo->bPlayingMetagame == false)) {          // Not available in meta-game
			m_bShowOutOfPlace = true;
			ShowOutOfPlace();                                           // Momentarily highlight out-of-place pieces
		}
} // end OnMButtonDown


// OnChar and OnSysChar
// These functions are called when keyboard input generates a character.
//

void CMainWindow::OnChar(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	CFrameWnd ::OnChar(nChar, nRepCnt, nFlags);     // default action
}

void CMainWindow::OnSysKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	if ((nChar == VK_F4) && (nFlags & 0x2000)) {
		PostMessage(WM_CLOSE, 0, 0);                 // *** remove later ***
	} else
		CFrameWnd::OnSysKeyDown(nChar, nRepCnt, nFlags);    // default action
}
void CMainWindow::OnKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	switch (nChar) {
	case VK_F1:                                             // F1 key is hit
		SendMessage(WM_COMMAND, IDC_RULES, BN_CLICKED);  // Activate the Rules dialog
		break;

	case VK_F2:                                             // F2 key is hit
		SendMessage(WM_COMMAND, IDC_SCROLL, BN_CLICKED);     // Activate the Options dialog
		break;

	case VK_RETURN:                                         // Return key is hit
	case VK_SPACE:                                          // Space bar is hit
		if (bGameStarted) {
			if (!m_bShowOutOfPlace  && (pGameInfo->bPlayingMetagame == false)) {
				m_bShowOutOfPlace = true;
				ShowOutOfPlace();                               // Momentarily highlight out-of-place pieces
			}
		}
		break;
	} // end switch
} // end OnKeyDown


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
	CDC     *pDC;
	CSound  *pEffect = nullptr;
	char    msg[64];

	pDC = GetDC();

	switch (nIDEvent) {

	case DISPLAY_TIMER:
		if (m_nTime == 0) {
			nSeconds++;                     // No time limit, increment
			if (nSeconds == 60) {
				nMinutes++;
				nSeconds = 0;
			}
		}

		else {                              // Count down time left
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

		if (nMinutes == 0 && nSeconds == 0) {
			char buf[64];
			bGameStarted = false;
			Common::sprintf_s(buf, "Score: %.0f%% correct", m_nScore);
			KillTimer(nIDEvent);                         // Stop the Display timer
			m_bNewGame = false;
			m_bFirst = false;
			if (pGameInfo->bSoundEffectsEnabled) {
				pEffect = new CSound((CWnd *)this, LOSE_SOUND,
				                     SOUND_WAVE | SOUND_ASYNCH |
				                     SOUND_QUEUE | SOUND_AUTODELETE);    //...Wave file, to delete itself
				(*pEffect).play();                                          //...play the narration
			}
			CheckForWin();                              // Update the score
			MSG lpmsg;
			while (PeekMessage(&lpmsg, nullptr, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE | PM_NOYIELD)) ;
			CMessageBox GameOverDlg((CWnd *)this, pGamePalette, "Game over.", buf);
			ReleaseCapture();                       // Let other windows get mouse events
			if (pGameInfo->bPlayingMetagame) {
				pGameInfo->lScore = 0;
				PostMessage(WM_CLOSE, 0, 0);         // and post a program exit
			}
		}

		break;

	case SHOW_TIMER:
		if (m_bShowOutOfPlace) {
			ShowOutOfPlace();                           // Un-highlight out-of-place pieces
			m_bShowOutOfPlace = false;
		}
		KillTimer(nIDEvent);
		break;

	default:
		CFrameWnd ::OnTimer(nIDEvent);
		break;
	} // end Switch

	ReleaseDC(pDC);

}

/**********************************************************
Other functions:
***********************************************************/

/*****************************************************************
 *
 *  SwitchAreas
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Draws a section of the source artwork to the scratch bitmap
 *
 *  FORMAL PARAMETERS:
 *
 *      CRect Src       Location of source area
 *      CRect Dst       Location of destination area
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
void CMainWindow::SwitchAreas(CRect Src, CRect Dst) {
	POINT SrcCR, DstCR, SizeCR, Temp;
	int c, r;

	SizeCR.x = Src.Width() / m_nWidth;
	SizeCR.y = Src.Height() / m_nHeight;
	SrcCR.x = Src.TopLeft().x / m_nWidth;
	SrcCR.y = Src.TopLeft().y / m_nHeight;
	DstCR.x = Dst.TopLeft().x / m_nWidth;
	DstCR.y = Dst.TopLeft().y / m_nHeight;

	DrawPart(Src.TopLeft(), Dst.TopLeft(), Src.Width(), Src.Height());

	CRect Overlap;
	if (Overlap.IntersectRect(Src, Dst) == 0) {          // They don't intersect
		DrawPart(Dst.TopLeft(), Src.TopLeft(), Src.Width(), Src.Height());

		for (c = 0; c < SizeCR.x; c++) {                // Update the Grid data
			for (r = 0; r < SizeCR.y; r++) {
				Temp.x = Grid[SrcCR.x + c][SrcCR.y + r].x;
				Temp.y = Grid[SrcCR.x + c][SrcCR.y + r].y;
				Grid[SrcCR.x + c][SrcCR.y + r].x = Grid[DstCR.x + c][DstCR.y + r].x;
				Grid[SrcCR.x + c][SrcCR.y + r].y = Grid[DstCR.x + c][DstCR.y + r].y;
				Grid[DstCR.x + c][DstCR.y + r].x = Temp.x;
				Grid[DstCR.x + c][DstCR.y + r].y = Temp.y;
			}
		}

	} else {                                            // Deal with intersection

		POINT *Movers, *MGrids, *SGrids, *p, *g, *s, Part, NewPart;
		Movers = (POINT *)malloc((SizeCR.x * SizeCR.y) * sizeof(POINT));
		MGrids = (POINT *)malloc((SizeCR.x * SizeCR.y) * sizeof(POINT));
		SGrids = (POINT *)malloc((SizeCR.x * SizeCR.y) * sizeof(POINT));

		s = SGrids;
		for (c = 0; c < SizeCR.x; c++) {                 // Put the locations of the source
			for (r = 0; r < SizeCR.y; r++) {            //...parts into a safe spot: SGrids
				s->x = Grid[SrcCR.x + c][SrcCR.y + r].x;
				s->y = Grid[SrcCR.x + c][SrcCR.y + r].y;
				s++;
			}
		}

		p = Movers;
		g = MGrids;
		for (c = 0; c < SizeCR.x; c++) {
			for (r = 0; r < SizeCR.y; r++) {
				Part.x = (DstCR.x + c) * m_nWidth;
				Part.y = (DstCR.y + r) * m_nHeight;
				if (Overlap.PtInRect(Part) == 0) {        //The parts which are not shared
					p->x = DstCR.x + c;                     //...by the source and destination
					p->y = DstCR.y + r;                     //...have their coord's stored in
					p++;                                    //...Movers, to which p points
					g->x = Grid[DstCR.x + c][DstCR.y + r].x;    //Put the locations of the
					g->y = Grid[DstCR.x + c][DstCR.y + r].y;    //...destinations in MGrids
					g++;                                        //...to which g points.
				}
			}
		}

		p = Movers;
		g = MGrids;
		for (c = 0; c < SizeCR.x; c++) {
			for (r = 0; r < SizeCR.y; r++) {
				Part.x = (SrcCR.x + c) * m_nWidth;
				Part.y = (SrcCR.y + r) * m_nHeight;
				if (Overlap.PtInRect(Part) == 0) {                    //Switch the parts that
					NewPart.x = p->x * m_nWidth;                    //...aren't in the overlap
					NewPart.y = p->y * m_nHeight;                   //...area
					DrawPart(NewPart, Part, m_nWidth, m_nHeight);
					p++;
					Grid[SrcCR.x + c][SrcCR.y + r].x = g->x;        //Then trade the Grid data
					Grid[SrcCR.x + c][SrcCR.y + r].y = g->y;        //...for the dst moved data
					g++;
				}
			}
		}

		s = SGrids;
		for (c = 0; c < SizeCR.x; c++) {
			for (r = 0; r < SizeCR.y; r++) {
				Grid[DstCR.x + c][DstCR.y + r].x = s->x;            //Trade Grid data for the
				Grid[DstCR.x + c][DstCR.y + r].y = s->y;            //...Source area
				s++;
			}
		}

		free(Movers);
		free(MGrids);
		free(SGrids);

	} // End else

	SplashScratch();                // Repaint the Art
	CheckForWin();                  // See if the new arrangement is a win

} // End SwitchAreas()

/*****************************************************************
 *
 *  DrawPart
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Draws a section of the source artwork to the scratch bitmap
 *
 *  FORMAL PARAMETERS:
 *
 *      CPoint Src      Location of source area
 *      CPoint Dst      Location of destination area
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
void CMainWindow::DrawPart(CPoint Src, CPoint Dst, int nWidth, int nHeight) {

	pScratch2DC->BitBlt(Dst.x, Dst.y, nWidth, nHeight, pScratch1DC, Src.x, Src.y, SRCCOPY);

} //End DrawPart();

bool CMainWindow::CopyPaletteContents(CPalette *pSource, CPalette *pDest) {
    ASSERT(pSource && pDest);

	// Get the number of entries in the source palette
	unsigned int nEntries = pSource->GetEntryCount();
	if (nEntries == 0)
		return false;

	// Use C++-isms to copy palette contents from src to dest
	Graphics::Palette *src = pSource->palette();
	Graphics::Palette *dest = pDest->palette();
	*dest = *src;

	return true;
}

/*****************************************************************
 *
 *  LoadArtWork
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Loads the Artwork to the Scratch1 bitmap
 *
 *  FORMAL PARAMETERS:
 *
 *      none
 *
 *  IMPLICIT INPUT PARAMETERS:
 *
 *      none
 *
 *  IMPLICIT OUTPUT PARAMETERS:
 *
 *      none
 *
 *  RETURN VALUE:
 *
 *      none
 *
 ****************************************************************/
bool CMainWindow::LoadArtWork() {
	CRect       rcDest;                                             // Art work - frame bitmap dims
	CRect       rcDIB;                                              // Source area for Art Work bmp
	HDIB        hDIB;
	CDC         *pDC;
	CDC         *pSourceDC;
	CBitmap     *pSource = nullptr,
	             *pOldBmp = nullptr;
	CPalette    *pOldPal = nullptr;
	char    ArtName[MAX_FILE_LENGTH];
	char        bufName[MAX_FILE_LENGTH + 10];

	int  nNumEntries;
	char chNumEntries[5];
	int i, pick;

	ifstream inFile;
	inFile.open(DATA_FILE);                                  // open the data store
	if (inFile.fail()) {
		return false;
	}
	inFile.getline(chNumEntries, sizeof(chNumEntries));     // read number of names in file
	nNumEntries = atoi(chNumEntries);
	assert(nNumEntries > 0);

	pick = nLastPick;
	while (pick == nLastPick) {
		pick = (brand() % nNumEntries) + 1;
	}

	for (i = 0; i < pick; i++) {
		if (!inFile.eof())
			inFile.getline(ArtName, sizeof(ArtName));       // load a name
	}

	inFile.close();                                         // close the data store
	nLastPick = pick;

	pDC = GetDC();
	pSourceDC = new CDC();
	pSource = new CBitmap();          // Source is the original, in-tact artwork for framed mode

	pSource->CreateCompatibleBitmap(pDC,    ART_WIDTH + (2 * FRAME_WIDTH),
	                                ART_HEIGHT + (2 * FRAME_HEIGHT));   // Set up Source
	pSourceDC->CreateCompatibleDC(pDC);                                          //...bitmap and DC
	pOldBmp = pSourceDC->SelectObject(pSource);


	CDibDoc     *pSourceDoc;

	pSourceDoc = new CDibDoc();
	ASSERT(pSourceDoc != nullptr);
	if (pSourceDoc == nullptr) {
		return false;
	}

	Common::sprintf_s(bufName, ".\\art\\%s", ArtName);
	Common::sprintf_s(szCurrentArt, "%s", bufName);                  // copy to a global for use in OnPaint

	(*pSourceDoc).OpenDocument(bufName);

	// Acquire the shared palette for our game from the art
	if (!pGamePalette) {
		pGamePalette = (*pSourceDoc).DetachPalette();
	} else {
		// WORKAROUND: Keep a single pGamePalette, since there
		// are UI elements that have pointers to it
		CPalette *src = pSourceDoc->DetachPalette();
		CopyPaletteContents(src, pGamePalette);
		delete src;
	}
	// setup new palette in scratch areas
	if (!pOldPal1) {
		pOldPal1 = pScratch1DC->SelectPalette(pGamePalette, false);
		pScratch1DC->RealizePalette();

		pOldPal2 = pScratch2DC->SelectPalette(pGamePalette, false);
		pScratch2DC->RealizePalette();

		pOldPal = pSourceDC->SelectPalette(pGamePalette, false);
		pSourceDC->RealizePalette();
	}

	hDIB = (*pSourceDoc).GetHDIB();

	if (hDIB) {
		if (bFramed)
			rcDest.SetRect(0, 0, ART_WIDTH + (2 * FRAME_WIDTH), ART_HEIGHT + (2 * FRAME_HEIGHT));
		else
			rcDest.SetRect(0, 0, ART_WIDTH, ART_HEIGHT);

		int cxDIB = (int) DIBWidth(hDIB);
		int cyDIB = (int) DIBHeight(hDIB);

		if (cxDIB > ART_WIDTH) {                     // Center and crop
			rcDIB.left = (cxDIB - ART_WIDTH) / 2;   //...too wide art
			rcDIB.right = rcDIB.left + ART_WIDTH;
		} else {                                    // Stretch too thin art
			rcDIB.left = 0;
			rcDIB.right = cxDIB;
		}

		if (cyDIB > ART_HEIGHT) {                    // Center and crop
			rcDIB.top = (cyDIB - ART_HEIGHT) / 2;   //...too long art
			rcDIB.bottom = rcDIB.top + ART_HEIGHT;
		} else {                                    // Stretch too short art
			rcDIB.top = 0;
			rcDIB.bottom = cyDIB;
		}
		if (bFramed) {
			PaintDIB((*pSourceDC).m_hDC, &rcDest, hDIB, &rcDIB, pGamePalette);
			pScratch1DC->BitBlt(0, 0, ART_WIDTH, ART_HEIGHT, pSourceDC,
			                    FRAME_WIDTH, FRAME_HEIGHT, SRCCOPY);
		} else {
			PaintDIB((*pScratch1DC).m_hDC, &rcDest, hDIB, &rcDIB, pGamePalette);

		}
	}

	delete pSourceDoc;
	pSourceDoc = nullptr;

	if (pOldBmp != nullptr)                        // Get rid of Source
		pSourceDC->SelectObject(pOldBmp);
	if (pOldPal != nullptr)
		pSourceDC->SelectPalette(pOldPal, false);
	if (pSourceDC->m_hDC != nullptr) {
		pSourceDC->DeleteDC();
		delete pSourceDC;
	}

	pSource->DeleteObject();
	delete pSource;

	ReleaseDC(pDC);


	POINT ScrOne, ScrTwo;
	int r, c;
	int x = 0, y = 0;
	bool bCheckGrid[MAX_COLUMNS][MAX_ROWS] = {{0}};   // Initialize all to zero
	bool bAssigning;                                // Flag for random assignment
	for (c = 0; c < m_nColumns; c++) {               // Each step in X
		for (r = 0; r < m_nRows; r++) {              // Each step in Y
			ScrTwo.x = c * m_nWidth;
			ScrTwo.y = r * m_nHeight;
			bAssigning = true;
			while (bAssigning) {
				x = brand() % m_nColumns;           // 0 thru m_nColumns - 1
				y = brand() % m_nRows;              // 0 thru m_nRows - 1
				bAssigning = bCheckGrid[x][y];
			}
			bCheckGrid[x][y] = true;
			Grid[c][r].x = x;                   // The r & c home of the art in
			// this spot.
			Grid[c][r].y = y;
			ScrOne.x = x * m_nWidth;            // Store Artwork X and Y
			ScrOne.y = y * m_nHeight;           //...placed at Grid[c][r]
			DrawPart(ScrOne, ScrTwo, m_nWidth, m_nHeight);
		} //end for r
	} //end for r

	return true;

} // end LoadArtWork()

/*****************************************************************
 *
 *  InitValues
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Sets several global variables to their default or initial states
 *
 *  FORMAL PARAMETERS:
 *
 *      none
 *
 *  IMPLICIT INPUT PARAMETERS:
 *
 *      m_bNewGame, m_nTime, m_bFirst, m_nRows, m_nColumns
 *
 *  IMPLICIT OUTPUT PARAMETERS:
 *
 *      none
 *
 *  RETURN VALUE:
 *
 *      void
 *
 ****************************************************************/
void CMainWindow::InitValues() {
	m_bPlaying = true;              // Start out playing
	m_bNewGame = true;              //...a new puzzle with
	m_bFirst = false;               //...minimum everything:
	bSwitched = false;
	bFramed = true;

	if (pGameInfo->bPlayingMetagame) {
		pGameInfo->lScore = 0L;
		if (pGameInfo->nSkillLevel == SKILLLEVEL_LOW) {
			m_nColumns = 3;
			m_nRows = 3;
			m_nTime = 60;
		} else if (pGameInfo->nSkillLevel == SKILLLEVEL_MEDIUM) {
			m_nColumns = START_COLUMNS;
			m_nRows = 3;
			m_nTime = 45;
		} else {                        // SKILLLEVEL_HIGH
			m_nColumns = START_COLUMNS;     //4;
			m_nRows = START_ROWS;           //4;
			m_nTime = 45;
		}
	} else {
		m_nColumns = START_COLUMNS;
		m_nRows = START_ROWS;
		m_nTime = 60;
	}

	m_nWidth = ART_WIDTH / m_nColumns;
	m_nHeight = ART_HEIGHT / m_nRows;

	nSeconds = m_nTime % 60;                // Always starts at one minute or less
	nMinutes = m_nTime / 60;

	tempTime = m_nTime;             // Set the temp values to start values
	tempRows = m_nRows;
	tempColumns = m_nColumns;
	tempFramed = bFramed;
} // End InitValues()


void CMainWindow::NewGame() {
	CDC         *pDC;
	CPalette    *pPalOld = nullptr;
	CBrush      *pBrushOld = nullptr,
	             *pBrushNew = nullptr;
	char        msg[64];

	KillTimer(DISPLAY_TIMER);        // Stop the Display timer

	CSound::clearWaveSounds();

	BeginWaitCursor();

	m_nTime = tempTime;             // get new time limit,
	m_nRows = tempRows;             //...new rows, and cols
	m_nColumns = tempColumns;
	bFramed = tempFramed;
	bSwitched = false;                      // Don't allow second Undo

	m_nWidth = ART_WIDTH / m_nColumns;
	m_nHeight = ART_HEIGHT / m_nRows;
	nMinutes = m_nTime / 60;
	nSeconds = m_nTime % 60;

	pDC = GetDC();

	pPalOld = (*pDC).SelectPalette(pGamePalette, false);            // Select in the artwork's palette
	(*pDC).RealizePalette();                                        // Use it

	PaintBitmap(pDC, pGamePalette, pLocaleBitmap, TIME_LOCATION_X, TIME_LOCATION_Y);

	pBrushNew = new CBrush();                                       // Construct a new brush object
	if (pBrushNew != nullptr) {                                         // If the constructor was successful:
		pBrushNew->CreateSolidBrush(PALETTERGB(128, 0, 0));           // Create my backdrop color brush
		pBrushOld = (*pDC).SelectObject(pBrushNew);                  // Select into the DC my new brush
		pDC->SetROP2(R2_COPYPEN);                                    // Set Draw mode to use the pen color
		if (bFramed)
			(*pDC).Rectangle(SIDE_BORDER - FRAME_WIDTH, TOP_BORDER - FRAME_HEIGHT,
			                 GAME_WIDTH - (SIDE_BORDER - FRAME_WIDTH),
			                 GAME_HEIGHT - (BOTTOM_BORDER - FRAME_HEIGHT));
		else
			(*pDC).Rectangle(SIDE_BORDER, TOP_BORDER, GAME_WIDTH - SIDE_BORDER, GAME_HEIGHT - BOTTOM_BORDER);
		(*pDC).SelectObject(pBrushOld);                          // Select in the old brush
		(*pDC).SelectPalette(pPalOld, false);                        // Select in the old palette
		delete pBrushNew;                                           // Delete the new brush
	}
	m_bNewGame = true;
	m_bPlaying = true;

	if (LoadArtWork() == false) {                                    // Load artwork to Scratch1
		PostMessage(WM_CLOSE, 0, 0);
		return;
	}

	CheckForWin();

	PaintBitmap(pDC, pGamePalette, pBlankBitmap, TIME_LOCATION_X, TIME_LOCATION_Y);
	if (m_nTime == 0)
		Common::sprintf_s(msg, "Time Used: %02d:%02d", nMinutes, nSeconds);
	else {
		Common::sprintf_s(msg, "Time Left: %02d:%02d", nMinutes, nSeconds);
	}
	(*m_pTimeText).DisplayString(pDC, msg, 16, FW_SEMIBOLD, OPTIONS_COLOR);

	EndWaitCursor();

	(*pDC).SelectPalette(pPalOld, false);
	ReleaseDC(pDC);

	InvalidateRect(nullptr, false);                                     // force a redraw of the entire window

	MSG lpmsg;
	while (PeekMessage(&lpmsg, nullptr, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE | PM_NOYIELD)) ;

	bStartOkay = true;
}

/*****************************************************************
 *
 *  InArtRegion
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Checks to see if a point is within the Artwork region of the window
 *
 *  FORMAL PARAMETERS:
 *
 *      CPoint point    The point to check
 *
 *  IMPLICIT INPUT PARAMETERS:
 *
 *      Extents of the main game window, and the extents of the artwork
 *
 *  IMPLICIT OUTPUT PARAMETERS:
 *
 *      none
 *
 *  RETURN VALUE:
 *
 *      bool:   true if point is within the Art Region,
 *              false if point is outside the Art Region
 *
 ****************************************************************/
bool InArtRegion(CPoint point) {
	if ((point.x > SIDE_BORDER && point.x < GAME_WIDTH - SIDE_BORDER) &&          // See if point lies within
	        (point.y > TOP_BORDER && point.y < GAME_HEIGHT - BOTTOM_BORDER))        //...ArtWork area
		return true;                                                            // Return true if it's inside
	else return false;                                                          //...and false if not
}

/******************************************************************************
Convert a point from the MainWindow coordinates to the ArtRegion coordinates,
******************************************************************************/

CPoint WinToArt(CPoint point) {
	point.x -= SIDE_BORDER;
	point.y -= TOP_BORDER;
	return point;
}

/*****************************************************************
 *
 *  ShowOutOfPlace();
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Highlights pieces which are not correctly placed
 *
 *  FORMAL PARAMETERS:
 *
 *      none
 *
 *  IMPLICIT INPUT PARAMETERS:
 *
 *      Grid
 *
 *  IMPLICIT OUTPUT PARAMETERS:
 *
 *      none
 *
 *  RETURN VALUE:
 *
 *      void
 *
 ****************************************************************/
void CMainWindow::ShowOutOfPlace() {
	CDC *pDC;
	CRect rect;
	int r, c;                                           // Row counter, column counter

	pDC = GetDC();

	for (c = 0; c < m_nColumns; c++) {                  // For each column
		for (r = 0; r < m_nRows; r++) {                 // For each row
			if ((Grid[c][r].x != c) || (Grid[c][r].y != r)) {              // If the piece is in the wrong location
				rect.SetRect(c * m_nWidth + SIDE_BORDER, r * m_nHeight + TOP_BORDER,
				             (c + 1) * m_nWidth + SIDE_BORDER, (r + 1) * m_nHeight + TOP_BORDER);
				MyFocusRect(pDC, rect, R2_NOT);                          // Increment the number correct
			}
		}
	}

	ReleaseDC(pDC);
	SetTimer(SHOW_TIMER, PAUSE_TIME, nullptr);

}

/*****************************************************************
 *
 *  MyFocusRect( CDC *pDC, CRect rect, int nDrawMode )
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Draws a rectangle which inverts the current pixels,
 *          thereby delineating the current area of focus.
 *
 *  FORMAL PARAMETERS:
 *
 *      CDC *pDC    The Device context in which the FocusRect is to be drawn
 *      CRect rect  The CRect object holding the location of the FocusRect
 *
 *  IMPLICIT INPUT PARAMETERS:
 *
 *      pMyPen and pMyBrush, global pointers to the Pen and Brush used
 *
 *  IMPLICIT OUTPUT PARAMETERS:
 *
 *      none
 *
 *  RETURN VALUE:
 *
 *      void
 *
 ****************************************************************/
void CMainWindow::MyFocusRect(CDC *pDC, CRect rect, int nDrawMode) {
	CBrush      *pMyBrush = nullptr;                   // New Brush
	CBrush      *pOldBrush = nullptr;                  // Pointer to old brush
	CPen        *pMyPen = nullptr;                     // New Pen
	CPen        *pOldPen = nullptr;                    // Pointer to old pen
	CPalette    *pPalOld = nullptr;                    // Pointer to old palette
	int         OldDrawMode;                        // Holder for old draw mode

	pMyBrush = new CBrush();                        // Construct new brush
	pMyPen = new CPen();                            // Construct new pen

	LOGBRUSH lb;                                    // log brush type
	lb.lbStyle = BS_HOLLOW;                         // Don't fill in area
	pMyBrush->CreateBrushIndirect(&lb);              // Create a new brush
	pMyPen->CreatePen(PS_INSIDEFRAME, HILITE_BORDER, RGB(255, 255, 255));    // Create a new pen

	pPalOld = (*pDC).SelectPalette(pGamePalette, false);     // Select in game palette
	(*pDC).RealizePalette();                                // Use it
	pOldPen = pDC->SelectObject(pMyPen);         // Select the new pen & save old
	pOldBrush = pDC->SelectObject(pMyBrush);     // Select the new brush & save old
	OldDrawMode = pDC->SetROP2(nDrawMode);       // Set pen mode, saving old state
	pDC->Rectangle(rect);                        // Draw the Rectangle to the DC
	pDC->SelectObject(pOldPen);                  // Select the old pen
	pDC->SelectObject(pOldBrush);                // Select the old brush
	pDC->SetROP2(OldDrawMode);                   // Set pen mode back to old state
	(*pDC).SelectPalette(pPalOld, false);           // Select back the old palette

	pMyBrush->DeleteObject();
	delete pMyBrush;

	pMyPen->DeleteObject();
	delete pMyPen;
}

/*****************************************************************
 *
 *  CheckForWin
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Checks to see how many parts are in their correct spaces
 *
 *  FORMAL PARAMETERS:
 *
 *      none
 *
 *  IMPLICIT INPUT PARAMETERS:
 *
 *      Grid, CurrentLocation, m_nRows, m_nColumns
 *
 *  IMPLICIT OUTPUT PARAMETERS:
 *
 *      none
 *
 *  RETURN VALUE:
 *
 *      float       Percent of parts correctly placed
 *
 ****************************************************************/
void CMainWindow::CheckForWin() {
	int r, c;                                           // Row counter, column counter
	float nCorrect;                                     // Number correct counter
	float Percent, TotalParts;                          // Percent correct, Total number of parts

	nCorrect = 0;                                       // Start with none correct
	for (c = 0; c < m_nColumns; c++) {                  // For each column
		for (r = 0; r < m_nRows; r++) {                 // For each row
			if ((Grid[c][r].x == c) && (Grid[c][r].y == r)) {              // If the piece is in the right location
				nCorrect++;                            // Increment the number correct
			}
		}
	}

	TotalParts = m_nColumns * m_nRows;                  // Calculate the number of parts
	Percent = (nCorrect / TotalParts) * 100;             // Determine the percentage that are correctly located

	m_nScore = Percent;                                 // Set the score equal to this percentage

} // End CheckForWin()


void CMainWindow::OnClose() {
	CDC     *pDC;
	CBrush  myBrush;
	CRect   myRect;

	pDC = GetDC();
	myRect.SetRect(0, 0, GAME_WIDTH, GAME_HEIGHT);
	myBrush.CreateStockObject(BLACK_BRUSH);
	(*pDC).FillRect(&myRect, &myBrush);
	ReleaseDC(pDC);

	// delete the game theme song
	//
	if (pGameSound != nullptr) {                   // IF we're playing the game theme song,
		delete pGameSound;                      //...get rid of it
		pGameSound = nullptr;
	}

	CSound::clearSounds();                      // Make sure it's cleared before returning to Metagame

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

	if (pOldBmp1 != nullptr)                   // Get rid of Scratch1
		pScratch1DC->SelectObject(pOldBmp1);
	if (pOldPal1 != nullptr)
		pScratch1DC->SelectPalette(pOldPal1, false);
	if (pScratch1DC->m_hDC != nullptr) {
		pScratch1DC->DeleteDC();
		delete pScratch1DC;
	}

	pScratch1->DeleteObject();
	delete pScratch1;

	if (pOldBmp2 != nullptr)                   // Get rid of Scratch2
		pScratch2DC->SelectObject(pOldBmp2);
	if (pOldPal2 != nullptr)
		pScratch2DC->SelectPalette(pOldPal2, false);
	if (pScratch2DC->m_hDC != nullptr) {
		pScratch2DC->DeleteDC();
		delete pScratch2DC;
	}

	if (pGamePalette != nullptr) {
		pGamePalette->DeleteObject();
		delete pGamePalette;
	}

	CFrameWnd::OnClose();

	MFC::PostMessage(ghParentWnd, WM_PARENTNOTIFY, WM_DESTROY, 0L);
}

void CMainWindow::GetSubOptions(CWnd *pParentWind) {
	COptnDlg OptionsDlg(pParentWind, pGamePalette);          // Call Specific Game
	//...Options dialog box
	OptionsDlg.m_nColumns = m_nColumns;
	OptionsDlg.m_nRows  = m_nRows;
	OptionsDlg.m_nTime  = m_nTime;
	OptionsDlg.m_nMins  = nMinutes;
	OptionsDlg.m_nSecs  = nSeconds;
	OptionsDlg.m_nScore = m_nScore;
	OptionsDlg.m_bFramed = bFramed;

	if (OptionsDlg.DoModal() == IDOK) {              // save values set in dialog box
		tempTime = OptionsDlg.m_nTime;              // get new time limit,
		tempRows = OptionsDlg.m_nRows;              //...new rows, and cols
		tempColumns = OptionsDlg.m_nColumns;        //...and store in temps
		tempFramed = OptionsDlg.m_bFramed;
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
	ON_WM_SYSKEYDOWN()
	ON_WM_KEYDOWN()
	ON_WM_TIMER()
	ON_WM_MBUTTONDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_MESSAGE(MM_MCINOTIFY, CMainWindow::OnMCINotify)
	ON_MESSAGE(MM_WOM_DONE, CMainWindow::OnMMIONotify)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

} // namespace ArtParts
} // namespace HodjNPodj
} // namespace Bagel
