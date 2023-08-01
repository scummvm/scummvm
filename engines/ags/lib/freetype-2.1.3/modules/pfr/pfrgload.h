/***************************************************************************/
/*                                                                         */
/*  pfrgload.h                                                             */
/*                                                                         */
/*    FreeType PFR glyph loader (specification).                           */
/*                                                                         */
/*  Copyright 2002 by                                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_PFRGLOAD_H
#define AGS_LIB_FREETYPE_PFRGLOAD_H

#include "pfrtypes.h"

namespace AGS3 {
namespace FreeType213 {

// FT_BEGIN_HEADER


FT_LOCAL( void )
pfr_glyph_init( PFR_Glyph       glyph,
				FT_GlyphLoader  loader );

FT_LOCAL( void )
pfr_glyph_done( PFR_Glyph  glyph );


FT_LOCAL( FT_Error )
pfr_glyph_load( PFR_Glyph  glyph,
				FT_Stream  stream,
				FT_ULong   gps_offset,
				FT_ULong   offset,
				FT_ULong   size );


// FT_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_PFRGLOAD_H */


/* END */
