
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

#ifndef BAGEL_SPACEBAR_NAV_WINDOW_H
#define BAGEL_SPACEBAR_NAV_WINDOW_H

#include "bagel/boflib/gui/window.h"
#include "bagel/baglib/storage_dev_win.h"
#include "bagel/boflib/gui/text_box.h"
#include "bagel/boflib/gui/button.h"
#include "bagel/boflib/gfx/sprite.h"
#include "bagel/boflib/sound.h"

namespace Bagel {
namespace SpaceBar {

struct NCARGO {
	const char *_pszCargo;
	int Weight;
	bool _bUsed;
};

struct NPLANET {
	const char *Name;
	int Left;
	int Top;
	int Right;
	int Bottom;
	NCARGO cargo;
};


class CNavWindow : public CBagStorageDevWnd {
public:
	CNavWindow();

	ErrorCode attach() override;
	ErrorCode detach() override;

	void onBofButton(CBofObject *pObject, int nState) override;

protected:
	void pause();
	void calcFuel(double hf);

	void onLButtonDown(uint32 nFlags, CBofPoint *pPoint, void * = nullptr) override;
	void onKeyHit(uint32 lKey, uint32 lRepCount) override;
	void onTimer(uint32 tId) override;
	void onPaint(CBofRect *pRect) override;
	void onMainLoop() override;

	// Split up the onMouseDown method
	void refreshData();     // Added func to just paint text

	void onPinna();
	void onHpoctyl();
	void onArmpit();
	void onYzore();
	void onBulbus();
	void onDingle();
	void onKarkas();
	void onFruufnia();
	void onWilbur();
	void onMcKelvey();
	void onMedge();
	void onWall();
	void onWoo();
	void onRoman();
	void onWeed();
	void onBok();
	void onPizer();
	void onBackwater();
	void onUranus();
	void onToejam();
	void onHellhole();
	void onPustule();
	void onSwamprock();
	void onSlug();
	void onMaggotsnest();
	void onPeggleboz();
	void onArcheroids();
	void onPackRat();
	void onBeacon();
	void onMankala();
	void onLife();
	void onFuge();
	void onGarfunkel();
	void onBattlefish();
	void onNoVacancy();

	// Data
	//
	int _level;
	int _fuel;
	int _cargo;
	int _ship;
	CBofRect *_pPinna;
	CBofRect *_pHpoctyl;
	CBofRect *_pArmpit;
	CBofRect *_pYzore;
	CBofRect *_pBulbus;
	CBofRect *_pDingle;
	CBofRect *_pKarkas;
	CBofRect *_pFruufnia;
	CBofBmpButton *_pButtons[2];
	CBofPalette *_pPal;
	CBofPalette *_pOldPal;
	CBofSprite *_pMap;
	CBofSprite *_pCurLoc;
	CBofBitmap *_pNewMap;
	CBofBitmap *_bmpTwo;
	CBofRect *_pCurPos;
	CBofString *_pPortName;
	CBofRect *_pWilbur;
	CBofRect *_pMcKelvey;
	CBofRect *_pMedge;
	CBofRect *_pWall;
	CBofRect *_pWoo;
	CBofRect *_pRoman;
	CBofRect *_pWeed;
	CBofRect *_pBok;
	CBofRect *_pPizer;
	CBofRect *_pBackwater;
	CBofRect *_pUranus;
	CBofRect *_pToejam;
	CBofRect *_pHellhole;
	CBofRect *_pPustule;
	CBofRect *_pSwamprock;
	CBofRect *_pSlug;
	CBofRect *_pMaggotsNest;
	CBofRect *_pPeggleboz;
	CBofRect *_pArcheroids;
	CBofRect *_pPackRat;
	CBofRect *_pBeacon;
	CBofRect *_pMankala;
	CBofRect *_pLife;
	CBofRect *_pFuge;
	CBofRect *_pGarfunkel;
	CBofRect *_pBattleFish;
	CBofRect *_pNoVacancy;

	NPLANET *_pLevel;
	bool _bNavAttached;
};

} // namespace SpaceBar
} // namespace Bagel

#endif
