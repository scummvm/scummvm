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

#include "bagel/hodjnpodj/mazedoom/main_window.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {
namespace MazeDoom {

CMainWindow::CMainWindow() : CFrameWnd() {}
void CMainWindow::MovePlayer(CPoint point) {}
void CMainWindow::GetNewCursor() {}
void CMainWindow::NewGame() {}
void CMainWindow::SplashScreen() {}
void CMainWindow::OnSoundNotify(CSound *pSound) {}
bool CMainWindow::OnCommand(uint16 wParam, int32 lParam) { return false; }
void CMainWindow::OnPaint() {}
void CMainWindow::OnChar(uint nChar, uint nRepCnt, uint nFlags) {}
void CMainWindow::OnSysChar(uint nChar, uint nRepCnt, uint nFlags) {}
void CMainWindow::OnSysKeyDown(uint nChar, uint nRepCnt, uint nFlags) {}
void CMainWindow::OnKeyDown(uint nChar, uint nRepCnt, uint nFlags) {}
void CMainWindow::OnTimer(uint nIDEvent) {}
void CMainWindow::OnLButtonDown(uint nFlags, CPoint point) {}
void CMainWindow::OnRButtonDown(uint nFlags, CPoint point) {}
void CMainWindow::OnLButtonUp(uint nFlags, CPoint point) {}
void CMainWindow::OnMouseMove(uint nFlags, CPoint point) {}
void CMainWindow::OnClose() {}
long CMainWindow::OnMCINotify(uint16, int32) { return 0; }
long CMainWindow::OnMMIONotify(uint16, int32) { return 0; }

} // namespace MazeDoom
} // namespace HodjNPodj
} // namespace Bagel
