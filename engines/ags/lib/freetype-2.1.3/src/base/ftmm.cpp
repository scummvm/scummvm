/***************************************************************************/
/*                                                                         */
/*  ftmm.c                                                                 */
/*                                                                         */
/*    Multiple Master font support (body).                                 */
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


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/ftmm.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftobjs.h"


/*************************************************************************/
/*                                                                       */
/* The macro FT2_1_3_COMPONENT is used in trace mode.  It is an implicit      */
/* parameter of the FT2_1_3_TRACE() and FT2_1_3_ERROR() macros, used to print/log  */
/* messages during execution.                                            */
/*                                                                       */
#undef  FT2_1_3_COMPONENT
#define FT2_1_3_COMPONENT  trace_mm

namespace AGS3 {
namespace FreeType213 {

/* documentation is in ftmm.h */

FT2_1_3_EXPORT_DEF( FT_Error )
FT_Get_Multi_Master( FT_Face           face,
					 FT_Multi_Master  *amaster ) {
	FT_Error  error;


	if ( !face )
		return FT2_1_3_Err_Invalid_Face_Handle;

	error = FT2_1_3_Err_Invalid_Argument;

	if ( FT2_1_3_HAS_MULTIPLE_MASTERS( face ) ) {
		FT_Driver       driver = face->driver;
		FT_Get_MM_Func  func;


		func = (FT_Get_MM_Func)driver->root.clazz->get_interface(
				   FT2_1_3_MODULE( driver ), "get_mm" );
		if ( func )
			error = func( face, amaster );
	}

	return error;
}


/* documentation is in ftmm.h */

FT2_1_3_EXPORT_DEF( FT_Error )
FT_Set_MM_Design_Coordinates( FT_Face   face,
							  FT_UInt   num_coords,
							  FT_Long*  coords ) {
	FT_Error  error;


	if ( !face )
		return FT2_1_3_Err_Invalid_Face_Handle;

	error = FT2_1_3_Err_Invalid_Argument;

	if ( FT2_1_3_HAS_MULTIPLE_MASTERS( face ) ) {
		FT_Driver              driver = face->driver;
		FT_Set_MM_Design_Func  func;


		func = (FT_Set_MM_Design_Func)driver->root.clazz->get_interface(
				   FT2_1_3_MODULE( driver ), "set_mm_design" );
		if ( func )
			error = func( face, num_coords, coords );
	}

	return error;
}


/* documentation is in ftmm.h */

FT2_1_3_EXPORT_DEF( FT_Error )
FT_Set_MM_Blend_Coordinates( FT_Face    face,
							 FT_UInt    num_coords,
							 FT_Fixed*  coords ) {
	FT_Error  error;


	if ( !face )
		return FT2_1_3_Err_Invalid_Face_Handle;

	error = FT2_1_3_Err_Invalid_Argument;

	if ( FT2_1_3_HAS_MULTIPLE_MASTERS( face ) ) {
		FT_Driver             driver = face->driver;
		FT_Set_MM_Blend_Func  func;


		func = (FT_Set_MM_Blend_Func)driver->root.clazz->get_interface(
				   FT2_1_3_MODULE( driver ), "set_mm_blend" );
		if ( func )
			error = func( face, num_coords, coords );
	}

	return error;
}

} // End of namespace FreeType213
} // End of namespace AGS3

/* END */
