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

#include "bagel/spacebar/thud.h"
#include "bagel/baglib/menu_dlg.h"
#include "bagel/baglib/sprite_object.h"
#include "bagel/boflib/app.h"

namespace Bagel {
namespace SpaceBar {

int SBarThud::m_nThudCursor = -1;

SBarThud::SBarThud(CBofWindow *pParent, const CBofRect &xRect) :
	CBagStorageDevBmp(pParent, xRect) {
	m_xSDevType = SDEV_WIELD;
	m_xYouBmp = nullptr;
	m_nObjects = 0;         // This should be changed on the attach
}

SBarThud::~SBarThud() {
}

ErrorCode SBarThud::LoadFile(const CBofString &sFile) {
	ErrorCode error = CBagStorageDev::LoadFile(sFile);
	return error;
}

ErrorCode SBarThud::attach() {
	int        nActiveObj = 0;
	ErrorCode rc = CBagStorageDevBmp::attach();

	// save a copy of the you icon
	m_xYouBmp = new CBofBitmap(GetBackgroundName());

	if ((m_xYouBmp == nullptr) || (m_xYouBmp->height() <= 0) || (m_xYouBmp->width() <= 0)) {
		BofMessageBox("You icon in Thud: Background Opened Failed", __FILE__);
		return ERR_FOPEN;
	}

	// Take care of objects being held
	for (int i = 0; i < GetObjectCount(); ++i) {
		CBagObject *pObj = GetObjectByPos(i);
		if (pObj->IsActive() && (pObj->GetType() == BOFSPRITEOBJ || pObj->GetType() == BOFBMPOBJ))
			nActiveObj++;

		// Detach all activated objects after the first
		if (nActiveObj > 1) {
			pObj->detach();
			nActiveObj--;
			// This can be removed later
			Assert(nActiveObj < 2);
		}
	}

	// The weild has been loaded with too many active objects
	m_nObjects = nActiveObj;

	return rc;
}

ErrorCode SBarThud::detach() {
	if (m_xYouBmp) {
		delete m_xYouBmp;
		m_xYouBmp = nullptr;
	}

	// Write one function in sdevbmp
	CBagStorageDev::detach();
	return CBagBmpObject::detach();
}


bool SBarThud::OnObjInteraction(CBagObject *pObj, CBagStorageDev *pSDev) {
	CBofString sObjName = pObj->GetRefName();
	if (sObjName.IsEmpty())
		return false;

	// Find the storage device
	if (activateLocalObject(sObjName) != ERR_NONE)
		return false;
	if (pSDev->deactivateLocalObject(sObjName) != ERR_NONE) {
		deactivateLocalObject(sObjName);
		return false;
	}

	return true;
}



ErrorCode SBarThud::LoadFileFromStream(CBagIfstream &fpInput, const CBofString &sWldName, bool bAttach) {
	return CBagStorageDevBmp::LoadFileFromStream(fpInput, sWldName, bAttach);
}


//
// This functionality now only allows 1 object to be active in the Thud sdev
// at a time.  If the default functionality should be to transfer the object to
// your stash and Thud the new object then a transfer command must be issued
// here.
//


ErrorCode SBarThud::ActivateLocalObject(CBagObject *pObj) {
	if (pObj == nullptr)
		return ERR_UNKNOWN;

	// Thud can only activate one object at a time
	if (m_nObjects)
		return ERR_UNKNOWN;

	return CBagStorageDev::ActivateLocalObject(pObj);
}

ErrorCode SBarThud::DeactivateLocalObject(CBagObject *pObj) {
	if (m_nObjects != 1)
		return ERR_NONE;

	if (pObj->GetType() == SPRITEOBJ) {
		SetThudCursor(-1);

	}

	if (pObj->GetType() == BOFSPRITEOBJ || pObj->GetType() == SPRITEOBJ) {
		m_nObjects--;
	}

	if (m_xYouBmp && getBackground())
		m_xYouBmp->paint(getBackground(), 0, 0);

	CBagMenu::SetUniversalObjectList(nullptr);
	return CBagStorageDev::DeactivateLocalObject(pObj);
}

ErrorCode SBarThud::activateLocalObject(const CBofString &sName) {
	return CBagStorageDev::activateLocalObject(sName);
}

ErrorCode SBarThud::deactivateLocalObject(const CBofString &sName) {
	return CBagStorageDev::deactivateLocalObject(sName);
}

} // namespace SpaceBar
} // namespace Bagel
