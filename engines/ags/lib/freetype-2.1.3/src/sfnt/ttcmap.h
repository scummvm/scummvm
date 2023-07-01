/***************************************************************************/
/*                                                                         */
/*  ttcmap.h                                                               */
/*                                                                         */
/*    TrueType character mapping table (cmap) support (specification).     */
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


#ifndef __TTCMAP_H__
#define __TTCMAP_H__


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/tttypes.h"


FT2_1_3_BEGIN_HEADER


FT2_1_3_LOCAL( FT2_1_3_Error )
tt_face_load_charmap( TT_Face       face,
                      TT_CMapTable  cmap,
                      FT2_1_3_Stream     input );

FT2_1_3_LOCAL( FT2_1_3_Error )
tt_face_free_charmap( TT_Face       face,
                      TT_CMapTable  cmap );


FT2_1_3_END_HEADER

#endif /* __TTCMAP_H__ */


/* END */
