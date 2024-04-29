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
#include "bagel/baglib/bagel.h"
#include "bagel/boflib/sound.h"
#include "bagel/dialogs/next_cd_dialog.h"
#include "bagel/dialogs/opt_window.h"
#include "bagel/baglib/paint_table.h"
#include "bagel/baglib/master_win.h"
#include "bagel/boflib/debug.h"
#include "bagel/boflib/file_functions.h"
#include "bagel/boflib/log.h"

namespace Bagel {

// Static member variables.
static unsigned stringHashFunction(const CBofString &s) {
	return s.Hash();
}
CBofVHashTable<CBofString, HASH_TABLE_SIZE> *CBagel::_cacheFileList;

// Initialize global variables.
//
CBofWindow *g_hackWindow;

CBagel::CBagel(const BagelReg *gameReg) {
	Assert(gameReg != nullptr);
	_cacheFileList = new CBofVHashTable<CBofString, HASH_TABLE_SIZE>(&stringHashFunction);

	registerGame(gameReg);
}

CBagel::~CBagel() {
	Assert(IsValidObject(this));

	// Release options file
	Release();

	// Empty the file cache.
	if (_cacheFileList) {
		delete _cacheFileList;
		_cacheFileList = nullptr;
	}

	m_szAppName[0] = '\0';
	m_pMainWnd = nullptr;
	_gameReg = nullptr;
}

void CBagel::registerGame(const BagelReg *gameReg) {
	Assert(IsValidObject(this));
	_gameReg = gameReg;

	// Init statics
	g_hackWindow = nullptr;

	// Use registration info to init this game object
	if (gameReg != nullptr) {
		// Keep application name
		setAppName(gameReg->_gameName);

		// Load this game's .ini file
		if (gameReg->_optionFile != nullptr)
			LoadOptionFile(gameReg->_optionFile);
	}
}

ErrorCode CBagel::setOption(const char *section, const char *option, const char *stringValue) {
	Assert(IsValidObject(this));
	return WriteSetting(section, option, stringValue);
}

ErrorCode CBagel::setOption(const char *section, const char *option, int intValue) {
	Assert(IsValidObject(this));
	return WriteSetting(section, option, intValue);
}

ErrorCode CBagel::getOption(const char *section, const char *option, char *stringValue, const char *defaultValue, uint32 size) {
	Assert(IsValidObject(this));
	return ReadSetting(section, option, stringValue, defaultValue, size);
}

ErrorCode CBagel::getOption(const char *section, const char *option, int *intValue, int defaultValue) {
	Assert(IsValidObject(this));
	return ReadSetting(section, option, intValue, defaultValue);
}

ErrorCode CBagel::getOption(const char *section, const char *option, bool *boolValue, int defaultValue) {
	Assert(IsValidObject(this));
	return ReadSetting(section, option, boolValue, defaultValue);
}

ErrorCode CBagel::initialize() {
	Assert(IsValidObject(this));

	// Game must already be registered with registerGame()
	Assert(_gameReg != nullptr);

	CBofApp::initialize();

	// Initialise engine data for the game
	Common::U32String errMsg;
	if (!Common::load_engine_data("bagel.dat", "", 1, 0, errMsg)) {
		Common::String msg(errMsg);
		BofMessageBox("Engine Data", msg.c_str());
		_errCode = ERR_FREAD;
		return _errCode;
	}

	Common::File paintTable;
	if (!paintTable.open("paint_table.txt")) {
		BofMessageBox("Engine Data", "Invalid Paint Table");
		_errCode = ERR_FREAD;
		return _errCode;
	}
	PaintTable::initialize(paintTable);

	getOption("UserOptions", "WrongCDRetries", &_numRetries, 20);
	if (_numRetries < 1) {
		_numRetries = 100;
	}

	LogInfo("Initializing BAGEL");

	// Turn off System cursor
	CBofCursor::Hide();

	// Initialize local game paths
	initLocalFilePaths();

	// Check for adequate system resources
	verifyRequirements();

	// Child class must instantiate the Main Window
	return _errCode;
}

ErrorCode CBagel::runApp() {
	Assert(IsValidObject(this));

	// The main game window must have been created by now
	Assert(m_pMainWnd != nullptr);

	return CBofApp::runApp();
}

ErrorCode CBagel::shutdown() {
	Assert(IsValidObject(this));

	LogInfo("Shutting down BAGEL");

	// Just in case the child class forgot to delete the main window,
	// I'll do it.
	if (m_pMainWnd != nullptr) {
		delete m_pMainWnd;
		m_pMainWnd = nullptr;
	}

	return CBofApp::shutdown();
}


ErrorCode CBagel::initLocalFilePaths() {
	Assert(IsValidObject(this));

	// Check for Installed state of game
	getOption("Startup", "InstallCode", &_installCode, BAG_INSTALL_DEFAULT);

	return _errCode;
}

ErrorCode CBagel::verifyCDInDrive(int diskId, const char *waveFile) {
	Assert(IsValidObject(this));

	if (_gameReg->_numberOfCDs > 0) {
		char szBuf[MAX_DIRPATH];

		// Find the drive that this disk is in
		Common::sprintf_s(szBuf, "DISK%d", diskId);
		Common::sprintf_s(szBuf, "$SBARDIR%sDISK%d", PATH_DELIMETER, diskId);

		CBofString cString(szBuf, MAX_DIRPATH);
		MACROREPLACE(cString);

		// If the disk is not in drive, then inform the user
		int i = 0;
		while (!shouldQuit() && !FileExists(cString.GetBuffer())) {
			// display a dialog box that tells the user to put the CD back into
			// the drive, or Abort.
			LogInfo(BuildString("Unable to find game's DiskID as '%s'", cString.GetBuffer()));

			if (i++ > _numRetries) {
				ReportError(ERR_FFIND, "Could not recover from missing CD");
				break;
			}

			// Play the Zelda "Alias, you should insert disk 1 at this time."
			if (i == 1 && waveFile != nullptr) {
				BofPlaySound(waveFile, SOUND_WAVE | SOUND_ASYNCH);
			}

			if (g_hackWindow == nullptr) {
				showNextCDDialog(m_pMainWnd, diskId);
			} else {
				showNextCDDialog(g_hackWindow, diskId);
			}
		}
	}

	return _errCode;
}

ErrorCode CBagel::verifyRequirements() {
	Assert(IsValidObject(this));
	Assert(_gameReg != nullptr);

	return _errCode;
}

bool MACROREPLACE(CBofString &s) {
	// Remove any homedir prefix. In ScummVM, all paths are relative
	// to the game folder automatically
	char *p = strstr(s.GetBuffer(), HOMEDIR_TOKEN);
	if (p != nullptr)
		s = p + strlen(HOMEDIR_TOKEN) + 1;

	// Replace any backslashes with forward slashes
	while ((p = strchr(s.GetBuffer(), '\\')) != nullptr)
		*p = '/';

	return true;
}


void CBagel::showNextCDDialog(CBofWindow *parentWin, int diskId) {
	CBagNextCDDialog cNextCDDialog;

	// Use specified bitmap as this dialog's image
	CBofPalette *pPal = nullptr;
	CBofBitmap *pBmp = nullptr;

	switch (diskId) {

	case 1:
		pBmp = Bagel::LoadBitmap(BuildSysDir("DISK1.BMP"), pPal);
		break;

	case 2:
		pBmp = Bagel::LoadBitmap(BuildSysDir("DISK2.BMP"), pPal);
		break;

	case 3:
		pBmp = Bagel::LoadBitmap(BuildSysDir("DISK3.BMP"), pPal);
		break;

	// Shouldn't ever get here
	default:
		Assert(false);
		break;
	}

	cNextCDDialog.SetBackdrop(pBmp);

	CBofRect cRect = cNextCDDialog.GetBackdrop()->GetRect();

	// Create the dialog box
	cNextCDDialog.create("NextCD", cRect.left, cRect.top, cRect.Width(), cRect.Height(), parentWin);
	cNextCDDialog.Center();

	cNextCDDialog.doModal();
}

} // namespace Bagel
