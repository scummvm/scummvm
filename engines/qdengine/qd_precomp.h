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

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#ifndef QDENGINE_QD_PRECOMP_H
#define QDENGINE_QD_PRECOMP_H
#ifndef _QUEST_EDITOR
#else
/*  #define   _STLP_NO_NEW_IOSTREAMS 1

    #ifdef _STLP_NO_NEW_IOSTREAMS
        #define   _STLP_NO_OWN_NAMESPACE 1
    #else
        #define _STLP_USE_OWN_NAMESPACE 1
        #define _STLP_REDEFINE_STD 1
    #endif*/
#endif // _QUEST_EDITOR

#ifndef _FINAL_VERSION
#ifndef __QD_SYSLIB__
#define __QD_DEBUG_ENABLE__
#endif
#endif

#ifdef __QD_DEBUG_ENABLE_
#define __QDBG(a) a
#else
#define __QDBG(a)
#endif

#ifdef _DEBUG
#define DBGCHECK _ASSERTE(_CrtCheckMemory())
#else
#define DBGCHECK
#endif

//#define _GR_ENABLE_ZBUFFER

#include "qdengine/xmath.h"

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "engines/qdengine/qd_fwd.h"

namespace QDEngine {

// STUB: XZipStream
#define _NO_ZIP_

#define BOOL          bool
#define HWND          void *
#define DWORD         int
#define COLORREF      unsigned

#define MAX_PATH      260

#define _MAX_PATH     256
#define _MAX_EXT      256

#define IDYES         6
#define IDOK          1
#define IDNO          7
#define IDRETRY       4
#define IDIGNORE      5
#define IDCANCEL      2

#define VK_ESCAPE     0x1B
#define VK_RETURN     0x0D
#define VK_SPACE      0x20
#define VK_LEFT       0x25
#define VK_UP         0x26
#define VK_RIGHT      0x27
#define VK_DOWN       0x28
#define VK_BACK       0x08
#define VK_DELETE     0x2E
#define VK_HOME       0x24
#define VK_END        0x23
#define VK_F10        0x79
#define VK_F9         0x78
#define VK_F5         0x74
#define VK_F6         0x75
#define VK_PAUSE      0x13
#define VK_NEXT       0x22
#define VK_PRIOR      0x21
#define PM_REMOVE     0x0001
#define SC_MAXIMIZE   0xF030

#define MB_OK               0x00000000L
#define MB_YESNO            0x00000004L
#define MB_TASKMODAL        0x00002000L
#define MB_OKCANCEL         0x00000001L
#define MB_ICONERROR        0x00000010L
#define MK_LBUTTON          0x0001
#define MK_RBUTTON          0x0002
#define MK_MBUTTON          0x0010
#define MB_ICONEXCLAMATION  0x00000030L
#define MB_ABORTRETRYIGNORE 0x00000002L

#define SW_HIDE         0

struct RECT {
	int left;
	int top;
	int right;
	int bottom;
};

int MessageBox(HWND hWnd, const char *lpText, const char *lpCaption, uint32 uType);
bool ShowWindow(HWND hWnd, int nCmdShow);
bool CloseWindow(HWND hWnd);
bool DestroyWindow(HWND hWnd);
bool UpdateWindow(HWND hWnd);
char *_fullpath(char *absPath, const char *relPath, size_t maxLength);

int engineMain();

} // namespace QDEngine

#endif // QDENGINE_QD_PRECOMP_H
