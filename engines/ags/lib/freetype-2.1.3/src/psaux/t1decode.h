/***************************************************************************/
/*                                                                         */
/*  t1decode.h                                                             */
/*                                                                         */
/*    PostScript Type 1 decoding routines (specification).                 */
/*                                                                         */
/*  Copyright 2000-2001, 2002 by                                           */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_T1DECODE_H
#define AGS_LIB_FREETYPE_T1DECODE_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/psaux.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/psnames.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/t1types.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER


FT2_1_3_CALLBACK_TABLE
const T1_Decoder_FuncsRec  t1_decoder_funcs;


FT2_1_3_LOCAL( FT_Error )
t1_decoder_parse_glyph( T1_Decoder  decoder,
						FT_UInt     glyph_index );

FT2_1_3_LOCAL( FT_Error )
t1_decoder_parse_charstrings( T1_Decoder  decoder,
							  FT_Byte*    base,
							  FT_UInt     len );

FT2_1_3_LOCAL( FT_Error )
t1_decoder_init( T1_Decoder           decoder,
				 FT_Face              face,
				 FT_Size              size,
				 FT_GlyphSlot         slot,
				 FT_Byte**            glyph_names,
				 PS_Blend             blend,
				 FT_Bool              hinting,
				 FT_Render_Mode       hint_mode,
				 T1_Decoder_Callback  parse_glyph );

FT2_1_3_LOCAL( void )
t1_decoder_done( T1_Decoder  decoder );


FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_T1DECODE_H */


/* END */
