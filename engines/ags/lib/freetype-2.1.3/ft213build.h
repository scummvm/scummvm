/***************************************************************************/
/*                                                                         */
/*  ft2build.h                                                             */
/*                                                                         */
/*    FreeType 2 build and setup macros.                                   */
/*    (Generic version)                                                    */
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


/*************************************************************************/
/*                                                                       */
/* This file corresponds to the default "ft2build.h" file for            */
/* FreeType 2.  It uses the "freetype" include root.                     */
/*                                                                       */
/* Note that specific platforms might use a different configuration.     */
/* See builds/unix/ft2unix.h for an example.                             */
/*                                                                       */
/*************************************************************************/


#ifndef AGS_LIB_FREETYPE_FT213_BUILD_H
#define AGS_LIB_FREETYPE_FT213_BUILD_H

#include "engines/ags/lib/freetype-2.1.3/include/freetype/config/ftheader.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/internal.h"

#ifdef __cplusplus
#define FT2_1_3_BEGIN_HEADER  extern "C" {
#define FT2_1_3_END_HEADER  }
#else
#define FT2_1_3_BEGIN_HEADER  /* nothing */
#define FT2_1_3_END_HEADER
#endif

#endif /* AGS_LIB_FREETYPE_FT213_BUILD_H */