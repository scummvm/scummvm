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

#include "bagel/spacebar/boflib/rect.h"
#include "common/serializer.h"
#include "graphics/managed_surface.h"

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
typedef Graphics::ManagedSurface *HDIB;
typedef CBofRect *LPRECT;
typedef Common::Serializer CDumpContext;
typedef char *LPSTR;
typedef void *HBITMAP;
typedef void *LPBITMAPINFO;
typedef void *LPBITMAPINFOHEADER;

} // namespace HodjNPodj
} // namespace Bagel

#endif
