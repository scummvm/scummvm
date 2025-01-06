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
#include "bagel/boflib/log.h"

namespace Bagel {
namespace SpaceBar {

#define FIXBMP      "BGNDDN.BMP"
#define MAX_CREDITS     200000
// return $1; for casino background sounds
#define CASINO_AUDIO            "CASINO.WAV"

#define BGCB_DIR         "$SBARDIR\\BAR\\CLOSEUP\\BGCB\\"
#define WIN_AUDIO        "BGNWIN.WAV"
#define SLOT_AUDIO       "BGNSLOT.WAV"

struct ST_BUTTONS {
	const char *_pszName;
	const char *_pszUp;
	const char *_pszDown;
	const char *_pszFocus;
	const char *_pszDisabled;
	int _nLeft;
	int _nTop;
	int _nWidth;
	int _nHeight;
	int _nID;
};

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

struct ST_SLOTBMPS {
	const char *_pszName[SLOT_BMP_NUM];
};

static ST_SLOTBMPS g_stSlotBmps[SLOT_NUM] = {
	{{"bgnst1.bmp", "bgnsq1.bmp", "bgnsc1.bmp", "bgnsn1.bmp", "bgnss1.bmp", "bgnsm1.bmp", "bgnse1.bmp", "bgnsa1.bmp", "bgnsh1.bmp" }},
	{{"bgnst3.bmp", "bgnsq3.bmp", "bgnsc3.bmp", "bgnsn3.bmp", "bgnss3.bmp", "bgnsm3.bmp", "bgnse3.bmp", "bgnsa3.bmp", "bgnsh3.bmp" }},
	{{"bgnst4.bmp", "bgnsq4.bmp", "bgnsc4.bmp", "bgnsn4.bmp", "bgnss4.bmp", "bgnsm4.bmp", "bgnse4.bmp", "bgnsa4.bmp", "bgnsh4.bmp" }},
	{{"bgnst2.bmp", "bgnsq2.bmp", "bgnsc2.bmp", "bgnsn2.bmp", "bgnss2.bmp", "bgnsm2.bmp", "bgnse2.bmp", "bgnsa2.bmp", "bgnsh2.bmp" }}
};

// Local functions
const char *BuildSlotDir(const char *pszFile);

static bool g_bFix = false;

SBarSlotWnd::SBarSlotWnd() : CBagStorageDevWnd() {
	for (int i = 0; i < NUM_SLOTBUTT; i++) {
		_pSlotButs[i] = nullptr;
	}

	// Init all our slot bmp
	for (int i = 0; i < SLOT_NUM; i++) {
		_cSlots[i]._nIdx = g_engine->getRandomNumber() % SLOT_BMP_NUM;
		for (int j = 0; j < SLOT_BMP_NUM; j++) {
			_cSlots[i]._pSlotBmp[j] = nullptr;
		}
	}

	_cSlots[0]._cSlotRect = Slot0Rect;
	_cSlots[1]._cSlotRect = Slot1Rect;
	_cSlots[2]._cSlotRect = Slot2Rect;
	_cSlots[3]._cSlotRect = Slot3Rect;

	_bFixBet = false;
	_nBet = 0;
	_nCredit = 0;
	_pCredText = nullptr;
	_pBetText = nullptr;
	_pOddsText = nullptr;
	_bAutoDecrement = false;

	_nPayOff1 = 0;
	_nPayOff2 = 0;

	_bFixBmp = nullptr;
	_pWinSound = nullptr;
	_pSlotSound = nullptr;
	_pLoseBmp = nullptr;
	_pBkgSnd = nullptr;
	_bLose = false;
	
	CBagStorageDevWnd::setHelpFilename(BuildSlotDir("SLOT.TXT"));

	// Call this thing a closeup so that time won't go
	// by when entering the closeup
	setCloseup(true);
}


void SBarSlotWnd::onPaint(CBofRect *pRect) {
	assert(isValidObject(this));

	if (!errorOccurred()) {
		CBofBitmap *pBackBmp = getBackdrop();

		//Paint the storage device
		if (pBackBmp != nullptr) {
			assert(getWorkBmp() != nullptr);

			// Erase everything from the background
			getWorkBmp()->paint(pBackBmp, pRect, pRect);

			// Paint all the objects to the background
			paintStorageDevice(nullptr, pBackBmp, pRect);
		}

		// Paint all our slot bmp
		for (int i = 0; i < SLOT_NUM; i++) {

			if (_cSlots[i]._pSlotBmp[_cSlots[i]._nIdx] != nullptr) {
				_cSlots[i]._pSlotBmp[_cSlots[i]._nIdx]->paint(pBackBmp, &_cSlots[i]._cSlotRect, nullptr, CBagel::getBagApp()->getChromaColor());
			}
		}

		if (_bFixBet && _bFixBmp != nullptr) {
			_bFixBmp->paint(pBackBmp, FixRect.left, FixRect.top);
		}

		// Paint the backdrop
		if (getBackdrop())
			paintBackdrop();

		updateText();
	}
}

void SBarSlotWnd::onMainLoop() {
	// Do nothing
}

ErrorCode  SBarSlotWnd::attach() {
	_bLose = false;
	_bFixBet = false;

	_pSlotSound = new CBofSound(this, BuildSlotDir(SLOT_AUDIO), SOUND_MIX, 1);

	if (CBagStorageDevWnd::attach() == ERR_NONE) {
		// Must have a valid backdrop by now
		assert(_pBackdrop != nullptr);
		CBofPalette *pPal = _pBackdrop->getPalette();

		_bFixBmp = new CBofBitmap(BuildSlotDir(FIXBMP));

		// Build all our buttons
		for (int i = 0; i < NUM_SLOTBUTT; i++) {

			_pSlotButs[i] = new CBofBmpButton;
			CBofBitmap *pUp = loadBitmap(BuildSlotDir(g_stButtons[i]._pszUp), pPal);
			CBofBitmap *pDown = loadBitmap(BuildSlotDir(g_stButtons[i]._pszDown), pPal);
			CBofBitmap *pFocus = loadBitmap(BuildSlotDir(g_stButtons[i]._pszFocus), pPal);
			CBofBitmap *pDis = loadBitmap(BuildSlotDir(g_stButtons[i]._pszDisabled), pPal);

			_pSlotButs[i]->loadBitmaps(pUp, pDown, pFocus, pDis);
			_pSlotButs[i]->create(g_stButtons[i]._pszName, g_stButtons[i]._nLeft, g_stButtons[i]._nTop, g_stButtons[i]._nWidth, g_stButtons[i]._nHeight, this, g_stButtons[i]._nID);
			_pSlotButs[i]->show();
		}

		if (_pLoseBmp == nullptr) {
			_pLoseBmp = new CBofBitmap(BuildSlotDir("BGNV.BMP"), pPal);
		}

		// Hide the GO, LOSE Button until a bet is made
		_pSlotButs[GO]->hide();

		// Build all our slot bmp
		for (int i = 0; i < SLOT_NUM; i++) {
			_cSlots[i]._nIdx = g_engine->getRandomNumber() % SLOT_BMP_NUM;
			for (int j = 0; j < SLOT_BMP_NUM; j++) {
				_cSlots[i]._pSlotBmp[j] = new CBofBitmap(BuildSlotDir(g_stSlotBmps[i]._pszName[j]), pPal);
			}
		}

		// Read in their total nuggets from game
		CBagVar *pVar = g_VarManager->getVariable("NUGGETS");

		if (pVar)
			_nCredit = pVar->getNumValue();

		// Initialize their bet
		_nBet = 0;

		// Setup the Credit text fields
		_pCredText = new CBofText;

		CBofRect cRect(CreditRect.left, CreditRect.top, CreditRect.right, CreditRect.bottom);
		_pCredText->setupText(&cRect, JUSTIFY_RIGHT, FORMAT_CENTER_RIGHT);
		_pCredText->setColor(CTEXT_WHITE);
		_pCredText->SetSize(20);
		_pCredText->setWeight(TEXT_BOLD);
		_pCredText->setText(buildString("%d", _nCredit));

		// Setup the Bet text fields
		//
		// Make sure that we don't already have one
		assert(_pBetText == nullptr);

		_pBetText = new CBofText;

		cRect.setRect(BetRect.left, BetRect.top, BetRect.right, BetRect.bottom);
		_pBetText->setupText(&cRect, JUSTIFY_RIGHT, FORMAT_CENTER_RIGHT);
		_pBetText->setColor(CTEXT_WHITE);
		_pBetText->SetSize(20);
		_pBetText->setWeight(TEXT_BOLD);
		_pBetText->setText(buildString("%d", _nBet));

		// Setup the Odds text fields
		//
		// Make sure that we don't already have one
		assert(_pOddsText == nullptr);

		_pOddsText = new CBofText;

		cRect.setRect(OddRect.left, OddRect.top, OddRect.right, OddRect.bottom);
		_pOddsText->setupText(&cRect, JUSTIFY_RIGHT, FORMAT_CENTER_RIGHT);
		_pOddsText->setColor(CTEXT_WHITE);
		_pOddsText->SetSize(32);
		_pOddsText->setWeight(TEXT_BOLD);
		_pOddsText->setText("");

		show();
		invalidateRect(nullptr);
		updateWindow();
	}

	_pBkgSnd = new CBofSound(this, BuildSlotDir(CASINO_AUDIO), SOUND_MIX, 99999);
	_pBkgSnd->play();

	CBagCursor::showSystemCursor();

	return _errCode;
}

ErrorCode SBarSlotWnd::detach() {
	CBagCursor::hideSystemCursor();

	// Put any credits left in machine back into you credit chip.
	if (_nBet > 0) {
		_nCredit += _nBet;
	}

	// Write out new value of nuggets
	CBagVar *pVar = g_VarManager->getVariable("NUGGETS");
	if (pVar)
		pVar->setValue(_nCredit);

	if (_pBkgSnd != nullptr && _pBkgSnd->isPlaying())
		_pBkgSnd->stop();

	delete _pBkgSnd;
	_pBkgSnd = nullptr;

	delete _pLoseBmp;
	_pLoseBmp = nullptr;

	// Destroy all buttons
	for (int i = 0; i < NUM_SLOTBUTT; i++) {
		delete _pSlotButs[i];
		_pSlotButs[i] = nullptr;
	}

	// Destroy all our slot bmp
	for (int i = 0; i < SLOT_NUM; i++) {
		_cSlots[i]._nIdx = 0;

		for (int j = 0; j < SLOT_BMP_NUM; j++) {
			delete _cSlots[i]._pSlotBmp[j];
			_cSlots[i]._pSlotBmp[j] = nullptr;
		}
	}

	delete _pCredText;
	_pCredText = nullptr;

	delete _pBetText;
	_pBetText = nullptr;

	delete _bFixBmp;
	_bFixBmp = nullptr;

	delete _pWinSound;
	_pWinSound = nullptr;

	delete _pSlotSound;
	_pSlotSound = nullptr;

	delete _pOddsText;
	_pOddsText = nullptr;

	CBagStorageDevWnd::detach();

	// One turn has gone by
	g_VarManager->incrementTimers();

	return _errCode;
}

void SBarSlotWnd::addBet(int nBetVal) {
	if (_nCredit < MAX_CREDITS) {
		if (nBetVal <= _nCredit) {

			_nBet += nBetVal;
			_nCredit -= nBetVal;

			// Check and see if we need to show the GO button
			if (_nBet && !(_pSlotButs[GO]->isVisible())) {
				_pSlotButs[GO]->show();
			}
			_nPayOff1 = 0;
			_bLose = false;
			_pOddsText->setText("");

			updateText();
		}
	} else {
		char szBuf[256];
		Common::strcpy_s(szBuf, "The slot machine reads your credit chip balance, and makes an almost organic gurgling noise. A faint whiff of burnt components come from the interior of the machine.");
		CBofString cString(szBuf, 256);

		CBofRect cRect(0, 440, 640 - 1, 480 - 1);

		CBofBitmap cBmp(cRect.width(), cRect.height(), (CBofPalette *)nullptr, false);
		cBmp.captureScreen(this, &cRect);

		paintBeveledText(&cRect, cString, FONT_15POINT, TEXT_NORMAL, CTEXT_WHITE, JUSTIFY_WRAP, FORMAT_TOP_LEFT);
		waitForInput();

		cBmp.paint(this, &cRect);
	}
}

void SBarSlotWnd::betAll() {
	if (_nCredit < MAX_CREDITS) {

		_nBet += _nCredit;
		_nCredit = 0;

		// Check and see if we need to show the GO button
		if (_nBet && !_pSlotButs[GO]->isVisible())
			_pSlotButs[GO]->show();

		updateText();

	} else {

		char szBuf[256];
		Common::strcpy_s(szBuf, "The bonkier apologizes, explaining that in light of your winning streak, the house can no longer accept your bets");
		CBofString cString(szBuf, 256);

		CBofRect cRect(0, 440, 640 - 1, 480 - 1);

		CBofBitmap cBmp(cRect.width(), cRect.height(), (CBofPalette *)nullptr, false);
		cBmp.captureScreen(this, &cRect);

		paintBeveledText(&cRect, cString, FONT_15POINT, TEXT_NORMAL, CTEXT_WHITE, JUSTIFY_WRAP, FORMAT_TOP_LEFT);
		waitForInput();

		cBmp.paint(this, &cRect);
	}
}

void SBarSlotWnd::clrBet() {
	_nCredit += _nBet;
	_nBet = 0;

	_pSlotButs[GO]->hide();

	updateText();
}

void SBarSlotWnd::fixBet() {
	_bFixBet = !_bFixBet;

	if (!_bFixBet) {
		_bAutoDecrement = false;
	}
}

void SBarSlotWnd::go() {
	for (int i = 0; i < SLOT_NUM; i++) {
		_cSlots[i]._nIdx = g_engine->getRandomNumber() % SLOT_BMP_NUM;
		invalidateRect(&(_cSlots[i]._cSlotRect));
	}

	slideSlots();
	calcOutcome();

	// Hide the GO button
	_pSlotButs[GO]->hide();

	if (_bFixBet)
		g_bFix = true;

	updateText();
	updateWindow();

	g_bFix = false;

	if (_bFixBet && _nBet != 0) {
		bofSleep(3000);
		_pSlotButs[GO]->show();
		_pSlotButs[GO]->paint();
	}
}

void SBarSlotWnd::calcOutcome() {
	int nMatch = 0;
	int nMatchVal = 0;
	int nGeo = 0;
	int nCelest = 0;
	int nLuck = 0;

	// Clear out Payoffs
	_nPayOff1 = 0;
	_nPayOff2 = 0;

	// Get number of matching slots
	for (int i = 0; i < SLOT_NUM; i++) {
		for (int j = i + 1; j < SLOT_NUM; j++) {
			if (_cSlots[i]._nIdx == _cSlots[j]._nIdx) {
				nMatch++;
				nMatchVal = _cSlots[i]._nIdx;
			}
		}
	}

	// See if we are already a winner
	if (nMatch > 3)             // Quad
		quadPays(nMatchVal);
	else if (nMatch == 3)       // Trio
		tripPays(nMatchVal);
	else if (nMatch == 2)       // Two Pair
		setPayOff(5, 2);
	else if (nMatch == 1)       // Pair
		pairPays(nMatchVal);
	else {
		// Check for special trio
		for (int i = 0; i < SLOT_NUM; i++) {
			if (_cSlots[i]._nIdx < 3) // Geometric Trio
				nGeo++;
			if (_cSlots[i]._nIdx >= 3 && _cSlots[i]._nIdx < 6) // Celestial Trio
				nCelest++;
			if (_cSlots[i]._nIdx >= 6) // Lucky Trio
				nLuck++;
		}

		if (nGeo == 3) {
			_nPayOff1 = 1;
			_nPayOff2 = 1;
		} else if (nCelest == 3) {
			_nPayOff1 = 3;
			_nPayOff2 = 2;
		} else if (nLuck == 3) {
			_nPayOff1 = 2;
			_nPayOff2 = 1;
		}
	}

	// Do we have a winner ?
	if (_nPayOff1 > 0) {
		// Play winning audio
		_pWinSound = new CBofSound(this, BuildSlotDir(WIN_AUDIO), SOUND_MIX, 1);
		_pWinSound->play();

		// Calc new credit
		_nCredit += (_nBet * _nPayOff1) / _nPayOff2;
		_bLose = false;

	} else {
		_pLoseBmp->paint(this, 401, 125);
		_bLose = true;
		_pOddsText->setText("");
	}

	// Put up better luck next time if bet not fixed, clear bet
	if (_bFixBet == false) {
		_nBet = 0;

	} else {
		_bAutoDecrement = true;

		// If the player has fixed the bet,now is the time to decrement
		if (_nCredit >= _nBet) {
			_nCredit -= _nBet;
		} else {
			_nBet = _nCredit;
			_nCredit = 0;
		}
	}
}

void SBarSlotWnd::quadPays(int nSlotIdx) {
	switch (nSlotIdx) {
	case 0: // triangle
		setPayOff(25, 1);
		break;
	case 1: // square
		setPayOff(50, 1);
		break;
	case 2: // circle
		setPayOff(75, 1);
		break;
	case 3: // sun
		setPayOff(100, 1);
		break;
	case 4: // star
		setPayOff(150, 1);
		break;
	case 5: // moon
		setPayOff(200, 1);
		break;
	case 6: // eye
		setPayOff(300, 1);
		break;
	case 7: // atom
		setPayOff(400, 1);
		break;
	case 8: // shoe
		setPayOff(500, 1);
		break;
	}
}

void SBarSlotWnd::tripPays(int nSlotIdx) {
	switch (nSlotIdx) {
	case 0: // triangle
	case 1: // square
	case 2: // circle
		setPayOff(3, 1);
		break;
	case 3: // sun
	case 4: // star
	case 5: // moon
		setPayOff(4, 1);
		break;
	case 6: // eye
		setPayOff(5, 1);
		break;
	case 7: // atom
		setPayOff(7, 1);
		break;
	case 8: // shoe
		setPayOff(10, 1);
		break;
	}
}

void SBarSlotWnd::pairPays(int nSlotIdx) {
	switch (nSlotIdx) {
	case 0: // triangle
	case 1: // square
	case 2: // circle
		setPayOff(1, 4);
		break;
	case 3: // sun
	case 4: // star
	case 5: // moon
		setPayOff(1, 3);
		break;
	case 6: // eye
		setPayOff(1, 2);
		break;
	case 7: // atom
		setPayOff(1, 1);
		break;
	case 8: // shoe
		setPayOff(3, 2);
		break;
	}
}

void SBarSlotWnd::setPayOff(int nPay1, int nPay2) {
	_nPayOff1 = nPay1;
	_nPayOff2 = nPay2;
}

void SBarSlotWnd::slideSlots() {
	assert(isValidObject(this));

	if (!errorOccurred()) {
		int nIncrement = 30;	// Number of pixels to move
		int nMaskClr = CBagel::getBagApp()->getChromaColor();

		// Erase Previous game
		CBofRect cRect(219, 12, 626, 276);
		if (_pBackdrop != nullptr) {
			_pBackdrop->paint(this, &cRect, &cRect);
		}

		// Slot #1
		CBofBitmap *pCurBmp = _cSlots[0]._pSlotBmp[_cSlots[0]._nIdx];
		CBofRect BmpRect = pCurBmp->getRect();
		CBofRect SrcRect = BmpRect;
		SrcRect.left = SrcRect.right;
		CBofRect DestRect = _cSlots[0]._cSlotRect;

		for (int i = 1; SrcRect.left > BmpRect.left; i++) {
			bofSleep(30);

			SrcRect.left = SrcRect.right - (i * nIncrement);

			// Check limits
			if (SrcRect.left < BmpRect.left)
				SrcRect.left = BmpRect.left;

			cRect.setRect(DestRect.left, DestRect.top, DestRect.left + SrcRect.width() - 1, DestRect.top + SrcRect.height() - 1);

			pCurBmp->paintMaskBackdrop(this, &cRect, &SrcRect, nMaskClr);
		}
		_pSlotSound->play();

		// Slot #2
		pCurBmp = _cSlots[1]._pSlotBmp[_cSlots[1]._nIdx];
		BmpRect = pCurBmp->getRect();
		SrcRect = BmpRect;
		SrcRect.top = SrcRect.bottom;
		DestRect = _cSlots[1]._cSlotRect;

		for (int i = 1; SrcRect.top > BmpRect.top; i++) {
			bofSleep(30);

			SrcRect.top = SrcRect.bottom - (i * nIncrement);

			// Check limits
			if (SrcRect.top < BmpRect.top)
				SrcRect.top = BmpRect.top;

			cRect.setRect(DestRect.left, DestRect.top, DestRect.left + SrcRect.width() - 1, DestRect.top + SrcRect.height() - 1);
			pCurBmp->paintMaskBackdrop(this, &cRect, &SrcRect, nMaskClr);
		}

		_pSlotSound->play();

		// Slot #3
		pCurBmp = _cSlots[2]._pSlotBmp[_cSlots[2]._nIdx];
		BmpRect = pCurBmp->getRect();
		SrcRect = BmpRect;
		SrcRect.right = SrcRect.left;
		DestRect = _cSlots[2]._cSlotRect;

		for (int i = 1; SrcRect.right < BmpRect.right; i++) {
			bofSleep(30);

			SrcRect.right = SrcRect.left + (i * nIncrement);

			// Check limits
			if (SrcRect.right > BmpRect.right)
				SrcRect.right = BmpRect.right;

			cRect.setRect(DestRect.right - SrcRect.width() + 1, DestRect.top, DestRect.right, DestRect.top + SrcRect.height() - 1);
			pCurBmp->paintMaskBackdrop(this, &cRect, &SrcRect, nMaskClr);
		}
		_pSlotSound->play();

		// Slot #4
		pCurBmp = _cSlots[3]._pSlotBmp[_cSlots[3]._nIdx];
		BmpRect = pCurBmp->getRect();
		SrcRect = BmpRect;
		SrcRect.bottom = SrcRect.top;
		DestRect = _cSlots[3]._cSlotRect;

		for (int i = 1; SrcRect.bottom < BmpRect.bottom; i++) {
			bofSleep(30);

			SrcRect.bottom = SrcRect.top + (i * nIncrement);

			// Check limits
			if (SrcRect.bottom > BmpRect.bottom)
				SrcRect.bottom = BmpRect.bottom;

			cRect.setRect(DestRect.left, DestRect.bottom - SrcRect.height() + 1, DestRect.left + SrcRect.width() - 1, DestRect.bottom);
			pCurBmp->paintMaskBackdrop(this, &cRect, &SrcRect, nMaskClr);
		}

		_pSlotSound->play();
	}
}

void SBarSlotWnd::updateText() {
	assert(isValidObject(this));
	if (errorOccurred())
		return;

	if (_nPayOff1 > 0 && _pOddsText != nullptr) {
		_pOddsText->setText(buildString("%d:%d", _nPayOff1, _nPayOff2));
	}

	if (_pCredText != nullptr) {
		_pCredText->setText(buildString("%d", _nCredit));
		_pCredText->display(this);
	}

	// Update bet
	if (_pBetText != nullptr) {
		_pBetText->setText(buildString("%d", _nBet));
		_pBetText->display(this);
	}

	if (_bLose) {

		if (_pLoseBmp != nullptr) {
			_pLoseBmp->paint(this, 401, 125);
		}

		if (_pOddsText != nullptr) {
			_pOddsText->setText("");
		}

	} else if (_nBet && !g_bFix) {
		// Check and see if we need to show the GO button
		if (_pSlotButs[GO] != nullptr) {
			_pSlotButs[GO]->show();
			_pSlotButs[GO]->paint();
		}
	} else if (_pOddsText != nullptr) {
		_pOddsText->display(this);
	}
}

void SBarSlotWnd::onTimer(uint32 /*nTimerId*/) {
}

void SBarSlotWnd::onLButtonDown(uint32 /*nFlags*/, CBofPoint *pPoint, void *) {
	if (FixRect.ptInRect(*pPoint)) {
		fixBet();
		invalidateRect(&FixRect);
	}
}

void SBarSlotWnd::onBofButton(CBofObject *pObject, int nState) {
	assert(isValidObject(this));
	assert(pObject != nullptr);

	if (nState != BUTTON_CLICKED)
		return;

	CBofButton *pButton = (CBofButton *)pObject;

	switch (pButton->getControlID()) {
	case ONE:
		addBet(1);
		break;
	case FOUR:
		addBet(4);
		break;
	case SEVEN:
		addBet(7);
		break;
	case TEN:
		addBet(10);
		break;
	case FORTY:
		addBet(40);
		break;
	case SEVENTY:
		addBet(70);
		break;
	case OHNDRD:
		addBet(100);
		break;
	case RHNDRD:
		addBet(400);
		break;
	case SHNDRD:
		addBet(700);
		break;
	case OTHSND:
		addBet(1000);
		break;
	case RTHSND:
		addBet(4000);
		break;
	case STHSND:
		addBet(7000);
		break;
	case CLRBET:
		clrBet();
		break;
	case BETALL:
		betAll();
		break;
	case GO:
		go();
		break;
	case SLOTQUIT:
		logInfo("\tClicked Quit");
		close();
		break;

	case SLOTHELP: {
		logInfo("\tClicked Help");

		CBagel *pApp = CBagel::getBagApp();
		if (pApp != nullptr) {
			CBagMasterWin *pWin = pApp->getMasterWnd();
			if (pWin != nullptr)
				pWin->onHelp(BuildSlotDir("SLOT.TXT"));
		}
		}
		break;

	default:
		logWarning(buildString("Clicked Unknown Button with ID %d", pButton->getControlID()));
		break;
	}
}


const char *BuildSlotDir(const char *pszFile) {
	assert(pszFile != nullptr);

	static char szBuf[MAX_DIRPATH];

	// Where Slot assets
	Common::sprintf_s(szBuf, "%s%s", BGCB_DIR, pszFile);

	CBofString sSlotDir(szBuf, MAX_DIRPATH);
	fixPathName(sSlotDir);

	return &szBuf[0];
}

} // namespace SpaceBar
} // namespace Bagel
