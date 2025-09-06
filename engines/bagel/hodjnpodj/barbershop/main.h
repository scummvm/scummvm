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
	CMainWindow(void);
	void        PlayGame(void);
	void        PaintScreen(void);
	void        LoadIniSettings(void);
	void        SaveIniSettings(void);
	void        FlushInputEvents();
	LRESULT     OnMCINotify(WPARAM, LPARAM);
	LRESULT     OnMMIONotify(WPARAM, LPARAM);

protected:
	void        GameReset(void);
	void        GamePause(void);
	void        GameResume(void);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
	void        HandleError(ERROR_CODE);
	void        DeleteSprite(CSprite *);

	void        OnPaint(void);
	void        OnTimer(UINT_PTR);
	void        OnMouseMove(UINT, CPoint);
	void        OnLButtonDown(UINT, CPoint);
	void        OnLButtonDblClk(UINT, CPoint);
	void        OnRButtonDown(UINT, CPoint);
	void        OnSysChar(UINT, UINT, UINT);
	void        OnSysKeyDown(UINT, UINT, UINT);
	void        OnKeyDown(UINT, UINT, UINT);
	void        OnActivate(UINT, CWnd *, BOOL) override;
	void        OnClose(void);
	void        OnLButtonUp(UINT, CPoint);

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
