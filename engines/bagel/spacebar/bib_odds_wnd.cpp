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

#include "bagel/spacebar/bib_odds_wnd.h"
#include "bagel/spacebar/bibble_window.h"
#include "bagel/spacebar/spacebar.h"
#include "bagel/baglib/master_win.h"
#include "bagel/baglib/bagel.h"
#include "bagel/baglib/pan_window.h"
#include "bagel/boflib/std_keys.h"

namespace Bagel {
namespace SpaceBar {

CBagObject *SBarBibOddsWnd::m_pWieldedObject;

void SBarBibOddsWnd::initialize() {
	m_pWieldedObject = nullptr;
}

SBarBibOddsWnd::SBarBibOddsWnd() : CBagChatWnd() {
}

SBarBibOddsWnd::~SBarBibOddsWnd() {
}

ErrorCode SBarBibOddsWnd::detach() {
	for (int i = 0; i < BIBBLE_NUM_BET_AREAS; i++) {

		// The sprite object start in the script at 500
		CBagObject *pObj = GetObject(500 + i);

		if (pObj != nullptr) {
			int nPayIdx = pObj->getState();
			g_engine->g_cBetAreas[i].m_nPayOff1 = PAY_OFFS[nPayIdx].m_nPay1;
			g_engine->g_cBetAreas[i].m_nPayOff2 = PAY_OFFS[nPayIdx].m_nPay2;
		}
	}

	if (m_pWieldedObject) {
		SDEVMNGR->AddObject(CBagPanWindow::m_pWieldBmp->GetName(), m_pWieldedObject->GetRefName());
		m_pWieldedObject = nullptr;
	}

	return CBagChatWnd::detach();
}

void SBarBibOddsWnd::onKeyHit(uint32 lKey, uint32 lRepCount) {
	CBagVar *pVar = VARMNGR->GetVariable("TORSOSTATE");

	if (pVar != nullptr) {
		CBofString StateStr = pVar->GetValue();
		if (StateStr == "MAINMENU") {
			switch (lKey) {
			case BKEY_1:
				pVar->SetValue("VIDINFO");
				AttachActiveObjects();
				break;
			case BKEY_2:
				pVar->SetValue("SETBIBBLE");
				AttachActiveObjects();
				break;
			case BKEY_3:
				pVar->SetValue("BADCODE");
				AttachActiveObjects();
				break;
			case BKEY_4:
				pVar->SetValue("TIPS");
				AttachActiveObjects();
				break;
			case BKEY_5:
				Close();
				break;
			default:
				break;
			}

		} else if (StateStr == "SETBIBBLE") {
			switch (lKey) {
			case BKEY_1:
				pVar->SetValue("SETBONK");
				AttachActiveObjects();
				break;
			case BKEY_2:
				pVar->SetValue("SETBAB");
				AttachActiveObjects();
				break;
			default:
				break;
			}
		}
	}

	CBagChatWnd::onKeyHit(lKey, lRepCount);
}

void SBarBibOddsWnd::onMouseMove(uint32 nFlags, CBofPoint *pPoint, void *) {
	CBagStorageDevWnd::onMouseMove(nFlags, pPoint);
}

ErrorCode SBarBibOddsWnd::attach() {
	Assert(IsValidObject(this));

	// If we have something wielded, put it on hold for now.
	if (CBagPanWindow::m_pWieldBmp != nullptr) {
		if ((m_pWieldedObject = CBagPanWindow::m_pWieldBmp->GetCurrObj()) != nullptr) {
			SDEVMNGR->RemoveObject(CBagPanWindow::m_pWieldBmp->GetName(), m_pWieldedObject->GetRefName());
		}
	}

	// Don't call CBagChatWnd::attach() - We are overriding it's behavior
	if (CBagStorageDevWnd::attach() == ERR_NONE) {
		Show();
		InvalidateRect(nullptr);
		UpdateWindow();
	}

	return m_errCode;
}

} // namespace SpaceBar
} // namespace Bagel
