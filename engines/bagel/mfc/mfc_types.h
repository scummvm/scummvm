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

#ifndef BAGEL_HODJNPODJ_MFC_TYPES_H
#define BAGEL_HODJNPODJ_MFC_TYPES_H

namespace Bagel {
namespace HodjNPodj {

#define DECLARE_MESSAGE_MAP()

typedef bool BOOL;
typedef uint16 WPARAM;
typedef int32 LPARAM;
typedef uint UINT;
#define TRUE true
#define FALSE false

typedef void *HANDLE;
typedef Graphics::ManagedSurface CBitmap;
typedef Graphics::ManagedSurface *HDIB;
typedef Common::Rect *LPRECT;
typedef Common::Rect CRect;
typedef Common::Point CPoint;
typedef Common::Point CSize;
typedef Common::File CFile;
typedef Common::Serializer CDumpContext;
typedef Graphics::Palette *HPALETTE;
typedef Common::String CString;

typedef char *LPSTR;
typedef void *HBITMAP;
typedef void *LPBITMAPINFO;
typedef void *LPBITMAPINFOHEADER;

class CBmpButton {
};
class CSprite {
};
class CText {
};
class CSound {
};

class CPalette : public Graphics::Palette {
public:
	CPalette() : Graphics::Palette(PALETTE_COUNT) {}
	CPalette(uint size) : Graphics::Palette(size) {}
	CPalette(const byte *data, uint size) : Graphics::Palette(data, size) {}

	void DeleteObject() {}
};

} // namespace HodjNPodj
} // namespace Bagel

#endif
