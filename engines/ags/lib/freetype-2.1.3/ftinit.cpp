/***************************************************************************/
/*                                                                         */
/*  ftinit.c                                                               */
/*                                                                         */
/*    FreeType initialization layer (body).                                */
/*                                                                         */
/*  Copyright 1996-2001, 2002 by                                           */
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
/*  The purpose of this file is to implement the following two           */
/*  functions:                                                           */
/*                                                                       */
/*  FT_Add_Default_Modules():                                            */
/*     This function is used to add the set of default modules to a      */
/*     fresh new library object.  The set is taken from the header file  */
/*     `freetype/config/ftmodule.h'.  See the document `FreeType 2.0     */
/*     Build System' for more information.                               */
/*                                                                       */
/*  FT2_1_3_Init_FreeType():                                                  */
/*     This function creates a system object for the current platform,   */
/*     builds a library out of it, then calls FT2_1_3_Default_Drivers().      */
/*                                                                       */
/*  Note that even if FT2_1_3_Init_FreeType() uses the implementation of the  */
/*  system object defined at build time, client applications are still   */
/*  able to provide their own `ftsystem.c'.                              */
/*                                                                       */
/*************************************************************************/


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/config/ftconfig.h"
#include "engines/ags/lib/freetype-2.1.3/ftobjs.h"
#include "engines/ags/lib/freetype-2.1.3/ftdebug.h"
#include "engines/ags/lib/freetype-2.1.3/ftmodule.h"

namespace AGS3 {
namespace FreeType213 {

/*************************************************************************/
/*                                                                       */
/* The macro FT2_1_3_COMPONENT is used in trace mode.  It is an implicit      */
/* parameter of the FT2_1_3_TRACE() and FT2_1_3_ERROR() macros, used to print/log  */
/* messages during execution.                                            */
/*                                                                       */
#undef  FT2_1_3_COMPONENT
#define FT2_1_3_COMPONENT  trace_init

#undef  FT2_1_3_USE_MODULE
#ifdef __cplusplus
#define FT2_1_3_USE_MODULE( x )  extern "C" const FT_Module_Class*  x;
#else
#define FT2_1_3_USE_MODULE( x )  extern const FT_Module_Class*  x;
#endif


#include "engines/ags/lib/freetype-2.1.3/config/ftmodule.h"


#undef  FT2_1_3_USE_MODULE
#define FT2_1_3_USE_MODULE( x )  (const FT_Module_Class*)&x,

static
const FT_Module_Class*  const ft_default_modules[] = {
#include "engines/ags/lib/freetype-2.1.3/config/ftmodule.h"
	0
};


/* documentation is in ftmodule.h */

FT2_1_3_EXPORT_DEF( void )
FT_Add_Default_Modules( FT_Library  library ) {
	FT_Error                       error;
	const FT_Module_Class* const*  cur;


	/* test for valid `library' delayed to FT_Add_Module() */

	cur = ft_default_modules;
	while ( *cur ) {
		error = FT_Add_Module( library, *cur );
		/* notify errors, but don't stop */
		if ( error ) {
			FT2_1_3_ERROR(( "FT2_1_3_Add_Default_Module: Cannot install `%s', error = 0x%x\n",
					   (*cur)->module_name, error ));
		}
		cur++;
	}
}


/* documentation is in freetype.h */

FT2_1_3_EXPORT_DEF( FT_Error )
FT2_1_3_Init_FreeType( FT_Library  *alibrary ) {
	FT_Error   error;
	FT_Memory  memory;


	/* First of all, allocate a new system object -- this function is part */
	/* of the system-specific component, i.e. `ftsystem.c'.                */

	memory = FT_New_Memory();
	if ( !memory ) {
		FT2_1_3_ERROR(( "FT2_1_3_Init_FreeType: cannot find memory manager\n" ));
		return FT2_1_3_Err_Unimplemented_Feature;
	}

	/* build a library out of it, then fill it with the set of */
	/* default drivers.                                        */

	error = FT_New_Library( memory, alibrary );
	if ( !error ) {
		(*alibrary)->version_major = FREETYPE213_MAJOR;
		(*alibrary)->version_minor = FREETYPE213_MINOR;
		(*alibrary)->version_patch = FREETYPE213_PATCH;

		FT_Add_Default_Modules( *alibrary );
	}

	return error;
}


/* documentation is in freetype.h */

FT2_1_3_EXPORT_DEF( FT_Error )
FT2_1_3_Done_FreeType( FT_Library  library ) {
	if ( library ) {
		FT_Memory  memory = library->memory;


		/* Discard the library object */
		FT_Done_Library( library );

		/* discard memory manager */
		FT_Done_Memory( memory );
	}

	return FT2_1_3_Err_Ok;
}

} // End of namespace FreeType213
} // End of namespace AGS3

/* END */
