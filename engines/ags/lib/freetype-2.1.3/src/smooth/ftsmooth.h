/***************************************************************************/
/*                                                                         */
/*  ftsmooth.h                                                             */
/*                                                                         */
/*    Anti-aliasing renderer interface (specification).                    */
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


#ifndef __FTSMOOTH_H__
#define __FTSMOOTH_H__


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/ftrender.h"


FT2_1_3_BEGIN_HEADER


#ifndef FT2_1_3_CONFIG_OPTION_NO_STD_RASTER
FT2_1_3_EXPORT_VAR( const FT2_1_3_Renderer_Class )  ft_std_renderer_class;
#endif

#ifndef FT2_1_3_CONFIG_OPTION_NO_SMOOTH_RASTER
FT2_1_3_EXPORT_VAR( const FT2_1_3_Renderer_Class )  ft_smooth_renderer_class;

FT2_1_3_EXPORT_VAR( const FT2_1_3_Renderer_Class )  ft_smooth_lcd_renderer_class;

FT2_1_3_EXPORT_VAR( const FT2_1_3_Renderer_Class )  ft_smooth_lcd_v_renderer_class;
#endif



FT2_1_3_END_HEADER

#endif /* __FTSMOOTH_H__ */


/* END */
