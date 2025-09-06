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

#ifndef HODJNPODJ_RIDDLES_USERCFG_H
#define HODJNPODJ_RIDDLES_USERCFG_H

#include "bagel/afxwin.h"
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/hnplibs/stdinc.h"

namespace Bagel {
namespace HodjNPodj {
namespace Riddles {

#define IDD_USERCFG 100

#define LIMIT_MIN           10
#define LIMIT_MAX          301
#define LIMIT_DEF           30

#define LEVEL_MIN            0
#define LEVEL_MAX            3
#define LEVEL_DEF            1

class CUserCfgDlg : public CBmpDialog {
public:
	CUserCfgDlg(CWnd *pParent = nullptr, CPalette *pPalette = nullptr, unsigned int = IDD_USERCFG);
	void ClearDialogImage();

protected:

	virtual void DoDataExchange(CDataExchange *) override;
	virtual bool OnCommand(WPARAM, LPARAM) override;
	virtual bool OnInitDialog() override;
	void PutDlgData();
	void GetDlgData();
	void DispLimit();
	void DispLevel();
	int GetIndex(int);

	void OnHScroll(unsigned int, unsigned int, CScrollBar *);
	void OnClose();
	void OnPaint();
	void OnLButtonDown(unsigned int, CPoint);
	DECLARE_MESSAGE_MAP()

	CText *m_pTxtTimeLimit;
	CText *m_pTxtLimit;
	CText *m_pTxtLevel;
	CScrollBar *m_pScrollBar;
	CScrollBar *m_pLevelScroll;
	unsigned int        m_nTimeLimit;
	int         m_nDifficultyLevel;
	bool        m_bSave;                    // True if should save theses values
};

} // namespace Riddles
} // namespace HodjNPodj
} // namespace Bagel

#endif
