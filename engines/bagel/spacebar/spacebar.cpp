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

#include "audio/mixer.h"
#include "common/config-manager.h"
#include "engines/util.h"
#include "bagel/console.h"
#include "bagel/music.h"
#include "bagel/spacebar/spacebar.h"
#include "bagel/spacebar/master_win.h"
#include "bagel/spacebar/bib_odds_wnd.h"
#include "bagel/boflib/app.h"
#include "bagel/dialogs/start_dialog.h"
#include "bagel/dialogs/opt_window.h"
#include "bagel/baglib/buttons.h"
#include "bagel/boflib/gui/movie.h"

namespace Bagel {
namespace SpaceBar {

#define LOGOSMK1        "$SBARDIR\\INTRO\\LOGO1.SMK"
#define LOGOSMK2        "$SBARDIR\\INTRO\\LOGO2.SMK"
#define LOGOSMK3        "$SBARDIR\\INTRO\\LOGO3.SMK"
#define LOGOSMK3EX      "$SBARDIR\\INTRO\\LOGO3EX.SMK"

static const BagelReg SPACEBAR_REG = {
	"The Space Bar",
	".",
	"spacebar.ini",
	"spacebar.sav",
	4000000,
	3,
	16,
	640,
	480
};

SpaceBarEngine::SpaceBarEngine(OSystem *syst, const ADGameDescription *gameDesc) :
	BagelEngine(syst, gameDesc), CBagel(&SPACEBAR_REG) {
	SBarBibOddsWnd::initialize();
}

void SpaceBarEngine::initializePath(const Common::FSNode &gamePath) {
	BagelEngine::initializePath(gamePath);
	SearchMan.addDirectory("spacebar", gamePath.getChild("spacebar"), 0, 3);
}

ERROR_CODE SpaceBarEngine::initialize() {
	Assert(IsValidObject(this));
	CBofBitmap *pBmp = nullptr;

	CBagel::initialize();

	if (!ErrorOccurred()) {
		bool bShowLogo = TRUE;

		if ((_masterWin = new CSBarMasterWin()) != nullptr) {
			// This is the primary game window
			SetMainWindow(_masterWin);

			// Init sound system
			InitializeSoundSystem(1, 22050, 8);

			if ((pBmp = new CBofBitmap(_masterWin->Width(), _masterWin->Height(), m_pPalette)) != nullptr) {
				pBmp->FillRect(nullptr, COLOR_BLACK);
			} else {
				ReportError(ERR_MEMORY, "Unable to allocate a CBofBitmap");
			}

			_masterWin->Show();
			_masterWin->ValidateRect(nullptr);

			// Paint the screen black
			if (pBmp != nullptr)
				pBmp->Paint(_masterWin, 0, 0);

			_useOriginalSaveLoad = ConfMan.getBool("original_menus");

			bool bRestart = TRUE;
			int saveSlot = ConfMan.hasKey("save_slot") ? ConfMan.getInt("save_slot") : -1;

			if (saveSlot != -1) {
				bRestart = loadGameState(saveSlot).getCode() != Common::kNoError;

			} else if (savesExist()) {
				bRestart = FALSE;

				CBagStartDialog cDlg(BuildSysDir("START.BMP"), nullptr, _masterWin);
				int nRetVal;

				CBofWindow *pLastWin = g_pHackWindow;
				g_pHackWindow = &cDlg;

				nRetVal = cDlg.DoModal();

				g_pHackWindow = pLastWin;

				switch (nRetVal) {
				case RESTORE_BTN:
					break;

				case RESTART_BTN:
					bRestart = TRUE;

					// Hide that dialog
					if (pBmp != nullptr) {
						pBmp->Paint(_masterWin, 0, 0);
					}
					break;

				case QUIT_BTN:
					// Hide that dialog
					if (pBmp != nullptr) {
						pBmp->Paint(_masterWin, 0, 0);
					}
					_masterWin->Close();
					_masterWin = nullptr;
					break;
				}
			}

			if (bRestart) {
				// Should we show the intro movies?
				GetOption("Startup", "ShowLogo", &bShowLogo, TRUE);

				// Play intro movies, logo screens, etc...
				// (Unless user holds down the shift key, or ShowLogo=0 in SPACEBAR.INI)
				if (bShowLogo && !IsKeyDown(BKEY_SHIFT)) {
					CBofString cString(LOGOSMK1);
					MACROREPLACE(cString);

					// Play the movie only if it exists
					if (FileExists(cString.GetBuffer())) {
						BofPlayMovie(_masterWin, cString.GetBuffer());
						if (pBmp != nullptr) {
							pBmp->Paint(_masterWin, 0, 0);
						}
					}
					if (shouldQuit())
						goto exit;

					cString = LOGOSMK2;
					MACROREPLACE(cString);
					if (FileExists(cString.GetBuffer())) {
						BofPlayMovie(_masterWin, cString.GetBuffer());
						if (pBmp != nullptr) {
							pBmp->Paint(_masterWin, 0, 0);
						}
					}
					if (shouldQuit())
						goto exit;

					// Use hi-res movie if user has a fast machine
					cString = (GetMachineSpeed() < 100) ? LOGOSMK3EX : LOGOSMK3;
					MACROREPLACE(cString);

					if (FileExists(cString.GetBuffer())) {
						BofPlayMovie(_masterWin, cString.GetBuffer());
						if (pBmp != nullptr) {
							pBmp->Paint(_masterWin, 0, 0);
						}
					}
				}
				if (shouldQuit())
					goto exit;

				// Start a new game (In entry vestible)
				_masterWin->NewGame();
			}

		} else {
			ReportError(ERR_MEMORY, "Unable to allocate the main SpaceBar Window");
		}
	}

exit:
	if (pBmp != nullptr)
		delete pBmp;

	return m_errCode;
}

ERROR_CODE SpaceBarEngine::shutdown() {
	CBagel::shutdown();

	// No more Sound System
	ShutDownSoundSystem();

	return m_errCode;
}


ERROR_CODE SpaceBarEngine::InitializeSoundSystem(uint16 nChannels, uint32 nFreq, uint16 nBitsPerSample) {
	// Nothing to do

	return ERR_NONE;
}

ERROR_CODE SpaceBarEngine::ShutDownSoundSystem() {
	_mixer->stopAll();
	return m_errCode;
}


Common::Error SpaceBarEngine::run() {
	// Initialize graphics mode
	Graphics::PixelFormat format(2, 5, 6, 5, 0, 11, 5, 0, 0);
	initGraphics(640, 480, &format);

	// Initialize systems
	_screen = new Graphics::Screen();
	_midi = new MusicPlayer();
	syncSoundSettings();

	// Set the engine's debugger console
	setDebugger(new Console());

	// Initialize
	PreInit();
	initialize();

	// Run the app
	if (!ErrorOccurred() && !shouldQuit() && _masterWin)
		RunApp();

	// shutdown
	PreShutDown();
	shutdown();
	PostShutDown();

	return Common::kNoError;
}

} // namespace SpaceBar
} // namespace Bagel
