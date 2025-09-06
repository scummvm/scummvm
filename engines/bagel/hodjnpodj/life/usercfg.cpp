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
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/hnplibs/menures.h"
#include "bagel/hodjnpodj/life/usercfg.h"
#include "bagel/hodjnpodj/life/life.h"

namespace Bagel {
namespace HodjNPodj {
namespace Life {

extern int  nSpeed,                 // Speed between evolutions
       nCountDown,             // Counts before an evolution
       nLife,                  // Num of lives given at game start
       nPlace;
extern int  nTurnCounter,           // Counts num of years before end of game
       nLifeCounter;           // Counts num of lives left to place

extern bool bPrePlaceColonies,      // Whether they want to pre-place some
       bIsInfiniteLife,        // Tells if num of lives is set to infi
       bIsInfiniteTurns;       // Tells if num of turns is set to infi

extern CPalette *pGamePalette;      // Used everywhere for ref

CUserCfgDlg::CUserCfgDlg(CWnd *pParent, CPalette *pPalette, unsigned int nID)
	: CBmpDialog(pParent, pPalette, nID, ".\\ART\\SSCROLL.BMP") {
	m_DisplayRounds     = "";
	m_DisplaySpeed      = "";
	m_DisplayVillages   = "";
}

void CUserCfgDlg::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_STARTVILLAGES, m_ScrollVillages);
	DDX_Control(pDX, IDC_ROUNDS, m_ScrollRounds);
	DDX_Control(pDX, IDC_EVOLVESPEED, m_ScrollSpeed);
}

bool CUserCfgDlg::OnInitDialog() {
	int     i;
	CRect   statsRect;                  // game stats displays
	int     nStat_col_offset;           // game stats placement
	int     nStat_row_offset;
	int     nStatWidth, nStatHeight;
	bool    bAssertCheck;
	CDC     *pDC;

	CBmpDialog::OnInitDialog();

	mSpeedTable[0] = "Time Flies";
	mSpeedTable[1] = "1.2 Seconds";
	mSpeedTable[2] = "2.4 Seconds";
	mSpeedTable[3] = "3.6 Seconds";
	mSpeedTable[4] = "4.8 Seconds";
	mSpeedTable[5] = "6.0 Seconds";
	mSpeedTable[6] = "7.2 Seconds";
	mSpeedTable[7] = "8.4 Seconds";
	mSpeedTable[8] = "Time Crawls";

	m_nLife[0] = 5;
	m_nLife[1] = 10;
	m_nLife[2] = 15;
	m_nLife[3] = 20;
	m_nLife[4] = 25;
	m_nLife[5] = 30;
	m_nLife[6] = 40;
	m_nLife[7] = 50;
	m_nLife[8] = 60;
	m_nLife[9] = 80;
	m_nLife[10] = 100;
	m_nLife[11] = 125;
	m_nLife[12] = 150;
	m_nLife[13] = 175;
	m_nLife[14] = 200;
	m_nLife[15] = 201;

	for (i = 0; i < TURNS_SETTINGS; i++)
		m_nTurns[i] = m_nLife[i];

	// Convert preferences
	nSpeedTemp = nSpeed;

	nLifeTemp = -1;
	for (i = 0; i < LIFE_SETTINGS; i++)
		if (m_nLife[i] == nLife) {
			nLifeTemp = i;
			break;
		}

	if (nLifeTemp == -1)
		nLifeTemp = 7;          // MAgIC NUmbER!!! (bec m_nLife[7] = VILLAGES_DEF)

	nTurnCounterTemp = -1;
	for (i = 0; i < TURNS_SETTINGS; i++)
		if (m_nTurns[i] == nTurnCounter) {
			nTurnCounterTemp = i;
			break;
		}

	if (nTurnCounterTemp == -1)
		nTurnCounterTemp = 10;  // MAgIC NUmbER!!! (bec m_nLife[10] = ROUNDS_DEF)

	pDC = GetDC();

	// setup the Starting Villages stat display box
	nStat_col_offset    = 25;   //20
	nStat_row_offset    = 25;
	nStatWidth          = 120;
	nStatHeight         = 15;
	statsRect.SetRect(
	    nStat_col_offset,
	    nStat_row_offset,
	    nStat_col_offset + nStatWidth,
	    nStat_row_offset + nStatHeight
	);

	if ((m_pVillages = new CText) != nullptr) {
		bAssertCheck = (*m_pVillages).SetupText(pDC, pGamePalette, &statsRect, JUSTIFY_LEFT);
		ASSERT(bAssertCheck);   // initialize the text objext
	} // end if

	// setup the starting villages scroll bar
	m_ScrollVillages.SetScrollRange(MIN_LIFE, MAX_LIFE, false);
	m_ScrollVillages.SetScrollPos(nLifeTemp, true);

	// set up the Speed stat display box
	nStat_row_offset += 35; //48;
	statsRect.SetRect(
	    nStat_col_offset,
	    nStat_row_offset,
	    nStat_col_offset + nStatWidth,
	    nStat_row_offset + nStatHeight
	);

	if ((m_pSpeed = new CText) != nullptr) {
		bAssertCheck = (*m_pSpeed).SetupText(pDC, pGamePalette, &statsRect, JUSTIFY_LEFT);
		ASSERT(bAssertCheck);   // initialize the text objext
	} // end if

	// setup the speed scroll bar
	m_ScrollSpeed.SetScrollRange(MIN_SPEED, MAX_SPEED, false);
	m_ScrollSpeed.SetScrollPos(nSpeedTemp, true);

	// set up the rounds stat display box
	nStat_row_offset += 38; //48;
	statsRect.SetRect(
	    nStat_col_offset,
	    nStat_row_offset,
	    nStat_col_offset + nStatWidth,
	    nStat_row_offset + nStatHeight
	);

	if ((m_pRounds = new CText) != nullptr) {
		bAssertCheck = (*m_pRounds).SetupText(pDC, pGamePalette, &statsRect, JUSTIFY_LEFT);
		ASSERT(bAssertCheck);   // initialize the text objext
	} // end if

	ReleaseDC(pDC);

	// setup the speed scroll bar
	m_ScrollRounds.SetScrollRange(MIN_TURNS, MAX_TURNS, false);
	m_ScrollRounds.SetScrollPos(nTurnCounterTemp, true);

	/************************
	* Set up color buttons. *
	************************/
	if ((m_pPrePlaceButton = new CCheckButton) != nullptr) {                   // build a color QUIT button to let us exit
		(*m_pPrePlaceButton).SetPalette(pGamePalette);                      // set the palette to use
		(*m_pPrePlaceButton).SetControl(IDC_PREPLACE, this);            // tie to the dialog control
	}
	((CWnd *)this)->CheckDlgButton(IDC_PREPLACE, bPrePlaceColonies);             // Set the frame option box

	if ((m_pOKButton = new CColorButton) != nullptr) {     // build a color OK button
		(*m_pOKButton).SetPalette(pGamePalette);        // set the palette to use
		(*m_pOKButton).SetControl(IDOK, this);          // tie to the dialog control
	} // end if

	if ((m_pCancelButton = new CColorButton) != nullptr) { // build a color CANCEL button
		(*m_pCancelButton).SetPalette(pGamePalette);
		(*m_pCancelButton).SetControl(IDCANCEL, this);
	} // end if

	return true;  // return true  unless you set the focus to a control
}

void CUserCfgDlg::OnHScroll(unsigned int nSBCode, unsigned int nPos, CScrollBar* pScrollBar) {
	// TODO: Add your message handler code here and/or call default
	CDC     *pDC;
	bool    bAssertCheck;
	char    msg[64];

	pDC = GetDC();

	if (pScrollBar->GetDlgCtrlID() == IDC_EVOLVESPEED) {
		int nOldSpeed = nSpeedTemp;

		switch (nSBCode) {
		case SB_LINERIGHT:
			nSpeedTemp++;
			break;
		case SB_PAGERIGHT:
			nSpeedTemp += 2; //SPEED/5;
			break;
		case SB_RIGHT:
			nSpeedTemp = MAX_SPEED;
			break;
		case SB_LINELEFT:
			nSpeedTemp--;
			break;
		case SB_PAGELEFT:
			nSpeedTemp -= 2; //SPEED/5;
			break;
		case SB_LEFT:
			nSpeedTemp = MIN_SPEED;
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			nSpeedTemp = (int) nPos;
			break;
		}

		if (nSpeedTemp < MIN_SPEED)
			nSpeedTemp = MIN_SPEED;
		if (nSpeedTemp > MAX_SPEED)
			nSpeedTemp = MAX_SPEED;

		if (nSpeedTemp != nOldSpeed) {                       //To prevent "flicker"
			Common::sprintf_s(msg, "Speed: %s", mSpeedTable[nSpeedTemp].c_str());   //  only redraw if
			//  m_nSpeedTemp has changed
			bAssertCheck = (*m_pSpeed).DisplayString(pDC, msg, FONT_SIZE, FW_BOLD, OPTIONS_COLOR);
			ASSERT(bAssertCheck);

			pScrollBar->SetScrollPos(nSpeedTemp, true);
		}
	} else if (pScrollBar->GetDlgCtrlID() == IDC_STARTVILLAGES) {
		int nOldCounter = nLifeTemp;

		switch (nSBCode) {
		case SB_LINERIGHT:
			nLifeTemp++;
			break;
		case SB_PAGERIGHT:
			nLifeTemp += LIFE_SETTINGS / 5; // want 5 pagerights end to end
			break;
		case SB_RIGHT:
			nLifeTemp = MAX_LIFE;
			break;
		case SB_LINELEFT:
			nLifeTemp--;
			break;
		case SB_PAGELEFT:
			nLifeTemp -= LIFE_SETTINGS / 5; // want 5 pagerights end to end
			break;
		case SB_LEFT:
			nLifeTemp = MIN_LIFE;
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			nLifeTemp = (int) nPos;
			break;
		}

		if (nLifeTemp < MIN_LIFE)
			nLifeTemp = MIN_LIFE;
		if (nLifeTemp > MAX_LIFE)
			nLifeTemp = MAX_LIFE;

		if (nLifeTemp != nOldCounter) {              //To prevent "flicker"
			//  only redraw if counter has changed
			if (nLifeTemp == MAX_LIFE)
				Common::sprintf_s(msg, "Villages: Unlimited");
			else
				Common::sprintf_s(msg, "Villages: %d", m_nLife[nLifeTemp]);

			bAssertCheck = (*m_pVillages).DisplayString(pDC, msg, FONT_SIZE, FW_BOLD, OPTIONS_COLOR);
			ASSERT(bAssertCheck);   // paint the text

			pScrollBar->SetScrollPos(nLifeTemp, true);
		}
	} else if (pScrollBar->GetDlgCtrlID() == IDC_ROUNDS) {
		int nOldCounter = nTurnCounterTemp;

		switch (nSBCode) {
		case SB_LINERIGHT:
			nTurnCounterTemp++;
			break;
		case SB_PAGERIGHT:
			nTurnCounterTemp += (MAX_TURNS - MIN_TURNS + 1) / 5; // want 5 pagerights end to end
			break;
		case SB_RIGHT:
			nTurnCounterTemp = MAX_TURNS;
			break;
		case SB_LINELEFT:
			nTurnCounterTemp--;
			break;
		case SB_PAGELEFT:
			nTurnCounterTemp -= (MAX_TURNS - MIN_TURNS + 1) / 5; // want 5 pagerights end to end
			break;
		case SB_LEFT:
			nTurnCounterTemp = MIN_TURNS;
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			nTurnCounterTemp = (int) nPos;
			break;
		}

		if (nTurnCounterTemp < MIN_TURNS)
			nTurnCounterTemp = MIN_TURNS;
		if (nTurnCounterTemp > MAX_TURNS)
			nTurnCounterTemp = MAX_TURNS;

		//  only redraw if changed to prevent "flicker"
		if (nTurnCounterTemp != nOldCounter) {
			if (nTurnCounterTemp == MAX_TURNS)
				Common::sprintf_s(msg, "Years: Unlimited");
			else
				Common::sprintf_s(msg, "Years: %d", m_nTurns[nTurnCounterTemp]);

			bAssertCheck = (*m_pRounds).DisplayString(pDC, msg, FONT_SIZE, FW_BOLD, OPTIONS_COLOR);
			ASSERT(bAssertCheck);   // paint the text

			pScrollBar->SetScrollPos(nTurnCounterTemp, true);
		}
	}

	ReleaseDC(pDC);
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CUserCfgDlg::OnPrePlace() {
	bPrePlaceColonies = !bPrePlaceColonies;
	((CWnd *)this)->CheckDlgButton(IDC_PREPLACE, bPrePlaceColonies);
}

void CUserCfgDlg::OnOK() {
	ClearDialogImage();
	if (m_nLife[nLifeTemp] == nLife  &&      // Starting Lives or Rounds option changed?
	        m_nTurns[nTurnCounterTemp] == nTurnCounter) {
		nSpeed = nSpeedTemp;                    // No - so just update speed counter
		nCountDown = nSpeed * MONTHS;
		EndDialog(false);                       // and don't start new game
		return;
	} // end if

	if (nLifeTemp == MAX_LIFE) {                 // infinite life?
		bIsInfiniteLife = true;                 // yes
	} else {
		bIsInfiniteLife = false;
		nLifeCounter    = m_nLife[nLifeTemp];
	} // end if
	nLife = m_nLife[nLifeTemp];

	nSpeed = nSpeedTemp;                        // save speed counter
	nCountDown = nSpeed * MONTHS;

	if (nTurnCounterTemp == MAX_TURNS) {         // fastest turn counter?
		bIsInfiniteTurns = true;
	} else {
		bIsInfiniteTurns = false;
	} // end if

	nTurnCounter = m_nTurns[nTurnCounterTemp];

	/******************************************
	* Save new game settings to the INI file. *
	******************************************/
	WritePrivateProfileString(INI_SECTION, "Speed",
	                          Common::String::format("%d", nSpeedTemp).c_str(),
	                          INI_FNAME);

	WritePrivateProfileString(INI_SECTION, "Rounds",
	                          Common::String::format("%d", m_nTurns[nTurnCounterTemp]).c_str(),
	                          INI_FNAME);

	WritePrivateProfileString(INI_SECTION, "Villages",
	                          Common::String::format("%d", m_nLife[nLifeTemp]).c_str(),
	                          INI_FNAME);

	if (bPrePlaceColonies)
		nPlace = 1;
	else
		nPlace = 0;

	WritePrivateProfileString(INI_SECTION, "PrePlace",
	                          Common::String::format("%d", nPlace).c_str(),
	                          INI_FNAME);

	EndDialog(IDOK);
}

void CUserCfgDlg::OnCancel() {
	ClearDialogImage();
	EndDialog(false);
}

void CUserCfgDlg::OnPaint() {
	// TODO: Add your message handler code here
	CDC     *pDC;
	bool    bAssertCheck;
	char    msg[64];

	CBmpDialog::OnPaint();

	pDC = GetDC();

	// Display Starting Villages stats
	if (bIsInfiniteLife == true)
		Common::sprintf_s(msg, "Villages: Unlimited");
	else
		Common::sprintf_s(msg, "Villages: %d", m_nLife[nLifeTemp]);

	bAssertCheck = (*m_pVillages).DisplayString(pDC, msg, FONT_SIZE, FW_BOLD, OPTIONS_COLOR);
	ASSERT(bAssertCheck);


	// Display Speed stats
	Common::sprintf_s(msg, "Speed: %s", mSpeedTable[nSpeedTemp].c_str());

	bAssertCheck = (*m_pSpeed).DisplayString(pDC, msg, FONT_SIZE, FW_BOLD, OPTIONS_COLOR);
	ASSERT(bAssertCheck);   // paint the text

	// set up the rounds stat display box
	if (bIsInfiniteTurns == true)
		Common::sprintf_s(msg, "Years: Unlimited");
	else
		Common::sprintf_s(msg, "Years: %d", m_nTurns[nTurnCounterTemp]);

	bAssertCheck = (*m_pRounds).DisplayString(pDC, msg, FONT_SIZE, FW_BOLD, OPTIONS_COLOR);
	ASSERT(bAssertCheck);   // paint the text

	ReleaseDC(pDC);
}

void CUserCfgDlg::ClearDialogImage() {
	if (m_pOKButton != nullptr) {      // release button
		delete m_pOKButton;
		m_pOKButton = nullptr;
	}

	if (m_pCancelButton != nullptr) {
		delete m_pCancelButton;
		m_pCancelButton = nullptr;
	}

	if (m_pPrePlaceButton != nullptr) {                        // release the button
		delete m_pPrePlaceButton;
		m_pPrePlaceButton = nullptr;
	}

	ValidateRect(nullptr);
}

void CUserCfgDlg::OnClose() {
	CBmpDialog::OnClose();
}

void CUserCfgDlg::OnDestroy() {
	CDC *pDC = GetDC();

	if (m_pVillages != nullptr) {
		delete m_pVillages;
		m_pVillages = nullptr;
	}

	if (m_pSpeed != nullptr) {
		delete m_pSpeed;
		m_pSpeed = nullptr;
	}

	if (m_pRounds != nullptr) {
		delete m_pRounds;
		m_pRounds = nullptr;
	}
	ReleaseDC(pDC);

	CBmpDialog::OnDestroy();
}


BEGIN_MESSAGE_MAP(CUserCfgDlg, CBmpDialog)
	ON_WM_PAINT()
	ON_WM_HSCROLL()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_PREPLACE, CUserCfgDlg::OnPrePlace)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

} // namespace Life
} // namespace HodjNPodj
} // namespace Bagel
