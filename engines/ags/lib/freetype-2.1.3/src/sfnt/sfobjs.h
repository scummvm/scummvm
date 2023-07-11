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


#ifndef AGS_LIB_FREETYPE_SFOBJS_H
#define AGS_LIB_FREETYPE_SFOBJS_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/sfnt.h"
#include "engines/ags/lib/freetype-2.1.3/ftobjs.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER


FT2_1_3_LOCAL( FT_Error )
sfnt_init_face( FT_Stream      stream,
				TT_Face        face,
				FT_Int         face_index,
				FT_Int         num_params,
				FT_Parameter*  params );

FT2_1_3_LOCAL( FT_Error )
sfnt_load_face( FT_Stream      stream,
				TT_Face        face,
				FT_Int         face_index,
				FT_Int         num_params,
				FT_Parameter*  params );

FT2_1_3_LOCAL( void )
sfnt_done_face( TT_Face  face );


FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_SFOBJS_H */


/* END */
