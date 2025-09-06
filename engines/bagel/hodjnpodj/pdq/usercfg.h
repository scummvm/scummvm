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

#ifndef HODJNPODJ_PDQ_USERCFG_H
#define HODJNPODJ_PDQ_USERCFG_H

#include "bagel/afxwin.h"
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/hnplibs/button.h"

namespace Bagel {
namespace HodjNPodj {
namespace PDQ {

#define IDD_USERCFG 100

#define SPEED_MIN     1
#define SPEED_DEF     7
#define SPEED_MAX    10
#define SHOWN_MIN     1
#define SHOWN_DEF     3
#define SHOWN_MAX     9

class CUserCfgDlg : public CBmpDialog {
public:
	CUserCfgDlg(CWnd* pParent = nullptr, CPalette *pPalette = nullptr, unsigned int = IDD_USERCFG);

protected:

	virtual void DoDataExchange(CDataExchange *) override;
	virtual bool OnCommand(WPARAM, LPARAM) override;
	virtual bool OnInitDialog() override;
	void PutDlgData();
	void GetDlgData();
	void DispSpeed();
	void DispShown();
	void ClearDialogImage();


	void OnHScroll(unsigned int, unsigned int, CScrollBar *);
	void OnPaint();
	void OnDestroy();
	DECLARE_MESSAGE_MAP()

	CScrollBar   *m_pSpeedScroll;
	CScrollBar   *m_pShownScroll;
	CCheckButton *m_pNamesButton;

	/*
	* user preference data
	*/
	unsigned int        m_nShown;               // initial number of letters to be revealed
	unsigned int        m_nGameSpeed;           // game speed 1..5 (1 = fast, 5 = slow)
	bool        m_bRandomLetters;       // true if letters to be revealed in random order
	bool        m_bShowNames;           // true if we shouldm show category names
	bool        m_bShouldSave;          // true if we should save theses values
};

} // namespace PDQ
} // namespace HodjNPodj
} // namespace Bagel

#endif
