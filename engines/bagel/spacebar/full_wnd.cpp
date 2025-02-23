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

#include "bagel/spacebar/full_wnd.h"
#include "bagel/spacebar/baglib/bagel.h"
#include "bagel/spacebar/baglib/master_win.h"
#include "bagel/spacebar/baglib/pan_window.h"

namespace Bagel {
namespace SpaceBar {

CBagObject *SBarFullWnd::_pWieldedObject = nullptr;

SBarFullWnd::SBarFullWnd() {
	_bAllowEventWorld = true;
}

ErrorCode SBarFullWnd::attach() {
	assert(isValidObject(this));

	// If we have something wielded, put it on hold for now.
	if (CBagPanWindow::_pWieldBmp != nullptr) {
		_pWieldedObject = CBagPanWindow::_pWieldBmp->getCurrObj();
		if (_pWieldedObject != nullptr) {
			g_SDevManager->removeObject(CBagPanWindow::_pWieldBmp->getName(), _pWieldedObject->getRefName());
		}
	}

	if (CBagStorageDevWnd::attach() == ERR_NONE) {
		if (!_bAllowEventWorld) {
			g_waitOKFl = false;
		}

		show();
		invalidateRect(nullptr);
		updateWindow();
	}

	return _errCode;
}

ErrorCode SBarFullWnd::detach() {
	CBagStorageDevWnd::detach();

	if (_pWieldedObject) {
		g_SDevManager->addObject(CBagPanWindow::_pWieldBmp->getName(), _pWieldedObject->getRefName());
		_pWieldedObject = nullptr;
	}

	return ERR_NONE;
}

void SBarFullWnd::onTimer(uint32 nTimerId) {
	assert(isValidObject(this));

	// If allowing EVENT_WLD to execute
	if (_bAllowEventWorld) {
		CBagStorageDevWnd::onTimer(nTimerId);

	} else {
		// Otherwise, don't allow turns to pass
		g_waitOKFl = false;
	}
}

void SBarFullWnd::onMouseMove(uint32 nFlags, CBofPoint *pPoint, void *) {
	CBagStorageDevWnd::onMouseMove(nFlags, pPoint);
}

} // namespace SpaceBar
} // namespace Bagel
