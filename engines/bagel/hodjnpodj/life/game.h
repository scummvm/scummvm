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

#ifndef HODJNPODJ_LIFE_GAME_H
#define HODJNPODJ_LIFE_GAME_H

#include "bagel/boflib/sound.h"
#include "bagel/hodjnpodj/hnplibs/gamedll.h"

namespace Bagel {
namespace HodjNPodj {
namespace Life {

#define SPLASHSPEC          ".\\art\\backgrnd.BMP"


#define CURLY_X         25      // Colony dimensions
#define CURLY_Y         24

#define VILLAGES_PLACED 10      // Number of colonies pre-placed in meta-game mode
#define PLACE_COLS      10
#define PLACE_ROWS      10
#define OFFSET_X         6
#define OFFSET_Y         4

#define SCROLL_BUTTON   ".\\art\\SCROLBTN.BMP"  // scroll commands button bmp
#define CALENDAR_BMP    ".\\ART\\CALENDAR.BMP"  // the blank calendar artwork

// stat info
#define STATS_COLOR             RGB(255, 255, 0)    // color of visual game stats
#define STATS_FONT_SIZE			15
#define MONTH_COL_POS           17                  // sprite calendar pos
#define MONTH_ROW_POS           140

#define SCORE_LEFT_COL          459         // score pos
#define SCORE_LEFT_ROW          30
#define SCORE_RIGHT_COL         574
#define SCORE_RIGHT_ROW         52

#define CURRENT_LEFT_COL        44          // current villages pos
#define CURRENT_LEFT_ROW        30
#define CURRENT_RIGHT_COL       220
#define CURRENT_RIGHT_ROW       52

#define ROUND_LEFT_COL          15          // years past pos
#define ROUND_LEFT_ROW          315
#define ROUND_RIGHT_COL         62
#define ROUND_RIGHT_ROW         331

#define ROUND_TEXT1_LEFT_COL    15
#define ROUND_TEXT1_LEFT_ROW    330
#define ROUND_TEXT1_RIGHT_COL   62
#define ROUND_TEXT1_RIGHT_ROW   352

#define ROUND_TEXT2_LEFT_COL    15
#define ROUND_TEXT2_LEFT_ROW    351
#define ROUND_TEXT2_RIGHT_COL   62
#define ROUND_TEXT2_RIGHT_ROW   373

#define VILLAGE_LEFT_COL        15          // villages left pos
#define VILLAGE_LEFT_ROW        423
#define VILLAGE_RIGHT_COL       62
#define VILLAGE_RIGHT_ROW       441

#define VILLAGE_TEXT1_LEFT_COL      15
#define VILLAGE_TEXT1_LEFT_ROW      438
#define VILLAGE_TEXT1_RIGHT_COL     62
#define VILLAGE_TEXT1_RIGHT_ROW     459

// Button Identifier codes (BIDs)
#define IDC_EVOLVE  101
#define IDC_COMMAND 102

// Button positioning constants
#define QUIT_BUTTON_WIDTH           50
#define QUIT_BUTTON_HEIGHT          20
#define QUIT_BUTTON_OFFSET_X        10
#define QUIT_BUTTON_OFFSET_Y        10


#define COMMAND_BUTTON_WIDTH        80
#define COMMAND_BUTTON_HEIGHT       18
#define COMMAND_OFFSET_X            0
#define COMMAND_OFFSET_Y            0

#define EVOLVE_BUTTON_WIDTH         94
#define EVOLVE_BUTTON_HEIGHT        20
#define EVOLVE_BUTTON_OFFSET_X      0
#define EVOLVE_BUTTON_OFFSET_Y      30

// Timer ID's
#define EVOLVE_TIMER_ID             10001
#define EVOLVE_INTERVAL             100 // decaseconds

/////////////////////////////////////////////////////////////////////////////

// CMainWindow:
// See game.cpp for the code to the member functions and the message map.

class CLife;

class CMainWindow : public CFrameWnd {
private:
	CLife           *m_cLife = nullptr;
	LPGAMESTRUCT    m_lpGameStruct = nullptr;
	HWND            m_hCallAppWnd = nullptr;
	bool            m_bGameActive = false;
	CSound          *m_pSound = nullptr;
	CRect           m_rNewGameButton;

public:
	CMainWindow(HWND, LPGAMESTRUCT);
	~CMainWindow();

	void initStatics();
	void SplashScreen();
	void DisplayStats();
	void RefreshStats();
	void NewGame();
	void GamePause();
	void GameResume();

protected:
	virtual bool OnCommand(WPARAM wParam, LPARAM lParam) override;

	//{{AFX_MSG( CMainWindow )
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(unsigned int, CPoint);
	afx_msg void OnLButtonDblClk(unsigned int, CPoint);
	afx_msg void OnSysKeyDown(unsigned int, unsigned int, unsigned int);
	afx_msg void OnSysChar(unsigned int, unsigned int, unsigned int);
	afx_msg void OnKeyDown(unsigned int, unsigned int, unsigned int);
	afx_msg void OnTimer(uintptr);
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg LRESULT OnMCINotify(WPARAM, LPARAM);
	afx_msg LRESULT OnMMIONotify(WPARAM, LPARAM);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

// CTheApp:
// See game.cpp for the code to the InitInstance member function.
//
class CTheApp : public CWinApp {
public:
	bool InitInstance();
};

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CWindowMain frame

class CWindowMain : public CFrameWnd {
	DECLARE_DYNCREATE(CWindowMain)
protected:
	CWindowMain();          // protected constructor used by dynamic creation
	virtual ~CWindowMain();

	// Generated message map functions
	//{{AFX_MSG(CWindowMain)
	// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

} // namespace Life
} // namespace HodjNPodj
} // namespace Bagel

#endif
