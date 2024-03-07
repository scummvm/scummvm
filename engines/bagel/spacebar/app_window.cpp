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

#include "bagel/spacebar/app_window.h"
#include "bagel/boflib/gui/display_object.h"
#include "bagel/boflib/gui/display_window.h"

namespace Bagel {

VOID CBofAppWindow::OnPaint(CBofRect *pRect) {
	Assert(IsValidObject(this));
	Assert(pRect != nullptr);

	CBofBitmap cBmp(pRect->Width(), pRect->Height(), nullptr);

	cBmp.FillRect(nullptr, COLOR_BLACK);

	cBmp.Paint(this, pRect);

	// Must now repaint all display objects that have been invalidated
	//
	CBofDisplayObject::AddToDirtyRect(pRect);
	CBofDisplayObject::UpdateDirtyRect();
}


VOID CBofAppWindow::OnMouseMove(UINT nFlags, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

	CBofDisplayWindow::HandleMouseMove(nFlags, *pPoint);
}


VOID CBofAppWindow::OnLButtonDown(UINT nFlags, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

	CBofDisplayWindow::HandleLButtonDown(nFlags, *pPoint);
}

VOID CBofAppWindow::OnLButtonUp(UINT nFlags, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

	CBofDisplayWindow::HandleLButtonUp(nFlags, *pPoint);
}

VOID CBofAppWindow::OnLButtonDblClk(UINT nFlags, CBofPoint *pPoint) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

	CBofDisplayWindow::HandleLButtonDblClk(nFlags, *pPoint);
}

VOID CBofAppWindow::OnRButtonDown(UINT nFlags, CBofPoint *pPoint) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

	CBofDisplayWindow::HandleRButtonDown(nFlags, *pPoint);
}

VOID CBofAppWindow::OnRButtonUp(UINT nFlags, CBofPoint *pPoint) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

	CBofDisplayWindow::HandleRButtonUp(nFlags, *pPoint);
}

VOID CBofAppWindow::OnRButtonDblClk(UINT nFlags, CBofPoint *pPoint) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

	CBofDisplayWindow::HandleRButtonDblClk(nFlags, *pPoint);
}

VOID CBofAppWindow::OnKeyHit(ULONG lKey, ULONG lRepCount) {
	Assert(IsValidObject(this));

	CBofDisplayWindow::HandleKeyHit(lKey, lRepCount);
}

} // namespace Bagel
