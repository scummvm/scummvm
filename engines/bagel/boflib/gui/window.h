
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
#include "bagel/boflib/boffo.h"
#include "bagel/boflib/array.h"
#include "bagel/boflib/stdinc.h"
#include "bagel/boflib/error.h"
#include "bagel/boflib/object.h"
#include "bagel/boflib/point.h"
#include "bagel/boflib/rect.h"
#include "bagel/boflib/size.h"
#include "bagel/boflib/timer.h"
#include "bagel/boflib/llist.h"
#include "bagel/boflib/debug.h"
#include "bagel/boflib/gfx/bitmap.h"
#include "bagel/boflib/gfx/palette.h"
#include "bagel/boflib/std_keys.h"

namespace Bagel {

#define MAX_TITLE 64
#define USE_DEFAULT -1

class CBofBitmap;
class CBofTimerPacket;

class CBofWindow : public CLList, public CBofObject, public CBofError {
private:
	bool _visible = true;
	bool _enabled = true;
	Common::List<WindowTimer> _timers;

	/**
	 * Handles traslating from a ScummVM event structure to
	 * a code used by the game engine
	 */
	ULONG TranslateKey(const Common::Event &event) const;

protected:
	/**
	 * Checks window timers for expiry
	 */
	VOID CheckTimers();

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
	 * @paramnHeight    Height of window to create (optional)
	 * @param pParent   Parent of this window (optional)
	 */
	CBofWindow(const CHAR *pszName, INT x = 0, INT y = 0, INT nWidth = USE_DEFAULT, INT nHeight = USE_DEFAULT, CBofWindow *pParent = nullptr);

	/**
	 * Destructor
	 */
	virtual ~CBofWindow();

	static ERROR_CODE initialize();
	static ERROR_CODE shutdown();
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
	ERROR_CODE Create(const CHAR *pszName, INT x = 0, INT y = 0, INT nWidth = USE_DEFAULT, INT nHeight = USE_DEFAULT, CBofWindow *pParent = nullptr, UINT nControlID = 0);

	/**
	 * Creates a window
	 * @param pszName       Name of window
	 * @param pRect         Reactangle for window placement
	 * @param pParent       Parent of this window (optional)
	 * @paramnControlID     User defined ID of this window
	 * @return              Error return code
	 */
	ERROR_CODE Create(const CHAR *pszName, const CBofRect *pRect = nullptr, CBofWindow *pParent = nullptr, UINT nControlID = 0);

	/**
	 * Destroys the Window attached to this CBofWindow (if any)
	 */
	virtual VOID Destroy();
	virtual VOID DestroyWindow() {
		Destroy();
	}

	/**
	 * Shows current window (if hidden)
	 */
	VOID Show();

	VOID Select();

	/**
	 * Hides current window (if shown)
	 */
	VOID Hide();

	/**
	 * Centers current window in parent window or in screen
	 */
	VOID Center();

	/**
	 * Moves current window to specified location in parent
	 * @param x         New upper left corner X position
	 * @param y         New upper left corner Y position
	 * @param bRepaint  TRUE if should update the window
	 */
	VOID Move(const INT x, const INT y, BOOL bRepaint = FALSE);

	/**
	 * Resizes current window to specified area
	 * @param pRect     New area for window
	 * @parambRepaint   Optional repaint after resize
	 */
	VOID ReSize(CBofRect *pRect, BOOL bRepaint = FALSE);

	VOID Close() {
		OnClose();
	}

	/**
	 * Posts a message
	 * @param lMessage      Message to post
	 * @param lParam1       User info
	 * @param lParam2       More user info
	 */
	VOID PostMessage(ULONG nMessage, ULONG lParam1, ULONG lParam2);

	/**
	 * Posts a user defined message
	 */
	VOID PostUserMessage(ULONG nMessage, ULONG lExtraInfo);

	/**
	 * Sets a timer which calls specified callback (or OnTimer)
	 * @param nID           ID of timer to set
	 * @param nInterval     Number of milliseconds till event
	 * @param pCallBack     Function to call when time is up
	 */
	VOID SetTimer(UINT nID, UINT nInterval, BOFCALLBACK pCallBack = nullptr);

	/**
	 * Stops specified timer
	 * @param nID       ID of timer to stop
	 */
	VOID KillTimer(UINT nTimerID);

	/**
	 * Stops all timers associated with current window
	 */
	VOID KillMyTimers();

	/**
	 * Determines if specified window is a child to current window
	 * @param pWnd      Window to check
	 * @return          TRUE if pWnd is a child of current window, FALSE if not
	 */
	BOOL IsChildOf(CBofWindow *pWin);

	/**
	 * Determines if specified window is a parent to current window
	 * @param pWnd      Window to check
	 * @return          TRUE if pWnd is a parent of current window, FALSE if not
	 */
	BOOL IsParentOf(CBofWindow *pWin);

	/**
	 * Returns the parent window element, if any
	*/
	CBofWindow *GetParent() const {
		return _parent;
	}
	CBofWindow *GetAnscestor();

	/**
	 * Causes all parent windows to have valid paint regions
	 * @param pRect         Area to validate
	 */
	VOID ValidateAnscestors(CBofRect *pRect = nullptr);

	static CBofWindow *GetActiveWindow() {
		return m_pActiveWindow;
	}

	VOID SetActive() {
		m_pActiveWindow = this;
	}

	static CBofWindow *GetWindowList() {
		return m_pWindowList;
	}

	CBofRect GetWindowRect() const {
		return m_cWindowRect;
	}
	CBofRect GetClientRect();

	CBofRect GetRect() const {
		return m_cRect;
	}

	INT Width() const {
		return m_cRect.Width();
	}
	INT Height() const {
		return m_cRect.Height();
	}

	VOID ScreenToClient(CBofPoint *pPoint);
	VOID ClientToScreen(CBofPoint *pPoint);

	/**
	 * Selects and Realizes specified palette into current DC
	 * @param pPal      Palette to select
	 */
	VOID SelectPalette(CBofPalette *pPal);

	/**
	 * Associates a new background bitmap to this window
	 * @param pBitmap       New background bitmap
	 * @param bRefresh      TRUE if should repaint now
	 * @return              Error return code
	 */
	ERROR_CODE SetBackdrop(CBofBitmap *pBitmap, BOOL bRefresh = FALSE);

	/**
	 * Associates a new background bitmap to this window
	 * @param pszFileName   new background bitmap from file
	 * @param bRefresh      TRUE if should repaint now
	 * @return              Error return code
	 */
	ERROR_CODE SetBackdrop(const CHAR *pszBmpFile, BOOL bRefresh = FALSE);

	VOID ClearBackdrop() {
		m_pBackdrop = nullptr;
	}

	CBofBitmap *GetBackdrop() {
		return m_pBackdrop;
	}

	BOOL HasBackdrop() {
		return m_pBackdrop != nullptr;
	}

	/**
	 * Deletes the background bitmap associated with this window
	 */
	VOID KillBackdrop();

	/**
	 * Updates the specified section of the background bitmap
	 * @param pRect     Area of bitmap to update on screen
	 * @return          Error return code
	 */
	ERROR_CODE PaintBackdrop(CBofRect *pRect = nullptr, INT nTransparentColor = -1);

	VOID SetControlID(UINT nID) {
		m_nID = nID;
	}
	UINT GetControlID() {
		return m_nID;
	}

	VOID SetBkColor(RGBCOLOR cColor) {
		m_cBkColor = cColor;
	}
	RGBCOLOR GetBkColor() {
		return m_cBkColor;
	}

	VOID SetFgColor(RGBCOLOR cColor) {
		m_cFgColor = cColor;
	}
	RGBCOLOR GetFgColor() {
		return m_cFgColor;
	}

	VOID SetPrevMouseDown(CBofPoint p) {
		m_cPrevMouseDown = p;
	}
	CBofPoint GetPrevMouseDown() {
		return m_cPrevMouseDown;
	}

	/**
	 * Sets mouse capture for this window
	 */
	VOID SetCapture();

	/**
	 * Release mouse capture for this window
	 */
	VOID ReleaseCapture();

	/**
	 * Returns true if the control is capturing mouse events
	 */
	bool HasCapture() const;

	/**
	 * Sets the focus on a control for keyboard input
	 */
	VOID SetFocus();

	/**
	 * Releases focus from an edit control
	 */
	VOID ReleaseFocus();

	/**
	 * Returns true if the control has focus
	 */
	bool HasFocus() const;

	VOID FlushAllMessages();

	/**
	 * Adds specified rectangle to dirty rect list for this window
	 * @param pRect     Rectangle to add to dirty list
	 */
	VOID ValidateRect(const CBofRect *pRect);

	/**
	 * Removes specified rectangle from dirty rect for this window
	 * @param pRect     Rectangle to remove from dirty list
	 */
	VOID InvalidateRect(const CBofRect *pRect);

	virtual VOID OnBofButton(CBofObject *pButton, INT nExtraInfo);
	virtual VOID OnBofScrollBar(CBofObject *pButton, INT nNewPos);
	virtual VOID OnBofListBox(CBofObject *pListBox, INT nItemIndex);
	virtual VOID OnMainLoop();

	virtual VOID OnSoundNotify(CBofObject *pObject, ULONG lParam2);
	virtual VOID OnMovieNotify(ULONG lParam1, ULONG lParam2);

	virtual VOID OnMCINotify(ULONG lParam1, ULONG lParam2);

	virtual VOID OnTimer(UINT nTimerId);

	/**
	 * Handles a pending ScummVM event
	 * @param event		Event to process
	 */
	virtual void handleEvent(const Common::Event &event);

	Graphics::ManagedSurface *getSurface();

	BOOL IsCreated() const {
		return _surface != nullptr;
	}

	virtual VOID Enable() {
		_enabled = true;
		UpdateWindow();
	}
	virtual VOID Disable() {
		_enabled = false;
		UpdateWindow();
	}
	BOOL IsVisible() const {
		return _visible;
	}
	BOOL IsEnabled() const {
		return _enabled;
	}

	VOID UpdateWindow();

	void setParent(CBofWindow *parent);

	/**
	 * Handle all pending ScummVM events
	 */
	void handleEvents();

#if BOF_WINDOWS

	VOID ShowWindow() {
		Show();
	}
	VOID HideWindow() {
		Hide();
	}


	BOOL IsVisible() {
		return ::IsWindowVisible(m_hWnd);
	}
	BOOL IsEnabled() {
		return ::IsWindowEnabled(m_hWnd);
	}
	BOOL IsCreated() {
		return m_hWnd != nullptr;
	}

	HDC GetDC() {
		return ::GetDC(m_hWnd);
	}
	VOID ReleaseDC(HDC hDC) {
		::ReleaseDC(m_hWnd, hDC);
	}

	static CBofWindow *FromHandle(HWND hWnd);
	HWND GetHandle() {
		return m_hWnd;
	}

	virtual LONG OnDefWinProc(UINT nMessage, WPARAM wParam, LPARAM lParam);

	LONG WindowProcedure(UINT nMessage, WPARAM wParam, LPARAM lParam);

	VOID SetFocus() {
		::SetFocus(m_hWnd);
	}

	VOID UpdateWindow() {
		::UpdateWindow(m_hWnd);
	}

#if BOF_WINMAC
	WindowPtr GetMacWindow();
#endif

#elif BOF_MAC
	virtual VOID Enable() {
		m_bEnabled = TRUE;
	}
	virtual VOID Disable() {
		m_bEnabled = FALSE;
	}

	static CBofWindow *FromMacWindow(WindowPtr pWindow);

	/**
	 * Handles specified Event
	 */
	static BOOL HandleMacEvent(EventRecord *pEvent);
	static VOID HandleMacTimers();

	WindowPtr GetMacWindow() {
		return m_pWindow;
	}

	BOOL IsVisible() {
		return m_bVisible;
	}
	BOOL IsEnabled() {
		return m_bEnabled;
	}
	BOOL IsCreated() {
		return m_pWindow != nullptr;
	}

	VOID SetFocus() {
		SetActive();
	}

	VOID UpdateWindow() {
		HandleUpdate();
	}
	VOID SetCustomWindow(BOOL isCustom) {
		m_bCustomMacWindow = isCustom;
	}
	BOOL IsCustomWindow() {
		return m_bCustomMacWindow;
	}
#endif

	virtual VOID OnKeyHit(ULONG lKey, ULONG lRepCount);
	VOID FillWindow(UBYTE iColor);
	VOID FillRect(CBofRect *pRect, UBYTE iColor);

protected:
	CBofWindow *_parent = nullptr;	// Pointer to parent window
	Array<CBofWindow *> _children;	// Child element pointers

	virtual VOID OnMouseMove(UINT nFlags, CBofPoint *pPoint, void * = nullptr);
	virtual VOID OnLButtonDown(UINT nFlags, CBofPoint *pPoint, void * = nullptr);
	virtual VOID OnLButtonUp(UINT nFlags, CBofPoint *pPoint, void * = nullptr);
	virtual VOID OnLButtonDblClk(UINT nFlags, CBofPoint *pPoint);

	virtual VOID OnRButtonDown(UINT nFlags, CBofPoint *pPoint);
	virtual VOID OnRButtonUp(UINT nFlags, CBofPoint *pPoint);
	virtual VOID OnRButtonDblClk(UINT nFlags, CBofPoint *pPoint);

	virtual VOID OnReSize(CBofSize *pSize);
	virtual VOID OnPaint(CBofRect *pRect);
	virtual VOID OnClose();

	virtual VOID OnUserMessage(ULONG nMessage, ULONG lParam);

	virtual VOID OnActivate();
	virtual VOID OnDeActivate();
#if BOF_MAC
	static VOID HandleOSEvt(EventRecord *);
	static BOOL HandleApp3Evt(EventRecord *);
	static VOID HandleActivateEvt(EventRecord *);
	static VOID HandleMouseUp(EventRecord *);
	static VOID HandleMouseDown(EventRecord *);
	static VOID HandleUpdateEvt(EventRecord *);
	static VOID HandleKeyDown(EventRecord *);
#endif

	// Internal routines
	//
#if BOF_WINMAC
	/** Selects and Realizes specified palette into current DC
	 * @param pPal      Palette to select
	 * @return          success/failure
	 */
	BOOL SetMacPalette(CBofPalette *pPalette);
#endif

#if BOF_MAC
	VOID HandleUpdate();
#endif

#if BOF_DEBUG
	VOID CheckTimerID(UINT nID);
#endif

	// Window Data
	//
	CHAR m_szTitle[MAX_TITLE] = { 0 };	// Title of window
	CBofRect m_cWindowRect;				// Screen based area of this window
	CBofRect m_cRect;					// Window-based area of this window
	CBofBitmap *m_pBackdrop = nullptr;  // Backdrop bitmap
	UINT m_nID = 0;						// ID of this window

	RGBCOLOR m_cBkColor = RGB(255, 255, 255);
	RGBCOLOR m_cFgColor = RGB(0, 0, 0);

	BOOL m_bCaptured = FALSE;
	Graphics::ManagedSurface *_surface = nullptr;

	static CBofWindow *m_pWindowList;
	static CBofWindow *m_pActiveWindow;
	static CBofTimerPacket *m_pTimerList;
	CBofPoint m_cPrevMouseDown;
	static int _mouseX;
	static int _mouseY;

#if BOF_WINDOWS
	HWND m_hWnd;
	HWND m_hLastCapture;
	static HBRUSH m_hBrush;

#elif BOF_MAC
	static CBofWindow *m_pCapturedWindow;
	CBofWindow *m_pPrevActiveWindow;
	CBofWindow *m_pLastCapture;
	WindowPtr m_pWindow;
	BOOL m_bEnabled;
	BOOL m_bVisible;
	BOOL m_bCustomMacWindow;

public:
#if PALETTESHIFTFIX
	static CBofList<PaletteShiftItem> *m_pPaletteShiftList;
	static VOID AddToPaletteShiftList(ITEMTYPE inItemID, LONG inItemOfInterest, LONG inAssociatedItem = 0);
	static VOID CheckPaletteShiftList();
#endif
#endif
};

class CBofMessage : public CBofObject {
public:
	CBofWindow *m_pWindow; // destination window for message
	ULONG m_nMessage;      // message to send (usually WM_USER)
	ULONG m_lParam1;       // user defined info
	ULONG m_lParam2;       // more user defined info
};

class CBofTimerPacket : public CBofObject, public CLList {
public:
	CBofWindow *m_pOwnerWindow;
	BOFCALLBACK m_pCallBack;
	UINT m_nID;
	UINT m_nInterval;

#if BOF_MAC
	ULONG m_lLastTime;
#endif
};

#if BOF_MAC
class STBofScreen {
public:
	STBofScreen(Rect *);
	~STBofScreen();

private:
	Rect m_screenRect;
};

// preserve the mac port.

class STBofPort {
public:
	STBofPort(WindowPtr);
	~STBofPort();

private:
	short m_nCheckCookie;
	Boolean m_bNewPort;
	WindowPtr m_pSavePort;
};

#endif

class STBofFont {
public:
	STBofFont(INT);
	~STBofFont();

private:
	INT m_nSaveFont;
};

// Global Routines
#if BOF_MAC || BOF_WINMAC
// VOID    SetPaintWhite(BOOL bWhite);
#endif

extern CBofWindow *g_pHackWindow;

} // namespace Bagel

#endif
