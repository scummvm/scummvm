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
#include "bagel/boflib/sound.h"
#include "bagel/boflib/misc.h"

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

#if BOF_MAC
const char *MakeDir(const char *pszFile);
#endif

struct ST_BUTTONS {
	const char *m_pszName;
	const char *m_pszUp;
	const char *m_pszDown;
	const char *m_pszFocus;
	const char *m_pszDisabled;
	int m_nLeft;
	int m_nTop;
	int m_nWidth;
	int m_nHeight;
	int m_nID;

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
	LogInfo("Constructing NAV...");

	m_level = 0;
	m_pPinna = nullptr;
	m_pHpoctyl = nullptr;
	m_pArmpit = nullptr;
	m_pYzore = nullptr;
	m_pBulbus = nullptr;
	m_pDingle = nullptr;
	m_pKarkas = nullptr;
	m_pFruufnia = nullptr;
	m_pButtons[0] = nullptr;
	m_pButtons[1] = nullptr;
	m_pPal = nullptr;
	m_pOldPal = nullptr;
	m_pMap = nullptr;
	m_pCurLoc = nullptr;
	//  m_pLevelDone = nullptr;
	m_pNewMap = nullptr;
	m_pCurPos = nullptr;
	m_pPortName = nullptr;
	m_pWilbur = nullptr;
	m_pMcKelvey = nullptr;
	m_pMedge = nullptr;
	m_pWall = nullptr;
	m_pWoo = nullptr;
	m_pRoman = nullptr;
	m_pWeed = nullptr;
	m_pBok = nullptr;
	m_pPizer = nullptr;
	m_pBackwater = nullptr;
	m_pUranus = nullptr;
	m_pToejam = nullptr;
	m_pHellhole = nullptr;
	m_pPustule = nullptr;
	m_pSwamprock = nullptr;
	m_pSlug = nullptr;
	m_pMaggotsnest = nullptr;
	m_pPeggleboz = nullptr;
	m_pArcheroids = nullptr;
	m_pPackRat = nullptr;
	m_pBeacon = nullptr;
	m_pMankala = nullptr;
	m_pLife = nullptr;
	m_pFuge = nullptr;
	m_pGarfunkel = nullptr;
	m_pBattlefish = nullptr;
	m_pNoVacancy = nullptr;
	m_bNavAttached = false;
}


#ifdef BOF_DEBUG
CNavWindow::~CNavWindow() {
	Assert(IsValidObject(this));

	LogInfo("Destructed NAV.");
}
#endif


ErrorCode CNavWindow::Attach() {
	Assert(IsValidObject(this));

	LogInfo("\tCNavWindow::Attach()");
	CBagStorageDevWnd::Attach();

	m_level = 0;

	m_pPortName = new CBofString("Yzore");      // always starts at Yzore
	m_ship = 120;
	m_cargo = 86 + 72 + 56;
	m_fuel = 40;

	//navsim1
	m_pPinna = new CBofRect(36, 54, 45, 63);
	m_pHpoctyl = new CBofRect(181, 19, 190, 28);
	m_pArmpit = new CBofRect(108, 251, 117, 260);
	m_pYzore = new CBofRect(251, 271, 260, 280);
	m_pBulbus = new CBofRect(323, 197, 332, 206);
	m_pDingle = new CBofRect(394, 143, 403, 154);
	m_pKarkas = new CBofRect(163, 394, 172, 403);
	m_pFruufnia = new CBofRect(306, 393, 315, 402);

	//navsim2
	m_pWilbur = new CBofRect(108, 38, 116, 49);
	m_pMcKelvey = new CBofRect(394, 37, 403, 46);
	m_pMedge = new CBofRect(19, 90, 28, 99);
	m_pWall = new CBofRect(234, 90, 243, 99);
	m_pWoo = new CBofRect(108, 306, 119, 315);
	m_pRoman = new CBofRect(198, 270, 207, 279);
	m_pWeed = new CBofRect(322, 252, 331, 261);
	m_pBok = new CBofRect(55, 395, 64, 404);
	m_pPizer = new CBofRect(339, 412, 348, 421);

	//navsim3
	m_pBackwater = new CBofRect(108, 18, 117, 27);
	m_pUranus = new CBofRect(161, 126, 170, 135);
	m_pToejam = new CBofRect(252, 126, 261, 135);
	m_pHellhole = new CBofRect(412, 108, 421, 117);
	m_pPustule = new CBofRect(180, 269, 189, 278);
	m_pSwamprock = new CBofRect(340, 234, 349, 243);
	m_pSlug = new CBofRect(36, 341, 45, 350);
	m_pMaggotsnest = new CBofRect(251, 376, 260, 385);

	//navsim4
	m_pPeggleboz = new CBofRect(90, 20, 99, 29);
	m_pArcheroids = new CBofRect(144, 89, 153, 98);
	m_pPackRat = new CBofRect(324, 55, 333, 64);
	m_pBeacon = new CBofRect(270, 144, 279, 155);
	m_pMankala = new CBofRect(72, 251, 81, 260);
	m_pLife = new CBofRect(394, 234, 403, 243);
	m_pFuge = new CBofRect(270, 324, 279, 333);
	m_pGarfunkel = new CBofRect(90, 377, 99, 386);
	m_pBattlefish = new CBofRect(359, 359, 368, 368);
	m_pNoVacancy = new CBofRect(18, 107, 117, 116);

	// Reset all levels
	//
	int i;
	for (i = 0; i < 9; i++) {
		g_Level[i].cargo.m_bUsed = true;
	}
	for (i = 0; i < 10; i++) {
		g_LevelOne[i].cargo.m_bUsed = true;
	}
	for (i = 0; i < 9; i++) {
		g_LevelTwo[i].cargo.m_bUsed = true;
	}
	for (i = 0; i < 11; i++) {
		g_LevelThree[i].cargo.m_bUsed = true;
	}

	m_pLevel = g_Level;
	m_pCurPos = new CBofRect(*m_pYzore);

	m_pOldPal = CBofApp::GetApp()->GetPalette();
	m_pPal = m_pBackdrop->GetPalette()->CopyPalette();
	CBofApp::GetApp()->SetPalette(m_pPal);

	if ((m_pCurLoc = new CBofSprite) != nullptr) {
		m_pCurLoc->LoadSprite(MakeDir(CUR_LOC), 2);
		m_pCurLoc->SetMaskColor(MASK_COLOR);
		m_pCurLoc->SetZOrder(SPRITE_TOPMOST);
		m_pCurLoc->SetAnimated(true);
		m_pCurLoc->LinkSprite();
		m_pCurLoc->SetPosition(m_pCurPos->left, m_pCurPos->top);

	} else {
		ReportError(ERR_MEMORY);
	}
	// sign goes up between levels
	/*  if ((m_pLevelDone = new CBofSprite) != nullptr) {
	        m_pLevelDone->LoadSprite(MakeDir(LEVEL_DONE),1);
	        m_pLevelDone->SetZOrder(SPRITE_FOREGROUND);
	        m_pLevelDone->SetAnimated(false);
	        m_pLevelDone->LinkSprite();
	    } else {
	        ReportError(ERR_MEMORY);
	    }
	*/
// Build all our buttons
//
	for (i = 0; i < 2; i++) {

		if ((m_pButtons[i] = new CBofBmpButton) != nullptr) {

			CBofBitmap *pUp, *pDown, *pFocus, *pDis;

			pUp = LoadBitmap(MakeDir(g_NavButtons[i].m_pszUp), m_pPal);
			pDown = LoadBitmap(MakeDir(g_NavButtons[i].m_pszDown), m_pPal);
			pFocus = LoadBitmap(MakeDir(g_NavButtons[i].m_pszFocus), m_pPal);
			pDis = LoadBitmap(MakeDir(g_NavButtons[i].m_pszDisabled), m_pPal);

			m_pButtons[i]->LoadBitmaps(pUp, pDown, pFocus, pDis);

			m_pButtons[i]->Create(g_NavButtons[i].m_pszName, g_NavButtons[i].m_nLeft, g_NavButtons[i].m_nTop, g_NavButtons[i].m_nWidth, g_NavButtons[i].m_nHeight, this, g_NavButtons[i].m_nID);
			m_pButtons[i]->Show();
		} else {
			ReportError(ERR_MEMORY);
			break;
		}
	}

	Show();
	UpdateWindow();
	SetTimer(777, 200, nullptr);

	BofPlaySound(MakeDir(WELCOME_SND), SOUND_WAVE);
	CSound::WaitWaveSounds();

	CBagCursor::ShowSystemCursor();

	m_bNavAttached = true;

	return m_errCode;
}


ErrorCode CNavWindow::Detach() {
	Assert(IsValidObject(this));

	// If attached
	//
	if (m_bNavAttached) {

		// Get rid of the system cursor
		CBagCursor::HideSystemCursor();

		// One turn has gone by
		VARMNGR->IncrementTimers();

		m_bNavAttached = false;
	}

	LogInfo("\tCNavWindow::Detach()");

	CBofApp::GetApp()->SetPalette(m_pOldPal);

	if (m_pPal != nullptr) {
		delete m_pPal;
		m_pPal = nullptr;
	}

	if (m_pPortName != nullptr) {
		delete m_pPortName;
		m_pPortName = nullptr;
	}

	if (m_pPinna != nullptr) {
		delete m_pPinna;
		m_pPinna = nullptr;
	}

	if (m_pHpoctyl != nullptr) {
		delete m_pHpoctyl;
		m_pHpoctyl = nullptr;
	}

	if (m_pArmpit != nullptr) {
		delete m_pArmpit;
		m_pArmpit = nullptr;
	}

	if (m_pYzore != nullptr) {
		delete m_pYzore;
		m_pYzore = nullptr;
	}

	if (m_pBulbus != nullptr) {
		delete m_pBulbus;
		m_pBulbus = nullptr;
	}

	if (m_pDingle != nullptr) {
		delete m_pDingle;
		m_pDingle = nullptr;
	}

	if (m_pKarkas != nullptr) {
		delete m_pKarkas;
		m_pKarkas = nullptr;
	}

	if (m_pFruufnia != nullptr) {
		delete m_pFruufnia;
		m_pFruufnia = nullptr;
	}

	if (m_pCurPos != nullptr) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
	}

	if (m_pWilbur != nullptr) {
		delete m_pWilbur;
		m_pWilbur = nullptr;
	}

	if (m_pMcKelvey != nullptr) {
		delete m_pMcKelvey;
		m_pMcKelvey = nullptr;
	}

	if (m_pMedge != nullptr) {
		delete m_pMedge;
		m_pMedge = nullptr;
	}

	if (m_pWall != nullptr) {
		delete m_pWall;
		m_pWall = nullptr;
	}

	if (m_pWoo != nullptr) {
		delete m_pWoo;
		m_pWoo = nullptr;
	}

	if (m_pRoman != nullptr) {
		delete m_pRoman;
		m_pRoman = nullptr;
	}

	if (m_pWeed != nullptr) {
		delete m_pWeed;
		m_pWeed = nullptr;
	}

	if (m_pBok != nullptr) {
		delete m_pBok;
		m_pBok = nullptr;
	}

	if (m_pPizer != nullptr) {
		delete m_pPizer;
		m_pPizer = nullptr;
	}

	if (m_pBackwater != nullptr) {
		delete m_pBackwater;
		m_pBackwater = nullptr;
	}

	if (m_pUranus != nullptr) {
		delete m_pUranus;
		m_pUranus = nullptr;
	}

	if (m_pToejam != nullptr) {
		delete m_pToejam;
		m_pToejam = nullptr;
	}

	if (m_pHellhole != nullptr) {
		delete m_pHellhole;
		m_pHellhole = nullptr;
	}

	if (m_pPustule != nullptr) {
		delete m_pPustule;
		m_pPustule = nullptr;
	}

	if (m_pSlug != nullptr) {
		delete m_pSlug;
		m_pSlug = nullptr;
	}

	if (m_pSwamprock != nullptr) {
		delete m_pSwamprock;
		m_pSwamprock = nullptr;
	}

	if (m_pMaggotsnest != nullptr) {
		delete m_pMaggotsnest;
		m_pMaggotsnest = nullptr;
	}

	if (m_pPeggleboz != nullptr) {
		delete m_pPeggleboz;
		m_pPeggleboz = nullptr;
	}

	if (m_pArcheroids != nullptr) {
		delete m_pArcheroids;
		m_pArcheroids = nullptr;
	}

	if (m_pPackRat != nullptr) {
		delete m_pPackRat;
		m_pPackRat = nullptr;
	}

	if (m_pBeacon != nullptr) {
		delete m_pBeacon;
		m_pBeacon = nullptr;
	}

	if (m_pMankala != nullptr) {
		delete m_pMankala;
		m_pMankala = nullptr;
	}

	if (m_pLife != nullptr) {
		delete m_pLife;
		m_pLife = nullptr;
	}

	if (m_pFuge != nullptr) {
		delete m_pFuge;
		m_pFuge = nullptr;
	}

	if (m_pGarfunkel != nullptr) {
		delete m_pGarfunkel;
		m_pGarfunkel = nullptr;
	}

	if (m_pBattlefish != nullptr) {
		delete m_pBattlefish;
		m_pBattlefish = nullptr;
	}

	if (m_pCurLoc != nullptr) {
		delete m_pCurLoc;
		m_pCurLoc = nullptr;
	}

	if (m_pNoVacancy != nullptr) {
		delete m_pNoVacancy;
		m_pNoVacancy = nullptr;
	}

	// Destroy all buttons
	//
	for (int i = 0; i < 2; i++) {

		if (m_pButtons[i] != nullptr) {
			delete m_pButtons[i];
			m_pButtons[i] = nullptr;
		}
	}

	// Close sprite lib
	CBofSprite::CloseLibrary();

	KillBackdrop();

	CBagStorageDevWnd::Detach();

	return m_errCode;
}


void CNavWindow::OnMainLoop() {
	// Do nothing
}


void CNavWindow::OnPaint(CBofRect *pRect) {
	Assert(IsValidObject(this));
	Assert(pRect != nullptr);

	CBofBitmap *pBmp;

	// Render offscreen
	//
	if (m_pBackdrop != nullptr && (pBmp = GetWorkBmp()) != nullptr) {
		m_pBackdrop->Paint(pBmp, pRect, pRect);
		// Now update the screen
		pBmp->Paint(this, pRect, pRect);
	}

	SetTimer(777, 200, nullptr);

	// separated repainting of text
	RefreshData();
}

void CNavWindow::RefreshData() {
	Assert(IsValidObject(this));

	CBofBitmap *pBmp;
	CBofRect cRect(440, 0, 639, 439);
	char    szBuf[100];
	int     i = 0;

	// Render offscreen
	//
	if (m_pBackdrop != nullptr && (pBmp = GetWorkBmp()) != nullptr) {
		CBofBitmap cBmp(200, 440, m_pPal);
		pBmp->Paint(&cBmp, 0, 0, &cRect);

		cRect.SetRect(0, 10, 199, 25);
		PaintText(&cBmp, &cRect, BuildString("Current Port of call:"), 14, TEXT_NORMAL, NAVTEXT_COLOR, JUSTIFY_LEFT, FORMAT_DEFAULT, FONT_MONO);
		cRect.left += 10;       // indent this section
		cRect.top += 15;
		cRect.bottom += 15;
		PaintText(&cBmp, &cRect, BuildString("%s", m_pPortName->GetBuffer()), 14, TEXT_NORMAL, NAVTEXT_COLOR, JUSTIFY_LEFT, FORMAT_DEFAULT, FONT_MONO);
		cRect.left -= 10;       // un-indent
		// leave blank space before next section
		cRect.top += 30;
		cRect.bottom += 30;

		PaintText(&cBmp, &cRect, BuildString("Current Manifest:"), 14, TEXT_NORMAL, NAVTEXT_COLOR, JUSTIFY_LEFT, FORMAT_DEFAULT, FONT_MONO);

		cRect.left += 10;       // indent this section
		while (m_pLevel[i].Name != nullptr) {

			if (m_pLevel[i].cargo.m_pszCargo != nullptr && m_pLevel[i].cargo.m_bUsed) {

				cRect.top += 15;
				cRect.bottom += 15;
				if (m_pLevel[i].cargo.Weight != 1)
					Common::sprintf_s(szBuf, "%3d tons of %s", m_pLevel[i].cargo.Weight, m_pLevel[i].cargo.m_pszCargo/*, m_pLevel[i].Name*/);
				else
					Common::sprintf_s(szBuf, "%3d ton of %s", m_pLevel[i].cargo.Weight, m_pLevel[i].cargo.m_pszCargo/*, m_pLevel[i].Name*/);
				PaintText(&cBmp, &cRect, szBuf, 14, TEXT_NORMAL, NAVTEXT_COLOR, JUSTIFY_LEFT, FORMAT_DEFAULT, FONT_MONO);
				cRect.top += 15;
				cRect.bottom += 15;
				Common::sprintf_s(szBuf, "    for %s", m_pLevel[i].Name);
				PaintText(&cBmp, &cRect, szBuf, 14, TEXT_NORMAL, NAVTEXT_COLOR, JUSTIFY_LEFT, FORMAT_DEFAULT, FONT_MONO);
			};
			i++;
		};
		cRect.left -= 10;       // un-indent
		// leave blank space before next section
		cRect.top += 30;
		cRect.bottom += 30;

		PaintText(&cBmp, &cRect, BuildString("Current Fuel:"), 14, TEXT_NORMAL, NAVTEXT_COLOR, JUSTIFY_LEFT, FORMAT_DEFAULT, FONT_MONO);
		cRect.left += 10;       // indent this section
		cRect.top += 15;
		cRect.bottom += 15;
		if (m_fuel != 1)
			PaintText(&cBmp, &cRect, BuildString("%3d tons", m_fuel), 14, TEXT_NORMAL, NAVTEXT_COLOR, JUSTIFY_LEFT, FORMAT_DEFAULT, FONT_MONO);
		else
			PaintText(&cBmp, &cRect, BuildString("%3d ton", m_fuel), 14, TEXT_NORMAL, NAVTEXT_COLOR, JUSTIFY_LEFT, FORMAT_DEFAULT, FONT_MONO);
		cRect.left -= 10;       // un-indent
		// leave blank space before next section
		cRect.top += 30;
		cRect.bottom += 30;

		PaintText(&cBmp, &cRect, BuildString("Current Tonnage:"), 14, TEXT_NORMAL, NAVTEXT_COLOR, JUSTIFY_LEFT, FORMAT_DEFAULT, FONT_MONO);
		cRect.top += 15;
		cRect.bottom += 15;

		cRect.left += 10;       // indent this section
		if (m_cargo != 1)
			Common::sprintf_s(szBuf, "%3d tons Cargo", m_cargo);
		else
			Common::sprintf_s(szBuf, "%3d ton Cargo", m_cargo);
		PaintText(&cBmp, &cRect, szBuf, 14, TEXT_NORMAL, NAVTEXT_COLOR, JUSTIFY_LEFT, FORMAT_DEFAULT, FONT_MONO);
		cRect.top += 15;
		cRect.bottom += 15;
		if (m_ship != 1)
			Common::sprintf_s(szBuf, "%3d tons Ship", m_ship);
		else
			Common::sprintf_s(szBuf, "%3d ton Ship", m_ship);
		PaintText(&cBmp, &cRect, szBuf, 14, TEXT_NORMAL, NAVTEXT_COLOR, JUSTIFY_LEFT, FORMAT_DEFAULT, FONT_MONO);
		cRect.top += 15;
		cRect.bottom += 15;
		if (m_fuel != 1)
			Common::sprintf_s(szBuf, "%3d tons Fuel", m_fuel);
		else
			Common::sprintf_s(szBuf, "%3d ton Fuel", m_fuel);
		PaintText(&cBmp, &cRect, szBuf, 14, TEXT_NORMAL, NAVTEXT_COLOR, JUSTIFY_LEFT, FORMAT_DEFAULT, FONT_MONO);
		cRect.top += 15;
		cRect.bottom += 15;
		Common::sprintf_s(szBuf, "%3d tons TOTAL", (m_ship + m_fuel + m_cargo));
		PaintText(&cBmp, &cRect, szBuf, 14, TEXT_NORMAL, NAVTEXT_COLOR, JUSTIFY_LEFT, FORMAT_DEFAULT, FONT_MONO);
		cRect.left -= 10;       // un-indent

		cRect.top += 30;
		cRect.bottom += 30;
		Common::sprintf_s(szBuf, "Simulation Level:"/*, g_LevelTitle[m_level]*/);
		PaintText(&cBmp, &cRect, szBuf, 14, TEXT_NORMAL, NAVTEXT_COLOR, JUSTIFY_LEFT, FORMAT_DEFAULT, FONT_MONO);
		cRect.left += 10;       // indent
		cRect.top += 15;
		cRect.bottom += 15;
		Common::sprintf_s(szBuf, "%s", g_LevelTitle[m_level]);
		PaintText(&cBmp, &cRect, szBuf, 14, TEXT_NORMAL, NAVTEXT_COLOR, JUSTIFY_LEFT, FORMAT_DEFAULT, FONT_MONO);
		cRect.left -= 10;       // un-indent
		cBmp.Paint(this, 443, 0);
	}
}

void CNavWindow::OnBofButton(CBofObject *pObject, int nState) {
	Assert(IsValidObject(this));
	Assert(pObject != nullptr);

	CBofButton *pButton;

	pButton = (CBofButton *)pObject;

	if (nState == BUTTON_CLICKED) {

		switch (pButton->GetControlID()) {

		case QUIT: {
			LogInfo("\tClicked Quit");
			VARMNGR->GetVariable("NPLAYEDNAV")->SetBoolValue(true);
			Close();
			break;
		}
		case HELP: {
			LogInfo("\tClicked Help");

			CBagel *pApp;
			CBagMasterWin *pWin;

			KillTimer(777);
			if ((pApp = CBagel::GetBagApp()) != nullptr) {
				if ((pWin = pApp->GetMasterWnd()) != nullptr) {
					pWin->OnHelp(MakeDir("NAVHELP.TXT"));
				}
			}
			break;
		}

		default:
			LogWarning(BuildString("Clicked Unknown Button with ID %d", pButton->GetControlID()));
			break;
		}

	}
}


void CNavWindow::OnLButtonDown(uint32 /*nFlags*/, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

	switch (m_level) {
	case 0: {
		if (m_pPinna->PtInRect(*pPoint))
			OnPinna();

		if (m_pHpoctyl->PtInRect(*pPoint))
			OnHpoctyl();

		if (m_pArmpit->PtInRect(*pPoint))
			OnArmpit();

		if (m_pYzore->PtInRect(*pPoint))
			OnYzore();

		if (m_pBulbus->PtInRect(*pPoint))
			OnBulbus();

		if (m_pDingle->PtInRect(*pPoint))
			OnDingle();

		if (m_pKarkas->PtInRect(*pPoint))
			OnKarkas();

		if (m_pFruufnia->PtInRect(*pPoint))
			OnFruufnia();
		break;
	}//close case

	case 1: {
		if (m_pWilbur->PtInRect(*pPoint))
			OnWilbur();

		if (m_pMcKelvey->PtInRect(*pPoint))
			OnMcKelvey();

		if (m_pMedge->PtInRect(*pPoint))
			OnMedge();

		if (m_pWall->PtInRect(*pPoint))
			OnWall();

		if (m_pWoo->PtInRect(*pPoint))
			OnWoo();

		if (m_pRoman->PtInRect(*pPoint))
			OnRoman();

		if (m_pWeed->PtInRect(*pPoint))
			OnWeed();

		if (m_pBok->PtInRect(*pPoint))
			OnBok();

		if (m_pPizer->PtInRect(*pPoint))
			OnPizer();

		break;
	}//close case

	case 2: {
		if (m_pBackwater->PtInRect(*pPoint))
			OnBackwater();

		if (m_pUranus->PtInRect(*pPoint))
			OnUranus();

		if (m_pToejam->PtInRect(*pPoint))
			OnToejam();

		if (m_pHellhole->PtInRect(*pPoint))
			OnHellhole();

		if (m_pPustule->PtInRect(*pPoint))
			OnPustule();

		if (m_pSwamprock->PtInRect(*pPoint))
			OnSwamprock();

		if (m_pSlug->PtInRect(*pPoint))
			OnSlug();

		if (m_pMaggotsnest->PtInRect(*pPoint))
			OnMaggotsnest();

		break;
	}
	case 3: {
		if (m_pPeggleboz->PtInRect(*pPoint))
			OnPeggleboz();

		if (m_pNoVacancy->PtInRect(*pPoint))
			OnNoVacancy();

		if (m_pArcheroids->PtInRect(*pPoint))
			OnArcheroids();

		if (m_pPackRat->PtInRect(*pPoint))
			OnPackRat();

		if (m_pBeacon->PtInRect(*pPoint))
			OnBeacon();

		if (m_pMankala->PtInRect(*pPoint))
			OnMankala();

		if (m_pLife->PtInRect(*pPoint))
			OnLife();

		if (m_pFuge->PtInRect(*pPoint))
			OnFuge();

		if (m_pGarfunkel->PtInRect(*pPoint))
			OnGarfunkel();

		if (m_pBattlefish->PtInRect(*pPoint))
			OnBattlefish();

		break;
	}//close case
	} //close switch

	if (m_cargo <= 0)
		CalcFuel(0);
}


void CNavWindow::OnKeyHit(uint32 lKey, uint32 /*lRepCount*/) {
	Assert(IsValidObject(this));

	if ((lKey == BKEY_ALT_Q) || (lKey == BKEY_ALT_q) || (lKey == BKEY_ALT_F4)) {
		VARMNGR->GetVariable("NPLAYEDNAV")->SetBoolValue(true);
		Close();
	}
}

void CNavWindow::OnTimer(uint32 tId) {
	if (tId == 777) {
		m_pCurLoc->PaintSprite(this, m_pCurPos->left, m_pCurPos->top);
	}
}

void CNavWindow::OnPinna() {
	if ((*m_pCurPos == *m_pPinna)) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pPinna);
		*m_pPortName = "Pinna";
		CalcFuel(0);
	} else if (*m_pCurPos == *m_pHpoctyl) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pPinna);
		*m_pPortName = "Pinna";
		CalcFuel(1.6);
	} else if (*m_pCurPos == *m_pArmpit) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pPinna);
		*m_pPortName = "Pinna";
		CalcFuel(1.5);
	} else {
		BofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::OnHpoctyl() {
	if ((*m_pCurPos == *m_pHpoctyl)) {

		*m_pPortName = "H'poctyl";
		if (g_Level[1].cargo.m_pszCargo != nullptr && g_Level[1].cargo.m_bUsed) {
			g_Level[1].cargo.m_bUsed = false;
			m_cargo -= 72;
		}
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pHpoctyl);
		CalcFuel(0);
	} else if (*m_pCurPos == *m_pArmpit) {
		*m_pPortName = "H'poctyl";
		if (g_Level[1].cargo.m_pszCargo != nullptr && g_Level[1].cargo.m_bUsed) {
			g_Level[1].cargo.m_bUsed = false;
			m_cargo -= 72;
		}
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pHpoctyl);
		CalcFuel(3.2);
	} else if (*m_pCurPos == *m_pPinna) {
		*m_pPortName = "H'poctyl";
		if (g_Level[1].cargo.m_pszCargo != nullptr && g_Level[1].cargo.m_bUsed) {
			g_Level[1].cargo.m_bUsed = false;
			m_cargo -= 72;
		}
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pHpoctyl);
		CalcFuel(1.6);
	} else if (*m_pCurPos == *m_pDingle) {
		*m_pPortName = "H'poctyl";
		if (g_Level[1].cargo.m_pszCargo != nullptr && g_Level[1].cargo.m_bUsed) {
			g_Level[1].cargo.m_bUsed = false;
			m_cargo -= 72;
		}
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pHpoctyl);
		CalcFuel(2.4);
	} else if (*m_pCurPos == *m_pBulbus) {
		*m_pPortName = "H'poctyl";
		if (g_Level[1].cargo.m_pszCargo != nullptr && g_Level[1].cargo.m_bUsed) {
			g_Level[1].cargo.m_bUsed = false;
			m_cargo -= 72;
		}
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pHpoctyl);
		CalcFuel(2.8);
	} else {
		BofMessageBox("Destination unavailable !!", "Test Warning");
	}
}


void CNavWindow::OnArmpit() {
	if ((*m_pCurPos == *m_pArmpit)) {

		delete m_pCurPos;
		m_pCurPos = new CBofRect(*m_pArmpit);
		*m_pPortName = "Armpit";
		if (g_Level[2].cargo.m_pszCargo != nullptr && g_Level[2].cargo.m_bUsed) {
			g_Level[2].cargo.m_bUsed = false;
			m_cargo -= 56;
		}

	} else if ((*m_pCurPos == *m_pHpoctyl)) {

		delete m_pCurPos;
		m_pCurPos = new CBofRect(*m_pArmpit);
		*m_pPortName = "Armpit";
		if (g_Level[2].cargo.m_pszCargo != nullptr && g_Level[2].cargo.m_bUsed) {
			g_Level[2].cargo.m_bUsed = false;
			m_cargo -= 56;
		}
		CalcFuel(3.2);
	} else if ((*m_pCurPos == *m_pPinna)) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pArmpit);
		*m_pPortName = "Armpit";
		if (g_Level[2].cargo.m_pszCargo != nullptr && g_Level[2].cargo.m_bUsed) {
			g_Level[2].cargo.m_bUsed = false;
			m_cargo -= 56;
		}
		CalcFuel(1.5);
	} else if ((*m_pCurPos == *m_pKarkas)) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pArmpit);
		*m_pPortName = "Armpit";
		if (g_Level[2].cargo.m_pszCargo != nullptr && g_Level[2].cargo.m_bUsed) {
			g_Level[2].cargo.m_bUsed = false;
			m_cargo -= 56;
		}
		CalcFuel(2.3);
	} else {
		BofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::OnYzore() {
	if ((*m_pCurPos == *m_pYzore)) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		*m_pPortName = "Yzore";
		m_pCurPos = new CBofRect(*m_pYzore);
	} else if ((*m_pCurPos == *m_pKarkas)) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pYzore);
		*m_pPortName = "Yzore";
		CalcFuel(2.6);
	} else if ((*m_pCurPos == *m_pFruufnia)) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pYzore);
		*m_pPortName = "Yzore";
		CalcFuel(4.0);
	} else if ((*m_pCurPos == *m_pBulbus)) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pYzore);
		*m_pPortName = "Yzore";
		CalcFuel(1.2);
	} else {
		BofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::OnBulbus() {
	if (*m_pCurPos == *m_pBulbus) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		*m_pPortName = "Bulbus";
		m_pCurPos = new CBofRect(*m_pBulbus);
	} else if ((*m_pCurPos == *m_pHpoctyl)) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pBulbus);
		*m_pPortName = "Bulbus";
		CalcFuel(2.8);
	} else if ((*m_pCurPos == *m_pYzore)) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pBulbus);
		*m_pPortName = "Bulbus";
		CalcFuel(1.2);
	} else if ((*m_pCurPos == *m_pDingle)) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pBulbus);
		*m_pPortName = "Bulbus";
		CalcFuel(1.9);
	} else if ((*m_pCurPos == *m_pFruufnia)) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pBulbus);
		*m_pPortName = "Bulbus";
		CalcFuel(3.7);
	} else {
		BofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::OnDingle() {
	if (*m_pCurPos == *m_pDingle) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		*m_pPortName = "Dingle";
		m_pCurPos = new CBofRect(*m_pDingle);
	} else if ((*m_pCurPos == *m_pHpoctyl)) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pDingle);
		*m_pPortName = "Dingle";
		CalcFuel(2.4);
	} else if ((*m_pCurPos == *m_pBulbus)) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pDingle);
		*m_pPortName = "Dingle";
		CalcFuel(1.9);
	} else {
		BofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::OnKarkas() {
	if ((*m_pCurPos == *m_pKarkas)) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		*m_pPortName = "Karkas";
		m_pCurPos = new CBofRect(*m_pKarkas);
	} else if (*m_pCurPos == *m_pArmpit) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pKarkas);
		*m_pPortName = "Karkas";
		CalcFuel(2.3);
	} else if (*m_pCurPos == *m_pYzore) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pKarkas);
		*m_pPortName = "Karkas";
		CalcFuel(2.6);
	} else if (*m_pCurPos == *m_pFruufnia) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pKarkas);
		*m_pPortName = "Karkas";
		CalcFuel(1.7);
	} else {
		BofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::OnFruufnia() {
	if (*m_pCurPos == *m_pFruufnia) {
		delete m_pCurPos;
		*m_pPortName = "Fruufnia";
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pFruufnia);
		if (g_Level[7].cargo.m_pszCargo != nullptr && g_Level[7].cargo.m_bUsed) {
			g_Level[7].cargo.m_bUsed = false;
			m_cargo -= 86;
		}
	} else if (*m_pCurPos == *m_pKarkas) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pFruufnia);
		*m_pPortName = "Fruufnia";
		if (g_Level[7].cargo.m_pszCargo != nullptr && g_Level[7].cargo.m_bUsed) {
			g_Level[7].cargo.m_bUsed = false;
			m_cargo -= 86;
		}
		CalcFuel(1.7);
	} else if (*m_pCurPos == *m_pYzore) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pFruufnia);
		*m_pPortName = "Fruufnia";
		if (g_Level[7].cargo.m_pszCargo != nullptr && g_Level[7].cargo.m_bUsed) {
			g_Level[7].cargo.m_bUsed = false;
			m_cargo -= 86;
		}
		CalcFuel(4.0);
	} else if (*m_pCurPos == *m_pBulbus) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pFruufnia);
		*m_pPortName = "Fruufnia";
		if (g_Level[7].cargo.m_pszCargo != nullptr && g_Level[7].cargo.m_bUsed) {
			g_Level[7].cargo.m_bUsed = false;
			m_cargo -= 86;
		}
		CalcFuel(3.7);
	} else {
		BofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::OnWilbur() {
	if (*m_pCurPos == *m_pWilbur) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		*m_pPortName = "Wilbur";
		m_pCurPos = new CBofRect(*m_pWilbur);
	} else if (*m_pCurPos == *m_pWall) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pWilbur);
		*m_pPortName = "Wilbur";
		CalcFuel(1.6);
	} else if (*m_pCurPos == *m_pMcKelvey) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pWilbur);
		*m_pPortName = "Wilbur";
		CalcFuel(3.6);
	} else if (*m_pCurPos == *m_pMedge) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pWilbur);
		*m_pPortName = "Wilbur";
		CalcFuel(2.1);

	} else {
		BofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::OnMcKelvey() {
	if ((*m_pCurPos == *m_pMcKelvey)) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		*m_pPortName = "McKelvey";
		m_pCurPos = new CBofRect(*m_pMcKelvey);
	} else if (*m_pCurPos == *m_pWilbur) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pMcKelvey);
		*m_pPortName = "McKelvey";
		CalcFuel(3.6);
	} else if (*m_pCurPos == *m_pWall) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pMcKelvey);
		*m_pPortName = "McKelvey";
		CalcFuel(5.3);
	} else if (*m_pCurPos == *m_pWeed) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pMcKelvey);
		*m_pPortName = "McKelvey";
		CalcFuel(2.7);
	} else {
		BofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::OnMedge() {
	if ((*m_pCurPos == *m_pMedge)) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pMedge);
		*m_pPortName = "Medge";
		if (g_LevelOne[2].cargo.m_pszCargo != nullptr && g_LevelOne[2].cargo.m_bUsed) {
			g_LevelOne[2].cargo.m_bUsed = false;
			m_cargo -= 127;
		}
	} else if (*m_pCurPos == *m_pWilbur) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pMedge);
		*m_pPortName = "Medge";
		if (g_LevelOne[2].cargo.m_pszCargo != nullptr && g_LevelOne[2].cargo.m_bUsed) {
			g_LevelOne[2].cargo.m_bUsed = false;
			m_cargo -= 127;
		}
		CalcFuel(2.1);
	} else if (*m_pCurPos == *m_pWall) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pMedge);
		*m_pPortName = "Medge";
		if (g_LevelOne[2].cargo.m_pszCargo != nullptr && g_LevelOne[2].cargo.m_bUsed) {
			g_LevelOne[2].cargo.m_bUsed = false;
			m_cargo -= 127;
		}
		CalcFuel(1.2);
	} else {
		BofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::OnWall() {
	if ((*m_pCurPos == *m_pWall)) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		*m_pPortName = "Wall";
		m_pCurPos = new CBofRect(*m_pWall);
	} else if (*m_pCurPos == *m_pMcKelvey) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pWall);
		*m_pPortName = "Wall";
		CalcFuel(5.3);
	} else if (*m_pCurPos == *m_pWeed) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pWall);
		*m_pPortName = "Wall";
		CalcFuel(1.4);
	} else if (*m_pCurPos == *m_pWilbur) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pWall);
		*m_pPortName = "Wall";
		CalcFuel(1.6);
	} else if (*m_pCurPos == *m_pMedge) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pWall);
		*m_pPortName = "Wall";
		CalcFuel(1.2);
	} else if (*m_pCurPos == *m_pRoman) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		*m_pPortName = "Wall";
		m_pCurPos = new CBofRect(*m_pWall);
		CalcFuel(1.9);
	} else {
		BofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::OnWoo() {
	if ((*m_pCurPos == *m_pWoo)) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		*m_pPortName = "Woo";
		m_pCurPos = new CBofRect(*m_pWoo);
	} else if (*m_pCurPos == *m_pRoman) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pWoo);
		*m_pPortName = "Woo";
		CalcFuel(2.7);
	} else if (*m_pCurPos == *m_pBok) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pWoo);
		*m_pPortName = "Woo";
		CalcFuel(3.9);
	} else if (*m_pCurPos == *m_pPizer) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pWoo);
		*m_pPortName = "Woo";
		CalcFuel(1.7);
	} else {
		BofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::OnRoman() {
	if ((*m_pCurPos == *m_pRoman)) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		*m_pPortName = "Roman";
		m_pCurPos = new CBofRect(*m_pRoman);
		if (g_LevelOne[5].cargo.m_pszCargo != nullptr && g_LevelOne[5].cargo.m_bUsed) {
			g_LevelOne[5].cargo.m_bUsed = false;
			m_cargo -= 20;
		}
	} else if (*m_pCurPos == *m_pWall) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pRoman);
		*m_pPortName = "Roman";
		if (g_LevelOne[5].cargo.m_pszCargo != nullptr && g_LevelOne[5].cargo.m_bUsed) {
			g_LevelOne[5].cargo.m_bUsed = false;
			m_cargo -= 20;
		}
		CalcFuel(1.9);
	} else if (*m_pCurPos == *m_pWoo) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pRoman);
		*m_pPortName = "Roman";
		if (g_LevelOne[5].cargo.m_pszCargo != nullptr && g_LevelOne[5].cargo.m_bUsed) {
			g_LevelOne[5].cargo.m_bUsed = false;
			m_cargo -= 20;
		}
		CalcFuel(2.7);
	} else if (*m_pCurPos == *m_pPizer) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pRoman);
		*m_pPortName = "Roman";
		if (g_LevelOne[5].cargo.m_pszCargo != nullptr && g_LevelOne[5].cargo.m_bUsed) {
			g_LevelOne[5].cargo.m_bUsed = false;
			m_cargo -= 20;
		}
		CalcFuel(4.6);
	} else {
		BofMessageBox("Destination unavailable !!", "Test Warning");
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

void CNavWindow::CalcFuel(double hf) {
	Assert(IsValidObject(this));
	CBofRect cRect(0, 0, 439, 439);

	m_fuel -= (int)((m_ship + m_fuel + m_cargo) * (hf) * (.01));

	if (m_cargo == 0) {
		m_pCurLoc->EraseSprite(this);
		RefreshData();
		KillTimer(777);
		m_pCurLoc->SetCel(0);
		m_pCurLoc->PaintSprite(this, m_pCurPos->left, m_pCurPos->top);

		// WORKAROUND: m_pBackdrop shares it's palette with m_pCurLoc,
		// so as the backdrop is changed, don't free the palette
		m_pBackdrop->SetIsOwnPalette(false);
		bool isDone = m_level == 3;

		if (m_level == 3) {
			VARMNGR->GetVariable("NPASSEDTEST")->SetBoolValue(true);
			VARMNGR->GetVariable("NPLAYEDNAV")->SetBoolValue(true);
			Close();
		}

		if (m_level == 2) {
			pause();
			CBofString sNebDir(NEBSIM4BMP);
			MACROREPLACE(sNebDir);
			Assert(m_pBackdrop != nullptr);
			bmptwo = new CBofBitmap(sNebDir.GetBuffer(), m_pPal);
			SetBackground(bmptwo);
			m_cargo = 125 + 10 + 17 + 8 + 99 + 24;
			m_ship = 65;
			m_fuel = 45;
			delete m_pCurPos;
			m_pCurPos = nullptr;
			m_pCurPos = new CBofRect(*m_pMankala);
			m_level = 3;
			m_pLevel = g_LevelThree;
			*m_pPortName = "Mankala";
		}

		if (m_level == 1) {
			pause();
			CBofString sNebDir(NEBSIM3BMP);
			MACROREPLACE(sNebDir);
			Assert(m_pBackdrop != nullptr);
			bmptwo = new CBofBitmap(sNebDir.GetBuffer(), m_pPal);
			SetBackground(bmptwo);
			m_cargo = 100 + 75 + 28 + 45 + 14;
			m_ship = 99;
			m_fuel = 36;
			delete m_pCurPos;
			m_pCurPos = nullptr;
			m_pCurPos = new CBofRect(*m_pMaggotsnest);
			m_level = 2;
			m_pLevel = g_LevelTwo;
			*m_pPortName = "Maggot's Nest";
		}

		if (m_level == 0) {
			pause();
			CBofString sNebDir(NEBSIM2BMP);
			MACROREPLACE(sNebDir);
			Assert(m_pBackdrop != nullptr);
			bmptwo = new CBofBitmap(sNebDir.GetBuffer(), m_pPal);
			SetBackground(bmptwo);
			m_cargo = 54 + 119 + 20 + 127;
			m_ship = 120;
			m_fuel = 75;
			delete m_pCurPos;
			m_pCurPos = nullptr;
			m_pCurPos = new CBofRect(*m_pMcKelvey);
			m_level = 1;
			m_pLevel = g_LevelOne;
			*m_pPortName = "McKelvey";
		}

		InvalidateRect(&cRect);
		if (!isDone)
			UpdateWindow();

		// WORKAROUND: Restore owns palette flag back again
		m_pBackdrop->SetIsOwnPalette(true);
	}

	if (m_fuel <= 0) {
		VARMNGR->GetVariable("NPLAYEDNAV")->SetBoolValue(true);
		KillTimer(777);
		Close();
	}

	m_pCurLoc->EraseSprite(this);
	RefreshData();
	m_pCurLoc->PaintSprite(this, m_pCurPos->left, m_pCurPos->top);
}

void CNavWindow::OnWeed() {
	if ((*m_pCurPos == *m_pWeed)) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		*m_pPortName = "Weed";
		m_pCurPos = new CBofRect(*m_pWeed);
	} else if (*m_pCurPos == *m_pMcKelvey) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pWeed);
		*m_pPortName = "Weed";
		CalcFuel(2.7);
	} else if (*m_pCurPos == *m_pWall) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pWeed);
		*m_pPortName = "Weed";
		CalcFuel(1.4);
	} else if (*m_pCurPos == *m_pPizer) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pWeed);
		*m_pPortName = "Weed";
		CalcFuel(2.2);
	} else {
		BofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::OnBok() {
	if ((*m_pCurPos == *m_pBok)) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pBok);
		*m_pPortName = "Bok";
		if (g_LevelOne[7].cargo.m_pszCargo != nullptr && g_LevelOne[7].cargo.m_bUsed) {
			g_LevelOne[7].cargo.m_bUsed = false;
			m_cargo -= 119;
		}
	} else if (*m_pCurPos == *m_pWoo) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pBok);
		*m_pPortName = "Bok";
		if (g_LevelOne[7].cargo.m_pszCargo != nullptr && g_LevelOne[7].cargo.m_bUsed) {
			g_LevelOne[7].cargo.m_bUsed = false;
			m_cargo -= 119;
		}
		CalcFuel(3.9);
	} else {
		BofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::OnPizer() {
	if ((*m_pCurPos == *m_pPizer)) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pPizer);
		*m_pPortName = "Pizer";
		if (g_LevelOne[8].cargo.m_pszCargo != nullptr && g_LevelOne[8].cargo.m_bUsed) {
			g_LevelOne[8].cargo.m_bUsed = false;
			m_cargo -= 54;
		}
	} else if (*m_pCurPos == *m_pWoo) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pPizer);
		*m_pPortName = "Pizer";
		if (g_LevelOne[8].cargo.m_pszCargo != nullptr && g_LevelOne[8].cargo.m_bUsed) {
			g_LevelOne[8].cargo.m_bUsed = false;
			m_cargo -= 54;
		}
		CalcFuel(1.7);
	} else if (*m_pCurPos == *m_pRoman) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pPizer);
		*m_pPortName = "Pizer";
		if (g_LevelOne[8].cargo.m_pszCargo != nullptr && g_LevelOne[8].cargo.m_bUsed) {
			g_LevelOne[8].cargo.m_bUsed = false;
			m_cargo -= 54;
		}
		CalcFuel(4.6);
	} else if (*m_pCurPos == *m_pWeed) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pPizer);
		*m_pPortName = "Pizer";
		if (g_LevelOne[8].cargo.m_pszCargo != nullptr && g_LevelOne[8].cargo.m_bUsed) {
			g_LevelOne[8].cargo.m_bUsed = false;
			m_cargo -= 54;
		}
		CalcFuel(2.2);
	} else {
		BofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::OnBackwater() {
	if (*m_pCurPos == *m_pBackwater) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pBackwater);
		*m_pPortName = "Backwater";
		if (g_LevelTwo[0].cargo.m_pszCargo != nullptr && g_LevelTwo[0].cargo.m_bUsed) {
			g_LevelTwo[0].cargo.m_bUsed = false;
			m_cargo -= 28;
		}
		CalcFuel(0);
	} else if (*m_pCurPos == *m_pUranus) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pBackwater);
		*m_pPortName = "Backwater";
		if (g_LevelTwo[0].cargo.m_pszCargo != nullptr && g_LevelTwo[0].cargo.m_bUsed) {
			g_LevelTwo[0].cargo.m_bUsed = false;
			m_cargo -= 28;
		}
		CalcFuel(4.5);
	} else {
		BofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::OnUranus() {
	if (*m_pCurPos == *m_pUranus) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pUranus);
		*m_pPortName = "Uranus";
		if (g_LevelTwo[1].cargo.m_pszCargo != nullptr && g_LevelTwo[1].cargo.m_bUsed) {
			g_LevelTwo[1].cargo.m_bUsed = false;
			m_cargo -= 14;
		}
		CalcFuel(0);
	} else if (*m_pCurPos == *m_pBackwater) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pUranus);
		*m_pPortName = "Uranus";
		if (g_LevelTwo[1].cargo.m_pszCargo != nullptr && g_LevelTwo[1].cargo.m_bUsed) {
			g_LevelTwo[1].cargo.m_bUsed = false;
			m_cargo -= 14;
		}
		CalcFuel(4.5);
	} else if (*m_pCurPos == *m_pToejam) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pUranus);
		*m_pPortName = "Uranus";
		if (g_LevelTwo[1].cargo.m_pszCargo != nullptr && g_LevelTwo[1].cargo.m_bUsed) {
			g_LevelTwo[1].cargo.m_bUsed = false;
			m_cargo -= 14;
		}
		CalcFuel(2.6);
	} else if (*m_pCurPos == *m_pPustule) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pUranus);
		*m_pPortName = "Uranus";
		if (g_LevelTwo[1].cargo.m_pszCargo != nullptr && g_LevelTwo[1].cargo.m_bUsed) {
			g_LevelTwo[1].cargo.m_bUsed = false;
			m_cargo -= 14;
		}
		CalcFuel(2.9);
	} else {
		BofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::OnToejam() {
	if (*m_pCurPos == *m_pToejam) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pToejam);
		*m_pPortName = "Toe Jam";
		if (g_LevelTwo[2].cargo.m_pszCargo != nullptr && g_LevelTwo[2].cargo.m_bUsed) {
			g_LevelTwo[2].cargo.m_bUsed = false;
			m_cargo -= 45;
		}
		CalcFuel(0);
	} else if (*m_pCurPos == *m_pUranus) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pToejam);
		*m_pPortName = "Toe Jam";
		if (g_LevelTwo[2].cargo.m_pszCargo != nullptr && g_LevelTwo[2].cargo.m_bUsed) {
			g_LevelTwo[2].cargo.m_bUsed = false;
			m_cargo -= 45;
		}
		CalcFuel(2.6);
	} else if (*m_pCurPos == *m_pHellhole) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pToejam);
		*m_pPortName = "Toe Jam";
		if (g_LevelTwo[2].cargo.m_pszCargo != nullptr && g_LevelTwo[2].cargo.m_bUsed) {
			g_LevelTwo[2].cargo.m_bUsed = false;
			m_cargo -= 45;
		}
		CalcFuel(2.0);
	} else if (*m_pCurPos == *m_pPustule) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pToejam);
		*m_pPortName = "Toe Jam";
		if (g_LevelTwo[2].cargo.m_pszCargo != nullptr && g_LevelTwo[2].cargo.m_bUsed) {
			g_LevelTwo[2].cargo.m_bUsed = false;
			m_cargo -= 45;
		}
		CalcFuel(1.1);
	} else if (*m_pCurPos == *m_pSwamprock) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pToejam);
		*m_pPortName = "Toe Jam";
		if (g_LevelTwo[2].cargo.m_pszCargo != nullptr && g_LevelTwo[2].cargo.m_bUsed) {
			g_LevelTwo[2].cargo.m_bUsed = false;
			m_cargo -= 45;
		}
		CalcFuel(1.8);
	} else {
		BofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::OnHellhole() {
	if (*m_pCurPos == *m_pHellhole) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pHellhole);
		*m_pPortName = "Hellhole";
		if (g_LevelTwo[3].cargo.m_pszCargo != nullptr && g_LevelTwo[3].cargo.m_bUsed) {
			g_LevelTwo[3].cargo.m_bUsed = false;
			m_cargo -= 100;
		}
		CalcFuel(1.8);
	} else if (*m_pCurPos == *m_pToejam) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pHellhole);
		*m_pPortName = "Hellhole";
		if (g_LevelTwo[3].cargo.m_pszCargo != nullptr && g_LevelTwo[3].cargo.m_bUsed) {
			g_LevelTwo[3].cargo.m_bUsed = false;
			m_cargo -= 100;
		}
		CalcFuel(2.0);
	} else if (*m_pCurPos == *m_pSwamprock) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pHellhole);
		*m_pPortName = "Hellhole";
		if (g_LevelTwo[3].cargo.m_pszCargo != nullptr && g_LevelTwo[3].cargo.m_bUsed) {
			g_LevelTwo[3].cargo.m_bUsed = false;
			m_cargo -= 100;
		}
		CalcFuel(1.3);
	} else {
		BofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::OnPustule() {
	if (*m_pCurPos == *m_pPustule) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pPustule);
		*m_pPortName = "Pustule";
		CalcFuel(0);
	} else if (*m_pCurPos == *m_pUranus) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pPustule);
		*m_pPortName = "Pustule";
		CalcFuel(2.9);
	} else if (*m_pCurPos == *m_pToejam) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pPustule);
		*m_pPortName = "Pustule";
		CalcFuel(1.1);
	} else if (*m_pCurPos == *m_pSlug) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pPustule);
		*m_pPortName = "Pustule";
		CalcFuel(3.4);
	} else if (*m_pCurPos == *m_pMaggotsnest) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pPustule);
		*m_pPortName = "Pustule";
		CalcFuel(2.3);
	} else {
		BofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::OnSwamprock() {
	if (*m_pCurPos == *m_pSwamprock) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pSwamprock);
		*m_pPortName = "Swamp Rock";
		CalcFuel(0);
	} else if (*m_pCurPos == *m_pToejam) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pSwamprock);
		*m_pPortName = "Swamp Rock";
		CalcFuel(1.8);
	} else if (*m_pCurPos == *m_pHellhole) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		*m_pPortName = "Swamp Rock";
		m_pCurPos = new CBofRect(*m_pSwamprock);
		CalcFuel(1.3);
	} else if (*m_pCurPos == *m_pMaggotsnest) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pSwamprock);
		*m_pPortName = "Swamp Rock";
		CalcFuel(1.9);

	} else {
		BofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::OnSlug() {
	if (*m_pCurPos == *m_pSlug) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pSlug);
		*m_pPortName = "Slug";
		if (g_LevelTwo[6].cargo.m_pszCargo != nullptr && g_LevelTwo[6].cargo.m_bUsed) {
			g_LevelTwo[6].cargo.m_bUsed = false;
			m_cargo -= 75;
		}
		CalcFuel(0);
	} else if (*m_pCurPos == *m_pPustule) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pSlug);
		*m_pPortName = "Slug";
		if (g_LevelTwo[6].cargo.m_pszCargo != nullptr && g_LevelTwo[6].cargo.m_bUsed) {
			g_LevelTwo[6].cargo.m_bUsed = false;
			m_cargo -= 75;
		}
		CalcFuel(3.4);
	} else if (*m_pCurPos == *m_pMaggotsnest) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pSlug);
		*m_pPortName = "Slug";
		if (g_LevelTwo[6].cargo.m_pszCargo != nullptr && g_LevelTwo[6].cargo.m_bUsed) {
			g_LevelTwo[6].cargo.m_bUsed = false;
			m_cargo -= 75;
		}
		CalcFuel(1.7);
	} else {
		BofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::OnMaggotsnest() {
	if (*m_pCurPos == *m_pMaggotsnest) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pMaggotsnest);
		*m_pPortName = "Maggot's Nest";
		CalcFuel(0);
	} else if (*m_pCurPos == *m_pPustule) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pMaggotsnest);
		*m_pPortName = "Maggot's Nest";
		CalcFuel(2.3);
	} else if (*m_pCurPos == *m_pSwamprock) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pMaggotsnest);
		*m_pPortName = "Maggot's Nest";
		CalcFuel(1.9);
	} else if (*m_pCurPos == *m_pSlug) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pMaggotsnest);
		*m_pPortName = "Maggot's Nest";
		CalcFuel(1.7);

	} else {
		BofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::OnPeggleboz() {
	if (*m_pCurPos == *m_pPeggleboz) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pPeggleboz);
		*m_pPortName = "Peggleboz";
		if (g_LevelThree[0].cargo.m_pszCargo != nullptr && g_LevelThree[0].cargo.m_bUsed) {
			g_LevelThree[0].cargo.m_bUsed = false;
			m_cargo -= 99;
		}
		CalcFuel(0);
	} else if (*m_pCurPos == *m_pArcheroids) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pPeggleboz);
		*m_pPortName = "Peggleboz";
		if (g_LevelThree[0].cargo.m_pszCargo != nullptr && g_LevelThree[0].cargo.m_bUsed) {
			g_LevelThree[0].cargo.m_bUsed = false;
			m_cargo -= 99;
		}
		CalcFuel(4.7);
	} else if (*m_pCurPos == *m_pNoVacancy) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pPeggleboz);
		*m_pPortName = "Peggleboz";
		if (g_LevelThree[0].cargo.m_pszCargo != nullptr && g_LevelThree[0].cargo.m_bUsed) {
			g_LevelThree[0].cargo.m_bUsed = false;
			m_cargo -= 99;
		}
		CalcFuel(2.4);
	} else {
		BofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::OnArcheroids() {
	if (*m_pCurPos == *m_pArcheroids) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pArcheroids);
		*m_pPortName = "Archeroids";
		if (g_LevelThree[1].cargo.m_pszCargo != nullptr && g_LevelThree[1].cargo.m_bUsed) {
			g_LevelThree[1].cargo.m_bUsed = false;
			m_cargo -= 8;
		}
		CalcFuel(0);
	} else if (*m_pCurPos == *m_pPeggleboz) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pArcheroids);
		*m_pPortName = "Archeroids";
		if (g_LevelThree[1].cargo.m_pszCargo != nullptr && g_LevelThree[1].cargo.m_bUsed) {
			g_LevelThree[1].cargo.m_bUsed = false;
			m_cargo -= 8;
		}
		CalcFuel(4.7);
	} else if (*m_pCurPos == *m_pPackRat) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pArcheroids);
		*m_pPortName = "Archeroids";
		if (g_LevelThree[1].cargo.m_pszCargo != nullptr && g_LevelThree[1].cargo.m_bUsed) {
			g_LevelThree[1].cargo.m_bUsed = false;
			m_cargo -= 8;
		}
		CalcFuel(3.3);
	} else if (*m_pCurPos == *m_pMankala) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pArcheroids);
		*m_pPortName = "Archeroids";
		if (g_LevelThree[1].cargo.m_pszCargo != nullptr && g_LevelThree[1].cargo.m_bUsed) {
			g_LevelThree[1].cargo.m_bUsed = false;
			m_cargo -= 8;
		}
		CalcFuel(2.6);
	} else if (*m_pCurPos == *m_pBeacon) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pArcheroids);
		*m_pPortName = "Archeroids";
		if (g_LevelThree[1].cargo.m_pszCargo != nullptr && g_LevelThree[1].cargo.m_bUsed) {
			g_LevelThree[1].cargo.m_bUsed = false;
			m_cargo -= 8;
		}
		CalcFuel(1.2);
	} else {
		BofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::OnPackRat() {
	if (*m_pCurPos == *m_pPackRat) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pPackRat);
		*m_pPortName = "Pack-Rat";
		if (g_LevelThree[2].cargo.m_pszCargo != nullptr && g_LevelThree[2].cargo.m_bUsed) {
			g_LevelThree[2].cargo.m_bUsed = false;
			m_cargo -= 24;
		}
		CalcFuel(0);
	} else if (*m_pCurPos == *m_pArcheroids) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pPackRat);
		*m_pPortName = "Pack-Rat";
		if (g_LevelThree[2].cargo.m_pszCargo != nullptr && g_LevelThree[2].cargo.m_bUsed) {
			g_LevelThree[2].cargo.m_bUsed = false;
			m_cargo -= 24;
		}
		CalcFuel(3.3);
	} else if (*m_pCurPos == *m_pBeacon) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pPackRat);
		*m_pPortName = "Pack-Rat";
		if (g_LevelThree[2].cargo.m_pszCargo != nullptr && g_LevelThree[2].cargo.m_bUsed) {
			g_LevelThree[2].cargo.m_bUsed = false;
			m_cargo -= 24;
		}
		CalcFuel(5.2);
	} else {
		BofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::OnBeacon() {
	if (*m_pCurPos == *m_pBeacon) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pBeacon);
		*m_pPortName = "Beacon";
		if (g_LevelThree[3].cargo.m_pszCargo != nullptr && g_LevelThree[3].cargo.m_bUsed) {
			g_LevelThree[3].cargo.m_bUsed = false;
			m_cargo -= 125;
		}
		CalcFuel(0);
	} else if (*m_pCurPos == *m_pArcheroids) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pBeacon);
		*m_pPortName = "Beacon";
		if (g_LevelThree[3].cargo.m_pszCargo != nullptr && g_LevelThree[3].cargo.m_bUsed) {
			g_LevelThree[3].cargo.m_bUsed = false;
			m_cargo -= 125;
		}
		CalcFuel(1.2);
	} else if (*m_pCurPos == *m_pPackRat) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pBeacon);
		*m_pPortName = "Beacon";
		if (g_LevelThree[3].cargo.m_pszCargo != nullptr && g_LevelThree[3].cargo.m_bUsed) {
			g_LevelThree[3].cargo.m_bUsed = false;
			m_cargo -= 125;
		}
		CalcFuel(5.2);
	} else if (*m_pCurPos == *m_pMankala) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pBeacon);
		*m_pPortName = "Beacon";
		if (g_LevelThree[3].cargo.m_pszCargo != nullptr && g_LevelThree[3].cargo.m_bUsed) {
			g_LevelThree[3].cargo.m_bUsed = false;
			m_cargo -= 125;
		}
		CalcFuel(3.8);
	} else if (*m_pCurPos == *m_pLife) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pBeacon);
		*m_pPortName = "Beacon";
		if (g_LevelThree[3].cargo.m_pszCargo != nullptr && g_LevelThree[3].cargo.m_bUsed) {
			g_LevelThree[3].cargo.m_bUsed = false;
			m_cargo -= 125;
		}
		CalcFuel(3.1);
	} else if (*m_pCurPos == *m_pFuge) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pBeacon);
		*m_pPortName = "Beacon";
		if (g_LevelThree[3].cargo.m_pszCargo != nullptr && g_LevelThree[3].cargo.m_bUsed) {
			g_LevelThree[3].cargo.m_bUsed = false;
			m_cargo -= 125;
		}
		CalcFuel(1.8);
	} else {
		BofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::OnMankala() {
	if (*m_pCurPos == *m_pMankala) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pMankala);
		*m_pPortName = "Mankala";
		CalcFuel(0);
	} else if (*m_pCurPos == *m_pArcheroids) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pMankala);
		*m_pPortName = "Mankala";
		CalcFuel(2.6);
	} else if (*m_pCurPos == *m_pNoVacancy) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pMankala);
		*m_pPortName = "Mankala";
		CalcFuel(1.6);
	} else if (*m_pCurPos == *m_pBeacon) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pMankala);
		*m_pPortName = "Mankala";
		CalcFuel(3.8);
	} else if (*m_pCurPos == *m_pGarfunkel) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pMankala);
		*m_pPortName = "Mankala";
		CalcFuel(1.7);
	} else {
		BofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::OnLife() {
	if (*m_pCurPos == *m_pLife) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pLife);
		*m_pPortName = "Life";
		CalcFuel(0);
	} else if (*m_pCurPos == *m_pFuge) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pLife);
		*m_pPortName = "Life";
		CalcFuel(2.9);
	} else if (*m_pCurPos == *m_pBeacon) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pLife);
		*m_pPortName = "Life";
		CalcFuel(3.1);
	} else if (*m_pCurPos == *m_pBattlefish) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pLife);
		*m_pPortName = "Life";
		CalcFuel(1.1);
	} else {
		BofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::OnFuge() {
	if (*m_pCurPos == *m_pFuge) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pFuge);
		*m_pPortName = "Fuge";
		if (g_LevelThree[6].cargo.m_pszCargo != nullptr && g_LevelThree[6].cargo.m_bUsed) {
			g_LevelThree[6].cargo.m_bUsed = false;
			m_cargo -= 10;
		}
		CalcFuel(0);
	} else if (*m_pCurPos == *m_pGarfunkel) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pFuge);
		*m_pPortName = "Fuge";
		if (g_LevelThree[6].cargo.m_pszCargo != nullptr && g_LevelThree[6].cargo.m_bUsed) {
			g_LevelThree[6].cargo.m_bUsed = false;
			m_cargo -= 10;
		}
		CalcFuel(1.5);
	} else if (*m_pCurPos == *m_pBeacon) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pFuge);
		*m_pPortName = "Fuge";
		if (g_LevelThree[6].cargo.m_pszCargo != nullptr && g_LevelThree[6].cargo.m_bUsed) {
			g_LevelThree[6].cargo.m_bUsed = false;
			m_cargo -= 10;
		}
		CalcFuel(1.8);
	} else if (*m_pCurPos == *m_pLife) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pFuge);
		*m_pPortName = "Fuge";
		if (g_LevelThree[6].cargo.m_pszCargo != nullptr && g_LevelThree[6].cargo.m_bUsed) {
			g_LevelThree[6].cargo.m_bUsed = false;
			m_cargo -= 10;
		}
		CalcFuel(2.9);
	} else if (*m_pCurPos == *m_pBattlefish) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pFuge);
		*m_pPortName = "Fuge";
		if (g_LevelThree[6].cargo.m_pszCargo != nullptr && g_LevelThree[6].cargo.m_bUsed) {
			g_LevelThree[6].cargo.m_bUsed = false;
			m_cargo -= 10;
		}
		CalcFuel(2.0);
	} else {
		BofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::OnGarfunkel() {
	if (*m_pCurPos == *m_pGarfunkel) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pGarfunkel);
		*m_pPortName = "Garfunkel";
		CalcFuel(0);
	} else if (*m_pCurPos == *m_pMankala) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pGarfunkel);
		*m_pPortName = "Garfunkel";
		CalcFuel(1.7);
	} else if (*m_pCurPos == *m_pFuge) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pGarfunkel);
		*m_pPortName = "Garfunkel";
		CalcFuel(1.5);
	} else {
		BofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::OnBattlefish() {
	if (*m_pCurPos == *m_pBattlefish) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pBattlefish);
		*m_pPortName = "Battlefish";
		if (g_LevelThree[8].cargo.m_pszCargo != nullptr && g_LevelThree[8].cargo.m_bUsed) {
			g_LevelThree[8].cargo.m_bUsed = false;
			m_cargo -= 17;
		}
		CalcFuel(0);
	} else if (*m_pCurPos == *m_pFuge) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pBattlefish);
		*m_pPortName = "Battlefish";
		if (g_LevelThree[8].cargo.m_pszCargo != nullptr && g_LevelThree[8].cargo.m_bUsed) {
			g_LevelThree[8].cargo.m_bUsed = false;
			m_cargo -= 17;
		}
		CalcFuel(2);

	} else if (*m_pCurPos == *m_pLife) {

		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pBattlefish);
		*m_pPortName = "Battlefish";
		if (g_LevelThree[8].cargo.m_pszCargo != nullptr && g_LevelThree[8].cargo.m_bUsed) {
			g_LevelThree[8].cargo.m_bUsed = false;
			m_cargo -= 17;
		}
		CalcFuel(1.1);
	} else {
		BofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

void CNavWindow::OnNoVacancy() {
	if (*m_pCurPos == *m_pNoVacancy) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pNoVacancy);
		*m_pPortName = "No Vacancy";
		CalcFuel(0);
	} else if (*m_pCurPos == *m_pMankala) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pNoVacancy);
		*m_pPortName = "No Vacancy";
		CalcFuel(1.6);
	} else if (*m_pCurPos == *m_pPeggleboz) {
		delete m_pCurPos;
		m_pCurPos = nullptr;
		m_pCurPos = new CBofRect(*m_pNoVacancy);
		*m_pPortName = "No Vacancy";
		CalcFuel(2.4);
	} else {
		BofMessageBox("Destination unavailable !!", "Test Warning");
	}
}

} // namespace SpaceBar
} // namespace Bagel
