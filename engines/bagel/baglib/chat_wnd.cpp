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

#include "bagel/baglib/chat_wnd.h"
#include "bagel/baglib/text_object.h"

namespace Bagel {



CBagChatWnd::CBagChatWnd() : CBagStorageDevWnd() {
}

ERROR_CODE CBagChatWnd::Attach() {
	Assert(IsValidObject(this));

	if (CBagStorageDevWnd::Attach() == ERR_NONE) {
		Show();
		InvalidateRect(nullptr);
		UpdateWindow();
	}

	return m_errCode;
}

VOID CBagChatWnd::OnTimer(UINT /*nTimerId*/) {
	Assert(IsValidObject(this));
}

VOID CBagChatWnd::OnMouseMove(UINT nFlags, CBofPoint *pPos) {
	Assert(IsValidObject(this));
	Assert(pPos != nullptr);

	// Use default behavoir 1st
	CBagStorageDevWnd::OnMouseMove(nFlags, pPos);

	// If over a chat choice, then highlight it
	CBofList<CBagObject *> *pList;

	// Run thru background object list and find if the cursor is over an object
	if ((pList = GetObjectList()) != nullptr) {
		CBofListNode<CBagObject *> *pNode;
		CBagObject *pObj;

		// Go thru list backwards to find the 1st top-most object
		pNode = pList->GetTail();
		while (pNode != nullptr) {
			pObj = pNode->GetNodeItem();

			if (pObj->GetType() == TEXTOBJ) {
				((CBagTextObject *)pObj)->SetColor(4);
				pObj->SetDirty(TRUE);

				if (pObj->IsAttached() && pObj->IsInside(*pPos)) {
					((CBagTextObject *)pObj)->SetColor(8);
				}
			}

			pNode = pNode->m_pPrev;
		}
	}
}

} // namespace Bagel
