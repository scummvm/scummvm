
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

#ifndef BAGEL_BOFLIB_BOFF_APP_H
#define BAGEL_BOFLIB_BOFF_APP_H

#include "bagel/boflib/boffo.h"
#include "bagel/boflib/gui/window.h"
#include "bagel/boflib/gfx/cursor.h"
#include "bagel/boflib/bof_error.h"
#include "bagel/boflib/bof_list.h"

namespace Bagel {

#define MAX_APPNAME 128

class CBofApp : public CBofError {
private:
	VOID ConstructorInits(VOID);

protected:
#if BOF_MAC
	VOID HandleMacEvent(EventRecord *pEvent);
	VOID InitMacToolBox(VOID);
#if USEDRAWSPROCKET
	VOID InitDrawSprocket(VOID);
#endif
#endif

	VOID StartupCode(VOID);
	VOID ShutDownCode(VOID);

	CHAR m_szAppName[MAX_APPNAME];

#if !BOF_WIN16
	CBofList<CBofCursor> m_cCursorList;
#endif

	CBofCursor m_cDefaultCursor;
	CBofWindow *m_pMainWnd;
	CBofPalette *m_pPalette;
	CBofPalette *m_pDefPalette;
	INT m_nScreenDX;
	INT m_nScreenDY;
	INT m_nColorDepth;

	INT m_nIterations;

	static CBofApp *m_pBofApp;
	static ULONG m_lCPUSpeed;

#if BOF_WINDOWS
	static HINSTANCE m_hInstance;
	HDIGDRIVER m_hDriver;
	HMDIDRIVER m_hMidiDriver;
#elif BOF_MAC
	BOOL m_bQuit;
	// scg 01.20.97
	ULONG m_nextWNETime;
	BOOL m_bSuspended;
	Point m_prevMouse;
#endif

private:
	CBofWindow *m_pWindow;

public:
	CBofApp(VOID);
	CBofApp(const CHAR *pszAppName);
	virtual ~CBofApp();

	ERROR_CODE PreInit(VOID);
	ERROR_CODE PreShutDown(VOID); // scg 01.13.97
	ERROR_CODE PostShutDown(VOID);

	// these functions can be overridden by the child class
	//
	virtual ERROR_CODE Initialize(VOID);
	virtual ERROR_CODE RunApp(VOID);
	virtual ERROR_CODE ShutDown(VOID);

	VOID SetAppName(const CHAR *pszNewAppName) { Common::strcpy_s(m_szAppName, pszNewAppName); }

	const CHAR *GetAppName(VOID) { return ((const CHAR *)m_szAppName); }

	VOID SetMainWindow(CBofWindow *pWnd) { m_pMainWnd = pWnd; }
	CBofWindow *GetMainWindow(VOID) { return (m_pMainWnd); }

	CBofWindow *GetActualWindow(VOID) { return (m_pWindow); }

	VOID SetPalette(CBofPalette *pPalette);
	CBofPalette *GetPalette(VOID) { return (m_pPalette); }

	INT ScreenWidth(VOID) { return (m_nScreenDX); }
	INT ScreenHeight(VOID) { return (m_nScreenDY); }
	INT ScreenDepth(VOID) { return (m_nColorDepth); }

	CBofCursor GetDefaultCursor(VOID) { return (m_cDefaultCursor); }
	VOID SetDefaultCursor(CBofCursor &cCursor) { m_cDefaultCursor = cCursor; }

#if !BOF_WIN16
	VOID AddCursor(CBofCursor &cCursor);
	VOID DelCursor(INT nIndex);
	CBofCursor GetCursor(INT nIndex) { return (m_cCursorList[nIndex]); }
	INT GetNumberOfCursors(VOID) { return (m_cCursorList.GetCount()); }
#endif

	static CBofApp *GetApp(VOID) { return (m_pBofApp); }
	static ULONG GetMachineSpeed(VOID) { return (m_lCPUSpeed); }
	static VOID CalcCPUSpeed(VOID);

#if BOF_WINDOWS
	HDIGDRIVER GetDriver(VOID) { return (m_hDriver); }
	HMDIDRIVER GetMidiDriver(VOID) { return (m_hMidiDriver); }

	static VOID SetInstanceHandle(HINSTANCE hInstance) { m_hInstance = hInstance; }

	static HINSTANCE GetInstanceHandle(VOID) { return (m_hInstance); }
#elif BOF_MAC

#endif
};

// The actual window that is used as our application
//
class CBofAppWindow : public CBofWindow {
protected:
	virtual VOID OnPaint(CBofRect *pRect);
	virtual VOID OnKeyHit(ULONG lKey, ULONG lRepCount);

	virtual VOID OnMouseMove(UINT nFlags, CBofPoint *pPoint);

	virtual VOID OnLButtonDown(UINT nFlags, CBofPoint *pPoint);
	virtual VOID OnLButtonUp(UINT nFlags, CBofPoint *pPoint);
	virtual VOID OnLButtonDblClk(UINT nFlags, CBofPoint *pPoint);

	virtual VOID OnRButtonDown(UINT nFlags, CBofPoint *pPoint);
	virtual VOID OnRButtonUp(UINT nFlags, CBofPoint *pPoint);
	virtual VOID OnRButtonDblClk(UINT nFlags, CBofPoint *pPoint);
};

// global routines
//
VOID BofPostMessage(CBofWindow *pWindow, ULONG lMessage, ULONG lParam1, ULONG lParam2);
VOID BofMessageBox(const CHAR *pszTitle, const CHAR *pszMessage);

#if BOF_WINDOWS
#define Quit() PostQuitMessage(0);
#else
#define Quit() BofPostMessage(NULL, BM_QUIT, 0, 0);
#endif

VOID SetMousePos(CBofPoint &cPoint);
CBofPoint GetMousePos(VOID);

} // namespace Bagel

#endif
