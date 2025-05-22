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

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Frame {


// Button Identifier codes (BIDs)
#define IDC_ZOOM		888
#define IDC_META		887
#define IDC_QUIT		886
#define IDC_MAINDLG 	885
#define IDC_GRANDTOUR	884
// Sound File Identifiers

// Bitmap Identifiers
#define MAINSCROLL	".\\ART\\MLSCROLL.BMP" 

// Button positioning constants

/////////////////////////////////////////////////////////////////////////////

// CMainPokerWindow:
// See game.cpp for the code to the member functions and the message map.
//              

class CHodjPodjWindow : public CFrameWnd {
private:
    CBitmap *m_pCurrentBmp;
    UINT     m_nFlags;
    BOOL     m_bInCredits;

public:
    CHodjPodjWindow();

	void BlackScreen(void);

	void FreeCurrentDLL(void);
	BOOL LoadMetaDLL(void);
	BOOL LoadZoomDLL(void);
	BOOL LoadGrandTourDLL(void);

    void HandleError(ERROR_CODE);

    void PlayMovie(const int, const char *, BOOL);

	void GetProfilePath(void);
	void GetHomePath(void);
	BOOL GetCDPath(void);
	BOOL PositionAtHomePath(void);
	BOOL PositionAtMiniPath(int);
	BOOL PositionAtCDPath(void);
	BOOL FindCDROM(void);
	BOOL DriveWriteLocked(void);

	long DetermineChallengeScore(void);
	LPARAM UpdateChallengePhase(LPARAM);
	void SetComputerScore(void);

	void GetProfileSettings(void);
	void SaveProfileSettings(void);
	
	BOOL CheckLowMemory(void);

    BOOL Restore(VOID);
    VOID ShowCredits(VOID);

	static  void ReleaseResources(void);
	static  void FlushInputEvents(void);

private:
	BOOL CheckConfig(CDC *pDC);
	BOOL SetupNewMeta(void);
	void LoadNewDLL( LPARAM );
	void UpdateDLLRouting(void);
	void StartBackgroundMidi(void);
	void StopBackgroundMidi(void);

protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

    //{{AFX_MSG( CHodjPodjWindow )
    afx_msg void OnPaint();
    afx_msg void OnClose();
    afx_msg void OnDestroy();
    afx_msg void OnMouseMove( UINT, CPoint );
    afx_msg void OnLButtonDown( UINT, CPoint );
    afx_msg void OnRButtonDown( UINT, CPoint );
    afx_msg long OnMCINotify( WPARAM, LPARAM);
    afx_msg BOOL OnEraseBkgnd( CDC * );
    afx_msg void OnParentNotify( UINT, LPARAM);
    afx_msg void OnActivate( UINT, CWnd *, BOOL );
    afx_msg void OnKeyDown( UINT, UINT, UINT );
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()
};

/**
 * Main app class for the game
 */
class CTheApp : public CWinApp {
public:
    BOOL InitInstance() override;
    int ExitInstance() override;
};

extern CTheApp theApp;

} // namespace Frame
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
