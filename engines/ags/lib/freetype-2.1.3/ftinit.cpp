/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

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

#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/config/ftconfig.h"
#include "engines/ags/lib/freetype-2.1.3/ftobjs.h"
#include "engines/ags/lib/freetype-2.1.3/ftdebug.h"
#include "engines/ags/lib/freetype-2.1.3/ftmodule.h"

namespace AGS3 {
namespace FreeType213 {

#undef FT_COMPONENT
#define FT_COMPONENT trace_init

#undef FT_USE_MODULE
#ifdef __cplusplus
#define FT_USE_MODULE(x) extern const FT_Module_Class x;
#define FT_USE_DRIVER(x) extern const FT_Driver_ClassRec x;
#define FT_USE_RENDERER(x) extern const FT_Renderer_Class x;
#else
#define FT_USE_MODULE(x) extern const FT_Module_Class *x;
#define FT_USE_DRIVER(x) extern const FT_Driver_ClassRec *x;
#define FT_USE_RENDERER(x) extern const FT_Renderer_Class *x;
#endif

#include "engines/ags/lib/freetype-2.1.3/config/ftmodule.h"

#undef FT_USE_MODULE
#undef FT_USE_DRIVER
#undef FT_USE_RENDERER
#define FT_USE_MODULE(x) &x,
#define FT_USE_DRIVER(x) &x.root,
#define FT_USE_RENDERER(x) &x.root,

static const FT_Module_Class *const ft_default_modules[] = {
	#include "engines/ags/lib/freetype-2.1.3/config/ftmodule.h"
	0
};


FT_EXPORT_DEF(void)
FT_Add_Default_Modules(FT_Library library) {
	FT_Error error;
	const FT_Module_Class *const *cur;

	/* test for valid `library' delayed to FT_Add_Module() */

	cur = ft_default_modules;
	while (*cur) {
		error = FT_Add_Module(library, *cur);
		/* notify errors, but don't stop */
		if (error) {
			FT_ERROR(("FT2_1_3_Add_Default_Module: Cannot install `%s', error = 0x%x\n", (*cur)->module_name, error));
		}
		cur++;
	}
}


FT_EXPORT_DEF(FT_Error)
FT2_1_3_Init_FreeType(FT_Library *alibrary) {
	FT_Error error;
	FT_Memory memory;

	/* First of all, allocate a new system object -- this function is part */
	/* of the system-specific component, i.e. `ftsystem.c'.                */

	memory = FT_New_Memory();
	if (!memory) {
		FT_ERROR(("FT2_1_3_Init_FreeType: cannot find memory manager\n"));
		return FT_Err_Unimplemented_Feature;
	}

	/* build a library out of it, then fill it with the set of */
	/* default drivers.                                        */

	error = FT_New_Library(memory, alibrary);
	if (!error) {
		(*alibrary)->version_major = FREETYPE213_MAJOR;
		(*alibrary)->version_minor = FREETYPE213_MINOR;
		(*alibrary)->version_patch = FREETYPE213_PATCH;

		FT_Add_Default_Modules(*alibrary);
	}

	return error;
}


FT_EXPORT_DEF(FT_Error)
FT2_1_3_Done_FreeType(FT_Library library) {
	if (library) {
		FT_Memory memory = library->memory;

		/* Discard the library object */
		FT_Done_Library(library);

		/* discard memory manager */
		FT_Done_Memory(memory);
	}

	return FT_Err_Ok;
}

} // End of namespace FreeType213
} // End of namespace AGS3
