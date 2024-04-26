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
CBofVHashTable<CBofString, HASH_TABLE_SIZE> *CBagel::m_pCacheFileList;

// Initialize global variables.
//
CBofWindow *g_pHackWindow;

CBagel::CBagel(const BagelReg *pGameReg) {
	Assert(pGameReg != nullptr);
	m_pCacheFileList = new CBofVHashTable<CBofString, HASH_TABLE_SIZE>(&stringHashFunction);

	registerGame(pGameReg);
}

CBagel::~CBagel() {
	Assert(IsValidObject(this));

	// Release options file
	Release();

	// Empty the file cache.
	if (m_pCacheFileList) {
		delete m_pCacheFileList;
		m_pCacheFileList = nullptr;
	}

	m_szAppName[0] = '\0';
	m_pMainWnd = nullptr;
	m_pGameReg = nullptr;
}

void CBagel::registerGame(const BagelReg *pGameReg) {
	Assert(IsValidObject(this));
	m_pGameReg = pGameReg;

	// Init statics
	g_pHackWindow = nullptr;

	// Use registration info to init this game object
	if (pGameReg != nullptr) {
		// Keep application name
		setAppName(pGameReg->_gameName);

		// Load this game's .ini file
		if (pGameReg->_optionFile != nullptr)
			LoadOptionFile(pGameReg->_optionFile);
	}
}

ErrorCode CBagel::setOption(const char *pszSection, const char *pszOption, const char *pszValue) {
	Assert(IsValidObject(this));
	return WriteSetting(pszSection, pszOption, pszValue);
}

ErrorCode CBagel::setOption(const char *pszSection, const char *pszOption, int nValue) {
	Assert(IsValidObject(this));
	return WriteSetting(pszSection, pszOption, nValue);
}

ErrorCode CBagel::getOption(const char *pszSection, const char *pszOption, char *pszValue, const char *pszDefault, uint32 nSize) {
	Assert(IsValidObject(this));
	return ReadSetting(pszSection, pszOption, pszValue, pszDefault, nSize);
}

ErrorCode CBagel::getOption(const char *pszSection, const char *pszOption, int *pValue, int nDefault) {
	Assert(IsValidObject(this));
	return ReadSetting(pszSection, pszOption, pValue, nDefault);
}

ErrorCode CBagel::getOption(const char *pszSection, const char *pszOption, bool *pValue, int nDefault) {
	Assert(IsValidObject(this));
	return ReadSetting(pszSection, pszOption, pValue, nDefault);
}

ErrorCode CBagel::initialize() {
	Assert(IsValidObject(this));

	// Game must already be registered with registerGame()
	Assert(m_pGameReg != nullptr);

	CBofApp::initialize();

	// Initialise engine data for the game
	Common::U32String errMsg;
	if (!Common::load_engine_data("bagel.dat", "", 1, 0, errMsg)) {
		Common::String msg(errMsg);
		BofMessageBox("Engine Data", msg.c_str());
		m_errCode = ERR_FREAD;
		return m_errCode;
	}

	Common::File paintTable;
	if (!paintTable.open("paint_table.txt")) {
		BofMessageBox("Engine Data", "Invalid Paint Table");
		m_errCode = ERR_FREAD;
		return m_errCode;
	}
	PaintTable::initialize(paintTable);

	getOption("UserOptions", "WrongCDRetries", &m_nNumRetries, 20);
	if (m_nNumRetries < 1) {
		m_nNumRetries = 100;
	}

	LogInfo("Initializing BAGEL");

	// Turn off System cursor
	CBofCursor::Hide();

	// Initialize local game paths
	InitLocalFilePaths();

	// Check for adequate system resources
	VerifyRequirements();

	// Child class must instantiate the Main Window
	return m_errCode;
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


ErrorCode CBagel::InitLocalFilePaths() {
	Assert(IsValidObject(this));

	// Check for Installed state of game
	getOption("Startup", "InstallCode", &m_nInstallCode, BAG_INSTALL_DEFAULT);

	return m_errCode;
}

ErrorCode CBagel::VerifyCDInDrive(int nDiskID, const char *pszWaveFile) {
	Assert(IsValidObject(this));

	if (m_pGameReg->_numberOfCDs > 0) {
		char szBuf[MAX_DIRPATH];

		// Find the drive that this disk is in
		Common::sprintf_s(szBuf, "DISK%d", nDiskID);
		Common::sprintf_s(szBuf, "$SBARDIR%sDISK%d", PATH_DELIMETER, nDiskID);

		CBofString cString(szBuf, MAX_DIRPATH);
		MACROREPLACE(cString);

		// If the disk is not in drive, then inform the user
		int i = 0;
		while (!shouldQuit() && !FileExists(cString.GetBuffer())) {
			// display a dialog box that tells the user to put the CD back into
			// the drive, or Abort.
			LogInfo(BuildString("Unable to find game's DiskID as '%s'", cString.GetBuffer()));

			if (i++ > m_nNumRetries) {
				ReportError(ERR_FFIND, "Could not recover from missing CD");
				break;
			}

			// Play the Zelda "Alias, you should insert disk 1 at this time."
			if (i == 1 && pszWaveFile != nullptr) {
				BofPlaySound(pszWaveFile, SOUND_WAVE | SOUND_ASYNCH);
			}

			if (g_pHackWindow == nullptr) {
				ShowNextCDDialog(m_pMainWnd, nDiskID);
			} else {
				ShowNextCDDialog(g_pHackWindow, nDiskID);
			}
		}
	}

	return m_errCode;
}

ErrorCode CBagel::VerifyRequirements() {
	Assert(IsValidObject(this));
	Assert(m_pGameReg != nullptr);

	return m_errCode;
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


void CBagel::ShowNextCDDialog(CBofWindow *pParentWin, int nCDID) {
	CBagNextCDDialog cNextCDDialog;

	// Use specified bitmap as this dialog's image
	CBofPalette *pPal = nullptr;
	CBofBitmap *pBmp = nullptr;

	switch (nCDID) {

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
	cNextCDDialog.Create("NextCD", cRect.left, cRect.top, cRect.Width(), cRect.Height(), pParentWin);
	cNextCDDialog.Center();

	cNextCDDialog.DoModal();
}

} // namespace Bagel
