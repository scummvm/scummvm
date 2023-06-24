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


#ifndef __PFRGLOAD_H__
#define __PFRGLOAD_H__

#include "pfrtypes.h"

FT2_1_3_BEGIN_HEADER


FT2_1_3_LOCAL( void )
pfr_glyph_init( PFR_Glyph       glyph,
                FT2_1_3_GlyphLoader  loader );

FT2_1_3_LOCAL( void )
pfr_glyph_done( PFR_Glyph  glyph );


FT2_1_3_LOCAL( FT2_1_3_Error )
pfr_glyph_load( PFR_Glyph  glyph,
                FT2_1_3_Stream  stream,
                FT2_1_3_ULong   gps_offset,
                FT2_1_3_ULong   offset,
                FT2_1_3_ULong   size );


FT2_1_3_END_HEADER


#endif /* __PFRGLOAD_H__ */


/* END */
