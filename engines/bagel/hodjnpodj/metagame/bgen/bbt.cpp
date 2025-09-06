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

#include "bagel/hodjnpodj/metagame/bgen/stdafx.h"
#include "bagel/hodjnpodj/metagame/bgen/bbt.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Gtl {

///DEFS bbt.h


//* CBbtMgr::~CBbtMgr -- destructor
CBbtMgr::~CBbtMgr() {
}


//* CBbtMgr::LinkButton -- link button into button manager
bool CBbtMgr::LinkButton(CBbutton FAR *lpBbt,
	CBgbObject FAR *lpcBgbObject1,
	CBgbObject FAR *lpcBgbObject2)
	// lpBbt -- button object
	// lpcBgbObject1, lpcBgbObject2 -- up/down graphic objects
	// returns: true if error, false otherwise
{
	JXENTER(CBbtMgr::LinkButton);
	int iError = 0;        // error code

	if (lpcBgbObject1)
		lpBbt->m_lpcBgbObject1 = lpcBgbObject1;
	if (lpcBgbObject2)
		lpBbt->m_lpcBgbObject2 = lpcBgbObject2;

	if (!lpBbt->m_bChained) {
		lpBbt->m_lpBbtNext = m_lpBbtChain;
		m_lpBbtChain = lpBbt;
		lpBbt->m_bChained = true;
	}

	// cleanup:

	JXELEAVE(CBbtMgr::LinkButton);
	RETURN(iError != 0);
}


//* CBbtMgr::AcceptClick -- process mouse click or mouse move
int CBbtMgr::AcceptClick(CRPoint crPoint, int iClickType)
// returns: id of affected button, 0 otherwise
{
	JXENTER(CBbtMgr::AcceptClick);
	int iId = 0;   // return value
	CBbutton FAR *lpPtBbt = nullptr, FAR *lpBbt;
	bool bButtonUp = false, bButtonDown = false;

	for (lpBbt = m_lpBbtChain; lpBbt; lpBbt = lpBbt->m_lpBbtNext)
		if (lpBbt->PtInButton(crPoint))
			lpPtBbt = lpBbt;   // find button containing click point

	switch (iClickType) {
	case CLICK_LDOWN:
	case CLICK_LMOVE:
		bButtonDown = true;
		break;

	case CLICK_LUP:
		bButtonUp = (lpPtBbt && lpPtBbt == m_lpDownBbt);
		break;

		//	case CLICK_UMOVE:
		//	    bButtonUp = (lpPtBbt && lpPtBbt == m_lpDownBbt) ;
		//	    break ;

	default:
		break;
	}

	if (bButtonUp && lpPtBbt)
		iId = lpPtBbt->m_iBbtId;   // return id of button up

	if (bButtonDown && lpPtBbt) {
		iId = lpPtBbt->m_iBbtId;   // return id of button down
		MoveDown(lpPtBbt);
	}

	if (!bButtonDown)
		MoveDown();    // no more button down

	// cleanup:

	JXELEAVE(CBbtMgr::AcceptClick);
	RETURN(iId);
}

//* CBbtMgr::MoveDown -- move button down
bool CBbtMgr::MoveDown(CBbutton FAR *lpDownBbt)
// lpDownBbt -- button to move down, if any
// returns: true if error, false otherwise
{
	JXENTER(CBbtMgr::MoveDown);
	int iError = 0;        // error code

	if (m_lpDownBbt && m_lpDownBbt != lpDownBbt) {
		m_lpBgbMgr->PaintBitmapObject(m_lpDownBbt->m_lpcBgbObject1);
		// paint button in up position
		m_lpDownBbt = nullptr;    // no more down button
	}

	if (lpDownBbt && m_lpDownBbt != lpDownBbt)
		// if we want to move a button down
	{
		m_lpBgbMgr->PaintBitmapObject(lpDownBbt->m_lpcBgbObject2);
		// paint button in down position
		m_lpDownBbt = lpDownBbt;   // new down button
	}

	// cleanup:

	JXELEAVE(CBbtMgr::MoveDown);
	RETURN(iError != 0);
}



//* CBbtMgr::GetCurrentBitmap -- get current bitmap for button
CBgbObject FAR *CBbtMgr::GetCurrentBitmap(CBbutton *xpBbt)
// xpBbt -- button to be processed
// returns: pointer to Boffo bitmap object
{
	JXENTER(CBbtMgr::GetCurrentBitmap);
	CBgbObject FAR *lpcBgbObject = nullptr;  // return value

	if (xpBbt) {
		if (xpBbt == m_lpDownBbt)
			lpcBgbObject = xpBbt->m_lpcBgbObject2; // down bitmap
		else
			lpcBgbObject = xpBbt->m_lpcBgbObject1; // up bitmap
	}

	//cleanup:

	JXELEAVE(CBbtMgr::GetCurrentBitmap);
	RETURN(lpcBgbObject);
}

} // namespace Gtl
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
