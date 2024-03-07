
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

#ifndef BAGEL_SPACEBAR_APP_WINDOW_H
#define BAGEL_SPACEBAR_APP_WINDOW_H

#include "bagel/boflib/gui/window.h"

namespace Bagel {

/**
 * The actual window that is used as our application
 */
class CBofAppWindow : public CBofWindow {
protected:
	virtual VOID OnPaint(CBofRect *pRect);
	virtual VOID OnKeyHit(ULONG lKey, ULONG lRepCount);

	virtual VOID OnMouseMove(UINT nFlags, CBofPoint *pPoint, void * = nullptr);

	virtual VOID OnLButtonDown(UINT nFlags, CBofPoint *pPoint, void * = nullptr);
	virtual VOID OnLButtonUp(UINT nFlags, CBofPoint *pPoint, void * = nullptr);
	virtual VOID OnLButtonDblClk(UINT nFlags, CBofPoint *pPoint);

	virtual VOID OnRButtonDown(UINT nFlags, CBofPoint *pPoint);
	virtual VOID OnRButtonUp(UINT nFlags, CBofPoint *pPoint);
	virtual VOID OnRButtonDblClk(UINT nFlags, CBofPoint *pPoint);
};

} // namespace Bagel

#endif
