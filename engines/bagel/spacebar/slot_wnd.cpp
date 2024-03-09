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

#include "bagel/spacebar/slot_wnd.h"
#include "bagel/baglib/bagel.h"
#include "bagel/boflib/sound.h"
#include "bagel/bagel.h"

namespace Bagel {
namespace SpaceBar {

CBofRect Slot0Rect(220, 46, 464, 98);
CBofRect Slot1Rect(469, 14, 522, 187);
CBofRect Slot2Rect(378, 192, 623, 243);
CBofRect Slot3Rect(321, 102, 374, 274);

CBofRect CreditRect(24, 334, 135, 360);
CBofRect BetRect(15, 410, 131, 432);
CBofRect OddRect(401, 125, 443, 166);
CBofRect FixRect(150, 306, 150 + 109 - 1, 306 + 64 - 1);

#define FIXBMP      "BGNDDN.BMP"
#define MAX_CREDITS     200000
// Added for casino background sounds BFW 12/24/96
#define CASINO_AUDIO            "CASINO.WAV"

#if BOF_MAC || BOF_WINMAC
#define BGCBDIR         "$SBARDIR:BAR:CLOSEUP:BGCB:"
// scg 01.07.97 added PLAYWAVONMAC conditional
#if !PLAYWAVONMAC
#define WINAUDIO        "BGNWIN.SND"
#define SLOTAUDIO       "BGNSLOT.SND"
#else
#define WINAUDIO        "BGNWIN.WAV"
#define SLOTAUDIO       "BGNSLOT.WAV"
#endif
#else
#define BGCBDIR         "$SBARDIR\\BAR\\CLOSEUP\\BGCB\\"
#define WINAUDIO        "BGNWIN.WAV"
#define SLOTAUDIO       "BGNSLOT.WAV"
#endif

typedef struct {
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
} ST_BUTTONS;

static ST_BUTTONS g_stButtons[NUM_SLOTBUTT] = {
	{ "One",    "bgng01up.bmp", "bgng01dn.bmp", "bgng01up.bmp", "bgng01ds.bmp", 380, 304, 60, 42, ONE },
	{ "Four",   "bgng04up.bmp", "bgng04dn.bmp", "bgng04up.bmp", "bgng04ds.bmp", 380, 346, 60, 42, FOUR },
	{ "Seven",  "bgng07up.bmp", "bgng07dn.bmp", "bgng07up.bmp", "bgng07ds.bmp", 385, 391, 60, 42, SEVEN },
	{ "Ten",    "bgng10up.bmp", "bgng10dn.bmp", "bgng10up.bmp", "bgng10ds.bmp", 440, 304, 60, 42, TEN },
	{ "Forty",  "bgng40up.bmp", "bgng40dn.bmp", "bgng40up.bmp", "bgng40ds.bmp", 442, 346, 60, 42, FORTY },
	{ "Sevty",  "bgng70up.bmp", "bgng70dn.bmp", "bgng70up.bmp", "bgng70ds.bmp", 445, 391, 60, 42, SEVENTY },
	{ "OneH",   "bgng1hup.bmp", "bgng1hdn.bmp", "bgng1hup.bmp", "bgng1hds.bmp", 500, 304, 60, 42, OHNDRD },
	{ "FourH",  "bgng4hup.bmp", "bgng4hdn.bmp", "bgng4hup.bmp", "bgng4hds.bmp", 502, 346, 60, 42, RHNDRD },
	{ "SevH",   "bgng7hup.bmp", "bgng7hdn.bmp", "bgng7hup.bmp", "bgng7hds.bmp", 507, 391, 60, 42, SHNDRD },
	{ "OneK",   "bgng1kup.bmp", "bgng1kdn.bmp", "bgng1kup.bmp", "bgng1kds.bmp", 560, 304, 60, 42, OTHSND },
	{ "FourK",  "bgng4kup.bmp", "bgng4kdn.bmp", "bgng4kup.bmp", "bgng4kds.bmp", 564, 346, 60, 42, RTHSND },
	{ "SevK",   "bgng7kup.bmp", "bgng7kdn.bmp", "bgng7kup.bmp", "bgng7kds.bmp", 570, 391, 60, 42, STHSND },
	{ "ClrBet", "bgneup.bmp",   "bgnedn.bmp", "bgneup.bmp", "bgneds.bmp", 143, 371, 118, 69, CLRBET },
	{ "BetAll", "bgnfup.bmp",   "bgnfdn.bmp", "bgnfup.bmp", "bgnfds.bmp", 262, 322, 120, 90, BETALL },
	{ "GO",     "bgncdn.bmp",   "bgncdn.bmp", "bgncdn.bmp", "bgncds.bmp", 401, 125, 42, 41, GO },
	{ "Help",   "helpup.bmp",   "helpdown.bmp", "helpup.bmp", "helpup.bmp", 31, 445, 200, 30, SLOTHELP},
	{ "Quit",   "quitup.bmp",   "quitdown.bmp", "quitup.bmp", "quitup.bmp", 407, 445, 200, 30, SLOTQUIT}
};

typedef struct {
	const CHAR *m_pszName[SLOT_BMP_NUM];
} ST_SLOTBMPS;

static ST_SLOTBMPS g_stSlotBmps[SLOT_NUM] = {
	{"bgnst1.bmp", "bgnsq1.bmp", "bgnsc1.bmp", "bgnsn1.bmp", "bgnss1.bmp", "bgnsm1.bmp", "bgnse1.bmp", "bgnsa1.bmp", "bgnsh1.bmp" },
	{"bgnst3.bmp", "bgnsq3.bmp", "bgnsc3.bmp", "bgnsn3.bmp", "bgnss3.bmp", "bgnsm3.bmp", "bgnse3.bmp", "bgnsa3.bmp", "bgnsh3.bmp" },
	{"bgnst4.bmp", "bgnsq4.bmp", "bgnsc4.bmp", "bgnsn4.bmp", "bgnss4.bmp", "bgnsm4.bmp", "bgnse4.bmp", "bgnsa4.bmp", "bgnsh4.bmp" },
	{"bgnst2.bmp", "bgnsq2.bmp", "bgnsc2.bmp", "bgnsn2.bmp", "bgnss2.bmp", "bgnsm2.bmp", "bgnse2.bmp", "bgnsa2.bmp", "bgnsh2.bmp" }
};

// Local functions
const CHAR *BuildSlotDir(const CHAR *pszFile);

static BOOL g_bFix = FALSE;

SBarSlotWnd::SBarSlotWnd() : CBagStorageDevWnd() {
	INT i, j;

	for (i = 0; i < NUM_SLOTBUTT; i++) {
		m_pSlotButs[i] = NULL;
	}

	// Init all our slotbmp
	for (i = 0; i < SLOT_NUM; i++) {
		m_cSlots[i].m_nIdx = g_engine->getRandomNumber() % SLOT_BMP_NUM;
		for (j = 0; j < SLOT_BMP_NUM; j++) {
			m_cSlots[i].m_pSlotBmp[j] = NULL;
		}
	}

	m_cSlots[0].m_cSlotRect = Slot0Rect;
	m_cSlots[1].m_cSlotRect = Slot1Rect;
	m_cSlots[2].m_cSlotRect = Slot2Rect;
	m_cSlots[3].m_cSlotRect = Slot3Rect;

	m_bFixBet = FALSE;
	m_nBet = 0;
	m_nCredit = 0;
	m_pCredText = NULL;
	m_pBetText = NULL;
	m_pOddsText = NULL;
	m_bAutoDecrement = FALSE;

	m_nPayOff1 = 0;
	m_nPayOff2 = 0;

	m_bFixBmp = NULL;
	m_pWinSound = NULL;
	m_pSlotSound = NULL;
	m_pLoseBmp = NULL;

	SetHelpFilename(BuildSlotDir("SLOT.TXT"));

	// jwl 1.13.97 call this thing a closeup so that time won't go
	// by when entering the closeup
	SetCloseup(TRUE);
}


VOID SBarSlotWnd::OnPaint(CBofRect *pRect) {
	Assert(IsValidObject(this));

	if (!ErrorOccurred()) {

		//ERROR_CODE errCode = ERR_NONE;
		CBofBitmap *pBackBmp;

		//Paint the storage device
		if ((pBackBmp = GetBackdrop()) != NULL) {

			Assert(GetWorkBmp() != NULL);

			// erase everything from the background
			GetWorkBmp()->Paint(pBackBmp, pRect, pRect);

			// paint all the objects to the background
			PaintStorageDevice(NULL, pBackBmp, pRect);
		}

		// Paint all our slotbmp
		for (int i = 0; i < SLOT_NUM; i++) {

			if (m_cSlots[i].m_pSlotBmp[m_cSlots[i].m_nIdx] != NULL) {
				m_cSlots[i].m_pSlotBmp[m_cSlots[i].m_nIdx]->Paint(pBackBmp, &m_cSlots[i].m_cSlotRect, NULL, CBagel::GetBagApp()->GetChromaColor());
			}
		}

		if (m_bFixBet && m_bFixBmp != NULL) {
			m_bFixBmp->Paint(pBackBmp, FixRect.left, FixRect.top);
		}

		// Paint the backdrop
		if (GetBackdrop())
			PaintBackdrop();

		UpdateText();

		// jwl 1.17.97 must show buttons after bringing the window frontmost
#if BOF_MAC
		for (INT i = 0; i < NUM_SLOTBUTT; i++) {
			if (m_pSlotButs[i] != NULL) {
				m_pSlotButs[i]->Show();
			}
		}
#endif

		/*
		    // Update credits
		    if (m_pCredText != NULL){
		        m_pCredText->Display(this);
		    }

		    // Update bet
		    if (m_pBetText != NULL){
		        m_pBetText->Display(this);
		    }

		    // Update Odds
		    if (m_pOddsText != NULL){
		        m_pOddsText->Display(this);
		    }
		*/
	}
}

VOID SBarSlotWnd::OnMainLoop(VOID) {
	// Do nothing
}

/*****************************************************************************
*
*  Attach -
*
*  DESCRIPTION:
*   This is where we allocate storage, load bitmaps, etc.
*
*
*  RETURNS:
*       ERROR_CODE
*
*****************************************************************************/
ERROR_CODE  SBarSlotWnd::Attach() {
	CBofPalette *pPal;
	INT i;

	m_bPaused = FALSE;
	m_bLose = FALSE;
	m_bFixBet = FALSE;

	if ((m_pSlotSound = new CBofSound(this, BuildSlotDir(SLOTAUDIO), SOUND_MIX, 1)) != NULL) {

	} else {
		ReportError(ERR_MEMORY, "Could not allocate a CBofSound");
	}


	if (CBagStorageDevWnd::Attach() == ERR_NONE) {

		// Must have a valid backdrop by now
		Assert(m_pBackdrop != NULL);
		pPal = m_pBackdrop->GetPalette();

		m_bFixBmp = new CBofBitmap(BuildSlotDir(FIXBMP));

		// Build all our buttons
		for (i = 0; i < NUM_SLOTBUTT; i++) {
			if ((m_pSlotButs[i] = new CBofBmpButton) != NULL) {

				CBofBitmap *pUp, *pDown, *pFocus, *pDis;

				pUp = LoadBitmap(BuildSlotDir(g_stButtons[i].m_pszUp), pPal);
				pDown = LoadBitmap(BuildSlotDir(g_stButtons[i].m_pszDown), pPal);
				pFocus = LoadBitmap(BuildSlotDir(g_stButtons[i].m_pszFocus), pPal);
				pDis = LoadBitmap(BuildSlotDir(g_stButtons[i].m_pszDisabled), pPal);

				m_pSlotButs[i]->LoadBitmaps(pUp, pDown, pFocus, pDis);

#if BOF_MAC
				// jwl 1.17.97 this will be a mac custom window thus not having the
				// black frame drawn around it.
				m_pSlotButs[i]->SetCustomWindow(TRUE);
#endif
				m_pSlotButs[i]->Create(g_stButtons[i].m_pszName, g_stButtons[i].m_nLeft, g_stButtons[i].m_nTop, g_stButtons[i].m_nWidth, g_stButtons[i].m_nHeight, this, g_stButtons[i].m_nID);
#if !BOF_MAC
				m_pSlotButs[i]->Show();
#endif
			}
		}

		if (m_pLoseBmp == NULL) {
			if ((m_pLoseBmp = new CBofBitmap(BuildSlotDir("BGNV.BMP"), pPal)) != NULL) {

			} else {
				ReportError(ERR_MEMORY);
			}
		}

		// Hide the GO, LOSE Button until a bet is made
		m_pSlotButs[GO]->Hide();
		//m_pSlotButs[LOSE]->Hide();

		// Build all our slotbmp
		for (i = 0; i < SLOT_NUM; i++) {

			m_cSlots[i].m_nIdx = g_engine->getRandomNumber() % SLOT_BMP_NUM;
			for (int j = 0; j < SLOT_BMP_NUM; j++) {
				m_cSlots[i].m_pSlotBmp[j] = new CBofBitmap(BuildSlotDir(g_stSlotBmps[i].m_pszName[j]), pPal);
			}
		}

		// Read in their total nuggets from game
		CBagVar *pVar = NULL;

		pVar = VARMNGR->GetVariable("NUGGETS");

		if (pVar)
			m_nCredit = pVar->GetNumValue();

		// Initialize their bet
		m_nBet = 0;

		// Setup the Credit text fields
		if ((m_pCredText = new CBofText) != NULL) {
			CBofRect cRect(CreditRect.left, CreditRect.top, CreditRect.right, CreditRect.bottom);
			m_pCredText->SetupText(&cRect, JUSTIFY_RIGHT, FORMAT_CENTER_RIGHT);
#if BOF_MAC || BOF_WINMAC       // jwl 08.01.96
			m_pCredText->SetColor(RGB(0, 0, 0));
#else
			m_pCredText->SetColor(RGB(255, 255, 255));
#endif
			m_pCredText->SetSize(20);
			m_pCredText->SetWeight(TEXT_BOLD);
			m_pCredText->SetText(BuildString("%d", m_nCredit));
		} else {
			ReportError(ERR_MEMORY);
		}

		// Setup the Bet text fields
		//
		// Make sure that we don't already have one
		Assert(m_pBetText == NULL);

		if ((m_pBetText = new CBofText) != NULL) {
			CBofRect cRect(BetRect.left, BetRect.top, BetRect.right, BetRect.bottom);
			m_pBetText->SetupText(&cRect, JUSTIFY_RIGHT, FORMAT_CENTER_RIGHT);
#if BOF_MAC || BOF_WINMAC       // jwl 08.01.96
			m_pBetText->SetColor(RGB(0, 0, 0));
#else
			m_pBetText->SetColor(RGB(255, 255, 255));
#endif
			m_pBetText->SetSize(20);
			m_pBetText->SetWeight(TEXT_BOLD);
			m_pBetText->SetText(BuildString("%d", m_nBet));
		} else {
			ReportError(ERR_MEMORY);
		}

		// Setup the Odds text fields
		//
		// Make sure that we don't already have one
		Assert(m_pOddsText == NULL);

		if ((m_pOddsText = new CBofText) != NULL) {
			CBofRect cRect(OddRect.left, OddRect.top, OddRect.right, OddRect.bottom);
			m_pOddsText->SetupText(&cRect, JUSTIFY_RIGHT, FORMAT_CENTER_RIGHT);
#if BOF_MAC || BOF_WINMAC       // jwl 08.01.96
			m_pOddsText->SetColor(RGB(0, 0, 0));
#else
			m_pOddsText->SetColor(RGB(255, 255, 255));
#endif
			m_pOddsText->SetSize(32);
			m_pOddsText->SetWeight(TEXT_BOLD);
			m_pOddsText->SetText("");
		} else {
			ReportError(ERR_MEMORY);
		}

		Show();

		// jwl 1.17.97 must show buttons after bringing the window frontmost
#if BOF_MAC
		for (i = 0; i < NUM_SLOTBUTT; i++) {
			if (m_pSlotButs[i] != NULL) {
				m_pSlotButs[i]->Show();
			}
		}
#endif
		InvalidateRect(NULL);
		UpdateWindow();
	}


	// Added for casino background sounds BFW 12/24/96
	if ((m_pBkgSnd = new CBofSound(this, BuildSlotDir(CASINO_AUDIO), SOUND_MIX, 99999)) != NULL) {
		m_pBkgSnd->Play();
	} else {
		ReportError(ERR_MEMORY);
	}

	CBofCursor::Show();

	return (m_errCode);
}


/*****************************************************************************
*
*  Detach -
*
*  DESCRIPTION:
*
*  SAMPLE USAGE:
*
*
*  RETURNS:
*       ERROR_CODE
*
*****************************************************************************/
ERROR_CODE SBarSlotWnd::Detach() {
	CBagVar *pVar;
	INT i, j;

	CBofCursor::Hide();

	// BCW - 12/16/96 10:01 pm
	// Put any credits left in machine back into you credit chip.
	//
	if (m_nBet > 0) {
		m_nCredit += m_nBet;
	}

	// Write out new value of nuggets
	pVar = VARMNGR->GetVariable("NUGGETS");
	if (pVar)
		pVar->SetValue(m_nCredit);

	// Added for casino background sounds BFW 12/24/96
	if (m_pBkgSnd->IsPlaying()) {
		m_pBkgSnd->Stop();
	}
	if (m_pBkgSnd != NULL) {
		delete m_pBkgSnd;
		m_pBkgSnd = NULL;
	}

	if (m_pLoseBmp != NULL) {
		delete m_pLoseBmp;
		m_pLoseBmp = NULL;
	}

	// Destroy all buttons
	for (i = 0; i < NUM_SLOTBUTT; i++) {
		if (m_pSlotButs[i] != NULL) {
			delete m_pSlotButs[i];
			m_pSlotButs[i] = NULL;
		}
	}

	// Destroy all our slotbmp
	for (i = 0; i < SLOT_NUM; i++) {
		m_cSlots[i].m_nIdx = 0;

		for (j = 0; j < SLOT_BMP_NUM; j++) {

			if (m_cSlots[i].m_pSlotBmp[j]) {
				delete m_cSlots[i].m_pSlotBmp[j];
				m_cSlots[i].m_pSlotBmp[j] = NULL;
			}
		}
	}

	if (m_pCredText != NULL) {
		delete m_pCredText;
		m_pCredText = NULL;
	}
	if (m_pBetText != NULL) {
		delete m_pBetText;
		m_pBetText = NULL;
	}

	if (m_bFixBmp != NULL) {
		delete m_bFixBmp;
		m_bFixBmp = NULL;
	}

	if (m_pWinSound != NULL) {
		delete m_pWinSound;
		m_pWinSound = NULL;
	}

	if (m_pSlotSound != NULL) {
		delete m_pSlotSound;
		m_pSlotSound = NULL;
	}
	if (m_pOddsText != NULL) {
		delete m_pOddsText;
		m_pOddsText = NULL;
	}

	CBagStorageDevWnd::Detach();

	// One turn has gone by
	VARMNGR->IncrementTimers();

	return (m_errCode);
}


VOID SBarSlotWnd::AddBet(INT nBetVal) {
	if (m_nCredit < MAX_CREDITS) {
		if (nBetVal <= m_nCredit) {

			m_nBet += nBetVal;
			m_nCredit -= nBetVal;

			// Check and see if we need to show the GO button
			if (m_nBet && !(m_pSlotButs[GO]->IsVisible())) {
				m_pSlotButs[GO]->Show();
			}
			m_nPayOff1 = 0;
			m_bLose = FALSE;
			m_pOddsText->SetText("");

			UpdateText();
		}

	} else {

		CHAR szBuf[256];
		Common::strcpy_s(szBuf, "The slot machine reads your credit chip balance, and makes an almost organic gurgling noise. A faint whiff of burnt components come from the interior of the machine.");
		CBofString cString(szBuf, 256);

		CBofRect cRect(0, 440, 640 - 1, 480 - 1);

		CBofBitmap cBmp(cRect.Width(), cRect.Height(), (CBofPalette *)NULL, FALSE);
		cBmp.CaptureScreen(this, &cRect);

		PaintBeveledText(this, &cRect, cString, FONT_15POINT, TEXT_NORMAL, RGB(255, 255, 255), JUSTIFY_WRAP, FORMAT_TOP_LEFT);
		WaitForInput();

		cBmp.Paint(this, &cRect);
	}
}


VOID SBarSlotWnd::BetAll() {
	if (m_nCredit < MAX_CREDITS) {

		m_nBet += m_nCredit;
		m_nCredit = 0;

		// Check and see if we need to show the GO button
		if (m_nBet && !(m_pSlotButs[GO]->IsVisible()))
			m_pSlotButs[GO]->Show();

		UpdateText();

	} else {

		CHAR szBuf[256];
		Common::strcpy_s(szBuf, "The bonkier apologizes, explaining that in light of your winning streak, the house can no longer accept your bets");
		CBofString cString(szBuf, 256);

		CBofRect cRect(0, 440, 640 - 1, 480 - 1);

		CBofBitmap cBmp(cRect.Width(), cRect.Height(), (CBofPalette *)NULL, FALSE);
		cBmp.CaptureScreen(this, &cRect);

		PaintBeveledText(this, &cRect, cString, FONT_15POINT, TEXT_NORMAL, RGB(255, 255, 255), JUSTIFY_WRAP, FORMAT_TOP_LEFT);
		WaitForInput();

		cBmp.Paint(this, &cRect);
	}
}

VOID SBarSlotWnd::ClrBet() {
	m_nCredit += m_nBet;
	m_nBet = 0;

	m_pSlotButs[GO]->Hide();

	UpdateText();
}

VOID SBarSlotWnd::FixBet() {
	m_bFixBet = !m_bFixBet;

	if (!m_bFixBet) {
		m_bAutoDecrement = FALSE;
	}
}

VOID SBarSlotWnd::Go() {
	INT i;

	// If the player has fixed the bet,now is the time to decrement
	//
	/*if (m_bAutoDecrement) {

	    m_nCredit -= m_nBet;

	     // Update credits
	    UpdateText();
	}*/

	for (i = 0; i < SLOT_NUM; i++) {

		m_cSlots[i].m_nIdx = g_engine->getRandomNumber() % SLOT_BMP_NUM;
		InvalidateRect(&(m_cSlots[i].m_cSlotRect));
	}

	SlideSlots();

	CalcOutcome();

	// Hide the GO button
	m_pSlotButs[GO]->Hide();

	// set it up to show again in 3 seconds
	//SetTimer(SHOWGO, 1000 * 3);

	if (m_bFixBet)
		g_bFix = TRUE;

	UpdateText();

	UpdateWindow();
	g_bFix = FALSE;

	if (m_bFixBet && m_nBet != 0) {
		Sleep(3000);
		m_pSlotButs[GO]->Show();
		m_pSlotButs[GO]->Paint();
	}
}

VOID SBarSlotWnd::CalcOutcome() {
	int i, j;
	int nMatch = 0;
	int nMatchVal = 0;
	int nGeo = 0;
	int nCelest = 0;
	int nLuck = 0;

	// Clear out Payoffs
	m_nPayOff1 = 0;
	m_nPayOff2 = 0;

	// Get number of matching slots
	for (i = 0; i < SLOT_NUM; i++) {
		for (j = i + 1; j < SLOT_NUM; j++) {
			if (m_cSlots[i].m_nIdx == m_cSlots[j].m_nIdx) {
				nMatch++;
				nMatchVal = m_cSlots[i].m_nIdx;
			}
		}
	}
	// see if we are already a winner
	if (nMatch > 3)             // Quad
		QuadPays(nMatchVal);
	else if (nMatch == 3)       // Trio
		TripPays(nMatchVal);
	else if (nMatch == 2)       // Two Pair
		SetPayOff(5, 2);
	else if (nMatch == 1)       // Pair
		PairPays(nMatchVal);
	else {
		// check for special trio
		for (i = 0; i < SLOT_NUM; i++) {
			if (m_cSlots[i].m_nIdx < 3) // Geometric Trio
				nGeo++;
			if (m_cSlots[i].m_nIdx >= 3 && m_cSlots[i].m_nIdx < 6) // Celestrial Trio
				nCelest++;
			if (m_cSlots[i].m_nIdx >= 6) // Lucky Trio
				nLuck++;
		}

		if (nGeo == 3) {
			m_nPayOff1 = 1;
			m_nPayOff2 = 1;
		} else if (nCelest == 3) {
			m_nPayOff1 = 3;
			m_nPayOff2 = 2;
		} else if (nLuck == 3) {
			m_nPayOff1 = 2;
			m_nPayOff2 = 1;
		}
	}

#if 0
	// Calc payoffs of the special trios, geometric, celestial, lucky
	int TrioPay1 = 0;
	int TrioPay2 = 0;

	// Calculate which pays better, the special trio or the other outcome
	if (TrioPay2 && m_nPayOff2) {
		// although this isn't the most percise evaluation, it works fine for this
		if ((TrioPay1 / TrioPay2) > (m_nPayOff1 / m_nPayOff2)) {
			m_nPayOff1 = TrioPay1;
			m_nPayOff2 = TrioPay2;
		}
	}
	// }
#endif

	// Do we have a winner ?
	if (m_nPayOff1 > 0) {
		// Play winning audio

		if ((m_pWinSound = new CBofSound(this, BuildSlotDir(WINAUDIO), SOUND_MIX, 1)) != NULL) {
			m_pWinSound->Play();
		} else {
			ReportError(ERR_MEMORY);
		}

		// Calc new credit
		// m_nCredit += ((m_nBet * m_nPayOff1)/m_nPayOff2 ) + m_nBet;
		m_nCredit += (m_nBet * m_nPayOff1) / m_nPayOff2;
		m_bLose = FALSE;

	} else {
		//m_pSlotButs[LOSE]->Show();
		m_pLoseBmp->Paint(this, 401, 125);
		m_bLose = TRUE;
		m_pOddsText->SetText("");
	}

	// put up better luck next time
	// if bet not fixed, clear bet
	//
	if (m_bFixBet == FALSE) {
		m_nBet = 0;

	} else {

		m_bAutoDecrement = TRUE;

		// If the player has fixed the bet,now is the time to decrement
		//

		if (m_nCredit >= m_nBet) {
			m_nCredit -= m_nBet;
		} else {
			m_nBet = m_nCredit;
			m_nCredit = 0;
		}

		// Update credits
		//UpdateText();
	}
}

VOID SBarSlotWnd::QuadPays(int nSlotIdx) {
	//int nOddIdx;

	switch (nSlotIdx) {

	case 0: //triangle
		SetPayOff(25, 1);
		break;
	case 1: // square
		SetPayOff(50, 1);
		break;
	case 2: // circle
		SetPayOff(75, 1);
		break;
	case 3: // sun
		SetPayOff(100, 1);
		break;
	case 4: // star
		SetPayOff(150, 1);
		break;
	case 5: // moon
		SetPayOff(200, 1);
		break;
	case 6: // eye
		SetPayOff(300, 1);
		break;
	case 7: // atom
		SetPayOff(400, 1);
		break;
	case 8: // shoe
		SetPayOff(500, 1);
		break;
	}
}


VOID SBarSlotWnd::TripPays(int nSlotIdx) {
	//int nOddIdx;

	switch (nSlotIdx) {
	case 0: //triangle
	case 1: // square
	case 2: // circle
		SetPayOff(3, 1);
		break;
	case 3: // sun
	case 4: // star
	case 5: // moon
		SetPayOff(4, 1);
		break;
	case 6: // eye
		SetPayOff(5, 1);
		break;
	case 7: // atom
		SetPayOff(7, 1);
		break;
	case 8: // shoe
		SetPayOff(10, 1);
		break;
	}
}

VOID SBarSlotWnd::PairPays(int nSlotIdx) {
	//int nOddIdx;

	switch (nSlotIdx) {
	case 0: //triangle
	case 1: // square
	case 2: // circle
		SetPayOff(1, 4);
		break;
	case 3: // sun
	case 4: // star
	case 5: // moon
		SetPayOff(1, 3);
		break;
	case 6: // eye
		SetPayOff(1, 2);
		break;
	case 7: // atom
		SetPayOff(1, 1);
		break;
	case 8: // shoe
		SetPayOff(3, 2);
		break;
	}
}


VOID SBarSlotWnd::SetPayOff(INT nPay1, INT nPay2) {
	m_nPayOff1 = nPay1;
	m_nPayOff2 = nPay2;
}


VOID SBarSlotWnd::SlideSlots() {
	Assert(IsValidObject(this));

	if (!ErrorOccurred()) {

		INT i;
		INT nIncrement = 30; // number of pixels to move
		INT nMaskClr = CBagel::GetBagApp()->GetChromaColor();
		CBofBitmap *pCurBmp;
		CBofRect        BmpRect;
		CBofRect        SrcRect;
		CBofRect        DestRect;

		// Erase Previous game
		//
		CBofRect cRect(219, 12, 626, 276);
		if (m_pBackdrop != NULL) {
			m_pBackdrop->Paint(this, &cRect, &cRect);
		}

		// Slot #1
		pCurBmp = m_cSlots[0].m_pSlotBmp[m_cSlots[0].m_nIdx];
		BmpRect = pCurBmp->GetRect();
		SrcRect = BmpRect;
		SrcRect.left = SrcRect.right;
		DestRect = m_cSlots[0].m_cSlotRect;

		for (i = 1; SrcRect.left > BmpRect.left; i++) {
			Sleep(30);

			SrcRect.left = SrcRect.right - (i * nIncrement);

			// Check limits
			if (SrcRect.left < BmpRect.left)
				SrcRect.left = BmpRect.left;

			cRect.SetRect(DestRect.left, DestRect.top, DestRect.left + SrcRect.Width() - 1, DestRect.top + SrcRect.Height() - 1);

			pCurBmp->PaintMaskBackdrop(this, &cRect, &SrcRect, nMaskClr);
		}
		m_pSlotSound->Play();

		// Slot #2
		pCurBmp = m_cSlots[1].m_pSlotBmp[m_cSlots[1].m_nIdx];
		BmpRect = pCurBmp->GetRect();
		SrcRect = BmpRect;
		SrcRect.top = SrcRect.bottom;
		DestRect = m_cSlots[1].m_cSlotRect;

		for (i = 1; SrcRect.top > BmpRect.top; i++) {
			Sleep(30);

			SrcRect.top = SrcRect.bottom - (i * nIncrement);

			// Check limits
			if (SrcRect.top < BmpRect.top)
				SrcRect.top = BmpRect.top;

			cRect.SetRect(DestRect.left, DestRect.top, DestRect.left + SrcRect.Width() - 1, DestRect.top + SrcRect.Height() - 1);
			pCurBmp->PaintMaskBackdrop(this, &cRect, &SrcRect, nMaskClr);
		}

		m_pSlotSound->Play();

		// Slot #3
		pCurBmp = m_cSlots[2].m_pSlotBmp[m_cSlots[2].m_nIdx];
		BmpRect = pCurBmp->GetRect();
		SrcRect = BmpRect;
		SrcRect.right = SrcRect.left;
		DestRect = m_cSlots[2].m_cSlotRect;

		for (i = 1; SrcRect.right < BmpRect.right; i++) {
			Sleep(30);

			SrcRect.right = SrcRect.left + (i * nIncrement);

			// Check limits
			if (SrcRect.right > BmpRect.right)
				SrcRect.right = BmpRect.right;

			cRect.SetRect(DestRect.right - SrcRect.Width() + 1, DestRect.top, DestRect.right, DestRect.top + SrcRect.Height() - 1);
			pCurBmp->PaintMaskBackdrop(this, &cRect, &SrcRect, nMaskClr);
		}
		m_pSlotSound->Play();

		// Slot #4
		pCurBmp = m_cSlots[3].m_pSlotBmp[m_cSlots[3].m_nIdx];
		BmpRect = pCurBmp->GetRect();
		SrcRect = BmpRect;
		SrcRect.bottom = SrcRect.top;
		DestRect = m_cSlots[3].m_cSlotRect;

		for (i = 1; SrcRect.bottom < BmpRect.bottom; i++) {
			Sleep(30);

			SrcRect.bottom = SrcRect.top + (i * nIncrement);

			// Check limits
			if (SrcRect.bottom > BmpRect.bottom)
				SrcRect.bottom = BmpRect.bottom;

			cRect.SetRect(DestRect.left, DestRect.bottom - SrcRect.Height() + 1, DestRect.left + SrcRect.Width() - 1, DestRect.bottom);
			pCurBmp->PaintMaskBackdrop(this, &cRect, &SrcRect, nMaskClr);
		}
		m_pSlotSound->Play();
	}
}


VOID SBarSlotWnd::UpdateText() {
	Assert(IsValidObject(this));

	if (!ErrorOccurred()) {

		if (m_nPayOff1 > 0) {
			if (m_pOddsText != NULL) {
				m_pOddsText->SetText(BuildString("%d:%d", m_nPayOff1, m_nPayOff2));
			}
		}

		if (m_pCredText != NULL) {
			m_pCredText->SetText(BuildString("%d", m_nCredit));
			m_pCredText->Display(this);
		}

		// Update bet
		if (m_pBetText != NULL) {
			m_pBetText->SetText(BuildString("%d", m_nBet));
			m_pBetText->Display(this);
		}

		if (m_bLose) {

			if (m_pLoseBmp != NULL) {
				m_pLoseBmp->Paint(this, 401, 125);
			}

			if (m_pOddsText != NULL) {
				m_pOddsText->SetText("");
			}

		} else {

			// Check and see if we need to show the GO button
			//
			if (m_nBet && !g_bFix) {

				if (m_pSlotButs[GO] != NULL) {
					m_pSlotButs[GO]->Show();
					m_pSlotButs[GO]->Paint();
				}

			} else {
				if (m_pOddsText != NULL) {
					m_pOddsText->Display(this);
				}
			}
		}

		//InvalidateRect(&CreditRect);
		//InvalidateRect(&BetRect);

		//UpdateWindow();
	}
}


VOID SBarSlotWnd::OnTimer(UINT /*nTimerId*/) {
	// If the time has elapsed and there is a bet
	//
	/*if (nTimerId == SHOWGO) {
	    if (!m_bPaused) {

	        SetPayOff(0, 0);
	        //m_pSlotButs[LOSE]->Hide();

	        // Check and see if we need to show the GO button
	        if (m_nBet && !(m_pSlotButs[GO]->IsVisible()))
	            m_pSlotButs[GO]->Show();

	        KillTimer(nTimerId);
	    }
	}*/
}


VOID SBarSlotWnd::OnLButtonDown(UINT /*nFlags*/, CBofPoint *pPoint, void *) {
	if (FixRect.PtInRect(*pPoint)) {
		FixBet();
		InvalidateRect(&FixRect);
	}
}

VOID SBarSlotWnd::OnBofButton(CBofObject *pObject, INT nState) {
	Assert(IsValidObject(this));
	Assert(pObject != NULL);

	CBofButton *pButton;

	pButton = (CBofButton *)pObject;

	if (nState == BUTTON_CLICKED) {

		switch (pButton->GetControlID()) {
		case ONE:
			AddBet(1);
			break;
		case FOUR:
			AddBet(4);
			break;
		case SEVEN:
			AddBet(7);
			break;
		case TEN:
			AddBet(10);
			break;
		case FORTY:
			AddBet(40);
			break;
		case SEVENTY:
			AddBet(70);
			break;
		case OHNDRD:
			AddBet(100);
			break;
		case RHNDRD:
			AddBet(400);
			break;
		case SHNDRD:
			AddBet(700);
			break;
		case OTHSND:
			AddBet(1000);
			break;
		case RTHSND:
			AddBet(4000);
			break;
		case STHSND:
			AddBet(7000);
			break;
		case CLRBET:
			ClrBet();
			break;
		case BETALL:
			BetAll();
			break;
		case GO:
			Go();
			break;
		case SLOTQUIT:
			LogInfo("\tClicked Quit");
			Close();
			break;

		case SLOTHELP:
			LogInfo("\tClicked Help");

			CBagel *pApp;
			CBagMasterWin *pWin;

			if ((pApp = CBagel::GetBagApp()) != NULL) {
				if ((pWin = pApp->GetMasterWnd()) != NULL) {
					m_bPaused = TRUE;
					pWin->OnHelp(BuildSlotDir("SLOT.TXT"));
					m_bPaused = FALSE;
				}
			}
			break;
		default:
			LogWarning(BuildString("Clicked Unknown Button with ID %d", pButton->GetControlID()));
			break;
		}
	}
}


const CHAR *BuildSlotDir(const CHAR *pszFile) {
	Assert(pszFile != NULL);

	static CHAR szBuf[MAX_DIRPATH];

	// Where Slot assets
	Common::sprintf_s(szBuf, "%s%s%s", BGCBDIR, PATH_DELIMETER, pszFile);

	CBofString sSlotDir(szBuf, MAX_DIRPATH);
	MACROREPLACE(sSlotDir);

	return (&szBuf[0]);
}

} // namespace SpaceBar
} // namespace Bagel
