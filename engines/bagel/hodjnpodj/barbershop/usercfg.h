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

#ifndef HODJNPODJ_BARBERSHOP_USERCFG_H
#define HODJNPODJ_BARBERSHOP_USERCFG_H

#include "bagel/afxwin.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/barbershop/main.h"

namespace Bagel {
namespace HodjNPodj {
namespace Barbershop {

#define RGBCOLOR_DARKRED    RGB(0, 128, 0)
#define RGBCOLOR_BLACK      RGB(0, 0, 0)
#define RGBCOLOR_WHITE      RGB(255, 255, 255)

#define FONT_SIZE       16              // CText object font size

// CText message
#define TEXT_LEFT           20
#define TEXT_TOP            25
#define TEXT_RIG            220
#define TEXT_BOT            46

// card back rect 1
#define CBCK_RECT1_LEFT     40
#define CBCK_RECT1_TOP      55
#define CBCK_RECT1_RIG      110
#define CBCK_RECT1_BOT      125

// card back rect 2
#define CBCK_RECT2_LEFT     120
#define CBCK_RECT2_TOP      55
#define CBCK_RECT2_RIG      190
#define CBCK_RECT2_BOT      125

#define DELTA               0

#define IDD_USERCFG         100

class CUserCfgDlg : public CBmpDialog {
public:
	CUserCfgDlg(CWnd *pParent = nullptr, CPalette *pPalette = nullptr, unsigned int = IDD_USERCFG);

protected:

	virtual void DoDataExchange(CDataExchange *) override;
	virtual bool OnInitDialog() override;

	void OnClose();
	void OnPaint();
	void OnLButtonUp(unsigned int, CPoint);
	void OnCancel() override;
	void OnOK() override;
	DECLARE_MESSAGE_MAP()
	void MyFocusRect(CDC*, CRect, int nDrawMode, COLORREF);

private:
//	unsigned int            m_nTimeLimit;       //
	bool            m_bSave;            // True if should save theses values
	CRect           m_cRectCardBack1;
	CRect           m_cRectCardBack2;
	unsigned int            m_nCardBack;
	CColorButton    *m_pOKButton;       // OKAY button on scroll
	CText           *m_ctextBox;
};

} // namespace Barbershop
} // namespace HodjNPodj
} // namespace Bagel

#endif
