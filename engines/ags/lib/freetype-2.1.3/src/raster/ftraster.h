/***************************************************************************/
/*                                                                         */
/*  ftraster.h                                                             */
/*                                                                         */
/*    The FreeType glyph rasterizer (specification).                       */
/*                                                                         */
/*  Copyright 1996-2001 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used        */
/*  modified and distributed under the terms of the FreeType project       */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_FTRASTER_H
#define AGS_LIB_FREETYPE_FTRASTER_H


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/config/ftconfig.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/ftimage.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER


/*************************************************************************/
/*                                                                       */
/* Uncomment the following line if you are using ftraster.c as a         */
/* standalone module, fully independent of FreeType.                     */
/*                                                                       */
/* #define _STANDALONE_ */

FT2_1_3_EXPORT_VAR( const FT_Raster_Funcs )  ft_standard_raster;


FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_FTRASTER_H */


/* END */
