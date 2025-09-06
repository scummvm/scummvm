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

#include "bagel/hodjnpodj/hnplibs/stdafx.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/hodjnpodj/poker/resource.h"
#include "bagel/hodjnpodj/poker/dialogs.h"

namespace Bagel {
namespace HodjNPodj {
namespace Poker {

CPalette    *pPayoffPalette = nullptr;

CText           *ptxtPair = nullptr;
CText           *ptxtPairJacks = nullptr;
CText           *ptxtTwoPair = nullptr;
CText           *ptxtThreeOAK = nullptr;
CText           *ptxtStraight = nullptr;
CText           *ptxtFlush = nullptr;
CText           *ptxtFullHouse = nullptr;
CText           *ptxtFourOAK = nullptr;
CText           *ptxtStraightFlush = nullptr;
CText           *ptxtRoyalFlush = nullptr;

static CColorButton    *pSetPayButton = nullptr;
static CColorButton    *pCancelButton = nullptr;
static CColorButton    *pOKButton = nullptr;

static CRadioButton    *pKlingonButton = nullptr;
static CRadioButton    *pVegasButton = nullptr;
static CRadioButton    *pPoPorButton = nullptr;
static CRadioButton    *pKuwaitiButton = nullptr;
static CRadioButton    *pMartianButton = nullptr;

CSetPayoffsDlg::CSetPayoffsDlg(CWnd *pParent, CPalette *pPalette, unsigned int nID, int nOdds, bool bJustDisplay)
	:  CBmpDialog(pParent, pPalette, nID, ".\\ART\\MSCROLL.BMP"),
		crectRedraw(10, 140, 44, 290),
		rectPair(10, 131, 80, 146),
		rectPairJacks(10, 145, 80, 160),
		rectTwoPair(10, 159, 80, 174),
		rectThree(10, 173, 80, 188),
		rectStraight(10, 187, 80, 202),
		rectFlush(10, 201, 80, 216),
		rectFullHouse(10, 215, 80, 230),
		rectFour(10, 229, 80, 244),
		rectStraightFlush(10, 243, 80, 258),
		rectRoyalFlush(10, 257, 80, 272) {
	ptxtPair = new CText;
	ptxtPairJacks = new CText;
	ptxtTwoPair = new CText;
	ptxtThreeOAK = new CText;
	ptxtStraight = new CText;
	ptxtFlush = new CText;
	ptxtFullHouse = new CText;
	ptxtFourOAK = new CText;
	ptxtStraightFlush = new CText;
	ptxtRoyalFlush = new CText;

	m_bJustDisplay = bJustDisplay;
	m_nSetOfOdds = nOdds;

	pPayoffPalette = pPalette;

}

void CSetPayoffsDlg::DoDataExchange(CDataExchange *pDX) {
	CDialog::DoDataExchange(pDX);
}

bool CSetPayoffsDlg::OnInitDialog() {
	bool    bSuccess = false;

	CBmpDialog::OnInitDialog();

	pOKButton = new CColorButton();
	ASSERT(pOKButton != nullptr);
	pOKButton->SetPalette(pPayoffPalette);
	bSuccess = pOKButton->SetControl(ID_CANCEL_VIEW, this);
	ASSERT(bSuccess);

	pSetPayButton = new CColorButton();
	ASSERT(pSetPayButton != nullptr);
	pSetPayButton->SetPalette(pPayoffPalette);
	bSuccess = pSetPayButton->SetControl(ID_SETPAYOFFS, this);
	ASSERT(bSuccess);

	pCancelButton = new CColorButton();
	ASSERT(pCancelButton != nullptr);
	pCancelButton->SetPalette(pPayoffPalette);
	bSuccess = pCancelButton->SetControl(ID_CANCEL, this);
	ASSERT(bSuccess);

	pKlingonButton = new CRadioButton();
	ASSERT(pKlingonButton != nullptr);
	pKlingonButton->SetPalette(pPayoffPalette);
	bSuccess = pKlingonButton->SetControl(ID_KLINGON, this);
	ASSERT(bSuccess);

	pVegasButton = new CRadioButton();
	ASSERT(pVegasButton != nullptr);
	pVegasButton->SetPalette(pPayoffPalette);
	bSuccess = pVegasButton->SetControl(ID_VEGAS, this);
	ASSERT(bSuccess);

	pPoPorButton = new CRadioButton();
	ASSERT(pPoPorButton != nullptr);
	pPoPorButton->SetPalette(pPayoffPalette);
	bSuccess = pPoPorButton->SetControl(ID_POPO, this);
	ASSERT(bSuccess);

	pKuwaitiButton = new CRadioButton();
	ASSERT(pKuwaitiButton != nullptr);
	pKuwaitiButton->SetPalette(pPayoffPalette);
	bSuccess = pKuwaitiButton->SetControl(ID_KUWAITI, this);
	ASSERT(bSuccess);

	pMartianButton = new CRadioButton();
	ASSERT(pMartianButton != nullptr);
	pMartianButton->SetPalette(pPayoffPalette);
	bSuccess = pMartianButton->SetControl(ID_MARTIAN, this);
	ASSERT(bSuccess);

	switch (m_nSetOfOdds) {
	case IDC_KLINGON:
		OnKlingon(false);
		break;
	case IDC_VEGAS:
		OnVegas(false);
		break;
	case IDC_MARTIAN:
		OnMartian(false);
		break;
	case IDC_KUWAITI:
		OnKuwaiti(false);
		break;
	case IDC_POPO:
	default:
		OnPopo(false);
		break;
	}

	if (m_bJustDisplay) {
		pKlingonButton->EnableWindow(false);
		pVegasButton->EnableWindow(false);
		pPoPorButton->EnableWindow(false);
		pKuwaitiButton->EnableWindow(false);
		pMartianButton->EnableWindow(false);
		pSetPayButton->ShowWindow(SW_HIDE);
		pCancelButton->ShowWindow(SW_HIDE);
		pOKButton->SetFocus();
	} else {
		pOKButton->ShowWindow(SW_HIDE);
		pSetPayButton->SetFocus();
	}

//	SetDefID( ID_POPO );
	return true;
}

/*****************************************************************
 *
 * OnCommand
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * Process the "Set" and "Cancel" buttons
 *
 * This function is called when a WM_COMMAND message is issued,
 * typically in order to process control related activities.
 *
 * FORMAL PARAMETERS:
 *
 *      wParam          identifier for the button to be processed
 *      lParam          type of message to be processed
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      n/a
 *
 * RETURN VALUE:
 *
 *      n/a
 *
 ****************************************************************/
bool CSetPayoffsDlg::OnCommand(WPARAM wParam, LPARAM lParam) {

// What ever button is clicked, end the dialog and send the ID of the button
// clicked as the return from the dialog

	if (HIWORD(lParam) == BN_CLICKED) {
		switch (wParam) {
		case IDC_KLINGON:
			OnKlingon();
			break;
		case IDC_VEGAS:
			OnVegas();
			break;
		case IDC_POPO:
			OnPopo();
			break;
		case IDC_MARTIAN:
			OnMartian();
			break;
		case IDC_KUWAITI:
			OnKuwaiti();
			break;
		case ID_SETPAYOFFS:
		case IDC_SETPAYOFFS:
			OnSetpayoffs();
			break;
		case IDC_CANCEL:
		case ID_CANCEL:
		case ID_CANCEL_VIEW:
			OnCancel();
			break;
		default:
			CBmpDialog::OnCommand(wParam, lParam);
			break;
		}
	}
	return true;
}

/*****************************************************************
 *
 * OnPaint
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * Repaint the screen whenever needed; e.g. when uncovered by an
 * overlapping window, when maximized from an icon, and when it the
 * window is initially created.
 *
 * This uses the COptions Paint as its base, and displays the current
 * amount chosen from the scrollbar
 *
 * This routine is called whenever Windows sends a WM_PAINT message.
 * Note that creating a CPaintDC automatically does a BeginPaint and
 * an EndPaint call is done when it is destroyed at the end of this
 * function.  CPaintDC's constructor needs the window (this).
 *
 * FORMAL PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  n/a
 *
 * RETURN VALUE:
 *
 *  n/a
 *
 ****************************************************************/
void CSetPayoffsDlg::OnPaint() {
	CDC             *pDC;
	int         nOldBkMode;

//  call COptions onpaint, to paint the background
	CBmpDialog::OnPaint();

	pDC = GetDC();

//  now paint in my text with a transparent background
	ptxtPair->SetupText(pDC, pPayoffPalette, &rectPair, JUSTIFY_RIGHT);
	ptxtPairJacks->SetupText(pDC, pPayoffPalette, &rectPairJacks, JUSTIFY_RIGHT);
	ptxtTwoPair->SetupText(pDC, pPayoffPalette, &rectTwoPair, JUSTIFY_RIGHT);
	ptxtThreeOAK->SetupText(pDC, pPayoffPalette, &rectThree, JUSTIFY_RIGHT);
	ptxtStraight->SetupText(pDC, pPayoffPalette, &rectStraight, JUSTIFY_RIGHT);
	ptxtFlush->SetupText(pDC, pPayoffPalette, &rectFlush, JUSTIFY_RIGHT);
	ptxtFullHouse->SetupText(pDC, pPayoffPalette, &rectFullHouse, JUSTIFY_RIGHT);
	ptxtFourOAK->SetupText(pDC, pPayoffPalette, &rectFour, JUSTIFY_RIGHT);
	ptxtStraightFlush->SetupText(pDC, pPayoffPalette, &rectStraightFlush, JUSTIFY_RIGHT);
	ptxtRoyalFlush->SetupText(pDC, pPayoffPalette, &rectRoyalFlush, JUSTIFY_RIGHT);

	nOldBkMode = pDC->SetBkMode(TRANSPARENT);
	pDC->TextOut(65, 24, "Choose a set of payoffs.");
	pDC->TextOut(85, 130, "Pair");
	pDC->TextOut(85, 144, "Pair (Jacks  or higher)");
	pDC->TextOut(85, 158, "Two Pairs");
	pDC->TextOut(85, 172, "Three of a kind");
	pDC->TextOut(85, 186, "Straight");
	pDC->TextOut(85, 200, "Flush");
	pDC->TextOut(85, 214, "Full House");
	pDC->TextOut(85, 228, "Four of a Kind");
	pDC->TextOut(85, 242, "Straight Flush");
	pDC->TextOut(85, 256, "Royal Flush");

	ptxtPair->DisplayString(pDC, m_cPair, 12, FW_BOLD, RGB(0, 0, 200));
	ptxtPairJacks->DisplayString(pDC, m_cPairJacksPlus, 12, FW_BOLD, RGB(0, 0, 200));
	ptxtTwoPair->DisplayString(pDC, m_cTwoPair, 12, FW_BOLD, RGB(0, 0, 200));
	ptxtThreeOAK->DisplayString(pDC, m_cThreeOAK, 12, FW_BOLD, RGB(0, 0, 200));
	ptxtStraight->DisplayString(pDC, m_cStriaght, 12, FW_BOLD, RGB(0, 0, 200));
	ptxtFlush->DisplayString(pDC, m_cFlush, 12, FW_BOLD, RGB(0, 0, 200));
	ptxtFullHouse->DisplayString(pDC, m_cFullHouse, 12, FW_BOLD, RGB(0, 0, 200));
	ptxtFourOAK->DisplayString(pDC, m_cFourOAK, 12, FW_BOLD, RGB(0, 0, 200));
	ptxtStraightFlush->DisplayString(pDC, m_cStraightFlush, 12, FW_BOLD, RGB(0, 0, 200));
	ptxtRoyalFlush->DisplayString(pDC, m_cRoyalFlush, 12, FW_BOLD, RGB(0, 0, 200));

	pDC->SetBkMode(nOldBkMode);
	ReleaseDC(pDC);
	return;
}

//////////////////////////////////////////////////////////////////
void CSetPayoffsDlg::OnSetpayoffs() {
//	RefreshBackground();
	ClearDialogImage();
	EndDialog(m_nSetOfOdds);
	return;
}

void CSetPayoffsDlg::OnCancel() {
//	RefreshBackground();
	ClearDialogImage();
	EndDialog(0);
	return;
}

void CSetPayoffsDlg::OnKlingon(bool bDisplay) {
	CDC             *pDC;
	pDC = GetDC();

	pKlingonButton->SetCheck(1);
	Common::strcpy_s(m_cPair, 5, "0");
	Common::strcpy_s(m_cPairJacksPlus, 5, "0");
	Common::strcpy_s(m_cTwoPair, 5, "1");
	Common::strcpy_s(m_cThreeOAK, 5, "2");
	Common::strcpy_s(m_cStriaght, 5, "3");
	Common::strcpy_s(m_cFlush, 5, "4");
	Common::strcpy_s(m_cFullHouse, 5, "5");
	Common::strcpy_s(m_cFourOAK, 5, "10");
	Common::strcpy_s(m_cStraightFlush, 5, "20");
	Common::strcpy_s(m_cRoyalFlush, 5, "50");
	m_nSetOfOdds = IDC_KLINGON;

	if (bDisplay) {
		ptxtPair->DisplayString(pDC, m_cPair, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtPairJacks->DisplayString(pDC, m_cPairJacksPlus, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtTwoPair->DisplayString(pDC, m_cTwoPair, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtThreeOAK->DisplayString(pDC, m_cThreeOAK, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtStraight->DisplayString(pDC, m_cStriaght, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtFlush->DisplayString(pDC, m_cFlush, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtFullHouse->DisplayString(pDC, m_cFullHouse, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtFourOAK->DisplayString(pDC, m_cFourOAK, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtStraightFlush->DisplayString(pDC, m_cStraightFlush, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtRoyalFlush->DisplayString(pDC, m_cRoyalFlush, 12, FW_BOLD, RGB(0, 0, 200));
	}
	ReleaseDC(pDC);
	return;
}

void CSetPayoffsDlg::OnKuwaiti(bool bDisplay) {
	CDC             *pDC;
	pDC = GetDC();

	pKuwaitiButton->SetCheck(1);
	Common::strcpy_s(m_cPair, 5, "2");
	Common::strcpy_s(m_cPairJacksPlus, 5, "3");
	Common::strcpy_s(m_cTwoPair, 5, "5");
	Common::strcpy_s(m_cThreeOAK, 5, "10");
	Common::strcpy_s(m_cStriaght, 5, "15");
	Common::strcpy_s(m_cFlush, 5, "20");
	Common::strcpy_s(m_cFullHouse, 5, "50");
	Common::strcpy_s(m_cFourOAK, 5, "100");
	Common::strcpy_s(m_cStraightFlush, 5, "250");
	Common::strcpy_s(m_cRoyalFlush, 5, "1000");
	m_nSetOfOdds = IDC_KUWAITI;

	if (bDisplay) {
		ptxtPair->DisplayString(pDC, m_cPair, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtPairJacks->DisplayString(pDC, m_cPairJacksPlus, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtTwoPair->DisplayString(pDC, m_cTwoPair, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtThreeOAK->DisplayString(pDC, m_cThreeOAK, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtStraight->DisplayString(pDC, m_cStriaght, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtFlush->DisplayString(pDC, m_cFlush, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtFullHouse->DisplayString(pDC, m_cFullHouse, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtFourOAK->DisplayString(pDC, m_cFourOAK, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtStraightFlush->DisplayString(pDC, m_cStraightFlush, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtRoyalFlush->DisplayString(pDC, m_cRoyalFlush, 12, FW_BOLD, RGB(0, 0, 200));
	}
	ReleaseDC(pDC);
	return;
}

void CSetPayoffsDlg::OnMartian(bool bDisplay) {
	CDC             *pDC;
	pDC = GetDC();

	pMartianButton->SetCheck(1);
	Common::strcpy_s(m_cPair, 5, "2");
	Common::strcpy_s(m_cPairJacksPlus, 5, "2");
	Common::strcpy_s(m_cTwoPair, 5, "13");
	Common::strcpy_s(m_cThreeOAK, 5, "3");
	Common::strcpy_s(m_cStriaght, 5, "89");
	Common::strcpy_s(m_cFlush, 5, "98");
	Common::strcpy_s(m_cFullHouse, 5, "177");
	Common::strcpy_s(m_cFourOAK, 5, "4");
	Common::strcpy_s(m_cStraightFlush, 5, "23");
	Common::strcpy_s(m_cRoyalFlush, 5, "11");
	m_nSetOfOdds = IDC_MARTIAN;

	if (bDisplay) {
		ptxtPair->DisplayString(pDC, m_cPair, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtPairJacks->DisplayString(pDC, m_cPairJacksPlus, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtTwoPair->DisplayString(pDC, m_cTwoPair, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtThreeOAK->DisplayString(pDC, m_cThreeOAK, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtStraight->DisplayString(pDC, m_cStriaght, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtFlush->DisplayString(pDC, m_cFlush, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtFullHouse->DisplayString(pDC, m_cFullHouse, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtFourOAK->DisplayString(pDC, m_cFourOAK, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtStraightFlush->DisplayString(pDC, m_cStraightFlush, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtRoyalFlush->DisplayString(pDC, m_cRoyalFlush, 12, FW_BOLD, RGB(0, 0, 200));
	}
	ReleaseDC(pDC);
	return;
}

void CSetPayoffsDlg::OnPopo(bool bDisplay) {
	CDC             *pDC;
	pDC = GetDC();

	pPoPorButton->SetCheck(1);
	Common::strcpy_s(m_cPair, 5, "1");
	Common::strcpy_s(m_cPairJacksPlus, 5, "2");
	Common::strcpy_s(m_cTwoPair, 5, "3");
	Common::strcpy_s(m_cThreeOAK, 5, "5");
	Common::strcpy_s(m_cStriaght, 5, "8");
	Common::strcpy_s(m_cFlush, 5, "10");
	Common::strcpy_s(m_cFullHouse, 5, "15");
	Common::strcpy_s(m_cFourOAK, 5, "50");
	Common::strcpy_s(m_cStraightFlush, 5, "100");
	Common::strcpy_s(m_cRoyalFlush, 5, "500");
	m_nSetOfOdds = IDC_POPO;

	if (bDisplay) {
		ptxtPair->DisplayString(pDC, m_cPair, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtPairJacks->DisplayString(pDC, m_cPairJacksPlus, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtTwoPair->DisplayString(pDC, m_cTwoPair, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtThreeOAK->DisplayString(pDC, m_cThreeOAK, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtStraight->DisplayString(pDC, m_cStriaght, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtFlush->DisplayString(pDC, m_cFlush, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtFullHouse->DisplayString(pDC, m_cFullHouse, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtFourOAK->DisplayString(pDC, m_cFourOAK, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtStraightFlush->DisplayString(pDC, m_cStraightFlush, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtRoyalFlush->DisplayString(pDC, m_cRoyalFlush, 12, FW_BOLD, RGB(0, 0, 200));
	}
	ReleaseDC(pDC);
	return;
}

void CSetPayoffsDlg::OnVegas(bool bDisplay) {
	CDC             *pDC;
	pDC = GetDC();

	pVegasButton->SetCheck(1);
	Common::strcpy_s(m_cPair, 5, "0");
	Common::strcpy_s(m_cPairJacksPlus, 5, "1");
	Common::strcpy_s(m_cTwoPair, 5, "2");
	Common::strcpy_s(m_cThreeOAK, 5, "3");
	Common::strcpy_s(m_cStriaght, 5, "4");
	Common::strcpy_s(m_cFlush, 5, "6");
	Common::strcpy_s(m_cFullHouse, 5, "9");
	Common::strcpy_s(m_cFourOAK, 5, "25");
	Common::strcpy_s(m_cStraightFlush, 5, "50");
	Common::strcpy_s(m_cRoyalFlush, 5, "250");
	m_nSetOfOdds = IDC_VEGAS;

	if (bDisplay) {
		ptxtPair->DisplayString(pDC, m_cPair, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtPairJacks->DisplayString(pDC, m_cPairJacksPlus, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtTwoPair->DisplayString(pDC, m_cTwoPair, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtThreeOAK->DisplayString(pDC, m_cThreeOAK, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtStraight->DisplayString(pDC, m_cStriaght, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtFlush->DisplayString(pDC, m_cFlush, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtFullHouse->DisplayString(pDC, m_cFullHouse, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtFourOAK->DisplayString(pDC, m_cFourOAK, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtStraightFlush->DisplayString(pDC, m_cStraightFlush, 12, FW_BOLD, RGB(0, 0, 200));
		ptxtRoyalFlush->DisplayString(pDC, m_cRoyalFlush, 12, FW_BOLD, RGB(0, 0, 200));
	}
	ReleaseDC(pDC);
	return;
}

void CSetPayoffsDlg::OnDestroy() {
//  send a message to the calling app to tell it the user has quit the game
	delete ptxtPair;
	delete ptxtPairJacks;
	delete ptxtTwoPair;
	delete ptxtThreeOAK;
	delete ptxtStraight;
	delete ptxtFlush;
	delete ptxtFullHouse;
	delete ptxtFourOAK;
	delete ptxtStraightFlush;
	delete ptxtRoyalFlush;

	if (pSetPayButton != nullptr) {
		delete pSetPayButton;
		pSetPayButton = nullptr;
	}
	if (pCancelButton != nullptr) {
		delete pCancelButton;
		pCancelButton = nullptr;
	}
	if (pOKButton != nullptr) {
		delete pOKButton;
		pOKButton = nullptr;
	}
	if (pKlingonButton != nullptr) {
		delete pKlingonButton;
		pKlingonButton = nullptr;
	}
	if (pVegasButton != nullptr) {
		delete pVegasButton;
		pVegasButton = nullptr;
	}
	if (pPoPorButton != nullptr) {
		delete pPoPorButton;
		pPoPorButton = nullptr;
	}
	if (pKuwaitiButton != nullptr) {
		delete pKuwaitiButton;
		pKuwaitiButton = nullptr;
	}
	if (pMartianButton != nullptr) {
		delete pMartianButton;
		pMartianButton = nullptr;
	}

	CBmpDialog::OnDestroy();
}

bool CSetPayoffsDlg::OnEraseBkgnd(CDC *pDC) {
	return true;
}

void CSetPayoffsDlg::ClearDialogImage() {

	if (pSetPayButton != nullptr) {
		delete pSetPayButton;
		pSetPayButton = nullptr;
	}
	if (pOKButton != nullptr) {
		delete pOKButton;
		pOKButton = nullptr;
	}
	if (pCancelButton != nullptr) {
		delete pCancelButton;
		pCancelButton = nullptr;
	}
	if (pKlingonButton != nullptr) {
		delete pKlingonButton;
		pKlingonButton = nullptr;
	}
	if (pVegasButton != nullptr) {
		delete pVegasButton;
		pVegasButton = nullptr;
	}
	if (pPoPorButton != nullptr) {
		delete pPoPorButton;
		pPoPorButton = nullptr;
	}
	if (pKuwaitiButton != nullptr) {
		delete pKuwaitiButton;
		pKuwaitiButton = nullptr;
	}
	if (pMartianButton != nullptr) {
		delete pMartianButton;
		pMartianButton = nullptr;
	}

	ValidateRect(nullptr);

	return;
}

BEGIN_MESSAGE_MAP(CSetPayoffsDlg, CBmpDialog)
	//{{AFX_MSG_MAP( CMainPokerWindow )
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

} // namespace Poker
} // namespace HodjNPodj
} // namespace Bagel
