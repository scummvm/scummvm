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


#ifndef __FTRASTER_H__
#define __FTRASTER_H__


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include FT2_1_3_CONFIG_CONFIG_H
#include FT2_1_3_IMAGE_H


FT2_1_3_BEGIN_HEADER


/*************************************************************************/
/*                                                                       */
/* Uncomment the following line if you are using ftraster.c as a         */
/* standalone module, fully independent of FreeType.                     */
/*                                                                       */
/* #define _STANDALONE_ */

FT2_1_3_EXPORT_VAR( const FT2_1_3_Raster_Funcs )  ft_standard_raster;


FT2_1_3_END_HEADER

#endif /* __FTRASTER_H__ */


/* END */
