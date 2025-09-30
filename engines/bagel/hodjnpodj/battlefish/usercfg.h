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

#ifndef HODJNPODJ_BATTLEFISH_USERCFG_H
#define HODJNPODJ_BATTLEFISH_USERCFG_H

#include "bagel/afxwin.h"
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/boflib/misc.h"

namespace Bagel {
namespace HodjNPodj {
namespace Battlefish {

#define IDD_USERCFG 100

#define DIFF_MIN    0
#define DIFF_MAX    2
#define DIFF_DEF    2

#define TURN_DEF    false

class CUserCfgDlg : public CBmpDialog {
public:
	CUserCfgDlg(CWnd *pParent = nullptr, CPalette *pPalette = nullptr, unsigned int = IDD_USERCFG);

protected:

	virtual bool OnCommand(WPARAM, LPARAM) override;
	virtual bool OnInitDialog() override;
	void PutDlgData();
	void GetDlgData();
	void DispLimit();
	void LoadIniSettings();
	void SaveIniSettings();

	void OnHScroll(unsigned int, unsigned int, CScrollBar *);
	void OnDestroy();
	void OnPaint();
	void ClearDialogImage();
	DECLARE_MESSAGE_MAP()

	CColorButton *m_pOKButton = nullptr;		// OKAY button on scroll
	CColorButton *m_pCancelButton = nullptr;	// Cancel button on scroll
	CColorButton *m_pDefaultsButton = nullptr;	// Defaults button on scroll
	CRadioButton *m_pUserButton = nullptr;
	CRadioButton *m_pCompButton = nullptr;
	CText        *m_pTxtDifficulty = nullptr;
	CText        *m_pTxtLevel = nullptr;
	CText        *m_pTxtOrder = nullptr;
	CScrollBar   *m_pScrollBar = nullptr;
	unsigned int m_nDifficultyLevel = 0;		// Difficulty level for BattleFish
	bool m_bSave = false;						// True if should save theses settings
	bool m_bUserGoesFirst = false;				// true if Human player goes 1st
};

} // namespace Battlefish
} // namespace HodjNPodj
} // namespace Bagel

#endif
