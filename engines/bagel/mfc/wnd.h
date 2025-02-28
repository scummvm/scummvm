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

#ifndef BAGEL_MFC_WND_H
#define BAGEL_MFC_WND_H

#include "graphics/managed_surface.h"
#include "bagel/mfc/mfc_types.h"
#include "bagel/mfc/object.h"
#include "bagel/mfc/dc.h"
#include "bagel/mfc/rect.h"
#include "bagel/mfc/str.h"

namespace Bagel {
namespace MFC {

#define ODS_SELECTED    0x0001  // Item is selected (highlighted).
#define ODS_GRAYED      0x0002  // Item is grayed (disabled).
#define ODS_DISABLED    0x0004  // Item is disabled (similar to grayed but distinct in behavior).
#define ODS_CHECKED     0x0008  // Item is checked (for checkable menu items).
#define ODS_FOCUS       0x0010  // Item has keyboard focus.
#define ODS_DEFAULT     0x0020  // Item is the default action.
#define ODS_COMBOBOXEDIT 0x1000 // Item is in a combo box's edit control.
#define ODS_HOTLIGHT    0x0040  // Item is being hovered over (hot-tracked).
#define ODS_INACTIVE    0x0080  // Item is inactive.
#define ODS_NOACCEL     0x0100  // Draw without underlining keyboard accelerators.
#define ODS_NOFOCUSRECT 0x0200  // Do not draw a focus rectangle.

enum {
	WM_NULL            = 0x0000,
	WM_CREATE          = 0x0001,
	WM_DESTROY         = 0x0002,
	WM_MOVE            = 0x0003,
	WM_SIZE            = 0x0005,
	WM_ACTIVATE        = 0x0006,
	WM_SETFOCUS        = 0x0007,
	WM_KILLFOCUS       = 0x0008,
	WM_ENABLE          = 0x000A,
	WM_SETREDRAW       = 0x000B,
	WM_SETTEXT         = 0x000C,
	WM_GETTEXT         = 0x000D,
	WM_GETTEXTLENGTH   = 0x000E,
	WM_PAINT           = 0x000F,
	WM_CLOSE           = 0x0010,
	WM_QUERYENDSESSION = 0x0011,
	WM_QUERYOPEN       = 0x0013,
	WM_ENDSESSION      = 0x0016,
	WM_QUIT            = 0x0012,
	WM_ERASEBKGND      = 0x0014,
	WM_SYSCOLORCHANGE  = 0x0015,
	WM_SHOWWINDOW      = 0x0018,
	WM_WININICHANGE    = 0x001A,

	WM_CTLCOLORMSGBOX     = 0x0132,
	WM_CTLCOLOREDIT       = 0x0133,
	WM_CTLCOLORLISTBOX    = 0x0134,
	WM_CTLCOLORBTN        = 0x0135,
	WM_CTLCOLORDLG        = 0x0136,
	WM_CTLCOLORSCROLLBAR  = 0x0137,
	WM_CTLCOLORSTATIC     = 0x0138,
	MN_GETHMENU           = 0x01E1,
	WM_MOUSEFIRST         = 0x0200,
	WM_MOUSEMOVE          = 0x0200,
	WM_LBUTTONDOWN        = 0x0201,
	WM_LBUTTONUP          = 0x0202,
	WM_LBUTTONDBLCLK      = 0x0203,
	WM_RBUTTONDOWN        = 0x0204,
	WM_RBUTTONUP          = 0x0205,
	WM_RBUTTONDBLCLK      = 0x0206,
	WM_MBUTTONDOWN        = 0x0207,
	WM_MBUTTONUP          = 0x0208,
	WM_MBUTTONDBLCLK      = 0x0209,
	WM_MOUSEWHEEL         = 0x020A,
	WM_XBUTTONDOWN        = 0x020B,
	WM_XBUTTONUP          = 0x020C,
	WM_XBUTTONDBLCLK      = 0x020D,
	WM_MOUSEHWHEEL        = 0x020E,
	WM_MOUSELAST          = 0x020E
};

enum {
	MK_LBUTTON  = 0x0001,
	MK_RBUTTON  = 0x0002,
	MK_SHIFT    = 0x0004,
	MK_CONTROL  = 0x0008,
	MK_MBUTTON  = 0x0010,
	MK_XBUTTON1 = 0x0020,
	MK_XBUTTON2 = 0x0040
};

enum {
	PM_NOREMOVE = 0x0000,
	PM_REMOVE   = 0x0001,
	PM_NOYIELD  = 0x0002
};

enum {
	CS_BYTEALIGNWINDOW,
	CS_OWNDC
};
enum {
	WS_POPUP,
	SW_SHOWNORMAL
};
enum {
	WM_COMMAND
};
enum {
	BN_CLICKED
};

class CScrollBar;
typedef void *TIMERPROC;
typedef void *HINSTANCE;
typedef void *HMENU;

typedef struct tagCREATESTRUCT {
	LPVOID    lpCreateParams;
	HINSTANCE hInstance;
	HMENU     hMenu;
	HWND      hwndParent;
	int       cy;
	int       cx;
	int       y;
	int       x;
	LONG      style;
	LPCSTR    lpszName;
	LPCSTR    lpszClass;
	DWORD     dwExStyle;
} CREATESTRUCT, *LPCREATESTRUCT;

typedef struct tagMSG {
	HWND   hwnd;    // Handle to the window that received the message
	UINT   message; // Message identifier (e.g., WM_PAINT, WM_KEYDOWN)
	WPARAM wParam;  // Additional message-specific information
	LPARAM lParam;  // Additional message-specific information
	DWORD  time;    // Timestamp of when the message was posted
	POINT  pt;      // Mouse cursor position when the message was posted
} MSG, *PMSG, *LPMSG;

class CWnd;
typedef CWnd *HWND;

class CWnd : public CObject {
protected:
	HWND m_hWnd;
	CWnd *m_pParentWnd = nullptr;
	Common::Rect _bounds;
	CString _text;
	uint _tag = 0;
	bool _visible = true;

	virtual bool OnCommand(uint16 wParam, int32 lParam) {
		return false;
	}

	//{{AFX_MSG( CMainWindow )
	virtual void OnPaint() {
	}
	virtual void OnChar(uint nChar, uint nRepCnt, uint nFlags) {
	}
	virtual void OnSysChar(uint nChar, uint nRepCnt, uint nFlags) {
	}
	virtual void OnSysKeyDown(uint nChar, uint nRepCnt, uint nFlags) {
	}
	virtual void OnKeyDown(uint nChar, uint nRepCnt, uint nFlags) {
	}
	virtual void OnTimer(uint nIDEvent) {
	}
	virtual void OnLButtonDown(uint nFlags, CPoint point) {
	}
	virtual void OnRButtonDown(uint nFlags, CPoint point) {
	}
	virtual void OnLButtonUp(uint nFlags, CPoint point) {
	}
	virtual void OnMouseMove(uint nFlags, CPoint point) {
	}
	virtual void OnClose() {
	}
	virtual long OnMCINotify(uint16, int32) {
		return 0;
	}
	virtual long OnMMIONotify(uint16, int32) {
		return 0;
	}

	// Dummy functions
	void BeginWaitCursor() {
	}
	void EndWaitCursor() {
	}

	CString AfxRegisterWndClass(int flags, void *, void *, void *) const {
		return CString();
	}

public:
	static CWnd *FromHandlePermanent(HWND hWnd);

public:
	virtual ~CWnd() {
	}

	bool Create(const CString &label, uint flags, const CRect &bounds,
		CWnd *parent, uint tag) {
		_text = label;
		_bounds = bounds;
		_tag = tag;
		return true;
	}

	CDC *GetDC();
	static void ReleaseDC(CDC *dc);

	void GetClientRect(CRect &r) const {
		r = _bounds;
	}
	void GetWindowRect(LPRECT lpRect) const {
		*lpRect = _bounds;
	}
	void InvalidateRect(const CRect *r, bool bErase = true);

	bool PaintDIB(HDC, CRect *lpDestRect, HDIB hSrc,
		CRect *lpSrcRect, CPalette *hPal);

	virtual void SetRect(int x1, int y1, int x2, int y2) {
		_bounds = Common::Rect(x1, y1, x2, y2);
	}

#ifdef _DEBUG
	virtual void AssertValid() const {
	}
	virtual void Dump(CDumpContext &dc) const {
	}
#endif //_DEBUG

	CWnd *GetParent() const {
		return m_pParentWnd;
	}
	bool IsWindowVisible() const {
		return _visible;
	}
	void GetWindowText(CString &text) const {
		text = _text;
	}
	int GetWindowText(char *text, int max) const {
		Common::strcpy_s(text, max, _text.c_str());
		return strlen(text);
	}

	BOOL SubclassDlgItem(UINT nID, CWnd *pParent);
	void ShowWindow(int);
	void UpdateWindow();
	void SetActiveWindow();
	CWnd *GetNextDlgGroupItem(CWnd *pWndCtl, BOOL bPrevious) const;
	BOOL ValidateRect(LPCRECT lpRect = NULL);
	BOOL GetUpdateRect(LPRECT lpRect, BOOL bErase = FALSE);
	void MoveWindow(int x, int y, int nWidth, int nHeight, BOOL bRepaint = TRUE);
	void MoveWindow(LPCRECT lpRect, BOOL bRepaint = TRUE);
	CWnd *SetFocus();

	long SendMessage(UINT message, WPARAM wParam = 0, LPARAM lParam = 0);
	long SetTimer(UINT nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc);
	bool KillTimer(UINT nIDEvent);
	BOOL PostMessage(UINT message, WPARAM wParam = 0, LPARAM lParam = 0);
	LRESULT SendDlgItemMessage(int nID, UINT message, WPARAM wParam = 0, LPARAM lParam = 0) const;
	BOOL PeekMessage(
		LPMSG lpMsg,       // Pointer to an MSG structure
		HWND hWnd,         // Handle to window (or NULL for all windows)
		UINT wMsgFilterMin,// Minimum message filter
		UINT wMsgFilterMax,// Maximum message filter
		UINT wRemoveMsg    // Flags: PM_NOREMOVE, PM_REMOVE, etc.
	);

	virtual int OnCreate(LPCREATESTRUCT lpCreateStruct) { return 0; }
	virtual void OnDestroy() {}
	virtual void OnShowWindow(BOOL bShow, UINT nStatus) {}
	virtual void OnSize(UINT nType, int cx, int cy) {}
	virtual void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar) {}
};

class CFrameWnd : public CWnd {
public:
	~CFrameWnd() override {
	}

	bool Create(const CString &wndClass, const CString &label, uint flags,
			const CRect &bounds, CWnd *, void *) {
		return CWnd::Create(label, flags, bounds, nullptr, 0);
	}

};

class CDialog : public CWnd {
public:
	CDialog(UINT nIDTemplate, CWnd *pParent = NULL);
	CDialog(LPCTSTR lpszTemplateName, CWnd *pParent = NULL);

	int DoModal();
	void EndDialog(int nResult);

	void OnInitDialog();
	void OnCancel();
	void DDX_Control(CDataExchange *pDX, int nIDC, CWnd &rControl);

	virtual void DoDataExchange(CDataExchange *pDX);

};

} // namespace MFC
} // namespace Bagel

#endif
