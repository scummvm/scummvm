
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

#ifndef BAGEL_BOFLIB_GFX_BOF_WINDOW_H
#define BAGEL_BOFLIB_GFX_BOF_WINDOW_H

#include "bagel/boflib/boffo.h"
#include "bagel/boflib/bof_error.h"
#include "bagel/boflib/bof_object.h"
#include "bagel/boflib/point.h"
#include "bagel/boflib/rect.h"
#include "bagel/boflib/size.h"
#include "bagel/boflib/llist.h"
#include "bagel/boflib/bof_debug.h"
#include "bagel/boflib/gfx/bof_bitmap.h"
#include "bagel/boflib/gfx/bof_palette.h"
#include "bagel/boflib/std_keys.h"

namespace Bagel {

#define MAX_TITLE 64
#define USE_DEFAULT -1

class CBofBitmap;
class CBofTimerPacket;

class CBofWindow : public CLList, public CBofObject, public CBofError {

public:
	CBofWindow(VOID);
	CBofWindow(const CHAR *pszName, INT x = 0, INT y = 0, INT nWidth = USE_DEFAULT, INT nHeight = USE_DEFAULT, CBofWindow *pParent = NULL);
	virtual ~CBofWindow();

	static ERROR_CODE Initialize(VOID);
	static ERROR_CODE ShutDown(VOID);

	ERROR_CODE Create(const CHAR *pszName, INT x = 0, INT y = 0, INT nWidth = USE_DEFAULT, INT nHeight = USE_DEFAULT, CBofWindow *pParent = NULL, UINT nControlID = 0);
	ERROR_CODE Create(const CHAR *pszName, CBofRect *pRect = NULL, CBofWindow *pParent = NULL, UINT nControlID = 0);

	virtual VOID Destroy(VOID);
	virtual VOID DestroyWindow(VOID) { Destroy(); }

	VOID Show(VOID);
	VOID Select(VOID);
	VOID Hide(VOID);

	VOID Center(VOID);
	VOID Move(const INT x, const INT y, BOOL bRepaint = FALSE);
	VOID ReSize(CBofRect *pRect, BOOL bRepaint = FALSE);

	VOID Close(VOID) { /*PostMessage(BM_CLOSE, 0, 0);*/ }

	VOID PostMessage(ULONG nMessage, ULONG lParam1, ULONG lParam2);
	VOID PostUserMessage(ULONG nMessage, ULONG lExtraInfo);

	VOID SetTimer(UINT nID, UINT nInterval, BOFCALLBACK pCallBack = NULL);
	VOID KillTimer(UINT nTimerID);
	VOID KillMyTimers(VOID);

	BOOL IsChildOf(CBofWindow *pWin);
	BOOL IsParentOf(CBofWindow *pWin);

	CBofWindow *GetParent(VOID) { return (m_pParentWnd); }
	CBofWindow *GetAnscestor(VOID);

	VOID ValidateAnscestors(CBofRect *pRect = NULL);

	static CBofWindow *GetActiveWindow(VOID) { return (m_pActiveWindow); }
#if BOF_MAC
	// jwl 08.23.96 mac stuff is in cbofwin.cpp
	BOOL IsInActiveList(VOID);
	VOID SetActive(VOID);
	VOID RemoveFromActiveList(VOID);
#else
	VOID SetActive(VOID) { m_pActiveWindow = this; }
#endif

	static CBofWindow *GetWindowList(VOID) { return (m_pWindowList); }

	CBofRect GetWindowRect(VOID) { return (m_cWindowRect); }
	CBofRect GetClientRect(VOID);

	CBofRect GetRect(VOID) { return (m_cRect); }

	INT Width(VOID) { return (m_cRect.Width()); }
	INT Height(VOID) { return (m_cRect.Height()); }

	VOID ScreenToClient(CBofPoint *pPoint);
	VOID ClientToScreen(CBofPoint *pPoint);

	VOID SelectPalette(CBofPalette *pPal);

	ERROR_CODE SetBackdrop(CBofBitmap *pBitmap, BOOL bRefresh = FALSE);
	ERROR_CODE SetBackdrop(const CHAR *pszBmpFile, BOOL bRefresh = FALSE);

	// jwl 09.12.96 used to clear backdrop when we "borrow" one from another object.
	VOID ClearBackdrop(VOID) { m_pBackdrop = NULL; }

	CBofBitmap *GetBackdrop(VOID) { return (m_pBackdrop); }

	BOOL HasBackdrop(VOID) { return (m_pBackdrop != NULL); }

	VOID KillBackdrop(VOID);
	ERROR_CODE PaintBackdrop(CBofRect *pRect = NULL, INT nTransparentColor = -1);

	VOID SetControlID(UINT nID) { m_nID = nID; }
	UINT GetControlID(VOID) { return (m_nID); }

	VOID SetBkColor(RGBCOLOR cColor) { m_cBkColor = cColor; }
	RGBCOLOR GetBkColor(VOID) { return (m_cBkColor); }

	VOID SetFgColor(RGBCOLOR cColor) { m_cFgColor = cColor; }
	RGBCOLOR GetFgColor(VOID) { return (m_cFgColor); }

	VOID SetPrevMouseDown(CBofPoint p) { m_cPrevMouseDown = p; }
	CBofPoint GetPrevMouseDown(VOID) { return (m_cPrevMouseDown); }

	VOID SetCapture(VOID);
	VOID ReleaseCapture(VOID);

	VOID FlushAllMessages(VOID);

	VOID ValidateRect(CBofRect *pRect);
	VOID InvalidateRect(CBofRect *pRect);

	virtual VOID OnBofButton(CBofObject *pButton, INT nExtraInfo);
	virtual VOID OnBofScrollBar(CBofObject *pButton, INT nNewPos);
	virtual VOID OnBofListBox(CBofObject *pListBox, INT nItemIndex);
	virtual VOID OnMainLoop(VOID);

	virtual VOID OnSoundNotify(CBofObject *pObject, ULONG lParam2);
	virtual VOID OnMovieNotify(ULONG lParam1, ULONG lParam2);

	virtual VOID OnMCINotify(ULONG lParam1, ULONG lParam2);

	virtual VOID OnTimer(UINT nTimerId);

	Graphics::ManagedSurface *getSurface();

#if BOF_WINDOWS

	VOID ShowWindow(VOID) { Show(); }
	VOID HideWindow(VOID) { Hide(); }

	virtual VOID Enable(VOID) { ::EnableWindow(m_hWnd, TRUE); }
	virtual VOID Disable(VOID) { ::EnableWindow(m_hWnd, FALSE); }

	BOOL IsVisible(VOID) { return (::IsWindowVisible(m_hWnd)); }
	BOOL IsEnabled(VOID) { return (::IsWindowEnabled(m_hWnd)); }
	BOOL IsCreated(VOID) { return (m_hWnd != NULL); }

	HDC GetDC(VOID) { return ::GetDC(m_hWnd); }
	VOID ReleaseDC(HDC hDC) { ::ReleaseDC(m_hWnd, hDC); }

	static CBofWindow *FromHandle(HWND hWnd);
	HWND GetHandle(VOID) { return (m_hWnd); }

	virtual LONG OnDefWinProc(UINT nMessage, WPARAM wParam, LPARAM lParam);

	LONG WindowProcedure(UINT nMessage, WPARAM wParam, LPARAM lParam);

	VOID SetFocus(VOID) { ::SetFocus(m_hWnd); }

	VOID UpdateWindow(VOID) { ::UpdateWindow(m_hWnd); }

#if BOF_WINMAC
	WindowPtr GetMacWindow(VOID);
#endif

#elif BOF_MAC
	virtual VOID Enable(VOID) { m_bEnabled = TRUE; }
	virtual VOID Disable(VOID) { m_bEnabled = FALSE; }

	static CBofWindow *FromMacWindow(WindowPtr pWindow);
	static BOOL HandleMacEvent(EventRecord *pEvent);
	static VOID HandleMacTimers(VOID);

	WindowPtr GetMacWindow(VOID) { return (m_pWindow); }

	BOOL IsVisible(VOID) { return (m_bVisible); }
	BOOL IsEnabled(VOID) { return (m_bEnabled); }
	BOOL IsCreated(VOID) { return (m_pWindow != NULL); }

	VOID SetFocus(VOID) { SetActive(); }

	VOID UpdateWindow(VOID) { HandleUpdate(); }
	VOID SetCustomWindow(BOOL isCustom) { m_bCustomMacWindow = isCustom; }
	BOOL IsCustomWindow(VOID) { return m_bCustomMacWindow; }
#endif

	virtual VOID OnKeyHit(ULONG lKey, ULONG lRepCount);
	VOID FillWindow(UBYTE iColor);
	VOID FillRect(CBofRect *pRect, UBYTE iColor);

protected:
	virtual VOID OnMouseMove(UINT nFlags, CBofPoint *pPoint);
	virtual VOID OnLButtonDown(UINT nFlags, CBofPoint *pPoint);
	virtual VOID OnLButtonUp(UINT nFlags, CBofPoint *pPoint);
	virtual VOID OnLButtonDblClk(UINT nFlags, CBofPoint *pPoint);

	virtual VOID OnRButtonDown(UINT nFlags, CBofPoint *pPoint);
	virtual VOID OnRButtonUp(UINT nFlags, CBofPoint *pPoint);
	virtual VOID OnRButtonDblClk(UINT nFlags, CBofPoint *pPoint);

	virtual VOID OnReSize(CBofSize *pSize);
	virtual VOID OnPaint(CBofRect *pRect);
	virtual VOID OnClose(VOID);

	virtual VOID OnCommand(ULONG lParam1, ULONG lParam2);
	virtual VOID OnUserMessage(ULONG nMessage, ULONG lParam);

	virtual VOID OnActivate(VOID);
	virtual VOID OnDeActivate(VOID);
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
	BOOL SetMacPalette(CBofPalette *pPalette);
#endif

#if BOF_MAC
	VOID HandleUpdate(VOID);
#endif

#if BOF_DEBUG
	VOID CheckTimerID(UINT nID);
#endif

	// Window Data
	//
	CHAR m_szTitle[MAX_TITLE]; // Title of window
	CBofRect m_cWindowRect;    // Screen based area of this window
	CBofRect m_cRect;          // Window-based area of this window
	CBofBitmap *m_pBackdrop;   // Backdrop bitmap
	CBofWindow *m_pParentWnd;  // Pointer to parent window
	UINT m_nID;                // ID of this window

	RGBCOLOR m_cBkColor;
	RGBCOLOR m_cFgColor;

	BOOL m_bCaptured;

	static CBofWindow *m_pWindowList;
	static CBofWindow *m_pActiveWindow;
	static CBofTimerPacket *m_pTimerList;
	CBofPoint m_cPrevMouseDown; // jwl 08.29.96 need to save (local) mouse coords

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
	static VOID CheckPaletteShiftList(VOID);
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

// jwl 07.15.96 really hacky stuff to prevent onscreen window border
// drawing from taking place.
#if BOF_MAC
class STBofScreen {
public:
	STBofScreen(Rect *);
	~STBofScreen();

private:
	Rect m_screenRect;
};

// jwl 08.07.96 preserve the mac port.

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
// VOID    SetPaintWhite(BOOL bWhite); // scg 01.24.97 use LMSetPaintWhite instead
#endif

} // namespace Bagel

#endif
