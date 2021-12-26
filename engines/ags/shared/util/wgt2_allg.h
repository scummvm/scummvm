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

//=============================================================================
//
// Few graphic utility functions, remains of a bigger deprecated api.
//
//=============================================================================

#ifndef AGS_SHARED_UTIL_WGT2_ALIG_H
#define AGS_SHARED_UTIL_WGT2_ALIG_H

#include "ags/lib/allegro.h" // RGB

namespace AGS3 {

namespace AGS {
namespace Shared {
class Bitmap;
}
}
using namespace AGS; // FIXME later

//=============================================================================

// [IKM] 2012-09-13: this function is now defined in engine and editor separately
extern void __my_setcolor(int *ctset, int newcol, int wantColDep);

extern void wsetrgb(int coll, int r, int g, int b, RGB *pall);
extern void wcolrotate(unsigned char start, unsigned char finish, int dir, RGB *pall);

extern Shared::Bitmap *wnewblock(Shared::Bitmap *src, int x1, int y1, int x2, int y2);

extern void wputblock(Shared::Bitmap *ds, int xx, int yy, Shared::Bitmap *bll, int xray);
// CHECKME: temporary solution for plugin system
extern void wputblock_raw(Shared::Bitmap *ds, int xx, int yy, BITMAP *bll, int xray);
extern const int col_lookups[32];

// TODO: these are used only in the Editor's agsnative.cpp
extern int __wremap_keep_transparent;
extern void wremap(RGB *pal1, Shared::Bitmap *picc, RGB *pal2);
extern void wremapall(RGB *pal1, Shared::Bitmap *picc, RGB *pal2);

} // namespace AGS3

#endif
