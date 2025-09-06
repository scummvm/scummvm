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

#ifndef HODJNPODJ_POKER_DIALOGS_H
#define HODJNPODJ_POKER_DIALOGS_H

#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/poker/poker.h"

namespace Bagel {
namespace HodjNPodj {
namespace Poker {

#define AMOUNTMAX   10
#define AMOUNTMIN   1

//////////////////////////////////////////////////////////////////////////////
////////    Options Specific to Poker
//////////////////////////////////////////////////////////////////////////////
class COptionsDlg : public CBmpDialog {
private:
	bool    m_bSoundOn;                 // bool for telling me should I play sounds or not
	bool    m_bDisableSets;             // bool for telling me should I disable the
	// "Set Amount" and "Set Payoffs" buttons

public:
	COptionsDlg(CWnd* pParent = nullptr, CPalette *pPalette = nullptr, unsigned int = IDD_OPTIONS);
	void SetInitialOptions(bool = false, bool = true);   // Sets the private members
	void ClearDialogImage();

protected:
	virtual bool OnCommand(WPARAM, LPARAM) override;
	virtual bool OnInitDialog() override;
	afx_msg void OnPaint();
	afx_msg bool OnEraseBkgnd(CDC *pDC);
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////////////
////////    Set Current Number of Crowns
//////////////////////////////////////////////////////////////////////////////
class CSetAmountDlg : public CBmpDialog {
private:
	int     m_nCurrentAmount;   // contains the current amount set
	char    m_cAmount [6];
	CRect   rectDisplayAmount;

public:
	CSetAmountDlg(CWnd* pParent = nullptr, CPalette *pPalette = nullptr, unsigned int = IDD_SETAMOUNT);
	void SetInitialOptions(long = 0);    // Sets the private members
	void ClearDialogImage();
	bool OnInitDialog() override;

protected:
	//{{AFX_MSG(COptions)
	virtual bool OnCommand(WPARAM, LPARAM) override;
	afx_msg void OnHScroll(unsigned int nSBCode, unsigned int nPos, CScrollBar* pScrollBar);
	afx_msg void OnPaint();
	afx_msg bool OnEraseBkgnd(CDC *pDC);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
//////////////////////////////////////////////////////////////////////////////
////////    Set the Payoffs
//////////////////////////////////////////////////////////////////////////////
class CSetPayoffsDlg : public CBmpDialog {
private:
	char m_cPair[6];
	char m_cPairJacksPlus[6];
	char m_cTwoPair[6];
	char m_cThreeOAK[6];
	char m_cStriaght[6];
	char m_cFlush[6];
	char m_cFullHouse[6];
	char m_cFourOAK[6];
	char m_cStraightFlush[6];
	char m_cRoyalFlush[6];
	int  m_nSetOfOdds;
	bool m_bJustDisplay;
	CRect crectRedraw;
	CRect rectPair, rectPairJacks;
	CRect rectTwoPair, rectThree;
	CRect rectStraight, rectFlush;
	CRect rectFullHouse, rectFour;
	CRect rectStraightFlush;
	CRect rectRoyalFlush;

public:
	CSetPayoffsDlg(CWnd* pParent = nullptr, CPalette *pPalette = nullptr, unsigned int = IDD_WINRATIO, int = IDC_POPO, bool = false);
	void ClearDialogImage();
	void OnKlingon(bool = true);
	void OnKuwaiti(bool = true);
	void OnMartian(bool = true);
	void OnPopo(bool = true);
	void OnVegas(bool = true);

protected:
	virtual void DoDataExchange(CDataExchange *) override;
	virtual bool OnInitDialog() override;
	afx_msg void OnSetpayoffs();
	virtual bool OnCommand(WPARAM, LPARAM) override;
	afx_msg void OnCancel() override;
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg bool OnEraseBkgnd(CDC *pDC);
	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////////////
////////    User won dialog
//////////////////////////////////////////////////////////////////////////////
class CUserWonDlg : public CBmpDialog {
private:
	int m_nWinPhrase;

public:
	CUserWonDlg(CWnd *pParent = nullptr, CPalette *pPalette = nullptr, unsigned int = IDD_USERWON);
	void SetInitialOptions(int = 0);
	void ClearDialogImage();
	bool OnInitDialog() override;

protected:
	virtual void OnOK() override;
	virtual bool OnCommand(WPARAM, LPARAM) override;
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()
};

} // namespace Poker
} // namespace HodjNPodj
} // namespace Bagel

#endif
