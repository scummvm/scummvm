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


#ifndef __CFFGLOAD_H__
#define __CFFGLOAD_H__


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/freetype.h"
#include "cffobjs.h"


FT2_1_3_BEGIN_HEADER


#define CFF_MAX_OPERANDS     48
#define CFF_MAX_SUBRS_CALLS  32


/*************************************************************************/
/*                                                                       */
/* <Structure>                                                           */
/*    CFF_Builder                                                        */
/*                                                                       */
/* <Description>                                                         */
/*     A structure used during glyph loading to store its outline.       */
/*                                                                       */
/* <Fields>                                                              */
/*    memory        :: The current memory object.                        */
/*                                                                       */
/*    face          :: The current face object.                          */
/*                                                                       */
/*    glyph         :: The current glyph slot.                           */
/*                                                                       */
/*    loader        :: The current glyph loader.                         */
/*                                                                       */
/*    base          :: The base glyph outline.                           */
/*                                                                       */
/*    current       :: The current glyph outline.                        */
/*                                                                       */
/*    last          :: The last point position.                          */
/*                                                                       */
/*    scale_x       :: The horizontal scale (FUnits to sub-pixels).      */
/*                                                                       */
/*    scale_y       :: The vertical scale (FUnits to sub-pixels).        */
/*                                                                       */
/*    pos_x         :: The horizontal translation (if composite glyph).  */
/*                                                                       */
/*    pos_y         :: The vertical translation (if composite glyph).    */
/*                                                                       */
/*    left_bearing  :: The left side bearing point.                      */
/*                                                                       */
/*    advance       :: The horizontal advance vector.                    */
/*                                                                       */
/*    bbox          :: Unused.                                           */
/*                                                                       */
/*    path_begun    :: A flag which indicates that a new path has begun. */
/*                                                                       */
/*    load_points   :: If this flag is not set, no points are loaded.    */
/*                                                                       */
/*    no_recurse    :: Set but not used.                                 */
/*                                                                       */
/*    error         :: An error code that is only used to report memory  */
/*                     allocation problems.                              */
/*                                                                       */
/*    metrics_only  :: A boolean indicating that we only want to compute */
/*                     the metrics of a given glyph, not load all of its */
/*                     points.                                           */
/*                                                                       */
/*    hints_funcs   :: Auxiliary pointer for hinting.                    */
/*                                                                       */
/*    hints_globals :: Auxiliary pointer for hinting.                    */
/*                                                                       */
typedef struct  CFF_Builder_ {
	FT2_1_3_Memory       memory;
	TT_Face         face;
	CFF_GlyphSlot   glyph;
	FT2_1_3_GlyphLoader  loader;
	FT2_1_3_Outline*     base;
	FT2_1_3_Outline*     current;

	FT2_1_3_Vector       last;

	FT2_1_3_Fixed        scale_x;
	FT2_1_3_Fixed        scale_y;

	FT2_1_3_Pos          pos_x;
	FT2_1_3_Pos          pos_y;

	FT2_1_3_Vector       left_bearing;
	FT2_1_3_Vector       advance;

	FT2_1_3_BBox         bbox;          /* bounding box */
	FT2_1_3_Bool         path_begun;
	FT2_1_3_Bool         load_points;
	FT2_1_3_Bool         no_recurse;

	FT2_1_3_Error        error;         /* only used for memory errors */
	FT2_1_3_Bool         metrics_only;

	FT2_1_3_UInt32       hint_flags;

	void*           hints_funcs;    /* hinter-specific */
	void*           hints_globals;  /* hinter-specific */

} CFF_Builder;


/* execution context charstring zone */

typedef struct  CFF_Decoder_Zone_ {
	FT2_1_3_Byte*  base;
	FT2_1_3_Byte*  limit;
	FT2_1_3_Byte*  cursor;

} CFF_Decoder_Zone;


typedef struct  CFF_Decoder_ {
	CFF_Builder        builder;
	CFF_Font           cff;

	FT2_1_3_Fixed           stack[CFF_MAX_OPERANDS + 1];
	FT2_1_3_Fixed*          top;

	CFF_Decoder_Zone   zones[CFF_MAX_SUBRS_CALLS + 1];
	CFF_Decoder_Zone*  zone;

	FT2_1_3_Int             flex_state;
	FT2_1_3_Int             num_flex_vectors;
	FT2_1_3_Vector          flex_vectors[7];

	FT2_1_3_Pos             glyph_width;
	FT2_1_3_Pos             nominal_width;

	FT2_1_3_Bool            read_width;
	FT2_1_3_Int             num_hints;
	FT2_1_3_Fixed*          buildchar;
	FT2_1_3_Int             len_buildchar;

	FT2_1_3_UInt            num_locals;
	FT2_1_3_UInt            num_globals;

	FT2_1_3_Int             locals_bias;
	FT2_1_3_Int             globals_bias;

	FT2_1_3_Byte**          locals;
	FT2_1_3_Byte**          globals;

	FT2_1_3_Byte**          glyph_names;   /* for pure CFF fonts only  */
	FT2_1_3_UInt            num_glyphs;    /* number of glyphs in font */

	FT2_1_3_Render_Mode     hint_mode;

} CFF_Decoder;


FT2_1_3_LOCAL( void )
cff_decoder_init( CFF_Decoder*    decoder,
                  TT_Face         face,
                  CFF_Size        size,
                  CFF_GlyphSlot   slot,
                  FT2_1_3_Bool         hinting,
                  FT2_1_3_Render_Mode  hint_mode );

FT2_1_3_LOCAL( void )
cff_decoder_prepare( CFF_Decoder*  decoder,
                     FT2_1_3_UInt       glyph_index );

#if 0  /* unused until we support pure CFF fonts */

/* Compute the maximum advance width of a font through quick parsing */
FT2_1_3_LOCAL( FT2_1_3_Error )
cff_compute_max_advance( TT_Face  face,
                         FT2_1_3_Int*  max_advance );

#endif /* 0 */

FT2_1_3_LOCAL( FT2_1_3_Error )
cff_decoder_parse_charstrings( CFF_Decoder*  decoder,
                               FT2_1_3_Byte*      charstring_base,
                               FT2_1_3_ULong      charstring_len );

FT2_1_3_LOCAL( FT2_1_3_Error )
cff_slot_load( CFF_GlyphSlot  glyph,
               CFF_Size       size,
               FT2_1_3_Int         glyph_index,
               FT2_1_3_Int32       load_flags );


FT2_1_3_END_HEADER

#endif /* __CFFGLOAD_H__ */


/* END */
