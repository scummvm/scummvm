/***************************************************************************/
/*                                                                         */
/*  sfobjs.h                                                               */
/*                                                                         */
/*    SFNT object management (specification).                              */
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


#ifndef __SFOBJS_H__
#define __SFOBJS_H__


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/sfnt.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftobjs.h"


FT2_1_3_BEGIN_HEADER


FT2_1_3_LOCAL( FT2_1_3_Error )
sfnt_init_face( FT2_1_3_Stream      stream,
                TT_Face        face,
                FT2_1_3_Int         face_index,
                FT2_1_3_Int         num_params,
                FT2_1_3_Parameter*  params );

FT2_1_3_LOCAL( FT2_1_3_Error )
sfnt_load_face( FT2_1_3_Stream      stream,
                TT_Face        face,
                FT2_1_3_Int         face_index,
                FT2_1_3_Int         num_params,
                FT2_1_3_Parameter*  params );

FT2_1_3_LOCAL( void )
sfnt_done_face( TT_Face  face );


FT2_1_3_END_HEADER

#endif /* __SFDRIVER_H__ */


/* END */
