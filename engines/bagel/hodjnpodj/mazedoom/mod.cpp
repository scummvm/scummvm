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
#include "bagel/hodjnpodj/hnplibs/sprite.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/hodjnpodj/hnplibs/mainmenu.h"
#include "bagel/hodjnpodj/hnplibs/cmessbox.h"
#include "bagel/hodjnpodj/hnplibs/rules.h"
#include "bagel/hodjnpodj/hnplibs/gamedll.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/mazedoom/resource.h"
#include "bagel/hodjnpodj/mazedoom/globals.h"
#include "bagel/hodjnpodj/mazedoom/mod.h"
#include "bagel/hodjnpodj/mazedoom/mazegen.h"
#include "bagel/hodjnpodj/mazedoom/optndlg.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {
namespace MazeDoom {

//
// bitmap locations in modparts.bmp
//
#define WALL_X      0
#define WALL_Y      22
#define PATH_WIDTH  24
#define PATH_HEIGHT 24
#define PATH_X      48
#define START_X     24
#define EDGE_Y      46
#define EDGE_WIDTH  5
#define EDGE_HEIGHT 24
#define TRAP_WIDTH  22
#define TRAP_HEIGHT 22

void CALLBACK GetSubOptions(CWnd* pParentWind);
void initialize_maze();                             // draw the surrounding wall and start/end squares
void create_maze();                                 // create a maze layout given the intiialized maze
int choose_door();                                  // pick a new path
int backup();                                       // back up a move
void SetUpMaze();
void SetInvisibleWalls();
void SetTraps();
void AddEdges(CDC *pDC, int x, int y, int offset_x, int offset_y);
void PaintMaze(CDC *pDC);
CPoint GetRandomPoint(bool bRight);
CPoint ScreenToTile(CPoint pointScreen);
bool InArtRegion(CPoint point);

CBmpButton  *m_pScrollButton;
CSprite     *_playerSprite = nullptr;
CPalette    *pGamePalette = nullptr,                   // Palette of current artwork
	*pOldPal = nullptr;
CBitmap     *pMazeBitmap = nullptr,
	*pOldBmp = nullptr,
	*_wallBitmap = nullptr,
	*_pathBitmap = nullptr,
	*_startBitmap = nullptr,
	*_topEdgeBmp = nullptr,
	*_rightEdgeBmp = nullptr,
	*_bottomEdgeBmp = nullptr,
	*_leftEdgeBmp = nullptr,
	*_trapBitmap[NUM_TRAP_MAPS];
CDC         *pMazeDC = nullptr;                    // DC for the MazeBitmap
CText       *m_pTimeText = nullptr;                // Time to be posted in Locale box of screen
CBitmap     *_localeBitmap = nullptr,              // Locale of game bitmap for title bar
             *_blankBitmap = nullptr;               // Blank area of locale for time display

bool        _success;
bool        m_bIgnoreScrollClick;
bool        _playing;
bool        _gameOver = false;
POINT       _playerPos;
unsigned int        m_nPlayerID = PODJ;         // Hodj = 0, Podj = 4 to Offset the Bitmap ID for player

int         _difficulty;
int         _time,
            _seconds,
            _minutes;
int         tempDifficulty;
int         tempTime;

struct  TILE {          // Data type for each square of the underlying Grid of the Maze
	POINT   m_nStart;   // Upper-left-hand corner where the bmp is to be drawn (24 X 24)
	unsigned int    m_nWall;    // 0 = Path, 1 = Wall, 2 = Trap, etc...
	unsigned int    m_nTrap;    // Index of trap bitmap to use for drawing
	POINT   m_nDest;    // x,y Tile location of Trap exit point
	bool    m_bHidden;  // 0 = Visible, 1 = Invisible
} _mazeTile[NUM_COLUMNS][NUM_ROWS];

static CSound   *_gameSound = nullptr;                             // Game theme song

extern  LPGAMESTRUCT pGameInfo;
extern HWND ghParentWnd;

/////////////////////////////////////////////////////////////////////////////

// CMainWindow constructor:
// Create the window with the appropriate style, size, menu, etc.;
// it will be later revealed by CTheApp::InitInstance().  Then
// create our splash screen object by opening and loading its DIB.
//
CMainWindow::CMainWindow() {
	CString WndClass;
	CRect MainRect, tmpRect;
	CBitmap *pPartsBitmap = nullptr;
	CDC *pDC = nullptr;
	int i;	// Counter for trap assignment

	BeginWaitCursor();

	m_pScrollButton = nullptr;
	_playerSprite = nullptr;
	pGamePalette = nullptr,                   // Palette of current artwork
	pOldPal = nullptr;
	pMazeBitmap = nullptr;
	pOldBmp = nullptr;
	_wallBitmap = nullptr;
	_pathBitmap = nullptr;
	_startBitmap = nullptr;
	_topEdgeBmp = nullptr;
	_rightEdgeBmp = nullptr;
	_bottomEdgeBmp = nullptr;
	_leftEdgeBmp = nullptr;
	pMazeDC = nullptr;
	m_pTimeText = nullptr;
	_localeBitmap = nullptr;
	_blankBitmap = nullptr;
	_success = false;
	m_bIgnoreScrollClick = false;
	_playing = false;
	_gameOver = false;
	_playerPos.x = _playerPos.y = 0;
	m_nPlayerID = PODJ;
	_difficulty = 0;
	_time = _seconds = _minutes = 0;
	tempDifficulty = 0;
	tempTime = 0;
	Common::fill(_trapBitmap, _trapBitmap + NUM_TRAP_MAPS, nullptr);

	if (pGameInfo->bPlayingHodj)
		m_nPlayerID = HODJ;
	else
		m_nPlayerID = PODJ;

	// Define a special window class which traps double-clicks, is byte aligned
	// to maximize BITBLT performance, and creates "owned" DCs rather than sharing
	// the five system defined DCs which are not guaranteed to be available;
	// this adds a bit to our app size but avoids hangs/freezes/lockups.
	WndClass = AfxRegisterWndClass(CS_BYTEALIGNWINDOW | CS_OWNDC,
	    nullptr, nullptr, nullptr);

	// Center our window on the screen
	pDC = GetDC();
	MainRect.left = (pDC->GetDeviceCaps(HORZRES) - GAME_WIDTH) >> 1;
	MainRect.top = (pDC->GetDeviceCaps(VERTRES) - GAME_HEIGHT) >> 1;
	MainRect.right = MainRect.left + GAME_WIDTH;
	MainRect.bottom = MainRect.top + GAME_HEIGHT;

	// Create the window as a POPUP so no boarders, title, or menu are present;
	// this is because the game's background art will fill the entire 640x480 area.
	Create(WndClass, "Boffo Games -- Maze o' Doom", WS_POPUP, MainRect, nullptr, 0);

	CDibDoc *pSourceDoc;                // Get the game palette
	pSourceDoc = new CDibDoc();
	ASSERT(pSourceDoc != nullptr);
	(*pSourceDoc).OpenDocument(MAINSCREEN);
	pGamePalette = (*pSourceDoc).DetachPalette();       // Acquire the shared palette for our game from the art
	delete pSourceDoc;

	pDC->SelectPalette(pGamePalette, false);            // select the game palette
	pDC->RealizePalette();                              //...and realize it

	ShowWindow(SW_SHOWNORMAL);
	SplashScreen();

	// Build Scroll Command button
	m_pScrollButton = new CBmpButton;
	ASSERT(m_pScrollButton != nullptr);
	tmpRect.SetRect(SCROLL_BUTTON_X, SCROLL_BUTTON_Y,
	                SCROLL_BUTTON_X + SCROLL_BUTTON_DX - 1,
	                SCROLL_BUTTON_Y + SCROLL_BUTTON_DY - 1);
	_success = (*m_pScrollButton).Create(nullptr, BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, tmpRect, this, IDC_SCROLL);
	ASSERT(_success);
	_success = (*m_pScrollButton).LoadBitmaps(SCROLLUP, SCROLLDOWN, 0, 0);
	ASSERT(_success);
	m_bIgnoreScrollClick = false;

	pMazeBitmap = new CBitmap();
	pMazeDC = new CDC();

	pMazeBitmap->CreateCompatibleBitmap(pDC, NUM_COLUMNS * SQ_SIZE_X, NUM_ROWS * SQ_SIZE_Y);    // Set up MazeBitmap
	pMazeDC->CreateCompatibleDC(pDC);                                   //...and DC
	pOldBmp = pMazeDC->SelectObject(pMazeBitmap);                       // select the bitmap in
	pOldPal = pMazeDC->SelectPalette(pGamePalette, false);              // select the game palette
	pMazeDC->RealizePalette();                                          //...and realize it

//
// Load up the various bitmaps for wall, edge, booby traps, etc.
//
	pPartsBitmap = FetchResourceBitmap(pDC, nullptr, IDB_PARTS);

	_wallBitmap = ExtractBitmap(pDC, pPartsBitmap, pGamePalette, WALL_X, WALL_Y, PATH_WIDTH, PATH_HEIGHT);
	_pathBitmap = ExtractBitmap(pDC, pPartsBitmap, pGamePalette, PATH_X, WALL_Y, PATH_WIDTH, PATH_HEIGHT);
	_startBitmap = ExtractBitmap(pDC, pPartsBitmap, pGamePalette, START_X, WALL_Y, PATH_WIDTH, PATH_HEIGHT);

	_leftEdgeBmp = ExtractBitmap(pDC, pPartsBitmap, pGamePalette,
	                             0, EDGE_Y, EDGE_WIDTH, EDGE_HEIGHT);
	_rightEdgeBmp = ExtractBitmap(pDC, pPartsBitmap, pGamePalette,
	                              EDGE_WIDTH, EDGE_Y, EDGE_WIDTH, EDGE_HEIGHT);
	_topEdgeBmp = ExtractBitmap(pDC, pPartsBitmap, pGamePalette,
	                            EDGE_WIDTH * 2, EDGE_Y, EDGE_HEIGHT, EDGE_WIDTH);
	_bottomEdgeBmp = ExtractBitmap(pDC, pPartsBitmap, pGamePalette,
	                               (EDGE_WIDTH * 2) + EDGE_HEIGHT, EDGE_Y, EDGE_HEIGHT, EDGE_WIDTH);

	for (i = 0; i < NUM_TRAP_MAPS; i++) {
		_trapBitmap[i] = ExtractBitmap(pDC, pPartsBitmap, pGamePalette,
		                               TRAP_WIDTH * i, 0, TRAP_WIDTH, TRAP_HEIGHT);
	}

	if (pPartsBitmap != nullptr) {
		pPartsBitmap->DeleteObject();
		delete pPartsBitmap;
	}

	_playerSprite = new CSprite;
	(*_playerSprite).SharePalette(pGamePalette);
	_success = (*_playerSprite).LoadResourceCels(pDC, IDB_HODJ_LEFT + m_nPlayerID, NUM_CELS);
	ASSERT(_success);
	(*_playerSprite).SetMasked(true);
	(*_playerSprite).SetMobile(true);

	_localeBitmap = FetchResourceBitmap(pDC, nullptr, "IDB_LOCALE_BMP");
	ASSERT(_localeBitmap != nullptr);
	_blankBitmap = FetchResourceBitmap(pDC, nullptr, "IDB_BLANK_BMP");
	ASSERT(_blankBitmap != nullptr);

	tmpRect.SetRect(TIME_LOCATION_X, TIME_LOCATION_Y,
	                TIME_LOCATION_X + TIME_WIDTH, TIME_LOCATION_Y + TIME_HEIGHT);
	if ((m_pTimeText = new CText()) != nullptr) {
		(*m_pTimeText).SetupText(pDC, pGamePalette, &tmpRect, JUSTIFY_CENTER);
	}

	ReleaseDC(pDC);

	//srand((unsigned) time( nullptr ));         // seed the random number generator

	if (pGameInfo->bPlayingMetagame) {
		if (pGameInfo->nSkillLevel == SKILLLEVEL_LOW) {
			_difficulty = MIN_DIFFICULTY;              // Total Wussy
			_time = 60;
		} else if (pGameInfo->nSkillLevel == SKILLLEVEL_MEDIUM) {
			_difficulty = 2;              // Big Sissy
			_time = 60;
		} else { //if (pGameInfo->nSkillLevel == SKILLLEVEL_HIGH)
			_difficulty = 4;              // Minor Whimp
			_time = 60;
		}
	} // end if
	else {                                      // Use Defaults
		_difficulty = 6;                      // Miner
		_time = 180;
	} // end else

	tempDifficulty = _difficulty;
	tempTime = _time;
	_seconds = _time % 60;
	_minutes = _time / 60;
	initialize_maze();         // draw the surrounding wall and start/end squares
	create_maze();             // create a maze layout given the intiialized maze
	SetUpMaze();                // "translate" from the created maze into uniform grid of doom
	PaintMaze(pMazeDC);         // draw it in the MazeBitmap
	_playing = true;
	SetTimer(GAME_TIMER, CLICK_TIME, nullptr);     // Reset ticker

	if (pGameInfo->bMusicEnabled) {
		_gameSound = new CSound(this, GAME_THEME, SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END);
		if (_gameSound != nullptr) {
			(*_gameSound).midiLoopPlaySegment(3000, 32980, 0, FMT_MILLISEC);
		} // end if pGameSound
	}

	EndWaitCursor();

	if (!pGameInfo->bPlayingMetagame)
		PostMessage(WM_COMMAND, IDC_SCROLL, BN_CLICKED);        // Activate the Options dialog

} //End of CMainWindow


// OnPaint:
// This is called whenever Windows sends a WM_PAINT message.
// Note that creating a CPaintDC automatically does a BeginPaint and
// an EndPaint call is done when it is destroyed at the end of this
// function.  CPaintDC's constructor needs the window (this).
//
void CMainWindow::OnPaint() {
	PAINTSTRUCT lpPaint;

	InvalidateRect(nullptr, false);                // invalidate the entire window
	BeginPaint(&lpPaint);
	SplashScreen();
	EndPaint(&lpPaint);
}

// Paint the background art (splash screen) in the client area;
// called by both OnPaint and InitInstance.
void CMainWindow::SplashScreen() {
	CRect   rcDest;
	CRect   rcDIB;
	CDC     *pDC;
	CPalette    *pPalOld = nullptr;                                                    // Old palette holder
	CDibDoc myDoc;
	HDIB    hDIB;
	char    msg[64];

	pDC = GetDC();

	myDoc.OpenDocument(MAINSCREEN);
	pPalOld = (*pDC).SelectPalette(pGamePalette, false);                            // Select Game Palette
	pDC->RealizePalette();                              // Realize the palette to prevent palette shifting

	hDIB = myDoc.GetHDIB();

	if (pMazeDC && hDIB) {
		GetClientRect(rcDest);

		int cxDIB = (int) DIBWidth(hDIB);
		int cyDIB = (int) DIBHeight(hDIB);

		rcDIB.top = rcDIB.left = 0;
		rcDIB.right = cxDIB;
		rcDIB.bottom = cyDIB;
		PaintDIB((*pDC).m_hDC, &rcDest, hDIB, &rcDIB, pGamePalette);
		pDC->BitBlt(SIDE_BORDER, TOP_BORDER, ART_WIDTH, ART_HEIGHT, pMazeDC, 0, SQ_SIZE_Y / 2, SRCCOPY);    // Draw Maze
		if ((_playerSprite != nullptr) && _playing)
			(*_playerSprite).PaintSprite(pDC, (_playerPos.x * SQ_SIZE_X) + SIDE_BORDER,
			                             (_playerPos.y * SQ_SIZE_Y) + TOP_BORDER - SQ_SIZE_Y / 2); // Update PLAYER
	}

	if (_playing) {                         // only false when the options are displayed
		PaintBitmap(pDC, pGamePalette, _blankBitmap, TIME_LOCATION_X, TIME_LOCATION_Y);
		if (_time == 0)
			Common::sprintf_s(msg, "Time Used: %02d:%02d", _minutes, _seconds);
		else {
			Common::sprintf_s(msg, "Time Left: %02d:%02d", _minutes, _seconds);
		}
		(*m_pTimeText).DisplayString(pDC, msg, 16, FW_SEMIBOLD, OPTIONS_COLOR);
	} else {
		if (_localeBitmap != nullptr)
			PaintBitmap(pDC, pGamePalette, _localeBitmap, TIME_LOCATION_X, TIME_LOCATION_Y);
	}

	(*pDC).SelectPalette(pPalOld, false);                                           // Select back old palette
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

	if (HIWORD(lParam) == BN_CLICKED) {

		CDC *pDC;
		CRules  RulesDlg((CWnd *)this, RULES_TEXT, pGamePalette,
		                 pGameInfo->bSoundEffectsEnabled ? RULES_WAV : nullptr);
		CMainMenu COptionsWind((CWnd *)this, pGamePalette,
		                       pGameInfo->bPlayingMetagame ? (NO_NEWGAME | NO_OPTIONS) : 0,
		                       GetSubOptions, RULES_TEXT,
		                       pGameInfo->bSoundEffectsEnabled ? RULES_WAV : nullptr, pGameInfo) ;        // Construct option dialog

		pDC = GetDC();
		PaintBitmap(pDC, pGamePalette, _localeBitmap, TIME_LOCATION_X, TIME_LOCATION_Y);

		switch (wParam) {

		case IDC_RULES:
			KillTimer(GAME_TIMER);
			CSound::waitWaveSounds();
			m_bIgnoreScrollClick = true;
			(*m_pScrollButton).SendMessage(BM_SETSTATE, true, 0L);

			RulesDlg.DoModal();
			m_bIgnoreScrollClick = false;
			(*m_pScrollButton).SendMessage(BM_SETSTATE, false, 0L);
			if (!_gameOver)
				SetTimer(GAME_TIMER, CLICK_TIME, nullptr);                     // Reset ticker
			break;

		case IDC_SCROLL:
			KillTimer(GAME_TIMER);
			if (m_bIgnoreScrollClick) {
				(*m_pScrollButton).SendMessage(BM_SETSTATE, true, 0L);
				break;
			}

			m_bIgnoreScrollClick = true;
			(*m_pScrollButton).SendMessage(BM_SETSTATE, true, 0L);
			SendDlgItemMessage(IDC_SCROLL, BM_SETSTATE, true, 0L);

			CSound::waitWaveSounds();

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
				if (!_gameOver)
					SetTimer(GAME_TIMER, CLICK_TIME, nullptr);     // Reset ticker
				break;

			case IDC_OPTIONS_QUIT:                      // Quit button was clicked
				if (pGameInfo->bPlayingMetagame)
					pGameInfo->lScore = 0;
				PostMessage(WM_CLOSE, 0, 0);            // and post a program exit
				ReleaseDC(pDC);
				return false;

			} //end switch(ComDlg.DoModal())

			if (!pGameInfo->bMusicEnabled && (_gameSound != nullptr)) {

				_gameSound->stop();
				delete _gameSound;
				_gameSound = nullptr;

			} else if (pGameInfo->bMusicEnabled && (_gameSound == nullptr)) {

				if ((_gameSound = new CSound) != nullptr) {
					_gameSound->initialize(this, GAME_THEME, SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END);
					_gameSound->midiLoopPlaySegment(3000, 32980, 0, FMT_MILLISEC);
				}
			}

			m_bIgnoreScrollClick = false;
			(*m_pScrollButton).SendMessage(BM_SETSTATE, false, 0L);
			break;
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

	rectTitle.SetRect(NEWGAME_LOCATION_X, NEWGAME_LOCATION_Y,
	                  NEWGAME_LOCATION_X + NEWGAME_WIDTH,
	                  NEWGAME_LOCATION_Y + NEWGAME_HEIGHT);

	if (rectTitle.PtInRect(point) && (pGameInfo->bPlayingMetagame == false))
		NewGame();                              // Activate New Game

	if (_playing && InArtRegion(point)) {
		MovePlayer(point);
	}

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
	if (InArtRegion(point) && _playing && !_gameOver) {     // If the cursor is within the border
		GetNewCursor();                                     //...and we're playing, update the cursor
		if (nFlags & MK_LBUTTON) {                          // If the Left mouse button is down,
			MovePlayer(point);                              //...have the player follow the mouse
		}
	} else SetCursor(LoadCursor(nullptr, IDC_ARROW));          // Refresh cursor object to arrow
	//...when outside the maze area
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

	CFrameWnd ::OnRButtonDown(nFlags, point);

} // End OnRButtonDown

// OnChar and OnSysChar
// These functions are called when keyboard input generates a character.
//

void CMainWindow::OnChar(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	CFrameWnd ::OnChar(nChar, nRepCnt, nFlags);     // default action
}

void CMainWindow::OnSysChar(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	if ((nChar == 'q') && (nFlags & 0x2000)) {      // terminate app on ALT-q
		if (pGameInfo->bPlayingMetagame)
			pGameInfo->lScore = 0;
		PostMessage(WM_CLOSE, 0, 0);                 // *** remove later ***
	} else
		CFrameWnd::OnChar(nChar, nRepCnt, nFlags);  // default action
}

void CMainWindow::OnSysKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
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
}

void CMainWindow::OnKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	CPoint  NewPosition;
	NewPosition = (*_playerSprite).GetPosition();

	switch (nChar) {
	case VK_F1:                                             // F1 key is hit
		SendMessage(WM_COMMAND, IDC_RULES, BN_CLICKED);     // Activate the Rules dialog
		break;

	case VK_F2:                                             // F2 key is hit
		SendMessage(WM_COMMAND, IDC_SCROLL, BN_CLICKED);    // Activate the Options dialog
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
	CDC     *pDC;
	CSound  *pEffect = nullptr;
	char    msg[64];

	pDC = GetDC();

	switch (nIDEvent) {

	case GAME_TIMER:
		if (_time == 0) {                         // No time limit, increment
			_seconds++;
			if (_seconds == 60) {
				_minutes++;
				_seconds = 0;
			}
		}

		else {                                      // Count down time left
			if (_seconds == 0 && _minutes != 0) {
				_minutes--;
				_seconds = 60;
			}
			_seconds--;
		}

		if (_time == 0)
			Common::sprintf_s(msg, "Time Used: %02d:%02d", _minutes, _seconds);
		else {
			Common::sprintf_s(msg, "Time Left: %02d:%02d", _minutes, _seconds);
		}
		(*m_pTimeText).DisplayString(pDC, msg, 16, FW_SEMIBOLD, OPTIONS_COLOR);

		if (_minutes == 0 && _seconds == 0) {           // No time left on the clock!!
			KillTimer(nIDEvent);                        // Stop the Display timer
			_playing = false;
			_gameOver = true;
			if (pGameInfo->bSoundEffectsEnabled) {
				pEffect = new CSound((CWnd *)this, LOSE_SOUND,
				                     SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE);  //...Wave file, to delete itself
				(*pEffect).play();                                                      //...play the narration
			}
			MSG lpmsg;
			while (PeekMessage(&lpmsg, m_hWnd, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE)) ;

			CMessageBox GameOverDlg((CWnd *)this, pGamePalette, "Game over.", "Time ran out!");
			CSound::waitWaveSounds();
			if (pGameInfo->bPlayingMetagame) {
				pGameInfo->lScore = 0;
				PostMessage(WM_CLOSE, 0, 0);            // and post a program exit
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
	CDC     *pDC;
	char    msg[64];

	pDC = GetDC();

	_time = tempTime;;                            // get new time limit,
	_difficulty = tempDifficulty;                 //...new Difficulty

	if (_playerSprite != nullptr)                      // Refresh PLAYER
		(*_playerSprite).EraseSprite(pDC);          // Erase PlayerSprite

	if (_time != 0) {                             // If we've got a time limit
		_minutes = _time / 60;                    //...get the minutes and seconds
		_seconds = _time % 60;
	} else {
		_minutes = 0;
		_seconds = 0;
	}

	initialize_maze();                              // draw the surrounding wall and start/end squares
	create_maze();                                  // create a maze layout given the intiialized maze
	SetUpMaze();                                    // translate maze data to grid layout for display
	PaintMaze(pMazeDC);                             // paint that sucker to the offscreen bitmap
	pDC->BitBlt(SIDE_BORDER, TOP_BORDER, ART_WIDTH, ART_HEIGHT, pMazeDC, 0, SQ_SIZE_Y / 2, SRCCOPY);    // Draw Maze
	if (_playerSprite != nullptr)
		(*_playerSprite).PaintSprite(pDC, (_playerPos.x * SQ_SIZE_X) + SIDE_BORDER,
		                             (_playerPos.y * SQ_SIZE_Y) + TOP_BORDER - SQ_SIZE_Y / 2); // Display PLAYER
	_playing = true;                                // Game is started
	_gameOver = false;

	PaintBitmap(pDC, pGamePalette, _blankBitmap, TIME_LOCATION_X, TIME_LOCATION_Y);
	if (_time == 0)
		Common::sprintf_s(msg, "Time Used: %02d:%02d", _minutes, _seconds);
	else {
		Common::sprintf_s(msg, "Time Left: %02d:%02d", _minutes, _seconds);
	}
	(*m_pTimeText).DisplayString(pDC, msg, 16, FW_SEMIBOLD, OPTIONS_COLOR);

	SetTimer(GAME_TIMER, CLICK_TIME, nullptr);         // Reset ticker

	ReleaseDC(pDC);
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
void CMainWindow::MovePlayer(CPoint point) {
	CDC     *pDC;
	CSound  *pEffect = nullptr;
	CPoint  NewPosition;
	CPoint  TileLocation;
	CPoint  Hit;
	bool    bCollision = false;
	POINT   Delta;
	POINT   Step;
	unsigned int    nBmpID = IDB_HODJ_RIGHT;
	auto *app = AfxGetApp();

	if (_gameOver)
		// Don't allow movement after game finishes
		return;

	pDC = GetDC();

	Hit = ScreenToTile(point);

	Step.x = 0;
	Step.y = 0;

	NewPosition.x = _playerPos.x;
	NewPosition.y = _playerPos.y;

	Delta.x = _playerPos.x - Hit.x;            // Get x distance from mouse click to player in Tile spaces
	Delta.y = _playerPos.y - Hit.y;            // Get y distance from mouse click to player in Tile spaces

	if (ABS(Delta.x) > ABS(Delta.y)) {          // Moving horizontally:
		if (Delta.x < 0) {                      // To the RIGHT
			Step.x = 1;                             // move one tile at a time
			nBmpID = IDB_HODJ_RIGHT + m_nPlayerID;  // use the Bitmap of the player moving Right
		} else if (Delta.x > 0) {               // To the LEFT
			Step.x = -1;                            // move one tile at a time
			nBmpID = IDB_HODJ_LEFT + m_nPlayerID;   // use Bitmap of player moving Left
		}
	} else if (ABS(Delta.y) > ABS(Delta.x)) {
		if (Delta.y > 0) {                      // Going UPward
			Step.y = -1;                            // move one tile at a time
			nBmpID = IDB_HODJ_UP + m_nPlayerID;     // use Bitmap of player moving Up
		} else if (Delta.y < 0) {               // Going DOWNward
			Step.y = 1;                             // move one tile at a time
			nBmpID = IDB_HODJ_DOWN + m_nPlayerID;   // use Bitmap of player moving Down
		}
	}

	if ((Step.x != 0) || (Step.y != 0)) {       // If the click is not in the Player's Tile
		_success = (*_playerSprite).LoadResourceCels(pDC, nBmpID, NUM_CELS);
		ASSERT(_success);
		if (_playerSprite != nullptr)                                              // Refresh PLAYER
			(*_playerSprite).PaintSprite(pDC, (_playerPos.x * SQ_SIZE_X) + SIDE_BORDER,
			                             (_playerPos.y * SQ_SIZE_Y) + TOP_BORDER - SQ_SIZE_Y / 2);  //...in new direction

		while (!bCollision) {
			NewPosition.Offset(Step);

			if (_mazeTile[NewPosition.x][NewPosition.y].m_nWall == PATH ||       // Either a pathway
			        ((_mazeTile[NewPosition.x][NewPosition.y].m_nWall == TRAP &&     //...or a
			          _mazeTile[NewPosition.x][NewPosition.y].m_bHidden == false) ||  //...revealed trap
			         _mazeTile[NewPosition.x][NewPosition.y].m_nWall == EXIT)) {     //...or exit is a GO
				int i, x, y;
				x = (_playerPos.x * SQ_SIZE_X) + SIDE_BORDER;                  // Get player's position
				y = (_playerPos.y * SQ_SIZE_Y) + TOP_BORDER - SQ_SIZE_Y / 2;   //...in screen coords
				for (i = 0; i < 4; i++) {                                       // Go through three cels
					app->pause();
					x += Step.x * i * (SQ_SIZE_X / 4);                          //...per tile moved
					y += Step.y * i * (SQ_SIZE_Y / 4);
					if (_playerSprite != nullptr)
						(*_playerSprite).PaintSprite(pDC, x, y);                // Update PLAYER
				} // end for

				_playerPos.x = NewPosition.x;
				_playerPos.y = NewPosition.y;
				if (_playerSprite != nullptr)                                      // Refresh PLAYER
					(*_playerSprite).PaintSprite(pDC, (_playerPos.x * SQ_SIZE_X) + SIDE_BORDER,
						(_playerPos.y * SQ_SIZE_Y) + TOP_BORDER - SQ_SIZE_Y / 2);  //...in new direction
				app->pause();
			} // end if

			if ((_mazeTile[NewPosition.x][NewPosition.y].m_bHidden) &&
			        (_mazeTile[NewPosition.x][NewPosition.y].m_nWall == WALL)) {

				if (pGameInfo->bSoundEffectsEnabled) {
					pEffect = new CSound((CWnd *)this, HIT_SOUND,
					                     SOUND_WAVE | SOUND_AUTODELETE); //| SOUND_ASYNCH ...Wave file, to delete itself
					(*pEffect).play();                                                      //...play the narration
				}
				_mazeTile[NewPosition.x][NewPosition.y].m_bHidden = false;

				if (_playerSprite != nullptr)                                      // Refresh PLAYER
					(*_playerSprite).EraseSprite(pDC);                          // Erase PlayerSprite
				PaintBitmap(pDC, pGamePalette, _wallBitmap,                     // Paint wall on screen
				            _mazeTile[NewPosition.x][NewPosition.y].m_nStart.x + SIDE_BORDER,
				            _mazeTile[NewPosition.x][NewPosition.y].m_nStart.y + TOP_BORDER - SQ_SIZE_Y / 2);
				AddEdges(pDC, NewPosition.x, NewPosition.y, SIDE_BORDER, TOP_BORDER - SQ_SIZE_Y / 2);
				PaintBitmap(pMazeDC, pGamePalette, _wallBitmap,                 // Paint wall in Maze Bitmap
				            _mazeTile[NewPosition.x][NewPosition.y].m_nStart.x,
				            _mazeTile[NewPosition.x][NewPosition.y].m_nStart.y);
				AddEdges(pMazeDC, NewPosition.x, NewPosition.y, 0, 0);
				if (_playerSprite != nullptr)                                      // Refresh PLAYER
					(*_playerSprite).PaintSprite(pDC, (_playerPos.x * SQ_SIZE_X) + SIDE_BORDER,
					                             (_playerPos.y * SQ_SIZE_Y) + TOP_BORDER - SQ_SIZE_Y / 2);  //...in new direction
				bCollision = true;
			}

			if (_mazeTile[NewPosition.x][NewPosition.y].m_nWall == TRAP &&
			        (_mazeTile[NewPosition.x][NewPosition.y].m_bHidden)) {       // Traps are only good once

				_mazeTile[NewPosition.x][NewPosition.y].m_bHidden = false;
				_playerPos.x = _mazeTile[NewPosition.x][NewPosition.y].m_nDest.x;
				_playerPos.y = _mazeTile[NewPosition.x][NewPosition.y].m_nDest.y;
				if (_playerSprite != nullptr)
					(*_playerSprite).EraseSprite(pDC);                      // Erase PlayerSprite
				PaintBitmap(pDC, pGamePalette,                              // Paint trap on screen
				            _trapBitmap[_mazeTile[NewPosition.x][NewPosition.y].m_nTrap],
				            _mazeTile[NewPosition.x][NewPosition.y].m_nStart.x + SIDE_BORDER,
				            _mazeTile[NewPosition.x][NewPosition.y].m_nStart.y + TOP_BORDER - SQ_SIZE_Y / 2);

				if (pGameInfo->bSoundEffectsEnabled) {
					pEffect = new CSound((CWnd *)this, TRAP_SOUND,
					                     SOUND_WAVE | SOUND_AUTODELETE); //| SOUND_ASYNCH ...Wave file, to delete itself
					(*pEffect).play();                                                      //...play the narration
				}
				PaintBitmap(pMazeDC, pGamePalette,                          // Paint trap in Maze Bitmap
				            _trapBitmap[_mazeTile[NewPosition.x][NewPosition.y].m_nTrap],
				            _mazeTile[NewPosition.x][NewPosition.y].m_nStart.x,
				            _mazeTile[NewPosition.x][NewPosition.y].m_nStart.y);
				if (_playerSprite != nullptr)
					(*_playerSprite).PaintSprite(pDC, (_playerPos.x * SQ_SIZE_X) + SIDE_BORDER,
					                             (_playerPos.y * SQ_SIZE_Y) + TOP_BORDER - SQ_SIZE_Y / 2); // Update PLAYER
				bCollision = true;
			}

			if ((_mazeTile[NewPosition.x][NewPosition.y].m_nWall == WALL) ||
			        (_mazeTile[NewPosition.x][NewPosition.y].m_nWall == START)) {
				bCollision = true;
			}

			if (_mazeTile[NewPosition.x][NewPosition.y].m_nWall == EXIT) {
				_playing = false;
				_gameOver = true;
				KillTimer(GAME_TIMER);

				if (pGameInfo->bSoundEffectsEnabled) {
					pEffect = new CSound((CWnd *)this, WIN_SOUND,
					                     SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE);  //...Wave file, to delete itself
					(*pEffect).play();                                                      //...play the narration
				}
				MSG lpmsg;
				while (PeekMessage(&lpmsg, m_hWnd, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE)) ;
				CMessageBox GameOverDlg((CWnd *)this, pGamePalette, "Game over.", "He's free!");
				CSound::waitWaveSounds();
				if (pGameInfo->bPlayingMetagame) {
					pGameInfo->lScore = 1;                  // A victorious maze solving
					PostMessage(WM_CLOSE, 0, 0);            // and post a program exit
				}
				bCollision = true;
			}

			if ((NewPosition.x == Hit.x) && (NewPosition.y == Hit.y))
				bCollision = true;
		} // end while

		GetNewCursor();
	} // end if

	ReleaseDC(pDC);

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
	CPoint  Hit, Delta;
	POINT   pCursorLoc;

	HCURSOR hNewCursor = nullptr;
	CWinApp *pMyApp;

	pMyApp = AfxGetApp();

	MFC::GetCursorPos(&pCursorLoc);
	MFC::ScreenToClient(m_hWnd, &pCursorLoc);
	Delta.x = pCursorLoc.x;
	Delta.y = pCursorLoc.y;
	Hit = ScreenToTile(Delta);

	Delta.x = _playerPos.x - Hit.x;
	Delta.y = _playerPos.y - Hit.y;

	if ((_playerPos.x == Hit.x) && (_playerPos.y == Hit.y)) {     // Directly over player
		hNewCursor = (*pMyApp).LoadCursor(IDC_MOD_NOARROW);
	}

	else if (ABS(Delta.x) >= ABS(Delta.y)) {                        // Moving horizontally:
		if (Delta.x <= 0)                                           // To the RIGHT
			hNewCursor = (*pMyApp).LoadCursor(IDC_MOD_RTARROW);
		else if (Delta.x > 0)                                       // To the LEFT
			hNewCursor = (*pMyApp).LoadCursor(IDC_MOD_LFARROW);
	} else if (ABS(Delta.y) > ABS(Delta.x)) {
		if (Delta.y >= 0)                                           // Going UPward
			hNewCursor = (*pMyApp).LoadCursor(IDC_MOD_UPARROW);
		else if (Delta.y < 0)                                       // Going DOWNward
			hNewCursor = (*pMyApp).LoadCursor(IDC_MOD_DNARROW);
	}

//    if (hNewCursor != nullptr);
	MFC::SetCursor(hNewCursor);
}

/*****************************************************************
 *
 *  PaintMaze
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Paints the maze in the pDC
 *
 *  FORMAL PARAMETERS:
 *
 *      CDC pDC     Device context to which we're drawing the maze
 *
 *  IMPLICIT INPUT PARAMETERS:
 *
 *      struct TILE mazeTile grid
 *
 *  IMPLICIT OUTPUT PARAMETERS:
 *
 *      The bitmap associated with pDC
 *
 *  RETURN VALUE:
 *
 *      void
 *
 ****************************************************************/
void PaintMaze(CDC *pDC) {
	int x, y;

	for (x = 0; x < NUM_COLUMNS; x++) {
		for (y = 0; y < NUM_ROWS; y++) {
			_mazeTile[x][y].m_nStart.x = x * SQ_SIZE_X;                              // Put in location info
			_mazeTile[x][y].m_nStart.y = y * SQ_SIZE_Y;
			if ((_mazeTile[x][y].m_nWall == PATH) || (_mazeTile[x][y].m_nWall == EXIT) ||
			        _mazeTile[x][y].m_bHidden)       // Path or hidden obj.
				PaintBitmap(pDC, pGamePalette, _pathBitmap,                         //...draw path bitmap
				            _mazeTile[x][y].m_nStart.x, _mazeTile[x][y].m_nStart.y);
			else if (_mazeTile[x][y].m_nWall == START)                               // Start of maze
				PaintBitmap(pDC, pGamePalette, _startBitmap,                        //...draw start bitmap
				            _mazeTile[x][y].m_nStart.x, _mazeTile[x][y].m_nStart.y);
			else                                                                    // Otherwise, it's a
				PaintBitmap(pDC, pGamePalette, _wallBitmap,                         //...wall
				            _mazeTile[x][y].m_nStart.x, _mazeTile[x][y].m_nStart.y);
		} // end for y
	} // end for x

	for (x = 0; x < NUM_COLUMNS; x++) {                                     // Go through the grid
		for (y = 0; y < NUM_ROWS; y++) {                                    //...and for every square
			AddEdges(pDC, x, y, 0, 0);                                      //...add trim if needed
		} // end for y
	} // end for x

} // End PaintMaze

/*****************************************************************
 *
 *  SetUpMaze
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Translates the random maze generated into the mazeTile grid for the game
 *
 *  FORMAL PARAMETERS:
 *
 *      none
 *
 *  IMPLICIT INPUT PARAMETERS:
 *
 *      maze[][]            The randomly generated maze
 *      struct TILE mazeTile[][] grid
 *      start_y
 *      exit_y
 *
 *  IMPLICIT OUTPUT PARAMETERS:
 *
 *      struct TILE mazeTile[][] grid
 *
 *  RETURN VALUE:
 *
 *      void
 *
 ****************************************************************/
void SetUpMaze() {
	int     x, y;
	CPoint  m_pExit;

	for (y = 0; y < NUM_ROWS; y++)                          // Set the right wall solid
		_mazeTile[NUM_COLUMNS - 1][y].m_nWall = WALL;

	for (x = 0; x < MAX_MAZE_SIZE_X; x++) {
		for (y = 0; y < MAX_MAZE_SIZE_Y; y++) {
			_mazeTile[x * 2 + 1][y * 2 + 1].m_nWall = PATH;  // Always is PATH
			_mazeTile[x * 2][y * 2].m_nWall = PATH;          // Will be changed to WALL if
			if (_maze[x][y] & WALL_TOP) {                    //...it is found below
				_mazeTile[x * 2][y * 2].m_nWall = WALL;
				_mazeTile[x * 2 + 1][y * 2].m_nWall = WALL;
			} else
				_mazeTile[x * 2 + 1][y * 2].m_nWall = PATH;

			if (_maze[x][y] & WALL_LEFT) {
				_mazeTile[x * 2][y * 2].m_nWall = WALL;
				_mazeTile[x * 2][y * 2 + 1].m_nWall = WALL;
			} else
				_mazeTile[x * 2][y * 2 + 1].m_nWall = PATH;
		}
	}

	for (x = 0; x < NUM_COLUMNS; x++) {         // Now go through  mazeTile and fix up loose ends, as it were
		for (y = 0; y < NUM_ROWS; y++) {
			_mazeTile[x][y].m_bHidden = false;

			if (x > 0 && y > 0 && x < (NUM_COLUMNS - 1) && y < (NUM_ROWS - 1) && _mazeTile[x][y].m_nWall == PATH) {
				if (_mazeTile[x + 1][y + 1].m_nWall == PATH &&
					(_mazeTile[x + 1][y].m_nWall == PATH &&
				        (_mazeTile[x][y + 1].m_nWall == PATH &&
				         (_mazeTile[x - 1][y].m_nWall == WALL && _mazeTile[x][y - 1].m_nWall == WALL))))
					_mazeTile[x][y].m_nWall = WALL;              // If it's a right-hand corner

				if (_mazeTile[x][y + 1].m_nWall == PATH &&
					(_mazeTile[x + 1][y - 1].m_nWall == PATH &&
				        (_mazeTile[x - 1][y - 1].m_nWall == PATH &&
				         (_mazeTile[x - 1][y + 1].m_nWall == PATH && (_mazeTile[x + 1][y + 1].m_nWall == PATH &&
				                 (_mazeTile[x - 1][y].m_nWall == PATH && _mazeTile[x + 1][y].m_nWall == PATH))))))
					_mazeTile[x][y].m_nWall = WALL;              // If it's two wide vertically from the top

				if (_mazeTile[x][y - 1].m_nWall == PATH &&
					(_mazeTile[x - 1][y - 1].m_nWall == PATH &&
				        (_mazeTile[x - 1][y + 1].m_nWall == PATH &&
				         (_mazeTile[x][y + 1].m_nWall == PATH && (_mazeTile[x + 1][y - 1].m_nWall == PATH &&
				                 (_mazeTile[x + 1][y].m_nWall == PATH && _mazeTile[x + 1][y + 1].m_nWall == PATH))))))
					_mazeTile[x][y].m_nWall = WALL;              // If it's two wide horizontally from the left
			}

			if (y == NUM_ROWS - 1)
				_mazeTile[x][y].m_nWall = WALL;              // Make bottom wall
		}
	}

	x = NUM_COLUMNS - 1;                                // Get the Entry point
	y = (_startY * 2) + 1;

	_playerPos.x = x - 1;                              // Start player in one space from the entrance

	if (_mazeTile[x - 1][y].m_nWall == WALL) {           // If a wall runs into the entry space
		_mazeTile[x][y].m_nWall = WALL;                  //...make it a wall and put the entry
		_mazeTile[x][y + 1].m_nWall = START;             //...space under that
		_playerPos.y = y;                              // Put the player there
	} else {
		_mazeTile[x][y].m_nWall = START;                 // Put in the entry way where it was
		_mazeTile[x][y + 1].m_nWall = WALL;              //...and make sure the one below is a wall
		_playerPos.y = y;                              // Put the player there
	}

	x = _endX * 2;                                      // This should be 0
	y = _endY * 2;
	m_pExit.x = x;

	if (_mazeTile[x + 1][y].m_nWall == WALL) {           // If a wall runs into the top exit space
		_mazeTile[x][y].m_nWall = WALL;                  //...make it a wall and put the exit
		m_pExit.y = y + 1;                              //...one space above that
	} else {
		_mazeTile[x][y + 1].m_nWall = WALL;              // Put the exit in the top space
		m_pExit.y = y;                                  //...and store the y position in m_pExit
	}

	_mazeTile[m_pExit.x][m_pExit.y].m_nWall = EXIT;      // Make exit grid space a Pathway


	SetInvisibleWalls();                                // Hide some walls
	SetTraps();                                         // Put in some traps

}//end SetUpMaze

/*****************************************************************
 *
 *  AddEdges
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Draws fancy edgework around a piece of wall on the sides where possible
 *
 *  FORMAL PARAMETERS:
 *
 *      CDC     *pDC        The Device context to which it will draw
 *      int     x           Column of piece to check
 *      int     y           Row of piece to check
 *      int     offset_x    Width offset for drawing the bitmap ( Main window has a border)
 *      int     offset_y    Hieght offset as above
 *
 *  IMPLICIT INPUT PARAMETERS:
 *
 *      struct TILE mazeTile[][]
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
void AddEdges(CDC *pDC, int x, int y, int offset_x, int offset_y) {
	if ((_mazeTile[x][y].m_bHidden == false) && (_mazeTile[x][y].m_nWall == WALL)) {

		if ((y > 0) && ((((_mazeTile[x][y - 1].m_nWall == PATH) || (_mazeTile[x][y - 1].m_nWall == EXIT)) ||
		                 (_mazeTile[x][y - 1].m_nWall == START)) || _mazeTile[x][y - 1].m_bHidden))    // TOP
			PaintBitmap(pDC, pGamePalette, _bottomEdgeBmp,
			            _mazeTile[x][y - 1].m_nStart.x + offset_x,
			            _mazeTile[x][y - 1].m_nStart.y + offset_y + SQ_SIZE_Y - 1 - EDGE_SIZE);

		if ((x < (NUM_COLUMNS - 1)) && ((_mazeTile[x + 1][y].m_nWall == PATH) ||
		                                _mazeTile[x + 1][y].m_bHidden))                      // RIGHT
			PaintBitmap(pDC, pGamePalette, _leftEdgeBmp,
			            _mazeTile[x + 1][y].m_nStart.x + offset_x,
			            _mazeTile[x + 1][y].m_nStart.y + offset_y);

		if ((y < (NUM_ROWS - 1)) && ((((_mazeTile[x][y + 1].m_nWall == EXIT) ||
		                               (_mazeTile[x][y + 1].m_nWall == PATH)) ||
		                              (_mazeTile[x][y + 1].m_nWall == START)) || _mazeTile[x][y + 1].m_bHidden))    // BOTTOM
			PaintBitmap(pDC, pGamePalette, _topEdgeBmp,
			            _mazeTile[x][y + 1].m_nStart.x + offset_x,
			            _mazeTile[x][y + 1].m_nStart.y + offset_y);

		if ((x > 0) && ((_mazeTile[x - 1][y].m_nWall == PATH) ||
		                _mazeTile[x - 1][y].m_bHidden))                                  // LEFT
			PaintBitmap(pDC, pGamePalette, _rightEdgeBmp,
			            _mazeTile[x - 1][y].m_nStart.x + offset_x + SQ_SIZE_X - 1 - EDGE_SIZE,
			            _mazeTile[x - 1][y].m_nStart.y + offset_y);
	} // end if WALL

} // End AddEdges

/*****************************************************************
 *
 *  SetTraps
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Sets traps in the maze
 *
 *  FORMAL PARAMETERS:
 *
 *      none
 *
 *  IMPLICIT INPUT PARAMETERS:
 *
 *      struct TILE mazeTile[][] array
 *      m_nDifficulty       The the number of traps = difficulty setting
 *
 *  IMPLICIT OUTPUT PARAMETERS:
 *
 *      struct TILE mazeTile[][] array
 *
 *  RETURN VALUE:
 *
 *      void
 *
 ****************************************************************/
void SetTraps() {
	int     nTrapCount;
	int     nNumTraps;
	CPoint  In;

//  nNumTraps = m_nDifficulty + MIN_TRAPS;
	nNumTraps = MIN_TRAPS + (_difficulty / 2);        // 4 + ([1...10]/2) = 4 to 9

	for (nTrapCount = 0; nTrapCount < nNumTraps; nTrapCount++) {
		In = GetRandomPoint(false);                                         // Pick a random PATH square
		_mazeTile[In.x][In.y].m_nWall = TRAP;                                // Make it a TRAP
		_mazeTile[In.x][In.y].m_bHidden = true;                              // Hide it
		_mazeTile[In.x][In.y].m_nTrap = nTrapCount % NUM_TRAP_MAPS;          // Assign unique trap bitmap ID
		_mazeTile[In.x][In.y].m_nDest = GetRandomPoint(true);                // Pick a random Trap destination
	}
}

/*****************************************************************
 *
 *  GetRandomPoint
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Gets a random Grid Point in the maze, which is a PATH (not START or EXIT)
 *
 *  FORMAL PARAMETERS:
 *
 *      none
 *
 *  IMPLICIT INPUT PARAMETERS:
 *
 *      struct TILE mazeTile[][] grid
 *
 *  IMPLICIT OUTPUT PARAMETERS:
 *
 *      none
 *
 *  RETURN VALUE:
 *
 *      CPoint      the random X and Y of a path space in the mazeTile grid
 *
 ****************************************************************/
CPoint GetRandomPoint(bool bRight) {
	CPoint  point;
	bool    bLocated = false;

	if (bRight)                                                             // Get random column
		point.x = (brand() % (2 * (NUM_COLUMNS / 3))) + (NUM_COLUMNS / 3);   //...in the right half
	else
		point.x = brand() % (2 * (NUM_COLUMNS / 3));                         //...or the left half
	point.y = brand() % NUM_ROWS;                                            // Get random row

	while (!bLocated) {
		if (_mazeTile[point.x][point.y].m_nWall == PATH)
			bLocated = true;                                                // OK if it's a pathway
		else {                                                              // Otherwise, keep lookin'
			point.x++;                                                      // Increment Column
			point.y++;                                                      // Increment Row
			if (point.x == NUM_COLUMNS) point.x = 1;                        // If we're at the end,
			if (point.y == NUM_ROWS) point.y = 1;                           //...reset the counter
		}
	} // end while

	return point;

} // end GetRandomPoint


/*****************************************************************
 *
 *  SetInvisibleWalls
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Randomly sets a number of walls invisible
 *
 *  FORMAL PARAMETERS:
 *
 *      none
 *
 *  IMPLICIT INPUT PARAMETERS:
 *
 *      struct TILE mazeTile[][] grid
 *      m_nDifficulty       If it's MIN_DIFFICULTY, no walls are invisible
 *                          If it's MAX_DIFFICULTY, all walls are invisible
 *                          Otherwise, every m_nDifficulty-th wall is visible
 *
 *  IMPLICIT OUTPUT PARAMETERS:
 *
 *      struct TILE mazeTile[][] grid
 *
 *  RETURN VALUE:
 *
 *      void
 *
 ****************************************************************/
void SetInvisibleWalls() {
	int x, y, i, j;
	int nWallCount = 0;
	int nMaxWalls = 0;
	int nTotalWalls = 0;

	for (x = 1; x < (NUM_COLUMNS - 1); x++) {                       // Don't make edge walls invisible !!
		for (y = 1; y < (NUM_ROWS - 1); y++) {
			if (_mazeTile[x][y].m_nWall == WALL) {
				if (_difficulty > MIN_DIFFICULTY)                 // Most difficult has all walls hidden
					_mazeTile[x][y].m_bHidden = true;                // Start with all walls hidden
				else
					_mazeTile[x][y].m_bHidden = false;               // Least difficult has no walls hidden
				nTotalWalls++;
			} // end if
		} // end for y
	} // end for x

	if (_difficulty > MIN_DIFFICULTY && _difficulty < MAX_DIFFICULTY) {
		x = (brand() % (NUM_COLUMNS - 4)) + 2;                       // Avoid the edge walls
		y = (brand() % (NUM_ROWS - 4)) + 2;
		nMaxWalls = nTotalWalls - (int)(_difficulty * (nTotalWalls / 10));

		while (nWallCount < nMaxWalls) {
			if (_mazeTile[x][y].m_nWall == WALL && _mazeTile[x][y].m_bHidden) {
				for (i = x - 1; i <= x + 1; i++) {
					for (j = y - 1; j <= y + 1; j++) {
						if (_mazeTile[i][j].m_nWall == WALL && _mazeTile[i][j].m_bHidden) {
							_mazeTile[i][j].m_bHidden = false;       // so it's not hidden
							nWallCount++;                           // increment the count
						} // end if
					} // end j
				} // end i
			} // end if
			x += (brand() % NUM_NEIGHBORS);// + 1;                                   // Increment Column
			y += (brand() % NUM_NEIGHBORS);// + 1;                                   // Increment Row
			if (x >= (NUM_COLUMNS - 2))
				x = (brand() % (NUM_COLUMNS - 4)) + 2;               // If we're at the end,
			if (y >= (NUM_ROWS - 2))
				y = (brand() % (NUM_COLUMNS - 4)) + 2;               //...reset the counter
		} // end while
	} // end if
}


void initialize_maze() {       /* draw the surrounding wall and start/end squares */
	int i, j, wall;

	_mazeSizeX = MAX_MAZE_SIZE_X;
	_mazeSizeY = MAX_MAZE_SIZE_Y;

	/* initialize all squares */
	for (i = 0; i < _mazeSizeX; i++) {
		for (j = 0; j < _mazeSizeY; j++) {
			_maze[i][j] = 0;
		}
	}

	/* top wall */
	for (i = 0; i < _mazeSizeX; i++) {
		_maze[i][0] |= WALL_TOP;
	}

	/* right wall */
	for (j = 0; j < _mazeSizeY; j++) {
		_maze[_mazeSizeX - 1][j] |= WALL_RIGHT;
	}

	/* bottom wall */
	for (i = 0; i < _mazeSizeX; i++) {
		_maze[i][_mazeSizeY - 1] |= WALL_BOTTOM;
	}

	/* left wall */
	for (j = 0; j < _mazeSizeY; j++) {
		_maze[0][j] |= WALL_LEFT;
	}

	/* set start square */
	wall = 1;                           // Start on right side
	i = _mazeSizeX - 1;                // Set maze x location
	j = brand() % _mazeSizeY;       // Set a random y location not on the top row

	_maze[i][j] |= START_SQUARE;
	_maze[i][j] |= (DOOR_IN_TOP >> wall);
	_maze[i][j] &= ~(WALL_TOP >> wall);
	_startX = i;
	_startY = j;
	_curSqX = i;
	_curSqY = j;
	_sqNum = 0;

	/* set end square */
	wall = (wall + 2) % 4;
	switch (wall) {
	case 0:
		i = brand() % (_mazeSizeX);
		j = 0;
		break;
	case 1:
		i = _mazeSizeX - 1;
		j = brand() % (_mazeSizeY);
		break;
	case 2:
		i = brand() % (_mazeSizeX);
		j = _mazeSizeY - 1;
		break;
	case 3:
		i = 0;
		j = brand() % (_mazeSizeY);
		break;
	}
	_maze[i][j] |= END_SQUARE;
	_maze[i][j] |= (DOOR_OUT_TOP >> wall);
	_maze[i][j] &= ~(WALL_TOP >> wall);
	_endX = i;
	_endY = j;

}


void create_maze() {           /* create a maze layout given the intiialized maze */
	int newdoor = 0;         // i,

	do {
		_moveList[_sqNum].x = _curSqX;
		_moveList[_sqNum].y = _curSqY;
		_moveList[_sqNum].dir = newdoor;
		while ((newdoor = choose_door()) == -1) {     /* pick a door */
			if (backup() == -1) {   /* no more doors ... backup */
				return; /* done ... return */
			}
		}

		/* mark the out door */
		_maze[_curSqX][_curSqY] |= (DOOR_OUT_TOP >> newdoor);

		switch (newdoor) {
		case 0:
			_curSqY--;
			break;
		case 1:
			_curSqX++;
			break;
		case 2:
			_curSqY++;
			break;
		case 3:
			_curSqX--;
			break;
		}
		_sqNum++;

		/* mark the in door */
		_maze[_curSqX][_curSqY] |= (DOOR_IN_TOP >> ((newdoor + 2) % 4));

		/* if end square set path length and save path */
	} while (1);

}


int choose_door() {                    /* pick a new path */
	int candidates[4];
	int num_candidates;

	num_candidates = 0;

//topwall:
	/* top wall */
	if (_maze[_curSqX][_curSqY] & DOOR_IN_TOP)
		goto rightwall;
	if (_maze[_curSqX][_curSqY] & DOOR_OUT_TOP)
		goto rightwall;
	if (_maze[_curSqX][_curSqY] & WALL_TOP)
		goto rightwall;
	if (_maze[_curSqX][_curSqY - 1] & DOOR_IN_ANY) {
		_maze[_curSqX][_curSqY] |= WALL_TOP;
		_maze[_curSqX][_curSqY - 1] |= WALL_BOTTOM;
		goto rightwall;
	}
	candidates[num_candidates++] = 0;

rightwall:
	/* right wall */
	if (_maze[_curSqX][_curSqY] & DOOR_IN_RIGHT)
		goto bottomwall;
	if (_maze[_curSqX][_curSqY] & DOOR_OUT_RIGHT)
		goto bottomwall;
	if (_maze[_curSqX][_curSqY] & WALL_RIGHT)
		goto bottomwall;
	if (_maze[_curSqX + 1][_curSqY] & DOOR_IN_ANY) {
		_maze[_curSqX][_curSqY] |= WALL_RIGHT;
		_maze[_curSqX + 1][_curSqY] |= WALL_LEFT;
		goto bottomwall;
	}
	candidates[num_candidates++] = 1;

bottomwall:
	/* bottom wall */
	if (_maze[_curSqX][_curSqY] & DOOR_IN_BOTTOM)
		goto leftwall;
	if (_maze[_curSqX][_curSqY] & DOOR_OUT_BOTTOM)
		goto leftwall;
	if (_maze[_curSqX][_curSqY] & WALL_BOTTOM)
		goto leftwall;
	if (_maze[_curSqX][_curSqY + 1] & DOOR_IN_ANY) {
		_maze[_curSqX][_curSqY] |= WALL_BOTTOM;
		_maze[_curSqX][_curSqY + 1] |= WALL_TOP;
		goto leftwall;
	}
	candidates[num_candidates++] = 2;

leftwall:
	/* left wall */
	if (_maze[_curSqX][_curSqY] & DOOR_IN_LEFT)
		goto donewall;
	if (_maze[_curSqX][_curSqY] & DOOR_OUT_LEFT)
		goto donewall;
	if (_maze[_curSqX][_curSqY] & WALL_LEFT)
		goto donewall;
	if (_maze[_curSqX - 1][_curSqY] & DOOR_IN_ANY) {
		_maze[_curSqX][_curSqY] |= WALL_LEFT;
		_maze[_curSqX - 1][_curSqY] |= WALL_RIGHT;
		goto donewall;
	}
	candidates[num_candidates++] = 3;

donewall:
	if (num_candidates == 0)
		return -1;
	if (num_candidates == 1)
		return candidates[0];
	return candidates[brand() % (num_candidates)];
}


int backup() {                                                /* back up a move */
	_sqNum--;
	if (_sqNum > 0) {
		_curSqX = _moveList[_sqNum].x;
		_curSqY = _moveList[_sqNum].y;
	}

	return _sqNum;
}


/*****************************************************************
 *
 *  ScreenToTile
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Converts a point in screen coordinates to x & y location on maze grid
 *
 *  FORMAL PARAMETERS:
 *
 *      CPoint pointScreen      a point in screen coordinates
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
 *      CPoint  point       the x & y grid coordinates where pointScreen fell
 *
 ****************************************************************/
CPoint ScreenToTile(CPoint pointScreen) {
	CPoint point;

	point.x = (pointScreen.x - SIDE_BORDER) / SQ_SIZE_X;
	point.y = (pointScreen.y - TOP_BORDER + SQ_SIZE_Y / 2) / SQ_SIZE_Y;

	return point;
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
	if ((point.x > SIDE_BORDER && point.x < GAME_WIDTH - SIDE_BORDER) &&        // See if point lies within
	        (point.y > TOP_BORDER && point.y < GAME_HEIGHT - BOTTOM_BORDER))        //...ArtWork area
		return true;                                                            // Return true if it's inside
	else return false;                                                          //...and false if not
}

void CMainWindow::OnClose() {
	CDC     *pDC;
	CBrush  myBrush;
	CRect   myRect;

	pDC = GetDC();
	myRect.SetRect(0, 0, GAME_WIDTH, GAME_HEIGHT);
	myBrush.CreateStockObject(BLACK_BRUSH);
	(*pDC).FillRect(&myRect, &myBrush);
	ReleaseDC(pDC);

	SetCursor(LoadCursor(nullptr, IDC_ARROW));            // Refresh cursor object to arrow

	// delete the game theme song
	//
	if (_gameSound != nullptr) {
		delete _gameSound;
		_gameSound = nullptr;
	}

	CSound::clearSounds();                              // Clean up sounds before returning

	if (m_pScrollButton != nullptr)
		delete m_pScrollButton;

	if (m_pTimeText != nullptr)
		delete m_pTimeText;

	if (_blankBitmap != nullptr) {
		_blankBitmap->DeleteObject();
		delete _blankBitmap;
	}

	if (_localeBitmap != nullptr) {
		_localeBitmap->DeleteObject();
		delete _localeBitmap;
	}

	if (_playerSprite != nullptr)
		delete _playerSprite;

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
	if (_pathBitmap != nullptr) {
		_pathBitmap->DeleteObject();
		delete _pathBitmap;
	}
	if (_topEdgeBmp != nullptr) {
		_topEdgeBmp->DeleteObject();
		delete _topEdgeBmp;
	}
	if (_rightEdgeBmp != nullptr) {
		_rightEdgeBmp->DeleteObject();
		delete _rightEdgeBmp;
	}
	if (_bottomEdgeBmp != nullptr) {
		_bottomEdgeBmp->DeleteObject();
		delete _bottomEdgeBmp;
	}
	if (_leftEdgeBmp != nullptr) {
		_leftEdgeBmp->DeleteObject();
		delete _leftEdgeBmp;
	}
	if (_wallBitmap != nullptr) {
		_wallBitmap->DeleteObject();
		delete _wallBitmap;
	}
	if (_startBitmap != nullptr) {
		_startBitmap->DeleteObject();
		delete _startBitmap;
	}
	for (int i = 0; i < NUM_TRAP_MAPS; i++) {
		if (_trapBitmap[i] != nullptr) {
			_trapBitmap[i]->DeleteObject();
			delete _trapBitmap[i];
		}
	}
	if (pGamePalette != nullptr) {
		pGamePalette->DeleteObject();
		delete pGamePalette;
	}

	CFrameWnd::OnClose();

	MFC::PostMessage(ghParentWnd, WM_PARENTNOTIFY, WM_DESTROY, 0L);
}


void CALLBACK GetSubOptions(CWnd* pParentWind) {
	COptnDlg OptionsDlg(pParentWind, pGamePalette);     // Call Specific Game
	//...Options dialog box
	OptionsDlg._time  = _time;
	OptionsDlg._seconds = _seconds;             // Send clock info
	OptionsDlg._minutes = _minutes;
	OptionsDlg._difficulty = _difficulty;

	if (OptionsDlg.DoModal() == IDOK) {             // save values set in dialog box
		tempTime = OptionsDlg._time;;             // get new time limit,
		tempDifficulty = OptionsDlg._difficulty;  //...new Difficulty
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
	ON_WM_TIMER()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_MESSAGE(MM_MCINOTIFY, CMainWindow::OnMCINotify)
	ON_MESSAGE(MM_WOM_DONE, CMainWindow::OnMMIONotify)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

} // namespace MazeDoom
} // namespace HodjNPodj
} // namespace Bagel
