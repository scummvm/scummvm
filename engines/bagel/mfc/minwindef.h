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

#ifndef BAGEL_MFC_MINWINDEF_H
#define BAGEL_MFC_MINWINDEF_H

#include "common/scummsys.h"
#include "common/algorithm.h"
#include "common/rect.h"
#include "bagel/mfc/winnt.h"

namespace Bagel {
namespace MFC {

#define ASSERT assert

#undef FAR
#undef  NEAR
#define FAR
#define NEAR
#ifndef CONST
	#define CONST const
#endif

#define CALLBACK
#define WINAPI
#define WINAPIV
#define APIENTRY    WINAPI
#define APIPRIVATE
#define PASCAL
#define AFXAPI
#define AFX_DATA
#define AFX_NOVTABLE
#define BASED_CODE
#define AFX_CDECL

// We're temporary disabling virtual Create functions
// for ScummVM due to the warnings it generates
#define CVIRTUAL

#ifndef _In_
	#define _In_
#endif
#ifndef _Out_
	#define _Out_
#endif
#ifndef _In_z_
	#define _In_z_
#endif
#ifndef _Inout_
	#define _Inout_
#endif

// Dummy try/catch implementation
#define TRY if (1) {
#define END_TRY }
#define CATCH(KLASS, VAL) KLASS *VAL = nullptr; if (0)
#define END_CATCH }

DECLARE_HANDLE(HACCEL);
DECLARE_HANDLE(HCURSOR);
DECLARE_HANDLE(HDROP);
DECLARE_HANDLE(HHOOK);
DECLARE_HANDLE(HICON);
DECLARE_HANDLE(HMENU);
DECLARE_HANDLE(HRAWINPUT);

struct CGdiObjectImpl {
	virtual ~CGdiObjectImpl() {};
};
typedef CGdiObjectImpl *HGDIOBJ;
typedef HGDIOBJ HBITMAP;
typedef HGDIOBJ HBRUSH;
typedef HGDIOBJ HFONT;
typedef HGDIOBJ HPALETTE;
typedef HGDIOBJ HPEN;
typedef HGDIOBJ HRGN;
typedef HGDIOBJ HENHMETAFILE;

typedef void *HDC;
class CWnd;
typedef CWnd *HWND;


/* Types use for passing & returning polymorphic values */
typedef UINT_PTR            WPARAM;
typedef LONG_PTR            LPARAM;
typedef LONG_PTR            LRESULT;

typedef LRESULT(CALLBACK *HOOKPROC)(int code, WPARAM wParam, LPARAM lParam);
typedef LRESULT(CALLBACK *WNDPROC)(HWND, UINT, WPARAM, LPARAM);
typedef INT_PTR(FAR WINAPI *FARPROC)();
typedef INT_PTR(NEAR WINAPI *NEARPROC)();
typedef INT_PTR(WINAPI *PROC)();
typedef INT_PTR(CALLBACK *DLGPROC)(HWND, UINT, WPARAM, LPARAM);

#ifndef max
	#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
	#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#define MAKEWORD(a, b)      ((WORD)(((BYTE)(((DWORD_PTR)(a)) & 0xff)) | ((WORD)((BYTE)(((DWORD_PTR)(b)) & 0xff))) << 8))
#define MAKELONG(a, b)      ((LONG)(((WORD)(((DWORD_PTR)(a)) & 0xffff)) | ((DWORD)((WORD)(((DWORD_PTR)(b)) & 0xffff))) << 16))
#define LOWORD(l)           ((WORD)(((DWORD_PTR)(l)) & 0xffff))
#define HIWORD(l)           ((((DWORD_PTR)(l)) >> 16))
#define LOBYTE(w)           ((BYTE)(((DWORD_PTR)(w)) & 0xff))
#define HIBYTE(w)           ((BYTE)((((DWORD_PTR)(w)) >> 8) & 0xff))

#define POINTTOPOINTS(pt)      (MAKELONG((short)((pt).x), (short)((pt).y)))
#define MAKEWPARAM(l, h)      ((WPARAM)(DWORD)MAKELONG(l, h))
#define MAKELPARAM(l, h)      ((LPARAM)(DWORD)MAKELONG(l, h))
#define MAKELRESULT(l, h)     ((LRESULT)(DWORD)MAKELONG(l, h))

typedef void *HANDLE;
typedef HANDLE NEAR *SPHANDLE;
typedef HANDLE FAR *LPHANDLE;
typedef HANDLE HGLOBAL;
typedef HANDLE HLOCAL;
typedef HANDLE GLOBALHANDLE;
typedef HANDLE LOCALHANDLE;

#define DECLARE_HANDLE(name) struct name##__{int unused;}; typedef struct name##__ *name

typedef WORD                ATOM;   //BUGBUG - might want to remove this from minwin

DECLARE_HANDLE(HKEY);
typedef HKEY *PHKEY;
DECLARE_HANDLE(HMETAFILE);
DECLARE_HANDLE(HINSTANCE);
typedef HINSTANCE HMODULE;      /* HMODULEs can be used in place of HINSTANCEs */
DECLARE_HANDLE(HRSRC);
DECLARE_HANDLE(HSPRITE);
DECLARE_HANDLE(HLSURF);
DECLARE_HANDLE(HSTR);
DECLARE_HANDLE(HTASK);
DECLARE_HANDLE(HWINSTA);
DECLARE_HANDLE(HKL);

typedef struct tagPOINT {
	int  x;
	int  y;
} POINT, *PPOINT, NEAR *NPPOINT, FAR *LPPOINT;

typedef struct _POINTL {    /* ptl  */
	LONG  x;
	LONG  y;
} POINTL, *PPOINTL;

typedef struct tagSIZE {
	int        cx;
	int        cy;
} SIZE, *PSIZE, *LPSIZE;

typedef SIZE               SIZEL;
typedef SIZE *PSIZEL, *LPSIZEL;

typedef struct tagPOINTS {
	#ifndef _MAC
	SHORT   x;
	SHORT   y;
	#else
	SHORT   y;
	SHORT   x;
	#endif
} POINTS, *PPOINTS, *LPPOINTS;

typedef struct tagRECT {
	LONG    left = 0;
	LONG    top = 0;
	LONG    right = 0;
	LONG    bottom = 0;

	tagRECT() {}
	tagRECT(LONG x1, LONG y1, LONG x2, LONG y2) :
		left(x1), top(y1), right(x2), bottom(y2) {
	}
	tagRECT(const Common::Rect &r) :
		left(r.left), top(r.top), right(r.right), bottom(r.bottom) {
	}
	operator Common::Rect() const {
		return Common::Rect(left, top, right, bottom);
	}
	bool contains(int x, int y) const {
		Common::Rect r = *this;
		return r.contains(x, y);
	}
	bool contains(const POINT &pt) const {
		Common::Rect r = *this;
		return r.contains(pt.x, pt.y);
	}
} RECT, *PRECT, NEAR *NPRECT, FAR *LPRECT;

typedef const RECT FAR *LPCRECT;

typedef struct _FILETIME {
	DWORD dwLowDateTime;
	DWORD dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME;

typedef struct _SYSTEMTIME {
	WORD wYear;
	WORD wMonth;
	WORD wDayOfWeek;
	WORD wDay;
	WORD wHour;
	WORD wMinute;
	WORD wSecond;
	WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;

typedef struct tagPAINTSTRUCT {
	HDC         hdc;
	BOOL        fErase;
	RECT        rcPaint;
	BOOL        fRestore;
	BOOL        fIncUpdate;
	BYTE        rgbReserved[32];
} PAINTSTRUCT, *PPAINTSTRUCT, *NPPAINTSTRUCT, *LPPAINTSTRUCT;

/*
 * Message structure
 */
typedef struct tagMSG {
	HWND        hwnd = 0;
	UINT        message = 0;
	WPARAM      wParam = 0;
	LPARAM      lParam = 0;
	DWORD       time = 0;
	POINT       pt;

	tagMSG() {
		pt.x = pt.y = 0;
	}
	tagMSG(HWND hwnd_, UINT message_, WPARAM wParam_ = 0,
	       LPARAM lParam_ = 0) :
			hwnd(hwnd_), message(message_), wParam(wParam_),
			lParam(lParam_) {
		pt.x = pt.y = 0;
	}
} MSG, *PMSG, NEAR *NPMSG, FAR *LPMSG;

inline bool PtInRect(const RECT *lprc, POINT &pt) {
	return pt.x >= lprc->left && pt.x < lprc->right &&
	       pt.y >= lprc->top && pt.y < lprc->bottom;
}

typedef struct tagWNDCLASS {
	UINT        style;
	WNDPROC     lpfnWndProc;
	int         cbClsExtra;
	int         cbWndExtra;
	HINSTANCE   hInstance;
	HICON       hIcon;
	HCURSOR     hCursor;
	HBRUSH      hbrBackground;
	LPCSTR      lpszMenuName;
	LPCSTR      lpszClassName;
} WNDCLASS, *PWNDCLASS, *NPWNDCLASS, *LPWNDCLASS;

} // namespace MFC
} // namespace Bagel

#endif
