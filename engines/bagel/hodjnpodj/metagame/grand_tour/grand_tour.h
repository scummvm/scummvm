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

#ifndef BAGEL_METAGAME_GTR_GRAND_TOUR_H
#define BAGEL_METAGAME_GTR_GRAND_TOUR_H

#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/metagame/grand_tour/resource.h"
#include "bagel/hodjnpodj/hnplibs/dibdoc.h"
#include "bagel/hodjnpodj/hnplibs/gamedll.h"
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"            // header for the options library
#include "bagel/hodjnpodj/metagame/grand_tour/gtstruct.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/hodjnpodj/hnplibs/bitmaps.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace GrandTour {

// Button Identifier codes (BIDs)
#define IDC_JUNK                999

#define IDC_PLAY                100
#define IDC_SAVE                101
#define IDC_RESTORE             102
#define IDC_LEAVE               103
#define IDC_RESET_SCORES        104

#define IDC_AUDIO               105
#define IDC_TOP10               106

#define IDC_HODJ_SKILL_HARD     110
#define IDC_HODJ_SKILL_MEDIUM   111
#define IDC_HODJ_SKILL_EASY     112
#define IDC_HODJ_NO_PLAY        113

#define IDC_PODJ_SKILL_HARD     120
#define IDC_PODJ_SKILL_MEDIUM   121
#define IDC_PODJ_SKILL_EASY     122
#define IDC_PODJ_NO_PLAY        123

#define IDC_GAME_ALPHA          130
#define IDC_GAME_GEO            131
#define IDC_GAME_RAND           132

#define SCROLL_WIDTH            502
#define SCROLL_HEIGHT           395

#define SCROLL_LEFT             ( GAME_WIDTH / 2 ) - ( SCROLL_WIDTH / 2 )
#define SCROLL_TOP              ( GAME_HEIGHT / 2 ) - ( SCROLL_HEIGHT / 2 )

struct SCORESTRUCT {
	char        acName[64];
	int         nSkillLevel;
	int         nScore;
};

/////////////////////////////////////////////////////////////////////////////

// CMainPackRatWindow:
// See game.cpp for the code to the member functions and the message map.
//

class CMainGTWindow : public CFrameWnd {
private:
	HWND m_hCallAppWnd;
	LPGRANDTRSTRUCT m_pgtGTStruct;
	CRect MainRect;                           // screen area spanned by the game window
	CRect rCharRect;
	CRect cTextRect;
	const CRect   SplashRect;
	const CRect   ScoresLeaveRect;
	const CRect   ScoresResetRect;
	const CRect   PlayRect;
	const CRect   SaveRect;
	const CRect   RestoreRect;
	const CRect   AudioRect;
	const CRect   Top10Rect;
	const CRect   LeaveRect;
	const CRect   HSHRect, HSMRect, HSLRect, HSNPRect;
	const CRect   PSHRect, PSMRect, PSLRect, PSNPRect;
	const CRect   GARect, GGRect, GRRect;

	void initStatics();

public:
	CMainGTWindow(HWND, LPGRANDTRSTRUCT);
	~CMainGTWindow();

	void SplashScreen();
	void AdjustScore();
	void GetScores();
	void SaveScores();
	void ResetScores();
	void SaveCurrGame();
	void RestoreGame();
	int GetNextGameCode(bool = true);

	static  void ReleaseResources();
	static  void FlushInputEvents();

private:

protected:
	virtual bool OnCommand(WPARAM wParam, LPARAM lParam) override;

	//{{AFX_MSG( CMainPackRatWindow )
	afx_msg void OnPaint();
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDown(unsigned int, CPoint);
	afx_msg void OnLButtonUp(unsigned int, CPoint);
	afx_msg void OnMouseMove(unsigned int, CPoint);
	afx_msg void OnKeyDown(unsigned int, unsigned int, unsigned int);
	afx_msg void OnChar(unsigned int, unsigned int, unsigned int);
	afx_msg bool OnEraseBkgnd(CDC *);
	afx_msg void OnActivate(unsigned int nState, CWnd   *pWndOther, bool bMinimized) override;
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

} // namespace GrandTour
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
