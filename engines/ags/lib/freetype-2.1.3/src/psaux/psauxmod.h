/***************************************************************************/
/*                                                                         */
/*  psauxmod.h                                                             */
/*                                                                         */
/*    FreeType auxiliary PostScript module implementation (specification). */
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


#ifndef AGS_LIB_FREETYPE_PSAUXMOD_H
#define AGS_LIB_FREETYPE_PSAUXMOD_H


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/ftmodule.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER


FT2_1_3_EXPORT_VAR( const FT2_1_3_Module_Class )  psaux_driver_class;


FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_PSAUXMOD_H */


/* END */
