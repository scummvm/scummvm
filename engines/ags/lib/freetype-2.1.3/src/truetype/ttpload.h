/***************************************************************************/
/*                                                                         */
/*  ttpload.h                                                              */
/*                                                                         */
/*    TrueType glyph data/program tables loader (specification).           */
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


#ifndef __TTPLOAD_H__
#define __TTPLOAD_H__


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include FT2_1_3_INTERNAL_TRUETYPE_TYPES_H


FT2_1_3_BEGIN_HEADER


FT2_1_3_LOCAL( FT2_1_3_Error )
tt_face_load_loca( TT_Face    face,
                   FT2_1_3_Stream  stream );

FT2_1_3_LOCAL( FT2_1_3_Error )
tt_face_load_cvt( TT_Face    face,
                  FT2_1_3_Stream  stream );

FT2_1_3_LOCAL( FT2_1_3_Error )
tt_face_load_fpgm( TT_Face    face,
                   FT2_1_3_Stream  stream );


FT2_1_3_END_HEADER

#endif /* __TTPLOAD_H__ */


/* END */
