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

#include "bagel/spacebar/bibble_window.h"
#include "bagel/baglib/master_win.h"
#include "bagel/boflib/gui/button.h"
#include "bagel/boflib/sound.h"
#include "bagel/bagel.h"

namespace Bagel {
namespace SpaceBar {

#define CREDITS_AREA_X1 236
#define CREDITS_AREA_Y1 255
#define CREDITS_AREA_X2 393
#define CREDITS_AREA_Y2 270

// Max number of credits the users is allowed to bet with
#define MAX_AMOUNT  50000

//#define BIBBLE_BACKDROP "BIBBLE.BMP"
#define BIBBLE_BMP      "BIBSTRIP.BMP"
#define BIBBLE_CELS     16
#define BALL_BMP        "BALL2.BMP"
#define BALL_CELS       5
#define ARCH1_BMP       "ARCH1.BMP"
#define ARCH2_BMP       "ARCH2.BMP"
#define ARCH3_BMP       "ARCH3.BMP"
#define CASINO_AUDIO            "CASINO.WAV"

#define SOUNDFILEEXTLOWER       ".wav"
#define SOUNDFILEEXTUPPER       ".WAV"

#define BIBBLE_AUDIO_STARTGAME  "TEMP.WAV"
#define BIBBLE_AUDIO_WINNER     "EVBWIN01.WAV"
#define BIBBLE_AUDIO_LOSER      "TEMP.WAV"
#define BIBBLE_AUDIO_PAYS       "EVBWIN26.WAV"

#define BIBBLE_AUDIO_PAYS2      "EVBWIN43.WAV"
#define BIBBLE_AUDIO_PAYS3      "EVBWIN44.WAV"
#define BIBBLE_AUDIO_PAYS4      "EVBWIN45.WAV"


#define SHOUT_1     "BIBBLE1.WAV"
#define SHOUT_2     "BIBBLE2.WAV"
#define SHOUT_3     "BIBBLE3.WAV"
#define SHOUT_4     "BIBBLE4.WAV"

#define BIBBLE_BUTTON_READY     101
#define BIBBLE_BUTTON_BET1      102
#define BIBBLE_BUTTON_BET5      103
#define BIBBLE_BUTTON_BET10     104
#define BIBBLE_BUTTON_BET50     105
#define BIBBLE_BUTTON_BET100    106
#define BIBBLE_BUTTON_BET500    107
#define BIBBLE_BUTTON_BET1000   108
#define BIBBLE_BUTTON_BET5000   109
#define BIBBLE_BUTTON_CLEARBET  110
#define BIBBLE_BUTTON_QUIT      111
#define BIBBLE_BUTTON_HELP      112

#define HIGHLIGHT_COLOR     13
#define WINHIGHLIGHT_COLOR  1


#define BALL_SPEED      16
#define SPEED_DELAY     50
#define SPEED_DELAY2    60
#define BALL_START_X    0
#define BALL_START_Y1   133
#define BALL_START_Y2   166
#define BALL_START_Y3   198
#define START_YELL      3

#define BIBBLE_START_X1 275
#define BIBBLE_START_X2 365
#define BIBBLE_START_X3 470

#define BIBBLE_START_Y1   5
#define BIBBLE_START_Y2  37
#define BIBBLE_START_Y3  71

#define BIBBLE_NUM_PAINTS      15      // # of Paint calls so ball goes offscreen

#define MASK_COLOR 1

#define BIBBLEDIR "$SBARDIR\\BAR\\CLOSEUP\\BGCH\\"

// Globals
//

const ST_PAYOFFS g_stPayOffs[BIBBLE_NUM_PAYOFFS] = {
	{5, 4},
	{3, 2},
	{2, 1},
	{5, 2},
	{3, 1},
	{5, 1},
	{6, 1},
	{7, 1},
	{8, 1},
	{10, 1},
	{19, 1},
	{35, 1},
	{90, 1},
	{100, 1},
	{125, 1},
	{7500, 1}
};

CBetArea g_cBetAreas[BIBBLE_NUM_BET_AREAS] = {
	CBetArea(0, 66, 304, 66 + 49, 334, 3, 2, "EVBWIN02.WAV", "EVBWIN27.WAV"),
	CBetArea(0, 66, 336, 66 + 49, 366, 2, 1, "EVBWIN03.WAV", "EVBWIN28.WAV"),
	CBetArea(0, 66, 368, 66 + 49, 398, 10, 1, "EVBWIN04.WAV", "EVBWIN29.WAV"),
	CBetArea(0, 66, 401, 66 + 49, 431, 100, 1, "EVBWIN5.WAV", "EVBWIN30.WAV"),

	CBetArea(0, 118, 304, 118 + 47, 334, 7, 1, "EVBWIN06.WAV", "EVBWIN31.WAV"),
	CBetArea(0, 118, 336, 118 + 47, 366, 5, 2, "EVBWIN07.WAV", "EVBWIN32.WAV"),
	CBetArea(0, 118, 368, 118 + 47, 398, 5, 2, "EVBWIN08.WAV", "EVBWIN32.WAV"),
	CBetArea(0, 118, 401, 118 + 47, 431, 7, 1, "EVBWIN09.WAV", "EVBWIN31.WAV"),

	CBetArea(0, 168, 304, 168 + 48, 334, 5, 2, "EVBWIN10.WAV", "EVBWIN32.WAV"),
	CBetArea(0, 168, 336, 168 + 48, 366, 2, 1, "EVBWIN11.WAV", "EVBWIN28.WAV"),
	CBetArea(0, 168, 368, 168 + 48, 398, 5, 1, "EVBWIN12.WAV", "EVBWIN33.WAV"),
	CBetArea(0, 168, 401, 168 + 48, 431, 35, 1, "EVBWIN13.WAV", "EVBWIN34.WAV"),

	CBetArea(0, 471, 306, 471 + 49, 336, 5, 4, "EVBWIN14.WAV", "EVBWIN35.WAV"),
	CBetArea(0, 471, 338, 471 + 49, 368, 5, 4, "EVBWIN17.WAV", "EVBWIN35.WAV"),
	CBetArea(0, 471, 370, 471 + 49, 400, 3, 2, "EVBWIN20.WAV", "EVBWIN27.WAV"),
	CBetArea(0, 471, 403, 471 + 49, 433, 6, 1, "EVBWIN23.WAV", "EVBWIN40.WAV"),

	CBetArea(0, 523, 306, 523 + 47, 336, 3, 1, "EVBWIN15.WAV", "EVBWIN36.WAV"),
	CBetArea(0, 523, 338, 523 + 47, 368, 3, 1, "EVBWIN18.WAV", "EVBWIN36.WAV"),
	CBetArea(0, 523, 370, 523 + 47, 400, 8, 1, "EVBWIN21.WAV", "EVBWIN38.WAV"),
	CBetArea(0, 523, 403, 523 + 47, 433, 125, 1, "EVBWIN24.WAV", "EVBWIN41.WAV"),

	CBetArea(0, 574, 306, 574 + 47, 336, 19, 1, "EVBWIN16.WAV", "EVBWIN37.WAV"),
	CBetArea(0, 574, 338, 574 + 47, 368, 19, 1, "EVBWIN19.WAV", "EVBWIN37.WAV"),
	CBetArea(0, 574, 370, 574 + 47, 400, 90, 1, "EVBWIN22.WAV", "EVBWIN39.WAV"),
	CBetArea(0, 574, 403, 574 + 47, 433, 7500, 1, "EVBWIN25.WAV", "EVBWIN42.WAV")
};

struct ST_BUTTONS {
	const CHAR *m_pszName;
	const CHAR *m_pszUp;
	const CHAR *m_pszDown;
	const CHAR *m_pszFocus;
	const CHAR *m_pszDisabled;
	INT m_nLeft;
	INT m_nTop;
	INT m_nWidth;
	INT m_nHeight;
	INT m_nID;

};

static const ST_BUTTONS g_stButtons[BIBBLE_NUM_BUTTONS] = {
	{ "Bet 1", "bin1.bmp", "bio1.bmp", "big1.bmp", "big1.bmp", 230, 290, 58, 35, BIBBLE_BUTTON_BET1 },
	{ "Bet 5", "bin5.bmp", "bio5.bmp", "big5.bmp", "big5.bmp", 288, 290, 58, 35, BIBBLE_BUTTON_BET5 },
	{ "Bet 10", "bin10.bmp", "bio10.bmp", "big10.bmp", "big10.bmp", 346, 290, 58, 35, BIBBLE_BUTTON_BET10 },
	{ "Bet 50", "bin50.bmp", "bio50.bmp", "big50.bmp", "big50.bmp", 230, 325, 58, 35, BIBBLE_BUTTON_BET50 },
	{ "Bet 100", "bin100.bmp", "bio100.bmp", "big100.bmp", "big100.bmp", 288, 325, 58, 35, BIBBLE_BUTTON_BET100 },
	{ "Bet 500", "bin500.bmp", "bio500.bmp", "big500.bmp", "big500.bmp", 346, 325, 58, 35, BIBBLE_BUTTON_BET500 },
	{ "Bet 1000", "bin1000.bmp", "bio1000.bmp", "big1000.bmp", "big1000.bmp", 230, 360, 58, 35, BIBBLE_BUTTON_BET1000 },
	{ "Bet 5000", "bin5000.bmp", "bio5000.bmp", "big5000.bmp", "big5000.bmp", 288, 360, 58, 35, BIBBLE_BUTTON_BET5000 },
	{ "Clear Bet", "binclear.bmp", "bioclear.bmp", "bigclear.bmp", "bigclear.bmp", 346, 360, 58, 35, BIBBLE_BUTTON_CLEARBET },
	{ "Ready", "binready.bmp", "bioready.bmp", "bioready.bmp", "bioready.bmp", 270, 395, 100, 40, BIBBLE_BUTTON_READY },
	{ "Help", "helpup.bmp", "helpdown.bmp", "helpup.bmp", "helpup.bmp", 31, 445, 200, 30, BIBBLE_BUTTON_HELP },
	{ "Quit", "quitup.bmp", "quitdown.bmp", "quitup.bmp", "quitup.bmp", 407, 445, 200, 30, BIBBLE_BUTTON_QUIT }
};

// Local functions
const CHAR *BuildDir(const CHAR *pszFile);
CBofString LoadTextFile(const CHAR *pszFileName);

static BOOL g_bBibbleHack = FALSE;


CBetArea::CBetArea(UINT nBet, INT left, INT top, INT right, INT bottom, INT nPay1, INT nPay2, const CHAR *pszAudioFile, const CHAR *pszPayFile) {
	m_nBet = nBet;
	m_cRect.SetRect(left, top, right, bottom);
	m_nPayOff1 = nPay1;
	m_nPayOff2 = nPay2;
	m_cAudioFile = pszAudioFile;
	m_cPayFile = pszPayFile;
}


CBibbleWindow::CBibbleWindow() {
	LogInfo("Constructing BibbleBonk...");

	INT i;

	// Inits
	//
	m_pCreditsText = nullptr;
	m_nNumCredits = 0;
	m_nBall1 = 0;
	m_nBall2 = 0;
	m_nBall3 = 0;
	m_nBall1Said = 0;
	m_nBall2Said = 0;
	m_nBall3Said = 0;
	m_nNumShout1 = 0;
	m_nNumShout2 = 0;
	m_nNumShout3 = 0;
	m_nNumShout4 = 0;
	m_nNumTopBonks = 0;
	m_nNumMidBonks = 0;
	m_nNumBotBonks = 0;
	m_pSelected = nullptr;
	m_pBall = nullptr;
	m_pBibble[0] = m_pBibble[1] = m_pBibble[2] = nullptr;
	m_pShouts[0] = m_pShouts[1] = m_pShouts[2] = m_pShouts[3] = nullptr;
	m_pArch1 = m_pArch2 = m_pArch3 = nullptr;

	m_pBkgSnd = nullptr;                       // Added for casino background sounds BFW 12/24/96

	for (i = 0; i < BIBBLE_NUM_BUTTONS; i++) {
		m_pButtons[i] = nullptr;
	}

	SetHelpFilename(BuildDir("BIBBLE.TXT"));

	// Call this thing a closeup so that time won't go  by when entering the closeup
	SetCloseup(TRUE);
}


#if BOF_DEBUG
CBibbleWindow::~CBibbleWindow() {
	Assert(IsValidObject(this));

	LogInfo("Destructed BibbleBonk.");
}
#endif



ERROR_CODE CBibbleWindow::Attach() {
	Assert(IsValidObject(this));

	CBofPalette *pPal;
	INT i;

	const INT nBibbleYPos[BIBBLE_NUM_BIBBLES] = {
		BIBBLE_START_Y1,
		BIBBLE_START_Y2,
		BIBBLE_START_Y3
	};
	const INT nBibbleXPos[BIBBLE_NUM_BIBBLES] = {
		BIBBLE_START_X1,
		BIBBLE_START_X2,
		BIBBLE_START_X3
	};

	const CHAR *pszShouts[BIBBLE_NUM_SHOUTS] = {
		SHOUT_1,
		SHOUT_2,
		SHOUT_3,
		SHOUT_4
	};

	LogInfo("\tCBibbleWindow::Attach()");

	CBagStorageDevWnd::Attach();

	g_bWaitOK = FALSE;

	CBagVar *pVar;

	if ((pVar = VARMNGR->GetVariable("NUGGETS")) != nullptr) {
		m_nNumCredits = pVar->GetNumValue();
	}
	LogInfo(BuildString("\tCredits: %d", m_nNumCredits));

	g_bBibbleHack = FALSE;
	if ((pVar = VARMNGR->GetVariable("BIBBLEHACK")) != nullptr) {
		if (pVar->GetNumValue() != 0) {
			g_bBibbleHack = TRUE;
		}
	}

	// If player has modified the payoffs, then load new payoffs from Bar script
	//
	if (g_bBibbleHack) {

		CBagStorageDev *pSDev;
		CBagObject *pObj;
		INT nPayOff;

		if ((pSDev = SDEVMNGR->GetStorageDevice("BGNM_WLD")) != nullptr) {
			for (i = 0; i < BIBBLE_NUM_BET_AREAS; i++) {
				// The sprite object start in the script at 500
				if ((pObj = pSDev->GetObject(500 + i)) != nullptr) {

					nPayOff = pObj->GetState();
					g_cBetAreas[i].m_nPayOff1 = g_stPayOffs[nPayOff].m_nPay1;
					g_cBetAreas[i].m_nPayOff2 = g_stPayOffs[nPayOff].m_nPay2;
				}
			}
		}
	}

	// Must have a valid backdrop by now
	Assert(m_pBackdrop != nullptr);
	pPal = m_pBackdrop->GetPalette();

	// Setup the text fields
	//
	if ((m_pCreditsText = new CBofText) != nullptr) {
		CBofRect cRect(CREDITS_AREA_X1, CREDITS_AREA_Y1, CREDITS_AREA_X2, CREDITS_AREA_Y2);

		m_pCreditsText->SetupText(&cRect, JUSTIFY_RIGHT, FORMAT_CENTER_RIGHT);
		m_pCreditsText->SetColor(RGB(255, 255, 255));

		m_pCreditsText->SetSize(MapWindowsPointSize(20));
		m_pCreditsText->SetWeight(TEXT_BOLD);
		m_pCreditsText->SetText(BuildString("%d", m_nNumCredits));
	} else {
		ReportError(ERR_MEMORY);
	}

	// Pre-load the "One", "Two", "Three", and "Four" shouts
	//
	for (i = 0; i < BIBBLE_NUM_SHOUTS; i++) {

		if ((m_pShouts[i] = new CBofSound(this, BuildDir(pszShouts[i]), SOUND_MIX)) != nullptr) {

		} else {
			ReportError(ERR_MEMORY);
			break;
		}
	}

	// Pre-load the ball
	//
	if ((m_pBall = new CBofSprite) != nullptr) {
		m_pBall->LoadSprite(BuildDir(BALL_BMP), BALL_CELS);
		m_pBall->SetMaskColor(MASK_COLOR);
		m_pBall->SetZOrder(SPRITE_HINDMOST);
		m_pBall->SetAnimated(TRUE);
		m_pBall->LinkSprite();

	} else {
		ReportError(ERR_MEMORY);
	}

	// Pre-load the bibbles
	//
	if ((m_pMasterBibble = new CBofSprite) != nullptr) {
		m_pMasterBibble->LoadSprite(BuildDir(BIBBLE_BMP), BIBBLE_CELS);
		m_pMasterBibble->SetMaskColor(MASK_COLOR);
		m_pMasterBibble->SetZOrder(SPRITE_TOPMOST);
		m_pMasterBibble->SetAnimated(FALSE);

	} else {
		ReportError(ERR_MEMORY);
	}

	// Dup the bibbles
	//
	for (i = 0; i < BIBBLE_NUM_BIBBLES; i++) {

		if ((m_pBibble[i] = m_pMasterBibble->DuplicateSprite()) != nullptr) {
			m_pBibble[i]->SetPosition(nBibbleXPos[i], nBibbleYPos[i]);
			m_pBibble[i]->LinkSprite();

		} else {
			ReportError(ERR_MEMORY);
		}
	}

	// Load the arch bitmaps that the ball needs to go behind
	//
	if ((m_pArch1 = new CBofSprite) != nullptr) {
		m_pArch1->LoadSprite(BuildDir(ARCH1_BMP));
		m_pArch1->SetMaskColor(MASK_COLOR);
		m_pArch1->SetZOrder(SPRITE_MIDDLE);
		m_pArch1->LinkSprite();
	} else {
		ReportError(ERR_MEMORY);
	}

	if ((m_pArch2 = new CBofSprite) != nullptr) {
		m_pArch2->LoadSprite(BuildDir(ARCH2_BMP));
		m_pArch2->SetMaskColor(MASK_COLOR);
		m_pArch2->SetZOrder(SPRITE_MIDDLE);
		m_pArch2->LinkSprite();

	} else {
		ReportError(ERR_MEMORY);
	}

	if ((m_pArch3 = new CBofSprite) != nullptr) {
		m_pArch3->LoadSprite(BuildDir(ARCH3_BMP));
		m_pArch3->SetMaskColor(MASK_COLOR);
		m_pArch3->SetZOrder(SPRITE_MIDDLE);
		m_pArch3->LinkSprite();

	} else {
		ReportError(ERR_MEMORY);
	}


	// Build all our buttons
	//
	for (i = 0; i < BIBBLE_NUM_BUTTONS; i++) {

		if ((m_pButtons[i] = new CBofBmpButton) != nullptr) {

			CBofBitmap *pUp, *pDown, *pFocus, *pDis;

			pUp = LoadBitmap(BuildDir(g_stButtons[i].m_pszUp), pPal);
			pDown = LoadBitmap(BuildDir(g_stButtons[i].m_pszDown), pPal);
			pFocus = LoadBitmap(BuildDir(g_stButtons[i].m_pszFocus), pPal);
			pDis = LoadBitmap(BuildDir(g_stButtons[i].m_pszDisabled), pPal);

			m_pButtons[i]->LoadBitmaps(pUp, pDown, pFocus, pDis);
#if BOF_MAC
			// Make this our own custom window such that no frame is drawn
			// around the window/button
			m_pButtons[i]->SetCustomWindow(TRUE);
#endif
			m_pButtons[i]->Create(g_stButtons[i].m_pszName, g_stButtons[i].m_nLeft, g_stButtons[i].m_nTop, g_stButtons[i].m_nWidth, g_stButtons[i].m_nHeight, this, g_stButtons[i].m_nID);
#if !BOF_MAC
			m_pButtons[i]->Show();
#endif
		} else {
			ReportError(ERR_MEMORY);
			break;
		}
	}

	// Added for casino background sounds BFW 12/24/96
	if ((m_pBkgSnd = new CBofSound(this, BuildDir(CASINO_AUDIO), SOUND_MIX, 99999)) != nullptr) {
		m_pBkgSnd->Play();
	} else {
		ReportError(ERR_MEMORY);
	}

	// No bet area currently selected
	m_pSelected = nullptr;

	Show();

#if BOF_MAC
	// Show all our buttons after doing the
	for (INT i = 0; i < BIBBLE_NUM_BUTTONS; i++) {
		if (m_pButtons[i] != nullptr) {
			m_pButtons[i]->Show();
		}
	}
#endif

	UpdateWindow();

	CBofCursor::Show();

	return m_errCode;
}


ERROR_CODE CBibbleWindow::Detach() {
	Assert(IsValidObject(this));

	LogInfo("\tCBibbleWindow::Detach()");

	INT i;

	CBofCursor::Hide();

	// Remove any unused credits from the table, and put them back into your
	// Credit Chip
	//
	for (i = 0; i < BIBBLE_NUM_BET_AREAS; i++) {
		Assert(g_cBetAreas[i].m_nBet >= 0);

		m_nNumCredits += g_cBetAreas[i].m_nBet;
		g_cBetAreas[i].m_nBet = 0;
	}

	// Write out new value of nuggets
	//
	CBagVar *pVar;
	if ((pVar = VARMNGR->GetVariable("NUGGETS")) != nullptr) {
		pVar->SetValue(m_nNumCredits);
	}

	// Added for casino background sounds BFW 12/24/96
	if (m_pBkgSnd->IsPlaying()) {
		m_pBkgSnd->Stop();
	}
	if (m_pBkgSnd != nullptr) {
		delete m_pBkgSnd;
		m_pBkgSnd = nullptr;
	}

	// Destroy the ball, bibbles, and bibble shouts
	//
	if (m_pArch3 != nullptr) {
		delete m_pArch3;
		m_pArch3 = nullptr;
	}
	if (m_pArch2 != nullptr) {
		delete m_pArch2;
		m_pArch2 = nullptr;
	}
	if (m_pArch1 != nullptr) {
		delete m_pArch1;
		m_pArch1 = nullptr;
	}
	for (i = 0; i < BIBBLE_NUM_BIBBLES; i++) {
		if (m_pBibble[i] != nullptr) {
			delete m_pBibble[i];
			m_pBibble[i] = nullptr;
		}
	}
	if (m_pMasterBibble != nullptr) {
		delete m_pMasterBibble;
		m_pMasterBibble = nullptr;
	}
	if (m_pBall != nullptr) {
		delete m_pBall;
		m_pBall = nullptr;
	}
	for (i = 0; i < BIBBLE_NUM_SHOUTS; i++) {
		if (m_pShouts[i] != nullptr) {
			delete m_pShouts[i];
			m_pShouts[i] = nullptr;
		}
	}

	// Destroy all buttons
	//
	for (i = 0; i < BIBBLE_NUM_BUTTONS; i++) {
		if (m_pButtons[i] != nullptr) {
			delete m_pButtons[i];
			m_pButtons[i] = nullptr;
		}
	}

	if (m_pCreditsText != nullptr) {
		delete m_pCreditsText;
		m_pCreditsText = nullptr;
	}

	// Close sprite lib
	CBofSprite::CloseLibrary();

	KillBackdrop();

	g_bWaitOK = TRUE;

	CBagStorageDevWnd::Detach();

	// Playing BibbleBonk has made 1 turn go by.
	VARMNGR->IncrementTimers();

	return m_errCode;
}


VOID CBibbleWindow::OnMainLoop() {
	// Do nothing
}


VOID CBibbleWindow::OnPaint(CBofRect *pRect) {
	Assert(IsValidObject(this));
	Assert(pRect != nullptr);

	CBofBitmap *pBmp;
	CHAR szBuf[20];
	INT i;

	// Render offscreen
	//
	if (m_pBackdrop != nullptr && (pBmp = GetWorkBmp()) != nullptr) {

		m_pBackdrop->Paint(pBmp, pRect, pRect);

		// Paint the bibbles - just sitting there
		//
		for (i = 0; i < BIBBLE_NUM_BIBBLES; i++) {

			if (m_pBibble[i] != nullptr) {
				m_pBibble[i]->PaintSprite(pBmp, m_pBibble[i]->GetPosition());
			}
		}

		if (m_pArch1 != nullptr) {
			m_pArch1->PaintSprite(pBmp, 238, 107);
		}
		if (m_pArch2 != nullptr) {
			m_pArch2->PaintSprite(pBmp, 336, 145);
		}
		if (m_pArch3 != nullptr) {
			m_pArch3->PaintSprite(pBmp, 435, 178);
		}

		CBofRect cRect;

		// Update the display of all the bet areas;
		//
		for (i = 0; i < BIBBLE_NUM_BET_AREAS; i++) {
			// Add bet ammount text
			Common::sprintf_s(szBuf, "%d", g_cBetAreas[i].m_nBet);
			cRect = g_cBetAreas[i].m_cRect;
			cRect.top += cRect.Height() / 2;

			PaintText(pBmp, &cRect, szBuf, 16, TEXT_NORMAL, CTEXT_COLOR, JUSTIFY_RIGHT, FORMAT_BOT_RIGHT);
		}

		// Now update the screen
		pBmp->Paint(this, pRect, pRect);

		if (m_pSelected != nullptr) {
			Highlight(m_pSelected, HIGHLIGHT_COLOR);
		}

		// Update # of credits display
		//
		if (m_pCreditsText != nullptr) {
			m_pCreditsText->Display(this);
		}

#if BOF_MAC
		// out here in mac land, we need to layer out windows appropriately,
		// so make sure our buttons are on top.
		for (INT i = 0; i < BIBBLE_NUM_BUTTONS; i++) {
			if (m_pButtons[i] != nullptr) {
				m_pButtons[i]->Paint();
			}
		}
#endif
	}
}


VOID CBibbleWindow::OnBofButton(CBofObject *pObject, INT nState) {
	Assert(IsValidObject(this));
	Assert(pObject != nullptr);

	CBofButton *pButton;
	INT nBet;

	pButton = (CBofButton *)pObject;

	if (nState == BUTTON_CLICKED) {

		nBet = 0;
		switch (pButton->GetControlID()) {

		case BIBBLE_BUTTON_READY:
			LogInfo("\tClicked READY button");
			PlayGame();
			break;

		case BIBBLE_BUTTON_BET1:
			LogInfo("\tClicked BET 1");
			nBet = min(m_nNumCredits, 1);
			break;

		case BIBBLE_BUTTON_BET5:
			LogInfo("\tClicked BET 5");
			nBet = min(m_nNumCredits, 5);
			break;

		case BIBBLE_BUTTON_BET10:
			LogInfo("\tClicked BET 10");
			nBet = min(m_nNumCredits, 10);
			break;

		case BIBBLE_BUTTON_BET50:
			LogInfo("\tClicked BET 50");
			nBet = min(m_nNumCredits, 50);
			break;

		case BIBBLE_BUTTON_BET100:
			LogInfo("\tClicked BET 100");
			nBet = min(m_nNumCredits, 100);
			break;

		case BIBBLE_BUTTON_BET500:
			LogInfo("\tClicked BET 500");
			nBet = min(m_nNumCredits, 500);
			break;

		case BIBBLE_BUTTON_BET1000:
			LogInfo("\tClicked BET 1000");
			nBet = min(m_nNumCredits, 1000);
			break;

		case BIBBLE_BUTTON_BET5000:
			LogInfo("\tClicked BET 5000");
			nBet = min(m_nNumCredits, 5000);
			break;

		case BIBBLE_BUTTON_CLEARBET:
			LogInfo("\tClicked Clear Bet");
			if (m_pSelected != nullptr) {
				nBet -= m_pSelected->m_nBet;
			}
			break;

		case BIBBLE_BUTTON_QUIT:
			LogInfo("\tClicked Quit");
			Close();
			break;

		case BIBBLE_BUTTON_HELP: {
			LogInfo("\tClicked Help");

			CBagel *pApp;
			CBagMasterWin *pWin;

			if ((pApp = CBagel::GetBagApp()) != nullptr) {
				if ((pWin = pApp->GetMasterWnd()) != nullptr) {
					pWin->OnHelp(BuildDir("BIBBLE.TXT"));
				}
			}
			break;
		}

		default:
			LogWarning(BuildString("Clicked Unknown Button with ID %d", pButton->GetControlID()));
			break;
		}

		if (nBet != 0) {
			if (m_nNumCredits < MAX_AMOUNT) {
				if (m_pSelected != nullptr) {
					LogInfo(BuildString("\tHave %d Credits, Betting %d credits", m_nNumCredits, nBet));
					m_nNumCredits -= nBet;
					m_pSelected->m_nBet += nBet;
					LogInfo(BuildString("\t%d Credits remaining.", m_nNumCredits));
					Highlight(m_pSelected, HIGHLIGHT_COLOR);
				}

				// The user is not allowed to make anymore bets (It could roll a ULONG)
				//
			} else {

				CHAR szBuf[256];
				Common::strcpy_s(szBuf, "The bonkier apologizes, explaining that in light of your winning streak, the house can no longer accept your bets.");
				CBofString cString(szBuf, 256);

				CBofRect cRect(0, 440, 640 - 1, 480 - 1);

				CBofBitmap cBmp(cRect.Width(), cRect.Height(), (CBofPalette *)nullptr, FALSE);
				cBmp.CaptureScreen(this, &cRect);

				PaintBeveledText(this, &cRect, cString, FONT_15POINT, TEXT_NORMAL, RGB(255, 255, 255), JUSTIFY_WRAP, FORMAT_TOP_LEFT);
				WaitForInput();

				cBmp.Paint(this, &cRect);
			}

			DisplayCredits();
		}
	}
}

VOID CBibbleWindow::OnClose() {
	g_engine->quitGame();
}

ERROR_CODE CBibbleWindow::DisplayCredits() {
	Assert(IsValidObject(this));

	// Display new number of credits
	//
	if (m_pCreditsText != nullptr) {
		CHAR szBuf[20];

		Common::sprintf_s(szBuf, "%d", m_nNumCredits);
		m_pCreditsText->SetText(szBuf);
		m_pCreditsText->Display(this);
	}

	return m_errCode;
}


ERROR_CODE CBibbleWindow::PlayGame() {
	LogInfo(BuildString("\tPlaying BibbleBonk, Starting Credits: %d", m_nNumCredits));

	INT nPayoff, i;
	BOOL bWin;

	// Remove any current highlight
	//
	if (m_pSelected != nullptr)
		UnHighlight(m_pSelected);

	// Pre-determine the outcome
	//
	CalcOutcome();

	//
	// Bonk those Bibbles (Play game based on pre-determined outcome)
	//
	BonkBibble(m_nBall1, m_nBall1Said);
	BonkBibble(m_nBall2, m_nBall2Said);
	BonkBibble(m_nBall3, m_nBall3Said);

	// Assume we will lose
	bWin = FALSE;

	// Find each winner
	//
	for (i = 0; i < BIBBLE_NUM_BET_AREAS; i++) {

		// If user bet on this square
		//
		if (g_cBetAreas[i].m_bWon) {

			// Highlight this winning square
			Highlight(&g_cBetAreas[i], WINHIGHLIGHT_COLOR);

			if (g_cBetAreas[i].m_nBet != 0) {

				if (!bWin) {
					LogInfo("\tWinner");

					BofPlaySoundEx(BuildDir(BIBBLE_AUDIO_WINNER), SOUND_MIX | SOUND_QUEUE, 7, FALSE);
				}
				bWin = TRUE;

				// Play the "Zip Middle Bibble, pays seven to one", etc...
				//

				// "Zip Middle Bibble"
				//
				BofPlaySoundEx(BuildDir(g_cBetAreas[i].m_cAudioFile), SOUND_MIX | SOUND_QUEUE, 7, FALSE);

				// "Pays"
				BofPlaySoundEx(BuildDir(BIBBLE_AUDIO_PAYS), SOUND_MIX | SOUND_QUEUE, 7, FALSE);

				if (!g_bBibbleHack) {

					// "7 to 1"
					//
					BofPlaySoundEx(BuildDir(g_cBetAreas[i].m_cPayFile), SOUND_MIX | SOUND_QUEUE, 7, TRUE);

				} else {

					switch (g_engine->getRandomNumber() % 3) {
					case 0:
						BofPlaySoundEx(BuildDir(BIBBLE_AUDIO_PAYS2), SOUND_MIX | SOUND_QUEUE, 7, TRUE);
						break;

					case 1:
						BofPlaySoundEx(BuildDir(BIBBLE_AUDIO_PAYS3), SOUND_MIX | SOUND_QUEUE, 7, TRUE);
						break;

					case 2:
						BofPlaySoundEx(BuildDir(BIBBLE_AUDIO_PAYS4), SOUND_MIX | SOUND_QUEUE, 7, TRUE);
						break;
					}
				}

				// Calc Payoff
				nPayoff = (g_cBetAreas[i].m_nBet * g_cBetAreas[i].m_nPayOff1) / g_cBetAreas[i].m_nPayOff2;

				// Mo' money, Mo' money
				m_nNumCredits += nPayoff;

				// Display new number of credits
				//
				DisplayCredits();

				LogInfo(BuildString("\tWinner on square %d.  Pays %d credits", i, nPayoff));
			}
		}
	}

	// Otherwise, play loser audios
	//
	if (!bWin) {

		LogInfo("\tLoser");
		//BofPlaySound(BuildDir(BIBBLE_AUDIO_LOSER), SOUND_WAVE | SOUND_QUEUE | SOUND_ASYNCH);
	}

	Sleep(2000);

	// Clear all bets (On table)
	//
	LogInfo("\tClearing all Bets");
	for (i = 0; i < BIBBLE_NUM_BET_AREAS; i++) {
		g_cBetAreas[i].m_nBet = 0;
		UnHighlight(&g_cBetAreas[i]);
	}

	m_pSelected = nullptr;

	LogInfo(BuildString("\tDone BibbleBonk.  Credits: %d", m_nNumCredits));

	return m_errCode;
}


CBofString LoadTextFile(const CHAR *pszFileName) {
	Assert(pszFileName != nullptr);

	CBofString cString;
	CBofFile cFile(pszFileName, CBF_TEXT | CBF_READONLY);
	CHAR *pszBuf;
	INT nLength;

	nLength = cFile.GetLength();

	if ((pszBuf = (CHAR *)BofAlloc(nLength + 1)) != nullptr) {
		BofMemSet(pszBuf, 0, nLength + 1);
		cFile.Read(pszBuf, nLength);
		cString = pszBuf;

		BofFree(pszBuf);
	}
	return cString;
}

ERROR_CODE CBibbleWindow::BonkBibble(INT nBibbleID, INT nShouts) {
	Assert(IsValidObject(this));
	Assert(nBibbleID >= 1 && nBibbleID <= 3);
	Assert(nShouts >= 1 && nShouts <= 4);

	CBofPoint cBallPosition;
	CBofSprite *pBibble;
	INT i;

	INT nBallStartYPos[] = {
		BALL_START_Y1,
		BALL_START_Y2,
		BALL_START_Y3
	};

	//
	// Display the ball rolling down the chute and then hitting the bibble,
	// causeing the bibble to roll over the ball (which continues off the
	// edge of the screen).  The bibble shouts the specfied number.
	//

	// Set ball initial position
	//
	cBallPosition.x = BALL_START_X;
	cBallPosition.y = nBallStartYPos[nBibbleID - 1];

	// Get a local bibble
	pBibble = m_pBibble[nBibbleID - 1];
	pBibble->SetAnimated(TRUE);

	for (;;) {

		cBallPosition.x += BALL_SPEED;
		m_pBall->PaintSprite(this, cBallPosition);

		if (m_pBall->TestInterception(pBibble)) {

			m_pBall->EraseSprite(this);

			for (i = 0; i < pBibble->GetCelCount(); i++) {
				pBibble->PaintSprite(this, pBibble->GetPosition());

				if (i == START_YELL) {
					// Start shouting
					m_pShouts[nShouts - 1]->Play();
				}

				Sleep(SPEED_DELAY2);
			}
			break;
		}
		Sleep(SPEED_DELAY);
	}

	pBibble->SetAnimated(FALSE);

	return m_errCode;
}


VOID CBibbleWindow::CalcOutcome() {
	Assert(IsValidObject(this));

	INT i;

	const INT g_nBibbleTable[] = {
		1, 1,
		2, 2, 2, 2, 2,
		3, 3, 3
	};

	const INT g_nPayTable[] = {
		4,
		3,
		2,
	};

	// Start from scratch
	//
	m_nBall1 = 0;
	m_nBall2 = 0;
	m_nBall3 = 0;
	m_nBall1Said = 0;
	m_nBall2Said = 0;
	m_nBall3Said = 0;
	m_nNumShout1 = 0;
	m_nNumShout2 = 0;
	m_nNumShout3 = 0;
	m_nNumShout4 = 0;
	m_nNumTopBonks = 0;
	m_nNumMidBonks = 0;
	m_nNumBotBonks = 0;

	// Clear internal betting area
	for (i = 0; i < BIBBLE_NUM_BET_AREAS; i++) {
		g_cBetAreas[i].m_bWon = FALSE;
	}

	//
	// Determine which Bibbles were hit by each ball, and
	// Determine what each of those Bibbles shouted.
	//

	// Ball 1
	//
	m_nBall1 = g_nBibbleTable[g_engine->getRandomNumber() % 10];
	m_nBall1Said = (g_engine->getRandomNumber() % g_nPayTable[m_nBall1 - 1]) + 1;
	LogInfo(BuildString("\tBall 1 hit Bibble %d", m_nBall1));
	LogInfo(BuildString("\tBibble says: %d", m_nBall1Said));

	// Count number of each Shout
	//
	switch (m_nBall1Said) {

	case 1:
		m_nNumShout1++;
		break;

	case 2:
		m_nNumShout2++;
		break;

	case 3:
		m_nNumShout3++;
		break;

	case 4:
		m_nNumShout4++;
		break;

	default:
		LogWarning("Invalid case");
		break;
	}

	// Ball 2
	//
	m_nBall2 = g_nBibbleTable[g_engine->getRandomNumber() % 10];
	m_nBall2Said = (g_engine->getRandomNumber() % g_nPayTable[m_nBall2 - 1]) + 1;
	LogInfo(BuildString("\tBall 2 hit Bibble %d", m_nBall2));
	LogInfo(BuildString("\tBibble says: %d", m_nBall2Said));

	// Count number of each Shout
	//
	switch (m_nBall2Said) {

	case 1:
		m_nNumShout1++;
		break;

	case 2:
		m_nNumShout2++;
		break;

	case 3:
		m_nNumShout3++;
		break;

	case 4:
		m_nNumShout4++;
		break;

	default:
		LogWarning("Invalid case");
		break;
	}

	// Ball 3
	//
	m_nBall3 = g_nBibbleTable[g_engine->getRandomNumber() % 10];
	m_nBall3Said = (g_engine->getRandomNumber() % g_nPayTable[m_nBall3 - 1]) + 1;
	LogInfo(BuildString("\tBall 3 hit Bibble %d", m_nBall3));
	LogInfo(BuildString("\tBibble says: %d", m_nBall3Said));

	// Count number of each Shout
	//
	switch (m_nBall3Said) {

	case 1:
		m_nNumShout1++;
		break;

	case 2:
		m_nNumShout2++;
		break;

	case 3:
		m_nNumShout3++;
		break;

	case 4:
		m_nNumShout4++;
		break;

	default:
		LogWarning("Invalid case");
		break;
	}

	// Count number of each bonks
	//
	m_nNumTopBonks = (m_nBall1 == 1 ? 1 : 0) + (m_nBall2 == 1 ? 1 : 0) + (m_nBall3 == 1 ? 1 : 0);
	m_nNumMidBonks = (m_nBall1 == 2 ? 1 : 0) + (m_nBall2 == 2 ? 1 : 0) + (m_nBall3 == 2 ? 1 : 0);
	m_nNumBotBonks = (m_nBall1 == 3 ? 1 : 0) + (m_nBall2 == 3 ? 1 : 0) + (m_nBall3 == 3 ? 1 : 0);

	// Determine winning bets
	//

	g_cBetAreas[m_nNumTopBonks + 0].m_bWon = TRUE;
	g_cBetAreas[m_nNumMidBonks + 4].m_bWon = TRUE;
	g_cBetAreas[m_nNumBotBonks + 8].m_bWon = TRUE;

	for (i = 12; i < BIBBLE_NUM_BET_AREAS; i++) {

		switch (i % 4) {

		case 0:
			if (m_nNumShout1 >= ((i - 12) / 4) + 1) {
				g_cBetAreas[i].m_bWon = TRUE;
			}
			break;

		case 1:
			if (m_nNumShout2 >= ((i - 13) / 4) + 1) {
				g_cBetAreas[i].m_bWon = TRUE;
			}
			break;

		case 2:
			if (m_nNumShout3 >= ((i - 14) / 4) + 1) {
				g_cBetAreas[i].m_bWon = TRUE;
			}
			break;

		case 3:
			if (m_nNumShout4 >= ((i - 15) / 4) + 1) {
				g_cBetAreas[i].m_bWon = TRUE;
			}
			break;
		}
	}

	LogInfo(BuildString("\tNumber of Top Bonks: %d", m_nNumTopBonks));
	LogInfo(BuildString("\tNumber of Mid Bonks: %d", m_nNumMidBonks));
	LogInfo(BuildString("\tNumber of Bot Bonks: %d", m_nNumBotBonks));

	LogInfo(BuildString("\tNumber of '1' Babbles: %d", m_nNumShout1));
	LogInfo(BuildString("\tNumber of '2' Babbles: %d", m_nNumShout2));
	LogInfo(BuildString("\tNumber of '3' Babbles: %d", m_nNumShout3));
	LogInfo(BuildString("\tNumber of '4' Babbles: %d", m_nNumShout4));
}

VOID CBibbleWindow::OnLButtonDblClk(UINT /*nFlags*/, CBofPoint *pPoint) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

	CBetArea *pArea, *pPrevArea;
	INT i, nBet;

	nBet = min(m_nNumCredits, 100);

	for (i = 0; i < BIBBLE_NUM_BET_AREAS; i++) {

		pArea = &g_cBetAreas[i];

		// If this is the area the user selected
		//
		if (pArea->m_cRect.PtInRect(*pPoint)) {

			pPrevArea = m_pSelected;

			// Keep track of selected area
			m_pSelected = pArea;

			LogInfo(BuildString("\tDouble Clicked on Square: %d", i));

			// remove any previous selection highlight
			if (pPrevArea != nullptr) {
				UnHighlight(pPrevArea);
			}

			LogInfo(BuildString("\tHave %d Credits, Betting %d credits", m_nNumCredits, nBet));
			m_nNumCredits -= nBet;
			m_pSelected->m_nBet += nBet;
			LogInfo(BuildString("\t%d Credits remaining.", m_nNumCredits));

			// highlight the area selected
			Highlight(m_pSelected, HIGHLIGHT_COLOR);

			DisplayCredits();

			break;
		}
	}

}


VOID CBibbleWindow::OnLButtonUp(UINT /*nFlags*/, CBofPoint * /*pPoint*/, void *) {
	Assert(IsValidObject(this));
}


VOID CBibbleWindow::OnLButtonDown(UINT /*nFlags*/, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

#if 1

	CBetArea *pArea, *pPrevArea;
	INT i;

	for (i = 0; i < BIBBLE_NUM_BET_AREAS; i++) {

		pArea = &g_cBetAreas[i];

		// If this is the area the user selected
		//
		if (pArea->m_cRect.PtInRect(*pPoint)) {

			pPrevArea = m_pSelected;

			// Keep track of selected area
			m_pSelected = pArea;

			LogInfo(BuildString("\tSelected %d", i));

			// remove any previous selection highlight
			if (pPrevArea != nullptr) {
				UnHighlight(pPrevArea);
			}

			// highlight the area selected
			Highlight(pArea, HIGHLIGHT_COLOR);

			break;
		}
	}

#else
	LogInfo(BuildString("%d, %d", pPoint->x, pPoint->y));
#endif
}


ERROR_CODE CBibbleWindow::Highlight(CBetArea *pArea, UBYTE nColor) {
	Assert(IsValidObject(this));
	Assert(pArea != nullptr);

	CBofBitmap cBmp(pArea->m_cRect.Width(), pArea->m_cRect.Height(), m_pBackdrop->GetPalette());

	Assert(m_pBackdrop != nullptr);
	CBofRect r = cBmp.GetRect();
	m_pBackdrop->Paint(&cBmp, &r, &pArea->m_cRect);

	// Add hightlight rectangle
	CBofRect cRect;
	cRect = cBmp.GetRect();

	cBmp.DrawRect(&cRect, nColor);
	cRect.left += 1;
	cRect.right -= 1;
	cRect.top += 1;
	cRect.bottom -= 1;

	cBmp.DrawRect(&cRect, nColor);

	// Add bet ammount text
	CHAR szBuf[20];

	Common::sprintf_s(szBuf, "%d", pArea->m_nBet);
	cRect = cBmp.GetRect();
	cRect.top += cRect.Height() / 2;

	PaintText(&cBmp, &cRect, szBuf, 16, TEXT_NORMAL, CTEXT_COLOR, JUSTIFY_RIGHT, FORMAT_BOT_RIGHT);

	// Paint result to screen
	cBmp.Paint(this, &pArea->m_cRect);

	return m_errCode;
}


ERROR_CODE CBibbleWindow::UnHighlight(CBetArea *pArea) {
	Assert(IsValidObject(this));
	Assert(pArea != nullptr);

	CBofBitmap cBmp(pArea->m_cRect.Width(), pArea->m_cRect.Height(), m_pBackdrop->GetPalette());

	Assert(m_pBackdrop != nullptr);

	// Copy bet area
	CBofRect r = cBmp.GetRect();
	m_pBackdrop->Paint(&cBmp, &r, &pArea->m_cRect);

	// Add bet ammount text
	CBofRect cRect;
	CHAR szBuf[20];

	Common::sprintf_s(szBuf, "%d", pArea->m_nBet);
	cRect = cBmp.GetRect();
	cRect.top += cRect.Height() / 2;

	PaintText(&cBmp, &cRect, szBuf, 16, TEXT_NORMAL, CTEXT_COLOR, JUSTIFY_RIGHT, FORMAT_BOT_RIGHT);

	// Paint to screen
	cBmp.Paint(this, &pArea->m_cRect);

	return m_errCode;
}

VOID CBibbleWindow::OnKeyHit(ULONG lKey, ULONG /*lRepCount*/) {
	Assert(IsValidObject(this));

	switch (lKey) {
	case BKEY_ESC:
		Close();
		break;
	}
}


const CHAR *BuildDir(const CHAR *pszFile) {
	return formPath(BIBBLEDIR, pszFile);
}

} // namespace SpaceBar
} // namespace Bagel
