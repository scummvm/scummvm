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

#include "bagel/mfc/wnd.h"
#include "bagel/bagel.h"

namespace Bagel {
namespace MFC {

CDC *CWnd::GetDC() {
	return new CDC();
}

void CWnd::ReleaseDC(CDC *dc) {
	delete dc;
}

CWnd *CWnd::FromHandlePermanent(HWND hWnd) {
	error("TODO: CWnd::FromHandlePermanent");
	return nullptr;
}

void CWnd::InvalidateRect(const CRect *r, bool bErase) {
	error("TODO: CWnd::InvalidateRect");
}

bool CWnd::PaintDIB(HDC, CRect *lpDestRect, HDIB hSrc,
	CRect *lpSrcRect, CPalette *hPal) {
	Graphics::Screen &screen = *g_engine->_screen;
	screen.blitFrom(*hSrc, *lpSrcRect, *lpDestRect);
	return true;
}

BOOL CWnd::SubclassDlgItem(UINT nID, CWnd *pParent) {
	error("TODO: CBitmapButton::SubclassDlgItem");
	return false;
}

void CWnd::ShowWindow(int) {
}

void CWnd::UpdateWindow() {
}

void CWnd::SetActiveWindow() {
}

CWnd *CWnd::GetNextDlgGroupItem(CWnd *pWndCtl, BOOL bPrevious) const {
	error("TODO: CWnd::GetNextDlgGroupItem");
	return nullptr;
}

long CWnd::SendMessage(UINT message, WPARAM wParam, LPARAM lParam) {
	error("TODO: CWnd::SendMessage");
	return 0;
}

} // namespace MFC
} // namespace Bagel
