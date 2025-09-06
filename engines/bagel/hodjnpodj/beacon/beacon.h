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

#ifndef HODJNPODJ_BEACON_BEACON_H
#define HODJNPODJ_BEACON_BEACON_H

#include "bagel/boflib/sound.h"

namespace Bagel {
namespace HodjNPodj {
namespace Beacon {

#define ABS(a)      ( (a > 0) ? (a) : (-a) )

// Border info
#define TOP_BORDER       24 //25
#define BOTTOM_BORDER    16
#define SIDE_BORDER      16

#define BEACON_TIMER      1
#define MIN_SPEED         0
#define MAX_SPEED         2
#define SPEED_BASE       70
#define SPEED_STEP       10

#define NUM_BEAMS       120
#define CHANGE_BEAM     144
#define START_ENTRY      55     // Starting entry of available palette slots
#define BUTTON_ENTRY    176     // First entry of the Button palette slots

#define GRID_WIDTH        8     //5     // Number of pixels
#define GRID_HEIGHT       8     //5

// Game playing area
#define ART_WIDTH       488
#define ART_HEIGHT      440

#define NUM_BUTTONS      16     // Number of buttons
#define NUM_COLUMNS      (ART_WIDTH / GRID_WIDTH)
#define NUM_ROWS         (ART_HEIGHT / GRID_HEIGHT)

// Lighthouse bitmap placement
#define LTHOUSE_OFFSET_X    20
#define LTHOUSE_OFFSET_Y    15
#define LTHOUSE_WIDTH       50
#define LTHOUSE_HEIGHT      70

// Start area
#define START_OFFSET_X      36      // GRID_WIDTH * 4.5
#define START_OFFSET_Y      28      // GRID_HEIGHT * 3.5
#define START_WIDTH         72      // GRID_WIDTH * 9
#define START_HEIGHT        88      // GRID_HEIGHT * 11

// Color Button placement
#define BLOCK_SPACE_X         6
#define BLOCK_SPACE_Y         6
#define BLOCK_OFFSET_X      (ART_WIDTH + SIDE_BORDER + BLOCK_SPACE_X)
#define BLOCK_OFFSET_Y       74         // TOP_BORDER + 50
#define BLOCK_WIDTH          50
#define BLOCK_HEIGHT         40

#define SELECT_COLOR          0         // Always use Black for highlighting selected beacon color
#define HILITE_BORDER         4         // The highlighting frame is three pixels wide
#define XTRALITE_TRIM        42         // Color Index of lightest trim color
#define LITE_TRIM            30         // Color Index of light trim color
#define MEDIUM_TRIM          45         // Color Index of medium trim color
#define DARK_TRIM            46         // Color Index of dark trim color

// Starting value defaults
#define MAX_SWEEPS          300
#define MIN_SWEEPS            5

// Sound files
#define MID_SOUND   ".\\sound\\bong.wav"
#define PICK_SOUND  ".\\sound\\pick.wav"
#define LOSE_SOUND  ".\\sound\\buzzer.wav"
#define WIN_SOUND   ".\\SOUND\\SOUNDWON.WAV"

#define GAME_THEME  ".\\sound\\beacon.mid"
#define RULES_WAV   ".\\SOUND\\BEACON.WAV"
#define RULES_TEXT  "BEACON.TXT"

// Backdrop bitmaps
#define MAINSCREEN  ".\\ART\\BEACBRDR.BMP"
#define BEAMSCREEN  ".\\ART\\BEAMS3.BMP"

// Artwork data file constants
#define DATA_FILE   ".\\BEACON.DAT"
#define MAX_FILE_LENGTH      20

// New Game button area
#define NEWGAME_LOCATION_X   15
#define NEWGAME_LOCATION_Y    0
#define NEWGAME_WIDTH       217
#define NEWGAME_HEIGHT       20

/////////////////////////////////////////////////////////////////////////////

// CMainWindow:
// See game.cpp for the code to the member functions and the message map.
//
class CMainWindow : public CFrameWnd {
private:
	struct BLOCK {
		CRect   rLocation;
		unsigned int    nColorIndex;
	} colorBlock[NUM_BUTTONS];

	void initStatics();

public:
	CMainWindow();
	void NewGame();
	void DrawBeams(CDC *pDC);
	void MoveBeam();
	void CheckUnderBeam();
	void SetBeamEntries(CDC *pDC);
	static void DrawPart(CDC *pDC, CPoint Src, CPoint Dst, int nWidth, int nHeight);
	static bool CompareColors(CDC *pDC, CPoint point);
	static bool UnderLighthouse(CPoint point);
	static bool InPictureSquare(CPoint point);
	static CPoint PointToGrid(CPoint point);
	bool LoadArtWork(CDC *pDC);

//added data members:
private:
	void OnSoundNotify(CSound *pSound);

protected:
	virtual bool OnCommand(WPARAM wParam, LPARAM lParam) override;

	//{{AFX_MSG( CMainWindow )
	afx_msg void OnPaint();
	afx_msg void OnChar(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags);
	afx_msg void OnSysKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags);
	afx_msg void OnKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags);
	afx_msg void OnTimer(uintptr nIDEvent);
	afx_msg void OnLButtonDown(unsigned int nFlags, CPoint point);
	afx_msg void OnLButtonUp(unsigned int nFlags, CPoint point);
	afx_msg void OnMouseMove(unsigned int nFlags, CPoint point);
	afx_msg void OnClose();
	afx_msg LRESULT OnMCINotify(WPARAM, LPARAM);
	afx_msg LRESULT OnMMIONotify(WPARAM, LPARAM);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

} // namespace Beacon
} // namespace HodjNPodj
} // namespace Bagel

#endif
