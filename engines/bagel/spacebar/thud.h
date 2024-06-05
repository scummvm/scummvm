
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
	CBagObject *_currentObj;
	int _nObjects;         // The number of currently active objects
	CBofBitmap *_xYouBmp;
	static int  _nThudCursor;
public:
	SBarThud(CBofWindow *pParent = nullptr, const CBofRect &xRect = CBofRect());
	virtual ~SBarThud();

	CBagObject *getCurrentObj() {
		return _currentObj;
	}
	CBagObject *setCurrentObj(CBagObject *pObj) {
		return _currentObj = pObj;
	}

	static int      getThudCursor() {
		return _nThudCursor;
	}
	static void     setThudCursor(int n) {
		_nThudCursor = n;
	}

	ErrorCode loadFile(const CBofString &sFile) override;
	ErrorCode loadFileFromStream(CBagIfstream &fpInput, const CBofString &sWldName, bool bAttach) override;
	bool       onObjInteraction(CBagObject *pObj, CBagStorageDev *pSDev) override;

	ErrorCode  attach() override;
	ErrorCode  detach() override;

	ErrorCode activateLocalObject(CBagObject *pObj) override;
	ErrorCode deactivateLocalObject(CBagObject *pObj) override;

	ErrorCode  activateLocalObject(const CBofString &sName) override;
	ErrorCode  deactivateLocalObject(const CBofString &sName) override;
};

} // namespace SpaceBar
} // namespace Bagel

#endif
