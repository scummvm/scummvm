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
#include "bagel/boflib/sound.h"
#include "bagel/hodjnpodj/hnplibs/mainmenu.h"
#include "bagel/hodjnpodj/hnplibs/cmessbox.h"
#include "bagel/hodjnpodj/hnplibs/rules.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/hnplibs/gamedll.h"
#include "bagel/hodjnpodj/beacon/resource.h"
#include "bagel/hodjnpodj/beacon/globals.h"
#include "bagel/hodjnpodj/beacon/beacon.h"
#include "bagel/hodjnpodj/beacon/optndlg.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {
namespace Beacon {

void CALLBACK GetSubOptions(CWnd *pParentWind);
void CALLBACK StepAlongLine(int xpos, int ypos, CDC *cdc);
bool InArtRegion(CPoint point);
void MyFocusRect(CDC *pDC, CRect rect, bool bPressed);

CBmpButton  *m_pScrollButton;               // Scroll button

CBitmap     *pScrollUp = nullptr,
             *pScrollDown = nullptr;

CPalette    *pOldPal1 = nullptr,
             *pOldPal2 = nullptr,
              *pGamePalette = nullptr;           // Palette of current artwork

CBitmap     *pBeaconBitmap = nullptr,          // Lighthouse bitmap
             *pBackBitmap = nullptr,            // Offscreen bitmap of border, pie wedges and lighthouse
              *pOldBmp1 = nullptr,
               *pArtBitmap = nullptr,             // Holds the latest piece of art
                *pOldBmp2 = nullptr;

CDC         *pArtDC = nullptr,
             *pBackDC = nullptr;

CText       *m_pScoreTxt = nullptr;
CText       *m_pSweepTxt = nullptr;

bool        bSuccess;
bool        m_bIgnoreScrollClick;
bool        bChanged;
bool        bPlaying = false;
bool        bNewGame = false;                           // Not playing a game right now

unsigned int        nCurrentIndex = BUTTON_ENTRY;               // The current beacon color index
unsigned int        nNextIndex = BUTTON_ENTRY;                  // The next beacon color index

unsigned int        nSlice = START_ENTRY;
int         nLastPick = 0;

int         m_nSweepCount = 0;
constexpr int         m_nTotalSquares = (NUM_COLUMNS * NUM_ROWS) -
                              ((START_WIDTH / GRID_WIDTH) * (START_HEIGHT / GRID_HEIGHT));
int         m_nRemoved = 0;                             // Count blocks removed
unsigned int        m_nSweeps = MAX_SWEEPS;
unsigned int        m_nSpeed = MAX_SPEED;
unsigned int        tempSweeps = 100;                           // default sweeps = 100
unsigned int        tempSpeed = MAX_SPEED;                      // default speed = Hovercraft
bool        m_bAutomatic = false;
bool        m_bChangeAtTwelve = false;
float       m_Score = 0;

bool        PictureGrid[NUM_COLUMNS][NUM_ROWS];

static CSound   *pGameSound = nullptr;                             // Game theme song

extern  LPGAMESTRUCT pGameInfo;
extern HWND ghParentWnd;

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

// CMainWindow constructor:
// Create the window with the appropriate style, size, menu, etc.;
// it will be later revealed by CTheApp::InitInstance().  Then
// create our splash screen object by opening and loading its DIB.
//
CMainWindow::CMainWindow() {
	CDC     *pDC;
	CPalette *pOldPal;
	CBitmap *pBeamBitmap = nullptr,            // Beam bitmap
	         *pBorderBitmap = nullptr;          // Border bitmap
	CString WndClass;
	CRect   MainRect, ScrollRect, statRect;
	CBrush  *pMyBrush = nullptr;
	int     i;

	BeginWaitCursor();
	initStatics();

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
	Create(WndClass, "Boffo Games -- Beacon", WS_POPUP, MainRect, nullptr, 0);

	pDC = GetDC();
	pBorderBitmap = FetchBitmap(pDC, &pGamePalette, MAINSCREEN);

	pOldPal = pDC->SelectPalette(pGamePalette, false);  // select the game palette
	pDC->RealizePalette();                              //...and realize it

//	SetBeamEntries( pDC );

	// Build Scroll Command button

	pScrollUp = FetchResourceBitmap(pDC, nullptr, SCROLLUP);            // Get the up scroll button
	pScrollDown = FetchResourceBitmap(pDC, nullptr, SCROLLDOWN);        // Get the dowm scroll button

	ShowWindow(SW_SHOWNORMAL);
	PaintBitmap(pDC, pGamePalette, pBorderBitmap, 0, 0);
	PaintMaskedBitmap(pDC, pGamePalette, pScrollUp, SCROLL_BUTTON_X, SCROLL_BUTTON_Y);

	m_pScrollButton = new CBmpButton;
	ASSERT(m_pScrollButton != nullptr);
	ScrollRect.SetRect(SCROLL_BUTTON_X, SCROLL_BUTTON_Y,
	                   SCROLL_BUTTON_X + SCROLL_BUTTON_DX - 1,
	                   SCROLL_BUTTON_Y + SCROLL_BUTTON_DY - 1);
	bSuccess = (*m_pScrollButton).Create(nullptr, BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, ScrollRect, this, IDC_SCROLL);
	ASSERT(bSuccess);
	bSuccess = (*m_pScrollButton).LoadBitmaps(pGamePalette, pScrollUp, pScrollDown, nullptr, nullptr);
	pScrollUp = nullptr;
	pScrollDown = nullptr;

	ASSERT(bSuccess);
	m_bIgnoreScrollClick = false;

	pBackBitmap = new CBitmap();
	pBackDC = new CDC();

	pBackBitmap->CreateCompatibleBitmap(pDC, GAME_WIDTH, GAME_HEIGHT);
	pBackDC->CreateCompatibleDC(pDC);
	pOldBmp1 = pBackDC->SelectObject(pBackBitmap);
	pOldPal1 = pBackDC->SelectPalette(pGamePalette, false);
	pBackDC->RealizePalette();

	pArtBitmap = new CBitmap();
	pArtDC = new CDC();

	pArtBitmap->CreateCompatibleBitmap(pDC, GAME_WIDTH, GAME_HEIGHT);
	pArtDC->CreateCompatibleDC(pDC);
	pOldBmp2 = pArtDC->SelectObject(pArtBitmap);
	pOldPal2 = pArtDC->SelectPalette(pGamePalette, false);
	pArtDC->RealizePalette();

	PaintBitmap(pArtDC, pGamePalette, pBorderBitmap, 0, 0);      //SplashScreen to art bitmap
	pBorderBitmap->DeleteObject();
	delete pBorderBitmap;

	#ifndef BACKDROP
	pBeamBitmap = FetchBitmap(pDC, nullptr, BEAMSCREEN);
	PaintBitmap(pBackDC, pGamePalette, pBeamBitmap, 0, 0);   //SplashScreen to back
	pBeamBitmap->DeleteObject();
	#endif
	delete pBeamBitmap;

	for (i = 0; i < NUM_BUTTONS; i++) {
		int level = i / 2;
		colorBlock[i].rLocation.SetRect(
			BLOCK_OFFSET_X + ((BLOCK_WIDTH + BLOCK_SPACE_X) * ((i + 2) % 2)),
		    BLOCK_OFFSET_Y + ((BLOCK_HEIGHT + BLOCK_SPACE_Y) * level),
		    BLOCK_OFFSET_X + ((BLOCK_WIDTH + BLOCK_SPACE_X) * ((i + 2) % 2)) + BLOCK_WIDTH,
		    BLOCK_OFFSET_Y +
		    ((BLOCK_HEIGHT + BLOCK_SPACE_Y) * level) + BLOCK_HEIGHT);
		colorBlock[i].nColorIndex = BUTTON_ENTRY + i;

		pMyBrush = new CBrush();                        // Construct new brush
		(*pMyBrush).CreateSolidBrush(PALETTEINDEX((uint16)(colorBlock[i].nColorIndex)));

		(*pArtDC).FillRect(colorBlock[i].rLocation, pMyBrush);

		MyFocusRect(pArtDC, colorBlock[i].rLocation, false);     // standard highlight on button

		pMyBrush->DeleteObject();
		delete pMyBrush;
		pMyBrush = nullptr;
	}

	statRect.SetRect(SIDE_BORDER + ART_WIDTH, TOP_BORDER,
	                 GAME_WIDTH - SIDE_BORDER, GAME_HEIGHT - BOTTOM_BORDER);
	MyFocusRect(pArtDC, statRect, false);                // highlight button area

	statRect.SetRect(BLOCK_OFFSET_X + 5, TOP_BORDER + 5, GAME_WIDTH - SIDE_BORDER, TOP_BORDER + 25);
	if ((m_pScoreTxt = new CText()) != nullptr) {
		(*m_pScoreTxt).SetupText(pDC, pGamePalette, &statRect, JUSTIFY_LEFT);
	}

	statRect.SetRect(BLOCK_OFFSET_X + 5, TOP_BORDER + 25, GAME_WIDTH - SIDE_BORDER, TOP_BORDER + 45);
	if ((m_pSweepTxt = new CText()) != nullptr) {
		(*m_pSweepTxt).SetupText(pDC, pGamePalette, &statRect, JUSTIFY_LEFT);
	}

	(*pDC).SelectPalette(pOldPal, false);           // Select back the old palette
	ReleaseDC(pDC);

	//srand((unsigned) time(nullptr));         // seed the random number generator
	NewGame();

	if (pGameInfo->bMusicEnabled) {
		pGameSound = new CSound(this, GAME_THEME, SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END);
		if (pGameSound != nullptr) {
			(*pGameSound).midiLoopPlaySegment(1300, 36500, 0, FMT_MILLISEC);
		} // end if pGameSound
	}

	EndWaitCursor();

	if (pGameInfo->bPlayingMetagame) {
		bNewGame = true;                                        // in meta start out in a game
		pGameInfo->lScore = 0L;                                 // Make sure if they bail out it's 0
	} else
		PostMessage(WM_COMMAND, IDC_SCROLL, BN_CLICKED);         // Activate the Options dialog

} //End of CMainWindow

void CMainWindow::initStatics() {
	m_pScrollButton = nullptr;
	pScrollUp = pScrollDown = nullptr;
	pOldPal1 = pOldPal2 = pGamePalette = nullptr;
	pBeaconBitmap = pBackBitmap = pOldBmp1 = nullptr;
	pArtBitmap = pOldBmp2 = nullptr;

	pArtDC = pBackDC = nullptr;
	m_pScoreTxt = m_pSweepTxt = nullptr;

	bSuccess = false;
	m_bIgnoreScrollClick = false;
	bChanged = false;
	bPlaying = false;
	bNewGame = false;

	nCurrentIndex = BUTTON_ENTRY;
	nNextIndex = BUTTON_ENTRY;
	nSlice = START_ENTRY;
	nLastPick = 0;
	m_nSweepCount = 0;
	m_nRemoved = 0;
	m_nSweeps = MAX_SWEEPS;
	m_nSpeed = MAX_SPEED;
	tempSweeps = 100;
	tempSpeed = MAX_SPEED;
	m_bAutomatic = false;
	m_bChangeAtTwelve = false;
	m_Score = 0;
}

// OnPaint:
// This is called whenever Windows sends a WM_PAINT message.
// Note that creating a CPaintDC automatically does a BeginPaint and
// an EndPaint call is done when it is destroyed at the end of this
// function.  CPaintDC's constructor needs the window (this).
//
void CMainWindow::OnPaint() {
	CDC *pDC;
	CPalette *pOldPal = nullptr;
	PAINTSTRUCT     lpPaint;
	char    msg[64];

	pDC = GetDC();
	pOldPal = (*pDC).SelectPalette(pGamePalette, false);
	(*pDC).RealizePalette();

	InvalidateRect(nullptr, false);            // invalidate the entire window
	BeginPaint(&lpPaint);

	pBeaconBitmap = FetchResourceBitmap(pArtDC, nullptr, IDB_BEACON_BMP);

	PaintMaskedBitmap(pArtDC, pGamePalette, pBeaconBitmap,
	                  SIDE_BORDER + (ART_WIDTH / 2) - LTHOUSE_OFFSET_X,
	                  TOP_BORDER + (ART_HEIGHT / 2) - LTHOUSE_OFFSET_Y);


	pDC->BitBlt(0, 0, GAME_WIDTH, GAME_HEIGHT, pArtDC, 0, 0, SRCCOPY);

	Common::sprintf_s(msg, "Score: %.0f %%", m_Score);
	(*m_pScoreTxt).DisplayString(pDC, msg, 16, FW_BOLD, OPTIONS_COLOR);

	if ((m_nSweeps < MAX_SWEEPS) && bPlaying)
		Common::sprintf_s(msg, "Sweeps: %d", m_nSweepCount - 1);                     // Update text on screen
	else
		Common::sprintf_s(msg, "Sweeps: %d", m_nSweepCount);
	(*m_pSweepTxt).DisplayString(pDC, msg, 16, FW_BOLD, OPTIONS_COLOR);

	EndPaint(&lpPaint);

	(*pDC).SelectPalette(pOldPal, false);
	ReleaseDC(pDC);
}

void CMainWindow::DrawBeams(CDC *pDC) {
	CDC         *pBigDC = nullptr;
	CBitmap     *pBigBmp = nullptr,
	             *pBigBmpOld = nullptr;
	CBrush      *pMyBrush = nullptr;                   // New Brush
	CBrush      *pOldBrush = nullptr;                  // Pointer to old brush
	CPen        *pMyPen = nullptr;                     // New Brush
	CPen        *pOldPen = nullptr;                    // Pointer to old brush
	CPalette    *pPalOld = nullptr,                    // Pointer to old palette
	             *pBigPalOld = nullptr;
	CRect       rect;
	CPoint      StartPt, EndPt, Center;
	unsigned int    i;
	int     radius;
	float   degrees, x, y, radians, rads;

	pPalOld = (*pDC).SelectPalette(pGamePalette, false);     // Select in game palette
	(*pDC).RealizePalette();                                // Use it

	pBigBmp = new CBitmap();
	pBigDC = new CDC();

	bSuccess = pBigBmp->CreateCompatibleBitmap(pDC, ART_WIDTH * 2, ART_HEIGHT * 2);
	ASSERT(bSuccess);
	bSuccess = pBigDC->CreateCompatibleDC(pDC);
	ASSERT(bSuccess);
	pBigBmpOld = pBigDC->SelectObject(pBigBmp);
	ASSERT(pBigBmpOld != nullptr);
	pBigPalOld = pBigDC->SelectPalette(pGamePalette, false);
	pBigDC->RealizePalette();

	Center.x = ART_WIDTH;
	Center.y = ART_HEIGHT;

	rect.SetRect(0, 0, ART_WIDTH * 2, ART_HEIGHT * 2);
	radius = ART_WIDTH;

	x = (float)cos(0.0);
	y = (float)sin(0.0);

	EndPt.x = Center.x + (int)(x * radius);     // + radius
	EndPt.y = Center.y  + (int)(y * radius);    //- radius;

	for (i = 0; i < NUM_BEAMS; i++) {
		degrees = ((float)(i + 1)) * (float)(360 / NUM_BEAMS);

		radians = degrees * (float)0.017453292;
		rads = (degrees + 0.5F) * (float)0.017453292;

		x = (float)cos(rads);
		y = (float)sin(rads);

		StartPt.x = Center.x + (int)(x * radius);
		StartPt.y = Center.y + (int)(y * radius);

		pMyPen = new CPen();                            // Construct new pen
		(*pMyPen).CreatePen(PS_SOLID, 1, PALETTEINDEX((uint16)(i + START_ENTRY)));
		pOldPen = (*pBigDC).SelectObject(pMyPen);
		pMyBrush = new CBrush();                        // Construct new brush
		(*pMyBrush).CreateSolidBrush(PALETTEINDEX((uint16)(i + START_ENTRY)));
		pOldBrush = (*pBigDC).SelectObject(pMyBrush);

		(*pBigDC).Pie(&rect, StartPt, EndPt);

		(*pBigDC).SelectObject(pOldPen);

		pMyPen->DeleteObject();
		delete pMyPen;
		pMyPen = nullptr;

		(*pBigDC).SelectObject(pOldBrush);

		pMyBrush->DeleteObject();
		delete pMyBrush;
		pMyBrush = nullptr;

		x = (float)cos(radians);
		y = (float)sin(radians);

		EndPt.x = Center.x + (int)(x * radius);
		EndPt.y = Center.y + (int)(y * radius);
	}

	(*pDC).BitBlt(SIDE_BORDER, TOP_BORDER, ART_WIDTH, ART_HEIGHT, pBigDC, ART_WIDTH / 2, ART_HEIGHT / 2, SRCCOPY);
	(*pDC).SelectPalette(pPalOld, false);           // Select back the old palette

	if (pBigBmpOld != nullptr)                         // Get rid of Big stuff
		pBigDC->SelectObject(pBigBmpOld);
	if (pBigPalOld != nullptr)
		pBigDC->SelectPalette(pBigPalOld, false);
	if (pBigDC->m_hDC != nullptr) {
		pBigDC->DeleteDC();
		delete pBigDC;
	}

	pBigBmp->DeleteObject();
	delete pBigBmp;
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
	CDC *pDC = nullptr;

	if (HIWORD(lParam) == BN_CLICKED) {

		pDC = GetDC();
		CRules  RulesDlg((CWnd *)this, RULES_TEXT, pGamePalette, pGameInfo->bSoundEffectsEnabled ? RULES_WAV : nullptr);                // Construct Rules dialog
		CMainMenu COptionsWind((CWnd *)this, pGamePalette,
		                       pGameInfo->bPlayingMetagame ? (NO_NEWGAME | NO_OPTIONS) : 0,
		                       GetSubOptions, RULES_TEXT, pGameInfo->bSoundEffectsEnabled ? RULES_WAV : nullptr, pGameInfo) ;         // Construct Option dialog
		KillTimer(BEACON_TIMER);

		switch (wParam) {

		case IDC_RULES:
			CSound::waitWaveSounds();

			m_bIgnoreScrollClick = true;
			(*m_pScrollButton).SendMessage(BM_SETSTATE, true, 0L);

			RulesDlg.DoModal();
			m_bIgnoreScrollClick = false;
			(*m_pScrollButton).SendMessage(BM_SETSTATE, false, 0L);

			if (bPlaying) SetTimer(BEACON_TIMER, SPEED_BASE - (m_nSpeed * SPEED_STEP), nullptr);
			SetBeamEntries(pDC);
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

			case IDC_OPTIONS_NEWGAME:                           // Selected New Game
				bPlaying = false;
				MyFocusRect(pDC, colorBlock[nNextIndex - BUTTON_ENTRY].rLocation, false);    // erase highlight on nNextIndex
				NewGame();
				char        msg[64];
				Common::sprintf_s(msg, "Score: %.0f %%", m_Score);
				(*m_pScoreTxt).DisplayString(pDC, msg, 16, FW_BOLD, OPTIONS_COLOR);

				Common::sprintf_s(msg, "Sweeps: %d", m_nSweepCount);
				(*m_pSweepTxt).DisplayString(pDC, msg, 16, FW_BOLD, OPTIONS_COLOR);

				(*m_pScrollButton).SendMessage(BM_SETSTATE, false, 0L);
				m_bIgnoreScrollClick = false;
				break;

			case IDC_OPTIONS_RETURN:
				(*m_pScrollButton).SendMessage(BM_SETSTATE, false, 0L);
				m_bIgnoreScrollClick = false;
				if (bPlaying)
					SetTimer(BEACON_TIMER, SPEED_BASE - (m_nSpeed * SPEED_STEP), nullptr);
				break;

			case IDC_OPTIONS_QUIT:                      // Quit button was clicked
				PostMessage(WM_CLOSE, 0, 0);         // Post a program exit
				ReleaseDC(pDC);
				return false;

			} //end switch(ComDlg.DoModal())

			SetBeamEntries(pDC);                             // Make sure the palette is black

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
						(*pGameSound).midiLoopPlaySegment(1300, 36500, 0, FMT_MILLISEC);
				} // end if pGameSound
			} // end else..musicenabled check

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
	CDC     *pDC;
	CRect   rectTitle;

	pDC = GetDC();

	rectTitle.SetRect(NEWGAME_LOCATION_X, NEWGAME_LOCATION_Y,
	                  NEWGAME_LOCATION_X + NEWGAME_WIDTH,
	                  NEWGAME_LOCATION_Y + NEWGAME_HEIGHT);

	if (rectTitle.PtInRect(point) && (pGameInfo->bPlayingMetagame == false)) {
		KillTimer(BEACON_TIMER);
		bPlaying = false;                   // Want to get a new piece of artwork
		bNewGame = false;                   // Will be reset to true in NewGame()
		MyFocusRect(pDC, colorBlock[nNextIndex - BUTTON_ENTRY].rLocation, false);        // erase highlight on nNextIndex
		NewGame();                          // Activate New Game
		char        msg[64];
		Common::sprintf_s(msg, "Score: %.0f %%", m_Score);
		(*m_pScoreTxt).DisplayString(pDC, msg, 16, FW_BOLD, OPTIONS_COLOR);

		if (m_nSweepCount < 0)  m_nSweepCount = 0;                          // Make sure it doesn't go neg
		Common::sprintf_s(msg, "Sweeps: %d", m_nSweepCount);
		(*m_pSweepTxt).DisplayString(pDC, msg, 16, FW_BOLD, OPTIONS_COLOR);
	}

	ReleaseDC(pDC);

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
	CDC     *pDC;
	int     i;
	bool    bHit = false;
	char    msg[64];

	pDC = GetDC();

	if (!m_bAutomatic && bNewGame) {
		for (i = 0; i < NUM_BUTTONS; i++) {
			if (colorBlock[i].rLocation.PtInRect(point)) {
				bHit = true;
				if (pGameInfo->bSoundEffectsEnabled)
					sndPlaySound(PICK_SOUND, SND_ASYNC);                                     // Make a noise
				MyFocusRect(pDC, colorBlock[nNextIndex - BUTTON_ENTRY].rLocation, false);    // erase highlight on nNextIndex
				MyFocusRect(pArtDC, colorBlock[nNextIndex - BUTTON_ENTRY].rLocation, false);     // erase highlight on nNextIndex
				nNextIndex = colorBlock[i].nColorIndex;                                     // make this the next beam color
				MyFocusRect(pDC, colorBlock[i].rLocation, true);                             // draw hightlight on nNextIndex
				MyFocusRect(pArtDC, colorBlock[i].rLocation, true);                          // draw hightlight on nNextIndex
//bar
// SWITCH ON DEMAND:
//bar               if ( m_bChangeAtTwelve == false )
//bar                   nCurrentIndex = nNextIndex;                                             //...switch to the new beacon color
			}
		} // end for

		if (!bPlaying && bHit) {                                                 // first time
			bPlaying = true;
			nCurrentIndex = nNextIndex;
			SetTimer(BEACON_TIMER, SPEED_BASE - (m_nSpeed * SPEED_STEP), nullptr);
			if (m_nSweeps < MAX_SWEEPS)
				Common::sprintf_s(msg, "Sweeps: %d", m_nSweepCount - 1);                     // Update text on screen
			else
				Common::sprintf_s(msg, "Sweeps: %d", m_nSweepCount);                         // Update text on screen
			(*m_pSweepTxt).DisplayString(pDC, msg, 16, FW_BOLD, OPTIONS_COLOR);
		}
	}

	ReleaseDC(pDC);

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

	CFrameWnd ::OnMouseMove(nFlags, point);
}

/*****************************************************************
 *
 *  OnChar / OnSysKeyDown / OnKeyDown
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Keyboard input handling functions
 *
 *  FORMAL PARAMETERS:
 *
 *      unsigned int nChar      Virtual key info
 *      unsigned int nRepCnt    Virtual key info
 *      unsigned int nFlags     Virtual key info
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
void CMainWindow::OnChar(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	CFrameWnd ::OnChar(nChar, nRepCnt, nFlags);     // default action
}

void CMainWindow::OnSysKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	if ((nChar == VK_F4) && (nFlags & 0x2000))      // terminate app on ALT-F4
		PostMessage(WM_CLOSE, 0, 0);
	else
		CFrameWnd::OnChar(nChar, nRepCnt, nFlags);  // default action
}

void CMainWindow::OnKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	if (nChar == VK_F1) {                                  // F1 key is hit
		SendMessage(WM_COMMAND, IDC_RULES, BN_CLICKED);      // Activate the Rules dialog
	} else if (nChar == VK_F2) {                         // F2 key is hit
		SendMessage(WM_COMMAND, IDC_SCROLL, BN_CLICKED);         // Activate the Options dialog
	}
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
	CDC *pDC;
	char    msg[64];

	pDC = GetDC();

	switch (nIDEvent) {

	case BEACON_TIMER:
		CheckUnderBeam();
		MoveBeam();
		if (nSlice == CHANGE_BEAM) {                             // At the turnover point,
			if (pGameInfo->bSoundEffectsEnabled)
				sndPlaySound(MID_SOUND, SND_ASYNC);          // Make a "you lost" noise

			if (m_nSweeps < MAX_SWEEPS) m_nSweepCount--;        // If limited sweeps, count down
			else    m_nSweepCount++;                            //...or for unlimited, count up

			if ((m_nSweeps < MAX_SWEEPS) && (m_nSweepCount != 0))
				Common::sprintf_s(msg, "Sweeps: %d", m_nSweepCount - 1);                     // Update text on screen
			else
				Common::sprintf_s(msg, "Sweeps: %d", m_nSweepCount);                         // Update text on screen
			(*m_pSweepTxt).DisplayString(pDC, msg, 16, FW_BOLD, OPTIONS_COLOR);

			m_Score = ((float) m_nRemoved / (float) m_nTotalSquares) * 100;         // Score is the percentage
			if ((m_Score > 99.00) && (m_nRemoved < m_nTotalSquares))
				Common::sprintf_s(msg, "Score: 99%%");
			else
				Common::sprintf_s(msg, "Score: %.0f %%", m_Score);                           // Update score on screen
			(*m_pScoreTxt).DisplayString(pDC, msg, 16, FW_BOLD, OPTIONS_COLOR);

			if (m_bAutomatic) {                                                      // Game plays by itself
				if (++nNextIndex >= (BUTTON_ENTRY + NUM_BUTTONS))                    // Step through colors
					nNextIndex = BUTTON_ENTRY;
				MyFocusRect(pDC, colorBlock[nCurrentIndex - BUTTON_ENTRY].rLocation, false);     // erase highlight on nNextIndex
				MyFocusRect(pDC, colorBlock[nNextIndex - BUTTON_ENTRY].rLocation, true);         // draw hightlight on nNextIndex
				MyFocusRect(pArtDC, colorBlock[nCurrentIndex - BUTTON_ENTRY].rLocation, false);  // erase highlight on nNextIndex
				MyFocusRect(pArtDC, colorBlock[nNextIndex - BUTTON_ENTRY].rLocation, true);          // draw hightlight on nNextIndex
				nCurrentIndex = nNextIndex;                         //...switch to the new beacon color
			}

			if (m_bChangeAtTwelve)
				nCurrentIndex = nNextIndex;                         //...switch to the new beacon color

			if (m_nRemoved >= m_nTotalSquares) {
				KillTimer(BEACON_TIMER);
				bPlaying = false;                               // Want to get a new piece of artwork
				bNewGame = false;                           // Will be set to true in NewGame()
				if (m_bAutomatic) {
					MyFocusRect(pDC, colorBlock[nNextIndex - BUTTON_ENTRY].rLocation, false);    // erase highlight on nNextIndex
					NewGame();

					Common::sprintf_s(msg, "Score: %.0f %%", m_Score);
					(*m_pScoreTxt).DisplayString(pDC, msg, 16, FW_BOLD, OPTIONS_COLOR);

					Common::sprintf_s(msg, "Sweeps: %d", m_nSweepCount);
					(*m_pSweepTxt).DisplayString(pDC, msg, 16, FW_BOLD, OPTIONS_COLOR);
				} else {
					if (pGameInfo->bSoundEffectsEnabled)
						sndPlaySound(WIN_SOUND, SND_ASYNC);              // Make a "you won" noise
					CMessageBox GameOverDlg((CWnd *)this, pGamePalette,
					                        "Whooooopeeee!", "You removed them all!");
					if (pGameInfo->bPlayingMetagame) {
						PostMessage(WM_CLOSE, 0, 0);         // and post a program exit
					}
				}
			}
			if (m_nSweepCount == 0) {
				KillTimer(BEACON_TIMER);
				bPlaying = false;
				bNewGame = false;                           // Will be set to true in NewGame()
				if (pGameInfo->bSoundEffectsEnabled)
					sndPlaySound(LOSE_SOUND, SND_ASYNC);                 // Make a "you lost" noise
				CMessageBox GameOverDlg((CWnd *)this, pGamePalette, "Game over.", "Out of sweeps.");
				if (pGameInfo->bPlayingMetagame) {
					PostMessage(WM_CLOSE, 0, 0);         // and post a program exit
				}
			}
		}
		break;

	default:
		CFrameWnd ::OnTimer(nIDEvent);
		break;
	}

	ReleaseDC(pDC);

}

/**********************************************************
Other functions:
***********************************************************/

/*****************************************************************
 *
 *  MoveBeam
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Moves the lit beam one pie-slice clockwise
 *
 *  FORMAL PARAMETERS:
 *
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
void CMainWindow::MoveBeam() {
	CDC             *pDC;
	CPalette        *pOldPal = nullptr;                        // Pointer to old palette
	PALETTEENTRY    tempent[2];

	pDC = GetDC();
	pOldPal = (*pDC).SelectPalette(pGamePalette, false);     // Select in game palette

	tempent[1].peRed = 255;                                 // Make entry [1] white
	tempent[1].peGreen = 255;
	tempent[1].peBlue = 254;
	tempent[1].peFlags = PC_RESERVED;                       // Assign it the "changeable" flag

	(*pGamePalette).GetPaletteEntries(nCurrentIndex, 1, (LPPALETTEENTRY)&tempent[0]);    // Entry [0] is the chosen
	tempent[0].peFlags = PC_RESERVED;                                                   //...color & changeable

	if (nSlice == START_ENTRY) {
		(*pGamePalette).AnimatePalette(nSlice, 1, (LPPALETTEENTRY)&tempent[1]);
		(*pGamePalette).AnimatePalette(NUM_BEAMS + START_ENTRY - 1, 1,
		                               (LPPALETTEENTRY)&tempent[0]);
	} else
		(*pGamePalette).AnimatePalette(nSlice - 1, 2, (LPPALETTEENTRY)tempent);

	if (++nSlice >= (NUM_BEAMS + START_ENTRY)) {
		nSlice = START_ENTRY;
	}

	(*pDC).SelectPalette(pOldPal, false);           // Select back the old palette
	ReleaseDC(pDC);

} // end MoveBeam


/*****************************************************************
 *
 *  CheckUnderBeam
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Checks for beam-matching grid spaces currently under the beam
 *
 *  FORMAL PARAMETERS:
 *
 *
 *  IMPLICIT INPUT PARAMETERS:
 *
 *      int nSlice      Which pie-slice is currently 'lit' by the beam
 *
 *  IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 *  RETURN VALUE:
 *
 *      void

    0 deg is at 3 o'clock, and the slices are in order going clockwise
 *
 ****************************************************************/
void CMainWindow::CheckUnderBeam() {
	CDC     *pDC;
	CPoint  Start, End;
	int     radius;
	float   degrees, x, y, radians, endAngle;

	pDC = GetDC();

	Start.x = SIDE_BORDER + ART_WIDTH / 2;                              // Start at the center
	Start.y = TOP_BORDER + ART_HEIGHT / 2;

	radius = 330; //SQRT(ART_HEIGHT^2 + ART_WIDTH^2)
	if (nSlice == START_ENTRY) {
		degrees = (float)(NUM_BEAMS - 1) * (float)(360 / NUM_BEAMS);
		endAngle = (float)(NUM_BEAMS) * (float)(360 / NUM_BEAMS);
	} else {
		degrees = ((float)(nSlice - START_ENTRY - 1)) * (float)(360 / NUM_BEAMS);       // Get degrees cc-wise from 3 o'clock
		endAngle = ((float)(nSlice - START_ENTRY)) * (float)(360 / NUM_BEAMS);
	}

	while (degrees < endAngle) {
		radians = degrees * (float)0.017453292;                     // Convert to radians

		x = (float)cos(radians);
		y = (float)sin(radians);

		End.x = Start.x + (int)(x * radius);
		End.y = Start.y + (int)(y * radius);

		bChanged = true;
		LineDDA(Start.x, Start.y, End.x, End.y, StepAlongLine, pDC);

		degrees += 1.0F;
	} // end while

	if ((!m_bChangeAtTwelve) && (nCurrentIndex != nNextIndex))  // If we're changing colors on the fly
		nCurrentIndex = nNextIndex;                             //...switch to the new beacon color
	//...only at the end of a beam traversal
	ReleaseDC(pDC);
} // end CheckUnderBeam

void CALLBACK StepAlongLine(int xpos, int ypos, CDC *cdc) {
	CDC     *pDC = cdc;
	CPoint point;

	if (bChanged) {
		point.x = xpos;
		point.y = ypos;
		if (InArtRegion(point) && ((CMainWindow::UnderLighthouse(point) == false) &&
		                           CMainWindow::InPictureSquare(point))) {
			bChanged = CMainWindow::CompareColors(pDC, point);
		}
	}
}

/*****************************************************************
 *
 *  CompareColors
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Compares the color at point with the beam's current color
 *
 *  FORMAL PARAMETERS:
 *
 *
 *  IMPLICIT INPUT PARAMETERS:
 *
 *
 *  IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 *  RETURN VALUE:
 *
 *      bool    True if a grid space matched and was modified,
 *              False if the space did not match
 *
 ****************************************************************/
bool CMainWindow::CompareColors(CDC *pDC, CPoint point) {
	CPoint      square;
	COLORREF    test;
	byte        ar, ag, ab, br, bg, bb;
	PALETTEENTRY    tempent[1];
	unsigned int        index;

	test = (*pDC).GetPixel(point);

	ar = GetRValue(test);
	ag = GetGValue(test);
	ab = GetBValue(test);

	(*pGamePalette).GetPaletteEntries(nCurrentIndex, 1, (LPPALETTEENTRY)tempent);

	br = tempent[0].peRed;
	bg = tempent[0].peGreen;
	bb = tempent[0].peBlue;

	if ((ar == br) && ((ag == bg) && (ab == bb))) {          // Matches the beam
		square = PointToGrid(point);
		square.x = (square.x * GRID_WIDTH) + SIDE_BORDER;
		square.y = (square.y * GRID_HEIGHT) + TOP_BORDER;

		DrawPart(pDC, square, square, GRID_WIDTH, GRID_HEIGHT);
		DrawPart(pArtDC, square, square, GRID_WIDTH, GRID_HEIGHT);
		m_nRemoved++;
		square = PointToGrid(point);
		PictureGrid[square.x][square.y] = false;
		return true;
	}

	index = (*pGamePalette).GetNearestPaletteIndex(test);        // get the nearest index of test color
	if (((index < BUTTON_ENTRY) && (index >= START_ENTRY))       // if it's in the beam area
	        && (index != nSlice))                               // and doesn't match the beam's index
		return false;  //true );                                           //

	return false;
} // end CompareColors


/*****************************************************************
 *
 *  UnderLighthouse
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Checks to see if the point lies under the lighthouse area
 *
 *  FORMAL PARAMETERS:
 *
 *      CPoint point    point to check
 *
 *  IMPLICIT INPUT PARAMETERS:
 *
 *
 *  IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 *  RETURN VALUE:
 *
 *      bool    True if point lies under the lighthouse area,
 *              False if the point isn't
 *
 ****************************************************************/
bool CMainWindow::UnderLighthouse(CPoint point) {
	if ((point.x >= (SIDE_BORDER + ART_WIDTH / 2 - START_OFFSET_X)) &&
	        (point.x <= (SIDE_BORDER + ART_WIDTH / 2 + START_OFFSET_X))) {
		if ((point.y >= (TOP_BORDER + ART_HEIGHT / 2 - START_OFFSET_Y)) &&
		        (point.y <= (TOP_BORDER + ART_HEIGHT / 2 + (START_HEIGHT - START_OFFSET_Y)))) {
			return true;
		}
	}
	return false;
} // end CompareColors


/*****************************************************************
 *
 *  InPictureSquare()
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Checks to see if the point lies in a grid space filled with picture color
 *
 *  FORMAL PARAMETERS:
 *
 *      CPoint point    point to check
 *
 *  IMPLICIT INPUT PARAMETERS:
 *
 *
 *  IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 *  RETURN VALUE:
 *
 *      bool    True if point lies in a picture-filled grid space,
 *              False if the point doesn't
 *
 ****************************************************************/
bool CMainWindow::InPictureSquare(CPoint point) {
	CPoint  Grid;

	Grid = PointToGrid(point);

	return PictureGrid[Grid.x][Grid.y];
}


/*****************************************************************
 *
 *  PointToGrid()
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Finds the x & y gridspace for the given point
 *
 *  FORMAL PARAMETERS:
 *
 *      CPoint point    point to check
 *
 *  IMPLICIT INPUT PARAMETERS:
 *
 *
 *  IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 *  RETURN VALUE:
 *
 *      CPoint  NewPoint    The grid coords for the given point
 *
 ****************************************************************/
CPoint CMainWindow::PointToGrid(CPoint point) {
	CPoint  New;

	New.x = point.x - SIDE_BORDER;          // Put point into Art Space
	New.y = point.y - TOP_BORDER;

	New.x = (int) New.x / GRID_WIDTH;       // Get the grid space coord's
	New.y = (int) New.y / GRID_HEIGHT;

	return New;                             // Return the coord's
}


/*****************************************************************
 *
 *  LoadArtWork
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Loads the Artwork to the Screen
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
bool CMainWindow::LoadArtWork(CDC *pDC) {
//	CBitmap      *pNextArt = nullptr;
	CPalette    *pTempPalette = nullptr,
	             *pPalOld = nullptr;
	CPoint      Src;
	CRect       rcDest;                                             // Art work - frame bitmap dims
	CRect       rcDIB;                                              // Source area for Art Work bmp
	HDIB        hDIB;
	char        ArtName[MAX_FILE_LENGTH];

	int  nNumEntries;
	char chNumEntries[5];
	int i, pick = 1;

	ifstream inFile;
	inFile.open(DATA_FILE);                                  // open the data store
	if (inFile.fail()) {
		return false;
	}
	inFile.getline(chNumEntries, sizeof(chNumEntries));     // read number of names in file
	nNumEntries = atoi(chNumEntries);
	pick = nLastPick;
	assert(nNumEntries > 0);

	while (pick == nLastPick) {
		pick = (brand() % nNumEntries) + 1;
	}

	Common::strcpy_s(ArtName, "");
	for (i = 0; i < pick; i++) {
		if (!inFile.eof())
			inFile.getline(ArtName, sizeof(ArtName));       // load a name
	}
	inFile.close();                                         // close the data store
	nLastPick = pick;

	CDibDoc     *pSourceDoc;

	pSourceDoc = new CDibDoc();
	ASSERT(pSourceDoc != nullptr);
	if (pSourceDoc == nullptr) {
		return false;
	}

	char bufName[MAX_FILE_LENGTH + 10];
	Common::sprintf_s(bufName, ".\\ART\\%s", ArtName);

	(*pSourceDoc).OpenDocument(bufName);

	ASSERT(pTempPalette == nullptr);
	pTempPalette = (*pSourceDoc).DetachPalette();       // Acquire the shared palette for our game from the art

	pPalOld = (*pDC).SelectPalette(pGamePalette, false);                            // Select Game Palette
	(*pDC).RealizePalette();                                                        // Use it

	// Transfer the button colors to Game Palette
	LPPALETTEENTRY  ButtonEntry[NUM_BUTTONS];

	(*pTempPalette).GetPaletteEntries(BUTTON_ENTRY, NUM_BUTTONS, (LPPALETTEENTRY)ButtonEntry);
	(*pGamePalette).SetPaletteEntries(BUTTON_ENTRY, NUM_BUTTONS, (LPPALETTEENTRY)ButtonEntry);
	(*pDC).RealizePalette();

	hDIB = (*pSourceDoc).GetHDIB();

	if (hDIB) {
		rcDest.SetRect(SIDE_BORDER, TOP_BORDER, ART_WIDTH + SIDE_BORDER, ART_HEIGHT + TOP_BORDER);

		int cxDIB = (int) DIBWidth(hDIB);
		int cyDIB = (int) DIBHeight(hDIB);


		if (cxDIB > ART_WIDTH) {                     // Center and crop
			rcDIB.left = (cxDIB - ART_WIDTH) / 2;   //...too wide art
			while (rcDIB.left % 5 != 0) {
				rcDIB.left++;
			}
			rcDIB.right = rcDIB.left + ART_WIDTH;
		} else {                                    // Stretch too thin art
			rcDIB.left = 0;
			rcDIB.right = ART_WIDTH;    //cxDIB;
		}

		if (cyDIB > ART_HEIGHT) {                    // Center and crop
			rcDIB.top = (cyDIB - ART_HEIGHT) / 2;   //...too long art
			while (rcDIB.top % 5 != 0) {
				rcDIB.top++;
			}
			rcDIB.bottom = rcDIB.top + ART_HEIGHT;
		} else {                                    // Stretch too short art
			rcDIB.top = 0;
			rcDIB.bottom = ART_HEIGHT;  //cyDIB;
		}

		PaintDIB((*pDC).m_hDC, &rcDest, hDIB, &rcDIB, pGamePalette);

	}

	Src.x = SIDE_BORDER + (ART_WIDTH / 2) - START_OFFSET_X;
	Src.y = TOP_BORDER + (ART_HEIGHT / 2) - START_OFFSET_Y;
	DrawPart(pDC, Src, Src, START_WIDTH, START_HEIGHT);

	for (i = 0; i < NUM_BUTTONS; i++) {
		CBrush  *pMyBrush = nullptr;
		pMyBrush = new CBrush();                                // Construct new brush
		(*pMyBrush).CreateSolidBrush(PALETTEINDEX((uint16)(colorBlock[i].nColorIndex)));

		(*pArtDC).FillRect(colorBlock[i].rLocation, pMyBrush);

		MyFocusRect(pArtDC, colorBlock[i].rLocation, false);     // standard highlight on button

		pMyBrush->DeleteObject();
		delete pMyBrush;
		pMyBrush = nullptr;
	}

	pBeaconBitmap = FetchResourceBitmap(pDC, nullptr, IDB_BEACON_BMP);          // Add the lighthouse

	(*pDC).SelectPalette(pPalOld, false);

	pTempPalette->DeleteObject();
	delete pTempPalette;

	delete pSourceDoc;
	pSourceDoc = nullptr;

	return true;

} // end LoadArtWork()

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
void CMainWindow::DrawPart(CDC *pDC, CPoint Src, CPoint Dst, int nWidth, int nHeight) {

	pDC->BitBlt(Dst.x, Dst.y, nWidth, nHeight, pBackDC, Src.x, Src.y, SRCCOPY);

} //End DrawPart();

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
	if ((point.x > SIDE_BORDER && point.x < (ART_WIDTH + SIDE_BORDER)) &&         // See if point lies within
	        (point.y > TOP_BORDER && point.y < (ART_HEIGHT + TOP_BORDER)))        //...ArtWork area
		return true;                                                            // Return true if it's inside
	else return false;                                                          //...and false if not
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
void MyFocusRect(CDC *pDC, CRect rect, bool nPressed) {
	CBrush      *pMyBrush = nullptr;                   // New Brush
	CBrush      *pOldBrush = nullptr;                  // Pointer to old brush
	CPen        *pLitePen = nullptr;                       // New Pen
	CPen        *pDarkPen = nullptr;                   // Dark Pen
	CPen        *pOldPen = nullptr;                    // Pointer to old pen
	CPalette    *pPalOld = nullptr;                    // Pointer to old palette

	LOGBRUSH lb;                                    // log brush type
	lb.lbStyle = BS_HOLLOW;                         // Don't fill in area

	pMyBrush = new CBrush();                        // Construct new brush
	pMyBrush->CreateBrushIndirect(&lb);              // Create a new brush

	pDarkPen = new CPen();                          // Construct new pen
	pLitePen = new CPen();                          // Construct new pen
	pLitePen->CreatePen(PS_INSIDEFRAME, HILITE_BORDER,
	                    PALETTEINDEX((uint16)(LITE_TRIM)));        // Create a new pen

	pDarkPen->CreatePen(PS_INSIDEFRAME, HILITE_BORDER,
	                    PALETTEINDEX((uint16)(DARK_TRIM)));        // Create a new pen

	pPalOld = (*pDC).SelectPalette(pGamePalette, false);     // Select in game palette
	(*pDC).RealizePalette();                                // Use it
	pOldBrush = pDC->SelectObject(pMyBrush);     // Select the new brush & save old

	if (nPressed)
		pOldPen = pDC->SelectObject(pDarkPen);   // Select the new pen & save old
	else
		pOldPen = pDC->SelectObject(pLitePen);   // Select the new pen & save old

	pDC->MoveTo(rect.TopLeft().x + HILITE_BORDER / 2, rect.BottomRight().y - HILITE_BORDER / 2);
	pDC->LineTo(rect.TopLeft().x + HILITE_BORDER / 2, rect.TopLeft().y + HILITE_BORDER / 2);
	pDC->LineTo(rect.BottomRight().x - HILITE_BORDER / 2, rect.TopLeft().y + HILITE_BORDER / 2);

	if (nPressed)
		pDC->SelectObject(pLitePen);             // Select the new pen & save old
	else
		pDC->SelectObject(pDarkPen);             // Select the new pen & save old

	pDC->LineTo(rect.BottomRight().x - HILITE_BORDER / 2, rect.BottomRight().y - HILITE_BORDER / 2);
	pDC->LineTo(rect.TopLeft().x + HILITE_BORDER / 2, rect.BottomRight().y - HILITE_BORDER / 2);

	pDC->SelectObject(pOldPen);                  // Select the old pen
	pDC->SelectObject(pOldBrush);                // Select the old brush

	(*pDC).SelectPalette(pPalOld, false);           // Select back the old palette

	pMyBrush->DeleteObject();
	delete pMyBrush;
	pMyBrush = nullptr;

	pLitePen->DeleteObject();
	delete pLitePen;
	pLitePen = nullptr;

	pDarkPen->DeleteObject();
	delete pDarkPen;
	pDarkPen = nullptr;

} // End MyFocusRect()

/*****************************************************************
 *
 *  NewGame
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
 *
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

void CMainWindow::NewGame() {
	CDC         *pDC;
	CPalette    *pOldPal = nullptr;
	CBrush      *pBrushNew = nullptr,
	             *pBrushOld = nullptr;
	int         i, j;

	for (i = 0; i < NUM_COLUMNS; i++) {
		for (j = 0; j < NUM_ROWS; j++) {
			PictureGrid[i][j] = true;                               // Assume they all start with picture in 'em
		}
	}

	pDC = GetDC();
	pOldPal = pDC->SelectPalette(pGamePalette, false);           // select the game palette
	pDC->RealizePalette();                                          //...and realize it

	pBrushNew = new CBrush();                                       // Construct a new brush object
	if (pBrushNew != nullptr) {                                         // If the constructor was successful:
		pBrushNew->CreateSolidBrush(PALETTERGB(0, 0, 0));         // Create my backdrop color brush
		pBrushOld = (*pDC).SelectObject(pBrushNew);                  // Select into the DC my new brush
		(*pDC).Rectangle(SIDE_BORDER, TOP_BORDER, ART_WIDTH + SIDE_BORDER, ART_HEIGHT + TOP_BORDER);
		(*pDC).SelectObject(pBrushOld);                          // Select in the old brush
		(*pDC).SelectPalette(pOldPal, false);                        // Select in the old palette
		delete pBrushNew;                                           // Delete the new brush
	}

	LoadArtWork(pArtDC);                                                     // Get a new picture

	SetBeamEntries(pDC);

	pOldPal = pDC->SelectPalette(pGamePalette, false);                   // select the game palette
	pDC->RealizePalette();                                                  //...and realize it

	pDC->BitBlt(SIDE_BORDER, TOP_BORDER, GAME_WIDTH - (2 * SIDE_BORDER), GAME_HEIGHT - (2 * BOTTOM_BORDER),
	            pArtDC, SIDE_BORDER, TOP_BORDER, SRCCOPY);              // Put it on-screen

	PaintMaskedBitmap(pDC, pGamePalette, pBeaconBitmap,
	                  SIDE_BORDER + (ART_WIDTH / 2) - LTHOUSE_OFFSET_X,
	                  TOP_BORDER + (ART_HEIGHT / 2) - LTHOUSE_OFFSET_Y);

	nSlice = CHANGE_BEAM;                                                   // Start at 12 o'clock
	m_nRemoved = 0;
	m_Score = 0;

	if (pGameInfo->bPlayingMetagame) {
		switch (pGameInfo->nSkillLevel) {
		case SKILLLEVEL_LOW:
			m_nSweeps = 20;
			m_nSpeed = MIN_SPEED + 1;                       // middle speed = 1
			break;
		case SKILLLEVEL_MEDIUM:
			m_nSweeps = 18;
			m_nSpeed = MAX_SPEED;
			break;
		case SKILLLEVEL_HIGH:
		default:
			m_nSweeps = 15;
			m_nSpeed = MAX_SPEED;
			break;
		} //end switch
	} else {
		m_nSweeps = tempSweeps;
		m_nSpeed = tempSpeed;
	}

	if (m_nSweeps == MAX_SWEEPS)
		m_nSweepCount = 0;
	else
		m_nSweepCount = m_nSweeps;

	if (m_bAutomatic) {
		nCurrentIndex = BUTTON_ENTRY;
		nNextIndex = BUTTON_ENTRY;
		SetTimer(BEACON_TIMER, SPEED_BASE - (m_nSpeed * SPEED_STEP), nullptr);
		MyFocusRect(pDC, colorBlock[nNextIndex - BUTTON_ENTRY].rLocation, true);     // draw hightlight on nNextIndex
	}

	(*pDC).SelectPalette(pOldPal, false);
	ReleaseDC(pDC);

	bNewGame = true;

} // End NewGame()


void CMainWindow::SetBeamEntries(CDC *pDC) {
	PALETTEENTRY    entry[NUM_BEAMS];
	int i;

	pDC->SelectPalette(pGamePalette, false);             // select the game palette
	pDC->RealizePalette();                              //...and realize it

	for (i = 0; i < (NUM_BEAMS); i++) {                  // Set palette area to animate for beacon  + NUM_BUTTONS
		entry[i].peRed = 0;//i*2;                       //...light and button colors to black, and
		entry[i].peGreen = 0;                           //...make peFlags Reserved, so they can
		entry[i].peBlue = 1;//i*2;                      //...be changed later
		entry[i].peFlags = PC_RESERVED;
	}

	(*pGamePalette).SetPaletteEntries(START_ENTRY, NUM_BEAMS, (LPPALETTEENTRY)entry);
	pDC->RealizePalette();                              //...and realize it

} // End SetBeamEntries


void CMainWindow::OnClose() {
	CDC     *pDC;
	CBrush  myBrush;
	CRect   myRect;

	KillTimer(BEACON_TIMER);

	pDC = GetDC();
	myRect.SetRect(0, 0, GAME_WIDTH, GAME_HEIGHT);
	myBrush.CreateStockObject(BLACK_BRUSH);
	(*pDC).FillRect(&myRect, &myBrush);
	ReleaseDC(pDC);

	sndPlaySound(nullptr, SND_ASYNC);                                   // kill any wav that's playing

	m_Score = ((float) m_nRemoved / (float) m_nTotalSquares) * 100;     // Score is the percentage
	if (pGameInfo->bPlayingMetagame)
		pGameInfo->lScore = (long)(m_Score + 0.5);

	if (pGameSound != nullptr) {
		delete pGameSound;
		pGameSound = nullptr;
	}

	if (m_pScoreTxt != nullptr)
		delete m_pScoreTxt;
	if (m_pSweepTxt != nullptr)
		delete m_pSweepTxt;

	if (pScrollUp != nullptr) {
		pScrollUp->DeleteObject();
		pScrollUp = nullptr;
	}

	if (pScrollDown != nullptr) {
		pScrollDown->DeleteObject();
		pScrollDown = nullptr;
	}

	if (m_pScrollButton != nullptr)
		delete m_pScrollButton;

	if (pBeaconBitmap != nullptr) {
		pBeaconBitmap->DeleteObject();
		delete pBeaconBitmap;
	}

	if (pOldBmp1 != nullptr)                           // Get rid of Source
		pBackDC->SelectObject(pOldBmp1);
	if (pOldPal1 != nullptr)
		pBackDC->SelectPalette(pOldPal1, false);
	if (pBackDC->m_hDC != nullptr) {
		pBackDC->DeleteDC();
		delete pBackDC;
	}

	pBackBitmap->DeleteObject();
	delete pBackBitmap;

	if (pOldBmp2 != nullptr)                           // Get rid of Source
		pArtDC->SelectObject(pOldBmp2);
	if (pOldPal2 != nullptr)
		pArtDC->SelectPalette(pOldPal2, false);
	if (pArtDC->m_hDC != nullptr) {
		pArtDC->DeleteDC();
		delete pArtDC;
	}

	pArtBitmap->DeleteObject();
	delete pArtBitmap;

	if (pGamePalette != nullptr) {
		pGamePalette->DeleteObject();
		delete pGamePalette;
	}

	CFrameWnd::OnClose();

	MFC::PostMessage(ghParentWnd, WM_PARENTNOTIFY, WM_DESTROY, 0L);
}

void CALLBACK GetSubOptions(CWnd* pParentWind) {
	COptnDlg OptionsDlg(pParentWind, pGamePalette);      // Call Specific Game

	m_Score = ((float) m_nRemoved / (float) m_nTotalSquares) * 100;
	OptionsDlg.m_nSweeps = m_nSweeps;
	OptionsDlg.m_bAutomatic = m_bAutomatic;
	OptionsDlg.m_bChangeAtTwelve = m_bChangeAtTwelve;
	OptionsDlg.m_nSpeed = m_nSpeed;

	if (OptionsDlg.DoModal() == IDOK) {                      // save values set in dialog box
		tempSweeps = OptionsDlg.m_nSweeps;                  // sweeps takes effect on new game
		m_bAutomatic = OptionsDlg.m_bAutomatic;
		m_bChangeAtTwelve = OptionsDlg.m_bChangeAtTwelve;   // change point takes effect immediately
		tempSpeed = OptionsDlg.m_nSpeed;
		m_nSpeed = tempSpeed;                               // speed takes effect immediately
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
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_MESSAGE(MM_MCINOTIFY, CMainWindow::OnMCINotify)
	ON_MESSAGE(MM_WOM_DONE, CMainWindow::OnMMIONotify)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

} // namespace Beacon
} // namespace HodjNPodj
} // namespace Bagel
