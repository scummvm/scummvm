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
#include "bagel/music.h"
#include "bagel/spacebar/console.h"
#include "bagel/spacebar/spacebar.h"
#include "bagel/spacebar/master_win.h"
#include "bagel/spacebar/bib_odds_wnd.h"
#include "bagel/spacebar/main_window.h"
#include "bagel/spacebar/boflib/app.h"
#include "bagel/spacebar/baglib/button_object.h"
#include "bagel/boflib/file_functions.h"
#include "bagel/spacebar/boflib/gui/movie.h"

#include "bagel/music.h"
#include "bagel/spacebar/baglib/bagel.h"
#include "bagel/spacebar/baglib/character_object.h"
#include "bagel/spacebar/baglib/cursor.h"
#include "bagel/spacebar/baglib/dossier_object.h"
#include "bagel/spacebar/baglib/event_sdev.h"
#include "bagel/spacebar/baglib/expression.h"
#include "bagel/spacebar/baglib/inv.h"
#include "bagel/spacebar/baglib/log_msg.h"
#include "bagel/spacebar/baglib/menu_dlg.h"
#include "bagel/spacebar/baglib/moo.h"
#include "bagel/spacebar/baglib/paint_table.h"
#include "bagel/spacebar/baglib/pan_window.h"
#include "bagel/spacebar/baglib/parse_object.h"
#include "bagel/spacebar/baglib/pda.h"
#include "bagel/spacebar/baglib/sound_object.h"
#include "bagel/spacebar/dialogs/start_dialog.h"
#include "bagel/spacebar/dialogs/opt_window.h"
#include "bagel/spacebar/dialogs/start_dialog.h"
#include "bagel/spacebar/baglib/storage_dev_win.h"
#include "bagel/spacebar/baglib/var.h"
#include "bagel/spacebar/baglib/wield.h"
#include "bagel/spacebar/baglib/zoom_pda.h"

#include "bagel/boflib/cache.h"
#include "bagel/spacebar/boflib/gfx/cursor.h"
#include "bagel/boflib/error.h"
#include "bagel/boflib/sound.h"
#include "bagel/boflib/palette.h"
#include "bagel/spacebar/boflib/gfx/sprite.h"
#include "bagel/spacebar/boflib/gui/window.h"

namespace Bagel {
namespace SpaceBar {

#define SAVEGAME_VERSION 1

#define SMK_LOGO1        "$SBARDIR\\INTRO\\LOGO1.SMK"
#define SMK_LOGO2        "$SBARDIR\\INTRO\\LOGO2.SMK"
#define SMK_LOGO3        "$SBARDIR\\INTRO\\LOGO3.SMK"
#define SMK_LOGO3EX      "$SBARDIR\\INTRO\\LOGO3EX.SMK"

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

SpaceBarEngine *g_engine;

SpaceBarEngine::SpaceBarEngine(OSystem *syst, const ADGameDescription *gameDesc) :
	BagelEngine(syst, gameDesc), CBagel(&SPACEBAR_REG) {
	SBarBibOddsWnd::initialize();
	CMainWindow::initialize();

	g_engine = this;

	// baglib/ class statics initializations
	CBagCharacterObject::initialize();
	CBagCursor::initialize();
	CBagDossierObject::initialize();
	CBagEventSDev::initialize();
	CBagExpression::initialize();
	CBagInv::initialize();
	CBagLog::initialize();
	CBagMenu::initialize();
	CBagMenuDlg::initialize();
	CBagMoo::initialize();
	CBagPanWindow::initialize();
	CBagParseObject::initialize();
	CBagPDA::initialize();
	CBagSoundObject::initialize();
	CBagStorageDev::initialize();
	CBagStorageDevWnd::initialize();
	CBagVarManager::initialize();
	CBagWield::initialize();
	SBZoomPda::initialize();

	// boflib/ class statics initializations
	CCache::initialize();
	CBofCursor::initialize();
	CBofError::initialize();
	CBofPalette::initialize();
	CBofSound::initialize();
	CBofSprite::initialize();
	CBofWindow::initialize();

	for (int i = 0; i < BIBBLE_NUM_BET_AREAS; ++i)
		g_cBetAreas[i] = CBetArea(BET_AREAS[i]);

	_saveData.clear();
}

SpaceBarEngine::~SpaceBarEngine() {
	CBofSound::shutdown();
	CBofSprite::shutdown();
	CBagCursor::shutdown();
	CBagExpression::shutdown();
	CBagStorageDev::shutdown();
	CMainWindow::shutdown();

	delete _screen;
	g_engine = nullptr;
}

void SpaceBarEngine::initializePath(const Common::FSNode &gamePath) {
	BagelEngine::initializePath(gamePath);
	SearchMan.addDirectory("spacebar", gamePath.getChild("spacebar"), 0, 3);
}

ErrorCode SpaceBarEngine::initialize() {
	assert(isValidObject(this));
	CBofBitmap *pBmp = nullptr;

	CBagel::initialize();

	if (!errorOccurred()) {
		bool bShowLogo = true;

		_masterWin = new CSBarMasterWin();

		// This is the primary game window
		setMainWindow(_masterWin);

		// Init sound system
		InitializeSoundSystem(1, 22050, 8);

		pBmp = new CBofBitmap(_masterWin->width(), _masterWin->height(), _pPalette);
		pBmp->fillRect(nullptr, COLOR_BLACK);

		_masterWin->show();
		_masterWin->validateRect(nullptr);

		// Paint the screen black
		pBmp->paint(_masterWin, 0, 0);

		_useOriginalSaveLoad = ConfMan.getBool("original_menus");

		bool bRestart = true;
		int saveSlot = ConfMan.hasKey("save_slot") ? ConfMan.getInt("save_slot") : -1;

		if (saveSlot != -1) {
			bRestart = loadGameState(saveSlot).getCode() != Common::kNoError;

		} else if (savesExist() && !isDemo()) {
			bRestart = false;

			CBagStartDialog cDlg(buildSysDir("START.BMP"), _masterWin);

			CBofWindow *pLastWin = g_hackWindow;
			g_hackWindow = &cDlg;

			int nRetVal = cDlg.doModal();

			g_hackWindow = pLastWin;

			switch (nRetVal) {
			case RESTORE_BTN:
				break;

			case RESTART_BTN:
				bRestart = true;

				// Hide that dialog
				pBmp->paint(_masterWin, 0, 0);
				break;

			case QUIT_BTN:
				// Hide that dialog
				pBmp->paint(_masterWin, 0, 0);

				_masterWin->close();
				_masterWin = nullptr;
				break;
			}
		}

		if (bRestart) {
			// Should we show the intro movies?
			getOption("Startup", "ShowLogo", &bShowLogo, true);

			// Play intro movies, logo screens, etc...
			if (bShowLogo) {
				CBofString cString(SMK_LOGO1);
				fixPathName(cString);

				// Play the movie only if it exists
				if (fileExists(cString.getBuffer())) {
					bofPlayMovie(_masterWin, cString.getBuffer());
					pBmp->paint(_masterWin, 0, 0);
				}
				if (shouldQuit())
					goto exit;

				cString = SMK_LOGO2;
				fixPathName(cString);
				if (fileExists(cString.getBuffer())) {
					bofPlayMovie(_masterWin, cString.getBuffer());
					pBmp->paint(_masterWin, 0, 0);
				}
				if (shouldQuit())
					goto exit;

				// Use hi-res movie if user has a fast machine
				cString = (getMachineSpeed() < 100) ? SMK_LOGO3EX : SMK_LOGO3;
				fixPathName(cString);

				if (fileExists(cString.getBuffer())) {
					bofPlayMovie(_masterWin, cString.getBuffer());
					pBmp->paint(_masterWin, 0, 0);
				}
			}
			if (shouldQuit())
				goto exit;

			// Start a new game (In entry vestibule)
			_masterWin->newGame();
		}
	}

exit:

	delete pBmp;

	return _errCode;
}

ErrorCode SpaceBarEngine::shutdown() {
	CBagel::shutdown();

	// No more Sound System
	ShutDownSoundSystem();

	return _errCode;
}


ErrorCode SpaceBarEngine::InitializeSoundSystem(uint16 nChannels, uint32 nFreq, uint16 nBitsPerSample) {
	// Nothing to do

	return ERR_NONE;
}

ErrorCode SpaceBarEngine::ShutDownSoundSystem() {
	_mixer->stopAll();
	return _errCode;
}


Common::Error SpaceBarEngine::run() {
	// Initialize graphics mode
	initGraphics(640, 480, nullptr);

	// Initialize systems
	_screen = new Graphics::Screen();
	_midi = new MusicPlayer();
	syncSoundSettings();

	// Set the engine's debugger console
	setDebugger(new Console());

	// Initialize
	preInit();
	initialize();

	// Run the app
	if (!errorOccurred() && !shouldQuit() && _masterWin)
		runApp();

	// shutdown
	shutdown();
	postShutDown();

	return Common::kNoError;
}

void SpaceBarEngine::pauseEngineIntern(bool pause) {
	Engine::pauseEngineIntern(pause);
	if (pause) {
		_midi->pause();
	} else {
		_midi->resume();
	}
}


bool SpaceBarEngine::canSaveLoadFromWindow(bool save) const {
	CBofWindow *win = CBofWindow::getActiveWindow();

	// Don't allow saves when capture/focus is active
	if (CBofApp::getApp()->getCaptureControl() != nullptr ||
	        CBofApp::getApp()->getFocusControl() != nullptr ||
	        win == nullptr)
		return false;

	// These two dialogs need to allow save/load for the ScummVM
	// dialogs to work from them when original save/load is disabled
	if ((dynamic_cast<CBagStartDialog *>(win) != nullptr && !save) ||
	        dynamic_cast<CBagOptWindow *>(win) != nullptr)
		return true;

	// Otherwise, allow save/load if it's not a dialog, and it's
	// not a special view that shows the system cursor, like the
	// Nav Window minigame or Drink Mixer
	return dynamic_cast<CBofDialog *>(win) == nullptr &&
	       !CBagCursor::isSystemCursorVisible();
}

bool SpaceBarEngine::canLoadGameStateCurrently(Common::U32String *msg) {
	return canSaveLoadFromWindow(false);
}

bool SpaceBarEngine::canSaveGameStateCurrently(Common::U32String *msg) {
	return canSaveLoadFromWindow(true);
}

Common::Error SpaceBarEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	_masterWin->fillSaveBuffer(&_saveData);

	return Engine::saveGameState(slot, desc, isAutosave);
}

Common::Error SpaceBarEngine::saveGameState(int slot, const Common::String &desc,
        bool isAutosave, StBagelSave &saveData) {
	_saveData = saveData;
	return Engine::saveGameState(slot, desc, isAutosave);
}

Common::Error SpaceBarEngine::loadGameState(int slot) {
	Common::Error result = Engine::loadGameState(slot);

	if (result.getCode() == Common::kNoError) {
		// Make sure we close any GUI windows before loading from GMM
		CBofWindow *win = CBofWindow::getActiveWindow();
		if (win)
			win->close();
		_masterWin->doRestore(&_saveData);
	}

	return result;
}

Common::Error SpaceBarEngine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	stream->writeByte(SAVEGAME_VERSION);

	Common::Serializer s(nullptr, stream);
	_saveData.synchronize(s);
	return Common::kNoError;
}

Common::Error SpaceBarEngine::loadGameStream(Common::SeekableReadStream *stream) {
	const byte version = stream->readByte();
	if (version > SAVEGAME_VERSION)
		error("Tried to load unsupported savegame version");

	Common::Serializer s(stream, nullptr);
	_saveData.synchronize(s);
	return Common::kNoError;
}

} // namespace SpaceBar
} // namespace Bagel
