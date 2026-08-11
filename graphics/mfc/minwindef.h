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

#ifndef GRAPHICS_MFC_MINWINDEF_H
#define GRAPHICS_MFC_MINWINDEF_H

#include "common/scummsys.h"
#include "common/algorithm.h"
#include "common/rect.h"
#define WINVER 0
#include "graphics/mfc/winnt.h"

namespace Graphics {
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
typedef uintptr            WPARAM;
typedef intptr            LPARAM;
typedef intptr            LRESULT;

typedef LRESULT(CALLBACK *HOOKPROC)(int code, WPARAM wParam, LPARAM lParam);
typedef LRESULT(CALLBACK *WNDPROC)(HWND, unsigned int, WPARAM, LPARAM);
typedef intptr(FAR WINAPI *FARPROC)();
typedef intptr(NEAR WINAPI *NEARPROC)();
typedef intptr(WINAPI *PROC)();
typedef intptr(CALLBACK *DLGPROC)(HWND, unsigned int, WPARAM, LPARAM);

#ifndef max
	#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
	#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#define MAKEWORD(a, b)      ((uint16)(((byte)(((uintptr)(a)) & 0xff)) | ((uint16)((byte)(((uintptr)(b)) & 0xff))) << 8))
#define MAKELONG(a, b)      ((long)(((uint16)(((uintptr)(a)) & 0xffff)) | ((uint32)((uint16)(((uintptr)(b)) & 0xffff))) << 16))
#define LOWORD(l)           ((uint16)(((uintptr)(l)) & 0xffff))
#define HIWORD(l)           ((((uintptr)(l)) >> 16))
#define LOBYTE(w)           ((byte)(((uintptr)(w)) & 0xff))
#define HIBYTE(w)           ((byte)((((uintptr)(w)) >> 8) & 0xff))

#define POINTTOPOINTS(pt)      (MAKELONG((short)((pt).x), (short)((pt).y)))
#define MAKEWPARAM(l, h)      ((WPARAM)(uint32)MAKELONG(l, h))
#define MAKELPARAM(l, h)      ((LPARAM)(uint32)MAKELONG(l, h))
#define MAKELRESULT(l, h)     ((LRESULT)(uint32)MAKELONG(l, h))

typedef void *HANDLE;
typedef HANDLE NEAR *SPHANDLE;
typedef HANDLE FAR *LPHANDLE;
typedef HANDLE HGLOBAL;
typedef HANDLE HLOCAL;
typedef HANDLE GLOBALHANDLE;
typedef HANDLE LOCALHANDLE;

#define DECLARE_HANDLE(name) struct name##__{int unused;}; typedef struct name##__ *name

typedef uint16                ATOM;   //BUGBUG - might want to remove this from minwin

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
	int x;
	int y;
} POINT, *PPOINT, NEAR *NPPOINT, FAR *LPPOINT;

typedef struct tagSIZE {
	int cx;
	int cy;
} SIZE, *PSIZE, *LPSIZE;

typedef SIZE SIZEL;
typedef SIZE *PSIZEL, *LPSIZEL;

typedef struct tagPOINTS {
	SHORT x;
	SHORT y;
} POINTS, *PPOINTS, *LPPOINTS;

typedef struct tagRECT {
	int left;
	int top;
	int right;
	int bottom;

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

inline Common::Rect RECTtoRect(const RECT &src) {
	return src;
}
inline RECT RectToRECT(const Common::Rect &src) {
	RECT dest;
	dest.left = src.left;
	dest.top = src.top;
	dest.right = src.right;
	dest.bottom = src.bottom;
	return dest;
}
inline RECT RectToRECT(int x1, int y1, int x2, int y2) {
	RECT dest;
	dest.left = x1;
	dest.top = y1;
	dest.right = x2;
	dest.bottom = y2;
	return dest;
}

inline bool RectsIntersect(const RECT &r1, const RECT &r2) {
	return RECTtoRect(r1).intersects(RECTtoRect(r2));
}

typedef struct _FILETIME {
	uint32 dwLowDateTime;
	uint32 dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME;

typedef struct _SYSTEMTIME {
	uint16 wYear;
	uint16 wMonth;
	uint16 wDayOfWeek;
	uint16 wDay;
	uint16 wHour;
	uint16 wMinute;
	uint16 wSecond;
	uint16 wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;

typedef struct tagPAINTSTRUCT {
	HDC         hdc;
	bool        fErase;
	RECT        rcPaint;
	bool        fRestore;
	bool        fIncUpdate;
	byte        rgbReserved[32];
} PAINTSTRUCT, *PPAINTSTRUCT, *NPPAINTSTRUCT, *LPPAINTSTRUCT;

/*
 * Message structure
 */
typedef struct tagMSG {
	HWND        hwnd = 0;
	unsigned int        message = 0;
	WPARAM      wParam = 0;
	LPARAM      lParam = 0;
	uint32       time = 0;
	POINT       pt;

	// Extra fields for TranslateMessage convenience
	byte        _kbdFlags = 0;
	char        _ascii = 0;

	tagMSG() {
		pt.x = pt.y = 0;
	}
	tagMSG(HWND hwnd_, unsigned int message_, WPARAM wParam_ = 0,
	       LPARAM lParam_ = 0) :
			hwnd(hwnd_), message(message_), wParam(wParam_),
			lParam(lParam_) {
		pt.x = pt.y = 0;
	}
} MSG, *PMSG, NEAR *NPMSG, FAR *LPMSG;

inline bool PtInRect(const RECT *lprc, const POINT &pt) {
	return pt.x >= lprc->left && pt.x < lprc->right &&
	       pt.y >= lprc->top && pt.y < lprc->bottom;
}

typedef struct tagWNDCLASS {
	unsigned int        style;
	WNDPROC     lpfnWndProc;
	int         cbClsExtra;
	int         cbWndExtra;
	HINSTANCE   hInstance;
	HICON       hIcon;
	HCURSOR     hCursor;
	HBRUSH      hbrBackground;
	const char *     lpszMenuName;
	const char *     lpszClassName;
} WNDCLASS, *PWNDCLASS, *NPWNDCLASS, *LPWNDCLASS;

struct CDataExchange {
	bool m_bSaveAndValidate;
};

} // namespace MFC
} // namespace Graphics

#endif
