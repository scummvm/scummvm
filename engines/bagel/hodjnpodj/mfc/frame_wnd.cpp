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

#include "bagel/hodjnpodj/mfc/frame_wnd.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {

void CFrameWnd::ShowWindow(int) {
}

void CFrameWnd::UpdateWindow() {
}

void CFrameWnd::SetActiveWindow() {
}

CDC *CFrameWnd::GetDC() {
	return new CDC();
}

void CFrameWnd::ReleaseDC(CDC *dc) {
	delete dc;
}

void CFrameWnd::GetClientRect(CBofRect &r) {
	r.left = r.top = 0;
	r.right = GAME_WIDTH;
	r.bottom = GAME_HEIGHT;
}

bool CFrameWnd::PaintDIB(HDC, CBofRect *lpDestRect, HDIB hSrc,
		CBofRect *lpSrcRect, CBofPalette *hPal) {
	Graphics::Screen &screen = *g_engine->_screen;
	screen.blitFrom(*hSrc, *lpSrcRect, *lpDestRect);
	return true;
}

} // namespace HodjNPodj
} // namespace Bagel
