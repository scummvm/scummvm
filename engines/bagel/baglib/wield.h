
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

#ifndef BAGEL_BAGLIB_WIELD_H
#define BAGEL_BAGLIB_WIELD_H

#include "bagel/baglib/storage_dev_bmp.h"

namespace Bagel {

class CBagWield : public CBagStorageDevBmp {
private:
	static INT m_nWieldCursor;

	CBagObject *m_pCurrObj;
	INT m_nObjects; // The number of currently active objects

public:
	CBagWield(CBofWindow *pParent = nullptr, const CBofRect &xRect = CBofRect());
	virtual ~CBagWield();
	static void initialize() {
		m_nWieldCursor = -1;
	}

	CBagObject *GetCurrObj() const {
		return m_pCurrObj;
	}
	CBagObject *SetCurrObj(CBagObject *pObj) {
		return m_pCurrObj = pObj;
	}

	static INT GetWieldCursor() {
		return m_nWieldCursor;
	}
	static void SetWieldCursor(INT n) {
		m_nWieldCursor = n;
	}

	virtual ERROR_CODE Update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect = nullptr, INT nMaskColor = -1);

	virtual ERROR_CODE LoadFile(const CBofString &sFile);
	virtual bool OnObjInteraction(CBagObject *pObj, CBagStorageDev *pSDev);

	virtual ERROR_CODE Attach();
	virtual ERROR_CODE Detach();

	virtual ERROR_CODE ActivateLocalObject(CBagObject *pObj);
	virtual ERROR_CODE DeactivateLocalObject(CBagObject *pObj);
};

} // namespace Bagel

#endif
