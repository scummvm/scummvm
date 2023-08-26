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

/***************************************************************************/
/*                                                                         */
/*  ftgloadr.h                                                             */
/*                                                                         */
/*    The FreeType glyph loader (specification).                           */
/*                                                                         */
/*  Copyright 2002 by                                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg                       */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_FTGLOADR_H
#define AGS_LIB_FREETYPE_FTGLOADR_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/freetype.h"

namespace AGS3 {
namespace FreeType213 {

// FT_BEGIN_HEADER


typedef struct FT_GlyphLoaderRec_ *FT_GlyphLoader;

#define FT_SUBGLYPH_FLAG_ARGS_ARE_WORDS          1
#define FT_SUBGLYPH_FLAG_ARGS_ARE_XY_VALUES      2
#define FT_SUBGLYPH_FLAG_ROUND_XY_TO_GRID        4
#define FT_SUBGLYPH_FLAG_SCALE                   8
#define FT_SUBGLYPH_FLAG_XY_SCALE             0x40
#define FT_SUBGLYPH_FLAG_2X2                  0x80
#define FT_SUBGLYPH_FLAG_USE_MY_METRICS      0x200


enum {
	FT_GLYPH_OWN_BITMAP = 1
};


typedef struct  FT_SubGlyphRec_ {
	FT_Int     index;
	FT_UShort  flags;
	FT_Int     arg1;
	FT_Int     arg2;
	FT_Matrix  transform;

} FT_SubGlyphRec;


typedef struct  FT_GlyphLoadRec_ {
	FT_Outline   outline;        /* outline             */
	FT_Vector    *extra_points;  /* extra points table  */
	FT_UInt      num_subglyphs;  /* number of subglyphs */
	FT_SubGlyph  subglyphs;      /* subglyphs           */

} FT_GlyphLoadRec, *FT_GlyphLoad;


typedef struct  FT_GlyphLoaderRec_ {
	FT_Memory        memory;
	FT_UInt          max_points;
	FT_UInt          max_contours;
	FT_UInt          max_subglyphs;
	FT_Bool          use_extra;

	FT_GlyphLoadRec  base;
	FT_GlyphLoadRec  current;

	void*            other;            /* for possible future extension? */

} FT_GlyphLoaderRec;


/* create new empty glyph loader */
FT_BASE(FT_Error)
FT_GlyphLoader_New(FT_Memory memory, FT_GlyphLoader *aloader);

/* add an extra points table to a glyph loader */
FT_BASE(FT_Error)
FT_GlyphLoader_CreateExtra(FT_GlyphLoader loader);

/* destroy a glyph loader */
FT_BASE(void)
FT_GlyphLoader_Done(FT_GlyphLoader loader);

/* reset a glyph loader (frees everything int it) */
FT_BASE(void)
FT_GlyphLoader_Reset(FT_GlyphLoader loader);

/* rewind a glyph loader */
FT_BASE(void)
FT_GlyphLoader_Rewind(FT_GlyphLoader loader);

/* check that there is enough room to add 'n_points' and 'n_contours' */
/* to the glyph loader                                                */
FT_BASE(FT_Error)
FT_GlyphLoader_CheckPoints(FT_GlyphLoader loader, FT_UInt n_points, FT_UInt n_contours);

/* check that there is enough room to add 'n_subs' sub-glyphs to */
/* a glyph loader                                                */
FT_BASE(FT_Error)
FT_GlyphLoader_CheckSubGlyphs(FT_GlyphLoader loader, FT_UInt n_subs);

/* prepare a glyph loader, i.e. empty the current glyph */
FT_BASE(void)
FT_GlyphLoader_Prepare(FT_GlyphLoader loader);

/* add the current glyph to the base glyph */
FT_BASE(void)
FT_GlyphLoader_Add(FT_GlyphLoader loader);

/* copy points from one glyph loader to another */
FT_BASE(FT_Error)
FT_GlyphLoader_CopyPoints(FT_GlyphLoader target, FT_GlyphLoader source);


// FT_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_FTGLOADR_H */
