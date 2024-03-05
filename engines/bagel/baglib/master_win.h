
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
#include "bagel/baglib/save_dialog.h"
#include "bagel/baglib/var.h"
#include "bagel/boflib/gui/text_box.h"

namespace Bagel {

#define SDEVMNGR CBagel::GetBagApp()->GetMasterWnd()->GetStorageDevManager()
#define VARMNGR CBagel::GetBagApp()->GetMasterWnd()->GetVariableManager()

class CBagStorageDevManager;
class CBagStorageDevWnd;
class CBagStorageDev;

#define WM_ENTERPANWINDOW 1001L
#define WM_ENTERCLOSEUPWINDOW 1002L
#define WM_EXITCLOSEUPWINDOW 1003L
#define WM_ENTERNEWWLD 1004L
#define WM_DIE 1005L
#define WM_SHOWSYSTEMDLG 1006L

#define MAX_CURSORS 60

// Some utility routines
VOID SetCICStatus(CBagStorageDev *pSDev);
BOOL GetCICStatus();

/**
 * This is the main window that is never destroyed throughout the game.
 * It handles bringing new windows in and out of focus as well as
 * containing the parsing file and info.  It also captures
 * keyboard entry and will save state information
 */
class CBagMasterWin : public CBofWindow, public CBagParseObject {
protected:
	static BOOL m_bObjSave;
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

	USHORT m_nDiskID;
	INT m_nFadeIn;
	CBofList<CBagStorageDev *> *m_pGameSDevList;
	CBagStorageDevManager *m_pStorageDeviceList;
	CBagVarManager *m_pVariableList; // List of variables used throughout wld
	CBofSound *m_pWaitSound;
	static INT m_nCurCursor;

public:
	static INT m_lMenuCount;

	CBagMasterWin();
	virtual ~CBagMasterWin();
	virtual ERROR_CODE Run(const char *sCommandLine = nullptr);

	VOID Close();

	CBofString &GetWldScript() { return m_cWldScript; }

	static VOID SetActiveCursor(INT iCursor);
	static INT GetActiveCursor() { return m_nCurCursor; }

	// User options
	static BOOL GetFlyThru();

	static BOOL GetPanimations();
	static VOID SetPanimations(BOOL bPanims);

	static INT GetCorrection();
	static VOID SetCorrection(INT nCorrection);

	static INT GetPanSpeed();
	static VOID SetPanSpeed(INT nSpeed);

	static INT GetMidiVolume();
	static VOID SetMidiVolume(INT nVol);

	static INT GetWaveVolume();
	static VOID SetWaveVolume(INT nVol);

	static BOOL GetMidi() { return GetMidiVolume() != 0; }
	static BOOL GetDigitalAudio() { return GetWaveVolume() != 0; }

	static VOID MuteToggle();
	static VOID ForcePaintScreen(BOOL bShowCursor = TRUE);

	virtual ERROR_CODE ShowSystemDialog(BOOL bSave = TRUE);
	BOOL ShowRestartDialog(CBofWindow *pWin = nullptr, BOOL bSave = TRUE);
	BOOL ShowSaveDialog(CBofWindow *pWin, BOOL bSave = TRUE);
	BOOL ShowRestoreDialog(CBofWindow *pWin, BOOL bSave = TRUE);
	BOOL ShowQuitDialog(CBofWindow *pWin, BOOL bSave = TRUE);
	ERROR_CODE ShowCreditsDialog(CBofWindow *pWin, BOOL bSave = TRUE);

	VOID FillSaveBuffer(ST_BAGEL_SAVE *pSaveBuf);
	VOID DoRestore(ST_BAGEL_SAVE *pSaveBuf);

	ERROR_CODE NewGame();

	ERROR_CODE LoadFile(const CBofString &sWldName, const CBofString &sStartWldName, BOOL bRestart = FALSE, BOOL bSetStart = TRUE);

	ERROR_CODE SaveFile(const CBofString &sWldName);
	ERROR_CODE LoadFile(bof_ifstream &fpInput, const CBofString &sWldName, BOOL bAttach = FALSE);
	ERROR_CODE LoadGlobalVars(const CBofString &sWldName);

	ERROR_CODE SetCurrFadeIn(int nFade) {
		m_nFadeIn = nFade;
		return ERR_NONE;
	}
	ERROR_CODE SetStorageDev(const CBofString &sWldName, BOOL bEntry = TRUE);
	ERROR_CODE GotoNewWindow(CBofString *pStr);

	USHORT GetDiskID() { return m_nDiskID; }
	VOID SetDiskID(USHORT did) { m_nDiskID = did; }

	CBofWindow *GetCurrentGameWindow() { return (CBofWindow *)m_pGameWindow; }
	CBagStorageDevWnd *GetCurrentStorageDev() { return m_pGameWindow; }
	CBagStorageDevManager *GetStorageDevManager() { return m_pStorageDeviceList; }
	CBagVarManager *GetVariableManager() { return m_pVariableList; }

	virtual CBagStorageDev *OnNewStorageDev(const CBofString &namestr, const CBofString &typestr) = 0;
	virtual CBagStorageDev *OnNewStorageDev(const CBofString &namestr, int nType) = 0;

	virtual VOID OnNewFilter(CBagStorageDev *, const CBofString &typestr) = 0;
	virtual VOID OnNewFilter(CBagStorageDev *pSDev, const int nType) = 0;

	virtual ERROR_CODE OnHelp(const CBofString &sHelpFile, BOOL bSaveBkg = TRUE, CBofWindow *pParent = nullptr);

	virtual VOID OnUserMessage(ULONG nMessage, ULONG lParam);
	VOID OnKeyHit(ULONG lKey, ULONG lRepCount);
	VOID OnClose();

	ST_OBJ *GetObjList() { return m_pObjList; }
	VOID SetSaveObjs(BOOL bSave) { m_bObjSave = bSave; }
	BOOL IsObjSave() { return m_bObjSave; }

	VOID SaveSDevStack();

	virtual VOID *GetDataStart() { return &m_pGameWindow; }
	virtual VOID *GetDataEnd() { return &m_pVariableList + sizeof(CBagVarManager *); }

	// Since we do this from load file and do restore, centralize it in one location.
	VOID RestoreActiveMessages(CBagStorageDevManager *pSDevManager);
};

ERROR_CODE PaintBeveledText(CBofBitmap *pBmp, CBofRect *pRect, const CBofString &cStr, const INT nSize, const INT nWeight, const RGBCOLOR cColor = CTEXT_COLOR, INT nJustify = JUSTIFY_CENTER, UINT nFormat = FORMAT_DEFAULT, INT nFont = FONT_DEFAULT);
ERROR_CODE PaintBeveledText(CBofWindow *pWin, CBofRect *pRect, const CBofString &cStr, const INT nSize, const INT nWeight, const RGBCOLOR cColor = CTEXT_COLOR, INT nJustify = JUSTIFY_CENTER, UINT nFormat = FORMAT_DEFAULT, INT nFont = FONT_DEFAULT);
ERROR_CODE WaitForInput();

} // namespace Bagel

#endif
