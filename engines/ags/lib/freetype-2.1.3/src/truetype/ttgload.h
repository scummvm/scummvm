/***************************************************************************/
/*                                                                         */
/*  ttgload.h                                                              */
/*                                                                         */
/*    TrueType Glyph Loader (specification).                               */
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


#ifndef AGS_LIB_FREETYPE_TTGLOAD_H
#define AGS_LIB_FREETYPE_TTGLOAD_H


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "ttobjs.h"

#ifdef TT_CONFIG_OPTION_BYTECODE_INTERPRETER
#include "ttinterp.h"
#endif

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER


FT2_1_3_LOCAL( void )
TT_Get_Metrics( TT_HoriHeader*  header,
				FT2_1_3_UInt         index,
				FT2_1_3_Short*       bearing,
				FT2_1_3_UShort*      advance );

FT2_1_3_LOCAL( void )
TT_Init_Glyph_Loading( TT_Face  face );

FT2_1_3_LOCAL( FT2_1_3_Error )
TT_Load_Glyph( TT_Size       size,
			   TT_GlyphSlot  glyph,
			   FT2_1_3_UShort     glyph_index,
			   FT2_1_3_Int32      load_flags );


FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_TTGLOAD_H */


/* END */
