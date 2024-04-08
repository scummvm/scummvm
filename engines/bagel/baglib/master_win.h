
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

#ifndef BAGEL_BAGLIB_MASTER_WIN_H
#define BAGEL_BAGLIB_MASTER_WIN_H

#include "bagel/boflib/gfx/text.h"
#include "bagel/baglib/cursor.h"
#include "bagel/boflib/string.h"
#include "bagel/baglib/bagel.h"
#include "bagel/boflib/gui/window.h"
#include "bagel/boflib/sound.h"
#include "bagel/baglib/object.h"
#include "bagel/dialogs/save_dialog.h"
#include "bagel/baglib/var.h"
#include "bagel/boflib/gui/text_box.h"

namespace Bagel {

#define SDEVMNGR CBagel::GetBagApp()->GetMasterWnd()->GetStorageDevManager()
#define VARMNGR CBagel::GetBagApp()->GetMasterWnd()->GetVariableManager()

class CBagStorageDevManager;
class CBagStorageDevWnd;
class CBagStorageDev;

enum SpaceBarEventType {
	WM_ENTERPANWINDOW     = 1001,
	WM_ENTERCLOSEUPWINDOW = 1002,
	WM_EXITCLOSEUPWINDOW  = 1003,
	WM_ENTERNEWWLD        = 1004,
	WM_DIE                = 1005,
	WM_SHOWSYSTEMDLG      = 1006
};

#define MAX_CURSORS 60

// Some utility routines
void SetCICStatus(CBagStorageDev *pSDev);
bool GetCICStatus();

/**
 * This is the main window that is never destroyed throughout the game.
 * It handles bringing new windows in and out of focus as well as
 * containing the parsing file and info.  It also captures
 * keyboard entry and will save state information
 */
class CBagMasterWin : public CBofWindow, public CBagParseObject {
protected:
	static bool m_bObjSave;
	static ST_OBJ *m_pObjList;
	static CBagCursor *m_cCursorList[MAX_CURSORS];

	CBagStorageDevWnd *m_pGameWindow;

#if BOF_WINDOWS
	HDC m_hDc;
#endif

	CBofString m_cWldScript;
	CBofString m_sStartWld;
	CBofString m_cCDChangeAudio;
	CBofString m_cSysScreen;

	uint16 m_nDiskID;
	int m_nFadeIn;
	CBofList<CBagStorageDev *> *m_pGameSDevList;
	CBagStorageDevManager *m_pStorageDeviceList;
	CBagVarManager *m_pVariableList; // List of variables used throughout wld
	CBofSound *m_pWaitSound;
	static int m_nCurCursor;

public:
	static int m_lMenuCount;

	CBagMasterWin();
	virtual ~CBagMasterWin();
	virtual ERROR_CODE Run(const char *sCommandLine = nullptr);

	void Close();

	CBofString &GetWldScript() {
		return m_cWldScript;
	}

	static void SetActiveCursor(int iCursor);
	static int GetActiveCursor() {
		return m_nCurCursor;
	}

	// User options
	static bool GetFlyThru();

	static bool GetPanimations();
	static void SetPanimations(bool bPanims);

	static int GetCorrection();
	static void SetCorrection(int nCorrection);

	static int GetPanSpeed();
	static void SetPanSpeed(int nSpeed);

	static int GetMidiVolume();
	static void SetMidiVolume(int nVol);

	static int GetWaveVolume();
	static void SetWaveVolume(int nVol);

	static bool GetMidi() {
		return GetMidiVolume() != 0;
	}
	static bool GetDigitalAudio() {
		return GetWaveVolume() != 0;
	}

	static void MuteToggle();
	static void ForcePaintScreen(bool bShowCursor = TRUE);

	virtual ERROR_CODE ShowSystemDialog(bool bSave = TRUE);
	bool ShowRestartDialog(CBofWindow *pWin = nullptr, bool bSave = TRUE);
	bool ShowSaveDialog(CBofWindow *pWin, bool bSave = TRUE);
	bool ShowRestoreDialog(CBofWindow *pWin, bool bSave = TRUE);
	bool ShowQuitDialog(CBofWindow *pWin, bool bSave = TRUE);
	ERROR_CODE ShowCreditsDialog(CBofWindow *pWin, bool bSave = TRUE);

	void FillSaveBuffer(ST_BAGEL_SAVE *pSaveBuf);
	void DoRestore(ST_BAGEL_SAVE *pSaveBuf);

	ERROR_CODE NewGame();

	ERROR_CODE LoadFile(const CBofString &sWldName, const CBofString &sStartWldName, bool bRestart = FALSE, bool bSetStart = TRUE);

	ERROR_CODE SaveFile(const CBofString &sWldName);
	ERROR_CODE LoadFileFromStream(bof_ifstream &fpInput, const CBofString &sWldName, bool bAttach = FALSE);
	ERROR_CODE LoadGlobalVars(const CBofString &sWldName);

	ERROR_CODE SetCurrFadeIn(int nFade) {
		m_nFadeIn = nFade;
		return ERR_NONE;
	}
	ERROR_CODE SetStorageDev(const CBofString &sWldName, bool bEntry = TRUE);
	ERROR_CODE GotoNewWindow(const CBofString *pStr);

	uint16 GetDiskID() {
		return m_nDiskID;
	}
	void SetDiskID(uint16 did) {
		m_nDiskID = did;
	}

	CBofWindow *GetCurrentGameWindow() {
		return (CBofWindow *)m_pGameWindow;
	}
	CBagStorageDevWnd *GetCurrentStorageDev() {
		return m_pGameWindow;
	}
	CBagStorageDevManager *GetStorageDevManager() {
		return m_pStorageDeviceList;
	}
	CBagVarManager *GetVariableManager() {
		return m_pVariableList;
	}

	virtual CBagStorageDev *OnNewStorageDev(const CBofString &namestr, const CBofString &typestr) = 0;
	virtual CBagStorageDev *OnNewStorageDev(const CBofString &namestr, int nType) = 0;

	virtual void OnNewFilter(CBagStorageDev *, const CBofString &typestr) = 0;
	virtual void OnNewFilter(CBagStorageDev *pSDev, const int nType) = 0;

	virtual ERROR_CODE OnHelp(const CBofString &sHelpFile, bool bSaveBkg = TRUE, CBofWindow *pParent = nullptr);

	void OnUserMessage(uint32 nMessage, uint32 lParam) override;

	void OnKeyHit(uint32 lKey, uint32 lRepCount) override;
	void OnClose() override;

	ST_OBJ *GetObjList() {
		return m_pObjList;
	}
	void SetSaveObjs(bool bSave) {
		m_bObjSave = bSave;
	}
	bool IsObjSave() {
		return m_bObjSave;
	}

	void SaveSDevStack();

	virtual void *GetDataStart() override {
		return &m_pGameWindow;
	}
	virtual void *GetDataEnd() override {
		return &m_pVariableList + sizeof(CBagVarManager *);
	}

	// Since we do this from load file and do restore, centralize it in one location.
	void RestoreActiveMessages(CBagStorageDevManager *pSDevManager);
};

ERROR_CODE PaintBeveledText(CBofBitmap *pBmp, CBofRect *pRect, const CBofString &cStr, const int nSize, const int nWeight, const RGBCOLOR cColor = CTEXT_COLOR, int nJustify = JUSTIFY_CENTER, uint32 nFormat = FORMAT_DEFAULT, int nFont = FONT_DEFAULT);
ERROR_CODE PaintBeveledText(CBofWindow *pWin, CBofRect *pRect, const CBofString &cStr, const int nSize, const int nWeight, const RGBCOLOR cColor = CTEXT_COLOR, int nJustify = JUSTIFY_CENTER, uint32 nFormat = FORMAT_DEFAULT, int nFont = FONT_DEFAULT);
ERROR_CODE WaitForInput();

extern bool g_bWaitOK;

} // namespace Bagel

#endif
