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

#include "bagel/spacebar/baglib/chat_wnd.h"
#include "bagel/spacebar/baglib/text_object.h"

namespace Bagel {
namespace SpaceBar {

CBagChatWnd::CBagChatWnd() : CBagStorageDevWnd() {
}

ErrorCode CBagChatWnd::attach() {
	assert(isValidObject(this));

	if (CBagStorageDevWnd::attach() == ERR_NONE) {
		show();
		invalidateRect(nullptr);
		updateWindow();
	}

	return _errCode;
}

void CBagChatWnd::onTimer(uint32 /*nTimerId*/) {
	assert(isValidObject(this));
}

void CBagChatWnd::onMouseMove(uint32 flags, CBofPoint *pos, void *) {
	assert(isValidObject(this));
	assert(pos != nullptr);

	// Use default behavior 1st
	CBagStorageDevWnd::onMouseMove(flags, pos);

	// If over a chat choice, then highlight it
	CBofList<CBagObject *> *bagObjectList = getObjectList();

	// Run through background object list and find if the cursor is over an object
	if (bagObjectList != nullptr) {
		// Go through list backwards to find the 1st top-most object
		CBofListNode<CBagObject *> *currNode = bagObjectList->getTail();
		while (currNode != nullptr) {
			CBagObject *currNodeItem = currNode->getNodeItem();

			if (currNodeItem->getType() == TEXT_OBJ) {
				((CBagTextObject *)currNodeItem)->setColor(4);
				currNodeItem->setDirty(true);

				if (currNodeItem->isAttached() && currNodeItem->isInside(*pos)) {
					((CBagTextObject *)currNodeItem)->setColor(8);
				}
			}

			currNode = currNode->_pPrev;
		}
	}
}

} // namespace SpaceBar
} // namespace Bagel
