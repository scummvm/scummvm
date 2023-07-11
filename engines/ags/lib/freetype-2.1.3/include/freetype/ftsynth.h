/***************************************************************************/
/*                                                                         */
/*  ftsynth.h                                                              */
/*                                                                         */
/*    FreeType synthesizing code for emboldening and slanting              */
/*    (specification).                                                     */
/*                                                                         */
/*  Copyright 2000-2001 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
/*********                                                       *********/
/*********        WARNING, THIS IS ALPHA CODE, THIS API          *********/
/*********    IS DUE TO CHANGE UNTIL STRICTLY NOTIFIED BY THE    *********/
/*********            FREETYPE DEVELOPMENT TEAM                  *********/
/*********                                                       *********/
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/


#ifndef AGS_LIB_FREETYPE_FTSYNTH_H
#define AGS_LIB_FREETYPE_FTSYNTH_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/freetype.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER


/* This code is completely experimental -- use with care! */
/* It will probably be completely rewritten in the future */
/* or even integrated into the library.                   */
FT2_1_3_EXPORT( void )
FT_GlyphSlot_Embolden( FT_GlyphSlot  slot );


FT2_1_3_EXPORT( void )
FT_GlyphSlot_Oblique( FT_GlyphSlot  slot );

/* */

FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_FTSYNTH_H */


/* END */
