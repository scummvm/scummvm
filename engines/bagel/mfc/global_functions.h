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

#ifndef BAGEL_MFC_GLOBAL_FUNCTIONS_H
#define BAGEL_MFC_GLOBAL_FUNCTIONS_H

#include "bagel/mfc/minwindef.h"
#include "bagel/mfc/winnt.h"

namespace Bagel {
namespace MFC {

/* Global Memory Flags */
#define GMEM_FIXED          0x0000
#define GMEM_MOVEABLE       0x0002
#define GMEM_NOCOMPACT      0x0010
#define GMEM_NODISCARD      0x0020
#define GMEM_ZEROINIT       0x0040
#define GMEM_MODIFY         0x0080
#define GMEM_DISCARDABLE    0x0100
#define GMEM_NOT_BANKED     0x1000
#define GMEM_SHARE          0x2000
#define GMEM_DDESHARE       0x2000
#define GMEM_NOTIFY         0x4000
#define GMEM_LOWER          GMEM_NOT_BANKED
#define GMEM_VALID_FLAGS    0x7F72
#define GMEM_INVALID_HANDLE 0x8000

#define GHND                (GMEM_MOVEABLE | GMEM_ZEROINIT)
#define GPTR                (GMEM_FIXED | GMEM_ZEROINIT)

#define MB_ICONHAND                 0x00000010L
#define MB_ICONQUESTION             0x00000020L
#define MB_ICONEXCLAMATION          0x00000030L
#define MB_ICONASTERISK             0x00000040L
#define MB_ICONINFORMATION          MB_ICONASTERISK
#define MB_ICONSTOP                 MB_ICONHAND

extern HGLOBAL GlobalAlloc(UINT uFlags, SIZE_T dwBytes);
extern LPVOID GlobalLock(HGLOBAL hMem);
extern BOOL GlobalUnlock(HGLOBAL hMem);
extern HGLOBAL GlobalFree(HGLOBAL hMem);
extern SIZE_T GlobalSize(HGLOBAL hMem);
extern SIZE_T GlobalCompact(DWORD dwMinFree);
#define GetFreeSpace(w)                 (0x100000L)

extern int MessageBox(HWND hWnd, LPCSTR lpText,
	LPCSTR lpCaption, UINT uType);

} // namespace MFC
} // namespace Bagel

#endif
