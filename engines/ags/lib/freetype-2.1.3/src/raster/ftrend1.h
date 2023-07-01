/***************************************************************************/
/*                                                                         */
/*  ftrend1.h                                                              */
/*                                                                         */
/*    The FreeType glyph rasterizer interface (specification).             */
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


#ifndef __FTREND1_H__
#define __FTREND1_H__


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/ftrender.h"


FT2_1_3_BEGIN_HEADER


FT2_1_3_EXPORT_VAR( const FT2_1_3_Renderer_Class )  ft_raster1_renderer_class;

/* this renderer is _NOT_ part of the default modules, you'll need */
/* to register it by hand in your application.  It should only be  */
/* used for backwards-compatibility with FT 1.x anyway.            */
/*                                                                 */
FT2_1_3_EXPORT_VAR( const FT2_1_3_Renderer_Class )  ft_raster5_renderer_class;


FT2_1_3_END_HEADER

#endif /* __FTREND1_H__ */


/* END */
