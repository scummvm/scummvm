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

#ifndef BAGEL_METAGAME_FRAME_HODJPODJ_H
#define BAGEL_METAGAME_FRAME_HODJPODJ_H

#include "bagel/hodjnpodj/globals.h"
#include "bagel/boflib/error.h"
#include "bagel/hodjnpodj/hnplibs/bitmaps.h"
#include "bagel/hodjnpodj/hnplibs/dibdoc.h"
#include "bagel/hodjnpodj/hnplibs/dibapi.h"
#include "bagel/hodjnpodj/hnplibs/gamedll.h"
#include "bagel/hodjnpodj/hnplibs/stdinc.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/metagame/frame/resource.h"
#include "bagel/hodjnpodj/metagame/frame/movytmpl.h"
#include "bagel/hodjnpodj/metagame/bgen/bfc.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Frame {

// Button Identifier codes (BIDs)
#define IDC_ZOOM        888
#define IDC_META        887
#define IDC_QUIT        886
#define IDC_MAINDLG     885
#define IDC_GRANDTOUR   884
// Sound File Identifiers

// Bitmap Identifiers
#define MAINSCROLL  ".\\ART\\MLSCROLL.BMP"

extern CBfcMgr *lpMetaGame;

extern void InitBFCInfo(CBfcMgr *pBfcMgr);

/////////////////////////////////////////////////////////////////////////////

// CHodjPodjWindow:
// See game.cpp for the code to the member functions and the message map.
//

class CHodjPodjWindow : public CFrameWnd {
private:
	CMovieWindow pMovie;
	CBitmap *m_pCurrentBmp;
	unsigned int m_nFlags;
	bool m_bInCredits;

public:
	CHodjPodjWindow();

	void BlackScreen();

	void FreeCurrentDLL();
	bool LoadMetaDLL();
	bool LoadZoomDLL();
	bool LoadGrandTourDLL();

	void HandleError(ERROR_CODE);

	void PlayMovie(const int, const char *, bool);

	void GetProfilePath();
	void GetHomePath();
	bool GetCDPath();
	bool PositionAtHomePath();
	bool PositionAtMiniPath(int);
	bool PositionAtCDPath();
	bool FindCDROM();
	bool DriveWriteLocked();

	long DetermineChallengeScore();
	LPARAM UpdateChallengePhase(LPARAM);
	void SetComputerScore();

	void GetProfileSettings();
	void SaveProfileSettings();

	bool CheckLowMemory();

	bool Restore();
	void ShowCredits();

	static  void ReleaseResources();
	static  void FlushInputEvents();

private:
	bool CheckConfig(CDC *pDC);
	bool SetupNewMeta();
	void LoadNewDLL(LPARAM);
	void UpdateDLLRouting();
	void StartBackgroundMidi();
	void StopBackgroundMidi();

protected:
	virtual bool OnCommand(WPARAM wParam, LPARAM lParam) override;

	//{{AFX_MSG( CHodjPodjWindow )
	afx_msg void OnPaint();
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnMouseMove(unsigned int, CPoint);
	afx_msg void OnLButtonDown(unsigned int, CPoint);
	afx_msg void OnRButtonDown(unsigned int, CPoint);
	afx_msg LRESULT OnMCINotify(WPARAM, LPARAM);
	afx_msg bool OnEraseBkgnd(CDC *);
	afx_msg void OnParentNotify(unsigned int, LPARAM);
	afx_msg void OnActivate(unsigned int, CWnd *, bool) override;
	afx_msg void OnKeyDown(unsigned int, unsigned int, unsigned int);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

} // namespace Frame
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
