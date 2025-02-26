
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

#ifndef BAGEL_BOFLIB_APP_H
#define BAGEL_BOFLIB_APP_H

#include "video/video_decoder.h"
#include "bagel/spacebar/boflib/gui/window.h"
#include "bagel/spacebar/boflib/gfx/cursor.h"
#include "bagel/boflib/error.h"
#include "bagel/spacebar/boflib/list.h"

namespace Bagel {
namespace SpaceBar {

#define MAX_APP_NAME 128
#define DEFAULT_MAINLOOPS   1
#define kReallyFastPPC 50
#define kReallySlowPPC 200

class CBofApp : public CBofError {
private:
	CBofWindow *_pWindow = nullptr;
	CBofWindow *_captureControl = nullptr;
	CBofWindow *_focusControl = nullptr;
	Video::VideoDecoder *_consoleVideo = nullptr;

protected:
	void StartupCode();
	void ShutDownCode();

	char _szAppName[MAX_APP_NAME] = { 0 };
	CBofList<CBofCursor> _cCursorList;
	CBofCursor _cDefaultCursor;

	CBofWindow *_pMainWnd = nullptr;
	CBofPalette *_pPalette = nullptr;
	CBofPalette *_pDefPalette = nullptr;
	int _nScreenDX = 0;
	int _nScreenDY = 0;
	int _nColorDepth = 0;

	int _nIterations = DEFAULT_MAINLOOPS;

	static CBofApp *_pBofApp;

	virtual bool shouldQuit() const = 0;

public:
	CBofApp();
	CBofApp(const char *pszAppName);
	virtual ~CBofApp();

	ErrorCode preInit();
	void postShutDown();

	// These functions can be overridden by the child class
	virtual ErrorCode initialize();
	virtual ErrorCode runApp();
	virtual ErrorCode shutdown();

	virtual void setAppName(const char *pszNewAppName) {
		Common::strcpy_s(_szAppName, pszNewAppName);
	}

	const char *getAppName() const {
		return (const char *)_szAppName;
	}

	void setMainWindow(CBofWindow *pWnd) {
		_pMainWnd = pWnd;
	}
	CBofWindow *getMainWindow() const {
		return _pMainWnd;
	}

	CBofWindow *getActualWindow() const {
		return _pWindow;
	}

	void setPalette(CBofPalette *pPalette);

	CBofPalette *getPalette() const {
		return _pPalette;
	}

	int screenWidth() const {
		return _nScreenDX;
	}
	int screenHeight() const {
		return _nScreenDY;
	}
	int screenDepth() const {
		return _nColorDepth;
	}

	CBofCursor getDefaultCursor() const {
		return _cDefaultCursor;
	}
	void setDefaultCursor(CBofCursor &cCursor) {
		_cDefaultCursor = cCursor;
	}

	void addCursor(CBofCursor &cCursor);
	void delCursor(int nIndex);

	CBofCursor getCursor(int nIndex) {
		return _cCursorList[nIndex];
	}
	int getNumberOfCursors() const {
		return _cCursorList.getCount();
	}

	void setCaptureControl(CBofWindow *ctl) {
		_captureControl = ctl;
	}
	CBofWindow *getCaptureControl() const {
		return _captureControl;
	}
	void setFocusControl(CBofWindow *ctl) {
		_focusControl = ctl;
	}
	CBofWindow *getFocusControl() const {
		return _focusControl;
	}
	bool consolePlayVideo(const Common::Path &path);

	static uint32 getMachineSpeed() {
		return kReallyFastPPC;
	}

	static CBofApp *getApp() {
		return _pBofApp;
	}
};

// Global routines
//
void bofMessageBox(const char *pszTitle, const char *pszMessage);

CBofPoint getMousePos();

} // namespace SpaceBar
} // namespace Bagel

#endif
