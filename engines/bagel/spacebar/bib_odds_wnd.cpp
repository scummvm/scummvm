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
#include "bagel/spacebar/baglib/master_win.h"
#include "bagel/spacebar/baglib/bagel.h"
#include "bagel/spacebar/baglib/pan_window.h"
#include "bagel/spacebar/boflib/std_keys.h"

namespace Bagel {
namespace SpaceBar {

CBagObject *SBarBibOddsWnd::_wieldedObject;

void SBarBibOddsWnd::initialize() {
	_wieldedObject = nullptr;
}

SBarBibOddsWnd::SBarBibOddsWnd() : CBagChatWnd() {
}

SBarBibOddsWnd::~SBarBibOddsWnd() {
}

ErrorCode SBarBibOddsWnd::detach() {
	for (int i = 0; i < BIBBLE_NUM_BET_AREAS; i++) {

		// The sprite object start in the script at 500
		CBagObject *pObj = getObject(500 + i);

		if (pObj != nullptr) {
			int nPayIdx = pObj->getState();
			g_engine->g_cBetAreas[i]._nPayOff1 = PAY_OFFS[nPayIdx]._nPay1;
			g_engine->g_cBetAreas[i]._nPayOff2 = PAY_OFFS[nPayIdx]._nPay2;
		}
	}

	if (_wieldedObject) {
		g_SDevManager->addObject(CBagPanWindow::_pWieldBmp->getName(), _wieldedObject->getRefName());
		_wieldedObject = nullptr;
	}

	return CBagChatWnd::detach();
}

void SBarBibOddsWnd::onKeyHit(uint32 lKey, uint32 lRepCount) {
	CBagVar *pVar = g_VarManager->getVariable("TORSOSTATE");

	if (pVar != nullptr) {
		CBofString StateStr = pVar->getValue();
		if (StateStr == "MAINMENU") {
			switch (lKey) {
			case BKEY_1:
				pVar->setValue("VIDINFO");
				attachActiveObjects();
				break;
			case BKEY_2:
				pVar->setValue("SETBIBBLE");
				attachActiveObjects();
				break;
			case BKEY_3:
				pVar->setValue("BADCODE");
				attachActiveObjects();
				break;
			case BKEY_4:
				pVar->setValue("TIPS");
				attachActiveObjects();
				break;
			case BKEY_5:
				close();
				break;
			default:
				break;
			}

		} else if (StateStr == "SETBIBBLE") {
			switch (lKey) {
			case BKEY_1:
				pVar->setValue("SETBONK");
				attachActiveObjects();
				break;
			case BKEY_2:
				pVar->setValue("SETBAB");
				attachActiveObjects();
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
	assert(isValidObject(this));

	// If we have something wielded, put it on hold for now.
	if (CBagPanWindow::_pWieldBmp != nullptr) {
		_wieldedObject = CBagPanWindow::_pWieldBmp->getCurrObj();
		if (_wieldedObject != nullptr) {
			g_SDevManager->removeObject(CBagPanWindow::_pWieldBmp->getName(), _wieldedObject->getRefName());
		}
	}

	// Don't call CBagChatWnd::attach() - We are overriding it's behavior
	if (CBagStorageDevWnd::attach() == ERR_NONE) {
		show();
		invalidateRect(nullptr);
		updateWindow();
	}

	return _errCode;
}

} // namespace SpaceBar
} // namespace Bagel
