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

#include "common/textconsole.h"
#include "bagel/mfc/afxwin.h"

namespace Bagel {
namespace MFC {

IMPLEMENT_DYNAMIC(CFrameWnd, CWnd)
BEGIN_MESSAGE_MAP(CFrameWnd, CWnd)
ON_WM_CLOSE()
END_MESSAGE_MAP()

void CFrameWnd::OnClose() {
	if (this == AfxGetApp()->m_pMainWnd) {
		AfxGetApp()->PopActiveWindow();
	}
}

HMENU CFrameWnd::GetMenu() const {
	error("TODO: CFrameWnd::GetMenu");
}

void CFrameWnd::RecalcLayout(BOOL bNotify) {
	error("TODO: CFrameWnd::RecalcLayout");
}

BOOL CFrameWnd::RepositionBars(UINT nIDFirst, UINT nIDLast,
                               UINT nIDLeftOver, UINT nFlag, LPRECT lpRectParam,
                               LPCRECT lpRectClient, BOOL bStretch) {
	error("TODO: CFrameWnd::RepositionBars");
}

} // namespace MFC
} // namespace Bagel
