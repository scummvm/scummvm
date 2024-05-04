
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

#include "bagel/baglib/master_win.h"
#include "bagel/baglib/button_object.h"
#include "bagel/dialogs/credits_dialog.h"
#include "bagel/dialogs/quit_dialog.h"
#include "bagel/baglib/help.h"
#include "bagel/baglib/menu_dlg.h"
#include "bagel/dialogs/opt_window.h"
#include "bagel/dialogs/restore_dialog.h"
#include "bagel/dialogs/restart_dialog.h"
#include "bagel/dialogs/save_dialog.h"
#include "bagel/dialogs/start_dialog.h"
#include "bagel/baglib/storage_dev_win.h"
#include "bagel/baglib/wield.h"
#include "bagel/boflib/app.h"
#include "bagel/boflib/event_loop.h"
#include "bagel/boflib/file.h"
#include "bagel/boflib/gfx/palette.h"
#include "bagel/baglib/pan_window.h"
#include "bagel/boflib/rect.h"
#include "bagel/bagel.h"
#include "bagel/boflib/file_functions.h"
#include "bagel/boflib/log.h"
#include "bagel/boflib/std_keys.h"

namespace Bagel {

#define STARTWORLD "$SBARDIR\\WLD\\BAR.WLD"
#define GLOBALWORLD "$SBARDIR\\WLD\\GLOBAL.WLD"
#define LOADINGBMP "$SBARDIR\\GENERAL\\SYSTEM\\LOADING.BMP"

#define USER_OPTIONS "UserOptions"
#define WAVE_VOLUME "WaveVolume"

// Global vars
//
extern CBofWindow *g_hackWindow;
static bool g_bAllowRestore = false;

bool g_restoreObjectListFl = true;

#define NUM_MSG_STRINGS 3
static int g_nString = 0;
static char g_szString[NUM_MSG_STRINGS][512];

// static initializations

bool CBagMasterWin::_objSaveFl = false;
ST_OBJ *CBagMasterWin::_objList = nullptr;
CBagCursor *CBagMasterWin::_cursorList[MAX_CURSORS];
int CBagMasterWin::_menuCount = 0;
int CBagMasterWin::_curCursor = 0;

//
//
// CBagMasterWin
//
CBagMasterWin::CBagMasterWin() {
	CBofRect cRect(0, 0, 640 - 1, 480 - 1);

	const char *pAppName = "BAGEL Application";

	CBofApp *pApp = CBofApp::getApp();
	if (pApp != nullptr) {
		cRect.SetRect(0, 0, pApp->screenWidth() - 1, pApp->screenHeight() - 1);
		pAppName = pApp->getAppName();
	}

	cRect.SetRect(0, 0, 640 - 1, 480 - 1);
	_fadeIn = 0;
	_gameWindow = nullptr;

	_storageDeviceList = nullptr;
	_gameSDevList = nullptr;
	_variableList = nullptr;
	_diskId = 1;

	create(pAppName, &cRect);

	// Assume default system screen
	_sysScreen = "$SBARDIR\\GENERAL\\SYSTEM\\GAMBHALL.BMP";
	MACROREPLACE(_sysScreen);

	// Load wait sound for when user hits the spacebar
	CBofString cString("$SBARDIR\\GENERAL\\WAIT.WAV");
	MACROREPLACE(cString);

	_waitSound = new CBofSound(this, cString, SOUND_MIX);
}

CBagMasterWin::~CBagMasterWin() {
	Assert(IsValidObject(this));

	if (_waitSound != nullptr) {
		delete _waitSound;
		_waitSound = nullptr;
	}

	CBofApp *pApp;
	if ((pApp = CBofApp::getApp()) != nullptr) {
		pApp->setPalette(nullptr);
	}

	// Delete any remaining cursors
	for (int i = 0; i < MAX_CURSORS; i++) {
		if (_cursorList[i] != nullptr) {
			delete _cursorList[i];
			_cursorList[i] = nullptr;
		}
	}

	if (_gameWindow != nullptr) {
		delete _gameWindow;
		_gameWindow = nullptr;
	}
	if (_storageDeviceList != nullptr) {
		delete _storageDeviceList;
		_storageDeviceList = nullptr;
	}
	if (_variableList != nullptr) {
		delete _variableList;
		_variableList = nullptr;
	}
	if (_gameSDevList != nullptr) {
		delete _gameSDevList;
		_gameSDevList = nullptr;
	}

	// We can get rid of this buffer since the game is shutting down
	if (_objList != nullptr) {
		BofFree(_objList);
		_objList = nullptr;
	}
}

ErrorCode CBagMasterWin::showSystemDialog(bool bSaveBackground) {
	Assert(IsValidObject(this));
	if (g_engine->isDemo())
		return ERR_NONE;

	CBagStorageDevWnd *pSdev = getCurrentStorageDev();

	if ((pSdev == nullptr) || (pSdev->GetDeviceType() == SDEV_GAMEWIN) || (pSdev->GetDeviceType() == SDEV_ZOOMPDA)) {
		LogInfo("Showing System Screen");

		CBagOptWindow cOptionDialog;

		// Use specified bitmap as this dialog's image
		CBofBitmap *pBmp = Bagel::loadBitmap(_sysScreen.GetBuffer());

		cOptionDialog.setBackdrop(pBmp);

		CBofRect cRect = cOptionDialog.getBackdrop()->getRect();

		if (!bSaveBackground) {
			cOptionDialog.setFlags(cOptionDialog.getFlags() & ~BOFDLG_SAVEBACKGND);
		}

		// Create the dialog box
		cOptionDialog.create("System Dialog", cRect.left, cRect.top, cRect.width(), cRect.height(), this);

		CBofWindow *pLastWin = g_hackWindow;
		g_hackWindow = &cOptionDialog;

		g_bPauseTimer = true;
		int nReturnValue = cOptionDialog.doModal();
		g_bPauseTimer = false;
		cOptionDialog.detach();

		g_hackWindow = pLastWin;

		LogInfo("Exiting System Screen");

		// User chose to Quit
		if (nReturnValue == 0) {
			close();
		}
	}

	return _errCode;
}

ErrorCode CBagMasterWin::showCreditsDialog(CBofWindow *win, bool bSaveBkg) {
	Assert(IsValidObject(this));

	LogInfo("Showing Credits Screen");

	CBagCreditsDialog cCreditsDialog;

	// Use specified bitmap as this dialog's image
	CBofBitmap *pBmp = Bagel::loadBitmap(buildSysDir("BARAREA.BMP"));

	cCreditsDialog.setBackdrop(pBmp);

	CBofRect cRect = cCreditsDialog.getBackdrop()->getRect();

	// Don't allow save of background?
	if (!bSaveBkg) {
		int lFlags = cCreditsDialog.getFlags();

		cCreditsDialog.setFlags(lFlags & ~BOFDLG_SAVEBACKGND);
	}

	// Use CBagMasterWin if no parent specified
	if (win == nullptr) {
		win = this;
	}

	// Create the dialog box
	cCreditsDialog.create("Save Dialog", cRect.left, cRect.top, cRect.width(), cRect.height(), win);

	bool bSaveTimer = g_bPauseTimer;
	g_bPauseTimer = true;
	cCreditsDialog.doModal();
	g_bPauseTimer = bSaveTimer;

	LogInfo("Exiting Credits Screen");

	return _errCode;
}

bool CBagMasterWin::showQuitDialog(CBofWindow *win, bool bSaveBackground) {
	Assert(IsValidObject(this));

	CBagStorageDevWnd *pSdev = getCurrentStorageDev();
	bool bQuit = false;

	if ((pSdev == nullptr) || (pSdev->GetDeviceType() == SDEV_GAMEWIN) || (pSdev->GetDeviceType() == SDEV_ZOOMPDA)) {
		LogInfo("Showing Quit Screen");

		CBagQuitDialog cQuitDialog;

		CBofSound::PauseSounds();

		if (win == nullptr) {
			win = this;
		}

		// Use specified bitmap as this dialog's image
		CBofBitmap *pBmp = Bagel::loadBitmap(_sysScreen.GetBuffer());

		cQuitDialog.setBackdrop(pBmp);

		CBofRect cRect = cQuitDialog.getBackdrop()->getRect();

		if (!bSaveBackground) {
			cQuitDialog.setFlags(cQuitDialog.getFlags() & ~BOFDLG_SAVEBACKGND);
		}

		// Create the dialog box
		cQuitDialog.create("Quit Dialog", cRect.left, cRect.top, cRect.width(), cRect.height(), win);

		bool bSaveTimer = g_bPauseTimer;
		g_bPauseTimer = true;
		int nReturnValue = cQuitDialog.doModal();
		g_bPauseTimer = bSaveTimer;

		switch (nReturnValue) {
		case SAVE_BTN:
			// Quit as as well. Saving already done within dialog itself
			bQuit = true;
			break;

		case QUIT_BTN:
			bQuit = true;
			break;

		case CANCEL_BTN:
			bQuit = false;
			break;
		}

		if (!bQuit) {
			CBofSound::ResumeSounds();
		}

		LogInfo("Exiting Quit Screen");
	}

	return bQuit;
}

ErrorCode CBagMasterWin::newGame() {
	Assert(IsValidObject(this));

	char sWorkStr[256];
	char szCInit[256];

	sWorkStr[0] = '\0';
	szCInit[0] = '\0';

	CBofString cInitWld(szCInit, 256);

	CBagPanWindow::FlushInputEvents();

	// Inits for a New Game
	_objSaveFl = false;

	CBagel *pApp = CBagel::getBagApp();
	// Find the starting .WLD file name
	if (pApp != nullptr) {
		pApp->getOption("Startup", "WLDFile", sWorkStr, STARTWORLD, 255);

		cInitWld = sWorkStr;
		MACROREPLACE(cInitWld);
	}

	loadGlobalVars(GLOBALWORLD);
	loadFile(cInitWld, "", true);

	return _errCode;
}

ErrorCode CBagMasterWin::loadFile(const CBofString &wldName, const CBofString &startWldName, bool restartFl, bool setStartFl) {
	char szLocalBuff[256];

	szLocalBuff[0] = '\0';

	Common::strcpy_s(szLocalBuff, "$SBARDIR\\GENERAL\\SYSTEM\\LEGAL.BMP");
	CBofString sWldFileName(szLocalBuff, 256);

	static bool bPainted = false;

	// Make sure we get a new set of vildroid filter variables
	g_engine->g_bGetVilVars = true;

	// Reset the Queued sound slot volumes back to default
	CBofSound::ResetQVolumes();

	if (!bPainted) {
		bPainted = true;
		MACROREPLACE(sWldFileName);
		CBofRect cRect;
		cRect.left = (640 - 520) / 2;
		cRect.top = (480 - 240) / 2;
		cRect.right = cRect.left + 520 - 1;
		cRect.bottom = cRect.top + 240 - 1;

		paintBitmap(this, sWldFileName.GetBuffer(), &cRect);
	}

	sWldFileName = wldName;

	// Reset unique ID for menus
	_menuCount = 0;

	// Keep track of what script we are in
	_wldScript = wldName;

	// This palette will be deleted so don't let anyone use it, until it is
	// replaced with a new one.
	CBofApp *pApp = CBofApp::getApp();
	if (pApp != nullptr) {
		pApp->setPalette(nullptr);
	}

	// Save all used objects (if going to another .WLD file)
	bool bRestore = false;
	if ((_storageDeviceList != nullptr) && !restartFl) {

		if (!_objSaveFl) {

			_objSaveFl = true;

			// Only allocate the object list when we really need it...
			if (_objList == nullptr) {
				_objList = (ST_OBJ *)BofAlloc(MAX_OBJS * sizeof(ST_OBJ));
				if (_objList != nullptr) {
					// Init to zero (we might not use all slots)
					BofMemSet(_objList, 0, MAX_OBJS * sizeof(ST_OBJ));

				} else {
					ReportError(ERR_MEMORY, "Could not allocate Object list");
				}
			}

			_storageDeviceList->SaveObjList(_objList, MAX_OBJS); // xxx

			// Save our SDEV location, so we can restore it from Kerpupu
			saveSDevStack();

		} else {
			bRestore = true;
		}
	}

	if (_variableList != nullptr) {
		_variableList->ReleaseVariables(false);
	}

	if (_gameSDevList != nullptr) {
		delete _gameSDevList;
		_gameSDevList = nullptr;
	}

	if (_gameWindow != nullptr) {
		delete _gameWindow;
		_gameWindow = nullptr;
	}

	if (_storageDeviceList != nullptr) {
		delete _storageDeviceList;
		_storageDeviceList = nullptr;
	}

	// Unload all current cursors
	for (int i = 0; i < MAX_CURSORS; i++) {
		if (_cursorList[i] != nullptr) {
			delete _cursorList[i];
			_cursorList[i] = nullptr;
		}
	}

	CBagMenu::SetUniversalObjectList(nullptr);

	_storageDeviceList = new CBagStorageDevManager();
	if (!_variableList) {
		_variableList = new CBagVarManager();
	}
	_gameSDevList = new CBofList<CBagStorageDev *>;

	MACROREPLACE(sWldFileName);

	TimerStart();

	if (FileExists(sWldFileName)) {
		// Force buffer to be big enough so that the entire script
		// is pre-loaded
		int nLength = FileLength(sWldFileName);
		char *pBuf = (char *)BofAlloc(nLength);
		if (pBuf != nullptr) {
			CBagIfstream fpInput(pBuf, nLength);

			CBofFile cFile;
			cFile.open(sWldFileName);
			cFile.read(pBuf, nLength);
			cFile.close();

			CBagMasterWin::loadFileFromStream(fpInput, startWldName);

			BofFree(pBuf);
		}

		// Possibly need to switch CDs
		CBagel *pBagApp = CBagel::getBagApp();
		if (pBagApp != nullptr) {
			_errCode = pBagApp->verifyCDInDrive(_diskId, _cdChangeAudio.GetBuffer());
			if (_errCode != ERR_NONE || g_engine->shouldQuit()) {
				close();
				return _errCode;
			}
		}

		// Now that we know we are on the correct CD, we can load the cursors
		// Only load the cursors that are not wield cursors
		for (int i = 0; i < MAX_CURSORS; i++) {
			if (_cursorList[i] != nullptr) {
				_cursorList[i]->load();
			}
		}

		setActiveCursor(0);
		CBagWield::SetWieldCursor(-1);

		if (g_restoreObjectListFl) {

			if (bRestore && _objSaveFl) {
				Assert(_storageDeviceList != nullptr);
				if (_storageDeviceList != nullptr) {
					// Use a preallocated buffer, trash it when we're done.
					Assert(_objList != nullptr);
					_storageDeviceList->RestoreObjList(_objList, MAX_OBJS);

					// All done with this list, can trash it now
					BofFree(_objList);
					_objList = nullptr;
				}
				_objSaveFl = false;
			}
		}
		g_restoreObjectListFl = true;

		// If a start wld is passed in then use it
		if (!startWldName.IsEmpty()) {
			_startWld = startWldName;
		}
		if (setStartFl) {
			if (!_startWld.IsEmpty()) {
				setStorageDev(_startWld);
			}
		}

		restoreActiveMessages(_storageDeviceList);

	} else {
		ReportError(ERR_FFIND, "Could not find World Script: %s", sWldFileName.GetBuffer());
	}
	LogInfo(BuildString("Time to Load %s, %ld ms", sWldFileName.GetBuffer(), timerStop()));

	return _errCode;
}

void CBagMasterWin::saveSDevStack() {
	Assert(IsValidObject(this));

	// Save our SDEV location, so we can restore it from Kerpupu
	char szLocStack[MAX_CLOSEUP_DEPTH][MAX_VAR_VALUE];
	char szTempBuf[256];

	memset(&szLocStack[0][0], 0, sizeof(char) * MAX_CLOSEUP_DEPTH * MAX_VAR_VALUE);
	szTempBuf[0] = '\0';
	CBagStorageDevWnd *pSDevWin = getCurrentStorageDev();
	if (pSDevWin != nullptr) {
		int i = 0;

		CBofString cStr = pSDevWin->GetName();
		if (!cStr.IsEmpty()) {
			Common::strcpy_s(szLocStack[i], cStr.GetBuffer());
			cStr = pSDevWin->GetPrevSDev();
			i++;
		}

		while ((i < MAX_CLOSEUP_DEPTH) && !cStr.IsEmpty()) {
			pSDevWin = (CBagStorageDevWnd *)_storageDeviceList->GetStorageDevice(cStr);
			if (pSDevWin != nullptr) {

				Common::strcpy_s(szLocStack[i], cStr.GetBuffer());

				i++;
				cStr = pSDevWin->GetPrevSDev();
			} else {
				break;
			}
		}
		i--;
		for (int j = i; j >= 0; j--) {

			if (szLocStack[j][0] != '\0') {
				Common::strcat_s(szTempBuf, szLocStack[j]);
				if (j != 0) {
					Common::strcat_s(szTempBuf, "~~");
				}
			}
		}

		// Variables cannot exceed MAX_VAR_VALUE characters in length (for Save/Restore)
		Assert(strlen(szTempBuf) < MAX_VAR_VALUE);

		// Store our current sdev location stack in a global variable.
		CBagVar *pVar = VAR_MANAGER->GetVariable("$LASTWORLD");
		if (pVar != nullptr) {
			cStr = szTempBuf;
			pVar->SetValue(cStr);
		}
	}
}

ErrorCode CBagMasterWin::loadGlobalVars(const CBofString &wldName) {
	Assert(IsValidObject(this));

	char szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString sWldFileName(szLocalBuff, 256);
	sWldFileName = wldName;

	delete _variableList;
	_variableList = new CBagVarManager();

	if (_variableList != nullptr) {

		MACROREPLACE(sWldFileName);

		if (FileExists(sWldFileName)) {
			// Force buffer to be big enough so that the entire script
			// is pre-loaded
			int nLength = FileLength(sWldFileName);
			char *pBuf = (char *)BofAlloc(nLength);
			if (pBuf != nullptr) {
				CBagIfstream fpInput(pBuf, nLength);

				CBofFile cFile;
				cFile.open(sWldFileName);
				cFile.read(pBuf, nLength);
				cFile.close();

				while (!fpInput.eof()) {
					fpInput.eatWhite();

					KEYWORDS keyword;

					if (!fpInput.eatWhite()) {
						break;
					}

					GetKeywordFromStream(fpInput, keyword);

					switch (keyword) {

					case VARIABLE: {
						CBagVar *pVar = new CBagVar;
						fpInput.eatWhite();
						pVar->setInfo(fpInput);
						pVar->SetGlobal();
						break;
					}

					case REMARK: {
						char s[256];
						fpInput.getCh(s, 255);
						break;
					}

					case STORAGEDEV:
					case START_WLD:
					case SYSSCREEN:
					case DISKID:
					case DISKAUDIO:
					case SHAREDPAL:
					case PDASTATE:
					default: {
						ParseAlertBox(fpInput, "Syntax Error:  Unexpected Type in Global Var Wld:", __FILE__, __LINE__);
						break;
					}
					}
				}

				BofFree(pBuf);
			}
		}
	}

	return _errCode;
}

ErrorCode CBagMasterWin::loadFileFromStream(CBagIfstream &input, const CBofString &wldName) {
	char szLocalStr[256];
	szLocalStr[0] = 0;
	CBofRect rRect;
	bool bIsWieldCursor = false;

	memset(szLocalStr, 0, 256);
	CBofString sWorkStr(szLocalStr, 256);

	_startWld = wldName;

	while (!input.eof()) {
		input.eatWhite();
		CBagStorageDev *pSDev = nullptr;
		int nFilter = 0;
		rRect.right = rRect.left - 1;
		rRect.bottom = rRect.top - 1;

		KEYWORDS keyword;

		if (!input.eatWhite()) {
			break;
		}

		GetKeywordFromStream(input, keyword);

		switch (keyword) {

		case STORAGEDEV: {
			char szNameBuff[256];
			char szTypeBuff[256];
			szNameBuff[0] = 0;
			szTypeBuff[0] = 0;

			memset(szNameBuff, 0, 256);
			memset(szTypeBuff, 0, 256);
			CBofString namestr(szNameBuff, 256);
			CBofString typestr(szTypeBuff, 256);

			input.eatWhite();
			GetAlphaNumFromStream(input, namestr);

			input.eatWhite();

			int nFadeId = 0;

			while (input.peek() != '{') {
				GetAlphaNumFromStream(input, sWorkStr);
				input.eatWhite();
				if (!sWorkStr.Find("AS")) {
					input.eatWhite();
					GetAlphaNumFromStream(input, typestr);
				} else if (!sWorkStr.Find("RECT")) {
					input.eatWhite();
					getRectFromStream(input, rRect);
				} else if (!sWorkStr.Find("FILTER")) {
					input.eatWhite();
					GetIntFromStream(input, nFilter);
				} else if (!sWorkStr.Find("FADE")) { // Note that this should usually be set in the link
					input.eatWhite();
					GetIntFromStream(input, nFadeId);
				} else {
					// There is an error here
					LogError(BuildString("FAILED on argument of storage device %s : %s", namestr.GetBuffer(), typestr.GetBuffer()));
					return ERR_UNKNOWN;
				}

				input.eatWhite();
			}
			pSDev = onNewStorageDev(typestr);
			if (!pSDev) {
				LogError(BuildString("FAILED on open of storage device %s : %s", namestr.GetBuffer(), typestr.GetBuffer()));
				return ERR_UNKNOWN;
			}

			// Default DISK ID for this storage device is the same
			// as the ID specified for this .WLD script file.
			pSDev->SetDiskID(_diskId);

			if (rRect.width() && rRect.height())
				pSDev->SetRect(rRect);

			pSDev->loadFileFromStream(input, namestr, false);
			if (nFilter) {
				pSDev->SetFilterId((uint16)nFilter);

				// Set the filter on the window.
				onNewFilter(pSDev, typestr);
			}

			if (nFadeId != 0) {
				pSDev->SetFadeId((uint16)nFadeId);
			}

			_gameSDevList->addToTail(pSDev);
			break;
		}

		case START_WLD: {
			char str[256];
			str[0] = '\0';

			CBofString sStr(str, 256);

			input.eatWhite();
			if (input.peek() == '=') {
				input.getCh();
				input.eatWhite();
				GetAlphaNumFromStream(input, sStr);

				// Only use the start wld if not specified elsewhere
				_startWld = sStr;
				LogInfo(BuildString("START_WLD set to %s", _startWld.GetBuffer()));
			}
			break;
		}

		case WIELDCURSOR:
			bIsWieldCursor = true;
			// fallthrough
		case CURSOR: {
			char str[256];
			str[0] = 0;

			CBofString sStr(str, 256);
			int nId;

			input.eatWhite();
			GetIntFromStream(input, nId);
			input.eatWhite();
			if (input.peek() == '=') {
				int x, y;
				input.getCh();
				input.eatWhite();

				GetIntFromStream(input, x);
				input.eatWhite();

				GetIntFromStream(input, y);
				input.eatWhite();

				GetAlphaNumFromStream(input, sStr);
				MACROREPLACE(sStr);

				// Specify if we have a shared palette or not, look for
				// the USESHAREDPAL token after the full cursor specification
				bool bUseShared = false;

				input.eatWhite();
				if (input.peek() == '=') {
					char szSharedPalToken[256];
					CBofString tStr(szSharedPalToken, 256);

					input.getCh();
					input.eatWhite();

					// Check for shared pal token, if there, then create our cursor
					// with the shared palette bit set
					GetAlphaNumFromStream(input, tStr);
					if (tStr.Find("USESHAREDPAL") != -1) {
						bUseShared = true;
					}
				}

				CBagCursor *pCursor = new CBagCursor(sStr, bUseShared);
				if (pCursor != nullptr) {
					pCursor->setHotspot(x, y);

					Assert(nId >= 0 && nId < MAX_CURSORS);

					// Delete any previous cursor
					if (_cursorList[nId] != nullptr) {
						delete _cursorList[nId];
					}
					_cursorList[nId] = pCursor;

					// Set the wielded cursor status (needed for
					// a load time optimization)
					pCursor->setWieldCursor(bIsWieldCursor);

				} else {
					ReportError(ERR_MEMORY, "Could not allocate a CBagCursor");
				}

			} else {
				ReportError(ERR_UNKNOWN, "Bad cursor syntax");
			}
			break;
		}

		case PDASTATE: {
			char szPDAState[256];
			szPDAState[0] = '\0';
			CBofString sStr(szPDAState, 256);
			input.eatWhite();
			if (input.peek() == '=') {
				input.getCh();
				input.eatWhite();

				GetAlphaNumFromStream(input, sStr);

				if (sStr.Find("MAP") != -1) {
					SBBasePda::setPdaMode(MAPMODE);
				} else if (sStr.Find("INV") != -1) {
					SBBasePda::setPdaMode(INVMODE);
				} else if (sStr.Find("LOG") != -1) {
					SBBasePda::setPdaMode(LOGMODE);
				}
				LogInfo(BuildString("PDASTATE = %s", szPDAState));
			}
			break;
		}

		// Implement shared palettes
		case SHAREDPAL: {
			char szBmpFileName[256];
			szBmpFileName[0] = '\0';
			CBofString sStr(szBmpFileName, 256);
			input.eatWhite();
			if (input.peek() == '=') {
				input.getCh();
				input.eatWhite();

				GetAlphaNumFromStream(input, sStr);
				MACROREPLACE(sStr);

				// Read the palette in and keep it hanging around for later use
				CBofPalette::setSharedPalette(sStr);

				LogInfo(BuildString("SHAREDPAL = %s", sStr.GetBuffer()));
			}
			break;
		}

		case SYSSCREEN: {

			input.eatWhite();
			if (input.peek() == '=') {
				input.getCh();
				input.eatWhite();

				GetAlphaNumFromStream(input, _sysScreen);
				MACROREPLACE(_sysScreen);

				LogInfo(BuildString("SYSSCREEN = %s", _sysScreen.GetBuffer()));
			}
			break;
		}

		// What audio file should play for this disk swap
		case DISKAUDIO: {
			char szDiskID[256];
			szDiskID[0] = '\0';
			CBofString sStr(szDiskID, 256);

			input.eatWhite();
			if (input.peek() == '=') {
				input.getCh();
				input.eatWhite();

				GetAlphaNumFromStream(input, _cdChangeAudio);
				MACROREPLACE(_cdChangeAudio);

				LogInfo(BuildString("DISKAUDIO = %s", _cdChangeAudio.GetBuffer()));
			}
			break;
		}

		case DISKID: {
			input.eatWhite();
			if (input.peek() == '=') {
				input.getCh();
				input.eatWhite();
				int n;

				GetIntFromStream(input, n);
				_diskId = (uint16)n;

				LogInfo(BuildString("DISKID = %d", _diskId));

			} else {
			}
			break;
		}

		case VARIABLE: {
			CBagVar *xVar = new CBagVar;
			// LogInfo("New global variable");
			input.eatWhite();
			xVar->setInfo(input);
			break;
		}

		case REMARK: {
			char s[255];
			input.getCh(s, 255);
			break;
		}

		default: {
			ParseAlertBox(input, "Syntax Error:", __FILE__, __LINE__);
			break;
		}
		}

	} // While not eof

	// Add everything to the window
	return _errCode;
}

ErrorCode CBagMasterWin::setStorageDev(const CBofString &wldName, bool entry) {
	Assert(CBofObject::IsValidObject(&wldName));

	char szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString sExt(szLocalBuff, 256);

	sExt = wldName.Right(4);
	if (_gameWindow)
		_gameWindow->setOnUpdate(false);

	if (!sExt.Find(".wld") || !sExt.Find(".WLD") || (wldName.Find(".WLD~~") > 0) || (wldName.Find(".wld~~") > 0)) {

		// This is to stop it from going out of scope before
		// the message is received.
		Assert(g_nString >= 0 && g_nString < NUM_MSG_STRINGS);
		Assert(wldName.GetLength() < 512);

		strncpy(g_szString[g_nString], wldName, 511);

		postUserMessage(WM_ENTER_NEW_WLD, (uint32)g_nString);

		if (++g_nString >= NUM_MSG_STRINGS) {
			g_nString = 0;
		}

	} else if (entry) {

		gotoNewWindow(&wldName);

	} else {

		// This is to stop the string from going out of scope before
		// the message is received.
		Assert(g_nString >= 0 && g_nString < NUM_MSG_STRINGS);
		Assert(wldName.GetLength() < 512);
		Common::strcpy_s(g_szString[g_nString], wldName);

		postUserMessage(WM_EXIT_CLOSE_UP_WINDOW, (uint32)g_nString);

		if (++g_nString >= NUM_MSG_STRINGS) {
			g_nString = 0;
		}
	}

	return ERR_NONE;
}

ErrorCode CBagMasterWin::onHelp(const CBofString &helpFile, bool /*bSaveBkg*/, CBofWindow *parent) {
	Assert(IsValidObject(this));

	if (g_engine->isDemo())
		return ERR_NONE;

	if (!helpFile.IsEmpty()) {
		CBagHelp cHelp;

		char szLocalBuff[256];
		szLocalBuff[0] = '\0';
		CBofString sFile(szLocalBuff, 256);

		sFile = helpFile;
		MACROREPLACE(sFile);

		// use specified bitmap as this dialog's image
		char szBkg[256];
		szBkg[0] = '\0';
		CBofString sBkg(szBkg, 256);

		sBkg = BuildString("$SBARDIR%sGENERAL%sRULES%sHELPSCRN.BMP", PATH_DELIMETER, PATH_DELIMETER, PATH_DELIMETER);
		MACROREPLACE(sBkg);

		CBofBitmap *pBmp = Bagel::loadBitmap(sBkg);
		cHelp.setBackdrop(pBmp);

		CBofRect cRect = cHelp.getBackdrop()->getRect();

		if (parent == nullptr)
			parent = this;

		// create the dialog box
		cHelp.create("HelpDialog", cRect.left, cRect.top, cRect.width(), cRect.height(), parent);

		cHelp.SetHelpFile(sFile.GetBuffer());
		cHelp.doModal();
		cHelp.detach();
	}

	return _errCode;
}

bool g_bWaitOK = false;

void CBagMasterWin::onKeyHit(uint32 keyCode, uint32 repCount) {
	Assert(IsValidObject(this));

	int nVol;

	switch (keyCode) {
	// Dynamic Midi Volume increase
	case BKEY_ALT_UP:
		nVol = getMidiVolume();
		if (nVol < 12) {
			nVol++;
			setMidiVolume(nVol);
		}

		break;

	// Dynamic Midi Volume decrease
	case BKEY_ALT_DOWN:
		nVol = getMidiVolume();
		if (nVol > 0) {
			nVol--;
			setMidiVolume(nVol);
		}
		break;

	case BKEY_SPACE:
		if (g_bWaitOK) {
			g_bWaitOK = false;

			// Play the tick-tock sound
			if (_waitSound != nullptr) {
				_waitSound->play();
			}

			VAR_MANAGER->IncrementTimers();

			// Prefilter this guy, could cause something to change in the
			// pan or the PDA or a closeup.
			_gameWindow->SetPreFilterPan(true);

			_gameWindow->AttachActiveObjects();
		}
		break;

	// Quit
	case BKEY_ALT_q:
	case BKEY_ALT_F4:
		if (g_engine->isDemo() || showQuitDialog(this, false)) {
			close();
			g_engine->quitGame();
		}
		break;

	//  Help
	case BKEY_F1:
		if (_gameWindow != nullptr) {
			onHelp(_gameWindow->GetHelpFilename(), false);
		}
		break;

	// Save a Game
	case BKEY_ALT_s:
	case BKEY_SAVE:
		showSaveDialog(this, false);
		break;

	// Restore Game
	case BKEY_RESTORE:
		showRestoreDialog(this, false);
		break;

	// System options dialog
	case BKEY_ALT_o:
	case BKEY_F4:
		showSystemDialog(false);
		break;

	// Toggle sound On/Off
	case BKEY_ALT_m:
		muteToggle();
		break;

	// Restart the game
	case BKEY_F12:
		showRestartDialog(this, false);
		break;

	// Default action
	default:
		if (_gameWindow)
			_gameWindow->onKeyHit(keyCode, repCount);
		break;
	}

	CBofWindow::onKeyHit(keyCode, repCount);
}

void CBagMasterWin::onClose() {
	Assert(IsValidObject(this));

	if (_gameWindow)
		_gameWindow->onClose();

	g_engine->quitGame();
}

ErrorCode CBagMasterWin::gotoNewWindow(const CBofString *str) {
	Assert(IsValidObject(this));
	Assert(str != nullptr);
	Assert(CBofObject::IsValidObject(str));

	CBagStorageDev *pSDev;

	char szWorkStr[256];
	char szPrevSDevStr[256];
	char szCurSDevStr[256];

	szWorkStr[0] = '\0';
	szPrevSDevStr[0] = '\0';
	szCurSDevStr[0] = '\0';

	CBofString sWorkStr(szWorkStr, 256);
	CBofString sPrevSDevStr(szPrevSDevStr, 256);
	CBofString sCurrSDevStr(szCurSDevStr, 256);

	int n = str->Find("~~");
	sWorkStr = *str;
	bool bPrev = false;

	while (n > 0) {
		sPrevSDevStr = sWorkStr.Left(n);
		sWorkStr = sWorkStr.Mid(n + 2);
		n = sWorkStr.Find("~~");
		if (n > 0) {
			sCurrSDevStr = sWorkStr.Left(n);
			pSDev = _storageDeviceList->GetStorageDevice(sCurrSDevStr);
			if (pSDev != nullptr) {
				pSDev->SetPrevSDev(sPrevSDevStr);
				bPrev = true;
			}
		} else {
			sCurrSDevStr = sWorkStr;
			pSDev = _storageDeviceList->GetStorageDevice(sCurrSDevStr);
			if (pSDev != nullptr) {
				pSDev->SetPrevSDev(sPrevSDevStr);
				bPrev = true;
			}
		}
	}

	pSDev = _storageDeviceList->GetStorageDevice(sWorkStr);
	if (pSDev != nullptr) {

		LogInfo(BuildString("Switching to SDEV: %s", sWorkStr.GetBuffer()));

		if (_gameWindow) {
			_gameWindow->detach();

			//  If the new storage device is equal to the last windows previous
			//  lets not go in a circle
			//  If the current game window did not have a previous win
			if ((_gameWindow->GetPrevSDev().IsEmpty()) || (_gameWindow->GetPrevSDev() != pSDev->GetName())) {
				if (!bPrev) {
					pSDev->SetPrevSDev(_gameWindow->GetName());
				}
			}
		}

		// Don't allow recursion
		if (!pSDev->GetPrevSDev().IsEmpty() && pSDev->GetPrevSDev().CompareNoCase(pSDev->GetName()) == 0) {
			pSDev->SetPrevSDev("");
		}

		_gameWindow = (CBagStorageDevWnd *)pSDev;
		setCICStatus(pSDev);

		int nFadeId = pSDev->GetFadeId();

		if (_fadeIn != 0)
			pSDev->SetFadeId((uint16)_fadeIn);

		// Reset paints
		g_allowPaintFl = true;

		pSDev->attach();

		pSDev->SetFadeId((uint16)nFadeId);
		_fadeIn = 0;
	}

	return _errCode;
}

bool CBagMasterWin::showRestartDialog(CBofWindow *win, bool bSaveBkg) {
	Assert(IsValidObject(this));

	if (g_engine->isDemo())
		return false;

	CBagStorageDevWnd *pSdev = getCurrentStorageDev();
	if ((pSdev == nullptr) || (pSdev->GetDeviceType() == SDEV_GAMEWIN) || (pSdev->GetDeviceType() == SDEV_ZOOMPDA)) {

		LogInfo("Showing Restart Screen");

		if (win == nullptr) {
			win = this;
		}

		CBagRestartDialog cDlg(_sysScreen.GetBuffer(), win);

		CBofWindow *pLastWin = g_hackWindow;
		g_hackWindow = &cDlg;

		// Don't allow save of background
		if (!bSaveBkg) {
			int lFlags = cDlg.getFlags();
			cDlg.setFlags(lFlags & ~BOFDLG_SAVEBACKGND);
		}

		bool bSaveTimer = g_bPauseTimer;
		g_bPauseTimer = true;
		int nReturn = cDlg.doModal();
		g_bPauseTimer = bSaveTimer;

		g_hackWindow = pLastWin;

		LogInfo("Exiting Restart Screen");

		return nReturn == RESTART_BTN;
	}

	return false;
}

void CBagMasterWin::onUserMessage(uint32 message, uint32 param) {
	Assert(IsValidObject(this));

	switch (message) {
	case WM_SHOW_SYSTEM_DLG:
		showSystemDialog();
		break;

	// Ask user if they want to restart or restore a game
	case WM_DIE: {
		// This was causing a hang if you die while
		// there is a background audio looping.  So, I am going
		// to kill all audio.  This will probably break when you die
		// with audio only (i.e. no smacker),  but that can be fixed in
		// script.
		//
		CBofSound::StopSounds();

		// Kill any waiting PDA messages that are queued up...
		CBagPDA::RemoveFromMovieQueue(nullptr);

		CBofBitmap cBmp(width(), height(), CBagel::getApp()->getPalette());
		cBmp.fillRect(nullptr, COLOR_BLACK);
		cBmp.fadeLines(this, 0, 0);

		char szBuf[MAX_FNAME];

		Common::strcpy_s(szBuf, buildSysDir("DIED2.BMP"));
		if (param == 2) {
			Common::strcpy_s(szBuf, buildSysDir("START.BMP"));
		}

		CBagStartDialog cDlg(szBuf, this);

		CBofWindow *pLastWin = g_hackWindow;
		g_hackWindow = &cDlg;

		g_bAllowRestore = true;
		int nRetVal = cDlg.doModal();
		g_bAllowRestore = false;

		g_hackWindow = pLastWin;

		// Hide that dialog
		cBmp.paint(this, 0, 0);

		switch (nRetVal) {

		case RESTORE_BTN:
			break;

		case RESTART_BTN:
			newGame();
			break;

		case QUIT_BTN:
			close();
			g_engine->quitGame();
			break;
		}

		return;
	}

	case WM_ENTER_NEW_WLD: {
		char szLocalBuff[256];
		szLocalBuff[0] = '\0';
		CBofString sWldScript(szLocalBuff, 256);

		// User info is an index into an array of temporary string buffers
		Assert(param < NUM_MSG_STRINGS);
		sWldScript = g_szString[(int)param];

		char szStartWld[256];
		szStartWld[0] = '\0';
		CBofString sStartWld(szStartWld, 256);

		int n = sWldScript.Find("~~");
		if (n > 0) {
			sStartWld = sWldScript.Mid(n + 2);
			sWldScript = sWldScript.Left(n);
		}
		loadFile(sWldScript, sStartWld);
		break;
	}

	case WM_ENTER_PAN_WINDOW:
	case WM_ENTER_CLOSE_UP_WINDOW:
		// Should never be called
		Assert(false);
		break;

	case WM_EXIT_CLOSE_UP_WINDOW: {
		CBofString cStr;

		// User info is an index into an array of tempory string buffers
		Assert(param < NUM_MSG_STRINGS);
		cStr = g_szString[(int)param];

		CBagStorageDev *pSDev = _storageDeviceList->GetStorageDevice(cStr);

		if (pSDev) {
			if (_gameWindow) {
				_gameWindow->detach();
			}

			pSDev->attach();
			pSDev->SetPreFilterPan(true);

			_gameWindow = (CBagStorageDevWnd *)pSDev;

			// Reset the CIC var
			setCICStatus(pSDev);
		} else {
			// Report error
		}
		break;
	}

	default:
		break;
	}

	if (_gameWindow)
		_gameWindow->setOnUpdate(true);
}

ErrorCode CBagMasterWin::Run() {
	return _errCode;
}

void CBagMasterWin::setActiveCursor(int cursorId) {
	Assert(cursorId >= 0 && cursorId < MAX_CURSORS);

	if (_cursorList[cursorId] != nullptr) {
		_cursorList[cursorId]->setCurrent();
		_curCursor = cursorId;
	}
}

void CBagMasterWin::fillSaveBuffer(ST_BAGEL_SAVE *saveBuf) {
	Assert(IsValidObject(this));
	Assert(saveBuf != nullptr);

	//
	// Fill the save game buffer with all the info we need to restore this game
	//

	// 1st, wipe it
	BofMemSet(saveBuf, 0, sizeof(ST_BAGEL_SAVE));

	CBagel *pApp = CBagel::getBagApp();
	if (pApp != nullptr) {
		CBagMasterWin *pWin = pApp->getMasterWnd();
		if (pWin != nullptr) {
			// Save Global variables
			CBagVarManager *pVarManager = getVariableManager();
			if (pVarManager != nullptr) {

				// Walk variable list and save each global variable
				int j = 0;
				int n = pVarManager->GetNumVars();
				for (int i = 0; i < n; i++) {
					CBagVar *pVar = pVarManager->GetVariable(i);
					if (pVar != nullptr) {
						// Need to save local variables in flashbacks.
						// Let me know if this breaks anything.
						//
						// If it's a global variable, then we need to store it

						if (!pVar->GetName().IsEmpty()) {
							Assert(strlen(pVar->GetName()) < MAX_VAR_NAME);
							Common::strcpy_s(saveBuf->m_stVarList[j].m_szName, pVar->GetName());
						}

						if (!pVar->GetValue().IsEmpty()) {
							Assert(strlen(pVar->GetValue()) < MAX_VAR_VALUE);
							Common::strcpy_s(saveBuf->m_stVarList[j].m_szValue, pVar->GetValue());
						}

						saveBuf->m_stVarList[j].m_nType = (uint16)pVar->GetType();
						saveBuf->m_stVarList[j].m_bGlobal = (byte)pVar->IsGlobal();
						saveBuf->m_stVarList[j].m_bConstant = (byte)pVar->IsConstant();
						saveBuf->m_stVarList[j].m_bReference = (byte)pVar->IsReference();
						saveBuf->m_stVarList[j].m_bTimer = (byte)pVar->IsTimer();
						saveBuf->m_stVarList[j].m_bRandom = (byte)pVar->IsRandom();
						saveBuf->m_stVarList[j].m_bNumeric = (byte)pVar->IsNumeric();
						saveBuf->m_stVarList[j].m_bAttached = (byte)pVar->isAttached();
						saveBuf->m_stVarList[j].m_bUsed = 1;
						j++;

						// Can't exceed MAX_VARS
						Assert(j < MAX_VARS);
						//}
					}
				}
			}

			// Remember current script file
			strncpy(saveBuf->m_szScript, getWldScript().GetBuffer(), MAX_FNAME - 1);

			CBagStorageDevWnd *pSDevWin = getCurrentStorageDev();
			if (pSDevWin != nullptr) {
				char szLocalStr[256];
				szLocalStr[0] = 0;
				CBofString cStr(szLocalStr, 256);

				saveBuf->m_nLocType = pSDevWin->GetDeviceType();

				// Remember the pan's position
				if (saveBuf->m_nLocType == SDEV_GAMEWIN) {
					CBagPanWindow *pPanWin = (CBagPanWindow *)pSDevWin;
					CBofRect cPos = pPanWin->GetViewPort();

					saveBuf->m_nLocX = (uint16)cPos.left;
					saveBuf->m_nLocY = (uint16)cPos.top;
				}

				CBagStorageDevManager *pManager = getStorageDevManager();
				if (pManager != nullptr) {
					pManager->SaveObjList(&saveBuf->m_stObjList[0], MAX_OBJS);
					if (isObjSave()) {

						Assert(_objList != nullptr);

						BofMemCopy(&saveBuf->m_stObjListEx[0], _objList, sizeof(ST_OBJ) * MAX_OBJS);
						saveBuf->m_bUseEx = 1;
					}

					// Save current storage device location (stack)
					int i = 0;
					cStr = pSDevWin->GetName();
					while ((i < MAX_CLOSEUP_DEPTH) && !cStr.IsEmpty()) {
						pSDevWin = (CBagStorageDevWnd *)pManager->GetStorageDevice(cStr);
						Common::strcpy_s(saveBuf->m_szLocStack[i], cStr.GetBuffer());
						i++;
						cStr.Free();
						if (pSDevWin != nullptr) {
							cStr = pSDevWin->GetPrevSDev();
						}
					}
				}
			}
		}
	}
}

bool CBagMasterWin::showSaveDialog(CBofWindow *win, bool bSaveBkg) {
	Assert(IsValidObject(this));

	if (g_engine->isDemo())
		return false;

	if (!g_engine->_useOriginalSaveLoad) {
		return g_engine->saveGameDialog();
	}

	bool bSaved = false;
	CBagStorageDevWnd *pSdev = getCurrentStorageDev();
	if ((pSdev == nullptr) || (pSdev->GetDeviceType() == SDEV_GAMEWIN) || (pSdev->GetDeviceType() == SDEV_ZOOMPDA)) {

		LogInfo("Showing Save Screen");
		CBofSound::PauseSounds();
		ST_BAGEL_SAVE *pSaveBuf = (ST_BAGEL_SAVE *)BofAlloc(sizeof(ST_BAGEL_SAVE));

		if (pSaveBuf != nullptr) {
			CBagSaveDialog cSaveDialog;
			fillSaveBuffer(pSaveBuf);
			cSaveDialog.setSaveGameBuffer((byte *)pSaveBuf, sizeof(ST_BAGEL_SAVE));

			// Use specified bitmap as this dialog's image
			CBofBitmap *pBmp = Bagel::loadBitmap(_sysScreen.GetBuffer());

			cSaveDialog.setBackdrop(pBmp);

			CBofRect cRect = cSaveDialog.getBackdrop()->getRect();

			// Don't allow save of background
			if (!bSaveBkg) {
				int lFlags = cSaveDialog.getFlags();
				cSaveDialog.setFlags(lFlags & ~BOFDLG_SAVEBACKGND);
			}

			// Create the dialog box
			cSaveDialog.create("Save Dialog", cRect.left, cRect.top, cRect.width(), cRect.height(), win);

			bool bSaveTimer = g_bPauseTimer;
			g_bPauseTimer = true;
			int nId = cSaveDialog.doModal();
			g_bPauseTimer = bSaveTimer;

			bSaved = (nId == SAVE_BTN);

			cSaveDialog.detach();

			BofFree(pSaveBuf);
		} else {
			ReportError(ERR_MEMORY, "Unable to allocate the Save Game Buffer");
		}

		CBofSound::ResumeSounds();

		LogInfo("Exiting Save Screen");
	}

	return bSaved;
}

void CBagMasterWin::doRestore(ST_BAGEL_SAVE *saveBuf) {
	Assert(IsValidObject(this));
	Assert(saveBuf != nullptr);

	char szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString cScript(szLocalBuff, 256);

	cScript = saveBuf->m_szScript;

	char szBuf[60], szCloseup[256];
	char szLocalStr[256];
	szLocalStr[0] = 0;
	CBofString cStr(szLocalStr, 256);
	int i;

	// Rebuild the stack of locations (Could be 3 closups deep)
	szCloseup[0] = '\0';
	for (i = MAX_CLOSEUP_DEPTH - 1; i >= 0; i--) {
		if (saveBuf->m_szLocStack[i][0] != '\0') {
			Common::sprintf_s(szBuf, "%s~~", saveBuf->m_szLocStack[i]);
			Common::strcat_s(szCloseup, szBuf);
		}
	}
	int n = strlen(szCloseup);
	if (szCloseup[n - 1] == '~') {
		szCloseup[n - 1] = '\0';
		szCloseup[n - 2] = '\0';
	}

	// Designate closeups
	if (saveBuf->m_nLocType == SDEV_CLOSEP) {
		Common::strcat_s(szCloseup, " AS CLOSEUP");
	}

	cStr = szCloseup;

	// Don't allow a local restore
	setSaveObjs(false);

	// Make sure that all global variables are loaded and available
	loadGlobalVars(GLOBALWORLD);

	// Tell BAGEL to start over with this script
	loadFile(cScript, cStr, false, false);

	if (!ErrorOccurred()) {

		// Restore all variables
		//
		CBagVarManager *pVarManager = getVariableManager();
		if (pVarManager != nullptr) {

			// Reset the Global Vars with these new settings
			for (i = 0; i < MAX_VARS; i++) {
				// If this entry is actually used to store Var info
				if (saveBuf->m_stVarList[i].m_bUsed) {
					// Find the Global Var (already in memory)
					CBagVar *pVar = pVarManager->GetVariable(saveBuf->m_stVarList[i].m_szName);
					if (pVar != nullptr) {
						pVar->SetValue(saveBuf->m_stVarList[i].m_szValue);

					} else {
						LogError(BuildString("Global Variable NOT found: %s", saveBuf->m_stVarList[i].m_szName));
					}
				}
			}
		}

		CBagStorageDevManager *pSDevManager = getStorageDevManager();

		if (pSDevManager != nullptr) {
			// Restore any extra obj list info (for .WLD swapping)
			if (_objList == nullptr) {
				_objList = (ST_OBJ *)BofAlloc(MAX_OBJS * sizeof(ST_OBJ));
				if (_objList != nullptr) {
					// Init to nullptr (might not use all slots)
					BofMemSet(_objList, 0, MAX_OBJS * sizeof(ST_OBJ));

				} else {
					ReportError(ERR_MEMORY);
				}
			}

			BofMemCopy(getObjList(), &saveBuf->m_stObjListEx[0], sizeof(ST_OBJ) * MAX_OBJS);

			if (saveBuf->m_bUseEx) {
				setSaveObjs(true);
			} else {
				setSaveObjs(false);
			}

			pSDevManager->RestoreObjList(&saveBuf->m_stObjList[0], MAX_OBJS);
		}

		// If it's a Panorama, the set it's View position.
		if (saveBuf->m_nLocType == SDEV_GAMEWIN) {
			g_engine->g_cInitLoc.x = saveBuf->m_nLocX;
			g_engine->g_cInitLoc.y = saveBuf->m_nLocY;
			g_engine->g_bUseInitLoc = true;
		}

		// Now set the start storage device and let the game start
		if (!_startWld.IsEmpty()) {
			setStorageDev(_startWld);
		}

		restoreActiveMessages(pSDevManager);
	}
}

bool CBagMasterWin::showRestoreDialog(CBofWindow *win, bool bSaveBkg) {
	Assert(IsValidObject(this));
	if (g_engine->isDemo())
		return false;

	if (!g_engine->_useOriginalSaveLoad) {
		return g_engine->loadGameDialog();
	}

	bool bRestored = false;
	CBagStorageDevWnd *pSdev;

	if (g_bAllowRestore || ((pSdev = getCurrentStorageDev()) == nullptr) || (pSdev->GetDeviceType() == SDEV_GAMEWIN) || (pSdev->GetDeviceType() == SDEV_ZOOMPDA)) {

		LogInfo("Showing Restore Screen");

		CBofSound::PauseSounds();

		CBagRestoreDialog cRestoreDialog;

		// Use specified bitmap as this dialog's image
		CBofBitmap *pBmp = Bagel::loadBitmap(_sysScreen.GetBuffer());

		cRestoreDialog.setBackdrop(pBmp);

		CBofRect cRect = cRestoreDialog.getBackdrop()->getRect();

		// Don't allow save of background
		if (!bSaveBkg) {
			int lFlags;
			lFlags = cRestoreDialog.getFlags();

			cRestoreDialog.setFlags(lFlags & ~BOFDLG_SAVEBACKGND);
		}

		// Create the dialog box
		cRestoreDialog.create("Restore Dialog", cRect.left, cRect.top, cRect.width(), cRect.height(), win);

		CBofWindow *pLastWin = g_hackWindow;
		g_hackWindow = &cRestoreDialog;

		bool bSaveTimer = g_bPauseTimer;
		g_bPauseTimer = true;
		cRestoreDialog.doModal();
		g_bPauseTimer = bSaveTimer;

		cRestoreDialog.detach();

		bRestored = (!cRestoreDialog.ErrorOccurred() && cRestoreDialog.restored());
		cRestoreDialog.destroy();

		g_hackWindow = pLastWin;

		CBofSound::ResumeSounds();

		LogInfo("Exiting Restore Screen");
	}

	return bRestored;
}

// User options
#define DEFAULT_CORRECTION 2

bool CBagMasterWin::getFlyThru() {
	bool bFlyThrusOn = true;
	CBagel *pApp = CBagel::getBagApp();
	if (pApp != nullptr) {
		pApp->getOption(USER_OPTIONS, "FlyThroughs", &bFlyThrusOn, true);
	}

	return bFlyThrusOn;
}

int CBagMasterWin::getMidiVolume() {
	int nMidiVol = VOLUME_INDEX_DEFAULT;
	CBagel *pApp = CBagel::getBagApp();
	if (pApp != nullptr) {
		pApp->getOption(USER_OPTIONS, "MidiVolume", &nMidiVol, VOLUME_INDEX_DEFAULT);
		if (nMidiVol < VOLUME_INDEX_MIN || nMidiVol > VOLUME_INDEX_MAX) {
			nMidiVol = VOLUME_INDEX_DEFAULT;
		}
	}

	return nMidiVol;
}

void CBagMasterWin::setMidiVolume(int vol) {
	Assert(vol >= VOLUME_INDEX_MIN && vol <= VOLUME_INDEX_MAX);
	CBagel *pApp = CBagel::getBagApp();

	if (pApp != nullptr) {
		pApp->setOption(USER_OPTIONS, "MidiVolume", vol);
	}

	// We will let the sound subsystem do our volume control...
	CBofSound::SetVolume(vol, getWaveVolume());
}

int CBagMasterWin::getWaveVolume() {
	int nWaveVol = VOLUME_INDEX_DEFAULT;
	CBagel *pApp = CBagel::getBagApp();
	if (pApp != nullptr) {
		pApp->getOption(USER_OPTIONS, WAVE_VOLUME, &nWaveVol, VOLUME_INDEX_DEFAULT);

		if (nWaveVol < VOLUME_INDEX_MIN || nWaveVol > VOLUME_INDEX_MAX) {
			nWaveVol = VOLUME_INDEX_DEFAULT;
		}
	}

	return nWaveVol;
}

void CBagMasterWin::setWaveVolume(int vol) {
	Assert(vol >= VOLUME_INDEX_MIN && vol <= VOLUME_INDEX_MAX);
	CBagel *pApp = CBagel::getBagApp();

	if (pApp != nullptr) {
		pApp->setOption(USER_OPTIONS, WAVE_VOLUME, vol);
	}

	// We will let the sound subsystem do our volume control...
	CBofSound::SetVolume(getMidiVolume(), vol);
}

int CBagMasterWin::getCorrection() {
	int nCorrection = DEFAULT_CORRECTION;
	CBagel *pApp = CBagel::getBagApp();
	if (pApp != nullptr) {
		pApp->getOption(USER_OPTIONS, "Correction", &nCorrection, DEFAULT_CORRECTION);

		if (nCorrection < 0 || nCorrection > 6) {
			nCorrection = DEFAULT_CORRECTION;
		}
	}

	if (nCorrection == 6) {
		nCorrection = 0;
	} else {
		nCorrection = (1 << nCorrection);
	}

	return nCorrection;
}

void CBagMasterWin::setCorrection(int correction) {
	Assert(correction >= 0 && correction <= 32);

	int nActualCorr = 2;

	switch (correction) {
	case 0:
		nActualCorr = 6;
		break;

	case 1:
		nActualCorr = 0;
		break;

	case 2:
		nActualCorr = 1;
		break;

	case 4:
		nActualCorr = 2;
		break;

	case 8:
		nActualCorr = 3;
		break;

	case 16:
		nActualCorr = 4;
		break;

	case 32:
		nActualCorr = 5;
		break;
	}

	CBagel *pApp = CBagel::getBagApp();
	if (pApp != nullptr) {
		pApp->setOption(USER_OPTIONS, "Correction", nActualCorr);
	}
}

int CBagMasterWin::getPanSpeed() {
	int n = 1;
	CBagel *pApp = CBagel::getBagApp();
	if (pApp != nullptr) {
		pApp->getOption(USER_OPTIONS, "PanSpeed", &n, 1);

		if (n < 0 || n > 5) {
			n = 1;
		}
	}

	return n;
}

void CBagMasterWin::setPanSpeed(int speed) {
	Assert(speed >= 0 && speed <= 5);
	CBagel *pApp = CBagel::getBagApp();

	if (pApp != nullptr) {
		pApp->setOption(USER_OPTIONS, "PanSpeed", speed);
	}
}

bool CBagMasterWin::getPanimations() {
	bool bPanims = 0;
	CBagel *pApp = CBagel::getBagApp();
	if (pApp != nullptr) {
		pApp->getOption(USER_OPTIONS, "Panimations", &bPanims, true);
	}

	return bPanims;
}

void CBagMasterWin::setPanimations(bool panimsFl) {
	CBagel *pApp = CBagel::getBagApp();

	if (pApp != nullptr) {
		pApp->setOption(USER_OPTIONS, "Panimations", panimsFl);
	}
}

void CBagMasterWin::muteToggle() {
	static int nMidiVol = VOLUME_INDEX_MIN;
	static int nWaveVol = VOLUME_INDEX_MIN;
	static bool bMute = false;

	if (bMute) {
		// Restore settings
		setMidiVolume(nMidiVol);
		setWaveVolume(nWaveVol);

	} else {
		// Save current settings
		nMidiVol = getMidiVolume();
		nWaveVol = getWaveVolume();

		// Mute
		setMidiVolume(VOLUME_INDEX_MIN);
		setWaveVolume(VOLUME_INDEX_MIN);
	}

	// Toggle mute-ness
	bMute = !bMute;
}

void CBagMasterWin::forcePaintScreen() {
	CBagel *pApp = CBagel::getBagApp();
	if (pApp != nullptr) {
		CBagMasterWin *pWin = pApp->getMasterWnd();
		if (pWin != nullptr) {
			CBagStorageDevWnd *pSDev = pWin->getCurrentStorageDev();
			if (pSDev != nullptr) {
				pSDev->PaintScreen(nullptr);
			}
		}
	}
}

ErrorCode paintBeveledText(CBofWindow *win, CBofRect *rect, const CBofString &cString, const int size, const int weight, const RGBCOLOR color, int justify, uint32 format, int font) {
	Assert(win != nullptr);
	Assert(rect != nullptr);

	CBofBitmap cBmp(rect->width(), rect->height(), nullptr, false);

	// Assume no error
	ErrorCode errCode = ERR_NONE;

	CBofRect r = cBmp.getRect();
	CBofPalette *pPal = nullptr;
	CBofApp *pApp = CBofApp::getApp();
	if (pApp != nullptr) {
		pPal = pApp->getPalette();
	}

	if (pPal != nullptr) {
		cBmp.fillRect(nullptr, pPal->getNearestIndex(RGB(92, 92, 92)));

		cBmp.drawRect(&r, pPal->getNearestIndex(RGB(0, 0, 0)));
	} else {
		cBmp.fillRect(nullptr, COLOR_BLACK);
	}

	int i, left, top, right, bottom;
	byte c1, c2;

	c1 = 3;
	c2 = 9;
	CBofRect cBevel = r;

	left = cBevel.left;
	top = cBevel.top;
	right = cBevel.right;
	bottom = cBevel.bottom;

	r.left += 6;
	r.top += 3;
	r.right -= 5;
	r.bottom -= 5;

	for (i = 1; i <= 3; i++) {
		cBmp.line(left + i, bottom - i, right - i, bottom - i, c1);
		cBmp.line(right - i, bottom - i, right - i, top + i - 1, c1);
	}

	for (i = 1; i <= 3; i++) {
		cBmp.line(left + i, bottom - i, left + i, top + i - 1, c2);
		cBmp.line(left + i, top + i - 1, right - i, top + i - 1, c2);
	}

	paintText(&cBmp, &r, cString, size, weight, color, justify, format, font);

	cBmp.paint(win, rect);

	return errCode;
}

ErrorCode waitForInput() {
	EventLoop eventLoop;

	while (!g_engine->shouldQuit() && !eventLoop.frame()) {
	}

	return ERR_NONE;
}

void CBagMasterWin::close() {
	Assert(IsValidObject(this));

	g_allowPaintFl = false;
}

void CBagMasterWin::restoreActiveMessages(CBagStorageDevManager *sdevManager) {
	Assert(sdevManager != nullptr);

	if (sdevManager != nullptr) {
		// Make sure the Message Log light will flash if user has
		// waiting messages.
		int n = sdevManager->GetNumStorageDevices();
		for (int i = 0; i < n; i++) {
			CBagStorageDev *pSDev = sdevManager->GetStorageDevice(i);
			if (pSDev != nullptr) {

				int m = pSDev->GetObjectCount();
				for (int j = 0; j < m; j++) {
					CBagObject *pObj = pSDev->GetObjectByPos(j);
					if (pObj != nullptr && pObj->IsMsgWaiting()) {
						pSDev->activateLocalObject(pObj);
					}
				}
			}
		}
	}
}

// Set the CIC var to either true or false so that our scripting
// code can tell whether or not to play certain movies (primarily flashbacks).
void setCICStatus(CBagStorageDev *sdev) {
	char szLocalBuff[256];
	CBofString sWorkStr(szLocalBuff, 256);

	// If the new game window is a CIC, then set the global var indicating
	// that this is the case.  Don't reset when we're zooming the PDA.
	if (sdev && sdev->GetName() != "BPDAZ_WLD") {
		sWorkStr = "IN_CIC";
		CBagVar *pCICVar = VAR_MANAGER->GetVariable(sWorkStr);
		if (pCICVar) {
			sWorkStr = sdev->IsCIC() ? "TRUE" : "FALSE";
			pCICVar->SetValue(sWorkStr);
		}
	}
}

bool getCICStatus() {
	char szLocalBuff[256];
	CBofString sWorkStr(szLocalBuff, 256);
	bool bRetVal = false;

	sWorkStr = "IN_CIC";
	CBagVar *pCICVar = VAR_MANAGER->GetVariable(sWorkStr);
	if (pCICVar) {
		bRetVal = (pCICVar->GetValue() == "TRUE");
	}

	return bRetVal;
}

} // namespace Bagel
