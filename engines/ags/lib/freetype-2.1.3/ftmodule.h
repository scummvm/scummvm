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
/*  ftmodule.h                                                             */
/*    FreeType modules public interface (specification).                   */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_FTMODULE_H
#define AGS_LIB_FREETYPE_FTMODULE_H

#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/freetype.h"


namespace AGS3 {
namespace FreeType213 {

FT_BEGIN_HEADER


/* module bit flags */
typedef enum  FT_Module_Flags_ {
	ft_module_font_driver         = 1,     /* this module is a font driver  */
	ft_module_renderer            = 2,     /* this module is a renderer     */
	ft_module_hinter              = 4,     /* this module is a glyph hinter */
	ft_module_styler              = 8,     /* this module is a styler       */

	ft_module_driver_scalable     = 0x100, /* the driver supports scalable  */
	/* fonts                         */
	ft_module_driver_no_outlines  = 0x200, /* the driver does not support   */
	/* vector outlines               */
	ft_module_driver_has_hinter   = 0x400  /* the driver provides its own   */
									/* hinter                        */
} FT_Module_Flags;

typedef void (*FT_Module_Interface)(void);
typedef FT_Error (*FT_Module_Constructor)(FT_Module module);
typedef void (*FT_Module_Destructor)(FT_Module module);
typedef FT_Module_Interface (*FT_Module_Requester)(FT_Module module, const char *name);

typedef struct  FT_Module_Class_ {
	FT_ULong               module_flags;
	FT_Long                module_size;
	const FT_String 	   *module_name;
	FT_Fixed               module_version;
	FT_Fixed               module_requires;

	const void 			   *module_interface;

	FT_Module_Constructor  module_init;
	FT_Module_Destructor   module_done;
	FT_Module_Requester    get_interface;
} FT_Module_Class;


FT_EXPORT(FT_Error)
FT_Add_Module(FT_Library library, const FT_Module_Class *clazz);

FT_EXPORT(FT_Module)
FT_Get_Module(FT_Library library, const char *module_name);

FT_EXPORT(FT_Error)
FT_Remove_Module(FT_Library library, FT_Module module);

FT_EXPORT(FT_Error)
FT_New_Library(FT_Memory memory, FT_Library *alibrary);

FT_EXPORT(FT_Error)
FT_Done_Library(FT_Library library);


typedef void (*FT_DebugHook_Func)(void *arg);

FT_EXPORT(void)
FT_Set_Debug_Hook(FT_Library library, FT_UInt hook_index, FT_DebugHook_Func debug_hook);

FT_EXPORT(void)
FT_Add_Default_Modules(FT_Library library);


FT_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_FTMODULE_H */
