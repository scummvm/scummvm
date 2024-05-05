
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
	static int _nWieldCursor;

	CBagObject *_pCurrObj;
	int _nObjects; // The number of currently active objects

public:
	CBagWield(CBofWindow *pParent = nullptr, const CBofRect &xRect = CBofRect());
	virtual ~CBagWield();
	static void initialize() {
		_nWieldCursor = -1;
	}

	CBagObject *getCurrObj() const {
		return _pCurrObj;
	}
	CBagObject *setCurrObj(CBagObject *pObj) {
		return _pCurrObj = pObj;
	}

	static int getWieldCursor() {
		return _nWieldCursor;
	}
	static void setWieldCursor(int n) {
		_nWieldCursor = n;
	}

	virtual ErrorCode update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect = nullptr, int nMaskColor = -1);

	virtual ErrorCode loadFile(const CBofString &sFile);
	virtual bool onObjInteraction(CBagObject *pObj, CBagStorageDev *pSDev);

	virtual ErrorCode attach();
	virtual ErrorCode detach();

	virtual ErrorCode activateLocalObject(CBagObject *pObj);
	virtual ErrorCode deactivateLocalObject(CBagObject *pObj);
};

} // namespace Bagel

#endif
