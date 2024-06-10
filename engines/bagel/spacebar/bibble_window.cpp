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
#include "bagel/spacebar/spacebar.h"
#include "bagel/baglib/master_win.h"
#include "bagel/boflib/gui/button.h"
#include "bagel/boflib/sound.h"
#include "bagel/boflib/file_functions.h"
#include "bagel/boflib/log.h"
#include "bagel/boflib/std_keys.h"

namespace Bagel {
namespace SpaceBar {

#define CREDITS_AREA_X1 236
#define CREDITS_AREA_Y1 255
#define CREDITS_AREA_X2 393
#define CREDITS_AREA_Y2 270

// Max number of credits the users is allowed to bet with
#define MAX_AMOUNT  50000

#define BIBBLE_BMP      "BIBSTRIP.BMP"
#define BIBBLE_CELS     16
#define BALL_BMP        "BALL2.BMP"
#define BALL_CELS       5
#define ARCH1_BMP       "ARCH1.BMP"
#define ARCH2_BMP       "ARCH2.BMP"
#define ARCH3_BMP       "ARCH3.BMP"
#define CASINO_AUDIO            "CASINO.WAV"

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

const ST_PAYOFFS PAY_OFFS[BIBBLE_NUM_PAYOFFS] = {
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

CBetAreaDef BET_AREAS[BIBBLE_NUM_BET_AREAS] = {
	{ 0, 66, 304, 66 + 49, 334, 3, 2, "EVBWIN02.WAV", "EVBWIN27.WAV" },
	{ 0, 66, 336, 66 + 49, 366, 2, 1, "EVBWIN03.WAV", "EVBWIN28.WAV" },
	{ 0, 66, 368, 66 + 49, 398, 10, 1, "EVBWIN04.WAV", "EVBWIN29.WAV" },
	{ 0, 66, 401, 66 + 49, 431, 100, 1, "EVBWIN5.WAV", "EVBWIN30.WAV" },

	{ 0, 118, 304, 118 + 47, 334, 7, 1, "EVBWIN06.WAV", "EVBWIN31.WAV" },
	{ 0, 118, 336, 118 + 47, 366, 5, 2, "EVBWIN07.WAV", "EVBWIN32.WAV" },
	{ 0, 118, 368, 118 + 47, 398, 5, 2, "EVBWIN08.WAV", "EVBWIN32.WAV" },
	{ 0, 118, 401, 118 + 47, 431, 7, 1, "EVBWIN09.WAV", "EVBWIN31.WAV" },

	{ 0, 168, 304, 168 + 48, 334, 5, 2, "EVBWIN10.WAV", "EVBWIN32.WAV" },
	{ 0, 168, 336, 168 + 48, 366, 2, 1, "EVBWIN11.WAV", "EVBWIN28.WAV" },
	{ 0, 168, 368, 168 + 48, 398, 5, 1, "EVBWIN12.WAV", "EVBWIN33.WAV" },
	{ 0, 168, 401, 168 + 48, 431, 35, 1, "EVBWIN13.WAV", "EVBWIN34.WAV" },

	{ 0, 471, 306, 471 + 49, 336, 5, 4, "EVBWIN14.WAV", "EVBWIN35.WAV" },
	{ 0, 471, 338, 471 + 49, 368, 5, 4, "EVBWIN17.WAV", "EVBWIN35.WAV" },
	{ 0, 471, 370, 471 + 49, 400, 3, 2, "EVBWIN20.WAV", "EVBWIN27.WAV" },
	{ 0, 471, 403, 471 + 49, 433, 6, 1, "EVBWIN23.WAV", "EVBWIN40.WAV" },

	{ 0, 523, 306, 523 + 47, 336, 3, 1, "EVBWIN15.WAV", "EVBWIN36.WAV" },
	{ 0, 523, 338, 523 + 47, 368, 3, 1, "EVBWIN18.WAV", "EVBWIN36.WAV" },
	{ 0, 523, 370, 523 + 47, 400, 8, 1, "EVBWIN21.WAV", "EVBWIN38.WAV" },
	{ 0, 523, 403, 523 + 47, 433, 125, 1, "EVBWIN24.WAV", "EVBWIN41.WAV" },

	{ 0, 574, 306, 574 + 47, 336, 19, 1, "EVBWIN16.WAV", "EVBWIN37.WAV" },
	{ 0, 574, 338, 574 + 47, 368, 19, 1, "EVBWIN19.WAV", "EVBWIN37.WAV" },
	{ 0, 574, 370, 574 + 47, 400, 90, 1, "EVBWIN22.WAV", "EVBWIN39.WAV" },
	{ 0, 574, 403, 574 + 47, 433, 7500, 1, "EVBWIN25.WAV", "EVBWIN42.WAV" }
};

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
const char *BuildDir(const char *pszFile);

static bool g_bBibbleHack = false;


CBetArea::CBetArea(const CBetAreaDef &def) :
	_cRect(def._left, def._top, def._right, def._bottom),
	_nBet(def._nBet), _nPayOff1(def._nPayOff1), _nPayOff2(def._nPayOff2),
	_cAudioFile(def._cAudioFile), _cPayFile(def._cPayFile) {
}


CBibbleWindow::CBibbleWindow() {
	logInfo("Constructing BibbleBonk...");

	// Inits
	_pCreditsText = nullptr;
	_nNumCredits = 0;
	_nBall1 = 0;
	_nBall2 = 0;
	_nBall3 = 0;
	_nBall1Said = 0;
	_nBall2Said = 0;
	_nBall3Said = 0;
	_nNumShout1 = 0;
	_nNumShout2 = 0;
	_nNumShout3 = 0;
	_nNumShout4 = 0;
	_nNumTopBonks = 0;
	_nNumMidBonks = 0;
	_nNumBotBonks = 0;
	_pSelected = nullptr;
	_pBall = nullptr;
	_pBibble[0] = _pBibble[1] = _pBibble[2] = nullptr;
	_pShouts[0] = _pShouts[1] = _pShouts[2] = _pShouts[3] = nullptr;
	_pArch1 = _pArch2 = _pArch3 = nullptr;
	_pBkgSnd = nullptr;
	_pMasterBibble = nullptr;

	for (int i = 0; i < BIBBLE_NUM_BUTTONS; i++)
		_pButtons[i] = nullptr;

	CBagStorageDevWnd::setHelpFilename(BuildDir("BIBBLE.TXT"));

	// Call this thing a closeup so that time won't go  by when entering the closeup
	setCloseup(true);
}

ErrorCode CBibbleWindow::attach() {
	assert(isValidObject(this));

	const int nBibbleYPos[BIBBLE_NUM_BIBBLES] = {
		BIBBLE_START_Y1,
		BIBBLE_START_Y2,
		BIBBLE_START_Y3
	};
	const int nBibbleXPos[BIBBLE_NUM_BIBBLES] = {
		BIBBLE_START_X1,
		BIBBLE_START_X2,
		BIBBLE_START_X3
	};

	const char *pszShouts[BIBBLE_NUM_SHOUTS] = {
		SHOUT_1,
		SHOUT_2,
		SHOUT_3,
		SHOUT_4
	};

	logInfo("\tCBibbleWindow::attach()");

	CBagStorageDevWnd::attach();

	g_waitOKFl = false;

	CBagVar *pVar = g_VarManager->getVariable("NUGGETS");

	if (pVar != nullptr) {
		_nNumCredits = pVar->getNumValue();
	}
	logInfo(buildString("\tCredits: %d", _nNumCredits));

	g_bBibbleHack = false;
	pVar = g_VarManager->getVariable("BIBBLEHACK");
	if ((pVar != nullptr) && (pVar->getNumValue() != 0)) {
		g_bBibbleHack = true;
	}

	// If player has modified the payoffs, then load new payoffs from Bar script
	if (g_bBibbleHack) {
		CBagStorageDev *pSDev = g_SDevManager->getStorageDevice("BGNM_WLD");

		if (pSDev != nullptr) {
			for (int i = 0; i < BIBBLE_NUM_BET_AREAS; i++) {
				// The sprite object start in the script at 500
				CBagObject *pObj = pSDev->getObject(500 + i);
				if (pObj != nullptr) {
					int nPayOff = pObj->getState();
					g_engine->g_cBetAreas[i]._nPayOff1 = PAY_OFFS[nPayOff]._nPay1;
					g_engine->g_cBetAreas[i]._nPayOff2 = PAY_OFFS[nPayOff]._nPay2;
				}
			}
		}
	}

	// Must have a valid backdrop by now
	assert(_pBackdrop != nullptr);
	CBofPalette *pPal = _pBackdrop->getPalette();

	// Setup the text fields
	_pCreditsText = new CBofText;

	CBofRect cRect(CREDITS_AREA_X1, CREDITS_AREA_Y1, CREDITS_AREA_X2, CREDITS_AREA_Y2);

	_pCreditsText->setupText(&cRect, JUSTIFY_RIGHT, FORMAT_CENTER_RIGHT);
	_pCreditsText->setColor(CTEXT_WHITE);
	_pCreditsText->SetSize(mapWindowsPointSize(20));
	_pCreditsText->setWeight(TEXT_BOLD);
	_pCreditsText->setText(buildString("%d", _nNumCredits));

	// Pre-load the "One", "Two", "Three", and "Four" shouts
	for (int i = 0; i < BIBBLE_NUM_SHOUTS; i++) {
		_pShouts[i] = new CBofSound(this, BuildDir(pszShouts[i]), SOUND_MIX);
	}

	// Pre-load the ball
	_pBall = new CBofSprite;

	_pBall->loadSprite(BuildDir(BALL_BMP), BALL_CELS);
	_pBall->setMaskColor(MASK_COLOR);
	_pBall->setZOrder(SPRITE_HINDMOST);
	_pBall->setAnimated(true);
	_pBall->linkSprite();

	// Pre-load the bibbles
	_pMasterBibble = new CBofSprite;
	_pMasterBibble->loadSprite(BuildDir(BIBBLE_BMP), BIBBLE_CELS);
	_pMasterBibble->setMaskColor(MASK_COLOR);
	_pMasterBibble->setZOrder(SPRITE_TOPMOST);
	_pMasterBibble->setAnimated(false);

	// Dup the bibbles
	for (int i = 0; i < BIBBLE_NUM_BIBBLES; i++) {
		_pBibble[i] = _pMasterBibble->duplicateSprite();
		if (_pBibble[i] == nullptr)
			fatalError(ERR_MEMORY, "Unable to duplicate a CBofSprite");

		_pBibble[i]->setPosition(nBibbleXPos[i], nBibbleYPos[i]);
		_pBibble[i]->linkSprite();
	}

	// Load the arch bitmaps that the ball needs to go behind
	_pArch1 = new CBofSprite;
	_pArch1->loadSprite(BuildDir(ARCH1_BMP));
	_pArch1->setMaskColor(MASK_COLOR);
	_pArch1->setZOrder(SPRITE_MIDDLE);
	_pArch1->linkSprite();

	_pArch2 = new CBofSprite;
	_pArch2->loadSprite(BuildDir(ARCH2_BMP));
	_pArch2->setMaskColor(MASK_COLOR);
	_pArch2->setZOrder(SPRITE_MIDDLE);
	_pArch2->linkSprite();

	_pArch3 = new CBofSprite;
	_pArch3->loadSprite(BuildDir(ARCH3_BMP));
	_pArch3->setMaskColor(MASK_COLOR);
	_pArch3->setZOrder(SPRITE_MIDDLE);
	_pArch3->linkSprite();

	// Build all our buttons
	for (int i = 0; i < BIBBLE_NUM_BUTTONS; i++) {
		_pButtons[i] = new CBofBmpButton;

		CBofBitmap *pUp = loadBitmap(BuildDir(g_stButtons[i]._pszUp), pPal);
		CBofBitmap *pDown = loadBitmap(BuildDir(g_stButtons[i]._pszDown), pPal);
		CBofBitmap *pFocus = loadBitmap(BuildDir(g_stButtons[i]._pszFocus), pPal);
		CBofBitmap *pDis = loadBitmap(BuildDir(g_stButtons[i]._pszDisabled), pPal);

		_pButtons[i]->loadBitmaps(pUp, pDown, pFocus, pDis);
		_pButtons[i]->create(g_stButtons[i]._pszName, g_stButtons[i]._nLeft, g_stButtons[i]._nTop, g_stButtons[i]._nWidth, g_stButtons[i]._nHeight, this, g_stButtons[i]._nID);
		_pButtons[i]->show();
	}

	_pBkgSnd = new CBofSound(this, BuildDir(CASINO_AUDIO), SOUND_MIX, 99999);
	_pBkgSnd->play();

	// No bet area currently selected
	_pSelected = nullptr;

	show();

	updateWindow();

	CBagCursor::showSystemCursor();

	return _errCode;
}


ErrorCode CBibbleWindow::detach() {
	assert(isValidObject(this));

	logInfo("\tCBibbleWindow::detach()");

	CBagCursor::hideSystemCursor();

	// Remove any unused credits from the table, and put them back into your
	// Credit Chip
	for (int i = 0; i < BIBBLE_NUM_BET_AREAS; i++) {
		assert(g_engine->g_cBetAreas[i]._nBet >= 0);

		_nNumCredits += g_engine->g_cBetAreas[i]._nBet;
		g_engine->g_cBetAreas[i]._nBet = 0;
	}

	// Write out new value of nuggets
	CBagVar *pVar = g_VarManager->getVariable("NUGGETS");
	if (pVar != nullptr) {
		pVar->setValue(_nNumCredits);
	}

	if (_pBkgSnd->isPlaying()) {
		_pBkgSnd->stop();
	}

	delete _pBkgSnd;
	_pBkgSnd = nullptr;

	// Destroy the ball, bibbles, and bibble shouts
	delete _pArch3;
	_pArch3 = nullptr;

	delete _pArch2;
	_pArch2 = nullptr;

	delete _pArch1;
	_pArch1 = nullptr;

	for (int i = 0; i < BIBBLE_NUM_BIBBLES; i++) {
		delete _pBibble[i];
		_pBibble[i] = nullptr;
	}

	delete _pMasterBibble;
	_pMasterBibble = nullptr;

	delete _pBall;
	_pBall = nullptr;

	for (int i = 0; i < BIBBLE_NUM_SHOUTS; i++) {
		delete _pShouts[i];
		_pShouts[i] = nullptr;
	}

	// Destroy all buttons
	for (int i = 0; i < BIBBLE_NUM_BUTTONS; i++) {
		delete _pButtons[i];
		_pButtons[i] = nullptr;
	}

	delete _pCreditsText;
	_pCreditsText = nullptr;

	// Close sprite lib
	CBofSprite::closeLibrary();

	killBackdrop();

	g_waitOKFl = true;

	CBagStorageDevWnd::detach();

	// Playing BibbleBonk has made 1 turn go by.
	g_VarManager->incrementTimers();

	return _errCode;
}


void CBibbleWindow::onMainLoop() {
	// Do nothing
}


void CBibbleWindow::onPaint(CBofRect *pRect) {
	assert(isValidObject(this));
	assert(pRect != nullptr);

	CBofBitmap *pBmp;

	// Render offscreen
	if (_pBackdrop != nullptr && (pBmp = getWorkBmp()) != nullptr) {

		_pBackdrop->paint(pBmp, pRect, pRect);

		// Paint the bibbles - just sitting there
		for (int i = 0; i < BIBBLE_NUM_BIBBLES; i++) {
			if (_pBibble[i] != nullptr) {
				_pBibble[i]->paintSprite(pBmp, _pBibble[i]->getPosition());
			}
		}

		if (_pArch1 != nullptr) {
			_pArch1->paintSprite(pBmp, 238, 107);
		}
		if (_pArch2 != nullptr) {
			_pArch2->paintSprite(pBmp, 336, 145);
		}
		if (_pArch3 != nullptr) {
			_pArch3->paintSprite(pBmp, 435, 178);
		}

		CBofRect cRect;

		// Update the display of all the bet areas
		for (int i = 0; i < BIBBLE_NUM_BET_AREAS; i++) {
			// Add bet amount text
			char szBuf[20];
			Common::sprintf_s(szBuf, "%d", g_engine->g_cBetAreas[i]._nBet);
			cRect = g_engine->g_cBetAreas[i]._cRect;
			cRect.top += cRect.height() / 2;

			paintText(pBmp, &cRect, szBuf, 16, TEXT_NORMAL, CTEXT_COLOR, JUSTIFY_RIGHT, FORMAT_BOT_RIGHT);
		}

		// Now update the screen
		pBmp->paint(this, pRect, pRect);

		if (_pSelected != nullptr) {
			highlight(_pSelected, HIGHLIGHT_COLOR);
		}

		// Update # of credits display
		if (_pCreditsText != nullptr) {
			_pCreditsText->display(this);
		}
	}
}


void CBibbleWindow::onBofButton(CBofObject *pObject, int nState) {
	assert(isValidObject(this));
	assert(pObject != nullptr);

	if (nState != BUTTON_CLICKED)
		return;

	CBofButton *pButton = (CBofButton *)pObject;

	int nBet = 0;

	switch (pButton->getControlID()) {
	case BIBBLE_BUTTON_READY:
		logInfo("\tClicked READY button");
		playGame();
		break;

	case BIBBLE_BUTTON_BET1:
		logInfo("\tClicked BET 1");
		nBet = MIN((int)_nNumCredits, 1);
		break;

	case BIBBLE_BUTTON_BET5:
		logInfo("\tClicked BET 5");
		nBet = MIN((int)_nNumCredits, 5);
		break;

	case BIBBLE_BUTTON_BET10:
		logInfo("\tClicked BET 10");
		nBet = MIN((int)_nNumCredits, 10);
		break;

	case BIBBLE_BUTTON_BET50:
		logInfo("\tClicked BET 50");
		nBet = MIN((int)_nNumCredits, 50);
		break;

	case BIBBLE_BUTTON_BET100:
		logInfo("\tClicked BET 100");
		nBet = MIN((int)_nNumCredits, 100);
		break;

	case BIBBLE_BUTTON_BET500:
		logInfo("\tClicked BET 500");
		nBet = MIN((int)_nNumCredits, 500);
		break;

	case BIBBLE_BUTTON_BET1000:
		logInfo("\tClicked BET 1000");
		nBet = MIN((int)_nNumCredits, 1000);
		break;

	case BIBBLE_BUTTON_BET5000:
		logInfo("\tClicked BET 5000");
		nBet = MIN((int)_nNumCredits, 5000);
		break;

	case BIBBLE_BUTTON_CLEARBET:
		logInfo("\tClicked Clear Bet");
		if (_pSelected != nullptr) {
			nBet -= _pSelected->_nBet;
		}
		break;

	case BIBBLE_BUTTON_QUIT:
		logInfo("\tClicked Quit");
		close();
		break;

	case BIBBLE_BUTTON_HELP: {
		logInfo("\tClicked Help");

		CBagel *pApp = CBagel::getBagApp();
		if (pApp != nullptr) {
			CBagMasterWin *pWin = pApp->getMasterWnd();
			if (pWin != nullptr) {
				pWin->onHelp(BuildDir("BIBBLE.TXT"));
			}
		}
		break;
	}

	default:
		logWarning(buildString("Clicked Unknown Button with ID %d", pButton->getControlID()));
		break;
	}

	if (nBet != 0) {
		if (_nNumCredits < MAX_AMOUNT) {
			if (_pSelected != nullptr) {
				logInfo(buildString("\tHave %d Credits, Betting %d credits", _nNumCredits, nBet));
				_nNumCredits -= nBet;
				_pSelected->_nBet += nBet;
				logInfo(buildString("\t%d Credits remaining.", _nNumCredits));
				highlight(_pSelected, HIGHLIGHT_COLOR);
			}

		} else {
			// The user is not allowed to make anymore bets
			char szBuf[256];
			Common::strcpy_s(szBuf, "The bonkier apologizes, explaining that in light of your winning streak, the house can no longer accept your bets.");
			CBofString cString(szBuf, 256);

			CBofRect cRect(0, 440, 640 - 1, 480 - 1);

			CBofBitmap cBmp(cRect.width(), cRect.height(), (CBofPalette *)nullptr, false);
			cBmp.captureScreen(this, &cRect);

			paintBeveledText(&cRect, cString, FONT_15POINT, TEXT_NORMAL, CTEXT_WHITE, JUSTIFY_WRAP, FORMAT_TOP_LEFT);
			waitForInput();

			cBmp.paint(this, &cRect);
		}

		displayCredits();
	}
}

void CBibbleWindow::onClose() {
	g_engine->quitGame();
}

ErrorCode CBibbleWindow::displayCredits() {
	assert(isValidObject(this));

	// Display new number of credits
	if (_pCreditsText != nullptr) {
		char szBuf[20];

		Common::sprintf_s(szBuf, "%d", _nNumCredits);
		_pCreditsText->setText(szBuf);
		_pCreditsText->display(this);
	}

	return _errCode;
}


ErrorCode CBibbleWindow::playGame() {
	logInfo(buildString("\tPlaying BibbleBonk, Starting Credits: %d", _nNumCredits));

	// Remove any current highlight
	if (_pSelected != nullptr)
		unHighlight(_pSelected);

	// Pre-determine the outcome
	calcOutcome();

	//
	// Bonk those Bibbles (Play game based on pre-determined outcome)
	//
	bonkBibble(_nBall1, _nBall1Said);
	bonkBibble(_nBall2, _nBall2Said);
	bonkBibble(_nBall3, _nBall3Said);

	// Assume we will lose
	bool bWin = false;

	// Find each winner
	for (int i = 0; i < BIBBLE_NUM_BET_AREAS; i++) {
		// If user bet on this square
		if (g_engine->g_cBetAreas[i]._bWon) {
			// Highlight this winning square
			highlight(&g_engine->g_cBetAreas[i], WINHIGHLIGHT_COLOR);

			if (g_engine->g_cBetAreas[i]._nBet != 0) {
				if (!bWin) {
					logInfo("\tWinner");

					BofPlaySoundEx(BuildDir(BIBBLE_AUDIO_WINNER), SOUND_MIX | SOUND_QUEUE, 7, false);
				}
				bWin = true;

				// Play the "Zip Middle Bibble, pays seven to one", etc...

				// "Zip Middle Bibble"
				BofPlaySoundEx(BuildDir(g_engine->g_cBetAreas[i]._cAudioFile), SOUND_MIX | SOUND_QUEUE, 7, false);

				// "Pays"
				BofPlaySoundEx(BuildDir(BIBBLE_AUDIO_PAYS), SOUND_MIX | SOUND_QUEUE, 7, false);

				if (!g_bBibbleHack) {

					// "7 to 1"
					BofPlaySoundEx(BuildDir(g_engine->g_cBetAreas[i]._cPayFile), SOUND_MIX | SOUND_QUEUE, 7, true);

				} else {
					switch (g_engine->getRandomNumber() % 3) {
					case 0:
						BofPlaySoundEx(BuildDir(BIBBLE_AUDIO_PAYS2), SOUND_MIX | SOUND_QUEUE, 7, true);
						break;

					case 1:
						BofPlaySoundEx(BuildDir(BIBBLE_AUDIO_PAYS3), SOUND_MIX | SOUND_QUEUE, 7, true);
						break;

					case 2:
						BofPlaySoundEx(BuildDir(BIBBLE_AUDIO_PAYS4), SOUND_MIX | SOUND_QUEUE, 7, true);
						break;

					default:
						break;
					}
				}

				// Calc Payoff
				int nPayoff = (g_engine->g_cBetAreas[i]._nBet * g_engine->g_cBetAreas[i]._nPayOff1) / g_engine->g_cBetAreas[i]._nPayOff2;

				// Mo' money, Mo' money
				_nNumCredits += nPayoff;

				// Display new number of credits
				displayCredits();

				logInfo(buildString("\tWinner on square %d.  Pays %d credits", i, nPayoff));
			}
		}
	}

	// Otherwise, play loser audios
	if (!bWin) {
		logInfo("\tLoser");
	}

	bofSleep(2000);

	// Clear all bets (On table)
	logInfo("\tClearing all Bets");
	for (int i = 0; i < BIBBLE_NUM_BET_AREAS; i++) {
		g_engine->g_cBetAreas[i]._nBet = 0;
		unHighlight(&g_engine->g_cBetAreas[i]);
	}

	_pSelected = nullptr;

	logInfo(buildString("\tDone BibbleBonk.  Credits: %d", _nNumCredits));

	return _errCode;
}

ErrorCode CBibbleWindow::bonkBibble(int nBibbleID, int nShouts) {
	assert(isValidObject(this));
	assert(nBibbleID >= 1 && nBibbleID <= 3);
	assert(nShouts >= 1 && nShouts <= 4);

	int nBallStartYPos[] = {
		BALL_START_Y1,
		BALL_START_Y2,
		BALL_START_Y3
	};

	//
	// Display the ball rolling down the chute and then hitting the bibble,
	// causing the bibble to roll over the ball (which continues off the
	// edge of the screen).  The bibble shouts the specified number.
	//

	// Set ball initial position
	CBofPoint cBallPosition;
	cBallPosition.x = BALL_START_X;
	cBallPosition.y = nBallStartYPos[nBibbleID - 1];

	// Get a local bibble
	CBofSprite *pBibble = _pBibble[nBibbleID - 1];
	pBibble->setAnimated(true);

	for (;;) {
		cBallPosition.x += BALL_SPEED;
		_pBall->paintSprite(this, cBallPosition);

		if (_pBall->testInterception(pBibble)) {

			_pBall->eraseSprite(this);

			for (int i = 0; i < pBibble->getCelCount(); i++) {
				pBibble->paintSprite(this, pBibble->getPosition());

				if (i == START_YELL) {
					// Start shouting
					_pShouts[nShouts - 1]->play();
				}

				bofSleep(SPEED_DELAY2);
			}
			break;
		}

		bofSleep(SPEED_DELAY);
	}

	pBibble->setAnimated(false);

	return _errCode;
}


void CBibbleWindow::calcOutcome() {
	assert(isValidObject(this));
	int i;

	const int g_nBibbleTable[] = {
		1, 1,
		2, 2, 2, 2, 2,
		3, 3, 3
	};
	const int g_nPayTable[] = {
		4,
		3,
		2,
	};

	// Start from scratch
	_nBall1 = 0;
	_nBall2 = 0;
	_nBall3 = 0;
	_nBall1Said = 0;
	_nBall2Said = 0;
	_nBall3Said = 0;
	_nNumShout1 = 0;
	_nNumShout2 = 0;
	_nNumShout3 = 0;
	_nNumShout4 = 0;
	_nNumTopBonks = 0;
	_nNumMidBonks = 0;
	_nNumBotBonks = 0;

	// Clear internal betting area
	for (i = 0; i < BIBBLE_NUM_BET_AREAS; i++) {
		g_engine->g_cBetAreas[i]._bWon = false;
	}

	//
	// Determine which Bibbles were hit by each ball, and
	// Determine what each of those Bibbles shouted.
	//

	// Ball 1
	_nBall1 = g_nBibbleTable[g_engine->getRandomNumber() % 10];
	_nBall1Said = (g_engine->getRandomNumber() % g_nPayTable[_nBall1 - 1]) + 1;
	logInfo(buildString("\tBall 1 hit Bibble %d", _nBall1));
	logInfo(buildString("\tBibble says: %d", _nBall1Said));

	// Count number of each Shout
	switch (_nBall1Said) {
	case 1:
		_nNumShout1++;
		break;

	case 2:
		_nNumShout2++;
		break;

	case 3:
		_nNumShout3++;
		break;

	case 4:
		_nNumShout4++;
		break;

	default:
		logWarning("Invalid case");
		break;
	}

	// Ball 2
	_nBall2 = g_nBibbleTable[g_engine->getRandomNumber() % 10];
	_nBall2Said = (g_engine->getRandomNumber() % g_nPayTable[_nBall2 - 1]) + 1;
	logInfo(buildString("\tBall 2 hit Bibble %d", _nBall2));
	logInfo(buildString("\tBibble says: %d", _nBall2Said));

	// Count number of each Shout
	switch (_nBall2Said) {
	case 1:
		_nNumShout1++;
		break;

	case 2:
		_nNumShout2++;
		break;

	case 3:
		_nNumShout3++;
		break;

	case 4:
		_nNumShout4++;
		break;

	default:
		logWarning("Invalid case");
		break;
	}

	// Ball 3
	_nBall3 = g_nBibbleTable[g_engine->getRandomNumber() % 10];
	_nBall3Said = (g_engine->getRandomNumber() % g_nPayTable[_nBall3 - 1]) + 1;
	logInfo(buildString("\tBall 3 hit Bibble %d", _nBall3));
	logInfo(buildString("\tBibble says: %d", _nBall3Said));

	// Count number of each Shout
	switch (_nBall3Said) {
	case 1:
		_nNumShout1++;
		break;

	case 2:
		_nNumShout2++;
		break;

	case 3:
		_nNumShout3++;
		break;

	case 4:
		_nNumShout4++;
		break;

	default:
		logWarning("Invalid case");
		break;
	}

	// Count number of each bonks
	_nNumTopBonks = (_nBall1 == 1 ? 1 : 0) + (_nBall2 == 1 ? 1 : 0) + (_nBall3 == 1 ? 1 : 0);
	_nNumMidBonks = (_nBall1 == 2 ? 1 : 0) + (_nBall2 == 2 ? 1 : 0) + (_nBall3 == 2 ? 1 : 0);
	_nNumBotBonks = (_nBall1 == 3 ? 1 : 0) + (_nBall2 == 3 ? 1 : 0) + (_nBall3 == 3 ? 1 : 0);

	// Determine winning bets
	g_engine->g_cBetAreas[_nNumTopBonks + 0]._bWon = true;
	g_engine->g_cBetAreas[_nNumMidBonks + 4]._bWon = true;
	g_engine->g_cBetAreas[_nNumBotBonks + 8]._bWon = true;

	for (i = 12; i < BIBBLE_NUM_BET_AREAS; i++) {
		switch (i % 4) {
		case 0:
			if (_nNumShout1 >= ((i - 12) / 4) + 1) {
				g_engine->g_cBetAreas[i]._bWon = true;
			}
			break;

		case 1:
			if (_nNumShout2 >= ((i - 13) / 4) + 1) {
				g_engine->g_cBetAreas[i]._bWon = true;
			}
			break;

		case 2:
			if (_nNumShout3 >= ((i - 14) / 4) + 1) {
				g_engine->g_cBetAreas[i]._bWon = true;
			}
			break;

		case 3:
			if (_nNumShout4 >= ((i - 15) / 4) + 1) {
				g_engine->g_cBetAreas[i]._bWon = true;
			}
			break;

		default:
			break;
		}
	}

	logInfo(buildString("\tNumber of Top Bonks: %d", _nNumTopBonks));
	logInfo(buildString("\tNumber of Mid Bonks: %d", _nNumMidBonks));
	logInfo(buildString("\tNumber of Bot Bonks: %d", _nNumBotBonks));

	logInfo(buildString("\tNumber of '1' Babbles: %d", _nNumShout1));
	logInfo(buildString("\tNumber of '2' Babbles: %d", _nNumShout2));
	logInfo(buildString("\tNumber of '3' Babbles: %d", _nNumShout3));
	logInfo(buildString("\tNumber of '4' Babbles: %d", _nNumShout4));
}

void CBibbleWindow::onLButtonDblClk(uint32 /*nFlags*/, CBofPoint *pPoint) {
	assert(isValidObject(this));
	assert(pPoint != nullptr);

	int nBet = MIN((int)_nNumCredits, 100);

	for (int i = 0; i < BIBBLE_NUM_BET_AREAS; i++) {
		CBetArea *pArea = &g_engine->g_cBetAreas[i];

		// If this is the area the user selected
		if (pArea->_cRect.ptInRect(*pPoint)) {
			CBetArea *pPrevArea = _pSelected;

			// Keep track of selected area
			_pSelected = pArea;

			logInfo(buildString("\tDouble Clicked on Square: %d", i));

			// Remove any previous selection highlight
			if (pPrevArea != nullptr) {
				unHighlight(pPrevArea);
			}

			logInfo(buildString("\tHave %d Credits, Betting %d credits", _nNumCredits, nBet));
			_nNumCredits -= nBet;
			_pSelected->_nBet += nBet;
			logInfo(buildString("\t%d Credits remaining.", _nNumCredits));

			// Highlight the area selected
			highlight(_pSelected, HIGHLIGHT_COLOR);

			displayCredits();
			break;
		}
	}
}


void CBibbleWindow::onLButtonUp(uint32 /*nFlags*/, CBofPoint * /*pPoint*/, void *) {
	assert(isValidObject(this));
}


void CBibbleWindow::onLButtonDown(uint32 /*nFlags*/, CBofPoint *pPoint, void *) {
	assert(isValidObject(this));
	assert(pPoint != nullptr);

	for (int i = 0; i < BIBBLE_NUM_BET_AREAS; i++) {
		CBetArea *pArea = &g_engine->g_cBetAreas[i];

		// If this is the area the user selected
		//
		if (pArea->_cRect.ptInRect(*pPoint)) {
			CBetArea *pPrevArea = _pSelected;

			// Keep track of selected area
			_pSelected = pArea;

			logInfo(buildString("\tSelected %d", i));

			// remove any previous selection highlight
			if (pPrevArea != nullptr) {
				unHighlight(pPrevArea);
			}

			// highlight the area selected
			highlight(pArea, HIGHLIGHT_COLOR);

			break;
		}
	}
}


ErrorCode CBibbleWindow::highlight(CBetArea *pArea, byte nColor) {
	assert(isValidObject(this));
	assert(pArea != nullptr);
	assert(_pBackdrop != nullptr);

	CBofBitmap cBmp(pArea->_cRect.width(), pArea->_cRect.height(), _pBackdrop->getPalette());
	CBofRect r = cBmp.getRect();
	_pBackdrop->paint(&cBmp, &r, &pArea->_cRect);

	// Add highlight rectangle
	CBofRect cRect = cBmp.getRect();

	cBmp.drawRect(&cRect, nColor);
	cRect.left += 1;
	cRect.right -= 1;
	cRect.top += 1;
	cRect.bottom -= 1;

	cBmp.drawRect(&cRect, nColor);

	// Add bet amount text
	char szBuf[20];

	Common::sprintf_s(szBuf, "%d", pArea->_nBet);
	cRect = cBmp.getRect();
	cRect.top += cRect.height() / 2;

	paintText(&cBmp, &cRect, szBuf, 16, TEXT_NORMAL, CTEXT_COLOR, JUSTIFY_RIGHT, FORMAT_BOT_RIGHT);

	// Paint result to screen
	cBmp.paint(this, &pArea->_cRect);

	return _errCode;
}


ErrorCode CBibbleWindow::unHighlight(CBetArea *pArea) {
	assert(isValidObject(this));
	assert(pArea != nullptr);
	assert(_pBackdrop != nullptr);

	CBofBitmap cBmp(pArea->_cRect.width(), pArea->_cRect.height(), _pBackdrop->getPalette());

	// Copy bet area
	CBofRect r = cBmp.getRect();
	_pBackdrop->paint(&cBmp, &r, &pArea->_cRect);

	// Add bet amount text
	char szBuf[20];

	Common::sprintf_s(szBuf, "%d", pArea->_nBet);
	CBofRect cRect = cBmp.getRect();
	cRect.top += cRect.height() / 2;

	paintText(&cBmp, &cRect, szBuf, 16, TEXT_NORMAL, CTEXT_COLOR, JUSTIFY_RIGHT, FORMAT_BOT_RIGHT);

	// Paint to screen
	cBmp.paint(this, &pArea->_cRect);

	return _errCode;
}

void CBibbleWindow::onKeyHit(uint32 lKey, uint32 /*lRepCount*/) {
	assert(isValidObject(this));

	if (lKey == BKEY_ESC)
		close();
}


const char *BuildDir(const char *pszFile) {
	return formPath(BIBBLEDIR, pszFile);
}

} // namespace SpaceBar
} // namespace Bagel
