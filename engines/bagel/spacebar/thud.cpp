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

ERROR_CODE SBarThud::LoadFile(const CBofString &sFile) {
	ERROR_CODE error;

	error = CBagStorageDev::LoadFile(sFile);

	return error;

}

ERROR_CODE SBarThud::Attach() {
	ERROR_CODE rc;
	int        nActiveObj = 0;
	rc = CBagStorageDevBmp::Attach();

	// save a copy of the you icon
	//CBofBitmap *m_xYouBmp
	m_xYouBmp = new CBofBitmap(GetBackgroundName());

	if ((m_xYouBmp == nullptr) || (m_xYouBmp->Height() <= 0) || (m_xYouBmp->Width() <= 0)) {
		BofMessageBox("You icon in Thud: Background Opened Failed", __FILE__);
		return ERR_FOPEN;
	} else {
	}

	// Take care of objects being held
	for (int i = 0; i < GetObjectCount(); ++i) {
		CBagObject *pObj = GetObjectByPos(i);
		if (pObj->IsActive() && (pObj->GetType() == BOFSPRITEOBJ || pObj->GetType() == BOFBMPOBJ))
			nActiveObj++;
		//Detach all activated objects after the first
		if (nActiveObj > 1) {
			pObj->Detach();
			nActiveObj--;
			// This can be removed latere
			Assert(nActiveObj < 2);
		}
	}

	// The weild has been loaded with too many active objects
	m_nObjects = nActiveObj;

	return rc;
}

ERROR_CODE SBarThud::Detach() {
	if (m_xYouBmp) {
		delete m_xYouBmp;
		m_xYouBmp = nullptr;
	}

	// write one function in sdevbmp
	CBagStorageDev::Detach();
	return CBagBmpObject::Detach();
}


bool SBarThud::OnObjInteraction(CBagObject *pObj, CBagStorageDev *pSDev) {
	//AddObject(pObj);
	CBofString sObjName = pObj->GetRefName();
	if (sObjName.IsEmpty())
		return FALSE;

	// Find the storage device
	if (ActivateLocalObject(sObjName) != ERR_NONE)
		return FALSE;
	if (pSDev->DeactivateLocalObject(sObjName) != ERR_NONE) {
		DeactivateLocalObject(sObjName);
		return FALSE;
	}

	return TRUE;
}



ERROR_CODE SBarThud::LoadFileFromStream(bof_ifstream &fpInput, const CBofString &sWldName, bool bAttach) {
	return CBagStorageDevBmp::LoadFileFromStream(fpInput, sWldName, bAttach);
}


//
// This functionality now only allows 1 object to be active in the Thud sdev
// at a time.  If the default functionality should be to transfer the object to
// your stash and Thud the new object then a transfer command must be issued
// here.
//


ERROR_CODE SBarThud::ActivateLocalObject(CBagObject *pObj) {
	if (pObj == nullptr)
		return ERR_UNKNOWN;

	// Thud can only activate one object at a time
	if (m_nObjects)
		return ERR_UNKNOWN;

	/*
	    CBagMenu::SetUniversalObjectList(pObj->GetMenuPtr()->GetObjectList());

	    if (pObj->GetType()==BOFSPRITEOBJ || pObj->GetType()==SPRITEOBJ) {
	        m_nObjects++;
	        CBagSpriteObject *pSpObj = (CBagSpriteObject *)pObj;
	        SetThudCursor(pSpObj->GetWeildCursor());
	    }
	*/
	return CBagStorageDev::ActivateLocalObject(pObj);
}

ERROR_CODE SBarThud::DeactivateLocalObject(CBagObject *pObj) {
	if (m_nObjects != 1)
		return ERR_NONE;

	if (pObj->GetType() == SPRITEOBJ) {
		SetThudCursor(-1);

	}

	if (pObj->GetType() == BOFSPRITEOBJ || pObj->GetType() == SPRITEOBJ) {
		m_nObjects--;
	}

	if (m_xYouBmp && GetBackground())
		m_xYouBmp->Paint(GetBackground(), 0, 0);

	CBagMenu::SetUniversalObjectList(nullptr);
	return CBagStorageDev::DeactivateLocalObject(pObj);
}


ERROR_CODE SBarThud::ActivateLocalObject(const CBofString &sName) {
	return CBagStorageDev::ActivateLocalObject(sName);
}

ERROR_CODE SBarThud::DeactivateLocalObject(const CBofString &sName) {
	return CBagStorageDev::DeactivateLocalObject(sName);
}

} // namespace SpaceBar
} // namespace Bagel
