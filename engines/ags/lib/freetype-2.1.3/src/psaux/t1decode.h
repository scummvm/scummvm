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


#ifndef __T1DECODE_H__
#define __T1DECODE_H__


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/psaux.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/psnames.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/t1types.h"


FT2_1_3_BEGIN_HEADER


FT2_1_3_CALLBACK_TABLE
const T1_Decoder_FuncsRec  t1_decoder_funcs;


FT2_1_3_LOCAL( FT2_1_3_Error )
t1_decoder_parse_glyph( T1_Decoder  decoder,
                        FT2_1_3_UInt     glyph_index );

FT2_1_3_LOCAL( FT2_1_3_Error )
t1_decoder_parse_charstrings( T1_Decoder  decoder,
                              FT2_1_3_Byte*    base,
                              FT2_1_3_UInt     len );

FT2_1_3_LOCAL( FT2_1_3_Error )
t1_decoder_init( T1_Decoder           decoder,
                 FT2_1_3_Face              face,
                 FT2_1_3_Size              size,
                 FT2_1_3_GlyphSlot         slot,
                 FT2_1_3_Byte**            glyph_names,
                 PS_Blend             blend,
                 FT2_1_3_Bool              hinting,
                 FT2_1_3_Render_Mode       hint_mode,
                 T1_Decoder_Callback  parse_glyph );

FT2_1_3_LOCAL( void )
t1_decoder_done( T1_Decoder  decoder );


FT2_1_3_END_HEADER

#endif /* __T1DECODE_H__ */


/* END */
