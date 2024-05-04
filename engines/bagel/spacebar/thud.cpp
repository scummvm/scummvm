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

int SBarThud::_nThudCursor = -1;

SBarThud::SBarThud(CBofWindow *pParent, const CBofRect &xRect) :
	CBagStorageDevBmp(pParent, xRect) {
	_xSDevType = SDEV_WIELD;
	_xYouBmp = nullptr;
	_nObjects = 0;         // This should be changed on the attach
}

SBarThud::~SBarThud() {
}

ErrorCode SBarThud::loadFile(const CBofString &sFile) {
	ErrorCode error = CBagStorageDev::loadFile(sFile);
	return error;
}

ErrorCode SBarThud::attach() {
	int        nActiveObj = 0;
	ErrorCode rc = CBagStorageDevBmp::attach();

	// save a copy of the you icon
	_xYouBmp = new CBofBitmap(GetBackgroundName());

	if ((_xYouBmp == nullptr) || (_xYouBmp->height() <= 0) || (_xYouBmp->width() <= 0)) {
		bofMessageBox("You icon in Thud: Background Opened Failed", __FILE__);
		return ERR_FOPEN;
	}

	// Take care of objects being held
	for (int i = 0; i < GetObjectCount(); ++i) {
		CBagObject *pObj = GetObjectByPos(i);
		if (pObj->isActive() && (pObj->GetType() == BOFSPRITEOBJ || pObj->GetType() == BOFBMPOBJ))
			nActiveObj++;

		// Detach all activated objects after the first
		if (nActiveObj > 1) {
			pObj->detach();
			nActiveObj--;
			// This can be removed later
			assert(nActiveObj < 2);
		}
	}

	// The weild has been loaded with too many active objects
	_nObjects = nActiveObj;

	return rc;
}

ErrorCode SBarThud::detach() {
	if (_xYouBmp) {
		delete _xYouBmp;
		_xYouBmp = nullptr;
	}

	// Write one function in sdevbmp
	CBagStorageDev::detach();
	return CBagBmpObject::detach();
}


bool SBarThud::onObjInteraction(CBagObject *pObj, CBagStorageDev *pSDev) {
	CBofString sObjName = pObj->GetRefName();
	if (sObjName.isEmpty())
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



ErrorCode SBarThud::loadFileFromStream(CBagIfstream &fpInput, const CBofString &sWldName, bool bAttach) {
	return CBagStorageDevBmp::loadFileFromStream(fpInput, sWldName, bAttach);
}


//
// This functionality now only allows 1 object to be active in the Thud sdev
// at a time.  If the default functionality should be to transfer the object to
// your stash and Thud the new object then a transfer command must be issued
// here.
//


ErrorCode SBarThud::activateLocalObject(CBagObject *pObj) {
	if (pObj == nullptr)
		return ERR_UNKNOWN;

	// Thud can only activate one object at a time
	if (_nObjects)
		return ERR_UNKNOWN;

	return CBagStorageDev::activateLocalObject(pObj);
}

ErrorCode SBarThud::deactivateLocalObject(CBagObject *pObj) {
	if (_nObjects != 1)
		return ERR_NONE;

	if (pObj->GetType() == SPRITEOBJ) {
		setThudCursor(-1);

	}

	if (pObj->GetType() == BOFSPRITEOBJ || pObj->GetType() == SPRITEOBJ) {
		_nObjects--;
	}

	if (_xYouBmp && getBackground())
		_xYouBmp->paint(getBackground(), 0, 0);

	CBagMenu::SetUniversalObjectList(nullptr);
	return CBagStorageDev::deactivateLocalObject(pObj);
}

ErrorCode SBarThud::activateLocalObject(const CBofString &sName) {
	return CBagStorageDev::activateLocalObject(sName);
}

ErrorCode SBarThud::deactivateLocalObject(const CBofString &sName) {
	return CBagStorageDev::deactivateLocalObject(sName);
}

} // namespace SpaceBar
} // namespace Bagel
