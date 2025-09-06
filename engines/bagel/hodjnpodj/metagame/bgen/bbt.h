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

#ifndef BAGEL_METAGAME_BGEN_BBT_H
#define BAGEL_METAGAME_BGEN_BBT_H

#include "bagel/hodjnpodj/metagame/bgen/bgb.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Gtl {

#define BBTT_DIB 1
#define BBTT_SPRITE 2

class CBbutton;

// CBbtMgr -- boffo games button manager
class CBbtMgr {
public:
	char m_cStartData;
	CBgbMgr FAR *m_lpBgbMgr;
	CBbutton FAR *m_lpBbtChain;  // chain of bbt objects
	CBbutton FAR *m_lpDownBbt;
	char m_cEndData;

public:
	CBbtMgr(CBgbMgr FAR *lpBgbMgr = nullptr) {
		memset(&m_cStartData, 0,
			&m_cEndData - &m_cStartData);
		m_lpBgbMgr = lpBgbMgr;
	}

	~CBbtMgr();
	//- LinkButton -- link button into button manager
	bool LinkButton(CBbutton FAR *lpBbt,
		CBgbObject FAR *lpcBgbObject1,
		CBgbObject FAR *lpcBgbObject2);
	//- AcceptClick -- process mouse click or mouse move
	int AcceptClick(CRPoint crPoint, int iClickType);
	//- MoveDown -- move button down
	bool MoveDown(CBbutton FAR *lpDownBbt PDFT(nullptr));
	//- GetCurrentBitmap -- get current bitmap for button
	CBgbObject FAR *GetCurrentBitmap(CBbutton *xpBbt);
};


// CBbutton -- Boffo button object
class FAR CBbutton {
	friend class CBbtMgr;
public:
	char m_cStartData;
	int m_iBbtId;      // button id
	CBbutton FAR *m_lpBbtNext;     // pointer to next in chain
	bool m_bChained : 1;    // on m_xBbtChain
	bool m_bNoDelete;      // not allocated with "new"
	bool m_bDummy : 1;      // dummy object -- no bitmap
	bool m_bInit : 1;       // flag: object is initialized
	CBgbObject FAR *m_lpcBgbObject1, FAR *m_lpcBgbObject2;
	// graphics objects for on/off
	char m_cEndData;

public:
	CBbutton(int iId = 0) {
		memset(&m_cStartData, 0,
			&m_cEndData - &m_cStartData);
		m_iBbtId = iId;
	}
	bool PtInButton(const CRPoint &crPoint) const {
		return m_lpcBgbObject1 ?
			m_lpcBgbObject1->GetRect().PtInRect(crPoint) : false;
	}
};

} // namespace Gtl
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif // __bbt_H__
