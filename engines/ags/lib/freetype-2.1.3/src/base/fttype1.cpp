/***************************************************************************/
/*                                                                         */
/*  fttype1.c                                                              */
/*                                                                         */
/*    FreeType utility file for PS names support (body).                   */
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


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/t1types.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/t42types.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftobjs.h"

namespace AGS3 {
namespace FreeType213 {

/* documentation is in t1tables.h */

FT2_1_3_EXPORT_DEF( FT_Error )
FT_Get_PS_Font_Info( FT_Face          face,
					 PS_FontInfoRec*  afont_info ) {
	PS_FontInfo  font_info = NULL;
	FT_Error     error     = FT2_1_3_Err_Invalid_Argument;
	const char*  driver_name;


	if ( face && face->driver && face->driver->root.clazz ) {
		driver_name = face->driver->root.clazz->module_name;
		if ( ft_strcmp( driver_name, "type1" ) == 0 )
			font_info = &((T1_Face)face)->type1.font_info;
		else if ( ft_strcmp( driver_name, "t1cid" ) == 0 )
			font_info = &((CID_Face)face)->cid.font_info;
		else if ( ft_strcmp( driver_name, "type42" ) == 0 )
			font_info = &((T42_Face)face)->type1.font_info;
	}
	if ( font_info != NULL ) {
		*afont_info = *font_info;
		error = FT2_1_3_Err_Ok;
	}

	return error;
}


/* XXX: Bad hack, but I didn't want to change several drivers here. */

/* documentation is in t1tables.h */

FT2_1_3_EXPORT_DEF( FT_Int )
FT_Has_PS_Glyph_Names( FT_Face  face ) {
	FT_Int       result = 0;
	const char*  driver_name;


	if ( face && face->driver && face->driver->root.clazz ) {
		/* Currently, only the type1, type42, and cff drivers provide */
		/* reliable glyph names...                                    */

		/* We could probably hack the TrueType driver to recognize    */
		/* certain cases where the glyph names are most certainly     */
		/* correct (e.g. using a 20 or 22 format `post' table), but   */
		/* this will probably happen later...                         */

		driver_name = face->driver->root.clazz->module_name;
		result      = ( ft_strcmp( driver_name, "type1"  ) == 0 ||
						ft_strcmp( driver_name, "type42" ) == 0 ||
						ft_strcmp( driver_name, "cff"    ) == 0 );
	}

	return result;
}

} // End of namespace FreeType213
} // End of namespace AGS3

/* END */
