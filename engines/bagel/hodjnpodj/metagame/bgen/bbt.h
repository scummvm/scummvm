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

#ifndef HODJNPODJ_METAGAME_BGEN_BBT_H
#define HODJNPODJ_METAGAME_BGEN_BBT_H

#include "bagel/hodjnpodj/metagame/bgen/bgb.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

class CBbutton;

// CBbtMgr -- boffo games button manager
class CBbtMgr {
private:
	/**
	 * Move button down
	 */
	bool MoveDown(CBbutton *lpDownBbt = nullptr);

public:
	CBgbMgr *m_lpBgbMgr;
	CBbutton *m_lpBbtChain = nullptr; // chain of bbt objects
	CBbutton *m_lpDownBbt = nullptr;

	// methods
public:
	CBbtMgr(CBgbMgr *lpBgbMgr = nullptr) {
		m_lpBgbMgr = lpBgbMgr;
	}
	~CBbtMgr();

	/**
	 * Link button into button manager
	 * @param lpBbt		Button object
	 * @param lpcBgbObject1		Up graphic object
	 * @param lpcBgbObject2		Down graphic objects
	 * @returns			true if error, false otherwise
	 */
	bool LinkButton(CBbutton *lpBbt,
		CBgbObject *lpcBgbObject1,
		CBgbObject *lpcBgbObject2);

	/**
	 * Process mouse click or mouse move
	 */
	int AcceptClick(CRPoint crPoint, int iClickType);

	/**
	 * Get current bitmap for button
	 */
	CBgbObject *GetCurrentBitmap(CBbutton *xpBbt);
};

/**
 * Boffo button object
 */
class CBbutton {
	friend class CBbtMgr;
public:
	int m_iBbtId = 0;		// button id
	CBbutton *m_lpBbtNext = nullptr;	// pointer to next in chain
	bool m_bChained : 1;		// on m_xBbtChain
	bool m_bNoDelete;		// not allocated with "new"
	bool m_bDummy : 1;		// dummy object -- no bitmap
	bool m_bInit : 1;		// flag: object is initialized
	CBgbObject *m_lpcBgbObject1 = nullptr,
		*m_lpcBgbObject2 = nullptr;
	// graphics objects for on/off

public:
	CBbutton(int iId = 0);

	bool PtInButton(CRPoint crPoint) const {
		return m_lpcBgbObject1 ?
			m_lpcBgbObject1->GetRect().PtInRect(crPoint) : false;
	}
};

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
