/***************************************************************************/
/*                                                                         */
/*  t1gload.h                                                              */
/*                                                                         */
/*    Type 1 Glyph Loader (specification).                                 */
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


#ifndef AGS_LIB_FREETYPE_T1GLOAD_H
#define AGS_LIB_FREETYPE_T1GLOAD_H


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "t1objs.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER


FT2_1_3_LOCAL( FT2_1_3_Error )
T1_Compute_Max_Advance( T1_Face  face,
						FT2_1_3_Int*  max_advance );

FT2_1_3_LOCAL( FT2_1_3_Error )
T1_Load_Glyph( T1_GlyphSlot  glyph,
			   T1_Size       size,
			   FT2_1_3_UInt       glyph_index,
			   FT2_1_3_Int32      load_flags );


FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_T1GLOAD_H */


/* END */
