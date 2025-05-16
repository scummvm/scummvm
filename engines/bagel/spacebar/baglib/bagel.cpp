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

#include "common/engine_data.h"
#include "common/file.h"
#include "engines/engine.h"

#include "bagel/spacebar/baglib/bagel.h"
#include "bagel/boflib/sound.h"
#include "bagel/spacebar/dialogs/next_cd_dialog.h"
#include "bagel/spacebar/dialogs/opt_window.h"
#include "bagel/spacebar/baglib/paint_table.h"
#include "bagel/spacebar/baglib/master_win.h"
#include "bagel/spacebar/boflib/debug.h"
#include "bagel/boflib/file_functions.h"
#include "bagel/boflib/log.h"

namespace Bagel {
namespace SpaceBar {

// Initialize global variables.
//
CBofWindow *g_hackWindow;

CBagel::CBagel(const BagelReg *gameReg) {
	assert(gameReg != nullptr);

	registerGame(gameReg);
}

CBagel::~CBagel() {
	assert(isValidObject(this));

	// Release options file
	release();

	_szAppName[0] = '\0';
	_pMainWnd = nullptr;
	_gameReg = nullptr;
}

void CBagel::registerGame(const BagelReg *gameReg) {
	assert(isValidObject(this));
	_gameReg = gameReg;

	// Init statics
	g_hackWindow = nullptr;

	// Use registration info to init this game object
	if (gameReg != nullptr) {
		// Keep application name
		setAppName(gameReg->_gameName);

		// Load this game's .ini file
		if (gameReg->_optionFile != nullptr)
			loadOptionFile(gameReg->_optionFile);
	}
}

ErrorCode CBagel::setOption(const char *section, const char *option, const char *stringValue) {
	assert(isValidObject(this));
	return writeSetting(section, option, stringValue);
}

ErrorCode CBagel::setOption(const char *section, const char *option, int intValue) {
	assert(isValidObject(this));
	return writeSetting(section, option, intValue);
}

ErrorCode CBagel::getOption(const char *section, const char *option, char *stringValue, const char *defaultValue, uint32 size) {
	assert(isValidObject(this));
	return readSetting(section, option, stringValue, defaultValue, size);
}

ErrorCode CBagel::getOption(const char *section, const char *option, int *intValue, int defaultValue) {
	assert(isValidObject(this));
	return readSetting(section, option, intValue, defaultValue);
}

ErrorCode CBagel::getOption(const char *section, const char *option, bool *boolValue, int defaultValue) {
	assert(isValidObject(this));
	return readSetting(section, option, boolValue, defaultValue);
}

ErrorCode CBagel::initialize() {
	assert(isValidObject(this));

	// Game must already be registered with registerGame()
	assert(_gameReg != nullptr);

	CBofApp::initialize();

	// Initialise engine data for the game
	Common::U32String errMsg;
	if (!Common::load_engine_data("bagel.dat", "", 1, 0, errMsg)) {
		GUIErrorMessage("Could not find bagel.dat data file");
		const Common::String msg(errMsg);
		bofMessageBox("Engine Data", msg.c_str());
		_errCode = ERR_FREAD;
		return _errCode;
	}

	Common::File paintTable;
	if (!paintTable.open("paint_table.txt")) {
		bofMessageBox("Engine Data", "Invalid Paint Table");
		_errCode = ERR_FREAD;
		return _errCode;
	}
	PaintTable::initialize(paintTable);

	getOption("UserOptions", "WrongCDRetries", &_numRetries, 20);
	if (_numRetries < 1) {
		_numRetries = 100;
	}

	logInfo("Initializing BAGEL");

	// Turn off System cursor
	CBofCursor::hide();

	// Initialize local game paths
	initLocalFilePaths();

	// Child class must instantiate the Main Window
	return _errCode;
}

ErrorCode CBagel::runApp() {
	assert(isValidObject(this));

	// The main game window must have been created by now
	assert(_pMainWnd != nullptr);

	return CBofApp::runApp();
}

ErrorCode CBagel::shutdown() {
	assert(isValidObject(this));

	logInfo("Shutting down BAGEL");

	// Just in case the child class forgot to delete the main window,
	// I'll do it.
	delete _pMainWnd;
	_pMainWnd = nullptr;

	return CBofApp::shutdown();
}


ErrorCode CBagel::initLocalFilePaths() {
	assert(isValidObject(this));

	// Check for Installed state of game
	getOption("Startup", "InstallCode", &_installCode, BAG_INSTALL_DEFAULT);

	return _errCode;
}

ErrorCode CBagel::verifyCDInDrive(int diskId, const char *waveFile) {
	assert(isValidObject(this));

	if (_gameReg->_numberOfCDs > 0) {
		char szBuf[MAX_DIRPATH];

		// Find the drive that this disk is in
		Common::sprintf_s(szBuf, "DISK%d", diskId);
		Common::sprintf_s(szBuf, "$SBARDIR%sDISK%d", PATH_DELIMETER, diskId);

		CBofString cString(szBuf, MAX_DIRPATH);
		fixPathName(cString);

		// If the disk is not in drive, then inform the user
		int i = 0;
		while (!shouldQuit() && !fileExists(cString.getBuffer())) {
			// display a dialog box that tells the user to put the CD back into
			// the drive, or Abort.
			logInfo(buildString("Unable to find game's DiskID as '%s'", cString.getBuffer()));

			if (i++ > _numRetries) {
				reportError(ERR_FFIND, "Could not recover from missing CD");
				break;
			}

			// Play the Zelda "Alias, you should insert disk 1 at this time."
			if (i == 1 && waveFile != nullptr) {
				BofPlaySound(waveFile, SOUND_WAVE | SOUND_ASYNCH);
			}

			if (g_hackWindow == nullptr) {
				showNextCDDialog(_pMainWnd, diskId);
			} else {
				showNextCDDialog(g_hackWindow, diskId);
			}
		}
	}

	return _errCode;
}

void CBagel::showNextCDDialog(CBofWindow *parentWin, int diskId) {
	CBagNextCDDialog cNextCDDialog;

	// Use specified bitmap as this dialog's image
	CBofPalette *pPal = nullptr;
	CBofBitmap *pBmp = nullptr;

	switch (diskId) {

	case 1:
		pBmp = SpaceBar::loadBitmap(buildSysDir("DISK1.BMP"), pPal);
		break;

	case 2:
		pBmp = SpaceBar::loadBitmap(buildSysDir("DISK2.BMP"), pPal);
		break;

	case 3:
		pBmp = SpaceBar::loadBitmap(buildSysDir("DISK3.BMP"), pPal);
		break;

	// Shouldn't ever get here
	default:
		assert(false);
		break;
	}

	cNextCDDialog.setBackdrop(pBmp);

	const CBofRect cRect = cNextCDDialog.getBackdrop()->getRect();

	// Create the dialog box
	cNextCDDialog.create("NextCD", cRect.left, cRect.top, cRect.width(), cRect.height(), parentWin);
	cNextCDDialog.center();

	cNextCDDialog.doModal();
}

} // namespace SpaceBar
} // namespace Bagel
