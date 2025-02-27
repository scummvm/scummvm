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

#include "bagel/hodjnpodj/libs/main_menu.h"

namespace Bagel {
namespace HodjNPodj {

#define IDD_OPTIONS_DIALOG 0, 0, 131, 110

CMainMenu::CMainMenu(CWnd *pParent, CPalette *pPalette, UINT, FPFUNC, char *,
		const char *pWavFileName, LPGAMESTRUCT pGameParams) :
		CBmpDialog(pParent, pPalette, IDD_OPTIONS_DIALOG, "art/OSCROLL.BMP") {
}

void CMainMenu::ClearDialogImage() {
}
void CMainMenu::DoDataExchange(CDataExchange *pDX) {
}
BOOL CMainMenu::OnInitDialog() {
	return false;
}
void CMainMenu::OnOK() {
}
void CMainMenu::OnCancel() {
}
BOOL CMainMenu::OnEraseBkgnd(CDC *pDC) {
	return false;
}
void CMainMenu::OnPaint() {
}
void CMainMenu::OnDestroy() {
}
void CMainMenu::OnClickedRules() {
}
void CMainMenu::OnClickedNewgame() {
}
void CMainMenu::OnClickedOptions() {
}
void CMainMenu::OnClickedAudio() {
}
void CMainMenu::OnClickedReturn() {
}
void CMainMenu::OnClickedQuit() {
}

} // namespace HodjNPodj
} // namespace Bagel
