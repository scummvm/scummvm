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

#ifndef HODJNPODJ_PDQ_MAIN_H
#define HODJNPODJ_PDQ_MAIN_H

#include "bagel/afxwin.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/hnplibs/sprite.h"
#include "bagel/boflib/error.h"
#include "bagel/hodjnpodj/hnplibs/stdinc.h"
#include "bagel/boflib/sound.h"
#include "bagel/hodjnpodj/pdq/guess.h"

namespace Bagel {
namespace HodjNPodj {
namespace PDQ {

#define N_CATEGORIES    4

class CMainWindow : public CFrameWnd {
public:
	CMainWindow();
	VOID        PlayGame(void);
	VOID        PaintScreen(VOID);
	VOID        PaintCategory(INT);
	VOID        EraseCategory(VOID);

	int         m_nTurnCount;

protected:
	ERROR_CODE  LoadCategoryNames(VOID);
	VOID        ReleaseCategoryNames(VOID);

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
	VOID HandleError(ERROR_CODE);
	VOID OnSoundNotify(CSound *pSound);

	void OnPaint(void);
	void OnChar(UINT, UINT, UINT);
	void OnSysChar(UINT, UINT, UINT);
	void OnKeyDown(UINT, UINT, UINT);
	void OnSysKeyDown(UINT, UINT, UINT);
	void OnMouseMove(UINT, CPoint);
	void OnLButtonDown(UINT, CPoint);
	void OnActivate(UINT, CWnd *, BOOL);
	void OnSetFocus(CWnd *);
	void OnClose(void);
	long OnMCINotify(WPARAM, LPARAM);
	long OnMMIONotify(WPARAM, LPARAM);

	DECLARE_MESSAGE_MAP()

	CRect        m_rNewGameButton;
	CSprite     *m_pCategories[N_CATEGORIES];
	CSprite     *m_pScrollSprite;
	CGuessDlg   *m_pDlgGuess;
	CSound      *m_pSoundTrack;
	INT          m_iLastType;
	BOOL         m_bInMenu;
	BOOL         m_bInGuess;
};

} // namespace PDQ
} // namespace HodjNPodj
} // namespace Bagel

#endif
