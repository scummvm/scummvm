
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

#ifndef BAGEL_SPACEBAR_THUD_H
#define BAGEL_SPACEBAR_THUD_H

#include "bagel/baglib/storage_dev_bmp.h"

namespace Bagel {
namespace SpaceBar {

class SBarThud : public CBagStorageDevBmp {
private:
	CBagObject *pCurrObj;
	int m_nObjects;         // The number of currently active objects
	CBofBitmap *m_xYouBmp;
	static int  m_nThudCursor;
public:
	SBarThud(CBofWindow *pParent = nullptr, const CBofRect &xRect = CBofRect());
	virtual ~SBarThud();

	CBagObject *GetCurrObj() {
		return pCurrObj;
	}
	CBagObject *SetCurrObj(CBagObject *pObj) {
		return pCurrObj = pObj;
	}

	static int      GetThudCursor() {
		return m_nThudCursor;
	}
	static void     SetThudCursor(int n) {
		m_nThudCursor = n;
	}

	virtual ERROR_CODE LoadFile(const CBofString &sFile);
	virtual ERROR_CODE LoadFileFromStream(bof_ifstream &fpInput, const CBofString &sWldName, BOOL bAttach);
	virtual BOOL       OnObjInteraction(CBagObject *pObj, CBagStorageDev *pSDev);

	virtual ERROR_CODE  Attach();
	virtual ERROR_CODE  Detach();

	virtual ERROR_CODE ActivateLocalObject(CBagObject *pObj);
	virtual ERROR_CODE DeactivateLocalObject(CBagObject *pObj);

	virtual ERROR_CODE  ActivateLocalObject(const CBofString &sName);
	virtual ERROR_CODE  DeactivateLocalObject(const CBofString &sName);
};

} // namespace SpaceBar
} // namespace Bagel

#endif
