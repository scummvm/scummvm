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

#include "bagel/hodjnpodj/hodjnpodj.h"
#include "bagel/hodjnpodj/mazedoom/game_exe.h"

namespace Bagel {
namespace HodjNPodj {
namespace MazeDoom {

bool CTheApp::run() {
	bool success = false;
#if 0
	// Hook gray dialogs (was default in MFC V1)
	SetDialogBkColor();
#endif
	GAMESTRUCT &pGameInfo = g_engine->gGameInfo;

	pGameInfo.lCrowns = 0;
	pGameInfo.lScore = 0;
	pGameInfo.nSkillLevel = SKILLLEVEL_MEDIUM;
	pGameInfo.bSoundEffectsEnabled = true;
	pGameInfo.bMusicEnabled = true;
	pGameInfo.bPlayingMetagame = false;
	pGameInfo.bPlayingHodj = true;

#if 0
	CMainWindow *m_pMainWnd;
	if ((m_pMainWnd = new CMainWindow) != NULL) {

		m_pMainWnd->ShowWindow(m_nCmdShow);

		m_pMainWnd->UpdateWindow();

		success = true;
	}
#endif
#if 0
	hGameCursor = 0;
	hGameCursor = ::LoadCursor(m_hInstance, MAKEINTRESOURCE(IDC_MOD_NOARROW));
	hRightCursor = 0;
	hRightCursor = ::LoadCursor(m_hInstance, MAKEINTRESOURCE(IDC_MOD_RTARROW));
	hLeftCursor = 0;
	hLeftCursor = ::LoadCursor(m_hInstance, MAKEINTRESOURCE(IDC_MOD_LFARROW));
	hUpCursor = 0;
	hUpCursor = ::LoadCursor(m_hInstance, MAKEINTRESOURCE(IDC_MOD_UPARROW));
	hDownCursor = 0;
	hDownCursor = ::LoadCursor(m_hInstance, MAKEINTRESOURCE(IDC_MOD_DNARROW));
#endif
	return success;
}

} // namespace MazeDoom
} // namespace HodjNPodj
} // namespace Bagel
