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

#ifndef HODJNPODJ_BARBERSHOP_MAIN_H
#define HODJNPODJ_BARBERSHOP_MAIN_H

#include "bagel/afxwin.h"
#include "bagel/hodjnpodj/hnplibs/stdinc.h"
#include "bagel/hodjnpodj/hnplibs/sprite.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/boflib/error.h"
#include "bagel/boflib/sound.h"
#include "bagel/hodjnpodj/barbershop/barb.h"
#include "bagel/hodjnpodj/barbershop/animate.h"

namespace Bagel {
namespace HodjNPodj {
namespace Barbershop {

class CMainWindow : public CFrameWnd {
private:
	CBarber     *m_pBarb;
	CSound      *m_pMIDISound;
	CSound      *m_pWavSound;
	CAnimate    *m_pAnim;
	BOOL         m_bMIDIPlaying;

public:
	CMainWindow(VOID);
	VOID        PlayGame(VOID);
	VOID        PaintScreen(VOID);
	VOID        LoadIniSettings(VOID);
	VOID        SaveIniSettings(VOID);
	void        FlushInputEvents();
	LRESULT     OnMCINotify(WPARAM, LPARAM);
	LRESULT     OnMMIONotify(WPARAM, LPARAM);

protected:
	VOID        GameReset(VOID);
	VOID        GamePause(VOID);
	VOID        GameResume(VOID);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
	VOID        HandleError(ERROR_CODE);
	VOID        DeleteSprite(CSprite *);

	VOID        OnPaint(VOID);
	VOID        OnTimer(UINT_PTR);
	VOID        OnMouseMove(UINT, CPoint);
	VOID        OnLButtonDown(UINT, CPoint);
	VOID        OnLButtonDblClk(UINT, CPoint);
	VOID        OnRButtonDown(UINT, CPoint);
	VOID        OnSysChar(UINT, UINT, UINT);
	VOID        OnSysKeyDown(UINT, UINT, UINT);
	VOID        OnKeyDown(UINT, UINT, UINT);
	VOID        OnActivate(UINT, CWnd *, BOOL);
	VOID        OnClose(VOID);
	VOID        OnLButtonUp(UINT, CPoint);

	DECLARE_MESSAGE_MAP()

	CRect       m_rNewGameButton;
	CPalette   *m_pGamePalette;
	CSprite    *m_pScrollSprite;
	bool     m_bGameActive;
	bool     m_bPause;
	bool     m_bInMenu;
};

} // namespace Barbershop
} // namespace HodjNPodj
} // namespace Bagel

#endif
