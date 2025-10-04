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
	~CMainWindow() override {}

	void        PlayGame();
	void        PaintScreen();
	void        PaintCategory(int);
	void        EraseCategory();

	int         m_nTurnCount = 0;

protected:
	ERROR_CODE  LoadCategoryNames();
	void        ReleaseCategoryNames();

	virtual bool OnCommand(WPARAM wParam, LPARAM lParam) override;
	void HandleError(ERROR_CODE);
	void OnSoundNotify(CSound *pSound);

	void OnPaint();
	void OnChar(unsigned int, unsigned int, unsigned int);
	void OnSysChar(unsigned int, unsigned int, unsigned int);
	void OnKeyDown(unsigned int, unsigned int, unsigned int);
	void OnSysKeyDown(unsigned int, unsigned int, unsigned int);
	void OnMouseMove(unsigned int, CPoint);
	void OnLButtonDown(unsigned int, CPoint);
	void OnActivate(unsigned int, CWnd *, bool) override;
	void OnSetFocus(CWnd *);
	void OnClose();
	LRESULT OnMCINotify(WPARAM, LPARAM);
	LRESULT OnMMIONotify(WPARAM, LPARAM);

	DECLARE_MESSAGE_MAP()

	CRect m_rNewGameButton;
	CSprite *m_pCategories[N_CATEGORIES] = {};
	CSprite *m_pScrollSprite = nullptr;
	CGuessDlg *m_pDlgGuess = nullptr;
	CSound *m_pSoundTrack = nullptr;
	int m_iLastType = 0;
	bool m_bInMenu = false;
	bool m_bInGuess = false;
};

} // namespace PDQ
} // namespace HodjNPodj
} // namespace Bagel

#endif
