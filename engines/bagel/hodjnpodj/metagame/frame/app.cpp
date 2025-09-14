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

#include "bagel/hodjnpodj/metagame/frame/app.h"
#include "bagel/hodjnpodj/metagame/frame/hodjpodj.h"
#include "bagel/hodjnpodj/metagame/bgen/mgstat.h"
#include "bagel/hodjnpodj/hodjnpodj.h"
#include "bagel/boflib/sound.h"
#include "bagel/metaengine.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Frame {

bool dllLoaded;
CWnd *g_wndGame;
HINSTANCE hExeInst;
HINSTANCE hMetaInst;

CTheApp::CTheApp() {
	dllLoaded = false;
	g_wndGame = nullptr;
	hExeInst = nullptr;
	hMetaInst = nullptr;
}

bool CTheApp::InitApplication() {
	if (_startupMinigame.empty()) {
		// Main game
		addResources("meta/hodjpodj.exe");
		addResources("meta/hnpmeta.dll");
		setDirectory("meta");
	} else if (_startupMinigame == "mazedoom_demo") {
		addResources("mod.exe");
	}

	addFontResource("msserif.fon");
	setFocusChangeProc(focusChange);
	return CWinApp::InitApplication();
}

bool CTheApp::InitInstance() {
	if (_startupMinigame.empty()) {
		// Main game
		m_pMainWnd = new Frame::CHodjPodjWindow();
		m_pMainWnd->ShowWindow(SW_SHOWNORMAL);
		m_pMainWnd->UpdateWindow();

	} else {
		// Specific minigame
		selectMinigame();
	}

	return true;
}

int CTheApp::ExitInstance() {
	if (!shouldQuit()) {
		if (dllLoaded) {
			if (g_wndGame != nullptr)
				g_wndGame->SendMessage(WM_CLOSE);

			dllLoaded = false;
		}

		if (hMetaInst > HINSTANCE_ERROR) {
			if (g_wndGame != nullptr)
				g_wndGame->SendMessage(WM_CLOSE);

			FreeLibrary(hMetaInst);
			hMetaInst = nullptr;
		}
	}

	delete m_pMainWnd;
	return 0;
}

void CTheApp::selectMinigame() {
	bool isMODDemo = false;
	const auto *game = Metagame::CMgStatic::cGameTable;

	for (; game->m_iGameCode; ++game) {
		if (game->_path && _startupMinigame == game->_path)
			break;
		isMODDemo = _startupMinigame == "mazedoom_demo" &&
			game->m_iGameCode == MG_GAME_MAZEODOOM;
		if (isMODDemo)
			break;
	}

	if (!game->m_iGameCode)
		error("Unknown minigame - %s", _startupMinigame.c_str());

	GAMESTRUCT *lpGameStruct = new GAMESTRUCT();
	lpGameStruct->lCrowns = 1000;
	lpGameStruct->lScore = 0;
	lpGameStruct->nSkillLevel = SKILLLEVEL_MEDIUM;
	lpGameStruct->bSoundEffectsEnabled = true;
	lpGameStruct->bMusicEnabled = true;
	lpGameStruct->bPlayingMetagame = false;
	lpGameStruct->bPlayingHodj = true;

	// Don't change folder for Maze O Doom demo
	if (isMODDemo) {
		addResources("mod.exe");
	} else {
		setDirectory(game->_path);
		addResources(game->_dllName);
	}

	// Start the minigame
	game->_initFn(nullptr, lpGameStruct);
}

bool CTheApp::OnIdle(long lCount) {
	CBofSound::audioTask();
	return true;
}

void CTheApp::focusChange(CWnd *oldFocus, CWnd *newFocus) {
	CEdit *oldCtl = dynamic_cast<CEdit *>(oldFocus);
	CEdit *newCtl = dynamic_cast<CEdit *>(newFocus);
	if (oldCtl != newCtl)
		BagelMetaEngine::setKeybindingMode(newFocus ? KBMODE_MINIMAL : KBMODE_NORMAL);
}

} // namespace Frame
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
