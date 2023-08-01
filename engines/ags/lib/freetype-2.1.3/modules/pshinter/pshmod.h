/***************************************************************************/
/*                                                                         */
/*  pshmod.h                                                               */
/*                                                                         */
/*    PostScript hinter module interface (specification).                  */
/*                                                                         */
/*  Copyright 2001 by                                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_PSHMOD_H
#define AGS_LIB_FREETYPE_PSHMOD_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/ftmodule.h"

namespace AGS3 {
namespace FreeType213 {

// FT_BEGIN_HEADER


FT_EXPORT_VAR( const FT_Module_Class )  pshinter_module_class;


// FT_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_PSHMOD_H */


/* END */
