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

#include "bagel/spacebar/main_link_object.h"
#include "bagel/spacebar/master_win.h"
#include "bagel/spacebar/main_window.h"

namespace Bagel {
namespace SpaceBar {

CMainLinkObject::CMainLinkObject()
	: CBagLinkObject() {
	_bClickedOn = false;
	_bClickedResize = false;
}


void CMainLinkObject::onLButtonUp(uint32 nFlags, CBofPoint *xPoint, void *info) {
	CMainWindow *pWnd = (CMainWindow *)info;

	if (pWnd && pWnd->gameMode() == CMainWindow::VRPLAYMODE) {
		if (CBagMasterWin::getFlyThru()) {
			CBofPoint loc = getSrcLoc();
			pWnd->rotateTo(loc, 12);
		}

		// Set the link position for the storage device we are about to jump to
		CBagStorageDev *pDestWin = g_SDevManager->getStorageDevice(getFileName());
		if (pDestWin != nullptr) {
			pDestWin->setloadFilePos(getDestLoc());
		}

		CBagLinkObject::onLButtonUp(nFlags, xPoint);
	}
}

} // namespace SpaceBar
} // namespace Bagel
