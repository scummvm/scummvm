
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

#ifndef BAGEL_BAGLIB_ZOOM_PDA_H
#define BAGEL_BAGLIB_ZOOM_PDA_H

#include "bagel/baglib/storage_dev_win.h"
#include "bagel/baglib/base_pda.h"

namespace Bagel {

class SBZoomPda : public CBagStorageDevWnd, public SBBasePda {
public:
	/**
	 * Constructor
	 * @param pParent       Pointer to the parent window
	 * @param xPoint        Upper-left corner of PDA in parent window
	 * @param bActivated    State of PDA whe constructed (optional)
	 */
	SBZoomPda(CBofWindow *pParent = nullptr, const CBofRect &xRect = CBofRect(), bool bActivated = true);
	static void initialize();

	virtual ErrorCode Attach();
	virtual ErrorCode Detach();

	virtual ErrorCode LoadFile(const CBofString &sFile);
	virtual void OnMouseMove(uint32 nFlags, CBofPoint *, void * = nullptr);

	/**
	 * Called to overload specific types of sprite objects
	 * @retrn       Pointer to the new object
	 */
	CBagObject *OnNewButtonObject(const CBofString &sInit);

	virtual void OnMainLoop();

	void OnLButtonUp(uint32 nFlags, CBofPoint *xPoint, void * = nullptr);
	virtual ErrorCode OnRender(CBofBitmap *pBmp, CBofRect *pRect = nullptr);

	virtual ErrorCode AttachActiveObjects();
	virtual ErrorCode DetachActiveObjects();
};

} // namespace Bagel

#endif
