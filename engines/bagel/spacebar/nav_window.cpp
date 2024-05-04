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

#include "bagel/spacebar/nav_window.h"
#include "bagel/spacebar/master_win.h"
#include "bagel/boflib/event_loop.h"
#include "bagel/boflib/file_functions.h"
#include "bagel/boflib/log.h"
#include "bagel/boflib/sound.h"
#include "bagel/boflib/std_keys.h"

namespace Bagel {
namespace SpaceBar {

#define MAP_BMP       "nebsim1.bmp"
#define CUR_LOC       "curloc.bmp"
#define LEVEL_DONE    "levdone.bmp"
#define WELCOME_SND   "evnaawel.wav"
#define QUIT 111
#define HELP 112
#define MASK_COLOR 1
#define HIGHLIGHT_COLOR     13

#define NAVDIR      "$SBARDIR\\NEBLITZ\\CLOSEUP\\NAV\\"
#define NEBSIM2BMP  "$SBARDIR\\NEBLITZ\\CLOSEUP\\NAV\\nebsim2.bmp"
#define NEBSIM3BMP  "$SBARDIR\\NEBLITZ\\CLOSEUP\\NAV\\nebsim3.bmp"
#define NEBSIM4BMP  "$SBARDIR\\NEBLITZ\\CLOSEUP\\NAV\\nebsim4.bmp"

// New background bmps are black, so text will be white
#define NAVTEXT_COLOR         RGB(255,255,255)

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

NPLANET g_Level[9] = {
	{ "Pinna", 36, 54, 45, 63, {nullptr, 0, true} },
	{ "H'poctyl", 181, 19, 190, 28, { "Mattresses", 72, true} },
	{ "Armpit", 108, 251, 117, 260, { "Spam", 56, true} },
	{ "Yzore", 251, 271, 260, 280, {nullptr, 0, true} },
	{ "Bulbus", 323, 197, 332, 206, {nullptr, 0, true} },
	{ "Dingle", 394, 143, 403, 154, {nullptr, 0, true} },
	{ "Karkas", 163, 394, 172, 403, {nullptr, 0, true} },
	{ "Fruufnia", 306, 393, 315, 402, { "Swizzle Sticks", 86, true} },
	{ nullptr, 0, 0, 0, 0, {nullptr, 0, true} }
};

NPLANET g_LevelOne[10] = {
	{ "Wilbur", 108, 38, 116, 49, {nullptr, 0, true} },
	{ "McKelvey", 394, 37, 403, 46, {nullptr, 0, true} },
	{ "Medge", 19, 90, 28, 99, { "Memory Chips", 127, true} },
	{ "Wall", 234, 90, 243, 99, {nullptr, 0, true} },
	{ "Woo", 108, 306, 119, 315, {nullptr, 0, true} },
	{ "Roman", 198, 270, 207, 279, { "Motorcycles", 20, true} },
	{ "Weed", 322, 252, 331, 261, {nullptr, 0, true} },
	{ "Bok", 55, 395, 64, 404, { "Frisbees", 119, true} },
	{ "Pizer", 339, 412, 348, 421, { "Bird Seed", 54, true} },
	{nullptr, 0, 0, 0, 0, {nullptr, 0, true}}
};
NPLANET g_LevelTwo[9] = {
	{ "Backwater", 108, 18, 117, 27, { "Kayaks", 28, true} },
	{ "Uranus", 161, 126, 170, 135, { "Toilet Paper", 14, true} },
	{ "Toe Jam", 252, 126, 261, 135, { "Peanutbutter", 45, true} },
	{ "Hellhole", 412, 108, 421, 117, { "Pitchforks", 100, true} },
	{ "Pustule", 180, 269, 189, 278, {nullptr, 0, true} },
	{ "Swamp Rock", 340, 234, 349, 243, {nullptr, 0, true} },
	{ "Slug", 36, 341, 45, 350, { "Slime Remover", 75, true} },
	{ "Maggot's Nest", 251, 376, 260, 385, {nullptr, 0, true} },
	{ nullptr, 0, 0, 0, 0, {nullptr, 0, false}}
};
NPLANET g_LevelThree[11] = {
	{ "Peggleboz", 90, 20, 99, 29, { "Shot Glasses", 99, true} },
	{ "Archeroids", 144, 89, 153, 98, { "Arrows", 8, true} },
	{ "Pack-Rat", 324, 55, 333, 64, { "Turtle Food", 24, true} },
	{ "Beacon", 270, 144, 279, 155, { "Fog Horns", 125, true} },
	{ "Mankala", 72, 251, 81, 260, {nullptr, 0, true} },
	{ "Life", 394, 234, 403, 243, {nullptr, 0, true} },
	{ "Fuge", 270, 324, 279, 333, { "Cotton Candy", 10, true} },
	{ "Garfunkel", 90, 377, 99, 386, {nullptr, 0, true} },
	{ "Battlefish", 359, 359, 368, 368, { "Harpoons", 17, true} },
	{ "No Vacancy", 18, 107, 117, 116, {nullptr, 0, true} },
	{ nullptr, 0, 0, 0, 0, {nullptr, 0, true} }
};

ST_BUTTONS g_NavButtons[2] = {
	{ "Quit", "QUP.bmp", "QDOWN.bmp", "QDIS.bmp", "QDIS.bmp", 432, 443, 200, 30, QUIT },
	{ "Instructions", "HUP.bmp", "HDOWN.bmp", "HDIS.bmp", "HDIS.bmp", 4, 443, 200, 30, HELP },
};

const char *g_LevelTitle[4] = {
	"Novice",
	"Moderate",
	"Hard",
	"Expert"
};


// Local functions
const char *MakeDir(const char *pszFile) {
	return formPath(NAVDIR, pszFile);
}

CNavWindow::CNavWindow() {
	logInfo("Constructing NAV...");

	_level = 0;
	_pPinna = nullptr;
	_pHpoctyl = nullptr;
	_pArmpit = nullptr;
	_pYzore = nullptr;
	_pBulbus = nullptr;
	_pDingle = nullptr;
	_pKarkas = nullptr;
	_pFruufnia = nullptr;
	_pButtons[0] = nullptr;
	_pButtons[1] = nullptr;
	_pPal = nullptr;
	_pOldPal = nullptr;
	_pMap = nullptr;
	_pCurLoc = nullptr;
	//  _pLevelDone = nullptr;
	_pNewMap = nullptr;
	_pCurPos = nullptr;
	_pPortName = nullptr;
	_pWilbur = nullptr;
	_pMcKelvey = nullptr;
	_pMedge = nullptr;
	_pWall = nullptr;
	_pWoo = nullptr;
	_pRoman = nullptr;
	_pWeed = nullptr;
	_pBok = nullptr;
	_pPizer = nullptr;
	_pBackwater = nullptr;
	_pUranus = nullptr;
	_pToejam = nullptr;
	_pHellhole = nullptr;
	_pPustule = nullptr;
	_pSwamprock = nullptr;
	_pSlug = nullptr;
	_pMaggotsnest = nullptr;
	_pPeggleboz = nullptr;
	_pArcheroids = nullptr;
	_pPackRat = nullptr;
	_pBeacon = nullptr;
	_pMankala = nullptr;
	_pLife = nullptr;
	_pFuge = nullptr;
	_pGarfunkel = nullptr;
	_pBattlefish = nullptr;
	_pNoVacancy = nullptr;
	_bNavAttached = false;
}

ErrorCode CNavWindow::attach() {
	assert(isValidObject(this));

	logInfo("\tCNavWindow::attach()");
	CBagStorageDevWnd::attach();

	_level = 0;

	_pPortName = new CBofString("Yzore");      // always starts at Yzore
	_ship = 120;
	_cargo = 86 + 72 + 56;
	_fuel = 40;

	// navsim1
	_pPinna = new CBofRect(36, 54, 45, 63);
	_pHpoctyl = new CBofRect(181, 19, 190, 28);
	_pArmpit = new CBofRect(108, 251, 117, 260);
	_pYzore = new CBofRect(251, 271, 260, 280);
	_pBulbus = new CBofRect(323, 197, 332, 206);
	_pDingle = new CBofRect(394, 143, 403, 154);
	_pKarkas = new CBofRect(163, 394, 172, 403);
	_pFruufnia = new CBofRect(306, 393, 315, 402);

	// navsim2
	_pWilbur = new CBofRect(108, 38, 116, 49);
	_pMcKelvey = new CBofRect(394, 37, 403, 46);
	_pMedge = new CBofRect(19, 90, 28, 99);
	_pWall = new CBofRect(234, 90, 243, 99);
	_pWoo = new CBofRect(108, 306, 119, 315);
	_pRoman = new CBofRect(198, 270, 207, 279);
	_pWeed = new CBofRect(322, 252, 331, 261);
	_pBok = new CBofRect(55, 395, 64, 404);
	_pPizer = new CBofRect(339, 412, 348, 421);

	// navsim3
	_pBackwater = new CBofRect(108, 18, 117, 27);
	_pUranus = new CBofRect(161, 126, 170, 135);
	_pToejam = new CBofRect(252, 126, 261, 135);
	_pHellhole = new CBofRect(412, 108, 421, 117);
	_pPustule = new CBofRect(180, 269, 189, 278);
	_pSwamprock = new CBofRect(340, 234, 349, 243);
	_pSlug = new CBofRect(36, 341, 45, 350);
	_pMaggotsnest = new CBofRect(251, 376, 260, 385);

	// navsim4
	_pPeggleboz = new CBofRect(90, 20, 99, 29);
	_pArcheroids = new CBofRect(144, 89, 153, 98);
	_pPackRat = new CBofRect(324, 55, 333, 64);
	_pBeacon = new CBofRect(270, 144, 279, 155);
	_pMankala = new CBofRect(72, 251, 81, 260);
	_pLife = new CBofRect(394, 234, 403, 243);
	_pFuge = new CBofRect(270, 324, 279, 333);
	_pGarfunkel = new CBofRect(90, 377, 99, 386);
	_pBattlefish = new CBofRect(359, 359, 368, 368);
	_pNoVacancy = new CBofRect(18, 107, 117, 116);

	// Reset all levels
	int i;
	for (i = 0; i < 9; i++) {
		g_Level[i].cargo._bUsed = true;
	}
	for (i = 0; i < 10; i++) {
		g_LevelOne[i].cargo._bUsed = true;
	}
	for (i = 0; i < 9; i++) {
		g_LevelTwo[i].cargo._bUsed = true;
	}
	for (i = 0; i < 11; i++) {
		g_LevelThree[i].cargo._bUsed = true;
	}

	_pLevel = g_Level;
	_pCurPos = new CBofRect(*_pYzore);

	_pOldPal = CBofApp::getApp()->getPalette();
	_pPal = _pBackdrop->getPalette()->copyPalette();
	CBofApp::getApp()->setPalette(_pPal);
	_pCurLoc = new CBofSprite;
	if (_pCurLoc != nullptr) {
		_pCurLoc->loadSprite(MakeDir(CUR_LOC), 2);
		_pCurLoc->setMaskColor(MASK_COLOR);
		_pCurLoc->setZOrder(SPRITE_TOPMOST);
		_pCurLoc->setAnimated(true);
		_pCurLoc->linkSprite();
		_pCurLoc->setPosition(_pCurPos->left, _pCurPos->top);

	} else {
		reportError(ERR_MEMORY);
	}

	// Build all our buttons
	for (i = 0; i < 2; i++) {
		_pButtons[i] = new CBofBmpButton;
		if (_pButtons[i] != nullptr) {

			CBofBitmap *pUp = loadBitmap(MakeDir(g_NavButtons[i]._pszUp), _pPal);
			CBofBitmap *pDown = loadBitmap(MakeDir(g_NavButtons[i]._pszDown), _pPal);
			CBofBitmap *pFocus = loadBitmap(MakeDir(g_NavButtons[i]._pszFocus), _pPal);
			CBofBitmap *pDis = loadBitmap(MakeDir(g_NavButtons[i]._pszDisabled), _pPal);

			_pButtons[i]->loadBitmaps(pUp, pDown, pFocus, pDis);
			_pButtons[i]->create(g_NavButtons[i]._pszName, g_NavButtons[i]._nLeft, g_NavButtons[i]._nTop, g_NavButtons[i]._nWidth, g_NavButtons[i]._nHeight, this, g_NavButtons[i]._nID);
			_pButtons[i]->show();
		} else {
			reportError(ERR_MEMORY);
			break;
		}
	}

	show();
	updateWindow();
	setTimer(777, 200, nullptr);

	BofPlaySound(MakeDir(WELCOME_SND), SOUND_WAVE);
	CSound::WaitWaveSounds();

	CBagCursor::showSystemCursor();

	_bNavAttached = true;

	return _errCode;
}


ErrorCode CNavWindow::detach() {
	assert(isValidObject(this));

	// If attached
	if (_bNavAttached) {

		// Get rid of the system cursor
		CBagCursor::hideSystemCursor();

		// One turn has gone by
		VAR_MANAGER->IncrementTimers();

		_bNavAttached = false;
	}

	logInfo("\tCNavWindow::detach()");

	CBofApp::getApp()->setPalette(_pOldPal);

	if (_pPal != nullptr) {
		delete _pPal;
		_pPal = nullptr;
	}

	if (_pPortName != nullptr) {
		delete _pPortName;
		_pPortName = nullptr;
	}

	if (_pPinna != nullptr) {
		delete _pPinna;
		_pPinna = nullptr;
	}

	if (_pHpoctyl != nullptr) {
		delete _pHpoctyl;
		_pHpoctyl = nullptr;
	}

	if (_pArmpit != nullptr) {
		delete _pArmpit;
		_pArmpit = nullptr;
	}

	if (_pYzore != nullptr) {
		delete _pYzore;
		_pYzore = nullptr;
	}

	if (_pBulbus != nullptr) {
		delete _pBulbus;
		_pBulbus = nullptr;
	}

	if (_pDingle != nullptr) {
		delete _pDingle;
		_pDingle = nullptr;
	}

	if (_pKarkas != nullptr) {
		delete _pKarkas;
		_pKarkas = nullptr;
	}

	if (_pFruufnia != nullptr) {
		delete _pFruufnia;
		_pFruufnia = nullptr;
	}

	if (_pCurPos != nullptr) {
		delete _pCurPos;
		_pCurPos = nullptr;
	}

	if (_pWilbur != nullptr) {
		delete _pWilbur;
		_pWilbur = nullptr;
	}

	if (_pMcKelvey != nullptr) {
		delete _pMcKelvey;
		_pMcKelvey = nullptr;
	}

	if (_pMedge != nullptr) {
		delete _pMedge;
		_pMedge = nullptr;
	}

	if (_pWall != nullptr) {
		delete _pWall;
		_pWall = nullptr;
	}

	if (_pWoo != nullptr) {
		delete _pWoo;
		_pWoo = nullptr;
	}

	if (_pRoman != nullptr) {
		delete _pRoman;
		_pRoman = nullptr;
	}

	if (_pWeed != nullptr) {
		delete _pWeed;
		_pWeed = nullptr;
	}

	if (_pBok != nullptr) {
		delete _pBok;
		_pBok = nullptr;
	}

	if (_pPizer != nullptr) {
		delete _pPizer;
		_pPizer = nullptr;
	}

	if (_pBackwater != nullptr) {
		delete _pBackwater;
		_pBackwater = nullptr;
	}

	if (_pUranus != nullptr) {
		delete _pUranus;
		_pUranus = nullptr;
	}

	if (_pToejam != nullptr) {
		delete _pToejam;
		_pToejam = nullptr;
	}

	if (_pHellhole != nullptr) {
		delete _pHellhole;
		_pHellhole = nullptr;
	}

	if (_pPustule != nullptr) {
		delete _pPustule;
		_pPustule = nullptr;
	}

	if (_pSlug != nullptr) {
		delete _pSlug;
		_pSlug = nullptr;
	}

	if (_pSwamprock != nullptr) {
		delete _pSwamprock;
		_pSwamprock = nullptr;
	}

	if (_pMaggotsnest != nullptr) {
		delete _pMaggotsnest;
		_pMaggotsnest = nullptr;
	}

	if (_pPeggleboz != nullptr) {
		delete _pPeggleboz;
		_pPeggleboz = nullptr;
	}

	if (_pArcheroids != nullptr) {
		delete _pArcheroids;
		_pArcheroids = nullptr;
	}

	if (_pPackRat != nullptr) {
		delete _pPackRat;
		_pPackRat = nullptr;
	}

	if (_pBeacon != nullptr) {
		delete _pBeacon;
		_pBeacon = nullptr;
	}

	if (_pMankala != nullptr) {
		delete _pMankala;
		_pMankala = nullptr;
	}

	if (_pLife != nullptr) {
		delete _pLife;
		_pLife = nullptr;
	}

	if (_pFuge != nullptr) {
		delete _pFuge;
		_pFuge = nullptr;
	}

	if (_pGarfunkel != nullptr) {
		delete _pGarfunkel;
		_pGarfunkel = nullptr;
	}

	if (_pBattlefish != nullptr) {
		delete _pBattlefish;
		_pBattlefish = nullptr;
	}

	if (_pCurLoc != nullptr) {
		delete _pCurLoc;
		_pCurLoc = nullptr;
	}

	if (_pNoVacancy != nullptr) {
		delete _pNoVacancy;
		_pNoVacancy = nullptr;
	}

	// Destroy all buttons
	for (int i = 0; i < 2; i++) {
		if (_pButtons[i] != nullptr) {
			delete _pButtons[i];
			_pButtons[i] = nullptr;
		}
	}

	// Close sprite lib
	CBofSprite::closeLibrary();

	killBackdrop();

	CBagStorageDevWnd::detach();

	return _errCode;
}


void CNavWindow::onMainLoop() {
	// Do nothing
}


void CNavWindow::onPaint(CBofRect *pRect) {
	assert(isValidObject(this));
	assert(pRect != nullptr);

	// Render offscreen
	if (_pBackdrop != nullptr) {
		CBofBitmap *pBmp = GetWorkBmp();
		if (pBmp != nullptr) {
			_pBackdrop->paint(pBmp, pRect, pRect);

			// Now update the screen
			pBmp->paint(this, pRect, pRect);
		}
	}

	setTimer(777, 200, nullptr);

	// Separated repainting of text
	refreshData();
}

void CNavWindow::refreshData() {
	assert(isValidObject(this));

	if (_pBackdrop == nullptr)
		return;

	// Render offscreen
	CBofBitmap *pBmp = GetWorkBmp();
	if (pBmp == nullptr)
		return;

	CBofRect cRect(440, 0, 639, 439);
	CBofBitmap cBmp(200, 440, _pPal);
	pBmp->paint(&cBmp, 0, 0, &cRect);

	cRect.SetRect(0, 10, 199, 25);
	paintText(&cBmp, &cRect, buildString("Current Port of call:"), 14, TEXT_NORMAL, NAVTEXT_COLOR, JUSTIFY_LEFT, FORMAT_DEFAULT, FONT_MONO);
	cRect.left += 10;       // Indent this section
	cRect.top += 15;
	cRect.bottom += 15;
	paintText(&cBmp, &cRect, buildString("%s", _pPortName->GetBuffer()), 14, TEXT_NORMAL, NAVTEXT_COLOR, JUSTIFY_LEFT, FORMAT_DEFAULT, FONT_MONO);
	cRect.left -= 10;       // Un-indent

	// Leave blank space before next section
	cRect.top += 30;
	cRect.bottom += 30;

	paintText(&cBmp, &cRect, buildString("Current Manifest:"), 14, TEXT_NORMAL, NAVTEXT_COLOR, JUSTIFY_LEFT, FORMAT_DEFAULT, FONT_MONO);

	cRect.left += 10;       // Indent this section
	char szBuf[100];
	int i = 0;
	while (_pLevel[i].Name != nullptr) {
		if (_pLevel[i].cargo._pszCargo != nullptr && _pLevel[i].cargo._bUsed) {
			cRect.top += 15;
			cRect.bottom += 15;
			if (_pLevel[i].cargo.Weight != 1)
				Common::sprintf_s(szBuf, "%3d tons of %s", _pLevel[i].cargo.Weight, _pLevel[i].cargo._pszCargo/*, _pLevel[i].Name*/);
			else
				Common::sprintf_s(szBuf, "%3d ton of %s", _pLevel[i].cargo.Weight, _pLevel[i].cargo._pszCargo/*, _pLevel[i].Name*/);
			paintText(&cBmp, &cRect, szBuf, 14, TEXT_NORMAL, NAVTEXT_COLOR, JUSTIFY_LEFT, FORMAT_DEFAULT, FONT_MONO);
			cRect.top += 15;
			cRect.bottom += 15;
			Common::sprintf_s(szBuf, "    for %s", _pLevel[i].Name);
			paintText(&cBmp, &cRect, szBuf, 14, TEXT_NORMAL, NAVTEXT_COLOR, JUSTIFY_LEFT, FORMAT_DEFAULT, FONT_MONO);
		};
		i++;
	};
	cRect.left -= 10;		// Un-indent

	// Leave blank space before next section
	cRect.top += 30;
	cRect.bottom += 30;

	paintText(&cBmp, &cRect, buildString("Current Fuel:"), 14, TEXT_NORMAL, NAVTEXT_COLOR, JUSTIFY_LEFT, FORMAT_DEFAULT, FONT_MONO);
	cRect.left += 10;       // Indent this section
	cRect.top += 15;
	cRect.bottom += 15;
	if (_fuel != 1)
		paintText(&cBmp, &cRect, buildString("%3d tons", _fuel), 14, TEXT_NORMAL, NAVTEXT_COLOR, JUSTIFY_LEFT, FORMAT_DEFAULT, FONT_MONO);
	else
		paintText(&cBmp, &cRect, buildString("%3d ton", _fuel), 14, TEXT_NORMAL, NAVTEXT_COLOR, JUSTIFY_LEFT, FORMAT_DEFAULT, FONT_MONO);
	cRect.left -= 10;       // Un-indent

	// Leave blank space before next section
	cRect.top += 30;
	cRect.bottom += 30;

	paintText(&cBmp, &cRect, buildString("Current Tonnage:"), 14, TEXT_NORMAL, NAVTEXT_COLOR, JUSTIFY_LEFT, FORMAT_DEFAULT, FONT_MONO);
	cRect.top += 15;
	cRect.bottom += 15;

	cRect.left += 10;       // Indent this section
	if (_cargo != 1)
		Common::sprintf_s(szBuf, "%3d tons Cargo", _cargo);
	else
		Common::sprintf_s(szBuf, "%3d ton Cargo", _cargo);
	paintText(&cBmp, &cRect, szBuf, 14, TEXT_NORMAL, NAVTEXT_COLOR, JUSTIFY_LEFT, FORMAT_DEFAULT, FONT_MONO);
	cRect.top += 15;
	cRect.bottom += 15;
	if (_ship != 1)
		Common::sprintf_s(szBuf, "%3d tons Ship", _ship);
	else
		Common::sprintf_s(szBuf, "%3d ton Ship", _ship);
	paintText(&cBmp, &cRect, szBuf, 14, TEXT_NORMAL, NAVTEXT_COLOR, JUSTIFY_LEFT, FORMAT_DEFAULT, FONT_MONO);
	cRect.top += 15;
	cRect.bottom += 15;
	if (_fuel != 1)
		Common::sprintf_s(szBuf, "%3d tons Fuel", _fuel);
	else
		Common::sprintf_s(szBuf, "%3d ton Fuel", _fuel);
	paintText(&cBmp, &cRect, szBuf, 14, TEXT_NORMAL, NAVTEXT_COLOR, JUSTIFY_LEFT, FORMAT_DEFAULT, FONT_MONO);
	cRect.top += 15;
	cRect.bottom += 15;
	Common::sprintf_s(szBuf, "%3d tons TOTAL", _ship + _fuel + _cargo);
	paintText(&cBmp, &cRect, szBuf, 14, TEXT_NORMAL, NAVTEXT_COLOR, JUSTIFY_LEFT, FORMAT_DEFAULT, FONT_MONO);
	cRect.left -= 10;       // Un-indent

	cRect.top += 30;
	cRect.bottom += 30;
	Common::sprintf_s(szBuf, "Simulation Level:");
	paintText(&cBmp, &cRect, szBuf, 14, TEXT_NORMAL, NAVTEXT_COLOR, JUSTIFY_LEFT, FORMAT_DEFAULT, FONT_MONO);
	cRect.left += 10;       // Indent
	cRect.top += 15;
	cRect.bottom += 15;
	Common::sprintf_s(szBuf, "%s", g_LevelTitle[_level]);
	paintText(&cBmp, &cRect, szBuf, 14, TEXT_NORMAL, NAVTEXT_COLOR, JUSTIFY_LEFT, FORMAT_DEFAULT, FONT_MONO);
	cRect.left -= 10;       // Un-indent
	cBmp.paint(this, 443, 0);
}

void CNavWindow::onBofButton(CBofObject *pObject, int nState) {
	assert(isValidObject(this));
	assert(pObject != nullptr);

	if (nState != BUTTON_CLICKED)
		return;

	CBofButton *pButton = (CBofButton *)pObject;
	switch (pButton->getControlID()) {
	case QUIT: {
		logInfo("\tClicked Quit");
		VAR_MANAGER->GetVariable("NPLAYEDNAV")->SetBoolValue(true);
		close();
		break;
	}
	case HELP: {
		logInfo("\tClicked Help");

		killTimer(777);
		CBagel *pApp = CBagel::getBagApp();
		if (pApp != nullptr) {
			CBagMasterWin *pWin = pApp->getMasterWnd();
			if (pWin != nullptr) {
				pWin->onHelp(MakeDir("NAVHELP.TXT"));
			}
		}
		break;
	}

	default:
		logWarning(buildString("Clicked Unknown Button with ID %d", pButton->getControlID()));
		break;
	}

}


void CNavWindow::onLButtonDown(uint32 /*nFlags*/, CBofPoint *pPoint, void *) {
	assert(isValidObject(this));
	assert(pPoint != nullptr);

	switch (_level) {
	case 0: {
		if (_pPinna->PtInRect(*pPoint))
			onPinna();

		if (_pHpoctyl->PtInRect(*pPoint))
			onHpoctyl();

		if (_pArmpit->PtInRect(*pPoint))
			onArmpit();

		if (_pYzore->PtInRect(*pPoint))
			onYzore();

		if (_pBulbus->PtInRect(*pPoint))
			onBulbus();

		if (_pDingle->PtInRect(*pPoint))
			onDingle();

		if (_pKarkas->PtInRect(*pPoint))
			onKarkas();

		if (_pFruufnia->PtInRect(*pPoint))
			onFruufnia();
		break;
	}//close case

	case 1: {
		if (_pWilbur->PtInRect(*pPoint))
			onWilbur();

		if (_pMcKelvey->PtInRect(*pPoint))
			onMcKelvey();

		if (_pMedge->PtInRect(*pPoint))
			onMedge();

		if (_pWall->PtInRect(*pPoint))
			onWall();

		if (_pWoo->PtInRect(*pPoint))
			onWoo();

		if (_pRoman->PtInRect(*pPoint))
			onRoman();

		if (_pWeed->PtInRect(*pPoint))
			onWeed();

		if (_pBok->PtInRect(*pPoint))
			onBok();

		if (_pPizer->PtInRect(*pPoint))
			onPizer();

		break;
	}//close case

	case 2: {
		if (_pBackwater->PtInRect(*pPoint))
			onBackwater();

		if (_pUranus->PtInRect(*pPoint))
			onUranus();

		if (_pToejam->PtInRect(*pPoint))
			onToejam();

		if (_pHellhole->PtInRect(*pPoint))
			onHellhole();

		if (_pPustule->PtInRect(*pPoint))
			onPustule();

		if (_pSwamprock->PtInRect(*pPoint))
			onSwamprock();

		if (_pSlug->PtInRect(*pPoint))
			onSlug();

		if (_pMaggotsnest->PtInRect(*pPoint))
			onMaggotsnest();

		break;
	}
	case 3: {
		if (_pPeggleboz->PtInRect(*pPoint))
			onPeggleboz();

		if (_pNoVacancy->PtInRect(*pPoint))
			onNoVacancy();

		if (_pArcheroids->PtInRect(*pPoint))
			onArcheroids();

		if (_pPackRat->PtInRect(*pPoint))
			onPackRat();

		if (_pBeacon->PtInRect(*pPoint))
			onBeacon();

		if (_pMankala->PtInRect(*pPoint))
			onMankala();

		if (_pLife->PtInRect(*pPoint))
			onLife();

		if (_pFuge->PtInRect(*pPoint))
			onFuge();

		if (_pGarfunkel->PtInRect(*pPoint))
			onGarfunkel();

		if (_pBattlefish->PtInRect(*pPoint))
			onBattlefish();

		break;
	} //close case
	} //close switch

	if (_cargo <= 0)
		calcFuel(0);
}


void CNavWindow::onKeyHit(uint32 lKey, uint32 /*lRepCount*/) {
	assert(isValidObject(this));

	if (lKey == BKEY_ALT_q || lKey == BKEY_ALT_F4) {
		VAR_MANAGER->GetVariable("NPLAYEDNAV")->SetBoolValue(true);
		close();
	}
}

void CNavWindow::onTimer(uint32 tId) {
	if (tId == 777) {
		_pCurLoc->paintSprite(this, _pCurPos->left, _pCurPos->top);
	}
}

void CNavWindow::onPinna() {
	if (*_pCurPos == *_pPinna) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pPinna);
		*_pPortName = "Pinna";
		calcFuel(0);
	} else if (*_pCurPos == *_pHpoctyl) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pPinna);
		*_pPortName = "Pinna";
		calcFuel(1.6);
	} else if (*_pCurPos == *_pArmpit) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pPinna);
		*_pPortName = "Pinna";
		calcFuel(1.5);
	} else {
		bofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::onHpoctyl() {
	if (*_pCurPos == *_pHpoctyl) {

		*_pPortName = "H'poctyl";
		if (g_Level[1].cargo._pszCargo != nullptr && g_Level[1].cargo._bUsed) {
			g_Level[1].cargo._bUsed = false;
			_cargo -= 72;
		}
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pHpoctyl);
		calcFuel(0);
	} else if (*_pCurPos == *_pArmpit) {
		*_pPortName = "H'poctyl";
		if (g_Level[1].cargo._pszCargo != nullptr && g_Level[1].cargo._bUsed) {
			g_Level[1].cargo._bUsed = false;
			_cargo -= 72;
		}
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pHpoctyl);
		calcFuel(3.2);
	} else if (*_pCurPos == *_pPinna) {
		*_pPortName = "H'poctyl";
		if (g_Level[1].cargo._pszCargo != nullptr && g_Level[1].cargo._bUsed) {
			g_Level[1].cargo._bUsed = false;
			_cargo -= 72;
		}
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pHpoctyl);
		calcFuel(1.6);
	} else if (*_pCurPos == *_pDingle) {
		*_pPortName = "H'poctyl";
		if (g_Level[1].cargo._pszCargo != nullptr && g_Level[1].cargo._bUsed) {
			g_Level[1].cargo._bUsed = false;
			_cargo -= 72;
		}
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pHpoctyl);
		calcFuel(2.4);
	} else if (*_pCurPos == *_pBulbus) {
		*_pPortName = "H'poctyl";
		if (g_Level[1].cargo._pszCargo != nullptr && g_Level[1].cargo._bUsed) {
			g_Level[1].cargo._bUsed = false;
			_cargo -= 72;
		}
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pHpoctyl);
		calcFuel(2.8);
	} else {
		bofMessageBox("Destination unavailable !!", "Test Warning");
	}
}


void CNavWindow::onArmpit() {
	if (*_pCurPos == *_pArmpit) {

		delete _pCurPos;
		_pCurPos = new CBofRect(*_pArmpit);
		*_pPortName = "Armpit";
		if (g_Level[2].cargo._pszCargo != nullptr && g_Level[2].cargo._bUsed) {
			g_Level[2].cargo._bUsed = false;
			_cargo -= 56;
		}

	} else if (*_pCurPos == *_pHpoctyl) {

		delete _pCurPos;
		_pCurPos = new CBofRect(*_pArmpit);
		*_pPortName = "Armpit";
		if (g_Level[2].cargo._pszCargo != nullptr && g_Level[2].cargo._bUsed) {
			g_Level[2].cargo._bUsed = false;
			_cargo -= 56;
		}
		calcFuel(3.2);
	} else if (*_pCurPos == *_pPinna) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pArmpit);
		*_pPortName = "Armpit";
		if (g_Level[2].cargo._pszCargo != nullptr && g_Level[2].cargo._bUsed) {
			g_Level[2].cargo._bUsed = false;
			_cargo -= 56;
		}
		calcFuel(1.5);
	} else if (*_pCurPos == *_pKarkas) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pArmpit);
		*_pPortName = "Armpit";
		if (g_Level[2].cargo._pszCargo != nullptr && g_Level[2].cargo._bUsed) {
			g_Level[2].cargo._bUsed = false;
			_cargo -= 56;
		}
		calcFuel(2.3);
	} else {
		bofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::onYzore() {
	if (*_pCurPos == *_pYzore) {
		delete _pCurPos;
		_pCurPos = nullptr;
		*_pPortName = "Yzore";
		_pCurPos = new CBofRect(*_pYzore);
	} else if (*_pCurPos == *_pKarkas) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pYzore);
		*_pPortName = "Yzore";
		calcFuel(2.6);
	} else if (*_pCurPos == *_pFruufnia) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pYzore);
		*_pPortName = "Yzore";
		calcFuel(4.0);
	} else if (*_pCurPos == *_pBulbus) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pYzore);
		*_pPortName = "Yzore";
		calcFuel(1.2);
	} else {
		bofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::onBulbus() {
	if (*_pCurPos == *_pBulbus) {
		delete _pCurPos;
		_pCurPos = nullptr;
		*_pPortName = "Bulbus";
		_pCurPos = new CBofRect(*_pBulbus);
	} else if (*_pCurPos == *_pHpoctyl) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pBulbus);
		*_pPortName = "Bulbus";
		calcFuel(2.8);
	} else if (*_pCurPos == *_pYzore) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pBulbus);
		*_pPortName = "Bulbus";
		calcFuel(1.2);
	} else if (*_pCurPos == *_pDingle) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pBulbus);
		*_pPortName = "Bulbus";
		calcFuel(1.9);
	} else if (*_pCurPos == *_pFruufnia) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pBulbus);
		*_pPortName = "Bulbus";
		calcFuel(3.7);
	} else {
		bofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::onDingle() {
	if (*_pCurPos == *_pDingle) {
		delete _pCurPos;
		_pCurPos = nullptr;
		*_pPortName = "Dingle";
		_pCurPos = new CBofRect(*_pDingle);
	} else if (*_pCurPos == *_pHpoctyl) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pDingle);
		*_pPortName = "Dingle";
		calcFuel(2.4);
	} else if (*_pCurPos == *_pBulbus) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pDingle);
		*_pPortName = "Dingle";
		calcFuel(1.9);
	} else {
		bofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::onKarkas() {
	if (*_pCurPos == *_pKarkas) {
		delete _pCurPos;
		_pCurPos = nullptr;
		*_pPortName = "Karkas";
		_pCurPos = new CBofRect(*_pKarkas);
	} else if (*_pCurPos == *_pArmpit) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pKarkas);
		*_pPortName = "Karkas";
		calcFuel(2.3);
	} else if (*_pCurPos == *_pYzore) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pKarkas);
		*_pPortName = "Karkas";
		calcFuel(2.6);
	} else if (*_pCurPos == *_pFruufnia) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pKarkas);
		*_pPortName = "Karkas";
		calcFuel(1.7);
	} else {
		bofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::onFruufnia() {
	if (*_pCurPos == *_pFruufnia) {
		delete _pCurPos;
		*_pPortName = "Fruufnia";
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pFruufnia);
		if (g_Level[7].cargo._pszCargo != nullptr && g_Level[7].cargo._bUsed) {
			g_Level[7].cargo._bUsed = false;
			_cargo -= 86;
		}
	} else if (*_pCurPos == *_pKarkas) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pFruufnia);
		*_pPortName = "Fruufnia";
		if (g_Level[7].cargo._pszCargo != nullptr && g_Level[7].cargo._bUsed) {
			g_Level[7].cargo._bUsed = false;
			_cargo -= 86;
		}
		calcFuel(1.7);
	} else if (*_pCurPos == *_pYzore) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pFruufnia);
		*_pPortName = "Fruufnia";
		if (g_Level[7].cargo._pszCargo != nullptr && g_Level[7].cargo._bUsed) {
			g_Level[7].cargo._bUsed = false;
			_cargo -= 86;
		}
		calcFuel(4.0);
	} else if (*_pCurPos == *_pBulbus) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pFruufnia);
		*_pPortName = "Fruufnia";
		if (g_Level[7].cargo._pszCargo != nullptr && g_Level[7].cargo._bUsed) {
			g_Level[7].cargo._bUsed = false;
			_cargo -= 86;
		}
		calcFuel(3.7);
	} else {
		bofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::onWilbur() {
	if (*_pCurPos == *_pWilbur) {
		delete _pCurPos;
		_pCurPos = nullptr;
		*_pPortName = "Wilbur";
		_pCurPos = new CBofRect(*_pWilbur);
	} else if (*_pCurPos == *_pWall) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pWilbur);
		*_pPortName = "Wilbur";
		calcFuel(1.6);
	} else if (*_pCurPos == *_pMcKelvey) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pWilbur);
		*_pPortName = "Wilbur";
		calcFuel(3.6);
	} else if (*_pCurPos == *_pMedge) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pWilbur);
		*_pPortName = "Wilbur";
		calcFuel(2.1);

	} else {
		bofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::onMcKelvey() {
	if (*_pCurPos == *_pMcKelvey) {
		delete _pCurPos;
		_pCurPos = nullptr;
		*_pPortName = "McKelvey";
		_pCurPos = new CBofRect(*_pMcKelvey);
	} else if (*_pCurPos == *_pWilbur) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pMcKelvey);
		*_pPortName = "McKelvey";
		calcFuel(3.6);
	} else if (*_pCurPos == *_pWall) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pMcKelvey);
		*_pPortName = "McKelvey";
		calcFuel(5.3);
	} else if (*_pCurPos == *_pWeed) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pMcKelvey);
		*_pPortName = "McKelvey";
		calcFuel(2.7);
	} else {
		bofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::onMedge() {
	if (*_pCurPos == *_pMedge) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pMedge);
		*_pPortName = "Medge";
		if (g_LevelOne[2].cargo._pszCargo != nullptr && g_LevelOne[2].cargo._bUsed) {
			g_LevelOne[2].cargo._bUsed = false;
			_cargo -= 127;
		}
	} else if (*_pCurPos == *_pWilbur) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pMedge);
		*_pPortName = "Medge";
		if (g_LevelOne[2].cargo._pszCargo != nullptr && g_LevelOne[2].cargo._bUsed) {
			g_LevelOne[2].cargo._bUsed = false;
			_cargo -= 127;
		}
		calcFuel(2.1);
	} else if (*_pCurPos == *_pWall) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pMedge);
		*_pPortName = "Medge";
		if (g_LevelOne[2].cargo._pszCargo != nullptr && g_LevelOne[2].cargo._bUsed) {
			g_LevelOne[2].cargo._bUsed = false;
			_cargo -= 127;
		}
		calcFuel(1.2);
	} else {
		bofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::onWall() {
	if (*_pCurPos == *_pWall) {
		delete _pCurPos;
		_pCurPos = nullptr;
		*_pPortName = "Wall";
		_pCurPos = new CBofRect(*_pWall);
	} else if (*_pCurPos == *_pMcKelvey) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pWall);
		*_pPortName = "Wall";
		calcFuel(5.3);
	} else if (*_pCurPos == *_pWeed) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pWall);
		*_pPortName = "Wall";
		calcFuel(1.4);
	} else if (*_pCurPos == *_pWilbur) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pWall);
		*_pPortName = "Wall";
		calcFuel(1.6);
	} else if (*_pCurPos == *_pMedge) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pWall);
		*_pPortName = "Wall";
		calcFuel(1.2);
	} else if (*_pCurPos == *_pRoman) {
		delete _pCurPos;
		_pCurPos = nullptr;
		*_pPortName = "Wall";
		_pCurPos = new CBofRect(*_pWall);
		calcFuel(1.9);
	} else {
		bofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::onWoo() {
	if (*_pCurPos == *_pWoo) {
		delete _pCurPos;
		_pCurPos = nullptr;
		*_pPortName = "Woo";
		_pCurPos = new CBofRect(*_pWoo);
	} else if (*_pCurPos == *_pRoman) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pWoo);
		*_pPortName = "Woo";
		calcFuel(2.7);
	} else if (*_pCurPos == *_pBok) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pWoo);
		*_pPortName = "Woo";
		calcFuel(3.9);
	} else if (*_pCurPos == *_pPizer) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pWoo);
		*_pPortName = "Woo";
		calcFuel(1.7);
	} else {
		bofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::onRoman() {
	if (*_pCurPos == *_pRoman) {
		delete _pCurPos;
		_pCurPos = nullptr;
		*_pPortName = "Roman";
		_pCurPos = new CBofRect(*_pRoman);
		if (g_LevelOne[5].cargo._pszCargo != nullptr && g_LevelOne[5].cargo._bUsed) {
			g_LevelOne[5].cargo._bUsed = false;
			_cargo -= 20;
		}
	} else if (*_pCurPos == *_pWall) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pRoman);
		*_pPortName = "Roman";
		if (g_LevelOne[5].cargo._pszCargo != nullptr && g_LevelOne[5].cargo._bUsed) {
			g_LevelOne[5].cargo._bUsed = false;
			_cargo -= 20;
		}
		calcFuel(1.9);
	} else if (*_pCurPos == *_pWoo) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pRoman);
		*_pPortName = "Roman";
		if (g_LevelOne[5].cargo._pszCargo != nullptr && g_LevelOne[5].cargo._bUsed) {
			g_LevelOne[5].cargo._bUsed = false;
			_cargo -= 20;
		}
		calcFuel(2.7);
	} else if (*_pCurPos == *_pPizer) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pRoman);
		*_pPortName = "Roman";
		if (g_LevelOne[5].cargo._pszCargo != nullptr && g_LevelOne[5].cargo._bUsed) {
			g_LevelOne[5].cargo._bUsed = false;
			_cargo -= 20;
		}
		calcFuel(4.6);
	} else {
		bofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::pause() {
	CursorMan.showMouse(false);

	// Pause for four seconds
	EventLoop eventLoop;
	uint32 expiry = g_system->getMillis() + 4000;

	while (g_system->getMillis() < expiry && !eventLoop.frame()) {
	}

	CursorMan.showMouse(true);
}

void CNavWindow::calcFuel(double hf) {
	assert(isValidObject(this));
	CBofRect cRect(0, 0, 439, 439);

	_fuel -= (int)((_ship + _fuel + _cargo) * hf * .01);

	if (_cargo == 0) {
		_pCurLoc->eraseSprite(this);
		refreshData();
		killTimer(777);
		_pCurLoc->setCel(0);
		_pCurLoc->paintSprite(this, _pCurPos->left, _pCurPos->top);

		// WORKAROUND: _pBackdrop shares it's palette with _pCurLoc,
		// so as the backdrop is changed, don't free the palette
		_pBackdrop->setIsOwnPalette(false);
		bool isDone = (_level == 3);

		if (_level == 3) {
			VAR_MANAGER->GetVariable("NPASSEDTEST")->SetBoolValue(true);
			VAR_MANAGER->GetVariable("NPLAYEDNAV")->SetBoolValue(true);
			close();
		}

		if (_level == 2) {
			pause();
			CBofString sNebDir(NEBSIM4BMP);
			MACROREPLACE(sNebDir);
			assert(_pBackdrop != nullptr);
			bmptwo = new CBofBitmap(sNebDir.GetBuffer(), _pPal);
			setBackground(bmptwo);
			_cargo = 125 + 10 + 17 + 8 + 99 + 24;
			_ship = 65;
			_fuel = 45;
			delete _pCurPos;
			_pCurPos = nullptr;
			_pCurPos = new CBofRect(*_pMankala);
			_level = 3;
			_pLevel = g_LevelThree;
			*_pPortName = "Mankala";
		}

		if (_level == 1) {
			pause();
			CBofString sNebDir(NEBSIM3BMP);
			MACROREPLACE(sNebDir);
			assert(_pBackdrop != nullptr);
			bmptwo = new CBofBitmap(sNebDir.GetBuffer(), _pPal);
			setBackground(bmptwo);
			_cargo = 100 + 75 + 28 + 45 + 14;
			_ship = 99;
			_fuel = 36;
			delete _pCurPos;
			_pCurPos = nullptr;
			_pCurPos = new CBofRect(*_pMaggotsnest);
			_level = 2;
			_pLevel = g_LevelTwo;
			*_pPortName = "Maggot's Nest";
		}

		if (_level == 0) {
			pause();
			CBofString sNebDir(NEBSIM2BMP);
			MACROREPLACE(sNebDir);
			assert(_pBackdrop != nullptr);
			bmptwo = new CBofBitmap(sNebDir.GetBuffer(), _pPal);
			setBackground(bmptwo);
			_cargo = 54 + 119 + 20 + 127;
			_ship = 120;
			_fuel = 75;
			delete _pCurPos;
			_pCurPos = nullptr;
			_pCurPos = new CBofRect(*_pMcKelvey);
			_level = 1;
			_pLevel = g_LevelOne;
			*_pPortName = "McKelvey";
		}

		invalidateRect(&cRect);
		if (!isDone)
			updateWindow();

		// WORKAROUND: Restore owns palette flag back again
		_pBackdrop->setIsOwnPalette(true);
	}

	if (_fuel <= 0) {
		VAR_MANAGER->GetVariable("NPLAYEDNAV")->SetBoolValue(true);
		killTimer(777);
		close();
	}

	_pCurLoc->eraseSprite(this);
	refreshData();
	_pCurLoc->paintSprite(this, _pCurPos->left, _pCurPos->top);
}

void CNavWindow::onWeed() {
	if (*_pCurPos == *_pWeed) {
		delete _pCurPos;
		_pCurPos = nullptr;
		*_pPortName = "Weed";
		_pCurPos = new CBofRect(*_pWeed);
	} else if (*_pCurPos == *_pMcKelvey) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pWeed);
		*_pPortName = "Weed";
		calcFuel(2.7);
	} else if (*_pCurPos == *_pWall) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pWeed);
		*_pPortName = "Weed";
		calcFuel(1.4);
	} else if (*_pCurPos == *_pPizer) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pWeed);
		*_pPortName = "Weed";
		calcFuel(2.2);
	} else {
		bofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::onBok() {
	if (*_pCurPos == *_pBok) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pBok);
		*_pPortName = "Bok";
		if (g_LevelOne[7].cargo._pszCargo != nullptr && g_LevelOne[7].cargo._bUsed) {
			g_LevelOne[7].cargo._bUsed = false;
			_cargo -= 119;
		}
	} else if (*_pCurPos == *_pWoo) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pBok);
		*_pPortName = "Bok";
		if (g_LevelOne[7].cargo._pszCargo != nullptr && g_LevelOne[7].cargo._bUsed) {
			g_LevelOne[7].cargo._bUsed = false;
			_cargo -= 119;
		}
		calcFuel(3.9);
	} else {
		bofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::onPizer() {
	if (*_pCurPos == *_pPizer) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pPizer);
		*_pPortName = "Pizer";
		if (g_LevelOne[8].cargo._pszCargo != nullptr && g_LevelOne[8].cargo._bUsed) {
			g_LevelOne[8].cargo._bUsed = false;
			_cargo -= 54;
		}
	} else if (*_pCurPos == *_pWoo) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pPizer);
		*_pPortName = "Pizer";
		if (g_LevelOne[8].cargo._pszCargo != nullptr && g_LevelOne[8].cargo._bUsed) {
			g_LevelOne[8].cargo._bUsed = false;
			_cargo -= 54;
		}
		calcFuel(1.7);
	} else if (*_pCurPos == *_pRoman) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pPizer);
		*_pPortName = "Pizer";
		if (g_LevelOne[8].cargo._pszCargo != nullptr && g_LevelOne[8].cargo._bUsed) {
			g_LevelOne[8].cargo._bUsed = false;
			_cargo -= 54;
		}
		calcFuel(4.6);
	} else if (*_pCurPos == *_pWeed) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pPizer);
		*_pPortName = "Pizer";
		if (g_LevelOne[8].cargo._pszCargo != nullptr && g_LevelOne[8].cargo._bUsed) {
			g_LevelOne[8].cargo._bUsed = false;
			_cargo -= 54;
		}
		calcFuel(2.2);
	} else {
		bofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::onBackwater() {
	if (*_pCurPos == *_pBackwater) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pBackwater);
		*_pPortName = "Backwater";
		if (g_LevelTwo[0].cargo._pszCargo != nullptr && g_LevelTwo[0].cargo._bUsed) {
			g_LevelTwo[0].cargo._bUsed = false;
			_cargo -= 28;
		}
		calcFuel(0);
	} else if (*_pCurPos == *_pUranus) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pBackwater);
		*_pPortName = "Backwater";
		if (g_LevelTwo[0].cargo._pszCargo != nullptr && g_LevelTwo[0].cargo._bUsed) {
			g_LevelTwo[0].cargo._bUsed = false;
			_cargo -= 28;
		}
		calcFuel(4.5);
	} else {
		bofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::onUranus() {
	if (*_pCurPos == *_pUranus) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pUranus);
		*_pPortName = "Uranus";
		if (g_LevelTwo[1].cargo._pszCargo != nullptr && g_LevelTwo[1].cargo._bUsed) {
			g_LevelTwo[1].cargo._bUsed = false;
			_cargo -= 14;
		}
		calcFuel(0);
	} else if (*_pCurPos == *_pBackwater) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pUranus);
		*_pPortName = "Uranus";
		if (g_LevelTwo[1].cargo._pszCargo != nullptr && g_LevelTwo[1].cargo._bUsed) {
			g_LevelTwo[1].cargo._bUsed = false;
			_cargo -= 14;
		}
		calcFuel(4.5);
	} else if (*_pCurPos == *_pToejam) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pUranus);
		*_pPortName = "Uranus";
		if (g_LevelTwo[1].cargo._pszCargo != nullptr && g_LevelTwo[1].cargo._bUsed) {
			g_LevelTwo[1].cargo._bUsed = false;
			_cargo -= 14;
		}
		calcFuel(2.6);
	} else if (*_pCurPos == *_pPustule) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pUranus);
		*_pPortName = "Uranus";
		if (g_LevelTwo[1].cargo._pszCargo != nullptr && g_LevelTwo[1].cargo._bUsed) {
			g_LevelTwo[1].cargo._bUsed = false;
			_cargo -= 14;
		}
		calcFuel(2.9);
	} else {
		bofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::onToejam() {
	if (*_pCurPos == *_pToejam) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pToejam);
		*_pPortName = "Toe Jam";
		if (g_LevelTwo[2].cargo._pszCargo != nullptr && g_LevelTwo[2].cargo._bUsed) {
			g_LevelTwo[2].cargo._bUsed = false;
			_cargo -= 45;
		}
		calcFuel(0);
	} else if (*_pCurPos == *_pUranus) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pToejam);
		*_pPortName = "Toe Jam";
		if (g_LevelTwo[2].cargo._pszCargo != nullptr && g_LevelTwo[2].cargo._bUsed) {
			g_LevelTwo[2].cargo._bUsed = false;
			_cargo -= 45;
		}
		calcFuel(2.6);
	} else if (*_pCurPos == *_pHellhole) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pToejam);
		*_pPortName = "Toe Jam";
		if (g_LevelTwo[2].cargo._pszCargo != nullptr && g_LevelTwo[2].cargo._bUsed) {
			g_LevelTwo[2].cargo._bUsed = false;
			_cargo -= 45;
		}
		calcFuel(2.0);
	} else if (*_pCurPos == *_pPustule) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pToejam);
		*_pPortName = "Toe Jam";
		if (g_LevelTwo[2].cargo._pszCargo != nullptr && g_LevelTwo[2].cargo._bUsed) {
			g_LevelTwo[2].cargo._bUsed = false;
			_cargo -= 45;
		}
		calcFuel(1.1);
	} else if (*_pCurPos == *_pSwamprock) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pToejam);
		*_pPortName = "Toe Jam";
		if (g_LevelTwo[2].cargo._pszCargo != nullptr && g_LevelTwo[2].cargo._bUsed) {
			g_LevelTwo[2].cargo._bUsed = false;
			_cargo -= 45;
		}
		calcFuel(1.8);
	} else {
		bofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::onHellhole() {
	if (*_pCurPos == *_pHellhole) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pHellhole);
		*_pPortName = "Hellhole";
		if (g_LevelTwo[3].cargo._pszCargo != nullptr && g_LevelTwo[3].cargo._bUsed) {
			g_LevelTwo[3].cargo._bUsed = false;
			_cargo -= 100;
		}
		calcFuel(1.8);
	} else if (*_pCurPos == *_pToejam) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pHellhole);
		*_pPortName = "Hellhole";
		if (g_LevelTwo[3].cargo._pszCargo != nullptr && g_LevelTwo[3].cargo._bUsed) {
			g_LevelTwo[3].cargo._bUsed = false;
			_cargo -= 100;
		}
		calcFuel(2.0);
	} else if (*_pCurPos == *_pSwamprock) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pHellhole);
		*_pPortName = "Hellhole";
		if (g_LevelTwo[3].cargo._pszCargo != nullptr && g_LevelTwo[3].cargo._bUsed) {
			g_LevelTwo[3].cargo._bUsed = false;
			_cargo -= 100;
		}
		calcFuel(1.3);
	} else {
		bofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::onPustule() {
	if (*_pCurPos == *_pPustule) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pPustule);
		*_pPortName = "Pustule";
		calcFuel(0);
	} else if (*_pCurPos == *_pUranus) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pPustule);
		*_pPortName = "Pustule";
		calcFuel(2.9);
	} else if (*_pCurPos == *_pToejam) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pPustule);
		*_pPortName = "Pustule";
		calcFuel(1.1);
	} else if (*_pCurPos == *_pSlug) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pPustule);
		*_pPortName = "Pustule";
		calcFuel(3.4);
	} else if (*_pCurPos == *_pMaggotsnest) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pPustule);
		*_pPortName = "Pustule";
		calcFuel(2.3);
	} else {
		bofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::onSwamprock() {
	if (*_pCurPos == *_pSwamprock) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pSwamprock);
		*_pPortName = "Swamp Rock";
		calcFuel(0);
	} else if (*_pCurPos == *_pToejam) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pSwamprock);
		*_pPortName = "Swamp Rock";
		calcFuel(1.8);
	} else if (*_pCurPos == *_pHellhole) {
		delete _pCurPos;
		_pCurPos = nullptr;
		*_pPortName = "Swamp Rock";
		_pCurPos = new CBofRect(*_pSwamprock);
		calcFuel(1.3);
	} else if (*_pCurPos == *_pMaggotsnest) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pSwamprock);
		*_pPortName = "Swamp Rock";
		calcFuel(1.9);

	} else {
		bofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::onSlug() {
	if (*_pCurPos == *_pSlug) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pSlug);
		*_pPortName = "Slug";
		if (g_LevelTwo[6].cargo._pszCargo != nullptr && g_LevelTwo[6].cargo._bUsed) {
			g_LevelTwo[6].cargo._bUsed = false;
			_cargo -= 75;
		}
		calcFuel(0);
	} else if (*_pCurPos == *_pPustule) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pSlug);
		*_pPortName = "Slug";
		if (g_LevelTwo[6].cargo._pszCargo != nullptr && g_LevelTwo[6].cargo._bUsed) {
			g_LevelTwo[6].cargo._bUsed = false;
			_cargo -= 75;
		}
		calcFuel(3.4);
	} else if (*_pCurPos == *_pMaggotsnest) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pSlug);
		*_pPortName = "Slug";
		if (g_LevelTwo[6].cargo._pszCargo != nullptr && g_LevelTwo[6].cargo._bUsed) {
			g_LevelTwo[6].cargo._bUsed = false;
			_cargo -= 75;
		}
		calcFuel(1.7);
	} else {
		bofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::onMaggotsnest() {
	if (*_pCurPos == *_pMaggotsnest) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pMaggotsnest);
		*_pPortName = "Maggot's Nest";
		calcFuel(0);
	} else if (*_pCurPos == *_pPustule) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pMaggotsnest);
		*_pPortName = "Maggot's Nest";
		calcFuel(2.3);
	} else if (*_pCurPos == *_pSwamprock) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pMaggotsnest);
		*_pPortName = "Maggot's Nest";
		calcFuel(1.9);
	} else if (*_pCurPos == *_pSlug) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pMaggotsnest);
		*_pPortName = "Maggot's Nest";
		calcFuel(1.7);

	} else {
		bofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::onPeggleboz() {
	if (*_pCurPos == *_pPeggleboz) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pPeggleboz);
		*_pPortName = "Peggleboz";
		if (g_LevelThree[0].cargo._pszCargo != nullptr && g_LevelThree[0].cargo._bUsed) {
			g_LevelThree[0].cargo._bUsed = false;
			_cargo -= 99;
		}
		calcFuel(0);
	} else if (*_pCurPos == *_pArcheroids) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pPeggleboz);
		*_pPortName = "Peggleboz";
		if (g_LevelThree[0].cargo._pszCargo != nullptr && g_LevelThree[0].cargo._bUsed) {
			g_LevelThree[0].cargo._bUsed = false;
			_cargo -= 99;
		}
		calcFuel(4.7);
	} else if (*_pCurPos == *_pNoVacancy) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pPeggleboz);
		*_pPortName = "Peggleboz";
		if (g_LevelThree[0].cargo._pszCargo != nullptr && g_LevelThree[0].cargo._bUsed) {
			g_LevelThree[0].cargo._bUsed = false;
			_cargo -= 99;
		}
		calcFuel(2.4);
	} else {
		bofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::onArcheroids() {
	if (*_pCurPos == *_pArcheroids) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pArcheroids);
		*_pPortName = "Archeroids";
		if (g_LevelThree[1].cargo._pszCargo != nullptr && g_LevelThree[1].cargo._bUsed) {
			g_LevelThree[1].cargo._bUsed = false;
			_cargo -= 8;
		}
		calcFuel(0);
	} else if (*_pCurPos == *_pPeggleboz) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pArcheroids);
		*_pPortName = "Archeroids";
		if (g_LevelThree[1].cargo._pszCargo != nullptr && g_LevelThree[1].cargo._bUsed) {
			g_LevelThree[1].cargo._bUsed = false;
			_cargo -= 8;
		}
		calcFuel(4.7);
	} else if (*_pCurPos == *_pPackRat) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pArcheroids);
		*_pPortName = "Archeroids";
		if (g_LevelThree[1].cargo._pszCargo != nullptr && g_LevelThree[1].cargo._bUsed) {
			g_LevelThree[1].cargo._bUsed = false;
			_cargo -= 8;
		}
		calcFuel(3.3);
	} else if (*_pCurPos == *_pMankala) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pArcheroids);
		*_pPortName = "Archeroids";
		if (g_LevelThree[1].cargo._pszCargo != nullptr && g_LevelThree[1].cargo._bUsed) {
			g_LevelThree[1].cargo._bUsed = false;
			_cargo -= 8;
		}
		calcFuel(2.6);
	} else if (*_pCurPos == *_pBeacon) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pArcheroids);
		*_pPortName = "Archeroids";
		if (g_LevelThree[1].cargo._pszCargo != nullptr && g_LevelThree[1].cargo._bUsed) {
			g_LevelThree[1].cargo._bUsed = false;
			_cargo -= 8;
		}
		calcFuel(1.2);
	} else {
		bofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::onPackRat() {
	if (*_pCurPos == *_pPackRat) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pPackRat);
		*_pPortName = "Pack-Rat";
		if (g_LevelThree[2].cargo._pszCargo != nullptr && g_LevelThree[2].cargo._bUsed) {
			g_LevelThree[2].cargo._bUsed = false;
			_cargo -= 24;
		}
		calcFuel(0);
	} else if (*_pCurPos == *_pArcheroids) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pPackRat);
		*_pPortName = "Pack-Rat";
		if (g_LevelThree[2].cargo._pszCargo != nullptr && g_LevelThree[2].cargo._bUsed) {
			g_LevelThree[2].cargo._bUsed = false;
			_cargo -= 24;
		}
		calcFuel(3.3);
	} else if (*_pCurPos == *_pBeacon) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pPackRat);
		*_pPortName = "Pack-Rat";
		if (g_LevelThree[2].cargo._pszCargo != nullptr && g_LevelThree[2].cargo._bUsed) {
			g_LevelThree[2].cargo._bUsed = false;
			_cargo -= 24;
		}
		calcFuel(5.2);
	} else {
		bofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::onBeacon() {
	if (*_pCurPos == *_pBeacon) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pBeacon);
		*_pPortName = "Beacon";
		if (g_LevelThree[3].cargo._pszCargo != nullptr && g_LevelThree[3].cargo._bUsed) {
			g_LevelThree[3].cargo._bUsed = false;
			_cargo -= 125;
		}
		calcFuel(0);
	} else if (*_pCurPos == *_pArcheroids) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pBeacon);
		*_pPortName = "Beacon";
		if (g_LevelThree[3].cargo._pszCargo != nullptr && g_LevelThree[3].cargo._bUsed) {
			g_LevelThree[3].cargo._bUsed = false;
			_cargo -= 125;
		}
		calcFuel(1.2);
	} else if (*_pCurPos == *_pPackRat) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pBeacon);
		*_pPortName = "Beacon";
		if (g_LevelThree[3].cargo._pszCargo != nullptr && g_LevelThree[3].cargo._bUsed) {
			g_LevelThree[3].cargo._bUsed = false;
			_cargo -= 125;
		}
		calcFuel(5.2);
	} else if (*_pCurPos == *_pMankala) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pBeacon);
		*_pPortName = "Beacon";
		if (g_LevelThree[3].cargo._pszCargo != nullptr && g_LevelThree[3].cargo._bUsed) {
			g_LevelThree[3].cargo._bUsed = false;
			_cargo -= 125;
		}
		calcFuel(3.8);
	} else if (*_pCurPos == *_pLife) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pBeacon);
		*_pPortName = "Beacon";
		if (g_LevelThree[3].cargo._pszCargo != nullptr && g_LevelThree[3].cargo._bUsed) {
			g_LevelThree[3].cargo._bUsed = false;
			_cargo -= 125;
		}
		calcFuel(3.1);
	} else if (*_pCurPos == *_pFuge) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pBeacon);
		*_pPortName = "Beacon";
		if (g_LevelThree[3].cargo._pszCargo != nullptr && g_LevelThree[3].cargo._bUsed) {
			g_LevelThree[3].cargo._bUsed = false;
			_cargo -= 125;
		}
		calcFuel(1.8);
	} else {
		bofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::onMankala() {
	if (*_pCurPos == *_pMankala) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pMankala);
		*_pPortName = "Mankala";
		calcFuel(0);
	} else if (*_pCurPos == *_pArcheroids) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pMankala);
		*_pPortName = "Mankala";
		calcFuel(2.6);
	} else if (*_pCurPos == *_pNoVacancy) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pMankala);
		*_pPortName = "Mankala";
		calcFuel(1.6);
	} else if (*_pCurPos == *_pBeacon) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pMankala);
		*_pPortName = "Mankala";
		calcFuel(3.8);
	} else if (*_pCurPos == *_pGarfunkel) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pMankala);
		*_pPortName = "Mankala";
		calcFuel(1.7);
	} else {
		bofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::onLife() {
	if (*_pCurPos == *_pLife) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pLife);
		*_pPortName = "Life";
		calcFuel(0);
	} else if (*_pCurPos == *_pFuge) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pLife);
		*_pPortName = "Life";
		calcFuel(2.9);
	} else if (*_pCurPos == *_pBeacon) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pLife);
		*_pPortName = "Life";
		calcFuel(3.1);
	} else if (*_pCurPos == *_pBattlefish) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pLife);
		*_pPortName = "Life";
		calcFuel(1.1);
	} else {
		bofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::onFuge() {
	if (*_pCurPos == *_pFuge) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pFuge);
		*_pPortName = "Fuge";
		if (g_LevelThree[6].cargo._pszCargo != nullptr && g_LevelThree[6].cargo._bUsed) {
			g_LevelThree[6].cargo._bUsed = false;
			_cargo -= 10;
		}
		calcFuel(0);
	} else if (*_pCurPos == *_pGarfunkel) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pFuge);
		*_pPortName = "Fuge";
		if (g_LevelThree[6].cargo._pszCargo != nullptr && g_LevelThree[6].cargo._bUsed) {
			g_LevelThree[6].cargo._bUsed = false;
			_cargo -= 10;
		}
		calcFuel(1.5);
	} else if (*_pCurPos == *_pBeacon) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pFuge);
		*_pPortName = "Fuge";
		if (g_LevelThree[6].cargo._pszCargo != nullptr && g_LevelThree[6].cargo._bUsed) {
			g_LevelThree[6].cargo._bUsed = false;
			_cargo -= 10;
		}
		calcFuel(1.8);
	} else if (*_pCurPos == *_pLife) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pFuge);
		*_pPortName = "Fuge";
		if (g_LevelThree[6].cargo._pszCargo != nullptr && g_LevelThree[6].cargo._bUsed) {
			g_LevelThree[6].cargo._bUsed = false;
			_cargo -= 10;
		}
		calcFuel(2.9);
	} else if (*_pCurPos == *_pBattlefish) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pFuge);
		*_pPortName = "Fuge";
		if (g_LevelThree[6].cargo._pszCargo != nullptr && g_LevelThree[6].cargo._bUsed) {
			g_LevelThree[6].cargo._bUsed = false;
			_cargo -= 10;
		}
		calcFuel(2.0);
	} else {
		bofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::onGarfunkel() {
	if (*_pCurPos == *_pGarfunkel) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pGarfunkel);
		*_pPortName = "Garfunkel";
		calcFuel(0);
	} else if (*_pCurPos == *_pMankala) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pGarfunkel);
		*_pPortName = "Garfunkel";
		calcFuel(1.7);
	} else if (*_pCurPos == *_pFuge) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pGarfunkel);
		*_pPortName = "Garfunkel";
		calcFuel(1.5);
	} else {
		bofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::onBattlefish() {
	if (*_pCurPos == *_pBattlefish) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pBattlefish);
		*_pPortName = "Battlefish";
		if (g_LevelThree[8].cargo._pszCargo != nullptr && g_LevelThree[8].cargo._bUsed) {
			g_LevelThree[8].cargo._bUsed = false;
			_cargo -= 17;
		}
		calcFuel(0);
	} else if (*_pCurPos == *_pFuge) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pBattlefish);
		*_pPortName = "Battlefish";
		if (g_LevelThree[8].cargo._pszCargo != nullptr && g_LevelThree[8].cargo._bUsed) {
			g_LevelThree[8].cargo._bUsed = false;
			_cargo -= 17;
		}
		calcFuel(2);

	} else if (*_pCurPos == *_pLife) {

		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pBattlefish);
		*_pPortName = "Battlefish";
		if (g_LevelThree[8].cargo._pszCargo != nullptr && g_LevelThree[8].cargo._bUsed) {
			g_LevelThree[8].cargo._bUsed = false;
			_cargo -= 17;
		}
		calcFuel(1.1);
	} else {
		bofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::onNoVacancy() {
	if (*_pCurPos == *_pNoVacancy) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pNoVacancy);
		*_pPortName = "No Vacancy";
		calcFuel(0);
	} else if (*_pCurPos == *_pMankala) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pNoVacancy);
		*_pPortName = "No Vacancy";
		calcFuel(1.6);
	} else if (*_pCurPos == *_pPeggleboz) {
		delete _pCurPos;
		_pCurPos = nullptr;
		_pCurPos = new CBofRect(*_pNoVacancy);
		*_pPortName = "No Vacancy";
		calcFuel(2.4);
	} else {
		bofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

} // namespace SpaceBar
} // namespace Bagel
