/***************************************************************************/
/*                                                                         */
/*  ftbdf.c                                                                */
/*                                                                         */
/*    FreeType API for accessing BDF-specific strings (body).              */
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
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/bdftypes.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftobjs.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_EXPORT_DEF( FT_Error )
FT2_1_3_Get_BDF_Charset_ID( FT_Face       face,
					   const char*  *acharset_encoding,
					   const char*  *acharset_registry ) {
	FT_Error     error;
	const char*  encoding = NULL;
	const char*  registry = NULL;


	error = FT2_1_3_Err_Invalid_Argument;

	if ( face != NULL && face->driver != NULL ) {
		FT2_1_3_Module  driver = (FT2_1_3_Module) face->driver;


		if ( driver->clazz && driver->clazz->module_name         &&
				ft_strcmp( driver->clazz->module_name, "bdf" ) == 0 ) {
			BDF_Public_Face  bdf_face = (BDF_Public_Face)face;


			encoding = (const char*) bdf_face->charset_encoding;
			registry = (const char*) bdf_face->charset_registry;
			error    = 0;
		}
	}

	if ( acharset_encoding )
		*acharset_encoding = encoding;

	if ( acharset_registry )
		*acharset_registry = registry;

	return error;
}

} // End of namespace FreeType213
} // End of namespace AGS3

/* END */
