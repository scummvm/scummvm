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
#define afx_msg

typedef bool BOOL;
typedef uint16 WPARAM;
typedef int32 LPARAM;
typedef uint UINT;
typedef uint32 DWORD;

#define TRUE true
#define FALSE false

typedef void *HANDLE;
typedef Graphics::ManagedSurface CBitmap;
typedef Graphics::ManagedSurface *HDIB;
typedef Common::Rect RECT;
typedef Common::Rect *LPRECT;
typedef Common::Rect CRect;
typedef Common::Point CPoint;
typedef Common::Point CSize;
typedef Common::File CFile;
typedef Common::Serializer CDumpContext;
typedef Graphics::Palette *HPALETTE;
typedef Common::String CString;
typedef uint32 COLORREF;

typedef char *LPSTR;
typedef char *LPCSTR;
typedef void *HBITMAP;
typedef void *LPBITMAPINFO;
typedef void *LPBITMAPINFOHEADER;
typedef void *HWND;


class CBmpButton {
};
class CSprite {
};
class CText {
};
class CSound {
};
class CWnd {
};
class CDibDoc {
};

} // namespace MFC
} // namespace Bagel

#endif
