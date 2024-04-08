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

#include "bagel/baglib/wield.h"
#include "bagel/baglib/menu_dlg.h"
#include "bagel/baglib/sprite_object.h"
#include "bagel/baglib/bagel.h"

namespace Bagel {

int CBagWield::m_nWieldCursor = -1;

CBagWield::CBagWield(CBofWindow *pParent, const CBofRect &xRect) :
	CBagStorageDevBmp(pParent, xRect) {
	m_xSDevType = SDEV_WIELD;
	m_nObjects = 0;         // This should be changed on the attach
	m_pCurrObj = nullptr;
}


CBagWield::~CBagWield() {
}

ERROR_CODE CBagWield::LoadFile(const CBofString &sFile) {
	ERROR_CODE error;

	error = CBagStorageDev::LoadFile(sFile);

	return error;
}

ERROR_CODE CBagWield::Attach() {
	CBagStorageDevBmp::Attach();

	// Take care of objects being held
	int i, nObjects;

	nObjects = 0;
	m_nObjects = 0;

	for (i = 0; i < GetObjectCount(); ++i) {
		CBagObject *pObj = GetObjectByPos(i);

		if (pObj->IsActive() && (pObj->GetType() == BOFSPRITEOBJ || pObj->GetType() == BOFBMPOBJ)) {
			if (nObjects == 0) {
				nObjects++;
				ActivateLocalObject(pObj);

			} else {
				pObj->Detach();
			}
		}
	}

	return m_errCode;
}

ERROR_CODE CBagWield::Detach() {
	CBagStorageDev::Detach();
	return CBagBmpObject::Detach();
}

bool CBagWield::OnObjInteraction(CBagObject *pObj, CBagStorageDev *pSDev) {
	CBofString sObjName = pObj->GetRefName();

	if (sObjName.IsEmpty())
		return FALSE;

	// Find the storage device
	if (CBagStorageDev::ActivateLocalObject(sObjName) != ERR_NONE)
		return FALSE;

	if (pSDev->DeactivateLocalObject(sObjName) != ERR_NONE) {
		CBagStorageDev::DeactivateLocalObject(sObjName);
		return FALSE;
	}

	return true;
}

ERROR_CODE CBagWield::ActivateLocalObject(CBagObject *pObj) {
	ERROR_CODE errCode;

	errCode = ERR_UNKNOWN;

	if (pObj != nullptr) {
		// Since we can only hold one object at a time, put the object that
		// the user is currently holding into their stash (Inventory), and then
		// put the new object that they are trying to pick up into their wield.
		if (m_nObjects == 1) {
			CBagObject *pPrevObj;

			if ((pPrevObj = GetCurrObj()) != nullptr) {
				// Move current object to stash
				SDEVMNGR->MoveObject("INV_WLD", GetName(), pPrevObj->GetRefName());

			} else {
				ReportError(ERR_UNKNOWN, "Wielded Object has been lost");
			}

			// There are no objects in wield
			m_nObjects = 0;
		}

		// Add to wield
		if (pObj->GetMenuPtr() != nullptr) {
			CBagMenu::SetUniversalObjectList(pObj->GetMenuPtr()->GetObjectList());
		}

		if (pObj->GetType() == SPRITEOBJ) {
			SetCurrObj(pObj);
			m_nObjects++;
			CBagSpriteObject *pSpObj = (CBagSpriteObject *)pObj;
			SetWieldCursor(pSpObj->GetWieldCursor());
		}

		// Fix for sometimes getting the wield menu instead of thud's order
		// menu.  Do this by causing the wielded item to not have a location
		// on the You icon. (It's rectangle will appear empty)
		CBofPoint cPos(2056, 2056);
		pObj->SetPosition(cPos);
		pObj->SetFloating(FALSE);

		errCode = CBagStorageDev::ActivateLocalObject(pObj);
	}

	return errCode;
}

ERROR_CODE CBagWield::DeactivateLocalObject(CBagObject *pObj) {
	if (m_nObjects == 1) {

		if (pObj->GetType() == SPRITEOBJ) {
			SetWieldCursor(-1);
			m_nObjects--;
		}

		CBagMenu::SetUniversalObjectList(nullptr);
		CBagStorageDev::DeactivateLocalObject(pObj);
		SetCurrObj(nullptr);
	}

	return m_errCode;
}


ERROR_CODE CBagWield::Update(CBofBitmap *pBmp, CBofPoint /*cPoint*/, CBofRect * /*pSrcRect*/, int /*nMaskColor*/) {
	Assert(pBmp != nullptr);

	// If this object is visible
	if (IsVisible() && IsAttached()) {
		CBofBitmap *pYouIcon;

		pYouIcon = GetBackground();
		if (pYouIcon != nullptr) {
			pYouIcon->Paint(pBmp, GetPosition().x, GetPosition().y, nullptr, DEFAULT_CHROMA_COLOR);
			SetDirty(FALSE);
		}
	}

	// Set the firstpaint flag and attach objects to allow for immediate run objects to run
	if (m_bFirstPaint) {
		m_bFirstPaint = FALSE;
		AttachActiveObjects();
	}

	return ERR_NONE;
}

} // namespace Bagel
