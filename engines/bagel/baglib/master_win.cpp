
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

#define BAGAPPCPPFIEL

#include "bagel/baglib/master_win.h"
#include "bagel/baglib/buttons.h"
#include "bagel/baglib/chat_wnd.h"
#include "bagel/baglib/dialogs.h"
#include "bagel/baglib/help.h"
#include "bagel/baglib/menu_dlg.h"
#include "bagel/baglib/opt_window.h"
#include "bagel/baglib/restore_dialog.h"
#include "bagel/baglib/restart_dialog.h"
#include "bagel/baglib/save_dialog.h"
#include "bagel/baglib/start_dialog.h"
#include "bagel/baglib/storage_dev_win.h"
#include "bagel/baglib/wield.h"
#include "bagel/boflib/app.h"
#include "bagel/boflib/file.h"
#include "bagel/boflib/options.h"
#include "bagel/boflib/gfx/palette.h"
#include "bagel/baglib/pan_window.h"
#include "bagel/boflib/rect.h"
#include "bagel/bagel.h"

#undef BAGAPPCPPFIEL

namespace Bagel {

#define BAGAPPCPPFIEL

#if BOF_MAC || BOF_WINMAC
#define STARTWORLD "$SBARDIR:WLD:BAR.WLD"
#define GLOBALWORLD "$SBARDIR:WLD:GLOBAL.WLD"
#define LOADINGBMP "$SBARDIR:GENERAL:SYSTEM:LOADING.BMP"
#else
#define STARTWORLD "$SBARDIR\\WLD\\BAR.WLD"
#define GLOBALWORLD "$SBARDIR\\WLD\\GLOBAL.WLD"
#define LOADINGBMP "$SBARDIR\\GENERAL\\SYSTEM\\LOADING.BMP"
#endif

#define USER_OPTIONS "UserOptions"
#if BOF_MAC || BOF_WINMAC
#define WAVE_VOLUME "SndVolume"
#else
#define WAVE_VOLUME "WaveVolume"
#endif

// Global vars
//
extern BOOL g_bGetVilVars;
extern BOOL g_bUseInitLoc;
extern CBofWindow *g_pHackWindow;
extern BOOL g_bPauseTimer;
static BOOL g_bAllowRestore = FALSE;
extern BOOL g_bAllowPaint;

BOOL g_bRestoreObjList = TRUE;

#define NUM_MSG_STRINGS 3
static INT g_nString = 0;
static CHAR g_szString[NUM_MSG_STRINGS][512];

// static initializations

BOOL CBagMasterWin::m_bObjSave = FALSE;
ST_OBJ *CBagMasterWin::m_pObjList = nullptr;
CBagCursor *CBagMasterWin::m_cCursorList[MAX_CURSORS];
INT CBagMasterWin::m_lMenuCount = 0;
INT CBagMasterWin::m_nCurCursor = 0;

//
//
// CBagMasterWin
//
CBagMasterWin::CBagMasterWin() {
	CBofRect cRect(0, 0, 640 - 1, 480 - 1);

	CBofApp *pApp;
	const CHAR *pAppName = "BAGEL Application";

	if ((pApp = CBofApp::GetApp()) != nullptr) {
		cRect.SetRect(0, 0, pApp->ScreenWidth() - 1, pApp->ScreenHeight() - 1);
		pAppName = pApp->GetAppName();
	}

	// Put game into upper left corner always (BCW 10/08/96 12:39 pm)
	//
	// #ifdef _DEBUG
	cRect.SetRect(0, 0, 640 - 1, 480 - 1);
	// #endif

#if BOF_WINDOWS
	m_hDc = nullptr;
#endif

	m_nFadeIn = 0;
	m_pGameWindow = nullptr;

	m_pStorageDeviceList = nullptr;
	m_pGameSDevList = nullptr;
	m_pVariableList = nullptr;
	m_nDiskID = 1;

	Create(pAppName, &cRect);

	// Hide();

	// Pre-loading the Device context will optimize further paint operations
	// because they won't have to load and unload the DC each time.
	// It's a PARENT DC.
	//
#if BOF_WINDOWS
	m_hDc = GetDC();
#endif

	// Assume default system sceen
#if BOF_MAC || BOF_WINMAC
	m_cSysScreen = "$SBARDIR:GENERAL:SYSTEM:GAMBHALL.BMP";
#else
	m_cSysScreen = "$SBARDIR\\GENERAL\\SYSTEM\\GAMBHALL.BMP";
#endif
	MACROREPLACE(m_cSysScreen);

	// Load wait sound for when user hits the spacebar
	//

#if (BOF_MAC || BOF_WINMAC) && !PLAYWAVONMAC
	CBofString cString("$SBARDIR\\GENERAL\\WAIT.SND");
#else
	CBofString cString("$SBARDIR\\GENERAL\\WAIT.WAV");
#endif
	MACROREPLACE(cString);

	if ((m_pWaitSound = new CBofSound(this, cString, SOUND_MIX)) != nullptr) {
	}
}

CBagMasterWin::~CBagMasterWin() {
	Assert(IsValidObject(this));
	INT i;

	if (m_pWaitSound != nullptr) {
		delete m_pWaitSound;
		m_pWaitSound = nullptr;
	}

	// Get rid of thie static
	//
	if (CBagStorageDev::m_pUnderCursorBmp != nullptr) {
		delete CBagStorageDev::m_pUnderCursorBmp;
		CBagStorageDev::m_pUnderCursorBmp = nullptr;
	}

	CBofApp *pApp;
	if ((pApp = CBofApp::GetApp()) != nullptr) {
		pApp->SetPalette(nullptr);
	}

	// Don't need the pre-loaded device context anymore
	//
#if BOF_WINDOWS
	if (m_hDc != nullptr) {
		ReleaseDC(m_hDc);
		m_hDc = nullptr;
	}
#endif

	// Delete any remaining cursors
	//
	for (i = 0; i < MAX_CURSORS; i++) {
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

	// We can get rid of this buffer since the game is shuting down
	//
	if (m_pObjList != nullptr) {
		BofFree(m_pObjList);
		m_pObjList = nullptr;
	}
}

ERROR_CODE CBagMasterWin::ShowSystemDialog(BOOL bSaveBackground) {
	Assert(IsValidObject(this));

#ifndef DEMO

	CBagStorageDevWnd *pSdev;

	if (((pSdev = GetCurrentStorageDev()) == nullptr) || (pSdev->GetDeviceType() == SDEV_GAMEWIN) || (pSdev->GetDeviceType() == SDEV_ZOOMPDA)) {

		LogInfo("Showing System Screen");

		CBagOptWindow cOptionDialog;
		CBofRect cRect;

		// Use specified bitmap as this dialog's image
		//
		CBofBitmap *pBmp;
		pBmp = Bagel::LoadBitmap(m_cSysScreen.GetBuffer());

		cOptionDialog.SetBackdrop(pBmp);

		cRect = cOptionDialog.GetBackdrop()->GetRect();

		if (!bSaveBackground) {
			cOptionDialog.SetFlags(cOptionDialog.GetFlags() & ~BOFDLG_SAVEBACKGND);
		}

		// create the dialog box
		cOptionDialog.Create("System Dialog", cRect.left, cRect.top, cRect.Width(), cRect.Height(), this);

		CBofWindow *pLastWin = g_pHackWindow;
		g_pHackWindow = &cOptionDialog;

		INT nReturnValue;

		g_bPauseTimer = TRUE;
		nReturnValue = cOptionDialog.DoModal();
		g_bPauseTimer = FALSE;
		cOptionDialog.Detach();

		g_pHackWindow = pLastWin;

		LogInfo("Exiting System Screen");

		// User chose to Quit
		//
		if (nReturnValue == 0) {
			Close();
		}
	}

#endif // !DEMO

	return m_errCode;
}

ERROR_CODE CBagMasterWin::ShowCreditsDialog(CBofWindow *pWin, BOOL bSaveBkg) {
	Assert(IsValidObject(this));

	LogInfo("Showing Credits Screen");

	CBagCreditsDialog cCreditsDialog;
	CBofRect cRect;

	// Use specified bitmap as this dialog's image
	//
	CBofBitmap *pBmp;
	pBmp = Bagel::LoadBitmap(BuildSysDir("BARAREA.BMP"));

	cCreditsDialog.SetBackdrop(pBmp);

	cRect = cCreditsDialog.GetBackdrop()->GetRect();

	// Don't allow save of background?
	//
	if (!bSaveBkg) {
		INT lFlags;
		lFlags = cCreditsDialog.GetFlags();

		cCreditsDialog.SetFlags(lFlags & ~BOFDLG_SAVEBACKGND);
	}

	// Use CBagMasterWin if no parent specified
	//
	if (pWin == nullptr) {
		pWin = this;
	}

	// Create the dialog box
	cCreditsDialog.Create("Save Dialog", cRect.left, cRect.top, cRect.Width(), cRect.Height(), pWin);

	BOOL bSaveTimer;
	bSaveTimer = g_bPauseTimer;
	g_bPauseTimer = TRUE;
	cCreditsDialog.DoModal();
	g_bPauseTimer = bSaveTimer;

	LogInfo("Exiting Credits Screen");

	return m_errCode;
}

BOOL CBagMasterWin::ShowQuitDialog(CBofWindow *pWin, BOOL bSaveBackground) {
	Assert(IsValidObject(this));

	CBagStorageDevWnd *pSdev;
	BOOL bQuit;

	bQuit = FALSE;

	if (((pSdev = GetCurrentStorageDev()) == nullptr) || (pSdev->GetDeviceType() == SDEV_GAMEWIN) || (pSdev->GetDeviceType() == SDEV_ZOOMPDA)) {

		LogInfo("Showing Quit Screen");

		CBagQuitDialog cQuitDialog;
		CBofRect cRect;

		CBofSound::PauseSounds();

		if (pWin == nullptr) {
			pWin = this;
		}

		// Use specified bitmap as this dialog's image
		//
		CBofBitmap *pBmp;
		pBmp = Bagel::LoadBitmap(m_cSysScreen.GetBuffer());

		cQuitDialog.SetBackdrop(pBmp);

		cRect = cQuitDialog.GetBackdrop()->GetRect();

		if (!bSaveBackground) {
			cQuitDialog.SetFlags(cQuitDialog.GetFlags() & ~BOFDLG_SAVEBACKGND);
		}

		// create the dialog box
		cQuitDialog.Create("Quit Dialog", cRect.left, cRect.top, cRect.Width(), cRect.Height(), pWin);

		INT nReturnValue;

		BOOL bSaveTimer;
		bSaveTimer = g_bPauseTimer;
		g_bPauseTimer = TRUE;
		nReturnValue = cQuitDialog.DoModal();
		g_bPauseTimer = bSaveTimer;

		switch (nReturnValue) {

		case SAVE_BTN:
			// ShowSaveDialog(pWin, FALSE);
			bQuit = TRUE;
			break;

		case QUIT_BTN:
			bQuit = TRUE;
			break;

		case CANCEL_BTN:
			bQuit = FALSE;
			break;
		}

		if (!bQuit) {
			CBofSound::ResumeSounds();
		}

		LogInfo("Exiting Quit Screen");
	}

	return bQuit;
}

ERROR_CODE CBagMasterWin::NewGame(VOID) {
	Assert(IsValidObject(this));

	CHAR sWorkStr[256];
	CHAR szCInit[256];

	// BCW - 09/26/96 04:52 pm
	//
	sWorkStr[0] = '\0';
	szCInit[0] = '\0';

	CBofString cInitWld(szCInit, 256);

	CBagel *pApp;

	CBagPanWindow::FlushInputEvents();

	// Inits for a New Game
	m_bObjSave = FALSE;

	// Find the starting .WLD file name
	//
	if ((pApp = CBagel::GetBagApp()) != nullptr) {
		pApp->GetOption("Startup", "WLDFile", sWorkStr, STARTWORLD, 255);

		cInitWld = sWorkStr;
		MACROREPLACE(cInitWld);
	}

#if BOF_MAC && __profile__ && PROFILE_LOADTIME
	OSErr oserr = ::ProfilerInit(collectSummary, bestTimeBase, 300, 20);
	Assert(oserr == noErr);
	::ProfilerSetStatus(TRUE);
#endif

	LoadGlobalVars(GLOBALWORLD);
	LoadFile(cInitWld, "", TRUE);

#if BOF_MAC && __profile__ && PROFILE_LOADTIME
	::ProfilerSetStatus(FALSE);
#if __POWERPC__
	::ProfilerDump("\pMacintosh HD:spacebar_ppc.prof");
#else
	::ProfilerDump("\pMacintosh HD:spacebar_68k.prof");
#endif
	::ProfilerTerm();
#endif

	return m_errCode;
}

/*****************************************************************************
 *
 *  LoadFile -
 *
 *  DESCRIPTION:
 *		Called to load script file containing objects from a file named sFile
 *
 *  SAMPLE USAGE:
 *  		errCode = LoadFile();
 *
 *  RETURNS:
 *		True on success
 *
 *****************************************************************************/
ERROR_CODE CBagMasterWin::LoadFile(const CBofString &sWldName, const CBofString &sStartWldName, BOOL bRestart, BOOL bSetStart) {
	CHAR szLocalBuff[256];
	BOOL bRestore;

	szLocalBuff[0] = '\0';

#if BOF_MAC
	Common::strcpy_s(szLocalBuff, "$SBARDIR:GENERAL:SYSTEM:LEGAL.BMP");
#else
	Common::strcpy_s(szLocalBuff, "$SBARDIR\\GENERAL\\SYSTEM\\LEGAL.BMP");
#endif
	CBofString sWldFileName(szLocalBuff, 256);

	static BOOL bPainted = FALSE;

	// BCW - 01/09/97 02:59 am
	// Make sure we get a new set of vildroid filter variables
	g_bGetVilVars = TRUE;

	// Reset the Queued sound slot volumes back to default
	CBofSound::ResetQVolumes();

	if (!bPainted) {
		bPainted = TRUE;
		MACROREPLACE(sWldFileName);
		CBofRect cRect;
		cRect.left = (640 - 520) / 2;
		cRect.top = (480 - 240) / 2;
		cRect.right = cRect.left + 520 - 1;
		cRect.bottom = cRect.top + 240 - 1;

		PaintBitmap(this, sWldFileName.GetBuffer(), &cRect);
	}

	sWldFileName = sWldName;

	// Reset unique ID for menus
	m_lMenuCount = 0;

	// Keep track of what script we are in
	m_cWldScript = sWldName;

	// 08/22/96 02:00 pm BCW
	// This palette will be deleted so don't let anyone use it, until it is
	// replaced with a new one.
	//
	CBofApp *pApp;
	if ((pApp = CBofApp::GetApp()) != nullptr) {
		pApp->SetPalette(nullptr);
	}

	// Save all used objects (if going to another .WLD file)
	//
	bRestore = FALSE;
	if ((m_pStorageDeviceList != nullptr) && !bRestart) {

		if (!m_bObjSave) {

			m_bObjSave = TRUE;

			// Only allocate the object list when we really need it...
			if (m_pObjList == nullptr) {
				if ((m_pObjList = (ST_OBJ *)BofAlloc(MAX_OBJS * sizeof(ST_OBJ))) != nullptr) {
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
			bRestore = TRUE;
		}
	}

	if (m_pVariableList != nullptr) {
		m_pVariableList->ReleaseVariables(FALSE);
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
	//
	for (INT i = 0; i < MAX_CURSORS; i++) {
		if (m_cCursorList[i] != nullptr) {
			delete m_cCursorList[i];
			m_cCursorList[i] = nullptr;
		}
	}

	// Temp fix for 7-12-96 Demo
	CBagMenu::SetUniversalObjectList(nullptr);

	m_pStorageDeviceList = new CBagStorageDevManager();
	if (!m_pVariableList) {
		m_pVariableList = new CBagVarManager();
	}
	m_pGameSDevList = new CBofList<CBagStorageDev *>;

	MACROREPLACE(sWldFileName);

	// BCW - 11/19/96 06:23 pm
	// Defrag our memory pool to speed script loads
	//
#if BOF_DEBUG
	BofMemDefrag();
#endif

	TimerStart();

	if (FileExists(sWldFileName)) {

		CHAR *pBuf;
		INT nLength;

		// Force buffer to be big enough so that the entire script
		// is pre-loaded
		//
		nLength = FileLength(sWldFileName);
		if ((pBuf = (CHAR *)BofAlloc(nLength)) != nullptr) {
			bof_ifstream fpInput(pBuf, nLength);

			CBofFile cFile;
			cFile.Open(sWldFileName);
			cFile.Read(pBuf, nLength);
			cFile.Close();

			// fpInput.setbuf(pBuf, nLength);

			// BCW - 09/04/96 09:44 pm
			// Fix Visual C++ 4.1 bug
			// Tell IOS that this is my buffer, and don't touch it
			// fpInput.delbuf(0);

			CBagMasterWin::LoadFile(fpInput, sStartWldName, TRUE);

			BofFree(pBuf);
		}

		// Possibly need to switch CDs
		//
		CBagel *pBagApp;
		if ((pBagApp = CBagel::GetBagApp()) != nullptr) {
			if ((m_errCode = pBagApp->VerifyCDInDrive(m_nDiskID, m_cCDChangeAudio.GetBuffer())) != ERR_NONE) {
				Close();
				return m_errCode;
			}
		}

		// Now that we know we are on the correct CD, we can load the cursors
		// Only load the cursors that are not wield cursors
		for (INT i = 0; i < MAX_CURSORS; i++) {
			if (m_cCursorList[i] != nullptr) {
#if OPTIMIZELOADTIME
				if (m_cCursorList[i]->IsWieldCursor() == FALSE) {
					m_cCursorList[i]->Load();
				}
#else
				m_cCursorList[i]->Load();
#endif
			}
		}

		SetActiveCursor(0);
		CBagWield::SetWieldCursor(-1);

		if (g_bRestoreObjList) {

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
				m_bObjSave = FALSE;
			}
		}
		g_bRestoreObjList = TRUE;

		// if a start wld is passed in then use it
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
	LogInfo(BuildString("Time to Load %s, %ld ms", sWldFileName.GetBuffer(), TimerStop()));

	return m_errCode;
}

VOID CBagMasterWin::SaveSDevStack(VOID) {
	Assert(IsValidObject(this));

	// BCW - 12/01/96 02:49 pm
	// Save our SDEV location, so we can restore it from Kerpupu
	//
	CBagStorageDevWnd *pSDevWin;
	CHAR szLocStack[MAX_CLOSEUP_DEPTH][MAX_VAR_VALUE];
	CHAR szTempBuf[256];
	CBofString cStr;
	INT j, i = 0;

	memset(&szLocStack[0][0], 0, sizeof(CHAR) * MAX_CLOSEUP_DEPTH * MAX_VAR_VALUE);
	szTempBuf[0] = '\0';
	if ((pSDevWin = GetCurrentStorageDev()) != nullptr) {

		cStr = pSDevWin->GetName();
		if (!cStr.IsEmpty()) {
			Common::strcpy_s(szLocStack[i], cStr.GetBuffer());
			cStr = pSDevWin->GetPrevSDev();
			i++;
		}

		while ((i < MAX_CLOSEUP_DEPTH) && !cStr.IsEmpty()) {

			if ((pSDevWin = (CBagStorageDevWnd *)m_pStorageDeviceList->GetStorageDevice(cStr)) != nullptr) {

				Common::strcpy_s(szLocStack[i], cStr.GetBuffer());

				i++;
				cStr = pSDevWin->GetPrevSDev();
			} else {
				break;
			}
		}
		i--;
		for (j = i; j >= 0; j--) {

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
		CBagVar *pVar;
		if ((pVar = VARMNGR->GetVariable("$LASTWORLD")) != nullptr) {
			cStr = szTempBuf;
			pVar->SetValue(cStr);
		}
	}
}

/*****************************************************************************
 *
 *  LoadGlobalVars -
 *
 *  DESCRIPTION:
 *		Called to load script file containing variable objects from a file named sWldName
 *
 *  SAMPLE USAGE:
 *  		errCode = LoadFile();
 *
 *  RETURNS:
 *		True on success
 *
 *****************************************************************************/
ERROR_CODE CBagMasterWin::LoadGlobalVars(const CBofString &sWldName) {
	Assert(IsValidObject(this));

	CHAR szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString sWldFileName(szLocalBuff, 256);
	sWldFileName = sWldName;

	if (m_pVariableList != nullptr) {
		delete m_pVariableList;
	}

	if ((m_pVariableList = new CBagVarManager()) != nullptr) {

		MACROREPLACE(sWldFileName);

		if (FileExists(sWldFileName)) {

			CHAR *pBuf;
			INT nLength;

			// Force buffer to be big enough so that the entire script
			// is pre-loaded
			//
			nLength = FileLength(sWldFileName);
			if ((pBuf = (CHAR *)BofAlloc(nLength)) != nullptr) {
				bof_ifstream fpInput(pBuf, nLength);

				CBofFile cFile;
				cFile.Open(sWldFileName);
				cFile.Read(pBuf, nLength);
				cFile.Close();

				// fpInput.setbuf(pBuf, nLength);

				// BCW - 09/04/96 09:44 pm
				// Fix Visual C++ 4.1 bug
				// Tell IOS that this is my buffer, and don't touch it
				// fpInput.delbuf(0);

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
						// LogInfo("New global variable");
						fpInput.EatWhite();
						pVar->SetInfo(fpInput);
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

	return m_errCode;
}

/*****************************************************************************
 *
 *  LoadFile -
 *
 *  DESCRIPTION:
 *		Called to load script file containing objects from a file named sFile
 *
 *  SAMPLE USAGE:
 *  		errCode = LoadFile();
 *
 *  RETURNS:
 *		True on success
 *
 *****************************************************************************/
ERROR_CODE CBagMasterWin::LoadFile(bof_ifstream &fpInput, const CBofString &sWldName, BOOL /*bAttach*/) {
	CHAR szLocalStr[256];
	szLocalStr[0] = 0;
	CBagStorageDev *pSDev;
	CBofRect rRect;
	INT nFilter, nFadeId;
	BOOL bIsWieldCursor = FALSE;

	memset(szLocalStr, 0, 256);
	CBofString sWorkStr(szLocalStr, 256);

	m_sStartWld = sWldName;

	while (!fpInput.eof()) {
		fpInput.EatWhite();
		pSDev = nullptr;
		nFilter = 0;
		rRect.right = rRect.left - 1;
		rRect.bottom = rRect.top - 1;

		KEYWORDS keyword;

		if (fpInput.EatWhite() == -1) {
			break;
		}

		GetKeywordFromStream(fpInput, keyword);

		switch (keyword) {

		case STORAGEDEV: {
			CHAR szNameBuff[256];
			CHAR szTypeBuff[256];
			szNameBuff[0] = 0;
			szTypeBuff[0] = 0;

			memset(szNameBuff, 0, 256);
			memset(szTypeBuff, 0, 256);
			CBofString namestr(szNameBuff, 256);
			CBofString typestr(szTypeBuff, 256);

			fpInput.EatWhite();
			GetAlphaNumFromStream(fpInput, namestr);

			fpInput.EatWhite();

			nFadeId = 0;

			while (fpInput.peek() != '{') {
				GetAlphaNumFromStream(fpInput, sWorkStr);
				fpInput.EatWhite();
				if (!sWorkStr.Find("AS")) {
					fpInput.EatWhite();
					GetAlphaNumFromStream(fpInput, typestr);
				} else if (!sWorkStr.Find("RECT")) {
					fpInput.EatWhite();
					GetRectFromStream(fpInput, rRect);
				} else if (!sWorkStr.Find("FILTER")) {
					fpInput.EatWhite();
					GetIntFromStream(fpInput, nFilter);
				} else if (!sWorkStr.Find("FADE")) { // note that this should usually be set in the link
					fpInput.EatWhite();
					GetIntFromStream(fpInput, nFadeId);
				} else {
					// there is an error here
					LogError(BuildString("FAILED on argument of storage device %s : %s", namestr, typestr));
					return ERR_UNKNOWN;
				}

				fpInput.EatWhite();
			}
			if (!(pSDev = OnNewStorageDev(namestr, typestr))) {
				LogError(BuildString("FAILED on open of storage device %s : %s", namestr, typestr));
				return ERR_UNKNOWN;
			}

			// Default DISK ID for this storage device is the same
			// as the ID specified for this .WLD script file.
			//
			pSDev->SetDiskID(m_nDiskID);

			// LOGINFO"Loading storage device " << namestr << ";" << typestr << endl;

			if (rRect.Width() && rRect.Height())
				pSDev->SetRect(rRect);

			pSDev->LoadFile(fpInput, namestr, FALSE);
			if (nFilter) {
				pSDev->SetFilterId((USHORT)nFilter);

				// Set the filter on the window.  GJJ 9-7-96
				//
				OnNewFilter(pSDev, typestr);
			}

			if (nFadeId != 0) {
				pSDev->SetFadeId((USHORT)nFadeId);
			}

			m_pGameSDevList->AddToTail(pSDev);

#if defined(BOF_DEBUG) && defined(RMS_MESSAGES)
			LogInfo(BuildString("Storage device '%s' at '%d' [ %d / %d ]", namestr.GetBuffer(), pSDev, GetFreePhysMem(), GetFreeMem()));
#endif
			break;
		}

		case START_WLD: {
			CHAR str[256];
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
			bIsWieldCursor = TRUE;
		case CURSOR: {
			CHAR str[256];
			str[0] = 0;

			CBofString sStr(str, 256);
			CBagCursor *pCursor;
			INT nId, x, y;

			fpInput.EatWhite();
			GetIntFromStream(fpInput, nId);
			fpInput.EatWhite();
			if (fpInput.peek() == '=') {
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
				BOOL bUseShared = FALSE;

				fpInput.EatWhite();
				if (fpInput.peek() == '=') {
					CHAR szSharedPalToken[256];
					CBofString tStr(szSharedPalToken, 256);

					fpInput.Get();
					fpInput.EatWhite();

					// Check for shared pal token, if there, then create our cursor
					// with the shared palette bit set
					GetAlphaNumFromStream(fpInput, tStr);
					if (tStr.Find("USESHAREDPAL") != -1) {
						bUseShared = TRUE;
					}
				}

				if ((pCursor = new CBagCursor(sStr, bUseShared)) != nullptr) {
					pCursor->SetHotSpot(x, y);

					Assert(nId >= 0 && nId < MAX_CURSORS);

					// Delete any previous cursor
					if (m_cCursorList[nId] != nullptr) {
						delete m_cCursorList[nId];
					}
					m_cCursorList[nId] = pCursor;

					// Set the wielded cursor status (needed for
					// a load time optimization)
					pCursor->SetWieldCursor(bIsWieldCursor);

				} else {
					ReportError(ERR_MEMORY, "Could not allocate a CBagCursor");
				}

			} else {
				ReportError(ERR_UNKNOWN, "Bad cursor syntax");
			}
			break;
		}

		case PDASTATE: {
			CHAR szPDAState[256];
			szPDAState[0] = '\0';
			CBofString sStr(szPDAState, 256);
			fpInput.EatWhite();
			if (fpInput.peek() == '=') {
				fpInput.Get();
				fpInput.EatWhite();

				GetAlphaNumFromStream(fpInput, sStr);

				if (sStr.Find("MAP") != -1) {
					SBBasePda::SetPDAMode(MAPMODE);
				} else {
					if (sStr.Find("INV") != -1) {
						SBBasePda::SetPDAMode(INVMODE);
					} else {
						if (sStr.Find("LOG") != -1) {
							SBBasePda::SetPDAMode(LOGMODE);
						}
					}
				}
				LogInfo(BuildString("PDASTATE = %s", szPDAState));
			}
			break;
		}

		// Implement shared palettes
		case SHAREDPAL: {
			CHAR szBmpFileName[256];
			szBmpFileName[0] = '\0';
			CBofString sStr(szBmpFileName, 256);
			fpInput.EatWhite();
			if (fpInput.peek() == '=') {
				fpInput.Get();
				fpInput.EatWhite();

				GetAlphaNumFromStream(fpInput, sStr);
				MACROREPLACE(sStr);

				// read the palette in and keep it hanging around for later use
				CBofPalette::SetSharedPalette(sStr);

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
		//
		case DISKAUDIO: {
			CHAR szDiskID[256];
			szDiskID[0] = '\0';
			CBofString sStr(szDiskID, 256);

			fpInput.EatWhite();
			if (fpInput.peek() == '=') {
				fpInput.Get();
				fpInput.EatWhite();

				GetAlphaNumFromStream(fpInput, m_cCDChangeAudio);

				// scg 01.07.97 added !PLAYWAVONMAC conditional

#if BOF_MAC && !PLAYWAVONMAC
				m_cCDChangeAudio.ReplaceStr(".WAV", ".SND");
				m_cCDChangeAudio.ReplaceStr(".wav", ".snd");
#endif

				MACROREPLACE(m_cCDChangeAudio);

				LogInfo(BuildString("DISKAUDIO = %s", m_cCDChangeAudio.GetBuffer()));
			}
			break;
		}

		case DISKID: {
			/*CHAR szDiskID[256];
			szDiskID[0] = '\0';
			CBofString sStr(szDiskID, 256);*/

			// GetAlphaNumFromStream(fpInput, m_sStartWld);
			fpInput.EatWhite();
			if (fpInput.peek() == '=') {
				fpInput.Get();
				fpInput.EatWhite();
				INT n;

				// GetAlphaNumFromStream(fpInput, sStr);
				GetIntFromStream(fpInput, n);
				m_nDiskID = (USHORT)n;

#if BOF_MAC
				SetCurrentDisk(m_nDiskID);
#endif
				// m_cDiskID = sStr;

				/*fpInput.EatWhite();

				GetAlphaNumFromStream(fpInput, sStr);
				m_cDiskLabel = sStr;*/

				LogInfo(BuildString("DISKID = %d", m_nDiskID));
				// LogInfo(BuildString("Please insert the CD labeled '%s'", m_cDiskLabel.GetBuffer()));

			} else {
			}
			break;
		}

		case VARIABLE: {
			CBagVar *xVar = new CBagVar;
			// LogInfo("New global variable");
			fpInput.EatWhite();
			xVar->SetInfo(fpInput);
			break;
		}

		case REMARK: {
			char s[255];
#if BOF_MAC
			// There's a bug in the mac streams code where if the
			// first char that ".Get" is the delimeter (/r or /n) then the
			// next call to get will cause an EOF to be returned.

			char ch = fpInput.peek();
			if (ch == '\r' || ch == '\n')
				fpInput.EatWhite();
			else
#endif
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
	return m_errCode;
}

ERROR_CODE CBagMasterWin::SetStorageDev(const CBofString &sWldName, BOOL bEntry) {
	Assert(CBofObject::IsValidObject(&sWldName));

	CHAR szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString sExt(szLocalBuff, 256);

	sExt = sWldName.Right(4);
	if (m_pGameWindow)
		m_pGameWindow->SetOnUpdate(FALSE);

	if (!sExt.Find(".wld") || !sExt.Find(".WLD") || (sWldName.Find(".WLD~~") > 0) || (sWldName.Find(".wld~~") > 0)) {

		// This is to stop it from going out of scope before
		// the message is received.
		//
		Assert(g_nString >= 0 && g_nString < NUM_MSG_STRINGS);
		Assert(strlen(sWldName) < 512);

		strncpy(g_szString[g_nString], sWldName, 511);

		PostUserMessage(WM_ENTERNEWWLD, (ULONG)g_nString);

		if (++g_nString >= NUM_MSG_STRINGS) {
			g_nString = 0;
		}

	} else if (bEntry) {

		GotoNewWindow((CBofString *)&sWldName);

	} else {

		// This is to stop the string from going out of scope before
		// the message is received.
		//
		Assert(g_nString >= 0 && g_nString < NUM_MSG_STRINGS);
		Assert(strlen(sWldName) < 512);

		strncpy(g_szString[g_nString], sWldName, 511);

		PostUserMessage(WM_EXITCLOSEUPWINDOW, (ULONG)g_nString);

		if (++g_nString >= NUM_MSG_STRINGS) {
			g_nString = 0;
		}
	}

	return ERR_NONE;
}

ERROR_CODE CBagMasterWin::OnHelp(const CBofString &sHelpFile, BOOL /*bSaveBkg*/, CBofWindow *pParent) {
	Assert(IsValidObject(this));

#ifndef DEMO

	if (!sHelpFile.IsEmpty()) {
		CBofRect cRect;
		CBagHelp cHelp;

		CHAR szLocalBuff[256];
		szLocalBuff[0] = '\0';
		CBofString sFile(szLocalBuff, 256);

		sFile = sHelpFile;
		MACROREPLACE(sFile);

		// use specified bitmap as this dialog's image
		CHAR szBkg[256];
		szBkg[0] = '\0';
		CBofString sBkg(szBkg, 256);

		sBkg = BuildString("$SBARDIR%sGENERAL%sRULES%sHELPSCRN.BMP", PATH_DELIMETER, PATH_DELIMETER, PATH_DELIMETER);
		MACROREPLACE(sBkg);

		CBofBitmap *pBmp;
		pBmp = Bagel::LoadBitmap(sBkg);
		cHelp.SetBackdrop(pBmp);

		cRect = cHelp.GetBackdrop()->GetRect();

		if (pParent == nullptr)
			pParent = this;

		// create the dialog box
		cHelp.Create("HelpDialog", cRect.left, cRect.top, cRect.Width(), cRect.Height(), pParent);

		cHelp.SetHelpFile(sFile.GetBuffer());
		cHelp.DoModal();
		cHelp.Detach();
	}

#endif // !DEMO

	return m_errCode;
}

BOOL g_bWaitOK = FALSE;

VOID CBagMasterWin::OnKeyHit(ULONG lKey, ULONG lRepCount) {
	Assert(IsValidObject(this));

	INT nVol;

	switch (lKey) {

	// Dynamic Midi Volume increase
	//
	case BKEY_ALT_UP:
		nVol = GetMidiVolume();
		if (nVol < 12) {
			nVol++;
			SetMidiVolume(nVol);
		}

		break;

	// Dynamic Midi Volume decrease
	//
	case BKEY_ALT_DOWN:
		nVol = GetMidiVolume();
		if (nVol > 0) {
			nVol--;
			SetMidiVolume(nVol);
		}
		break;

	case BKEY_SPACE:
		// case BKEY_PERIOD:

#if BOF_MAC
		if (g_bWaitOK || TRUE) {
#else
		if (g_bWaitOK) {
#endif
			g_bWaitOK = FALSE;

			// Play the tick-tock sound
			//
			if (m_pWaitSound != nullptr) {
				m_pWaitSound->Play();
			}

			VARMNGR->IncrementTimers();

			// Prefilter this guy, could cause something to change in the
			// pan or the PDA or a closeup.
			m_pGameWindow->SetPreFilterPan(TRUE);

			m_pGameWindow->AttachActiveObjects();
		}
		break;

	// Quit
	//
	case BKEY_ALT_Q:
	case BKEY_ALT_q:
	case BKEY_ALT_F4:
#ifdef DEMO
		Close();
#else
		if (ShowQuitDialog(this, FALSE)) {
			Close();
		}
#endif // !DEMO
		break;

	// 	Help
	//
	case BKEY_F1:
		if (m_pGameWindow != nullptr) {
			OnHelp(m_pGameWindow->GetHelpFilename(), FALSE);
		}
		break;

	// Save a Game
	//
	case BKEY_ALT_S:
	case BKEY_ALT_s:
	case BKEY_F2:
		ShowSaveDialog(this, FALSE);
		break;

	// Restore Game
	//
	case BKEY_ALT_R:
	case BKEY_ALT_r:
	case BKEY_F3:
		ShowRestoreDialog(this, FALSE);
		break;

	// System options dialog
	//
	case BKEY_ALT_O:
	case BKEY_ALT_o:
	case BKEY_F4:
		ShowSystemDialog(FALSE);
		break;

	// Toggle sound On/Off
	//
	case BKEY_F5:
		MuteToggle();
		break;

	case BKEY_F6:
		break;

#if BOF_DEBUG

	// Dump contents of all variables (Debug modes 1 and 3 only)
	//
	case BKEY_F7: {
		CBagVar *pVar;
		INT i;
		for (i = 0; i < VARMNGR->GetNumVars(); i++) {

			if ((pVar = VARMNGR->GetVariable(i)) != nullptr) {
				LogInfo(BuildString("VAR[%d]: %s = %s", i, (const CHAR *)pVar->GetName(), (const CHAR *)pVar->GetValue()));
			}
		}

		break;
	}

	// Do some major debug tests
	case BKEY_ALT_D:
	case BKEY_ALT_d:
		VerifyMemoryBlocks();
		CBofObject::ValidateObjectList();
		break;

	case BKEY_ALT_C:
	case BKEY_ALT_c:
		CBofObject::m_bUseSlowTest = !CBofObject::m_bUseSlowTest;
		break;
#endif

	// Restart the game
	//
	case BKEY_F12: {
		ShowRestartDialog(this, FALSE);
		break;
	}

	// Default action
	default:
		if (m_pGameWindow)
			m_pGameWindow->OnKeyHit(lKey, lRepCount);
		break;
	}

	CBofWindow::OnKeyHit(lKey, lRepCount);
}

VOID CBagMasterWin::OnClose() {
	Assert(IsValidObject(this));

	if (m_pGameWindow)
		m_pGameWindow->OnClose();

	g_engine->quitGame();
}

ERROR_CODE CBagMasterWin::GotoNewWindow(CBofString *pStr) {
	Assert(IsValidObject(this));
	Assert(pStr != nullptr);
	Assert(CBofObject::IsValidObject(pStr));

	CBagStorageDev *pSDev;

	CHAR szWorkStr[256];
	CHAR szPrevSDevStr[256];
	CHAR szCurSDevStr[256];

	szWorkStr[0] = '\0';
	szPrevSDevStr[0] = '\0';
	szCurSDevStr[0] = '\0';

	CBofString sWorkStr(szWorkStr, 256);
	CBofString sPrevSDevStr(szPrevSDevStr, 256);
	CBofString sCurrSDevStr(szCurSDevStr, 256);
	BOOL bPrev;
	INT n;

	n = pStr->Find("~~");
	sWorkStr = *pStr;
	bPrev = FALSE;

	while (n > 0) {

		sPrevSDevStr = sWorkStr.Left(n);
		sWorkStr = sWorkStr.Mid(n + 2);
		if ((n = sWorkStr.Find("~~")) > 0) {
			sCurrSDevStr = sWorkStr.Left(n);
			if ((pSDev = m_pStorageDeviceList->GetStorageDevice(sCurrSDevStr)) != nullptr) {
				pSDev->SetPrevSDev(sPrevSDevStr);
				bPrev = TRUE;
			}
		} else {
			sCurrSDevStr = sWorkStr;
			if ((pSDev = m_pStorageDeviceList->GetStorageDevice(sCurrSDevStr)) != nullptr) {
				pSDev->SetPrevSDev(sPrevSDevStr);
				bPrev = TRUE;
			}
		}
	}

	if ((pSDev = m_pStorageDeviceList->GetStorageDevice(sWorkStr)) != nullptr) {

		LogInfo(BuildString("Switching to SDEV: %s", sWorkStr.GetBuffer()));

		if (m_pGameWindow) {
			m_pGameWindow->Detach();

			// MDM 6/3
			// MDM 6/10
			//  if the new storage device is equal to the last windows previous
			//  lets not go in a circle
			//  if the current game window did not have a previous win
			if ((m_pGameWindow->GetPrevSDev().IsEmpty()) || (m_pGameWindow->GetPrevSDev() != pSDev->GetName())) {
				if (!bPrev) {
					pSDev->SetPrevSDev(m_pGameWindow->GetName());
				}
			}
		}

		// BCW - 11/07/96 07:02 pm
		// Don't allow recursion
		//
		if (!pSDev->GetPrevSDev().IsEmpty() && pSDev->GetPrevSDev().CompareNoCase(pSDev->GetName()) == 0) {
			pSDev->SetPrevSDev("");
		}

		// BCW - 01/23/97 03:41 pm - Not used
		// Might have to switch CDs (based on Storage Dev)
		//
		// CBagel *pBagApp;
		// if ((pBagApp = CBagel::GetBagApp()) != nullptr) {
		//    pBagApp->VerifyCDInDrive(pSDev->GetDiskID());
		//}

		m_pGameWindow = (CBagStorageDevWnd *)pSDev;
		SetCICStatus(pSDev);

		INT nFadeId;

		nFadeId = pSDev->GetFadeId();

		if (m_nFadeIn != 0)
			pSDev->SetFadeId((USHORT)m_nFadeIn);

		// Make sure the cursor backdrop is not drawn for the first frame
		pSDev->SetDrawCursorBackdrop(FALSE);

		// Reset paints
		g_bAllowPaint = TRUE;

		pSDev->Attach();

		pSDev->SetFadeId((USHORT)nFadeId);
		m_nFadeIn = 0;

	} else {
		// report error
	}

	return m_errCode;
}

BOOL CBagMasterWin::ShowRestartDialog(CBofWindow *pWin, BOOL bSaveBkg) {
	Assert(IsValidObject(this));

#ifndef DEMO

	CBagStorageDevWnd *pSdev;

	if (((pSdev = GetCurrentStorageDev()) == nullptr) || (pSdev->GetDeviceType() == SDEV_GAMEWIN) || (pSdev->GetDeviceType() == SDEV_ZOOMPDA)) {

		LogInfo("Showing Restart Screen");

		INT nReturn;

		if (pWin == nullptr) {
			pWin = this;
		}

		CBagRestartDialog cDlg(m_cSysScreen.GetBuffer(), nullptr, pWin);

		CBofWindow *pLastWin = g_pHackWindow;
		g_pHackWindow = &cDlg;

		// Don't allow save of background
		if (!bSaveBkg) {
			INT lFlags;
			lFlags = cDlg.GetFlags();

			cDlg.SetFlags(lFlags & ~BOFDLG_SAVEBACKGND);
		}

		BOOL bSaveTimer;
		bSaveTimer = g_bPauseTimer;
		g_bPauseTimer = TRUE;
		nReturn = cDlg.DoModal();
		g_bPauseTimer = bSaveTimer;

		g_pHackWindow = pLastWin;

		LogInfo("Exiting Restart Screen");

		return nReturn == RESTART_BTN;
	}

#endif // !DEMO

	return FALSE;
}

VOID CBagMasterWin::OnUserMessage(ULONG nMessage, ULONG lParam) {
	Assert(IsValidObject(this));

	switch (nMessage) {

	case WM_SHOWSYSTEMDLG:
		ShowSystemDialog();
		break;

	// Ask user if they want to restart or restore a game
	//
	case WM_DIE: {

		// BCW - 11-01-96 - this was causing a hang if you die while
		// there is a background audio looping.  So, I am going
		// to kill all audio.  This will probably break when you die
		// with audio only (i.e. no smacker),  but that can be fixed in
		// script.
		//
		CBofSound::StopSounds();

		// Kill any waiting PDA messages that are queued up...
		CBagPDA::RemoveFromMovieQueue(nullptr);

		CBofBitmap cBmp(Width(), Height(), CBagel::GetApp()->GetPalette());
		cBmp.FillRect(nullptr, COLOR_BLACK);
		cBmp.FadeLines(this, 0, 0);

		CHAR szBuf[MAX_FNAME];

		Common::strcpy_s(szBuf, BuildSysDir("DIED2.BMP"));
		if (lParam == 2) {
			Common::strcpy_s(szBuf, BuildSysDir("START.BMP"));
		}

		CBagStartDialog cDlg(szBuf, nullptr, this);

		INT nRetVal;

		CBofWindow *pLastWin = g_pHackWindow;
		g_pHackWindow = &cDlg;

		g_bAllowRestore = TRUE;
		nRetVal = cDlg.DoModal();
		g_bAllowRestore = FALSE;

		g_pHackWindow = pLastWin;

		// Hide that dialog
		//
		cBmp.Paint(this, 0, 0);

		switch (nRetVal) {

		case RESTORE_BTN:
			break;

		case RESTART_BTN:
			NewGame();
			break;

		case QUIT_BTN:
			Close();
			break;
		}

		return;
	}

	case WM_ENTERNEWWLD: {
		CHAR szLocalBuff[256];
		szLocalBuff[0] = '\0';
		CBofString sWldScript(szLocalBuff, 256);

		//
		// BCW - 12/11/96 01:21 am
		//

		// User info is an index into an array of tempory string buffers
		Assert(lParam >= 0 && lParam < NUM_MSG_STRINGS);
		sWldScript = g_szString[(INT)lParam];

		CHAR szStartWld[256];
		szStartWld[0] = '\0';
		CBofString sStartWld(szStartWld, 256);

		int n = 0;
		if ((n = sWldScript.Find("~~")) > 0) {
			sStartWld = sWldScript.Mid(n + 2);
			sWldScript = sWldScript.Left(n);
		}
		LoadFile(sWldScript, sStartWld);
		break;
	}

	case WM_ENTERPANWINDOW:
	case WM_ENTERCLOSEUPWINDOW:

		// Should never be called
		Assert(FALSE);
		break;

	case WM_EXITCLOSEUPWINDOW: {

		CBofString cStr;

		// User info is an index into an array of tempory string buffers
		Assert(lParam >= 0 && lParam < NUM_MSG_STRINGS);
		cStr = g_szString[(INT)lParam];

		CBagStorageDev *pSDev = m_pStorageDeviceList->GetStorageDevice(cStr);

		if (pSDev) {
			if (m_pGameWindow) {
				m_pGameWindow->Detach();
			}
			pSDev->Attach();
			// make sure that the cursor backdrop is not drawn by the
			// next storage device.
			pSDev->SetDrawCursorBackdrop(FALSE);
			pSDev->SetPreFilterPan(TRUE);
			m_pGameWindow = (CBagStorageDevWnd *)pSDev;

			// Reset the CIC var
			SetCICStatus(pSDev);
		} else {
			// report error
		}
		break;
	}

	default:
		break;
	};

	if (m_pGameWindow)
		m_pGameWindow->SetOnUpdate(TRUE);
}

ERROR_CODE CBagMasterWin::Run(const CHAR * /*pszCommandLine*/) {
	return m_errCode;
}

VOID CBagMasterWin::SetActiveCursor(INT iCursor) {
	Assert(iCursor >= 0 && iCursor < MAX_CURSORS);

#if BOF_DEBUG
	if (iCursor < 0 || iCursor >= MAX_CURSORS) {
		LogError(BuildString("Bad cursor ID: %d", iCursor));
	}
#endif

	if (m_cCursorList[iCursor] != nullptr) {
		m_cCursorList[iCursor]->SetCurrent();
		m_nCurCursor = iCursor;
	}
}

VOID CBagMasterWin::FillSaveBuffer(ST_BAGEL_SAVE *pSaveBuf) {
	Assert(IsValidObject(this));
	Assert(pSaveBuf != nullptr);

	//
	// Fill the save game buffer with all the info we need to restore this game
	//

	// 1st, wipe it
	BofMemSet(pSaveBuf, 0, sizeof(ST_BAGEL_SAVE));

	// Remember how big I am (for Save Game Validation during Restores)
	pSaveBuf->m_lStructSize = sizeof(ST_BAGEL_SAVE);

	CBagel *pApp;
	if ((pApp = CBagel::GetBagApp()) != nullptr) {
		CBagMasterWin *pWin;
		INT i, j, n;

		if ((pWin = pApp->GetMasterWnd()) != nullptr) {
			CBagStorageDevWnd *pSDevWin;
			CBagVarManager *pVarManager;
			CBagVar *pVar;

			// Save Global variables
			//
			if ((pVarManager = GetVariableManager()) != nullptr) {

				// Walk variable list and save each global variable
				//
				j = 0;
				n = pVarManager->GetNumVars();
				for (i = 0; i < n; i++) {

					if ((pVar = pVarManager->GetVariable(i)) != nullptr) {

						// BCW - 11/18/96 01:50 pm
						// Need to save local variables in flashbacks.
						// Let me know if this breaks anything.
						//
						// If it's a global variable, then we need to store it
						//
						// if (pVar->IsGlobal()) {

						if (!pVar->GetName().IsEmpty()) {
							Assert(strlen(pVar->GetName()) < MAX_VAR_NAME);
							Common::strcpy_s(pSaveBuf->m_stVarList[j].m_szName, pVar->GetName());
						}

						if (!pVar->GetValue().IsEmpty()) {
							Assert(strlen(pVar->GetValue()) < MAX_VAR_VALUE);
							Common::strcpy_s(pSaveBuf->m_stVarList[j].m_szValue, pVar->GetValue());
						}

						pSaveBuf->m_stVarList[j].m_nType = (USHORT)pVar->GetType();
						pSaveBuf->m_stVarList[j].m_bGlobal = (UBYTE)pVar->IsGlobal();
						pSaveBuf->m_stVarList[j].m_bConstant = (UBYTE)pVar->IsConstant();
						pSaveBuf->m_stVarList[j].m_bReference = (UBYTE)pVar->IsReference();
						pSaveBuf->m_stVarList[j].m_bTimer = (UBYTE)pVar->IsTimer();
						pSaveBuf->m_stVarList[j].m_bRandom = (UBYTE)pVar->IsRandom();
						pSaveBuf->m_stVarList[j].m_bNumeric = (UBYTE)pVar->IsNumeric();
						pSaveBuf->m_stVarList[j].m_bAttached = (UBYTE)pVar->IsAttached();
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

			if ((pSDevWin = GetCurrentStorageDev()) != nullptr) {
				CHAR szLocalStr[256];
				szLocalStr[0] = 0;
				CBofString cStr(szLocalStr, 256);
				CBagPanWindow *pPanWin;
				CBagStorageDevManager *pManager;

				pSaveBuf->m_nLocType = pSDevWin->GetDeviceType();

				// Remember the pan's position
				//
				if (pSaveBuf->m_nLocType == SDEV_GAMEWIN) {
					CBofRect cPos;

					pPanWin = (CBagPanWindow *)pSDevWin;

					cPos = pPanWin->GetViewPort();

					pSaveBuf->m_nLocX = (USHORT)cPos.left;
					pSaveBuf->m_nLocY = (USHORT)cPos.top;
				}

				if ((pManager = GetStorageDevManager()) != nullptr) {

					pManager->SaveObjList(&pSaveBuf->m_stObjList[0], MAX_OBJS);
					if (IsObjSave()) {

						Assert(m_pObjList != nullptr);

						BofMemCopy(&pSaveBuf->m_stObjListEx[0], m_pObjList, sizeof(ST_OBJ) * MAX_OBJS);
						pSaveBuf->m_bUseEx = 1;

						// BCW - 10/11/96 11:42 am - Can't delete the object list.
						// We need it if we will go back into the Bar
						//
						// restore complete, all done with this list
						// BofFree(m_pObjList);
						// m_pObjList = nullptr;
					}

					// Save current storage device location (stack)
					//
					i = 0;
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

/*****************************************************************************
 *
 *  ShowSaveDialog   - Displays the modal Save Game dialog
 *
 *  DESCRIPTION:
 *
 *
 *  SAMPLE USAGE:
 *  bRestored = ShowSaveDialog();
 *
 *  RETURNS:  BOOL = TRUE if successfully saved a game, FALSE otherwise
 *
 *****************************************************************************/
BOOL CBagMasterWin::ShowSaveDialog(CBofWindow *pWin, BOOL bSaveBkg) {
	Assert(IsValidObject(this));

	BOOL bSaved;

	bSaved = FALSE;

#ifndef DEMO

	CBagStorageDevWnd *pSdev;
	if (((pSdev = GetCurrentStorageDev()) == nullptr) || (pSdev->GetDeviceType() == SDEV_GAMEWIN) || (pSdev->GetDeviceType() == SDEV_ZOOMPDA)) {

		LogInfo("Showing Save Screen");

		ST_BAGEL_SAVE *pSaveBuf;
		INT nId;

		CBofSound::PauseSounds();

		if ((pSaveBuf = (ST_BAGEL_SAVE *)BofAlloc(sizeof(ST_BAGEL_SAVE))) != nullptr) {

			CBagSaveDialog cSaveDialog;
			CBofRect cRect;

			FillSaveBuffer(pSaveBuf);
			cSaveDialog.SetSaveGameBuffer((UBYTE *)pSaveBuf, sizeof(ST_BAGEL_SAVE));

			// Use specified bitmap as this dialog's image
			//
			CBofBitmap *pBmp;
			pBmp = Bagel::LoadBitmap(m_cSysScreen.GetBuffer());

			cSaveDialog.SetBackdrop(pBmp);

			cRect = cSaveDialog.GetBackdrop()->GetRect();

			// Don't allow save of background
			if (!bSaveBkg) {
				INT lFlags;
				lFlags = cSaveDialog.GetFlags();

				cSaveDialog.SetFlags(lFlags & ~BOFDLG_SAVEBACKGND);
			}

			// Create the dialog box
			cSaveDialog.Create("Save Dialog", cRect.left, cRect.top, cRect.Width(), cRect.Height(), pWin);

			BOOL bSaveTimer;
			bSaveTimer = g_bPauseTimer;
			g_bPauseTimer = TRUE;
			nId = cSaveDialog.DoModal();
			g_bPauseTimer = bSaveTimer;

			bSaved = (nId == SAVE_BTN);

			cSaveDialog.Detach();

			BofFree(pSaveBuf);
		} else {
			ReportError(ERR_MEMORY, "Unable to allocate the Save Game Buffer");
		}

		CBofSound::ResumeSounds();

		LogInfo("Exiting Save Screen");
	}
#endif
	return bSaved;
}

VOID CBagMasterWin::DoRestore(ST_BAGEL_SAVE *pSaveBuf) {
	Assert(IsValidObject(this));
	Assert(pSaveBuf != nullptr);

	CHAR szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString cScript(szLocalBuff, 256);

	cScript = pSaveBuf->m_szScript;

	CHAR szBuf[60], szCloseup[256];
	CHAR szLocalStr[256];
	szLocalStr[0] = 0;
	CBofString cStr(szLocalStr, 256);
	INT i;

	// Rebuild the stack of locations (Could be 3 closups deep)
	//
	szCloseup[0] = '\0';
	for (i = MAX_CLOSEUP_DEPTH - 1; i >= 0; i--) {
		if (pSaveBuf->m_szLocStack[i][0] != '\0') {
			Common::sprintf_s(szBuf, "%s~~", pSaveBuf->m_szLocStack[i]);
			Common::strcat_s(szCloseup, szBuf);
		}
	}
	INT n;
	n = strlen(szCloseup);
	if (szCloseup[n - 1] == '~') {
		szCloseup[n - 1] = '\0';
		szCloseup[n - 2] = '\0';
	}

	// Designate closeups
	//
	if (pSaveBuf->m_nLocType == SDEV_CLOSEP) {
		Common::strcat_s(szCloseup, " AS CLOSEUP");
	}

	cStr = szCloseup;

	// Don't allow a local restore
	SetSaveObjs(FALSE);

	// Make sure that all global variables are loaded and available
	LoadGlobalVars(GLOBALWORLD);

	// Tell BAGEL to start over with this script
	LoadFile(cScript, cStr, FALSE, FALSE);

	if (!ErrorOccurred()) {

		// Restore all variables
		//
		CBagVarManager *pVarManager;
		CBagVar *pVar;

		if ((pVarManager = GetVariableManager()) != nullptr) {

			// Reset the Global Vars with these new settings
			//
			for (i = 0; i < MAX_VARS; i++) {

				// If this entry is actually used to store Var info
				//
				if (pSaveBuf->m_stVarList[i].m_bUsed) {

					// Find the Global Var (already in memory)
					//
					if ((pVar = pVarManager->GetVariable(pSaveBuf->m_stVarList[i].m_szName)) != nullptr) {
						// Must be a global var
						// Assert(pVar->IsGlobal());

						pVar->SetValue(pSaveBuf->m_stVarList[i].m_szValue);

						// I don't believe that any of these fields change
						// during the game, so I won't mess with them
						//
						// pVar->SetType(pSaveBuf->m_stVarList[i].m_nType);
						// pVar->SetGlobal(pSaveBuf->m_stVarList[i].m_bGlobal);
						// pVar->SetConstant(pSaveBuf->m_stVarList[i].m_bConstant);
						// pVar->SetReference(pSaveBuf->m_stVarList[i].m_bReference);
						// pVar->SetTimer(pSaveBuf->m_stVarList[i].m_bTimer);
						// pVar->SetRandom(pSaveBuf->m_stVarList[i].m_bRandom);
						// if (pSaveBuf->m_stVarList[i].m_bNumeric) {
						//    pVar->SetNumeric();
						//}
						// if (pSaveBuf->m_stVarList[i].m_bAttached) {
						//    pVar->Attach();
						//} else {
						//    pVar->Detach();
						//}
					} else {
						LogError(BuildString("Global Variable NOT found: %s", pSaveBuf->m_stVarList[i].m_szName));
					}
				}
			}
		}

		CBagStorageDevManager *pSDevManager;

		if ((pSDevManager = GetStorageDevManager()) != nullptr) {

			// Restore any extra obj list info (for .WLD swapping)
			if (m_pObjList == nullptr) {

				if ((m_pObjList = (ST_OBJ *)BofAlloc(MAX_OBJS * sizeof(ST_OBJ))) != nullptr) {

					// BCW - 12/23/96 01:22 pm
					// Init to nullptr (might not use all slots)
					BofMemSet(m_pObjList, 0, MAX_OBJS * sizeof(ST_OBJ));

				} else {
					ReportError(ERR_MEMORY);
				}
			}

			BofMemCopy(GetObjList(), &pSaveBuf->m_stObjListEx[0], sizeof(ST_OBJ) * MAX_OBJS);

			if (pSaveBuf->m_bUseEx) {
				SetSaveObjs(TRUE);
			} else {
				SetSaveObjs(FALSE);
			}

			pSDevManager->RestoreObjList(&pSaveBuf->m_stObjList[0], MAX_OBJS);
		}

		// If it's a Panorama, the set it's View position.
		//
		if (pSaveBuf->m_nLocType == SDEV_GAMEWIN) {
			g_cInitLoc.x = pSaveBuf->m_nLocX;
			g_cInitLoc.y = pSaveBuf->m_nLocY;
			g_bUseInitLoc = TRUE;
		}

		// Now set the start storage device and let the game start
		if (!m_sStartWld.IsEmpty()) {
			SetStorageDev(m_sStartWld);
		}

		RestoreActiveMessages(pSDevManager);
	}
}

/*****************************************************************************
 *
 *  ShowRestoreDialog - Displays the modal Restore Game dialog
 *
 *  DESCRIPTION:
 *
 *
 *  SAMPLE USAGE:
 *  bRestored = ShowRestoreDialog();
 *
 *  RETURNS:  BOOL = TRUE if successfully restored a saved game, FALSE otherwise
 *
 *****************************************************************************/
BOOL CBagMasterWin::ShowRestoreDialog(CBofWindow *pWin, BOOL bSaveBkg) {
	Assert(IsValidObject(this));
	BOOL bRestored;

	bRestored = FALSE;

#ifndef DEMO

	CBagStorageDevWnd *pSdev;
	ST_BAGEL_SAVE *pSaveBuf;

	if (g_bAllowRestore || ((pSdev = GetCurrentStorageDev()) == nullptr) || (pSdev->GetDeviceType() == SDEV_GAMEWIN) || (pSdev->GetDeviceType() == SDEV_ZOOMPDA)) {

		LogInfo("Showing Restore Screen");

		CBofSound::PauseSounds();

		if ((pSaveBuf = (ST_BAGEL_SAVE *)BofAlloc(sizeof(ST_BAGEL_SAVE))) != nullptr) {

			CBagRestoreDialog cRestoreDialog;
			CBofRect cRect;

			cRestoreDialog.SetSaveGameBuffer(pSaveBuf, sizeof(ST_BAGEL_SAVE));
			BofMemSet(pSaveBuf, 0, sizeof(ST_BAGEL_SAVE));

			// Use specified bitmap as this dialog's image
			//
			CBofBitmap *pBmp;
			pBmp = Bagel::LoadBitmap(m_cSysScreen.GetBuffer());

			cRestoreDialog.SetBackdrop(pBmp);

			cRect = cRestoreDialog.GetBackdrop()->GetRect();

			// Don't allow save of background
			if (!bSaveBkg) {
				INT lFlags;
				lFlags = cRestoreDialog.GetFlags();

				cRestoreDialog.SetFlags(lFlags & ~BOFDLG_SAVEBACKGND);
			}

			// Create the dialog box
			cRestoreDialog.Create("Restore Dialog", cRect.left, cRect.top, cRect.Width(), cRect.Height(), pWin);

			CBofWindow *pLastWin = g_pHackWindow;
			g_pHackWindow = &cRestoreDialog;

			BOOL bSaveTimer;
			bSaveTimer = g_bPauseTimer;
			g_bPauseTimer = TRUE;
			cRestoreDialog.DoModal();
			g_bPauseTimer = bSaveTimer;

			cRestoreDialog.Detach();

			bRestored = (!cRestoreDialog.ErrorOccurred() && cRestoreDialog.Restored());
			cRestoreDialog.Destroy();

			g_pHackWindow = pLastWin;

			if (bRestored) {
				CHAR szBuf[256];
				Common::strcpy_s(szBuf, LOADINGBMP);
				CBofString cStr(szBuf, 256);
				MACROREPLACE(cStr);

				CBofRect rect;
				rect.left = (640 - 180) / 2;
				rect.top = (480 - 50) / 2;
				rect.right = rect.left + 180 - 1;
				rect.bottom = rect.top + 50 - 1;

				PaintBitmap(pWin, cStr, &rect);
				CBofCursor::Hide();
				DoRestore(pSaveBuf);
				CBofCursor::Show();
			}

			// Don't need this buffer anymore
			BofFree(pSaveBuf);

		} else {
			ReportError(ERR_MEMORY, "Unable to allocate the Restore Game Buffer");
		}

		CBofSound::ResumeSounds();

		LogInfo("Exiting Restore Screen");
	}
#endif
	return bRestored;
}

// User options
//
#define DEFAULT_CORRECTION 2

BOOL CBagMasterWin::GetFlyThru() {
	CBagel *pApp;
	BOOL bFlyThrusOn;

	bFlyThrusOn = TRUE;
	if ((pApp = CBagel::GetBagApp()) != nullptr) {
		pApp->GetOption(USER_OPTIONS, "FlyThroughs", &bFlyThrusOn, TRUE);
	}

	return bFlyThrusOn;
}

INT CBagMasterWin::GetMidiVolume() {
	CBagel *pApp;
	INT nMidiVol;

	nMidiVol = VOLUME_INDEX_DEFAULT;
	if ((pApp = CBagel::GetBagApp()) != nullptr) {
		pApp->GetOption(USER_OPTIONS, "MidiVolume", &nMidiVol, VOLUME_INDEX_DEFAULT);
		if (nMidiVol < VOLUME_INDEX_MIN || nMidiVol > VOLUME_INDEX_MAX) {
			nMidiVol = VOLUME_INDEX_DEFAULT;
		}
	}

	return nMidiVol;
}

VOID CBagMasterWin::SetMidiVolume(INT nVol) {
	Assert(nVol >= VOLUME_INDEX_MIN && nVol <= VOLUME_INDEX_MAX);
	CBagel *pApp;

	if ((pApp = CBagel::GetBagApp()) != nullptr) {
		pApp->SetOption(USER_OPTIONS, "MidiVolume", nVol);
	}
	// We will let the sound subsystem do our volume control...
	// CBofSound::SetVolume(nVol, GetWaveVolume());
	CBofSound::SetVolume(nVol, GetWaveVolume());
}

INT CBagMasterWin::GetWaveVolume() {
	CBagel *pApp;
	INT nWaveVol;

	nWaveVol = VOLUME_INDEX_DEFAULT;
	if ((pApp = CBagel::GetBagApp()) != nullptr) {
#if BOF_MAC
		pApp->GetOption(USER_OPTIONS, WAVE_VOLUME, &nWaveVol, VOLUME_INDEX_DEFAULT);
#else
		pApp->GetOption(USER_OPTIONS, WAVE_VOLUME, &nWaveVol, VOLUME_INDEX_DEFAULT);
#endif
		if (nWaveVol < VOLUME_INDEX_MIN || nWaveVol > VOLUME_INDEX_MAX) {
			nWaveVol = VOLUME_INDEX_DEFAULT;
		}
	}

	return nWaveVol;
}

VOID CBagMasterWin::SetWaveVolume(INT nVol) {
	Assert(nVol >= VOLUME_INDEX_MIN && nVol <= VOLUME_INDEX_MAX);
	CBagel *pApp;

	if ((pApp = CBagel::GetBagApp()) != nullptr) {
		pApp->SetOption(USER_OPTIONS, WAVE_VOLUME, nVol);
	}

	// We will let the sound subsystem do our volume control...
	CBofSound::SetVolume(GetMidiVolume(), nVol);
}

INT CBagMasterWin::GetCorrection() {
	CBagel *pApp;
	INT nCorrection;

	nCorrection = DEFAULT_CORRECTION;
	if ((pApp = CBagel::GetBagApp()) != nullptr) {
		pApp->GetOption(USER_OPTIONS, "Correction", &nCorrection, DEFAULT_CORRECTION);

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

VOID CBagMasterWin::SetCorrection(INT nCorrection) {
	Assert(nCorrection >= 0 && nCorrection <= 32);

	CBagel *pApp;
	INT nActualCorr;

	nActualCorr = 2;

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

	if ((pApp = CBagel::GetBagApp()) != nullptr) {
		pApp->SetOption(USER_OPTIONS, "Correction", nActualCorr);
	}
}

INT CBagMasterWin::GetPanSpeed() {
	CBagel *pApp;
	INT n;

	n = 1;
	if ((pApp = CBagel::GetBagApp()) != nullptr) {
		pApp->GetOption(USER_OPTIONS, "PanSpeed", &n, 1);

		if (n < 0 || n > 5) {
			n = 1;
		}
	}

	return n;
}

VOID CBagMasterWin::SetPanSpeed(INT nSpeed) {
	Assert(nSpeed >= 0 && nSpeed <= 5);

	CBagel *pApp;

	if ((pApp = CBagel::GetBagApp()) != nullptr) {
		pApp->SetOption(USER_OPTIONS, "PanSpeed", nSpeed);
	}
}

BOOL CBagMasterWin::GetPanimations() {
	CBagel *pApp;
	BOOL bPanims;

	bPanims = 0;
	if ((pApp = CBagel::GetBagApp()) != nullptr) {
		pApp->GetOption(USER_OPTIONS, "Panimations", &bPanims, TRUE);
	}

	return bPanims;
}

VOID CBagMasterWin::SetPanimations(BOOL bPanims) {
	CBagel *pApp;

	if ((pApp = CBagel::GetBagApp()) != nullptr) {
		pApp->SetOption(USER_OPTIONS, "Panimations", bPanims);
	}
}

VOID CBagMasterWin::MuteToggle() {
	static INT nMidiVol = VOLUME_INDEX_MIN;
	static INT nWaveVol = VOLUME_INDEX_MIN;
	static BOOL bMute = FALSE;

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

VOID CBagMasterWin::ForcePaintScreen(BOOL bShowCursor) {
	CBagel *pApp;
	CBagMasterWin *pWin;
	CBagStorageDevWnd *pSDev;

	if ((pApp = CBagel::GetBagApp()) != nullptr) {
		if ((pWin = pApp->GetMasterWnd()) != nullptr) {
			if ((pSDev = pWin->GetCurrentStorageDev()) != nullptr) {
				pSDev->PaintScreen(nullptr, bShowCursor);
			}
		}
	}
}

ERROR_CODE PaintBeveledText(CBofBitmap *pBmp, CBofRect *pRect, const CBofString &cString, const INT nSize, const INT nWeight, const RGBCOLOR cColor, INT nJustify, UINT nFormat, INT nFont) {
	Assert(pBmp != nullptr);
	Assert(pRect != nullptr);

	CBofBitmap cBmp(pRect->Width(), pRect->Height(), nullptr, FALSE);
	CBofRect cBevel, r;
	CBofApp *pApp;
	CBofPalette *pPal;
	ERROR_CODE errCode;

	// Assume no error
	errCode = ERR_NONE;

	r = cBmp.GetRect();

	pPal = nullptr;
	if ((pApp = CBofApp::GetApp()) != nullptr) {
		pPal = pApp->GetPalette();
	}

	if (pPal != nullptr) {
		cBmp.FillRect(nullptr, pPal->GetNearestIndex(RGB(92, 92, 92)));

		cBmp.DrawRect(&r, pPal->GetNearestIndex(RGB(0, 0, 0)));
	} else {
		cBmp.FillRect(nullptr, COLOR_BLACK);
	}

	INT i, left, top, right, bottom;
	UBYTE c1, c2;

	c1 = 3;
	c2 = 9;
	cBevel = r;

	left = cBevel.left;
	top = cBevel.top;
	right = cBevel.right;
	bottom = cBevel.bottom;

	r.left += 6;
	r.top += 3;
	r.right -= 5;
	r.bottom -= 5;

	for (i = 1; i <= 3; i++) {
		cBmp.Line(left + i, bottom - i, right - i, bottom - i, c1);
		cBmp.Line(right - i, bottom - i, right - i, top + i - 1, c1);
	}

	for (i = 1; i <= 3; i++) {
		cBmp.Line(left + i, bottom - i, left + i, top + i - 1, c2);
		cBmp.Line(left + i, top + i - 1, right - i, top + i - 1, c2);
	}

	PaintText(&cBmp, &r, cString, nSize, nWeight, cColor, nJustify, nFormat, nFont);

	cBmp.Paint(pBmp, pRect);

	return errCode;
}

ERROR_CODE PaintBeveledText(CBofWindow *pWin, CBofRect *pRect, const CBofString &cString, const INT nSize, const INT nWeight, const RGBCOLOR cColor, INT nJustify, UINT nFormat, INT nFont) {
	Assert(pWin != nullptr);
	Assert(pRect != nullptr);

	CBofBitmap cBmp(pRect->Width(), pRect->Height(), nullptr, FALSE);
	CBofRect cBevel, r;
	CBofApp *pApp;
	CBofPalette *pPal;
	ERROR_CODE errCode;

	// Assume no error
	errCode = ERR_NONE;

	r = cBmp.GetRect();

	pPal = nullptr;
	if ((pApp = CBofApp::GetApp()) != nullptr) {
		pPal = pApp->GetPalette();
	}

	if (pPal != nullptr) {
		cBmp.FillRect(nullptr, pPal->GetNearestIndex(RGB(92, 92, 92)));

		cBmp.DrawRect(&r, pPal->GetNearestIndex(RGB(0, 0, 0)));
	} else {
		cBmp.FillRect(nullptr, COLOR_BLACK);
	}

	INT i, left, top, right, bottom;
	UBYTE c1, c2;

	c1 = 3;
	c2 = 9;
	cBevel = r;

	left = cBevel.left;
	top = cBevel.top;
	right = cBevel.right;
	bottom = cBevel.bottom;

	r.left += 6;
	r.top += 3;
	r.right -= 5;
	r.bottom -= 5;

	for (i = 1; i <= 3; i++) {
		cBmp.Line(left + i, bottom - i, right - i, bottom - i, c1);
		cBmp.Line(right - i, bottom - i, right - i, top + i - 1, c1);
	}

	for (i = 1; i <= 3; i++) {
		cBmp.Line(left + i, bottom - i, left + i, top + i - 1, c2);
		cBmp.Line(left + i, top + i - 1, right - i, top + i - 1, c2);
	}

	PaintText(&cBmp, &r, cString, nSize, nWeight, cColor, nJustify, nFormat, nFont);

	cBmp.Paint(pWin, pRect);

	return errCode;
}

ERROR_CODE WaitForInput(VOID) {
	ERROR_CODE errCode;

	// Assume no error
	errCode = ERR_NONE;

#if BOF_MAC

	while (!::Button())
		;

	::FlushEvents(everyEvent, 0); // swallow the mousedown, don't want it processed

#elif BOF_WINDOWS
	MSG msg;
	CBagPanWindow::FlushInputEvents();

	// Wait until the user hits a key, or clicks the mouse
	//
	BOOL bDone = FALSE;
	while (!bDone) {

		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {

			switch (msg.message) {

			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
			case WM_LBUTTONDBLCLK:
			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP:
			case WM_RBUTTONDBLCLK:
			case WM_SYSCHAR:
			case WM_CHAR:
			case WM_SYSKEYDOWN:
			case WM_KEYDOWN:
			case WM_SYSKEYUP:
			case WM_KEYUP:
				bDone = TRUE;
				break;

			// do nothing
			default:
				break;
			}

			if (!bDone) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	CBagPanWindow::FlushInputEvents();
#endif

	return errCode;
}

VOID CBagMasterWin::Close(VOID) {
	Assert(IsValidObject(this));

	g_bAllowPaint = FALSE;
#if BOF_WINDOWS
	PostMessage(BM_CLOSE, 0, 0);
#endif
}

VOID CBagMasterWin::RestoreActiveMessages(CBagStorageDevManager *pSDevManager) {
	Assert(pSDevManager != nullptr);

	if (pSDevManager != nullptr) {

		// Fix for CBofSprite assertion line 560
		//
		CBagStorageDev *pSDev;
		CBagObject *pObj;
		INT i, j, n, m;

		// Make sure the Message Log light will flash if user has
		// waiting messages.
		//
		n = pSDevManager->GetNumStorageDevices();
		for (i = 0; i < n; i++) {

			if ((pSDev = pSDevManager->GetStorageDevice(i)) != nullptr) {

				m = pSDev->GetObjectCount();
				for (j = 0; j < m; j++) {

					if ((pObj = pSDev->GetObjectByPos(j)) != nullptr) {
						if (pObj->IsMsgWaiting()) {
							pSDev->ActivateLocalObject(pObj);
						}
					}
				}
			}
		}
	}
}

// Set the CIC var to either true or false so that our scripting
// code can tell whether or not to play certain movies (primarily flashbacks).
VOID SetCICStatus(CBagStorageDev *pSDev) {
	CHAR szLocalBuff[256];
	CBofString sWorkStr(szLocalBuff, 256);

	// If the new game window is a CIC, then set the global var
	// indicating that this is the case.  Don't reset when we're zooming the
	// PDA.
	if (pSDev && pSDev->GetName() != "BPDAZ_WLD") {
		sWorkStr = "IN_CIC";
		CBagVar *pCICVar = VARMNGR->GetVariable(sWorkStr);
		if (pCICVar) {
			sWorkStr = pSDev->IsCIC() ? "TRUE" : "FALSE";
			pCICVar->SetValue(sWorkStr);
		}
	}
}

BOOL GetCICStatus(VOID) {
	CHAR szLocalBuff[256];
	CBofString sWorkStr(szLocalBuff, 256);
	BOOL bRetVal = FALSE;

	sWorkStr = "IN_CIC";
	CBagVar *pCICVar = VARMNGR->GetVariable(sWorkStr);
	if (pCICVar) {
		bRetVal = (pCICVar->GetValue() == "TRUE");
	}

	return bRetVal;
}

} // namespace Bagel
