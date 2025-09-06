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

#ifndef HODJNPODJ_LIFE_USERCFG_H
#define HODJNPODJ_LIFE_USERCFG_H

#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/life/resource.h"

namespace Bagel {
namespace HodjNPodj {
namespace Life {

#define LIFE_SETTINGS   16
#define MIN_LIFE        0
#define MAX_LIFE        15
#define LIFE_HI_BOUND   201

#define SPEED           9
#define MIN_SPEED       0
#define MAX_SPEED       8

#define TURNS_SETTINGS  16
#define MIN_TURNS       0
#define MAX_TURNS       15
#define TURN_HI_BOUND   201

#define OPTIONS_COLOR   RGB(0, 0, 0)    // Color of the stats info CText
#define FONT_SIZE       14

#define INI_FNAME       "HODJPODJ.INI"     // INI saved Life settings
#define INI_SECTION     "Life"

#define ROUNDS_DEF      100             // default skill levels
#define VILLAGES_DEF    50
#define SPEED_DEF       1
#define PLACE_DEF       0

#define VILLAGES_LOW    30              // meta game skill levels
#define VILLAGES_MED    25
#define VILLAGES_HI     20
#define ROUNDS_META     20

class CUserCfgDlg : public CBmpDialog {
private:
	CString     mSpeedTable[SPEED];
	int         m_nLife[LIFE_SETTINGS];
	int         m_nTurns[TURNS_SETTINGS];
	CText       *m_pVillages,
	            *m_pSpeed,
	            *m_pRounds;
public:
	int         nSpeedTemp,             // remembers cur settings
	            nTurnCounterTemp,
	            nLifeTemp;
	bool        bPrePlaceTemp;

	CUserCfgDlg(CWnd *pParent = nullptr, CPalette *pPalette = nullptr, unsigned int = IDD_USERCFG);

	enum { IDD = IDD_USERCFG };
	CScrollBar      m_ScrollVillages;
	CScrollBar      m_ScrollRounds;
	CScrollBar      m_ScrollSpeed;
	CString         m_DisplayRounds;
	CString         m_DisplaySpeed;
	CString         m_DisplayVillages;
	CCheckButton    *m_pPrePlaceButton;                 // PrePlace check box
	CColorButton    *m_pOKButton;                       // OKAY button on scroll
	CColorButton    *m_pCancelButton;                   // Cancel button on scroll

protected:
	void ClearDialogImage();
	virtual void DoDataExchange(CDataExchange*) override;    // DDX/DDV support
	virtual bool OnInitDialog() override;
	virtual void OnOK() override;
	virtual void OnCancel() override;
	afx_msg void OnPrePlace();
	afx_msg void OnPaint();
	afx_msg bool OnEraseBkgnd(CDC *pDC);
	afx_msg void OnHScroll(unsigned int nSBCode, unsigned int nPos, CScrollBar* pScrollBar);
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()
};

} // namespace Life
} // namespace HodjNPodj
} // namespace Bagel

#endif
