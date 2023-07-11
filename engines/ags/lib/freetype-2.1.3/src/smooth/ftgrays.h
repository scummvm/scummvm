/***************************************************************************/
/*                                                                         */
/*  ftgrays.h                                                              */
/*                                                                         */
/*    FreeType smooth renderer declaration                                 */
/*                                                                         */
/*  Copyright 1996-2001 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_FTGRAYS_H
#define AGS_LIB_FREETYPE_FTGRAYS_H

#ifdef __cplusplus
extern "C" {
#endif


#ifdef _STANDALONE_
#include "ftimage.h"
#else
#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/ftimage.h"
#endif

namespace AGS3 {
namespace FreeType213 {

/*************************************************************************/
/*                                                                       */
/* To make ftgrays.h independent from configuration files we check       */
/* whether FT2_1_3_EXPORT_VAR has been defined already.                       */
/*                                                                       */
/* On some systems and compilers (Win32 mostly), an extra keyword is     */
/* necessary to compile the library as a DLL.                            */
/*                                                                       */
#ifndef FT2_1_3_EXPORT_VAR
#define FT2_1_3_EXPORT_VAR( x )  extern  x
#endif

FT2_1_3_EXPORT_VAR( const FT_Raster_Funcs )  ft_grays_raster;


#ifdef __cplusplus
}
#endif

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_FTGRAYS_H */


/* END */
