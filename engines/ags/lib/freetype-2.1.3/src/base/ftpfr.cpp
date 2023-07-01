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


/* check the format */
static FT2_1_3_Error
ft_pfr_check( FT2_1_3_Face           face,
              FT2_1_3_PFR_Service   *aservice ) {
	FT2_1_3_Error  error = FT2_1_3_Err_Bad_Argument;

	if ( face && face->driver ) {
		FT2_1_3_Module    module = (FT2_1_3_Module) face->driver;
		const char*  name   = module->clazz->module_name;

		if ( name[0] == 'p' &&
		        name[1] == 'f' &&
		        name[2] == 'r' &&
		        name[4] == 0   ) {
			*aservice = (FT2_1_3_PFR_Service) module->clazz->module_interface;
			error = 0;
		}
	}
	return error;
}



FT2_1_3_EXPORT_DEF( FT2_1_3_Error )
FT2_1_3_Get_PFR_Metrics( FT2_1_3_Face     face,
                    FT2_1_3_UInt    *aoutline_resolution,
                    FT2_1_3_UInt    *ametrics_resolution,
                    FT2_1_3_Fixed   *ametrics_x_scale,
                    FT2_1_3_Fixed   *ametrics_y_scale ) {
	FT2_1_3_Error        error;
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

FT2_1_3_EXPORT_DEF( FT2_1_3_Error )
FT2_1_3_Get_PFR_Kerning( FT2_1_3_Face     face,
                    FT2_1_3_UInt     left,
                    FT2_1_3_UInt     right,
                    FT2_1_3_Vector  *avector ) {
	FT2_1_3_Error        error;
	FT2_1_3_PFR_Service  service;

	error = ft_pfr_check( face, &service );
	if ( !error ) {
		error = service->get_kerning( face, left, right, avector );
	}
	return error;
}


FT2_1_3_EXPORT_DEF( FT2_1_3_Error )
FT2_1_3_Get_PFR_Advance( FT2_1_3_Face    face,
                    FT2_1_3_UInt    gindex,
                    FT2_1_3_Pos    *aadvance ) {
	FT2_1_3_Error        error;
	FT2_1_3_PFR_Service  service;

	error = ft_pfr_check( face, &service );
	if ( !error ) {
		error = service->get_advance( face, gindex, aadvance );
	}
	return error;
}

/* END */
