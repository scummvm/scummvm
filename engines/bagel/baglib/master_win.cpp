
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

bool CBagMasterWin::m_bObjSave = false;
ST_OBJ *CBagMasterWin::m_pObjList = nullptr;
CBagCursor *CBagMasterWin::m_cCursorList[MAX_CURSORS];
int CBagMasterWin::m_lMenuCount = 0;
int CBagMasterWin::m_nCurCursor = 0;

//
//
// CBagMasterWin
//
CBagMasterWin::CBagMasterWin() {
	CBofRect cRect(0, 0, 640 - 1, 480 - 1);

	const char *pAppName = "BAGEL Application";

	CBofApp *pApp = CBofApp::GetApp();
	if (pApp != nullptr) {
		cRect.SetRect(0, 0, pApp->screenWidth() - 1, pApp->screenHeight() - 1);
		pAppName = pApp->GetAppName();
	}

	cRect.SetRect(0, 0, 640 - 1, 480 - 1);
	m_nFadeIn = 0;
	m_pGameWindow = nullptr;

	m_pStorageDeviceList = nullptr;
	m_pGameSDevList = nullptr;
	m_pVariableList = nullptr;
	m_nDiskID = 1;

	create(pAppName, &cRect);

	// Assume default system screen
	m_cSysScreen = "$SBARDIR\\GENERAL\\SYSTEM\\GAMBHALL.BMP";
	MACROREPLACE(m_cSysScreen);

	// Load wait sound for when user hits the spacebar
	CBofString cString("$SBARDIR\\GENERAL\\WAIT.WAV");
	MACROREPLACE(cString);

	m_pWaitSound = new CBofSound(this, cString, SOUND_MIX);
}

CBagMasterWin::~CBagMasterWin() {
	Assert(IsValidObject(this));

	if (m_pWaitSound != nullptr) {
		delete m_pWaitSound;
		m_pWaitSound = nullptr;
	}

	CBofApp *pApp;
	if ((pApp = CBofApp::GetApp()) != nullptr) {
		pApp->setPalette(nullptr);
	}

	// Delete any remaining cursors
	for (int i = 0; i < MAX_CURSORS; i++) {
		if (m_cCursorList[i] != nullptr) {
			delete m_cCursorList[i];
			m_cCursorList[i] = nullptr;
		}
	}

	if (m_pGameWindow != nullptr) {
		delete m_pGameWindow;
		m_pGameWindow = nullptr;
	}
	if (m_pStorageDeviceList != nullptr) {
		delete m_pStorageDeviceList;
		m_pStorageDeviceList = nullptr;
	}
	if (m_pVariableList != nullptr) {
		delete m_pVariableList;
		m_pVariableList = nullptr;
	}
	if (m_pGameSDevList != nullptr) {
		delete m_pGameSDevList;
		m_pGameSDevList = nullptr;
	}

	// We can get rid of this buffer since the game is shutting down
	if (m_pObjList != nullptr) {
		BofFree(m_pObjList);
		m_pObjList = nullptr;
	}
}

ErrorCode CBagMasterWin::ShowSystemDialog(bool bSaveBackground) {
	Assert(IsValidObject(this));
	if (g_engine->isDemo())
		return ERR_NONE;

	CBagStorageDevWnd *pSdev = GetCurrentStorageDev();

	if ((pSdev == nullptr) || (pSdev->GetDeviceType() == SDEV_GAMEWIN) || (pSdev->GetDeviceType() == SDEV_ZOOMPDA)) {
		LogInfo("Showing System Screen");

		CBagOptWindow cOptionDialog;

		// Use specified bitmap as this dialog's image
		CBofBitmap *pBmp = Bagel::loadBitmap(m_cSysScreen.GetBuffer());

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

ErrorCode CBagMasterWin::ShowCreditsDialog(CBofWindow *pWin, bool bSaveBkg) {
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
	if (pWin == nullptr) {
		pWin = this;
	}

	// Create the dialog box
	cCreditsDialog.create("Save Dialog", cRect.left, cRect.top, cRect.width(), cRect.height(), pWin);

	bool bSaveTimer = g_bPauseTimer;
	g_bPauseTimer = true;
	cCreditsDialog.doModal();
	g_bPauseTimer = bSaveTimer;

	LogInfo("Exiting Credits Screen");

	return _errCode;
}

bool CBagMasterWin::ShowQuitDialog(CBofWindow *pWin, bool bSaveBackground) {
	Assert(IsValidObject(this));

	CBagStorageDevWnd *pSdev = GetCurrentStorageDev();
	bool bQuit = false;

	if ((pSdev == nullptr) || (pSdev->GetDeviceType() == SDEV_GAMEWIN) || (pSdev->GetDeviceType() == SDEV_ZOOMPDA)) {
		LogInfo("Showing Quit Screen");

		CBagQuitDialog cQuitDialog;

		CBofSound::PauseSounds();

		if (pWin == nullptr) {
			pWin = this;
		}

		// Use specified bitmap as this dialog's image
		CBofBitmap *pBmp = Bagel::loadBitmap(m_cSysScreen.GetBuffer());

		cQuitDialog.setBackdrop(pBmp);

		CBofRect cRect = cQuitDialog.getBackdrop()->getRect();

		if (!bSaveBackground) {
			cQuitDialog.setFlags(cQuitDialog.getFlags() & ~BOFDLG_SAVEBACKGND);
		}

		// Create the dialog box
		cQuitDialog.create("Quit Dialog", cRect.left, cRect.top, cRect.width(), cRect.height(), pWin);

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

ErrorCode CBagMasterWin::NewGame() {
	Assert(IsValidObject(this));

	char sWorkStr[256];
	char szCInit[256];

	sWorkStr[0] = '\0';
	szCInit[0] = '\0';

	CBofString cInitWld(szCInit, 256);

	CBagPanWindow::FlushInputEvents();

	// Inits for a New Game
	m_bObjSave = false;

	CBagel *pApp = CBagel::getBagApp();
	// Find the starting .WLD file name
	if (pApp != nullptr) {
		pApp->getOption("Startup", "WLDFile", sWorkStr, STARTWORLD, 255);

		cInitWld = sWorkStr;
		MACROREPLACE(cInitWld);
	}

	LoadGlobalVars(GLOBALWORLD);
	LoadFile(cInitWld, "", true);

	return _errCode;
}

ErrorCode CBagMasterWin::LoadFile(const CBofString &sWldName, const CBofString &sStartWldName, bool bRestart, bool bSetStart) {
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

	sWldFileName = sWldName;

	// Reset unique ID for menus
	m_lMenuCount = 0;

	// Keep track of what script we are in
	m_cWldScript = sWldName;

	// This palette will be deleted so don't let anyone use it, until it is
	// replaced with a new one.
	CBofApp *pApp = CBofApp::GetApp();
	if (pApp != nullptr) {
		pApp->setPalette(nullptr);
	}

	// Save all used objects (if going to another .WLD file)
	bool bRestore = false;
	if ((m_pStorageDeviceList != nullptr) && !bRestart) {

		if (!m_bObjSave) {

			m_bObjSave = true;

			// Only allocate the object list when we really need it...
			if (m_pObjList == nullptr) {
				m_pObjList = (ST_OBJ *)BofAlloc(MAX_OBJS * sizeof(ST_OBJ));
				if (m_pObjList != nullptr) {
					// Init to zero (we might not use all slots)
					BofMemSet(m_pObjList, 0, MAX_OBJS * sizeof(ST_OBJ));

				} else {
					ReportError(ERR_MEMORY, "Could not allocate Object list");
				}
			}

			m_pStorageDeviceList->SaveObjList(m_pObjList, MAX_OBJS); // xxx

			// Save our SDEV location, so we can restore it from Kerpupu
			SaveSDevStack();

		} else {
			bRestore = true;
		}
	}

	if (m_pVariableList != nullptr) {
		m_pVariableList->ReleaseVariables(false);
	}

	if (m_pGameSDevList != nullptr) {
		delete m_pGameSDevList;
		m_pGameSDevList = nullptr;
	}

	if (m_pGameWindow != nullptr) {
		delete m_pGameWindow;
		m_pGameWindow = nullptr;
	}

	if (m_pStorageDeviceList != nullptr) {
		delete m_pStorageDeviceList;
		m_pStorageDeviceList = nullptr;
	}

	// Unload all current cursors
	for (int i = 0; i < MAX_CURSORS; i++) {
		if (m_cCursorList[i] != nullptr) {
			delete m_cCursorList[i];
			m_cCursorList[i] = nullptr;
		}
	}

	CBagMenu::SetUniversalObjectList(nullptr);

	m_pStorageDeviceList = new CBagStorageDevManager();
	if (!m_pVariableList) {
		m_pVariableList = new CBagVarManager();
	}
	m_pGameSDevList = new CBofList<CBagStorageDev *>;

	MACROREPLACE(sWldFileName);

	TimerStart();

	if (FileExists(sWldFileName)) {
		// Force buffer to be big enough so that the entire script
		// is pre-loaded
		int nLength = FileLength(sWldFileName);
		char *pBuf = (char *)BofAlloc(nLength);
		if (pBuf != nullptr) {
			bof_ifstream fpInput(pBuf, nLength);

			CBofFile cFile;
			cFile.open(sWldFileName);
			cFile.Read(pBuf, nLength);
			cFile.close();

			CBagMasterWin::LoadFileFromStream(fpInput, sStartWldName);

			BofFree(pBuf);
		}

		// Possibly need to switch CDs
		CBagel *pBagApp = CBagel::getBagApp();
		if (pBagApp != nullptr) {
			_errCode = pBagApp->verifyCDInDrive(m_nDiskID, m_cCDChangeAudio.GetBuffer());
			if (_errCode != ERR_NONE || g_engine->shouldQuit()) {
				close();
				return _errCode;
			}
		}

		// Now that we know we are on the correct CD, we can load the cursors
		// Only load the cursors that are not wield cursors
		for (int i = 0; i < MAX_CURSORS; i++) {
			if (m_cCursorList[i] != nullptr) {
				m_cCursorList[i]->load();
			}
		}

		setActiveCursor(0);
		CBagWield::SetWieldCursor(-1);

		if (g_restoreObjectListFl) {

			if (bRestore && m_bObjSave) {
				Assert(m_pStorageDeviceList != nullptr);
				if (m_pStorageDeviceList != nullptr) {
					// Use a preallocated buffer, trash it when we're done.
					Assert(m_pObjList != nullptr);
					m_pStorageDeviceList->RestoreObjList(m_pObjList, MAX_OBJS);

					// All done with this list, can trash it now
					BofFree(m_pObjList);
					m_pObjList = nullptr;
				}
				m_bObjSave = false;
			}
		}
		g_restoreObjectListFl = true;

		// If a start wld is passed in then use it
		if (!sStartWldName.IsEmpty()) {
			m_sStartWld = sStartWldName;
		}
		if (bSetStart) {
			if (!m_sStartWld.IsEmpty()) {
				SetStorageDev(m_sStartWld);
			}
		}

		RestoreActiveMessages(m_pStorageDeviceList);

	} else {
		ReportError(ERR_FFIND, "Could not find World Script: %s", sWldFileName.GetBuffer());
	}
	LogInfo(BuildString("Time to Load %s, %ld ms", sWldFileName.GetBuffer(), timerStop()));

	return _errCode;
}

void CBagMasterWin::SaveSDevStack() {
	Assert(IsValidObject(this));

	// Save our SDEV location, so we can restore it from Kerpupu
	char szLocStack[MAX_CLOSEUP_DEPTH][MAX_VAR_VALUE];
	char szTempBuf[256];

	memset(&szLocStack[0][0], 0, sizeof(char) * MAX_CLOSEUP_DEPTH * MAX_VAR_VALUE);
	szTempBuf[0] = '\0';
	CBagStorageDevWnd *pSDevWin = GetCurrentStorageDev();
	if (pSDevWin != nullptr) {
		int i = 0;

		CBofString cStr = pSDevWin->GetName();
		if (!cStr.IsEmpty()) {
			Common::strcpy_s(szLocStack[i], cStr.GetBuffer());
			cStr = pSDevWin->GetPrevSDev();
			i++;
		}

		while ((i < MAX_CLOSEUP_DEPTH) && !cStr.IsEmpty()) {
			pSDevWin = (CBagStorageDevWnd *)m_pStorageDeviceList->GetStorageDevice(cStr);
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
		CBagVar *pVar = VARMNGR->GetVariable("$LASTWORLD");
		if (pVar != nullptr) {
			cStr = szTempBuf;
			pVar->SetValue(cStr);
		}
	}
}

ErrorCode CBagMasterWin::LoadGlobalVars(const CBofString &sWldName) {
	Assert(IsValidObject(this));

	char szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString sWldFileName(szLocalBuff, 256);
	sWldFileName = sWldName;

	delete m_pVariableList;
	m_pVariableList = new CBagVarManager();

	if (m_pVariableList != nullptr) {

		MACROREPLACE(sWldFileName);

		if (FileExists(sWldFileName)) {
			// Force buffer to be big enough so that the entire script
			// is pre-loaded
			int nLength = FileLength(sWldFileName);
			char *pBuf = (char *)BofAlloc(nLength);
			if (pBuf != nullptr) {
				bof_ifstream fpInput(pBuf, nLength);

				CBofFile cFile;
				cFile.open(sWldFileName);
				cFile.Read(pBuf, nLength);
				cFile.close();

				while (!fpInput.eof()) {
					fpInput.EatWhite();

					KEYWORDS keyword;

					if (fpInput.EatWhite() == -1) {
						break;
					}

					GetKeywordFromStream(fpInput, keyword);

					switch (keyword) {

					case VARIABLE: {
						CBagVar *pVar = new CBagVar;
						fpInput.EatWhite();
						pVar->setInfo(fpInput);
						pVar->SetGlobal();
						break;
					}

					case REMARK: {
						char s[256];
						fpInput.Get(s, 255);
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

ErrorCode CBagMasterWin::LoadFileFromStream(bof_ifstream &fpInput, const CBofString &sWldName) {
	char szLocalStr[256];
	szLocalStr[0] = 0;
	CBofRect rRect;
	bool bIsWieldCursor = false;

	memset(szLocalStr, 0, 256);
	CBofString sWorkStr(szLocalStr, 256);

	m_sStartWld = sWldName;

	while (!fpInput.eof()) {
		fpInput.EatWhite();
		CBagStorageDev *pSDev = nullptr;
		int nFilter = 0;
		rRect.right = rRect.left - 1;
		rRect.bottom = rRect.top - 1;

		KEYWORDS keyword;

		if (fpInput.EatWhite() == -1) {
			break;
		}

		GetKeywordFromStream(fpInput, keyword);

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

			fpInput.EatWhite();
			GetAlphaNumFromStream(fpInput, namestr);

			fpInput.EatWhite();

			int nFadeId = 0;

			while (fpInput.peek() != '{') {
				GetAlphaNumFromStream(fpInput, sWorkStr);
				fpInput.EatWhite();
				if (!sWorkStr.Find("AS")) {
					fpInput.EatWhite();
					GetAlphaNumFromStream(fpInput, typestr);
				} else if (!sWorkStr.Find("RECT")) {
					fpInput.EatWhite();
					getRectFromStream(fpInput, rRect);
				} else if (!sWorkStr.Find("FILTER")) {
					fpInput.EatWhite();
					GetIntFromStream(fpInput, nFilter);
				} else if (!sWorkStr.Find("FADE")) { // Note that this should usually be set in the link
					fpInput.EatWhite();
					GetIntFromStream(fpInput, nFadeId);
				} else {
					// There is an error here
					LogError(BuildString("FAILED on argument of storage device %s : %s", namestr.GetBuffer(), typestr.GetBuffer()));
					return ERR_UNKNOWN;
				}

				fpInput.EatWhite();
			}
			pSDev = OnNewStorageDev(typestr);
			if (!pSDev) {
				LogError(BuildString("FAILED on open of storage device %s : %s", namestr.GetBuffer(), typestr.GetBuffer()));
				return ERR_UNKNOWN;
			}

			// Default DISK ID for this storage device is the same
			// as the ID specified for this .WLD script file.
			pSDev->SetDiskID(m_nDiskID);

			if (rRect.width() && rRect.height())
				pSDev->SetRect(rRect);

			pSDev->LoadFileFromStream(fpInput, namestr, false);
			if (nFilter) {
				pSDev->SetFilterId((uint16)nFilter);

				// Set the filter on the window.
				OnNewFilter(pSDev, typestr);
			}

			if (nFadeId != 0) {
				pSDev->SetFadeId((uint16)nFadeId);
			}

			m_pGameSDevList->addToTail(pSDev);
			break;
		}

		case START_WLD: {
			char str[256];
			str[0] = '\0';

			CBofString sStr(str, 256);

			fpInput.EatWhite();
			if (fpInput.peek() == '=') {
				fpInput.Get();
				fpInput.EatWhite();
				GetAlphaNumFromStream(fpInput, sStr);

				// Only use the start wld if not specified elsewhere
				m_sStartWld = sStr;
				LogInfo(BuildString("START_WLD set to %s", m_sStartWld.GetBuffer()));
			}
			break;
		}

		case WIELDCURSOR:
			bIsWieldCursor = true;
		case CURSOR: {
			char str[256];
			str[0] = 0;

			CBofString sStr(str, 256);
			int nId;

			fpInput.EatWhite();
			GetIntFromStream(fpInput, nId);
			fpInput.EatWhite();
			if (fpInput.peek() == '=') {
				int x, y;
				fpInput.Get();
				fpInput.EatWhite();

				GetIntFromStream(fpInput, x);
				fpInput.EatWhite();

				GetIntFromStream(fpInput, y);
				fpInput.EatWhite();

				GetAlphaNumFromStream(fpInput, sStr);
				MACROREPLACE(sStr);

				// Specify if we have a shared palette or not, look for
				// the USESHAREDPAL token after the full cursor specification
				bool bUseShared = false;

				fpInput.EatWhite();
				if (fpInput.peek() == '=') {
					char szSharedPalToken[256];
					CBofString tStr(szSharedPalToken, 256);

					fpInput.Get();
					fpInput.EatWhite();

					// Check for shared pal token, if there, then create our cursor
					// with the shared palette bit set
					GetAlphaNumFromStream(fpInput, tStr);
					if (tStr.Find("USESHAREDPAL") != -1) {
						bUseShared = true;
					}
				}

				CBagCursor *pCursor = new CBagCursor(sStr, bUseShared);
				if (pCursor != nullptr) {
					pCursor->setHotspot(x, y);

					Assert(nId >= 0 && nId < MAX_CURSORS);

					// Delete any previous cursor
					if (m_cCursorList[nId] != nullptr) {
						delete m_cCursorList[nId];
					}
					m_cCursorList[nId] = pCursor;

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
			fpInput.EatWhite();
			if (fpInput.peek() == '=') {
				fpInput.Get();
				fpInput.EatWhite();

				GetAlphaNumFromStream(fpInput, sStr);

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
			fpInput.EatWhite();
			if (fpInput.peek() == '=') {
				fpInput.Get();
				fpInput.EatWhite();

				GetAlphaNumFromStream(fpInput, sStr);
				MACROREPLACE(sStr);

				// Read the palette in and keep it hanging around for later use
				CBofPalette::setSharedPalette(sStr);

				LogInfo(BuildString("SHAREDPAL = %s", sStr.GetBuffer()));
			}
			break;
		}

		case SYSSCREEN: {

			fpInput.EatWhite();
			if (fpInput.peek() == '=') {
				fpInput.Get();
				fpInput.EatWhite();

				GetAlphaNumFromStream(fpInput, m_cSysScreen);
				MACROREPLACE(m_cSysScreen);

				LogInfo(BuildString("SYSSCREEN = %s", m_cSysScreen.GetBuffer()));
			}
			break;
		}

		// What audio file should play for this disk swap
		case DISKAUDIO: {
			char szDiskID[256];
			szDiskID[0] = '\0';
			CBofString sStr(szDiskID, 256);

			fpInput.EatWhite();
			if (fpInput.peek() == '=') {
				fpInput.Get();
				fpInput.EatWhite();

				GetAlphaNumFromStream(fpInput, m_cCDChangeAudio);
				MACROREPLACE(m_cCDChangeAudio);

				LogInfo(BuildString("DISKAUDIO = %s", m_cCDChangeAudio.GetBuffer()));
			}
			break;
		}

		case DISKID: {
			fpInput.EatWhite();
			if (fpInput.peek() == '=') {
				fpInput.Get();
				fpInput.EatWhite();
				int n;

				GetIntFromStream(fpInput, n);
				m_nDiskID = (uint16)n;

				LogInfo(BuildString("DISKID = %d", m_nDiskID));

			} else {
			}
			break;
		}

		case VARIABLE: {
			CBagVar *xVar = new CBagVar;
			// LogInfo("New global variable");
			fpInput.EatWhite();
			xVar->setInfo(fpInput);
			break;
		}

		case REMARK: {
			char s[255];
			fpInput.Get(s, 255);
			break;
		}

		default: {
			ParseAlertBox(fpInput, "Syntax Error:", __FILE__, __LINE__);
			break;
		}
		}

	} // While not eof

	// Add everything to the window
	return _errCode;
}

ErrorCode CBagMasterWin::SetStorageDev(const CBofString &sWldName, bool bEntry) {
	Assert(CBofObject::IsValidObject(&sWldName));

	char szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString sExt(szLocalBuff, 256);

	sExt = sWldName.Right(4);
	if (m_pGameWindow)
		m_pGameWindow->SetOnUpdate(false);

	if (!sExt.Find(".wld") || !sExt.Find(".WLD") || (sWldName.Find(".WLD~~") > 0) || (sWldName.Find(".wld~~") > 0)) {

		// This is to stop it from going out of scope before
		// the message is received.
		Assert(g_nString >= 0 && g_nString < NUM_MSG_STRINGS);
		Assert(sWldName.GetLength() < 512);

		strncpy(g_szString[g_nString], sWldName, 511);

		postUserMessage(WM_ENTERNEWWLD, (uint32)g_nString);

		if (++g_nString >= NUM_MSG_STRINGS) {
			g_nString = 0;
		}

	} else if (bEntry) {

		GotoNewWindow(&sWldName);

	} else {

		// This is to stop the string from going out of scope before
		// the message is received.
		Assert(g_nString >= 0 && g_nString < NUM_MSG_STRINGS);
		Assert(sWldName.GetLength() < 512);
		Common::strcpy_s(g_szString[g_nString], sWldName);

		postUserMessage(WM_EXITCLOSEUPWINDOW, (uint32)g_nString);

		if (++g_nString >= NUM_MSG_STRINGS) {
			g_nString = 0;
		}
	}

	return ERR_NONE;
}

ErrorCode CBagMasterWin::OnHelp(const CBofString &sHelpFile, bool /*bSaveBkg*/, CBofWindow *pParent) {
	Assert(IsValidObject(this));

	if (g_engine->isDemo())
		return ERR_NONE;

	if (!sHelpFile.IsEmpty()) {
		CBagHelp cHelp;

		char szLocalBuff[256];
		szLocalBuff[0] = '\0';
		CBofString sFile(szLocalBuff, 256);

		sFile = sHelpFile;
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

		if (pParent == nullptr)
			pParent = this;

		// create the dialog box
		cHelp.create("HelpDialog", cRect.left, cRect.top, cRect.width(), cRect.height(), pParent);

		cHelp.SetHelpFile(sFile.GetBuffer());
		cHelp.doModal();
		cHelp.detach();
	}

	return _errCode;
}

bool g_bWaitOK = false;

void CBagMasterWin::onKeyHit(uint32 lKey, uint32 lRepCount) {
	Assert(IsValidObject(this));

	int nVol;

	switch (lKey) {
	// Dynamic Midi Volume increase
	case BKEY_ALT_UP:
		nVol = GetMidiVolume();
		if (nVol < 12) {
			nVol++;
			SetMidiVolume(nVol);
		}

		break;

	// Dynamic Midi Volume decrease
	case BKEY_ALT_DOWN:
		nVol = GetMidiVolume();
		if (nVol > 0) {
			nVol--;
			SetMidiVolume(nVol);
		}
		break;

	case BKEY_SPACE:
		if (g_bWaitOK) {
			g_bWaitOK = false;

			// Play the tick-tock sound
			if (m_pWaitSound != nullptr) {
				m_pWaitSound->play();
			}

			VARMNGR->IncrementTimers();

			// Prefilter this guy, could cause something to change in the
			// pan or the PDA or a closeup.
			m_pGameWindow->SetPreFilterPan(true);

			m_pGameWindow->AttachActiveObjects();
		}
		break;

	// Quit
	case BKEY_ALT_q:
	case BKEY_ALT_F4:
		if (g_engine->isDemo() || ShowQuitDialog(this, false)) {
			close();
			g_engine->quitGame();
		}
		break;

	//  Help
	case BKEY_F1:
		if (m_pGameWindow != nullptr) {
			OnHelp(m_pGameWindow->GetHelpFilename(), false);
		}
		break;

	// Save a Game
	case BKEY_ALT_s:
	case BKEY_SAVE:
		ShowSaveDialog(this, false);
		break;

	// Restore Game
	case BKEY_RESTORE:
		ShowRestoreDialog(this, false);
		break;

	// System options dialog
	case BKEY_ALT_o:
	case BKEY_F4:
		ShowSystemDialog(false);
		break;

	// Toggle sound On/Off
	case BKEY_ALT_m:
		MuteToggle();
		break;

	// Restart the game
	case BKEY_F12:
		ShowRestartDialog(this, false);
		break;

	// Default action
	default:
		if (m_pGameWindow)
			m_pGameWindow->onKeyHit(lKey, lRepCount);
		break;
	}

	CBofWindow::onKeyHit(lKey, lRepCount);
}

void CBagMasterWin::onClose() {
	Assert(IsValidObject(this));

	if (m_pGameWindow)
		m_pGameWindow->onClose();

	g_engine->quitGame();
}

ErrorCode CBagMasterWin::GotoNewWindow(const CBofString *pStr) {
	Assert(IsValidObject(this));
	Assert(pStr != nullptr);
	Assert(CBofObject::IsValidObject(pStr));

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

	int n = pStr->Find("~~");
	sWorkStr = *pStr;
	bool bPrev = false;

	while (n > 0) {
		sPrevSDevStr = sWorkStr.Left(n);
		sWorkStr = sWorkStr.Mid(n + 2);
		n = sWorkStr.Find("~~");
		if (n > 0) {
			sCurrSDevStr = sWorkStr.Left(n);
			pSDev = m_pStorageDeviceList->GetStorageDevice(sCurrSDevStr);
			if (pSDev != nullptr) {
				pSDev->SetPrevSDev(sPrevSDevStr);
				bPrev = true;
			}
		} else {
			sCurrSDevStr = sWorkStr;
			pSDev = m_pStorageDeviceList->GetStorageDevice(sCurrSDevStr);
			if (pSDev != nullptr) {
				pSDev->SetPrevSDev(sPrevSDevStr);
				bPrev = true;
			}
		}
	}

	pSDev = m_pStorageDeviceList->GetStorageDevice(sWorkStr);
	if (pSDev != nullptr) {

		LogInfo(BuildString("Switching to SDEV: %s", sWorkStr.GetBuffer()));

		if (m_pGameWindow) {
			m_pGameWindow->detach();

			//  If the new storage device is equal to the last windows previous
			//  lets not go in a circle
			//  If the current game window did not have a previous win
			if ((m_pGameWindow->GetPrevSDev().IsEmpty()) || (m_pGameWindow->GetPrevSDev() != pSDev->GetName())) {
				if (!bPrev) {
					pSDev->SetPrevSDev(m_pGameWindow->GetName());
				}
			}
		}

		// Don't allow recursion
		if (!pSDev->GetPrevSDev().IsEmpty() && pSDev->GetPrevSDev().CompareNoCase(pSDev->GetName()) == 0) {
			pSDev->SetPrevSDev("");
		}

		m_pGameWindow = (CBagStorageDevWnd *)pSDev;
		SetCICStatus(pSDev);

		int nFadeId = pSDev->GetFadeId();

		if (m_nFadeIn != 0)
			pSDev->SetFadeId((uint16)m_nFadeIn);

		// Reset paints
		g_allowPaintFl = true;

		pSDev->attach();

		pSDev->SetFadeId((uint16)nFadeId);
		m_nFadeIn = 0;
	}

	return _errCode;
}

bool CBagMasterWin::ShowRestartDialog(CBofWindow *pWin, bool bSaveBkg) {
	Assert(IsValidObject(this));

	if (g_engine->isDemo())
		return false;

	CBagStorageDevWnd *pSdev = GetCurrentStorageDev();
	if ((pSdev == nullptr) || (pSdev->GetDeviceType() == SDEV_GAMEWIN) || (pSdev->GetDeviceType() == SDEV_ZOOMPDA)) {

		LogInfo("Showing Restart Screen");

		if (pWin == nullptr) {
			pWin = this;
		}

		CBagRestartDialog cDlg(m_cSysScreen.GetBuffer(), pWin);

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

void CBagMasterWin::onUserMessage(uint32 nMessage, uint32 lParam) {
	Assert(IsValidObject(this));

	switch (nMessage) {
	case WM_SHOWSYSTEMDLG:
		ShowSystemDialog();
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

		CBofBitmap cBmp(width(), height(), CBagel::GetApp()->getPalette());
		cBmp.fillRect(nullptr, COLOR_BLACK);
		cBmp.fadeLines(this, 0, 0);

		char szBuf[MAX_FNAME];

		Common::strcpy_s(szBuf, buildSysDir("DIED2.BMP"));
		if (lParam == 2) {
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
			NewGame();
			break;

		case QUIT_BTN:
			close();
			g_engine->quitGame();
			break;
		}

		return;
	}

	case WM_ENTERNEWWLD: {
		char szLocalBuff[256];
		szLocalBuff[0] = '\0';
		CBofString sWldScript(szLocalBuff, 256);

		// User info is an index into an array of temporary string buffers
		//Assert(lParam >= 0);
		Assert(lParam < NUM_MSG_STRINGS);
		sWldScript = g_szString[(int)lParam];

		char szStartWld[256];
		szStartWld[0] = '\0';
		CBofString sStartWld(szStartWld, 256);

		int n = sWldScript.Find("~~");
		if (n > 0) {
			sStartWld = sWldScript.Mid(n + 2);
			sWldScript = sWldScript.Left(n);
		}
		LoadFile(sWldScript, sStartWld);
		break;
	}

	case WM_ENTERPANWINDOW:
	case WM_ENTERCLOSEUPWINDOW:
		// Should never be called
		Assert(false);
		break;

	case WM_EXITCLOSEUPWINDOW: {
		CBofString cStr;

		// User info is an index into an array of tempory string buffers
		//Assert(lParam >= 0);
		Assert(lParam < NUM_MSG_STRINGS);
		cStr = g_szString[(int)lParam];

		CBagStorageDev *pSDev = m_pStorageDeviceList->GetStorageDevice(cStr);

		if (pSDev) {
			if (m_pGameWindow) {
				m_pGameWindow->detach();
			}

			pSDev->attach();
			pSDev->SetPreFilterPan(true);

			m_pGameWindow = (CBagStorageDevWnd *)pSDev;

			// Reset the CIC var
			SetCICStatus(pSDev);
		} else {
			// Report error
		}
		break;
	}

	default:
		break;
	}

	if (m_pGameWindow)
		m_pGameWindow->SetOnUpdate(true);
}

ErrorCode CBagMasterWin::Run() {
	return _errCode;
}

void CBagMasterWin::setActiveCursor(int iCursor) {
	Assert(iCursor >= 0 && iCursor < MAX_CURSORS);

	if (m_cCursorList[iCursor] != nullptr) {
		m_cCursorList[iCursor]->setCurrent();
		m_nCurCursor = iCursor;
	}
}

void CBagMasterWin::FillSaveBuffer(ST_BAGEL_SAVE *pSaveBuf) {
	Assert(IsValidObject(this));
	Assert(pSaveBuf != nullptr);

	//
	// Fill the save game buffer with all the info we need to restore this game
	//

	// 1st, wipe it
	BofMemSet(pSaveBuf, 0, sizeof(ST_BAGEL_SAVE));

	CBagel *pApp = CBagel::getBagApp();
	if (pApp != nullptr) {
		CBagMasterWin *pWin = pApp->getMasterWnd();
		if (pWin != nullptr) {
			// Save Global variables
			CBagVarManager *pVarManager = GetVariableManager();
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
							Common::strcpy_s(pSaveBuf->m_stVarList[j].m_szName, pVar->GetName());
						}

						if (!pVar->GetValue().IsEmpty()) {
							Assert(strlen(pVar->GetValue()) < MAX_VAR_VALUE);
							Common::strcpy_s(pSaveBuf->m_stVarList[j].m_szValue, pVar->GetValue());
						}

						pSaveBuf->m_stVarList[j].m_nType = (uint16)pVar->GetType();
						pSaveBuf->m_stVarList[j].m_bGlobal = (byte)pVar->IsGlobal();
						pSaveBuf->m_stVarList[j].m_bConstant = (byte)pVar->IsConstant();
						pSaveBuf->m_stVarList[j].m_bReference = (byte)pVar->IsReference();
						pSaveBuf->m_stVarList[j].m_bTimer = (byte)pVar->IsTimer();
						pSaveBuf->m_stVarList[j].m_bRandom = (byte)pVar->IsRandom();
						pSaveBuf->m_stVarList[j].m_bNumeric = (byte)pVar->IsNumeric();
						pSaveBuf->m_stVarList[j].m_bAttached = (byte)pVar->isAttached();
						pSaveBuf->m_stVarList[j].m_bUsed = 1;
						j++;

						// Can't exceed MAX_VARS
						Assert(j < MAX_VARS);
						//}
					}
				}
			}

			// Remember current script file
			strncpy(pSaveBuf->m_szScript, GetWldScript().GetBuffer(), MAX_FNAME - 1);

			CBagStorageDevWnd *pSDevWin = GetCurrentStorageDev();
			if (pSDevWin != nullptr) {
				char szLocalStr[256];
				szLocalStr[0] = 0;
				CBofString cStr(szLocalStr, 256);

				pSaveBuf->m_nLocType = pSDevWin->GetDeviceType();

				// Remember the pan's position
				if (pSaveBuf->m_nLocType == SDEV_GAMEWIN) {
					CBagPanWindow *pPanWin = (CBagPanWindow *)pSDevWin;
					CBofRect cPos = pPanWin->GetViewPort();

					pSaveBuf->m_nLocX = (uint16)cPos.left;
					pSaveBuf->m_nLocY = (uint16)cPos.top;
				}

				CBagStorageDevManager *pManager = GetStorageDevManager();
				if (pManager != nullptr) {
					pManager->SaveObjList(&pSaveBuf->m_stObjList[0], MAX_OBJS);
					if (IsObjSave()) {

						Assert(m_pObjList != nullptr);

						BofMemCopy(&pSaveBuf->m_stObjListEx[0], m_pObjList, sizeof(ST_OBJ) * MAX_OBJS);
						pSaveBuf->m_bUseEx = 1;
					}

					// Save current storage device location (stack)
					int i = 0;
					cStr = pSDevWin->GetName();
					while ((i < MAX_CLOSEUP_DEPTH) && !cStr.IsEmpty()) {
						pSDevWin = (CBagStorageDevWnd *)pManager->GetStorageDevice(cStr);
						Common::strcpy_s(pSaveBuf->m_szLocStack[i], cStr.GetBuffer());
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

bool CBagMasterWin::ShowSaveDialog(CBofWindow *pWin, bool bSaveBkg) {
	Assert(IsValidObject(this));

	if (g_engine->isDemo())
		return false;

	if (!g_engine->_useOriginalSaveLoad) {
		return g_engine->saveGameDialog();
	}

	bool bSaved = false;
	CBagStorageDevWnd *pSdev = GetCurrentStorageDev();
	if ((pSdev == nullptr) || (pSdev->GetDeviceType() == SDEV_GAMEWIN) || (pSdev->GetDeviceType() == SDEV_ZOOMPDA)) {

		LogInfo("Showing Save Screen");
		CBofSound::PauseSounds();
		ST_BAGEL_SAVE *pSaveBuf = (ST_BAGEL_SAVE *)BofAlloc(sizeof(ST_BAGEL_SAVE));

		if (pSaveBuf != nullptr) {
			CBagSaveDialog cSaveDialog;
			FillSaveBuffer(pSaveBuf);
			cSaveDialog.SetSaveGameBuffer((byte *)pSaveBuf, sizeof(ST_BAGEL_SAVE));

			// Use specified bitmap as this dialog's image
			CBofBitmap *pBmp = Bagel::loadBitmap(m_cSysScreen.GetBuffer());

			cSaveDialog.setBackdrop(pBmp);

			CBofRect cRect = cSaveDialog.getBackdrop()->getRect();

			// Don't allow save of background
			if (!bSaveBkg) {
				int lFlags = cSaveDialog.getFlags();
				cSaveDialog.setFlags(lFlags & ~BOFDLG_SAVEBACKGND);
			}

			// Create the dialog box
			cSaveDialog.create("Save Dialog", cRect.left, cRect.top, cRect.width(), cRect.height(), pWin);

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

void CBagMasterWin::DoRestore(ST_BAGEL_SAVE *pSaveBuf) {
	Assert(IsValidObject(this));
	Assert(pSaveBuf != nullptr);

	char szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString cScript(szLocalBuff, 256);

	cScript = pSaveBuf->m_szScript;

	char szBuf[60], szCloseup[256];
	char szLocalStr[256];
	szLocalStr[0] = 0;
	CBofString cStr(szLocalStr, 256);
	int i;

	// Rebuild the stack of locations (Could be 3 closups deep)
	szCloseup[0] = '\0';
	for (i = MAX_CLOSEUP_DEPTH - 1; i >= 0; i--) {
		if (pSaveBuf->m_szLocStack[i][0] != '\0') {
			Common::sprintf_s(szBuf, "%s~~", pSaveBuf->m_szLocStack[i]);
			Common::strcat_s(szCloseup, szBuf);
		}
	}
	int n = strlen(szCloseup);
	if (szCloseup[n - 1] == '~') {
		szCloseup[n - 1] = '\0';
		szCloseup[n - 2] = '\0';
	}

	// Designate closeups
	if (pSaveBuf->m_nLocType == SDEV_CLOSEP) {
		Common::strcat_s(szCloseup, " AS CLOSEUP");
	}

	cStr = szCloseup;

	// Don't allow a local restore
	SetSaveObjs(false);

	// Make sure that all global variables are loaded and available
	LoadGlobalVars(GLOBALWORLD);

	// Tell BAGEL to start over with this script
	LoadFile(cScript, cStr, false, false);

	if (!ErrorOccurred()) {

		// Restore all variables
		//
		CBagVarManager *pVarManager = GetVariableManager();
		if (pVarManager != nullptr) {

			// Reset the Global Vars with these new settings
			for (i = 0; i < MAX_VARS; i++) {
				// If this entry is actually used to store Var info
				if (pSaveBuf->m_stVarList[i].m_bUsed) {
					// Find the Global Var (already in memory)
					CBagVar *pVar = pVarManager->GetVariable(pSaveBuf->m_stVarList[i].m_szName);
					if (pVar != nullptr) {
						pVar->SetValue(pSaveBuf->m_stVarList[i].m_szValue);

					} else {
						LogError(BuildString("Global Variable NOT found: %s", pSaveBuf->m_stVarList[i].m_szName));
					}
				}
			}
		}

		CBagStorageDevManager *pSDevManager = GetStorageDevManager();

		if (pSDevManager != nullptr) {
			// Restore any extra obj list info (for .WLD swapping)
			if (m_pObjList == nullptr) {
				m_pObjList = (ST_OBJ *)BofAlloc(MAX_OBJS * sizeof(ST_OBJ));
				if (m_pObjList != nullptr) {
					// Init to nullptr (might not use all slots)
					BofMemSet(m_pObjList, 0, MAX_OBJS * sizeof(ST_OBJ));

				} else {
					ReportError(ERR_MEMORY);
				}
			}

			BofMemCopy(GetObjList(), &pSaveBuf->m_stObjListEx[0], sizeof(ST_OBJ) * MAX_OBJS);

			if (pSaveBuf->m_bUseEx) {
				SetSaveObjs(true);
			} else {
				SetSaveObjs(false);
			}

			pSDevManager->RestoreObjList(&pSaveBuf->m_stObjList[0], MAX_OBJS);
		}

		// If it's a Panorama, the set it's View position.
		if (pSaveBuf->m_nLocType == SDEV_GAMEWIN) {
			g_engine->g_cInitLoc.x = pSaveBuf->m_nLocX;
			g_engine->g_cInitLoc.y = pSaveBuf->m_nLocY;
			g_engine->g_bUseInitLoc = true;
		}

		// Now set the start storage device and let the game start
		if (!m_sStartWld.IsEmpty()) {
			SetStorageDev(m_sStartWld);
		}

		RestoreActiveMessages(pSDevManager);
	}
}

bool CBagMasterWin::ShowRestoreDialog(CBofWindow *pWin, bool bSaveBkg) {
	Assert(IsValidObject(this));
	if (g_engine->isDemo())
		return false;

	if (!g_engine->_useOriginalSaveLoad) {
		return g_engine->loadGameDialog();
	}

	bool bRestored = false;
	CBagStorageDevWnd *pSdev;

	if (g_bAllowRestore || ((pSdev = GetCurrentStorageDev()) == nullptr) || (pSdev->GetDeviceType() == SDEV_GAMEWIN) || (pSdev->GetDeviceType() == SDEV_ZOOMPDA)) {

		LogInfo("Showing Restore Screen");

		CBofSound::PauseSounds();

		CBagRestoreDialog cRestoreDialog;

		// Use specified bitmap as this dialog's image
		CBofBitmap *pBmp = Bagel::loadBitmap(m_cSysScreen.GetBuffer());

		cRestoreDialog.setBackdrop(pBmp);

		CBofRect cRect = cRestoreDialog.getBackdrop()->getRect();

		// Don't allow save of background
		if (!bSaveBkg) {
			int lFlags;
			lFlags = cRestoreDialog.getFlags();

			cRestoreDialog.setFlags(lFlags & ~BOFDLG_SAVEBACKGND);
		}

		// Create the dialog box
		cRestoreDialog.create("Restore Dialog", cRect.left, cRect.top, cRect.width(), cRect.height(), pWin);

		CBofWindow *pLastWin = g_hackWindow;
		g_hackWindow = &cRestoreDialog;

		bool bSaveTimer = g_bPauseTimer;
		g_bPauseTimer = true;
		cRestoreDialog.doModal();
		g_bPauseTimer = bSaveTimer;

		cRestoreDialog.detach();

		bRestored = (!cRestoreDialog.ErrorOccurred() && cRestoreDialog.Restored());
		cRestoreDialog.destroy();

		g_hackWindow = pLastWin;

		CBofSound::ResumeSounds();

		LogInfo("Exiting Restore Screen");
	}

	return bRestored;
}

// User options
#define DEFAULT_CORRECTION 2

bool CBagMasterWin::GetFlyThru() {
	bool bFlyThrusOn = true;
	CBagel *pApp = CBagel::getBagApp();
	if (pApp != nullptr) {
		pApp->getOption(USER_OPTIONS, "FlyThroughs", &bFlyThrusOn, true);
	}

	return bFlyThrusOn;
}

int CBagMasterWin::GetMidiVolume() {
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

void CBagMasterWin::SetMidiVolume(int nVol) {
	Assert(nVol >= VOLUME_INDEX_MIN && nVol <= VOLUME_INDEX_MAX);
	CBagel *pApp = CBagel::getBagApp();

	if (pApp != nullptr) {
		pApp->setOption(USER_OPTIONS, "MidiVolume", nVol);
	}

	// We will let the sound subsystem do our volume control...
	CBofSound::SetVolume(nVol, GetWaveVolume());
}

int CBagMasterWin::GetWaveVolume() {
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

void CBagMasterWin::SetWaveVolume(int nVol) {
	Assert(nVol >= VOLUME_INDEX_MIN && nVol <= VOLUME_INDEX_MAX);
	CBagel *pApp = CBagel::getBagApp();

	if (pApp != nullptr) {
		pApp->setOption(USER_OPTIONS, WAVE_VOLUME, nVol);
	}

	// We will let the sound subsystem do our volume control...
	CBofSound::SetVolume(GetMidiVolume(), nVol);
}

int CBagMasterWin::GetCorrection() {
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

void CBagMasterWin::SetCorrection(int nCorrection) {
	Assert(nCorrection >= 0 && nCorrection <= 32);

	int nActualCorr = 2;

	switch (nCorrection) {
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

int CBagMasterWin::GetPanSpeed() {
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

void CBagMasterWin::SetPanSpeed(int nSpeed) {
	Assert(nSpeed >= 0 && nSpeed <= 5);
	CBagel *pApp = CBagel::getBagApp();

	if (pApp != nullptr) {
		pApp->setOption(USER_OPTIONS, "PanSpeed", nSpeed);
	}
}

bool CBagMasterWin::GetPanimations() {
	bool bPanims = 0;
	CBagel *pApp = CBagel::getBagApp();
	if (pApp != nullptr) {
		pApp->getOption(USER_OPTIONS, "Panimations", &bPanims, true);
	}

	return bPanims;
}

void CBagMasterWin::SetPanimations(bool bPanims) {
	CBagel *pApp = CBagel::getBagApp();

	if (pApp != nullptr) {
		pApp->setOption(USER_OPTIONS, "Panimations", bPanims);
	}
}

void CBagMasterWin::MuteToggle() {
	static int nMidiVol = VOLUME_INDEX_MIN;
	static int nWaveVol = VOLUME_INDEX_MIN;
	static bool bMute = false;

	if (bMute) {
		// Restore settings
		SetMidiVolume(nMidiVol);
		SetWaveVolume(nWaveVol);

	} else {
		// Save current settings
		nMidiVol = GetMidiVolume();
		nWaveVol = GetWaveVolume();

		// Mute
		SetMidiVolume(VOLUME_INDEX_MIN);
		SetWaveVolume(VOLUME_INDEX_MIN);
	}

	// Toggle mute-ness
	bMute = !bMute;
}

void CBagMasterWin::ForcePaintScreen() {
	CBagel *pApp = CBagel::getBagApp();
	if (pApp != nullptr) {
		CBagMasterWin *pWin = pApp->getMasterWnd();
		if (pWin != nullptr) {
			CBagStorageDevWnd *pSDev = pWin->GetCurrentStorageDev();
			if (pSDev != nullptr) {
				pSDev->PaintScreen(nullptr);
			}
		}
	}
}

ErrorCode PaintBeveledText(CBofBitmap *pBmp, CBofRect *pRect, const CBofString &cString, const int nSize, const int nWeight, const RGBCOLOR cColor, int nJustify, uint32 nFormat, int nFont) {
	Assert(pBmp != nullptr);
	Assert(pRect != nullptr);

	CBofBitmap cBmp(pRect->width(), pRect->height(), nullptr, false);

	// Assume no error
	ErrorCode errCode = ERR_NONE;

	CBofRect r = cBmp.getRect();

	CBofPalette *pPal = nullptr;
	CBofApp *pApp = CBofApp::GetApp();
	if (pApp != nullptr) {
		pPal = pApp->getPalette();
	}

	if (pPal != nullptr) {
		cBmp.fillRect(nullptr, pPal->getNearestIndex(RGB(92, 92, 92)));

		cBmp.drawRect(&r, pPal->getNearestIndex(RGB(0, 0, 0)));
	} else {
		cBmp.fillRect(nullptr, COLOR_BLACK);
	}

	byte c1 = 3;
	byte c2 = 9;
	CBofRect cBevel = r;

	int left = cBevel.left;
	int top = cBevel.top;
	int right = cBevel.right;
	int bottom = cBevel.bottom;

	r.left += 6;
	r.top += 3;
	r.right -= 5;
	r.bottom -= 5;

	for (int i = 1; i <= 3; i++) {
		cBmp.line(left + i, bottom - i, right - i, bottom - i, c1);
		cBmp.line(right - i, bottom - i, right - i, top + i - 1, c1);
	}

	for (int i = 1; i <= 3; i++) {
		cBmp.line(left + i, bottom - i, left + i, top + i - 1, c2);
		cBmp.line(left + i, top + i - 1, right - i, top + i - 1, c2);
	}

	paintText(&cBmp, &r, cString, nSize, nWeight, cColor, nJustify, nFormat, nFont);

	cBmp.paint(pBmp, pRect);

	return errCode;
}

ErrorCode PaintBeveledText(CBofWindow *pWin, CBofRect *pRect, const CBofString &cString, const int nSize, const int nWeight, const RGBCOLOR cColor, int nJustify, uint32 nFormat, int nFont) {
	Assert(pWin != nullptr);
	Assert(pRect != nullptr);

	CBofBitmap cBmp(pRect->width(), pRect->height(), nullptr, false);

	// Assume no error
	ErrorCode errCode = ERR_NONE;

	CBofRect r = cBmp.getRect();
	CBofPalette *pPal = nullptr;
	CBofApp *pApp = CBofApp::GetApp();
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

	paintText(&cBmp, &r, cString, nSize, nWeight, cColor, nJustify, nFormat, nFont);

	cBmp.paint(pWin, pRect);

	return errCode;
}

ErrorCode WaitForInput() {
	EventLoop eventLoop;

	while (!g_engine->shouldQuit() && !eventLoop.frame()) {
	}

	return ERR_NONE;
}

void CBagMasterWin::close() {
	Assert(IsValidObject(this));

	g_allowPaintFl = false;
}

void CBagMasterWin::RestoreActiveMessages(CBagStorageDevManager *pSDevManager) {
	Assert(pSDevManager != nullptr);

	if (pSDevManager != nullptr) {
		// Make sure the Message Log light will flash if user has
		// waiting messages.
		int n = pSDevManager->GetNumStorageDevices();
		for (int i = 0; i < n; i++) {
			CBagStorageDev *pSDev = pSDevManager->GetStorageDevice(i);
			if (pSDev != nullptr) {

				int m = pSDev->GetObjectCount();
				for (int j = 0; j < m; j++) {
					CBagObject *pObj = pSDev->GetObjectByPos(j);
					if (pObj != nullptr && pObj->IsMsgWaiting()) {
						pSDev->ActivateLocalObject(pObj);
					}
				}
			}
		}
	}
}

// Set the CIC var to either true or false so that our scripting
// code can tell whether or not to play certain movies (primarily flashbacks).
void SetCICStatus(CBagStorageDev *pSDev) {
	char szLocalBuff[256];
	CBofString sWorkStr(szLocalBuff, 256);

	// If the new game window is a CIC, then set the global var indicating
	// that this is the case.  Don't reset when we're zooming the PDA.
	if (pSDev && pSDev->GetName() != "BPDAZ_WLD") {
		sWorkStr = "IN_CIC";
		CBagVar *pCICVar = VARMNGR->GetVariable(sWorkStr);
		if (pCICVar) {
			sWorkStr = pSDev->IsCIC() ? "TRUE" : "FALSE";
			pCICVar->SetValue(sWorkStr);
		}
	}
}

bool GetCICStatus() {
	char szLocalBuff[256];
	CBofString sWorkStr(szLocalBuff, 256);
	bool bRetVal = false;

	sWorkStr = "IN_CIC";
	CBagVar *pCICVar = VARMNGR->GetVariable(sWorkStr);
	if (pCICVar) {
		bRetVal = (pCICVar->GetValue() == "TRUE");
	}

	return bRetVal;
}

} // namespace Bagel
