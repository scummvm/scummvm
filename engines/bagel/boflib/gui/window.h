
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

#ifndef BAGEL_BOFLIB_GUI_WINDOW_H
#define BAGEL_BOFLIB_GUI_WINDOW_H

#include "common/list.h"
#include "common/events.h"
#include "bagel/boflib/array.h"
#include "bagel/boflib/stdinc.h"
#include "bagel/boflib/error.h"
#include "bagel/boflib/object.h"
#include "bagel/boflib/point.h"
#include "bagel/boflib/rect.h"
#include "bagel/boflib/size.h"
#include "bagel/boflib/timer.h"
#include "bagel/boflib/llist.h"
#include "bagel/boflib/gfx/bitmap.h"
#include "bagel/boflib/gfx/palette.h"
#include "bagel/boflib/gfx/text.h"

namespace Bagel {
class CBofString;

#define MAX_TITLE 64
#define USE_DEFAULT (-1)

class CBofBitmap;
class CBofTimerPacket;

class CBofWindow : public CLList, public CBofObject, public CBofError {
private:
	bool _visible = true;
	bool _enabled = true;
	bool _lockedScreen = true;
	Common::List<WindowTimer> _timers;
	uint32 _lastLButtonTime = 0, _lastRButtonTime = 0;

	/**
	 * Handles translating from a ScummVM event structure to
	 * a code used by the game engine
	 */
	uint32 translateKey(const Common::Event &event) const;

protected:
	/**
	 * Checks window timers for expiry
	 */
	void checkTimers();

public:
	/**
	 * Default constructor
	 */
	CBofWindow();

	/**
	 * Constructor for CBofWindow
	 * @param pszName   Name of window
	 * @param x         X position
	 * @param y         Y position
	 * @param nWidth    Width of window to create (optional)
	 * @param nHeight   Height of window to create (optional)
	 * @param pParent   Parent of this window (optional)
	 */
	CBofWindow(const char *pszName, int x, int y, int nWidth, int nHeight, CBofWindow *pParent);

	/**
	 * Destructor
	 */
	virtual ~CBofWindow();

	static ErrorCode initialize();
	static ErrorCode shutdown();
	static Common::Point getMousePos();

	/**
	 * Creates a window
	 * @param pszName       Name of window
	 * @param x             X position of upper-left corner
	 * @param y             Y position of upper-left corner
	 * @param nWidth        Width of window to create (optional)
	 * @param nHeight       Height of window to create (optional)
	 * @param pParent       Parent of this window (optional)
	 * @param nControlID    User defined ID of this window
	 * @return              Error return code
	 */
	virtual ErrorCode create(const char *pszName, int x, int y, int nWidth, int nHeight, CBofWindow *pParent, uint32 nControlID = 0);

	/**
	 * Creates a window
	 * @param pszName       Name of window
	 * @param pRect         Rectangle for window placement
	 * @param pParent       Parent of this window (optional)
	 * @param nControlID    User defined ID of this window
	 * @return              Error return code
	 */
	virtual ErrorCode create(const char *pszName, CBofRect *pRect, CBofWindow *pParent, uint32 nControlID = 0);

	/**
	 * Destroys the Window attached to this CBofWindow (if any)
	 */
	virtual void destroy();
	virtual void destroyWindow() {
		destroy();
	}

	/**
	 * Shows current window (if hidden)
	 */
	void show();

	void select();

	/**
	 * Hides current window (if shown)
	 */
	void hide();

	/**
	 * Centers current window in parent window or in screen
	 */
	void center();

	/**
	 * Moves current window to specified location in parent
	 * @param x         New upper left corner X position
	 * @param y         New upper left corner Y position
	 * @param bRepaint  true if should update the window
	 */
	void move(int x, int y, bool bRepaint = false);

	/**
	 * Resizes current window to specified area
	 * @param pRect      New area for window
	 * @param bRepaint   Optional repaint after resize
	 */
	void reSize(CBofRect *pRect, bool bRepaint = false);

	virtual ErrorCode close() {
		onClose();

		return ERR_NONE;
	}

	/**
	 * Posts a message
	 * @param nMessage      Message to post
	 * @param lParam1       User info
	 * @param lParam2       More user info
	 */
	void postMessage(uint32 nMessage, uint32 lParam1, uint32 lParam2);

	/**
	 * Posts a user defined message
	 */
	void postUserMessage(uint32 lMessage, uint32 lExtraInfo);

	/**
	 * Sets a timer which calls specified callback (or onTimer)
	 * @param nID           ID of timer to set
	 * @param nInterval     Number of milliseconds till event
	 * @param pCallBack     Function to call when time is up
	 */
	void setTimer(uint32 nID, uint32 nInterval, BofCallback pCallBack = nullptr);

	/**
	 * Stops specified timer
	 * @param nID       ID of timer to stop
	 */
	void killTimer(uint32 nID);

	/**
	 * Stops all timers associated with current window
	 */
	void killMyTimers();

	/**
	 * Returns the parent window element, if any
	*/
	CBofWindow *getParent() const {
		return _parent;
	}

	/**
	 * Causes all parent windows to have valid paint regions
	 * @param pRect         Area to validate
	 */
	void validateAnscestors(CBofRect *pRect = nullptr);

	static CBofWindow *getActiveWindow() {
		return _pActiveWindow;
	}

	void setActive() {
		_pActiveWindow = this;
	}

	static CBofWindow *getWindowList() {
		return _pWindowList;
	}

	CBofRect getWindowRect() const {
		return _cWindowRect;
	}
	CBofRect getClientRect();

	CBofRect getRect() const {
		return _cRect;
	}

	int width() const {
		return _cRect.width();
	}
	int height() const {
		return _cRect.height();
	}

	void screenToClient(CBofPoint *pPoint);

	/**
	 * Selects and Realizes specified palette into current DC
	 * @param pPal      Palette to select
	 */
	void selectPalette(CBofPalette *pPal);

	/**
	 * Associates a new background bitmap to this window
	 * @param pNewBitmap       New background bitmap
	 * @param bRefresh      true if should repaint now
	 * @return              Error return code
	 */
	ErrorCode setBackdrop(CBofBitmap *pNewBitmap, bool bRefresh = false);

	/**
	 * Associates a new background bitmap to this window
	 * @param pszFileName   new background bitmap from file
	 * @param bRefresh      true if should repaint now
	 * @return              Error return code
	 */
	ErrorCode setBackdrop(const char *pszFileName, bool bRefresh = false);

	void clearBackdrop() {
		_pBackdrop = nullptr;
	}

	CBofBitmap *getBackdrop() const {
		return _pBackdrop;
	}

	bool hasBackdrop() const {
		return _pBackdrop != nullptr;
	}

	/**
	 * Deletes the background bitmap associated with this window
	 */
	void killBackdrop();

	/**
	 * Updates the specified section of the background bitmap
	 * @param pRect               Area of bitmap to update on screen
	 * @param nTransparentColor   Color index used for transparency (-1 = none)
	 * @return                    Error return code
	 */
	ErrorCode paintBackdrop(CBofRect *pRect = nullptr, int nTransparentColor = -1);

	void setControlID(uint32 nID) {
		_nID = nID;
	}
	uint32 getControlID() const {
		return _nID;
	}

	void setBkColor(RGBCOLOR cColor) {
		_cBkColor = cColor;
	}
	RGBCOLOR getBkColor() const {
		return _cBkColor;
	}

	void setFgColor(RGBCOLOR cColor) {
		_cFgColor = cColor;
	}
	RGBCOLOR getFgColor() const {
		return _cFgColor;
	}

	void setPrevMouseDown(CBofPoint p) {
		_cPrevMouseDown = p;
	}
	CBofPoint getPrevMouseDown() const {
		return _cPrevMouseDown;
	}

	/**
	 * Sets mouse capture for this window
	 */
	void setCapture();

	/**
	 * Release mouse capture for this window
	 */
	void releaseCapture();

	/**
	 * Returns true if the control is capturing mouse events
	 */
	bool hasCapture() const;

	/**
	 * Sets the focus on a control for keyboard input
	 */
	void setFocus();

	/**
	 * Releases focus from an edit control
	 */
	void releaseFocus();

	/**
	 * Returns true if the control has focus
	 */
	bool hasFocus() const;

	void flushAllMessages();

	/**
	 * Adds specified rectangle to dirty rect list for this window
	 * @param pRect     Rectangle to add to dirty list
	 */
	void validateRect(const CBofRect *pRect);

	/**
	 * Removes specified rectangle from dirty rect for this window
	 * @param pRect     Rectangle to remove from dirty list
	 */
	void invalidateRect(const CBofRect *pRect);

	virtual void onBofButton(CBofObject *pButton, int nExtraInfo);
	virtual void onBofScrollBar(CBofObject *pButton, int nNewPos);
	virtual void onBofListBox(CBofObject *pListBox, int nItemIndex);
	virtual void onMainLoop();

	virtual void onSoundNotify(CBofObject *pObject, uint32 lParam2);
	virtual void onMovieNotify(uint32 lParam1, uint32 lParam2);

	virtual void onMCINotify(uint32 wParam, uint32 lParam);

	virtual void onTimer(uint32 nTimerId);

	/**
	 * Handles a pending ScummVM event
	 * @param event		Event to process
	 */
	virtual void handleEvent(const Common::Event &event);

	Graphics::ManagedSurface *getSurface();

	bool isCreated() const {
		return _surface != nullptr;
	}

	virtual void enable() {
		_enabled = true;
		updateWindow();
	}
	virtual void disable() {
		_enabled = false;
		updateWindow();
	}
	bool isVisible() const {
		return _visible;
	}
	bool isEnabled() const {
		return _enabled;
	}

	void updateWindow();

	void setParent(CBofWindow *parent);

	/**
	 * Handle all pending ScummVM events
	 */
	void handleEvents();

	virtual void onKeyHit(uint32 lKey, uint32 lRepCount);
	void fillWindow(byte iColor);
	void fillRect(CBofRect *pRect, byte iColor);

	ErrorCode paintBeveledText(CBofRect *rect, const CBofString &string, int size, int weight, RGBCOLOR color, int justify, uint32 format);


protected:
	CBofWindow *_parent = nullptr;	// Pointer to parent window
	Array<CBofWindow *> _children;	// Child element pointers

	virtual void onMouseMove(uint32 nFlags, CBofPoint *pPoint, void * = nullptr);
	virtual void onLButtonDown(uint32 nFlags, CBofPoint *pPoint, void * = nullptr);
	virtual void onLButtonUp(uint32 nFlags, CBofPoint *pPoint, void * = nullptr);
	virtual void onLButtonDblClk(uint32 nFlags, CBofPoint *pPoint);

	virtual void onRButtonDown(uint32 nFlags, CBofPoint *pPoint);
	virtual void onRButtonUp(uint32 nFlags, CBofPoint *pPoint);
	virtual void onRButtonDblClk(uint32 nFlags, CBofPoint *pPoint);

	virtual void onReSize(CBofSize *pSize);
	virtual void onPaint(CBofRect *pRect);
	virtual void onClose();

	virtual void onUserMessage(uint32 nMessage, uint32 lParam);

	virtual void onActivate();
	virtual void onDeActivate();

	// Window Data
	char _szTitle[MAX_TITLE] = { 0 };	// Title of window
	CBofRect _cWindowRect;				// Screen based area of this window
	CBofRect _cRect;					// Window-based area of this window
	CBofBitmap *_pBackdrop = nullptr;  // Backdrop bitmap
	uint32 _nID = 0;						// ID of this window

	RGBCOLOR _cBkColor = RGB(255, 255, 255);
	RGBCOLOR _cFgColor = RGB(0, 0, 0);

	bool _bCaptured = false;
	Graphics::ManagedSurface *_surface = nullptr;

	static CBofWindow *_pWindowList;
	static CBofWindow *_pActiveWindow;
	static CBofTimerPacket *_pTimerList;
	CBofPoint _cPrevMouseDown;
	static int _mouseX;
	static int _mouseY;
};

class CBofMessage : public CBofObject {
public:
	CBofWindow *_pWindow; // destination window for message
	uint32 _nMessage;      // message to send (usually WM_USER)
	uint32 _lParam1;       // user defined info
	uint32 _lParam2;       // more user defined info
};

class CBofTimerPacket : public CBofObject, public CLList {
public:
	CBofWindow *_pOwnerWindow;
	BofCallback _pCallBack;
	uint32 _nID;
	uint32 _nInterval;
};

extern CBofWindow *g_hackWindow;

} // namespace Bagel

#endif
