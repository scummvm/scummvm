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


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/freetype.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER


/*************************************************************************/
/*                                                                       */
/* <Struct>                                                              */
/*    FT2_1_3_GlyphLoader                                                     */
/*                                                                       */
/* <Description>                                                         */
/*    The glyph loader is an internal object used to load several glyphs */
/*    together (for example, in the case of composites).                 */
/*                                                                       */
/* <Note>                                                                */
/*    The glyph loader implementation is not part of the high-level API, */
/*    hence the forward structure declaration.                           */
/*                                                                       */
typedef struct FT2_1_3_GlyphLoaderRec_*  FT2_1_3_GlyphLoader ;


#define FT2_1_3_SUBGLYPH_FLAG_ARGS_ARE_WORDS          1
#define FT2_1_3_SUBGLYPH_FLAG_ARGS_ARE_XY_VALUES      2
#define FT2_1_3_SUBGLYPH_FLAG_ROUND_XY_TO_GRID        4
#define FT2_1_3_SUBGLYPH_FLAG_SCALE                   8
#define FT2_1_3_SUBGLYPH_FLAG_XY_SCALE             0x40
#define FT2_1_3_SUBGLYPH_FLAG_2X2                  0x80
#define FT2_1_3_SUBGLYPH_FLAG_USE_MY_METRICS      0x200


enum {
	FT2_1_3_GLYPH_OWN_BITMAP = 1
};


typedef struct  FT2_1_3_SubGlyphRec_ {
	FT2_1_3_Int     index;
	FT2_1_3_UShort  flags;
	FT2_1_3_Int     arg1;
	FT2_1_3_Int     arg2;
	FT2_1_3_Matrix  transform;

} FT2_1_3_SubGlyphRec;


typedef struct  FT2_1_3_GlyphLoadRec_ {
	FT2_1_3_Outline   outline;       /* outline             */
	FT2_1_3_Vector*   extra_points;  /* extra points table  */
	FT2_1_3_UInt      num_subglyphs; /* number of subglyphs */
	FT2_1_3_SubGlyph  subglyphs;     /* subglyphs           */

} FT2_1_3_GlyphLoadRec, *FT2_1_3_GlyphLoad;


typedef struct  FT2_1_3_GlyphLoaderRec_ {
	FT2_1_3_Memory        memory;
	FT2_1_3_UInt          max_points;
	FT2_1_3_UInt          max_contours;
	FT2_1_3_UInt          max_subglyphs;
	FT2_1_3_Bool          use_extra;

	FT2_1_3_GlyphLoadRec  base;
	FT2_1_3_GlyphLoadRec  current;

	void*            other;            /* for possible future extension? */

} FT2_1_3_GlyphLoaderRec;


/* create new empty glyph loader */
FT2_1_3_BASE( FT2_1_3_Error )
FT2_1_3_GlyphLoader_New( FT2_1_3_Memory        memory,
                    FT2_1_3_GlyphLoader  *aloader );

/* add an extra points table to a glyph loader */
FT2_1_3_BASE( FT2_1_3_Error )
FT2_1_3_GlyphLoader_CreateExtra( FT2_1_3_GlyphLoader  loader );

/* destroy a glyph loader */
FT2_1_3_BASE( void )
FT2_1_3_GlyphLoader_Done( FT2_1_3_GlyphLoader  loader );

/* reset a glyph loader (frees everything int it) */
FT2_1_3_BASE( void )
FT2_1_3_GlyphLoader_Reset( FT2_1_3_GlyphLoader  loader );

/* rewind a glyph loader */
FT2_1_3_BASE( void )
FT2_1_3_GlyphLoader_Rewind( FT2_1_3_GlyphLoader  loader );

/* check that there is enough room to add 'n_points' and 'n_contours' */
/* to the glyph loader                                                */
FT2_1_3_BASE( FT2_1_3_Error )
FT2_1_3_GlyphLoader_CheckPoints( FT2_1_3_GlyphLoader  loader,
                            FT2_1_3_UInt         n_points,
                            FT2_1_3_UInt         n_contours );

/* check that there is enough room to add 'n_subs' sub-glyphs to */
/* a glyph loader                                                */
FT2_1_3_BASE( FT2_1_3_Error )
FT2_1_3_GlyphLoader_CheckSubGlyphs( FT2_1_3_GlyphLoader  loader,
                               FT2_1_3_UInt         n_subs );

/* prepare a glyph loader, i.e. empty the current glyph */
FT2_1_3_BASE( void )
FT2_1_3_GlyphLoader_Prepare( FT2_1_3_GlyphLoader  loader );

/* add the current glyph to the base glyph */
FT2_1_3_BASE( void )
FT2_1_3_GlyphLoader_Add( FT2_1_3_GlyphLoader  loader );

/* copy points from one glyph loader to another */
FT2_1_3_BASE( FT2_1_3_Error )
FT2_1_3_GlyphLoader_CopyPoints( FT2_1_3_GlyphLoader  target,
                           FT2_1_3_GlyphLoader  source );

/* */


FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_FTGLOADR_H */


/* END */
