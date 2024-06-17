
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
static bool g_allowRestoreFl = false;

bool g_restoreObjectListFl = true;

#define NUM_MSG_STRINGS 3
static int g_string = 0;
static char g_stringArray[NUM_MSG_STRINGS][512];

// static initializations

bool CBagMasterWin::_objSaveFl = false;
StObj *CBagMasterWin::_objList = nullptr;
CBagCursor *CBagMasterWin::_cursorList[MAX_CURSORS];
int CBagMasterWin::_menuCount = 0;
int CBagMasterWin::_curCursor = 0;

//
//
// CBagMasterWin
//
CBagMasterWin::CBagMasterWin() {
	CBofRect screenRect(0, 0, 640 - 1, 480 - 1);

	const char *appName = "BAGEL Application";

	CBofApp *app = CBofApp::getApp();
	if (app != nullptr) {
		screenRect.setRect(0, 0, app->screenWidth() - 1, app->screenHeight() - 1);
		appName = app->getAppName();
	}

	screenRect.setRect(0, 0, 640 - 1, 480 - 1);
	_fadeIn = 0;
	_gameWindow = nullptr;

	_storageDeviceList = nullptr;
	_gameSDevList = nullptr;
	_variableList = nullptr;
	_diskId = 1;

	create(appName, &screenRect, nullptr);

	// Assume default system screen
	_sysScreen = "$SBARDIR\\GENERAL\\SYSTEM\\GAMBHALL.BMP";
	fixPathName(_sysScreen);

	// Load wait sound for when user hits the spacebar
	CBofString waitName("$SBARDIR\\GENERAL\\WAIT.WAV");
	fixPathName(waitName);

	_waitSound = new CBofSound(this, waitName, SOUND_MIX);
}

CBagMasterWin::~CBagMasterWin() {
	assert(isValidObject(this));

	delete _waitSound;
	_waitSound = nullptr;

	CBofApp *app = CBofApp::getApp();
	if (app != nullptr) {
		app->setPalette(nullptr);
	}

	// Delete any remaining cursors
	for (int i = 0; i < MAX_CURSORS; i++) {
		delete _cursorList[i];
		_cursorList[i] = nullptr;
	}

	delete _gameWindow;
	_gameWindow = nullptr;

	delete _storageDeviceList;
	_storageDeviceList = nullptr;

	delete _variableList;
	_variableList = nullptr;

	delete _gameSDevList;
	_gameSDevList = nullptr;

	// We can get rid of this buffer since the game is shutting down
	if (_objList != nullptr) {
		bofFree(_objList);
		_objList = nullptr;
	}
}

ErrorCode CBagMasterWin::showSystemDialog(bool bSaveBackground) {
	assert(isValidObject(this));
	if (g_engine->isDemo())
		return ERR_NONE;

	CBagStorageDevWnd *sdev = getCurrentStorageDev();

	if ((sdev == nullptr) || (sdev->getDeviceType() == SDEV_GAMEWIN) || (sdev->getDeviceType() == SDEV_ZOOMPDA)) {
		logInfo("Showing System Screen");

		// Use specified bitmap as this dialog's image
		CBofBitmap *dialogBmp = Bagel::loadBitmap(_sysScreen.getBuffer());

		CBagOptWindow optionDialog;
		optionDialog.setBackdrop(dialogBmp);

		const CBofRect dialogRect = optionDialog.getBackdrop()->getRect();

		if (!bSaveBackground) {
			optionDialog.setFlags(optionDialog.getFlags() & ~BOFDLG_SAVEBACKGND);
		}

		// Create the dialog box
		optionDialog.create("System Dialog", dialogRect.left, dialogRect.top, dialogRect.width(), dialogRect.height(), this);

		CBofWindow *lastWin = g_hackWindow;
		g_hackWindow = &optionDialog;

		g_pauseTimerFl = true;
		const int dialogReturnValue = optionDialog.doModal();
		g_pauseTimerFl = false;
		optionDialog.detach();

		g_hackWindow = lastWin;

		logInfo("Exiting System Screen");

		// User chose to Quit
		if (dialogReturnValue == 0) {
			close();
		}
	}

	return _errCode;
}

ErrorCode CBagMasterWin::showCreditsDialog(CBofWindow *win, bool bSaveBkg) {
	assert(isValidObject(this));

	logInfo("Showing Credits Screen");

	// Use specified bitmap as this dialog's image
	CBofBitmap *barBmp = Bagel::loadBitmap(buildSysDir("BARAREA.BMP"));

	CBagCreditsDialog creditsDialog;
	creditsDialog.setBackdrop(barBmp);

	const CBofRect dialogRect = creditsDialog.getBackdrop()->getRect();

	// Don't allow save of background?
	if (!bSaveBkg) {
		const int flags = creditsDialog.getFlags();

		creditsDialog.setFlags(flags & ~BOFDLG_SAVEBACKGND);
	}

	// Use CBagMasterWin if no parent specified
	if (win == nullptr) {
		win = this;
	}

	// Create the dialog box
	creditsDialog.create("Save Dialog", dialogRect.left, dialogRect.top, dialogRect.width(), dialogRect.height(), win);

	const bool saveTimerFl = g_pauseTimerFl;
	g_pauseTimerFl = true;
	creditsDialog.doModal();
	g_pauseTimerFl = saveTimerFl;

	logInfo("Exiting Credits Screen");

	return _errCode;
}

bool CBagMasterWin::showQuitDialog(CBofWindow *win, bool bSaveBackground) {
	assert(isValidObject(this));

	CBagStorageDevWnd *sdev = getCurrentStorageDev();
	bool quitFl = false;

	if ((sdev == nullptr) || (sdev->getDeviceType() == SDEV_GAMEWIN) || (sdev->getDeviceType() == SDEV_ZOOMPDA)) {
		logInfo("Showing Quit Screen");

		CBofSound::pauseSounds();

		if (win == nullptr) {
			win = this;
		}

		// Use specified bitmap as this dialog's image
		CBofBitmap *dialogBmp = Bagel::loadBitmap(_sysScreen.getBuffer());

		CBagQuitDialog quitDialog;
		quitDialog.setBackdrop(dialogBmp);

		const CBofRect dialogRect = quitDialog.getBackdrop()->getRect();

		if (!bSaveBackground) {
			quitDialog.setFlags(quitDialog.getFlags() & ~BOFDLG_SAVEBACKGND);
		}

		// Create the dialog box
		quitDialog.create("Quit Dialog", dialogRect.left, dialogRect.top, dialogRect.width(), dialogRect.height(), win);

		const bool saveTimerFl = g_pauseTimerFl;
		g_pauseTimerFl = true;
		const int dialogReturnValue = quitDialog.doModal();
		g_pauseTimerFl = saveTimerFl;

		switch (dialogReturnValue) {
		case SAVE_BTN:
			// Quit as as well. Saving already done within dialog itself
			quitFl = true;
			break;

		case QUIT_BTN:
			quitFl = true;
			break;

		case CANCEL_BTN:
			quitFl = false;
			break;

		default:
			break;
		}

		if (!quitFl) {
			CBofSound::resumeSounds();
		}

		logInfo("Exiting Quit Screen");
	}

	return quitFl;
}

ErrorCode CBagMasterWin::newGame() {
	assert(isValidObject(this));

	char workStr[256];
	char cInit[256];

	workStr[0] = '\0';
	cInit[0] = '\0';

	CBofString initWld(cInit, 256);

	CBagPanWindow::flushInputEvents();

	// Inits for a New Game
	_objSaveFl = false;

	CBagel *app = CBagel::getBagApp();
	// Find the starting .WLD file name
	if (app != nullptr) {
		app->getOption("Startup", "WLDFile", workStr, STARTWORLD, 255);

		initWld = workStr;
		fixPathName(initWld);
	}

	loadGlobalVars(GLOBALWORLD);
	loadFile(initWld, "", true);

	return _errCode;
}

ErrorCode CBagMasterWin::loadFile(const CBofString &wldName, const CBofString &startWldName, bool restartFl, bool setStartFl) {
	char localBuffer[256];
	localBuffer[0] = '\0';

	Common::strcpy_s(localBuffer, "$SBARDIR\\GENERAL\\SYSTEM\\LEGAL.BMP");
	CBofString wldFileName(localBuffer, 256);

	static bool paintedFl = false;

	// Make sure we get a new set of vildroid filter variables
	g_engine->g_getVilVarsFl = true;

	// Reset the Queued sound slot volumes back to default
	CBofSound::resetQVolumes();

	if (!paintedFl) {
		paintedFl = true;
		fixPathName(wldFileName);
		CBofRect cRect;
		cRect.left = (640 - 520) / 2;
		cRect.top = (480 - 240) / 2;
		cRect.right = cRect.left + 520 - 1;
		cRect.bottom = cRect.top + 240 - 1;

		paintBitmap(this, wldFileName.getBuffer(), &cRect);
	}

	wldFileName = wldName;

	// Reset unique ID for menus
	_menuCount = 0;

	// Keep track of what script we are in
	_wldScript = wldName;

	// This palette will be deleted so don't let anyone use it, until it is
	// replaced with a new one.
	CBofApp *app = CBofApp::getApp();
	if (app != nullptr) {
		app->setPalette(nullptr);
	}

	// Save all used objects (if going to another .WLD file)
	bool restoreFl = false;
	if ((_storageDeviceList != nullptr) && !restartFl) {

		if (!_objSaveFl) {

			_objSaveFl = true;

			// Only allocate the object list when we really need it...
			if (_objList == nullptr) {
				// Allocate a buffer filled with 0s
				_objList = (StObj *)bofCleanAlloc(MAX_OBJS * sizeof(StObj));
			}

			_storageDeviceList->saveObjList(_objList, MAX_OBJS); // xxx

			// Save our SDEV location, so we can restore it from Kerpupu
			saveSDevStack();

		} else {
			restoreFl = true;
		}
	}

	if (_variableList != nullptr) {
		_variableList->releaseVariables(false);
	}

	delete _gameSDevList;
	_gameSDevList = nullptr;

	delete _gameWindow;
	_gameWindow = nullptr;

	delete _storageDeviceList;
	_storageDeviceList = nullptr;

	// Unload all current cursors
	for (int i = 0; i < MAX_CURSORS; i++) {
		delete _cursorList[i];
		_cursorList[i] = nullptr;
	}

	CBagMenu::setUniversalObjectList(nullptr);

	_storageDeviceList = new CBagStorageDevManager();
	if (!_variableList) {
		_variableList = new CBagVarManager();
	}
	_gameSDevList = new CBofList<CBagStorageDev *>;

	fixPathName(wldFileName);

	timerStart();

	if (fileExists(wldFileName) && (fileLength(wldFileName) > 0)) {
		// Force buffer to be big enough so that the entire script
		// is pre-loaded
		const int length = fileLength(wldFileName);
		char *fileBuf = (char *)bofAlloc(length);
		CBagIfstream fpInput(fileBuf, length);

		CBofFile file;
		file.open(wldFileName);
		file.read(fileBuf, length);
		file.close();

		loadFileFromStream(fpInput, startWldName);

		bofFree(fileBuf);

		// Possibly need to switch CDs
		CBagel *bagApp = CBagel::getBagApp();
		if (bagApp != nullptr) {
			_errCode = bagApp->verifyCDInDrive(_diskId, _cdChangeAudio.getBuffer());
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
		CBagWield::setWieldCursor(-1);

		if (g_restoreObjectListFl) {

			if (restoreFl && _objSaveFl) {
				assert(_storageDeviceList != nullptr);
				if (_storageDeviceList != nullptr) {
					// Use a preallocated buffer, trash it when we're done.
					assert(_objList != nullptr);
					_storageDeviceList->restoreObjList(_objList, MAX_OBJS);

					// All done with this list, can trash it now
					bofFree(_objList);
					_objList = nullptr;
				}
				_objSaveFl = false;
			}
		}
		g_restoreObjectListFl = true;

		// If a start wld is passed in then use it
		if (!startWldName.isEmpty()) {
			_startWld = startWldName;
		}

		if (setStartFl && !_startWld.isEmpty()) {
			setStorageDev(_startWld);
		}

		restoreActiveMessages(_storageDeviceList);

	} else {
		reportError(ERR_FFIND, "Could not find World Script: %s", wldFileName.getBuffer());
	}
	logInfo(buildString("Time to Load %s, %u ms", wldFileName.getBuffer(), timerStop()));

	return _errCode;
}

void CBagMasterWin::saveSDevStack() {
	assert(isValidObject(this));

	// Save our SDEV location, so we can restore it from Kerpupu
	char locStack[MAX_CLOSEUP_DEPTH][MAX_VAR_VALUE];
	char tempBuf[256];

	memset(&locStack[0][0], 0, sizeof(char) * MAX_CLOSEUP_DEPTH * MAX_VAR_VALUE);
	tempBuf[0] = '\0';
	CBagStorageDevWnd *sdevWin = getCurrentStorageDev();
	if (sdevWin != nullptr) {
		int i = 0;

		CBofString curStr = sdevWin->getName();
		if (!curStr.isEmpty()) {
			Common::strcpy_s(locStack[i], curStr.getBuffer());
			curStr = sdevWin->getPrevSDev();
			i++;
		}

		while ((i < MAX_CLOSEUP_DEPTH) && !curStr.isEmpty()) {
			sdevWin = (CBagStorageDevWnd *)_storageDeviceList->getStorageDevice(curStr);
			if (sdevWin != nullptr) {

				Common::strcpy_s(locStack[i], curStr.getBuffer());

				i++;
				curStr = sdevWin->getPrevSDev();
			} else {
				break;
			}
		}
		i--;
		for (int j = i; j >= 0; j--) {

			if (locStack[j][0] != '\0') {
				Common::strcat_s(tempBuf, locStack[j]);
				if (j != 0) {
					Common::strcat_s(tempBuf, "~~");
				}
			}
		}

		// Variables cannot exceed MAX_VAR_VALUE characters in length (for Save/Restore)
		assert(strlen(tempBuf) < MAX_VAR_VALUE);

		// Store our current sdev location stack in a global variable.
		CBagVar *var = g_VarManager->getVariable("$LASTWORLD");
		if (var != nullptr) {
			curStr = tempBuf;
			var->setValue(curStr);
		}
	}
}

ErrorCode CBagMasterWin::loadGlobalVars(const CBofString &wldName) {
	assert(isValidObject(this));

	char localBuff[256];
	localBuff[0] = '\0';
	CBofString wldFileName(localBuff, 256);
	wldFileName = wldName;

	delete _variableList;
	_variableList = new CBagVarManager();

	fixPathName(wldFileName);

	if (fileExists(wldFileName) && (fileLength(wldFileName) > 0)) {
		// Force buffer to be big enough so that the entire script
		// is pre-loaded
		const int length = fileLength(wldFileName);
		char *buffer = (char *)bofAlloc(length);
		CBagIfstream fpInput(buffer, length);

		CBofFile file;
		file.open(wldFileName);
		file.read(buffer, length);
		file.close();

		while (!fpInput.eof()) {
			fpInput.eatWhite();

			if (!fpInput.eatWhite()) {
				break;
			}

			KEYWORDS keyword;
			getKeywordFromStream(fpInput, keyword);

			switch (keyword) {
			case VARIABLE: {
				CBagVar *var = new CBagVar;
				fpInput.eatWhite();
				var->setInfo(fpInput);
				var->setGlobal();
				break;
			}

			case REMARK: {
				char dummyStr[256];
				fpInput.getCh(dummyStr, 255);
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
				parseAlertBox(fpInput, "Syntax Error:  Unexpected Type in Global Var Wld:", __FILE__, __LINE__);
				break;
			}
			}
		}

		bofFree(buffer);
	}

	return _errCode;
}

ErrorCode CBagMasterWin::loadFileFromStream(CBagIfstream &input, const CBofString &wldName) {
	char localStr[256];
	localStr[0] = 0;
	bool isWieldCursorFl = false;

	memset(localStr, 0, 256);
	CBofString workStr(localStr, 256);

	_startWld = wldName;

	while (!input.eof()) {
		input.eatWhite();
		int filter = 0;
		CBofRect curRect;
		curRect.right = curRect.left - 1;
		curRect.bottom = curRect.top - 1;

		KEYWORDS keyword;

		if (!input.eatWhite()) {
			break;
		}

		getKeywordFromStream(input, keyword);

		switch (keyword) {

		case STORAGEDEV: {
			char nameBuff[256];
			char typeBuff[256];
			nameBuff[0] = 0;
			typeBuff[0] = 0;

			memset(nameBuff, 0, 256);
			memset(typeBuff, 0, 256);
			CBofString nameStr(nameBuff, 256);
			CBofString typeStr(typeBuff, 256);

			input.eatWhite();
			getAlphaNumFromStream(input, nameStr);

			input.eatWhite();

			int fadeId = 0;

			while (input.peek() != '{') {
				getAlphaNumFromStream(input, workStr);
				input.eatWhite();
				if (!workStr.find("AS")) {
					input.eatWhite();
					getAlphaNumFromStream(input, typeStr);
				} else if (!workStr.find("RECT")) {
					input.eatWhite();
					getRectFromStream(input, curRect);
				} else if (!workStr.find("FILTER")) {
					input.eatWhite();
					getIntFromStream(input, filter);
				} else if (!workStr.find("FADE")) { // Note that this should usually be set in the link
					input.eatWhite();
					getIntFromStream(input, fadeId);
				} else {
					// There is an error here
					logError(buildString("FAILED on argument of storage device %s : %s", nameStr.getBuffer(), typeStr.getBuffer()));
					return ERR_UNKNOWN;
				}

				input.eatWhite();
			}
			CBagStorageDev *sdev = onNewStorageDev(typeStr);
			if (!sdev) {
				logError(buildString("FAILED on open of storage device %s : %s", nameStr.getBuffer(), typeStr.getBuffer()));
				return ERR_UNKNOWN;
			}

			// Default DISK ID for this storage device is the same
			// as the ID specified for this .WLD script file.
			sdev->setDiskID(_diskId);

			if (curRect.width() && curRect.height())
				sdev->setRect(curRect);

			sdev->loadFileFromStream(input, nameStr, false);
			if (filter) {
				sdev->setFilterId((uint16)filter);

				// Set the filter on the window.
				onNewFilter(sdev, typeStr);
			}

			if (fadeId != 0) {
				sdev->setFadeId((uint16)fadeId);
			}

			_gameSDevList->addToTail(sdev);
			break;
		}

		case START_WLD: {
			char strBuf[256];
			strBuf[0] = '\0';

			CBofString str(strBuf, 256);

			input.eatWhite();
			if (input.peek() == '=') {
				input.getCh();
				input.eatWhite();
				getAlphaNumFromStream(input, str);

				// Only use the start wld if not specified elsewhere
				_startWld = str;
				logInfo(buildString("START_WLD set to %s", _startWld.getBuffer()));
			}
			break;
		}

		case WIELDCURSOR:
			isWieldCursorFl = true;
			// fallthrough
		case CURSOR: {
			char strBuf[256];
			strBuf[0] = 0;

			CBofString str(strBuf, 256);
			int id;

			input.eatWhite();
			getIntFromStream(input, id);
			input.eatWhite();
			if (input.peek() == '=') {
				int x, y;
				input.getCh();
				input.eatWhite();

				getIntFromStream(input, x);
				input.eatWhite();

				getIntFromStream(input, y);
				input.eatWhite();

				getAlphaNumFromStream(input, str);
				fixPathName(str);

				// Specify if we have a shared palette or not, look for
				// the USESHAREDPAL token after the full cursor specification
				bool bUseShared = false;

				input.eatWhite();
				if (input.peek() == '=') {
					char sharedPalTokenBuf[256];
					CBofString sharedPalToken(sharedPalTokenBuf, 256);

					input.getCh();
					input.eatWhite();

					// Check for shared pal token, if there, then create our cursor
					// with the shared palette bit set
					getAlphaNumFromStream(input, sharedPalToken);
					if (sharedPalToken.find("USESHAREDPAL") != -1) {
						bUseShared = true;
					}
				}

				CBagCursor *cursor = new CBagCursor(str, bUseShared);
				cursor->setHotspot(x, y);

				assert(id >= 0 && id < MAX_CURSORS);

				// Delete any previous cursor
				delete _cursorList[id];
				_cursorList[id] = cursor;

				// Set the wielded cursor status (needed for
				// a load time optimization)
				cursor->setWieldCursor(isWieldCursorFl);

			} else {
				reportError(ERR_UNKNOWN, "Bad cursor syntax");
			}
			break;
		}

		case PDASTATE: {
			char pdaStateBuf[256];
			pdaStateBuf[0] = '\0';
			CBofString pdaState(pdaStateBuf, 256);
			input.eatWhite();
			if (input.peek() == '=') {
				input.getCh();
				input.eatWhite();

				getAlphaNumFromStream(input, pdaState);

				if (pdaState.find("MAP") != -1) {
					SBBasePda::setPdaMode(PDA_MAP_MODE);
				} else if (pdaState.find("INV") != -1) {
					SBBasePda::setPdaMode(PDA_INV_MODE);
				} else if (pdaState.find("LOG") != -1) {
					SBBasePda::setPdaMode(PDA_LOG_MODE);
				}
				logInfo(buildString("PDASTATE = %s", pdaStateBuf));
			}
			break;
		}

		// Implement shared palettes
		case SHAREDPAL: {
			char bmpFileNameBuf[256];
			bmpFileNameBuf[0] = '\0';
			CBofString bmpFileName(bmpFileNameBuf, 256);
			input.eatWhite();
			if (input.peek() == '=') {
				input.getCh();
				input.eatWhite();

				getAlphaNumFromStream(input, bmpFileName);
				fixPathName(bmpFileName);

				// Read the palette in and keep it hanging around for later use
				CBofPalette::setSharedPalette(bmpFileName);

				logInfo(buildString("SHAREDPAL = %s", bmpFileName.getBuffer()));
			}
			break;
		}

		case SYSSCREEN: {

			input.eatWhite();
			if (input.peek() == '=') {
				input.getCh();
				input.eatWhite();

				getAlphaNumFromStream(input, _sysScreen);
				fixPathName(_sysScreen);

				logInfo(buildString("SYSSCREEN = %s", _sysScreen.getBuffer()));
			}
			break;
		}

		// What audio file should play for this disk swap
		case DISKAUDIO: {
			char diskIdBuf[256];
			diskIdBuf[0] = '\0';
			CBofString diskId(diskIdBuf, 256);

			input.eatWhite();
			if (input.peek() == '=') {
				input.getCh();
				input.eatWhite();

				getAlphaNumFromStream(input, _cdChangeAudio);
				fixPathName(_cdChangeAudio);

				logInfo(buildString("DISKAUDIO = %s", _cdChangeAudio.getBuffer()));
			}
			break;
		}

		case DISKID: {
			input.eatWhite();
			if (input.peek() == '=') {
				input.getCh();
				input.eatWhite();
				int n;

				getIntFromStream(input, n);
				_diskId = (uint16)n;

				logInfo(buildString("DISKID = %d", _diskId));

			} else {
			}
			break;
		}

		case VARIABLE: {
			CBagVar *var = new CBagVar;
			input.eatWhite();
			var->setInfo(input);
			break;
		}

		case REMARK: {
			char s[255];
			input.getCh(s, 255);
			break;
		}

		default: {
			parseAlertBox(input, "Syntax Error:", __FILE__, __LINE__);
			break;
		}
		}

	} // While not eof

	// Add everything to the window
	return _errCode;
}

ErrorCode CBagMasterWin::setStorageDev(const CBofString &wldName, bool entry) {
	assert(CBofObject::isValidObject(&wldName));

	char localBuff[256];
	localBuff[0] = '\0';
	CBofString extension(localBuff, 256);

	extension = wldName.right(4);
	if (_gameWindow)
		_gameWindow->setOnUpdate(false);

	if (!extension.find(".wld") || !extension.find(".WLD") || (wldName.find(".WLD~~") > 0) || (wldName.find(".wld~~") > 0)) {

		// This is to stop it from going out of scope before
		// the message is received.
		assert(g_string >= 0 && g_string < NUM_MSG_STRINGS);
		assert(wldName.getLength() < 512);

		strncpy(g_stringArray[g_string], wldName, 511);

		postUserMessage(WM_ENTER_NEW_WLD, (uint32)g_string);

		if (++g_string >= NUM_MSG_STRINGS) {
			g_string = 0;
		}

	} else if (entry) {

		gotoNewWindow(&wldName);

	} else {

		// This is to stop the string from going out of scope before
		// the message is received.
		assert(g_string >= 0 && g_string < NUM_MSG_STRINGS);
		assert(wldName.getLength() < 512);
		Common::strcpy_s(g_stringArray[g_string], wldName);

		postUserMessage(WM_EXIT_CLOSE_UP_WINDOW, (uint32)g_string);

		if (++g_string >= NUM_MSG_STRINGS) {
			g_string = 0;
		}
	}

	return ERR_NONE;
}

ErrorCode CBagMasterWin::onHelp(const CBofString &helpFile, bool /*bSaveBkg*/, CBofWindow *parent) {
	assert(isValidObject(this));

	if (g_engine->isDemo())
		return ERR_NONE;

	if (!helpFile.isEmpty()) {
		char localBuffer[256];
		localBuffer[0] = '\0';
		CBofString fileName(localBuffer, 256);

		fileName = helpFile;
		fixPathName(fileName);

		// use specified bitmap as this dialog's image
		char backGroundBuffer[256];
		backGroundBuffer[0] = '\0';
		CBofString backGround(backGroundBuffer, 256);

		backGround = buildString("$SBARDIR%sGENERAL%sRULES%sHELPSCRN.BMP", PATH_DELIMETER, PATH_DELIMETER, PATH_DELIMETER);
		fixPathName(backGround);

		CBofBitmap *bmp = Bagel::loadBitmap(backGround);
		CBagHelp help;
		help.setBackdrop(bmp);

		const CBofRect backRect = help.getBackdrop()->getRect();

		if (parent == nullptr)
			parent = this;

		// create the dialog box
		help.create("HelpDialog", backRect.left, backRect.top, backRect.width(), backRect.height(), parent);

		help.SetHelpFile(fileName.getBuffer());
		help.doModal();
		help.detach();
	}

	return _errCode;
}

bool g_waitOKFl = false;

void CBagMasterWin::onKeyHit(uint32 keyCode, uint32 repCount) {
	assert(isValidObject(this));

	int volume;

	switch (keyCode) {
	// Dynamic Midi Volume increase
	case BKEY_ALT_UP:
		volume = getMidiVolume();
		if (volume < 12) {
			volume++;
			setMidiVolume(volume);
		}

		break;

	// Dynamic Midi Volume decrease
	case BKEY_ALT_DOWN:
		volume = getMidiVolume();
		if (volume > 0) {
			volume--;
			setMidiVolume(volume);
		}
		break;

	case BKEY_SPACE:
		if (g_waitOKFl) {
			g_waitOKFl = false;

			// Play the tick-tock sound
			if (_waitSound != nullptr) {
				_waitSound->play();
			}

			g_VarManager->incrementTimers();

			// Prefilter this guy, could cause something to change in the
			// pan or the PDA or a closeup.
			_gameWindow->setPreFilterPan(true);

			_gameWindow->attachActiveObjects();
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
			onHelp(_gameWindow->getHelpFilename(), false);
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
	assert(isValidObject(this));

	if (_gameWindow)
		_gameWindow->onClose();

	g_engine->quitGame();
}

ErrorCode CBagMasterWin::gotoNewWindow(const CBofString *str) {
	assert(isValidObject(this));
	assert(str != nullptr);
	assert(CBofObject::isValidObject(str));

	CBagStorageDev *sdev;

	char workStringBuffer[256];
	char sprevSDevBuffer[256];
	char curSDevBuffer[256];

	workStringBuffer[0] = '\0';
	sprevSDevBuffer[0] = '\0';
	curSDevBuffer[0] = '\0';

	CBofString workString(workStringBuffer, 256);
	CBofString prevSDevString(sprevSDevBuffer, 256);
	CBofString currSDevString(curSDevBuffer, 256);

	int n = str->find("~~");
	workString = *str;
	bool prevFl = false;

	while (n > 0) {
		prevSDevString = workString.left(n);
		workString = workString.mid(n + 2);
		n = workString.find("~~");
		if (n > 0) {
			currSDevString = workString.left(n);
			sdev = _storageDeviceList->getStorageDevice(currSDevString);
			if (sdev != nullptr) {
				sdev->setPrevSDev(prevSDevString);
				prevFl = true;
			}
		} else {
			currSDevString = workString;
			sdev = _storageDeviceList->getStorageDevice(currSDevString);
			if (sdev != nullptr) {
				sdev->setPrevSDev(prevSDevString);
				prevFl = true;
			}
		}
	}

	sdev = _storageDeviceList->getStorageDevice(workString);
	if (sdev != nullptr) {

		logInfo(buildString("Switching to SDEV: %s", workString.getBuffer()));

		if (_gameWindow) {
			_gameWindow->detach();

			//  If the new storage device is equal to the last windows previous
			//  lets not go in a circle
			//  If the current game window did not have a previous win
			if ((_gameWindow->getPrevSDev().isEmpty()) || (_gameWindow->getPrevSDev() != sdev->getName())) {
				if (!prevFl) {
					sdev->setPrevSDev(_gameWindow->getName());
				}
			}
		}

		// Don't allow recursion
		if (!sdev->getPrevSDev().isEmpty() && sdev->getPrevSDev().compareNoCase(sdev->getName()) == 0) {
			sdev->setPrevSDev("");
		}

		_gameWindow = (CBagStorageDevWnd *)sdev;
		setCICStatus(sdev);

		const uint16 oldFadeId = sdev->getFadeId();

		if (_fadeIn != 0)
			sdev->setFadeId((uint16)_fadeIn);

		// Reset paints
		g_allowPaintFl = true;

		sdev->attach();

		sdev->setFadeId(oldFadeId);
		_fadeIn = 0;
	}

	return _errCode;
}

bool CBagMasterWin::showRestartDialog(CBofWindow *win, bool saveBkgFl) {
	assert(isValidObject(this));

	if (g_engine->isDemo())
		return false;

	CBagStorageDevWnd *sdev = getCurrentStorageDev();
	if ((sdev == nullptr) || (sdev->getDeviceType() == SDEV_GAMEWIN) || (sdev->getDeviceType() == SDEV_ZOOMPDA)) {

		logInfo("Showing Restart Screen");

		if (win == nullptr) {
			win = this;
		}

		CBagRestartDialog restartDialog(_sysScreen.getBuffer(), win);

		CBofWindow *pLastWin = g_hackWindow;
		g_hackWindow = &restartDialog;

		// Don't allow save of background
		if (!saveBkgFl) {
			const int lFlags = restartDialog.getFlags();
			restartDialog.setFlags(lFlags & ~BOFDLG_SAVEBACKGND);
		}

		const bool saveTimerFl = g_pauseTimerFl;
		g_pauseTimerFl = true;
		const int dialogReturn = restartDialog.doModal();
		g_pauseTimerFl = saveTimerFl;

		g_hackWindow = pLastWin;

		logInfo("Exiting Restart Screen");

		return dialogReturn == RESTART_BTN;
	}

	return false;
}

void CBagMasterWin::onUserMessage(uint32 message, uint32 param) {
	assert(isValidObject(this));

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
		CBofSound::stopSounds();

		// Kill any waiting PDA messages that are queued up...
		CBagPDA::removeFromMovieQueue(nullptr);

		CBofBitmap bmp(width(), height(), CBagel::getApp()->getPalette());
		bmp.fillRect(nullptr, COLOR_BLACK);
		bmp.fadeLines(this);

		char buffer[MAX_FNAME];

		Common::strcpy_s(buffer, buildSysDir("DIED2.BMP"));
		if (param == 2) {
			Common::strcpy_s(buffer, buildSysDir("START.BMP"));
		}

		CBagStartDialog dialog(buffer, this);

		CBofWindow *lastWin = g_hackWindow;
		g_hackWindow = &dialog;

		g_allowRestoreFl = true;
		int dialogRet = dialog.doModal();
		g_allowRestoreFl = false;

		g_hackWindow = lastWin;

		// Hide that dialog
		bmp.paint(this, 0, 0);

		switch (dialogRet) {

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
		char localBuff[256];
		localBuff[0] = '\0';
		CBofString wldScript(localBuff, 256);

		// User info is an index into an array of temporary string buffers
		assert(param < NUM_MSG_STRINGS);
		wldScript = g_stringArray[(int)param];

		char startWldBuf[256];
		startWldBuf[0] = '\0';
		CBofString startWld(startWldBuf, 256);

		int n = wldScript.find("~~");
		if (n > 0) {
			startWld = wldScript.mid(n + 2);
			wldScript = wldScript.left(n);
		}
		loadFile(wldScript, startWld);
		break;
	}

	case WM_ENTER_PAN_WINDOW:
	case WM_ENTER_CLOSE_UP_WINDOW:
		// Should never be called
		assert(false);
		break;

	case WM_EXIT_CLOSE_UP_WINDOW: {
		CBofString deviceStr;

		// User info is an index into an array of temporary string buffers
		assert(param < NUM_MSG_STRINGS);
		deviceStr = g_stringArray[(int)param];

		CBagStorageDev *sdev = _storageDeviceList->getStorageDevice(deviceStr);

		if (sdev) {
			if (_gameWindow) {
				_gameWindow->detach();
			}

			sdev->attach();
			sdev->setPreFilterPan(true);

			_gameWindow = (CBagStorageDevWnd *)sdev;

			// Reset the CIC var
			setCICStatus(sdev);
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
	assert(cursorId >= 0 && cursorId < MAX_CURSORS);

	if (_cursorList[cursorId] != nullptr) {
		_cursorList[cursorId]->setCurrent();
		_curCursor = cursorId;
	}
}

void CBagMasterWin::fillSaveBuffer(StBagelSave *saveBuf) {
	assert(isValidObject(this));
	assert(saveBuf != nullptr);

	//
	// Fill the save game buffer with all the info we need to restore this game
	//

	// 1st, wipe it
	memset(saveBuf, 0, sizeof(StBagelSave));

	CBagel *app = CBagel::getBagApp();
	if (app != nullptr) {
		CBagMasterWin *masterWin = app->getMasterWnd();
		if (masterWin != nullptr) {
			// Save Global variables
			CBagVarManager *varManager = getVariableManager();
			if (varManager != nullptr) {

				// Walk variable list and save each global variable
				int j = 0;
				int n = varManager->getNumVars();
				for (int i = 0; i < n; i++) {
					CBagVar *curVar = varManager->getVariable(i);
					if (curVar != nullptr) {
						// Need to save local variables in flashbacks.
						// Let me know if this breaks anything.
						//
						// If it's a global variable, then we need to store it

						if (!curVar->getName().isEmpty()) {
							assert(strlen(curVar->getName()) < MAX_VAR_NAME);
							Common::strcpy_s(saveBuf->_stVarList[j]._szName, curVar->getName());
						}

						if (!curVar->getValue().isEmpty()) {
							assert(strlen(curVar->getValue()) < MAX_VAR_VALUE);
							Common::strcpy_s(saveBuf->_stVarList[j]._szValue, curVar->getValue());
						}

						saveBuf->_stVarList[j]._nType = (uint16)curVar->getType();
						saveBuf->_stVarList[j]._bGlobal = (byte)curVar->isGlobal();
						saveBuf->_stVarList[j]._bConstant = (byte)curVar->isConstant();
						saveBuf->_stVarList[j]._bReference = (byte)curVar->isReference();
						saveBuf->_stVarList[j]._bTimer = (byte)curVar->isTimer();
						saveBuf->_stVarList[j]._bRandom = (byte)curVar->isRandom();
						saveBuf->_stVarList[j]._bNumeric = (byte)curVar->isNumeric();
						saveBuf->_stVarList[j]._bAttached = (byte)curVar->isAttached();
						saveBuf->_stVarList[j]._bUsed = 1;
						j++;

						// Can't exceed MAX_VARS
						assert(j < MAX_VARS);
						//}
					}
				}
			}

			// Remember current script file
			strncpy(saveBuf->_szScript, getWldScript().getBuffer(), MAX_FNAME - 1);

			CBagStorageDevWnd *sdevWin = getCurrentStorageDev();
			if (sdevWin != nullptr) {
				char stringBuffer[256];
				stringBuffer[0] = 0;
				CBofString curString(stringBuffer, 256);

				saveBuf->_nLocType = sdevWin->getDeviceType();

				// Remember the pan's position
				if (saveBuf->_nLocType == SDEV_GAMEWIN) {
					CBagPanWindow *panWin = (CBagPanWindow *)sdevWin;
					const CBofRect cPos = panWin->getViewPort();

					saveBuf->_nLocX = (uint16)cPos.left;
					saveBuf->_nLocY = (uint16)cPos.top;
				}

				CBagStorageDevManager *sdevManager = getStorageDevManager();
				if (sdevManager != nullptr) {
					sdevManager->saveObjList(&saveBuf->_stObjList[0], MAX_OBJS);
					if (isObjSave()) {

						assert(_objList != nullptr);

						memcpy(&saveBuf->_stObjListEx[0], _objList, sizeof(StObj) * MAX_OBJS);
						saveBuf->_bUseEx = 1;
					}

					// Save current storage device location (stack)
					int i = 0;
					curString = sdevWin->getName();
					while ((i < MAX_CLOSEUP_DEPTH) && !curString.isEmpty()) {
						sdevWin = (CBagStorageDevWnd *)sdevManager->getStorageDevice(curString);
						Common::strcpy_s(saveBuf->_szLocStack[i], curString.getBuffer());
						i++;
						curString.free();
						if (sdevWin != nullptr) {
							curString = sdevWin->getPrevSDev();
						}
					}
				}
			}
		}
	}
}

bool CBagMasterWin::showSaveDialog(CBofWindow *win, bool bSaveBkg) {
	assert(isValidObject(this));

	if (g_engine->isDemo())
		return false;

	if (!g_engine->_useOriginalSaveLoad) {
		return g_engine->saveGameDialog();
	}

	bool savedFl = false;
	CBagStorageDevWnd *sdev = getCurrentStorageDev();
	if ((sdev == nullptr) || (sdev->getDeviceType() == SDEV_GAMEWIN) || (sdev->getDeviceType() == SDEV_ZOOMPDA)) {

		logInfo("Showing Save Screen");
		CBofSound::pauseSounds();
		StBagelSave *saveBuf = (StBagelSave *)bofAlloc(sizeof(StBagelSave));

		fillSaveBuffer(saveBuf);
		CBagSaveDialog saveDialog;
		saveDialog.setSaveGameBuffer((byte *)saveBuf, sizeof(StBagelSave));

		// Use specified bitmap as this dialog's image
		CBofBitmap *bmp = Bagel::loadBitmap(_sysScreen.getBuffer());

		saveDialog.setBackdrop(bmp);

		const CBofRect backRect = saveDialog.getBackdrop()->getRect();

		// Don't allow save of background
		if (!bSaveBkg) {
			const int dlgFlags = saveDialog.getFlags();
			saveDialog.setFlags(dlgFlags & ~BOFDLG_SAVEBACKGND);
		}

		// Create the dialog box
		saveDialog.create("Save Dialog", backRect.left, backRect.top, backRect.width(), backRect.height(), win);

		const bool saveTimerFl = g_pauseTimerFl;
		g_pauseTimerFl = true;
		const int btnId = saveDialog.doModal();
		g_pauseTimerFl = saveTimerFl;

		savedFl = (btnId == SAVE_BTN);

		saveDialog.detach();

		bofFree(saveBuf);

		CBofSound::resumeSounds();

		logInfo("Exiting Save Screen");
	}

	return savedFl;
}

void CBagMasterWin::doRestore(StBagelSave *saveBuf) {
	assert(isValidObject(this));
	assert(saveBuf != nullptr);

	char scriptBuf[256];
	scriptBuf[0] = '\0';
	CBofString script(scriptBuf, 256);

	script = saveBuf->_szScript;

	char workBuf[60], closeUpBuf[256];
	char stringBuf[256];
	stringBuf[0] = 0;
	CBofString closeUpString(stringBuf, 256);
	int i;

	// Rebuild the stack of locations (Could be 3 closeups deep)
	closeUpBuf[0] = '\0';
	for (i = MAX_CLOSEUP_DEPTH - 1; i >= 0; i--) {
		if (saveBuf->_szLocStack[i][0] != '\0') {
			Common::sprintf_s(workBuf, "%s~~", saveBuf->_szLocStack[i]);
			Common::strcat_s(closeUpBuf, workBuf);
		}
	}
	int n = strlen(closeUpBuf);
	if (n > 2 && closeUpBuf[n - 1] == '~') {
		closeUpBuf[n - 1] = '\0';
		closeUpBuf[n - 2] = '\0';
	}

	// Designate closeups
	if (saveBuf->_nLocType == SDEV_CLOSEP) {
		Common::strcat_s(closeUpBuf, " AS CLOSEUP");
	}

	closeUpString = closeUpBuf;

	// Don't allow a local restore
	setSaveObjs(false);

	// Make sure that all global variables are loaded and available
	loadGlobalVars(GLOBALWORLD);

	// Tell BAGEL to start over with this script
	loadFile(script, closeUpString, false, false);

	if (!errorOccurred()) {

		// Restore all variables
		//
		CBagVarManager *varManager = getVariableManager();
		if (varManager != nullptr) {

			// Reset the Global Vars with these new settings
			for (i = 0; i < MAX_VARS; i++) {
				// If this entry is actually used to store Var info
				if (saveBuf->_stVarList[i]._bUsed) {
					// Find the Global Var (already in memory)
					CBagVar *var = varManager->getVariable(saveBuf->_stVarList[i]._szName);
					if (var != nullptr) {
						var->setValue(saveBuf->_stVarList[i]._szValue);

					} else {
						logError(buildString("Global Variable NOT found: %s", saveBuf->_stVarList[i]._szName));
					}
				}
			}
		}

		CBagStorageDevManager *sdevManager = getStorageDevManager();

		if (sdevManager != nullptr) {
			// Restore any extra obj list info (for .WLD swapping)
			if (_objList == nullptr) {
				// Allocate a buffer filled with 0s
				_objList = (StObj *)bofCleanAlloc(MAX_OBJS * sizeof(StObj));
			}

			memcpy(getObjList(), &saveBuf->_stObjListEx[0], sizeof(StObj) * MAX_OBJS);

			if (saveBuf->_bUseEx) {
				setSaveObjs(true);
			} else {
				setSaveObjs(false);
			}

			sdevManager->restoreObjList(&saveBuf->_stObjList[0], MAX_OBJS);
		}

		// If it's a Panorama, the set it's View position.
		if (saveBuf->_nLocType == SDEV_GAMEWIN) {
			g_engine->g_cInitLoc.x = saveBuf->_nLocX;
			g_engine->g_cInitLoc.y = saveBuf->_nLocY;
			g_engine->g_bUseInitLoc = true;
		}

		// Now set the start storage device and let the game start
		if (!_startWld.isEmpty()) {
			setStorageDev(_startWld);
		}

		restoreActiveMessages(sdevManager);
	}
}

bool CBagMasterWin::showRestoreDialog(CBofWindow *win, bool bSaveBkg) {
	assert(isValidObject(this));
	if (g_engine->isDemo())
		return false;

	if (!g_engine->_useOriginalSaveLoad) {
		return g_engine->loadGameDialog();
	}

	bool restoredFl = false;
	CBagStorageDevWnd *sdev;

	if (g_allowRestoreFl || ((sdev = getCurrentStorageDev()) == nullptr) || (sdev->getDeviceType() == SDEV_GAMEWIN) || (sdev->getDeviceType() == SDEV_ZOOMPDA)) {

		logInfo("Showing Restore Screen");

		CBofSound::pauseSounds();

		CBagRestoreDialog restoreDialog;

		// Use specified bitmap as this dialog's image
		CBofBitmap *pBmp = Bagel::loadBitmap(_sysScreen.getBuffer());

		restoreDialog.setBackdrop(pBmp);

		const CBofRect backRect = restoreDialog.getBackdrop()->getRect();

		// Don't allow save of background
		if (!bSaveBkg) {
			const int flags = restoreDialog.getFlags();
			restoreDialog.setFlags(flags & ~BOFDLG_SAVEBACKGND);
		}

		// Create the dialog box
		restoreDialog.create("Restore Dialog", backRect.left, backRect.top, backRect.width(), backRect.height(), win);

		CBofWindow *lastWin = g_hackWindow;
		g_hackWindow = &restoreDialog;

		const bool saveTimerFl = g_pauseTimerFl;
		g_pauseTimerFl = true;
		restoreDialog.doModal();
		g_pauseTimerFl = saveTimerFl;

		restoreDialog.detach();

		restoredFl = (!restoreDialog.errorOccurred() && restoreDialog.restored());
		restoreDialog.destroy();

		g_hackWindow = lastWin;

		CBofSound::resumeSounds();

		logInfo("Exiting Restore Screen");
	}

	return restoredFl;
}

// User options
#define DEFAULT_CORRECTION 2

bool CBagMasterWin::getFlyThru() {
	bool flyThrusOnFl = true;
	CBagel *app = CBagel::getBagApp();
	if (app != nullptr) {
		app->getOption(USER_OPTIONS, "FlyThroughs", &flyThrusOnFl, true);
	}

	return flyThrusOnFl;
}

int CBagMasterWin::getMidiVolume() {
	int midiVol = VOLUME_INDEX_DEFAULT;
	CBagel *app = CBagel::getBagApp();
	if (app != nullptr) {
		app->getOption(USER_OPTIONS, "MidiVolume", &midiVol, VOLUME_INDEX_DEFAULT);
		if (midiVol < VOLUME_INDEX_MIN || midiVol > VOLUME_INDEX_MAX) {
			midiVol = VOLUME_INDEX_DEFAULT;
		}
	}

	return midiVol;
}

void CBagMasterWin::setMidiVolume(int vol) {
	assert(vol >= VOLUME_INDEX_MIN && vol <= VOLUME_INDEX_MAX);
	CBagel *app = CBagel::getBagApp();

	if (app != nullptr) {
		app->setOption(USER_OPTIONS, "MidiVolume", vol);
	}

	// We will let the sound subsystem do our volume control...
	CBofSound::setVolume(vol, getWaveVolume());
}

int CBagMasterWin::getWaveVolume() {
	int waveVol = VOLUME_INDEX_DEFAULT;
	CBagel *app = CBagel::getBagApp();
	if (app != nullptr) {
		app->getOption(USER_OPTIONS, WAVE_VOLUME, &waveVol, VOLUME_INDEX_DEFAULT);

		if (waveVol < VOLUME_INDEX_MIN || waveVol > VOLUME_INDEX_MAX) {
			waveVol = VOLUME_INDEX_DEFAULT;
		}
	}

	return waveVol;
}

void CBagMasterWin::setWaveVolume(int vol) {
	assert(vol >= VOLUME_INDEX_MIN && vol <= VOLUME_INDEX_MAX);
	CBagel *app = CBagel::getBagApp();

	if (app != nullptr) {
		app->setOption(USER_OPTIONS, WAVE_VOLUME, vol);
	}

	// We will let the sound subsystem do our volume control...
	CBofSound::setVolume(getMidiVolume(), vol);
}

int CBagMasterWin::getCorrection() {
	int correction = DEFAULT_CORRECTION;
	CBagel *app = CBagel::getBagApp();
	if (app != nullptr) {
		app->getOption(USER_OPTIONS, "Correction", &correction, DEFAULT_CORRECTION);

		if (correction < 0 || correction > 6) {
			correction = DEFAULT_CORRECTION;
		}
	}

	if (correction == 6) {
		correction = 0;
	} else {
		correction = (1 << correction);
	}

	return correction;
}

void CBagMasterWin::setCorrection(int correction) {
	assert(correction >= 0 && correction <= 32);

	int actualCorr;

	switch (correction) {
	case 0:
		actualCorr = 6;
		break;

	case 1:
		actualCorr = 0;
		break;

	case 2:
		actualCorr = 1;
		break;

	case 4:
		actualCorr = 2;
		break;

	case 8:
		actualCorr = 3;
		break;

	case 16:
		actualCorr = 4;
		break;

	case 32:
		actualCorr = 5;
		break;

	default:
		actualCorr = DEFAULT_CORRECTION;
		break;
	}

	CBagel *app = CBagel::getBagApp();
	if (app != nullptr) {
		app->setOption(USER_OPTIONS, "Correction", actualCorr);
	}
}

int CBagMasterWin::getPanSpeed() {
	int n = 1;
	CBagel *app = CBagel::getBagApp();
	if (app != nullptr) {
		app->getOption(USER_OPTIONS, "PanSpeed", &n, 1);

		if (n < 0 || n > 5) {
			n = 1;
		}
	}

	return n;
}

void CBagMasterWin::setPanSpeed(int speed) {
	assert(speed >= 0 && speed <= 5);
	CBagel *app = CBagel::getBagApp();

	if (app != nullptr) {
		app->setOption(USER_OPTIONS, "PanSpeed", speed);
	}
}

bool CBagMasterWin::getPanimations() {
	bool panims = 0;
	CBagel *app = CBagel::getBagApp();
	if (app != nullptr) {
		app->getOption(USER_OPTIONS, "Panimations", &panims, true);
	}

	return panims;
}

void CBagMasterWin::setPanimations(bool panimsFl) {
	CBagel *app = CBagel::getBagApp();

	if (app != nullptr) {
		app->setOption(USER_OPTIONS, "Panimations", panimsFl);
	}
}

void CBagMasterWin::muteToggle() {
	static int midiVol = VOLUME_INDEX_MIN;
	static int waveVol = VOLUME_INDEX_MIN;
	static bool muteFl = false;

	if (muteFl) {
		// Restore settings
		setMidiVolume(midiVol);
		setWaveVolume(waveVol);

	} else {
		// Save current settings
		midiVol = getMidiVolume();
		waveVol = getWaveVolume();

		// Mute
		setMidiVolume(VOLUME_INDEX_MIN);
		setWaveVolume(VOLUME_INDEX_MIN);
	}

	// Toggle mute-ness
	muteFl = !muteFl;
}

void CBagMasterWin::forcePaintScreen() {
	CBagel *app = CBagel::getBagApp();
	if (app != nullptr) {
		CBagMasterWin *win = app->getMasterWnd();
		if (win != nullptr) {
			CBagStorageDevWnd *sdev = win->getCurrentStorageDev();
			if (sdev != nullptr) {
				sdev->paintScreen(nullptr);
			}
		}
	}
}

ErrorCode waitForInput() {
	EventLoop eventLoop;

	while (!g_engine->shouldQuit() && !eventLoop.frame()) {
	}

	return ERR_NONE;
}

ErrorCode CBagMasterWin::close() {
	assert(isValidObject(this));

	g_allowPaintFl = false;
	return ERR_NONE;
}

void CBagMasterWin::restoreActiveMessages(CBagStorageDevManager *sdevManager) {
	assert(sdevManager != nullptr);

	if (sdevManager != nullptr) {
		// Make sure the Message Log light will flash if user has
		// waiting messages.
		int n = sdevManager->getNumStorageDevices();
		for (int i = 0; i < n; i++) {
			CBagStorageDev *sdev = sdevManager->getStorageDevice(i);
			if (sdev != nullptr) {

				int m = sdev->getObjectCount();
				for (int j = 0; j < m; j++) {
					CBagObject *curObj = sdev->getObjectByPos(j);
					if (curObj != nullptr && curObj->isMsgWaiting()) {
						sdev->activateLocalObject(curObj);
					}
				}
			}
		}
	}
}

// Set the CIC var to either true or false so that our scripting
// code can tell whether or not to play certain movies (primarily flashbacks).
void setCICStatus(CBagStorageDev *sdev) {
	char workBuff[256];
	CBofString workStr(workBuff, 256);

	// If the new game window is a CIC, then set the global var indicating
	// that this is the case.  Don't reset when we're zooming the PDA.
	if (sdev && sdev->getName() != "BPDAZ_WLD") {
		workStr = "IN_CIC";
		CBagVar *cicVar = g_VarManager->getVariable(workStr);
		if (cicVar) {
			workStr = sdev->isCIC() ? "TRUE" : "FALSE";
			cicVar->setValue(workStr);
		}
	}
}

bool getCICStatus() {
	char workBuff[256];
	CBofString workStr(workBuff, 256);
	bool retValFl = false;

	workStr = "IN_CIC";
	CBagVar *cicVar = g_VarManager->getVariable(workStr);
	if (cicVar) {
		retValFl = (cicVar->getValue() == "TRUE");
	}

	return retValFl;
}

} // namespace Bagel
