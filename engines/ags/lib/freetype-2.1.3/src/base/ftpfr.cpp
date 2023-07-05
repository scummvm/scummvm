/***************************************************************************/
/*                                                                         */
/*  ftpfr.c                                                                */
/*                                                                         */
/*    FreeType API for accessing PFR-specific data                         */
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
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/pfr.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftobjs.h"

namespace AGS3 {
namespace FreeType213 {

/* check the format */
static FT_Error
ft_pfr_check( FT_Face           face,
			  FT2_1_3_PFR_Service   *aservice ) {
	FT_Error  error = FT2_1_3_Err_Bad_Argument;

	if ( face && face->driver ) {
		FT2_1_3_Module    module = (FT2_1_3_Module) face->driver;
		const char*  name   = module->clazz->module_name;

		if ( name[0] == 'p' &&
				name[1] == 'f' &&
				name[2] == 'r' &&
				name[4] == 0   ) {
			*aservice = const_cast<FT2_1_3_PFR_Service>(reinterpret_cast<const FT2_1_3_PFR_ServiceRec_ *>(module->clazz->module_interface));
			error = 0;
		}
	}
	return error;
}



FT2_1_3_EXPORT_DEF( FT_Error )
FT2_1_3_Get_PFR_Metrics( FT_Face     face,
					FT_UInt    *aoutline_resolution,
					FT_UInt    *ametrics_resolution,
					FT_Fixed   *ametrics_x_scale,
					FT_Fixed   *ametrics_y_scale ) {
	FT_Error        error;
	FT2_1_3_PFR_Service  service;

	error = ft_pfr_check( face, &service );
	if ( !error ) {
		error = service->get_metrics( face,
									  aoutline_resolution,
									  ametrics_resolution,
									  ametrics_x_scale,
									  ametrics_y_scale );
	}
	return error;
}

FT2_1_3_EXPORT_DEF( FT_Error )
FT2_1_3_Get_PFR_Kerning( FT_Face     face,
					FT_UInt     left,
					FT_UInt     right,
					FT_Vector  *avector ) {
	FT_Error        error;
	FT2_1_3_PFR_Service  service;

	error = ft_pfr_check( face, &service );
	if ( !error ) {
		error = service->get_kerning( face, left, right, avector );
	}
	return error;
}


FT2_1_3_EXPORT_DEF( FT_Error )
FT2_1_3_Get_PFR_Advance( FT_Face    face,
					FT_UInt    gindex,
					FT_Pos    *aadvance ) {
	FT_Error        error;
	FT2_1_3_PFR_Service  service;

	error = ft_pfr_check( face, &service );
	if ( !error ) {
		error = service->get_advance( face, gindex, aadvance );
	}
	return error;
}

} // End of namespace FreeType213
} // End of namespace AGS3

/* END */
