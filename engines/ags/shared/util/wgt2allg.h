/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

//=============================================================================
//
// WGT -> Allegro portability interface
//
// wsavesprites and wloadsprites are hi-color compliant
//
//=============================================================================

#include "core/platform.h"

#define _WGT45_

#ifndef AGS_SHARED_UTIL_WGT4_H
#define AGS_SHARED_UTIL_WGT4_H

#include "allegro.h"

namespace AGS3 {

namespace AGS {
namespace Shared {
class Bitmap;
} // namespace Shared
} // namespace AGS

using namespace AGS; // FIXME later


#if defined WGT2ALLEGRO_NOFUNCTIONS
#error WGT2ALLEGRO_NOFUNCTIONS macro is obsolete and should not be defined anymore.
#endif

#define color RGB

//=============================================================================

// [IKM] 2012-09-13: this function is now defined in engine and editor separately
extern void __my_setcolor(int *ctset, int newcol, int wantColDep);

#ifdef __cplusplus
extern "C"
{
#endif

	extern void wsetrgb(int coll, int r, int g, int b, color *pall);
	extern void wcolrotate(unsigned char start, unsigned char finish, int dir, color *pall);

	extern Common::Bitmap *wnewblock(Common::Bitmap *src, int x1, int y1, int x2, int y2);

	extern int wloadsprites(color *pall, char *filnam, Common::Bitmap **sarray, int strt, int eend);

	extern void wputblock(Common::Bitmap *ds, int xx, int yy, Common::Bitmap *bll, int xray);
	// CHECKME: temporary solution for plugin system
	extern void wputblock_raw(Common::Bitmap *ds, int xx, int yy, BITMAP *bll, int xray);
	extern const int col_lookups[32];

	//extern void wsetcolor(int nval);

	extern int __wremap_keep_transparent;
	extern void wremap(color *pal1, Common::Bitmap *picc, color *pal2);
	extern void wremapall(color *pal1, Common::Bitmap *picc, color *pal2);

#ifdef __cplusplus
}
#endif

#define XRAY    1
#define NORMAL  0

// archive attributes to search for - al_findfirst breaks with 0
#define FA_SEARCH -1

} // namespace AGS3

#endif
