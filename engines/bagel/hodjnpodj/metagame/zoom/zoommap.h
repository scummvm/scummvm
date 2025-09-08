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

#ifndef BAGEL_METAGAME_ZOOM_ZOOMMAP_H
#define BAGEL_METAGAME_ZOOM_ZOOMMAP_H

#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/metagame/zoom/resource.h"
#include "bagel/hodjnpodj/hnplibs/dibdoc.h"
#include "bagel/hodjnpodj/hnplibs/gamedll.h"
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"            // header for the options library
#include "bagel/hodjnpodj/hnplibs/mainmenu.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/hodjnpodj/hnplibs/rules.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/hnplibs/bitmaps.h"
#include "bagel/hodjnpodj/hnplibs/sprite.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Zoom {

// Button Identifier codes (BIDs)
#define IDC_OPTION          100
#define IDC_JUNK            999

// Button positioning constants
#define OPTION_WIDTH                146
#define OPTION_HEIGHT               23
#define OPTION_LEFT                 246
#define OPTION_TOP                  0

class CMainZoomWindow : public CFrameWnd {
private:
	HWND m_hCallAppWnd = (HWND)0;
	bool m_bShowExit = false;
	bool _isDemo = false;
	CRect MainRect;                           // screen area spanned by the game window
	CRect OptionRect;                         // screen area spanned by the option button

	CRect rText;
	CRect rLeaveRect;

	int rectIndexOf(const CPoint &pt);

public:
	CMainZoomWindow(HWND, bool);

	void SplashScreen();

	static void ReleaseResources();
	static void FlushInputEvents();

protected:
	bool OnCommand(WPARAM wParam, LPARAM lParam) override;

	//{{AFX_MSG( CMainPackRatWindow )
	afx_msg void OnPaint();
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(uintptr);
	afx_msg void OnLButtonDown(unsigned int, CPoint);
	afx_msg void OnLButtonUp(unsigned int, CPoint);
	afx_msg void OnMouseMove(unsigned int, CPoint);
	afx_msg void OnKeyDown(unsigned int, unsigned int, unsigned int);
	afx_msg bool OnEraseBkgnd(CDC *);
	afx_msg void OnActivate(unsigned int nState, CWnd   *pWndOther, bool bMinimized) override;
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

} // namespace Zoom
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
