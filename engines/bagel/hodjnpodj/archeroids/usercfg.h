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

#ifndef HODJNPODJ_ARCHEROIDS_USERCFG_H
#define HODJNPODJ_ARCHEROIDS_USERCFG_H

#include "bagel/afxwin.h"
#include "bagel/hodjnpodj/hnplibs/stdinc.h"
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/hnplibs/button.h"

namespace Bagel {
namespace HodjNPodj {
namespace Archeroids {

#define IDD_USERCFG 100

#define DEFAULT_GAME_SPEED        4
#define DEFAULT_ARCHER_LEVEL      1
#define DEFAULT_LIVES             3
#define DEFAULT_BADGUYS          16

#define SPEED_MIN       1
#define SPEED_MAX      10
#define LEVEL_MIN       1
#define LEVEL_MAX       8
#define LIVES_MIN       1
#define LIVES_MAX       5
#define BADGUYS_MIN     1
#define BADGUYS_MAX    16

class CUserCfgDlg : public CBmpDialog {
public:

	CUserCfgDlg(CWnd *, CPalette *, unsigned int);

protected:

	virtual bool OnCommand(WPARAM, LPARAM) override;
	virtual bool OnInitDialog() override;
	void PutDlgData();
	void ClearDialogImage();

	void OnDestroy();
	void OnHScroll(unsigned int, unsigned int, CScrollBar *);
	void OnPaint();
	DECLARE_MESSAGE_MAP()

	//
	// user preference data
	//
	CText       *m_pTxtSpeed;
	CText       *m_pTxtSpeedSetting;
	CText       *m_pTxtLevel;
	CText       *m_pTxtLives;
	int          m_nInitGameSpeed;
	int          m_nInitArcherLevel;
	int          m_nInitNumLives;
	int          m_nInitNumBadGuys;
	bool         m_bShouldSave;           // True if we should save theses settings
};

} // namespace Archeroids
} // namespace HodjNPodj
} // namespace Bagel

#endif
