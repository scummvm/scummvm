//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================
//
// WGT -> Allegro portability interface
//
// wsavesprites and wloadsprites are hi-color compliant
//
//=============================================================================

#include "core/platform.h"

#define _WGT45_

#ifndef __WGT4_H
#define __WGT4_H

#include "allegro.h"

namespace AGS { namespace Common { class Bitmap; }}
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
    
    extern void wsetrgb(int coll, int r, int g, int b, color * pall);
    extern void wcolrotate(unsigned char start, unsigned char finish, int dir, color * pall);

    extern Common::Bitmap *wnewblock(Common::Bitmap *src, int x1, int y1, int x2, int y2);

    extern int wloadsprites(color * pall, char *filnam, Common::Bitmap ** sarray, int strt, int eend);

    extern void wputblock(Common::Bitmap *ds, int xx, int yy, Common::Bitmap *bll, int xray);
	// CHECKME: temporary solution for plugin system
	extern void wputblock_raw(Common::Bitmap *ds, int xx, int yy, BITMAP *bll, int xray);
    extern const int col_lookups[32];

    //extern void wsetcolor(int nval);

    extern int __wremap_keep_transparent;
    extern void wremap(color * pal1, Common::Bitmap *picc, color * pal2);
    extern void wremapall(color * pal1, Common::Bitmap *picc, color * pal2);

#ifdef __cplusplus
}
#endif

#define XRAY    1
#define NORMAL  0

// archive attributes to search for - al_findfirst breaks with 0
#define FA_SEARCH -1


#endif // __WGT4_H
