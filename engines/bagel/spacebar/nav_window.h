
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
#include "bagel/boflib/gui/window.h"
#include "bagel/boflib/app.h"
#include "bagel/boflib/gui/button.h"
#include "bagel/boflib/gfx/sprite.h"
#include "bagel/boflib/sound.h"

namespace Bagel {
namespace SpaceBar {

struct NCARGO {
	const CHAR *m_pszCargo;
	INT Weight;
	bool m_bUsed;
};

struct NPLANET {
	const CHAR *Name;
	INT Left;
	INT Top;
	INT Right;
	INT Bottom;
	NCARGO cargo;
};


class CNavWindow : public CBagStorageDevWnd {
public:
	CNavWindow();

#if BOF_DEBUG
	virtual ~CNavWindow();
#endif

	ERROR_CODE Attach() override;
	ERROR_CODE Detach() override;

	void OnBofButton(CBofObject *pButton, INT nState) override;

protected:
	void pause();
	void CalcFuel(double hf);

	void OnLButtonDown(uint32 nFlags, CBofPoint *pPoint, void * = nullptr) override;
	void OnKeyHit(uint32 lKey, uint32 lRepCount) override;
	void OnTimer(uint32 tId) override;
	void OnPaint(CBofRect *pRect) override;
	void OnMainLoop() override;

	// Split up the onmousedown method
	void RefreshData();     // Added func to just paint text

	void OnPinna();
	void OnHpoctyl();
	void OnArmpit();
	void OnYzore();
	void OnBulbus();
	void OnDingle();
	void OnKarkas();
	void OnFruufnia();
	void OnWilbur();
	void OnMcKelvey();
	void OnMedge();
	void OnWall();
	void OnWoo();
	void OnRoman();
	void OnWeed();
	void OnBok();
	void OnPizer();
	void OnBackwater();
	void OnUranus();
	void OnToejam();
	void OnHellhole();
	void OnPustule();
	void OnSwamprock();
	void OnSlug();
	void OnMaggotsnest();
	void OnPeggleboz();
	void OnArcheroids();
	void OnPackRat();
	void OnBeacon();
	void OnMankala();
	void OnLife();
	void OnFuge();
	void OnGarfunkel();
	void OnBattlefish();
	void OnNoVacancy();

	// Data
	//
	INT m_level;
	INT m_fuel;
	INT m_cargo;
	INT m_ship;
	CBofRect *m_pPinna;
	CBofRect *m_pHpoctyl;
	CBofRect *m_pArmpit;
	CBofRect *m_pYzore;
	CBofRect *m_pBulbus;
	CBofRect *m_pDingle;
	CBofRect *m_pKarkas;
	CBofRect *m_pFruufnia;
	CBofBmpButton *m_pButtons[2];
	CBofPalette *m_pPal;
	CBofPalette *m_pOldPal;
	CBofSprite *m_pMap;
	CBofSprite *m_pCurLoc;
	CBofBitmap *m_pNewMap;
	CBofBitmap *bmptwo;
	CBofRect *m_pCurPos;
	CBofString *m_pPortName;
	CBofRect *m_pWilbur;
	CBofRect *m_pMcKelvey;
	CBofRect *m_pMedge;
	CBofRect *m_pWall;
	CBofRect *m_pWoo;
	CBofRect *m_pRoman;
	CBofRect *m_pWeed;
	CBofRect *m_pBok;
	CBofRect *m_pPizer;
	CBofRect *m_pBackwater;
	CBofRect *m_pUranus;
	CBofRect *m_pToejam;
	CBofRect *m_pHellhole;
	CBofRect *m_pPustule;
	CBofRect *m_pSwamprock;
	CBofRect *m_pSlug;
	CBofRect *m_pMaggotsnest;
	CBofRect *m_pPeggleboz;
	CBofRect *m_pArcheroids;
	CBofRect *m_pPackRat;
	CBofRect *m_pBeacon;
	CBofRect *m_pMankala;
	CBofRect *m_pLife;
	CBofRect *m_pFuge;
	CBofRect *m_pGarfunkel;
	CBofRect *m_pBattlefish;
	CBofRect *m_pNoVacancy;

	NPLANET *m_pLevel;
	bool m_bNavAttached;
};

} // namespace SpaceBar
} // namespace Bagel

#endif
