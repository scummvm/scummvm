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
/*  cffgload.h                                                             */
/*                                                                         */
/*    OpenType Glyph Loader (specification).                               */
/*                                                                         */
/*  Copyright 1996-2001, 2002 by                                           */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_CFFGLOAD_H
#define AGS_LIB_FREETYPE_CFFGLOAD_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/freetype.h"
#include "engines/ags/lib/freetype-2.1.3/modules/cff/cffobjs.h"

namespace AGS3 {
namespace FreeType213 {

// FT_BEGIN_HEADER


#define CFF_MAX_OPERANDS     48
#define CFF_MAX_SUBRS_CALLS  32


typedef struct  CFF_Builder_ {
	FT_Memory       memory;
	TT_Face         face;
	CFF_GlyphSlot   glyph;
	FT_GlyphLoader  loader;
	FT_Outline      *base;
	FT_Outline      *current;

	FT_Vector       last;

	FT_Fixed        scale_x;
	FT_Fixed        scale_y;

	FT_Pos          pos_x;
	FT_Pos          pos_y;

	FT_Vector       left_bearing;
	FT_Vector       advance;

	FT_BBox         bbox;          /* bounding box */
	FT_Bool         path_begun;
	FT_Bool         load_points;
	FT_Bool         no_recurse;

	FT_Error        error;         /* only used for memory errors */
	FT_Bool         metrics_only;

	FT_UInt32       hint_flags;

	void            *hints_funcs;    /* hinter-specific */
	void            *hints_globals;  /* hinter-specific */
} CFF_Builder;


/* execution context charstring zone */

typedef struct  CFF_Decoder_Zone_ {
	FT_Byte   *base;
	FT_Byte   *limit;
	FT_Byte   *cursor;
} CFF_Decoder_Zone;


typedef struct CFF_Decoder_ {
	CFF_Builder        builder;
	CFF_Font           cff;

	FT_Fixed           stack[CFF_MAX_OPERANDS + 1];
	FT_Fixed           *top;

	CFF_Decoder_Zone   zones[CFF_MAX_SUBRS_CALLS + 1];
	CFF_Decoder_Zone   *zone;

	FT_Int             flex_state;
	FT_Int             num_flex_vectors;
	FT_Vector          flex_vectors[7];

	FT_Pos             glyph_width;
	FT_Pos             nominal_width;

	FT_Bool            read_width;
	FT_Int             num_hints;
	FT_Fixed           *buildchar;
	FT_Int             len_buildchar;

	FT_UInt            num_locals;
	FT_UInt            num_globals;

	FT_Int             locals_bias;
	FT_Int             globals_bias;

	FT_Byte            **locals;
	FT_Byte            **globals;

	FT_Byte            **glyph_names; /* for pure CFF fonts only  */
	FT_UInt            num_glyphs;    /* number of glyphs in font */

	FT_Render_Mode     hint_mode;
} CFF_Decoder;

FT_LOCAL(void)
cff_decoder_init(CFF_Decoder *decoder, TT_Face face, CFF_Size size, CFF_GlyphSlot slot, FT_Bool hinting, FT_Render_Mode hint_mode);

FT_LOCAL(void)
cff_decoder_prepare(CFF_Decoder *decoder, FT_UInt glyph_index);

#if 0 /* unused until we support pure CFF fonts */

/* Compute the maximum advance width of a font through quick parsing */
FT_LOCAL(FT_Error)
cff_compute_max_advance(TT_Face face, FT_Int *max_advance);

#endif /* 0 */

FT_LOCAL(FT_Error)
cff_decoder_parse_charstrings(CFF_Decoder *decoder, FT_Byte *charstring_base, FT_ULong charstring_len);

FT_LOCAL(FT_Error)
cff_slot_load(CFF_GlyphSlot glyph, CFF_Size size, FT_Int glyph_index, FT_Int32 load_flags);


// FT_END_HEADER


} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_CFFGLOAD_H */
