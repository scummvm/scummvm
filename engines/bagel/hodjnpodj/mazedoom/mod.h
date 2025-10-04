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

#ifndef HODJNPODJ_MAZEDOOM_MOD_H
#define HODJNPODJ_MAZEDOOM_MOD_H

#include "bagel/boflib/sound.h"

namespace Bagel {
namespace HodjNPodj {
namespace MazeDoom {

#define ABS(a)      ( (a > 0) ? (a) : (-a) )

// Border info
#define SIDE_BORDER      20
#define TOP_BORDER       28
#define BOTTOM_BORDER    16
#define HILITE_BORDER     3

// Dimensions constants
#define ART_WIDTH   600
#define ART_HEIGHT  432

#define SQ_SIZE_X   24
#define SQ_SIZE_Y   24
#define EDGE_SIZE    5

// Starting value defaults
#define MAX_DIFFICULTY  10  //8
#define MIN_DIFFICULTY   1  //0

// Timer constants
#define MIN_TIME             15                     //  15 Seconds
#define MAX_TIME            180                     // 180 Seconds = 3 minute max
#define TIMER_MIN             0                     // Increment scrollbar in steps of 15 Secs
#define TIMER_MAX            10                     // if Time > MAX_TIME, No Time Limit

#define NUM_COLUMNS     25
#define NUM_ROWS        19
#define NUM_NEIGHBORS    9                          // The "clump" area is 3 X 3 grid spaces

#define NUM_TRAP_MAPS    7                          // There are seven trap icons available
#define MIN_TRAPS        4

#define NUM_CELS         8

#define PATH    0
#define WALL    1
#define TRAP    2
#define START   3
#define EXIT    4

#define HODJ    0
#define PODJ    4

// Timer stuff
#define GAME_TIMER      1
#define CLICK_TIME   1000       // Every Second, update timer clock

// Rules files
#define RULES_TEXT      "MAZEOD.TXT"
#define RULES_WAV       ".\\SOUND\\MAZEOD.WAV"

// Sound files
#define WIN_SOUND   ".\\sound\\fanfare2.wav"
#define LOSE_SOUND  ".\\sound\\buzzer.wav"
#define HIT_SOUND   ".\\sound\\thud.wav"
#define TRAP_SOUND  ".\\sound\\boing.wav"

#define GAME_THEME  ".\\sound\\mazeod.mid"

// Backdrop bitmaps
#define MAINSCREEN  ".\\ART\\DOOM2.BMP"

// New Game button area
#define NEWGAME_LOCATION_X   15
#define NEWGAME_LOCATION_Y    0
#define NEWGAME_WIDTH       217
#define NEWGAME_HEIGHT       20

// Time Display area
#define TIME_LOCATION_X     420
#define TIME_LOCATION_Y       4
#define TIME_WIDTH          195
#define TIME_HEIGHT          16

/////////////////////////////////////////////////////////////////////////////

// CMainWindow:
// See game.cpp for the code to the member functions and the message map.
//
class CMainWindow : public CFrameWnd {
public:
	CMainWindow();
	void MovePlayer(CPoint point);
	void GetNewCursor();                // Load up the new directional cursor
	void NewGame();                     // Set up a new game & start the timer

//added data members:

	void SplashScreen();

private:
	void OnSoundNotify(CSound *pSound);

protected:
	virtual bool OnCommand(WPARAM wParam, LPARAM lParam) override;

	//{{AFX_MSG( CMainWindow )
	afx_msg void OnPaint();
	afx_msg void OnChar(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags);
	afx_msg void OnSysChar(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags);
	afx_msg void OnSysKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags);
	afx_msg void OnKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags);
	afx_msg void OnTimer(uintptr nIDEvent);
	afx_msg void OnLButtonDown(unsigned int nFlags, CPoint point);
	afx_msg void OnRButtonDown(unsigned int nFlags, CPoint point);
	afx_msg void OnLButtonUp(unsigned int nFlags, CPoint point);
	afx_msg void OnMouseMove(unsigned int nFlags, CPoint point);
	afx_msg void OnClose();
	afx_msg LRESULT OnMCINotify(WPARAM, LPARAM);
	afx_msg LRESULT OnMMIONotify(WPARAM, LPARAM);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

} // namespace MazeDoom
} // namespace HodjNPodj
} // namespace Bagel

#endif
