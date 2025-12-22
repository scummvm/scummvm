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

#ifndef HODJNPODJ_ARTPARTS_ARTPARTS_H
#define HODJNPODJ_ARTPARTS_ARTPARTS_H

#include "bagel/boflib/sound.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/hodjnpodj/hnplibs/text.h"

namespace Bagel {
namespace HodjNPodj {
namespace ArtParts {

// Border info
#define SIDE_BORDER      20
#define TOP_BORDER       28 //32
#define BOTTOM_BORDER    20 //16
#define HILITE_BORDER     3

// Art Parts constants
#define ART_WIDTH   600
#define ART_HEIGHT  432

// For the "frame" of correct pieces around the artwork:
#define FRAME_WIDTH       4
#define FRAME_HEIGHT      4

// Starting value defaults
#define MAX_COLUMNS      30
#define MIN_COLUMNS       1

#define MAX_ROWS         24
#define MIN_ROWS          1

#define START_COLUMNS       4
#define START_ROWS          4

// Timer constants
#define MIN_TIME              0                     //  15 Seconds
#define MAX_TIME            600                     // 180 Seconds = 3 minute max
#define TIMER_START           0                     // Increment scrollbar in steps of 15 Secs
#define TIMER_MAX            12 // if Time > MAX_TIME, No Time Limit

#define DISPLAY_TIMER           1
#define SHOW_TIMER              2
#define MS_SCALE             1000       // Scale seconds to milliseconds

#define CLICK_TIME           1000       // Every Second, update timer clock
#define PAUSE_TIME           3000       // Wait three seconds before erasing hint screen

// Sound files
#define PICK_SOUND      ".\\sound\\pickart.wav"
#define SWITCH_SOUND    ".\\sound\\moveart.wav"
#define WIN_SOUND       ".\\sound\\sound146.wav"
#define UNDO_SOUND      ".\\sound\\sound355.wav"
#define LOSE_SOUND      ".\\sound\\buzzer.wav"
#define RULES_WAV       ".\\sound\\artparts.wav"

#define GAME_THEME      ".\\sound\\artparts.mid"

// Backdrop bitmaps
#define MAINSCREEN      ".\\ART\\ARTPART1.BMP"
#define FRAMESCREEN     ".\\ART\\ARTPART1.BMP"
#define TEXTSCREEN      ".\\ART\\ARTTEMP.BMP"

#define DATA_FILE       "artfiles.dat"
#define MAX_FILE_LENGTH          20     // Longest Art file name length allowed

// New Game button area
#define NEWGAME_LOCATION_X   70
#define NEWGAME_LOCATION_Y    5
#define NEWGAME_WIDTH       113
#define NEWGAME_HEIGHT       13

// Time Display area
#define TIME_LOCATION_X 457
#define TIME_LOCATION_Y   4
#define TIME_WIDTH      117
#define TIME_HEIGHT      16
/////////////////////////////////////////////////////////////////////////////

// CMainWindow:
// See game.cpp for the code to the member functions and the message map.
//
class CMainWindow : public CFrameWnd {
public:
	CMainWindow();
	~CMainWindow();

	bool LoadArtWork();
	void DrawPart(const CPoint &Src, const CPoint &Dst, int nWidth, int nHeight);
	void SwitchAreas(const CRect &Src, const CRect &Dst);
	void InitValues();
	void NewGame();
	void CheckForWin();
	void ShowOutOfPlace();

//added data members:
	bool m_bPlaying = false;	// Flag True if playing, False if setting options
	bool m_bNewGame = false;	// Flag to check if a new game is being played
	bool m_bFirst = false;		// Flag to check if the first area is being selected
	CPoint First;
	CPoint Second;
	CPoint Center;
	CPoint UpLeft;
	CRect BaseRect;				// The part that is the base of the select area
	CRect OldRect;				// The last area highlighted
	CRect HiLiteRect;			// The new area to highlight
	CRect SrcRect;				// The area to be moved
	CRect DstRect;				// The destination of the moving area

	void SplashScreen();
	void SplashScratch();
	void SplashScratchPaint();

private:
	POINT Grid[MAX_COLUMNS][MAX_ROWS];          // Location of the art parts

	CBmpButton *m_pScrollButton = nullptr;
	CBitmap *pScratch1 = nullptr,              // Off-screen bitmap of current positions
		*pScratch2 = nullptr,              // Off-screen bitmap of new positions
		*pOldBmp1 = nullptr,
		*pOldBmp2 = nullptr;
	CPalette *pOldPal1 = nullptr,
		*pOldPal2 = nullptr;
	CDC *pScratch1DC = nullptr,
		*pScratch2DC = nullptr;
	CText *m_pTimeText = nullptr;            // Time to be posted in Locale box of screen
	CBitmap *pLocaleBitmap = nullptr,          // Locale of game bitmap for title bar
		*pBlankBitmap = nullptr;           // Blank area of locale for time display

	bool bStartOkay = true;
	bool bGameStarted = false;		// becomes true at start time, false at game end
	bool bSwitched = false;			// flag for undo -- only true after a part switch
	bool bSuccess;
	bool m_bIgnoreScrollClick;
	bool m_bShowOutOfPlace = false;
	char szCurrentArt[64] = {};

	static CPalette *pGamePalette;	// Palette of current artwork
	static int nSeconds;
	static int nMinutes;
	static int nLastPick;
	static int m_nTime;				// Time is in SECONDS
	static int m_nRows;				// Number of rows in the artwork grid
	static int m_nColumns;			// Number of columns in the artwork grid
	static int m_nWidth;			// The Width of each Part
	static int m_nHeight;			// The Height of each Part
	static float m_nScore;			// The current percentage of correctly placed parts
	static bool bFramed;			// Framed (hint) mode is turned off by default

	static int tempTime;			// temporary holding places
	static int tempRows;			//...for options changes,
	static int tempColumns;			//...which only get used
	static bool tempFramed;			//...when NewGame is called.

	CSound *pGameSound = nullptr;	// Game theme song

private:
	void initStatics();
	void OnSoundNotify(CSound *pSound);
	static bool CopyPaletteContents(CPalette *pSource, CPalette *pDest);
	void MyFocusRect(CDC *pDC, CRect rect, int nDrawMode);
	static void GetSubOptions(CWnd *pParentWind);

protected:
	virtual bool OnCommand(WPARAM wParam, LPARAM lParam) override;

	//{{AFX_MSG( CMainWindow )
	afx_msg void OnPaint();
	afx_msg void OnChar(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags);
	afx_msg void OnSysKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags);
	afx_msg void OnTimer(uintptr nIDEvent);
	afx_msg void OnLButtonDown(unsigned int nFlags, CPoint point);
	afx_msg void OnRButtonDown(unsigned int nFlags, CPoint point);
	afx_msg void OnLButtonUp(unsigned int nFlags, CPoint point);
	afx_msg void OnMButtonDown(unsigned int nFlags, CPoint point);
	afx_msg void OnMouseMove(unsigned int nFlags, CPoint point);
	afx_msg void OnKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags);
	afx_msg void OnClose();
	afx_msg LRESULT OnMCINotify(WPARAM, LPARAM);
	afx_msg LRESULT OnMMIONotify(WPARAM, LPARAM);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

} // namespace ArtParts
} // namespace HodjNPodj
} // namespace Bagel

#endif
