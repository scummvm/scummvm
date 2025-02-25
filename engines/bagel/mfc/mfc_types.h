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

#include "common/file.h"
#include "common/serializer.h"
#include "graphics/managed_surface.h"
#include "graphics/palette.h"

#ifndef BAGEL_MFC_TYPES_H
#define BAGEL_MFC_TYPES_H

namespace Bagel {
namespace MFC {

#define GAME_WIDTH		640
#define GAME_HEIGHT		480

#define DECLARE_MESSAGE_MAP()
#define DECLARE_DYNCREATE(KLASS)
#define IMPLEMENT_DYNCREATE(KLASS, PARENT)
#define afx_msg
#define BEGIN_MESSAGE_MAP(KLASS, PARENT)
#define ON_WM_ERASEBKGND()
#define ON_WM_DRAWITEM()
#define END_MESSAGE_MAP()
#define ON_MESSAGE(TAG, METHOD)
#define BM_SETCHECK 0
#define BM_GETCHECK 1


typedef bool BOOL;
typedef uint16 WPARAM;
typedef int32 LPARAM;
typedef uint UINT;
typedef uint32 DWORD;

#define TRUE true
#define FALSE false

typedef void *HANDLE;
typedef Graphics::ManagedSurface *HDIB;
typedef Common::Serializer CDumpContext;
typedef Graphics::Palette *HPALETTE;
typedef uint32 COLORREF;
typedef Common::File CFile;

typedef char *LPSTR;
typedef const char *LPCSTR;
typedef void *HBITMAP;
typedef void *LPBITMAPINFO;
typedef void *LPBITMAPINFOHEADER;
typedef Graphics::ManagedSurface *HWND;

class CSprite {
};
class CText {
};
class CSound {
};
class CDibDoc {
};

} // namespace MFC
} // namespace Bagel

#endif
