/***************************************************************************/
/*                                                                         */
/*  cffload.h                                                              */
/*                                                                         */
/*    OpenType & CFF data/program tables loader (specification).           */
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


#ifndef __CFFLOAD_H__
#define __CFFLOAD_H__


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/cfftypes.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/psnames.h"


FT2_1_3_BEGIN_HEADER

FT2_1_3_LOCAL( FT2_1_3_UShort )
cff_get_standard_encoding( FT2_1_3_UInt  charcode );


FT2_1_3_LOCAL( FT2_1_3_String* )
cff_index_get_name( CFF_Index  idx,
                    FT2_1_3_UInt    element );

FT2_1_3_LOCAL( FT2_1_3_String* )
cff_index_get_sid_string( CFF_Index        idx,
                          FT2_1_3_UInt          sid,
                          PSNames_Service  psnames_interface );


FT2_1_3_LOCAL( FT2_1_3_Error )
cff_index_access_element( CFF_Index  idx,
                          FT2_1_3_UInt    element,
                          FT2_1_3_Byte**  pbytes,
                          FT2_1_3_ULong*  pbyte_len );

FT2_1_3_LOCAL( void )
cff_index_forget_element( CFF_Index  idx,
                          FT2_1_3_Byte**  pbytes );


FT2_1_3_LOCAL( FT2_1_3_Error )
cff_font_load( FT2_1_3_Stream  stream,
               FT2_1_3_Int     face_index,
               CFF_Font   font );

FT2_1_3_LOCAL( void )
cff_font_done( CFF_Font  font );


FT2_1_3_LOCAL( FT2_1_3_Byte )
cff_fd_select_get( CFF_FDSelect  select,
                   FT2_1_3_UInt       glyph_index );


FT2_1_3_END_HEADER

#endif /* __CFFLOAD_H__ */


/* END */
