/***************************************************************************/
/*                                                                         */
/*  ftxf86.h                                                               */
/*                                                                         */
/*    Support functions for X11.                                           */
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


#ifndef AGS_LIB_FREETYPE_FTXF86_H
#define AGS_LIB_FREETYPE_FTXF86_H

#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/freetype.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER

/* this comment is intentionally disabled for now, to prevent this       */
/* function from appearing in the API Reference.                         */

/*@***********************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    FT2_1_3_Get_X11_Font_Format                                             */
/*                                                                       */
/* <Description>                                                         */
/*    Return a string describing the format of a given face as an X11    */
/*    FONT_PROPERTY.  It should only be used by the FreeType 2 font      */
/*    backend of the XFree86 font server.                                */
/*                                                                       */
/* <Input>                                                               */
/*    face :: Input face handle.                                         */
/*                                                                       */
/* <Return>                                                              */
/*    Font format string.  NULL in case of error.                        */
/*                                                                       */
FT2_1_3_EXPORT_DEF( const char* )
FT2_1_3_Get_X11_Font_Format( FT_Face  face );

/* */

FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_FTXF86_H */
