
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
#include "bagel/baglib/save_game_file.h"
#include "bagel/baglib/var.h"

namespace Bagel {

#define g_SDevManager CBagel::getBagApp()->getMasterWnd()->getStorageDevManager()
#define g_VarManager CBagel::getBagApp()->getMasterWnd()->getVariableManager()

class CBagStorageDevManager;
class CBagStorageDevWnd;
class CBagStorageDev;

enum SpaceBarEventType {
	WM_ENTER_PAN_WINDOW     = 1001,
	WM_ENTER_CLOSE_UP_WINDOW = 1002,
	WM_EXIT_CLOSE_UP_WINDOW  = 1003,
	WM_ENTER_NEW_WLD        = 1004,
	WM_DIE                = 1005,
	WM_SHOW_SYSTEM_DLG      = 1006
};

#define MAX_CURSORS 60

// Some utility routines
void setCICStatus(CBagStorageDev *sdev);
bool getCICStatus();

/**
 * This is the main window that is never destroyed throughout the game.
 * It handles bringing new windows in and out of focus as well as
 * containing the parsing file and info.  It also captures
 * keyboard entry and will save state information
 */
class CBagMasterWin : public CBofWindow, public CBagParseObject {
protected:
	static bool _objSaveFl;
	static StObj *_objList;
	static CBagCursor *_cursorList[MAX_CURSORS];

	CBagStorageDevWnd *_gameWindow;

	CBofString _wldScript;
	CBofString _startWld;
	CBofString _cdChangeAudio;
	CBofString _sysScreen;

	uint16 _diskId;
	int _fadeIn;
	CBofList<CBagStorageDev *> *_gameSDevList;
	CBagStorageDevManager *_storageDeviceList;
	CBagVarManager *_variableList; // List of variables used throughout wld
	CBofSound *_waitSound;
	static int _curCursor;

public:
	static int _menuCount;

	CBagMasterWin();
	virtual ~CBagMasterWin();
	virtual ErrorCode Run();

	ErrorCode close() override;

	CBofString &getWldScript() {
		return _wldScript;
	}

	static void setActiveCursor(int cursorId);
	static int getActiveCursor() {
		return _curCursor;
	}

	// User options
	static bool getFlyThru();

	static bool getPanimations();
	static void setPanimations(bool panimsFl);

	static int getCorrection();
	static void setCorrection(int correction);

	static int getPanSpeed();
	static void setPanSpeed(int speed);

	static int getMidiVolume();
	static void setMidiVolume(int vol);

	static int getWaveVolume();
	static void setWaveVolume(int vol);

	static bool getMidi() {
		return getMidiVolume() != 0;
	}
	static bool getDigitalAudio() {
		return getWaveVolume() != 0;
	}

	static void muteToggle();
	static void forcePaintScreen();

	virtual ErrorCode showSystemDialog(bool saveFl = true);
	bool showRestartDialog(CBofWindow *win = nullptr, bool saveFl = true);
	bool showSaveDialog(CBofWindow *win, bool saveFl = true);
	bool showRestoreDialog(CBofWindow *win, bool saveFl = true);
	bool showQuitDialog(CBofWindow *win, bool saveFl = true);
	ErrorCode showCreditsDialog(CBofWindow *win, bool saveFl = true);

	void fillSaveBuffer(StBagelSave *saveBuf);
	void doRestore(StBagelSave *saveBuf);

	ErrorCode newGame();

	ErrorCode loadFile(const CBofString &wldName, const CBofString &startWldName, bool restartFl = false, bool setStartFl = true);

	ErrorCode loadFileFromStream(CBagIfstream &input, const CBofString &wldName);
	ErrorCode loadGlobalVars(const CBofString &wldName);

	ErrorCode setCurrfadeIn(int fade) {
		_fadeIn = fade;
		return ERR_NONE;
	}
	ErrorCode setStorageDev(const CBofString &wldName, bool entry = true);
	ErrorCode gotoNewWindow(const CBofString *str);

	uint16 getDiskID() const {
		return _diskId;
	}
	void setDiskID(uint16 id) {
		_diskId = id;
	}

	CBofWindow *getCurrentGameWindow() const {
		return (CBofWindow *)_gameWindow;
	}
	CBagStorageDevWnd *getCurrentStorageDev() const {
		return _gameWindow;
	}
	CBagStorageDevManager *getStorageDevManager() const {
		return _storageDeviceList;
	}
	CBagVarManager *getVariableManager() const {
		return _variableList;
	}

	virtual CBagStorageDev *onNewStorageDev(const CBofString &typestr) = 0;
	virtual CBagStorageDev *onNewStorageDev(int type) = 0;

	virtual void onNewFilter(CBagStorageDev *, const CBofString &typeStr) = 0;
	virtual void onNewFilter(CBagStorageDev *sdev, int type) = 0;

	virtual ErrorCode onHelp(const CBofString &helpFile, bool saveBkgFl = true, CBofWindow *parent = nullptr);

	void onUserMessage(uint32 message, uint32 param) override;

	void onKeyHit(uint32 keyCode, uint32 repCount) override;
	void onClose() override;

	StObj *getObjList() {
		return _objList;
	}
	void setSaveObjs(bool saveFl) {
		_objSaveFl = saveFl;
	}
	bool isObjSave() {
		return _objSaveFl;
	}

	void saveSDevStack();

	// Since we do this from load file and do restore, centralize it in one location.
	void restoreActiveMessages(CBagStorageDevManager *sdevManager);
};

ErrorCode waitForInput();

extern bool g_waitOKFl;

} // namespace Bagel

#endif
