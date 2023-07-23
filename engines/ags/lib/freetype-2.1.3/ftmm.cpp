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
/*  ftmm.c                                                                 */
/*    Multiple Master font support (body).                                 */
/*                                                                         */
/***************************************************************************/


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/ftmm.h"
#include "engines/ags/lib/freetype-2.1.3/ftobjs.h"

#undef  FT_COMPONENT
#define FT_COMPONENT  trace_mm

namespace AGS3 {
namespace FreeType213 {

// TODO: Multiple Masters functions are aot used anywhere

FT_EXPORT_DEF(FT_Error)
FT_Get_Multi_Master(FT_Face face, FT_Multi_Master *amaster) {
	FT_Error error;

	if (!face)
		return FT_Err_Invalid_Face_Handle;

	error = FT_Err_Invalid_Argument;

	if (FT_HAS_MULTIPLE_MASTERS(face)) {
		FT_Driver driver = face->driver;
		FT_Get_MM_Func func;

		func = (FT_Get_MM_Func)driver->root.clazz->get_interface(
			FT_MODULE(driver), "get_mm");
		if (func)
			error = func(face, amaster);
	}

	return error;
}

FT_EXPORT_DEF(FT_Error)
FT_Set_MM_Design_Coordinates(FT_Face face, FT_UInt num_coords, FT_Long *coords) {
	FT_Error error;

	if (!face)
		return FT_Err_Invalid_Face_Handle;

	error = FT_Err_Invalid_Argument;

	if (FT_HAS_MULTIPLE_MASTERS(face)) {
		FT_Driver driver = face->driver;
		FT_Set_MM_Design_Func func;

		func = (FT_Set_MM_Design_Func)driver->root.clazz->get_interface(
			FT_MODULE(driver), "set_mm_design");
		if (func)
			error = func(face, num_coords, coords);
	}

	return error;
}


FT_EXPORT_DEF(FT_Error)
FT_Set_MM_Blend_Coordinates(FT_Face face, FT_UInt num_coords, FT_Fixed *coords) {
	FT_Error error;

	if (!face)
		return FT_Err_Invalid_Face_Handle;

	error = FT_Err_Invalid_Argument;

	if (FT_HAS_MULTIPLE_MASTERS(face)) {
		FT_Driver driver = face->driver;
		FT_Set_MM_Blend_Func func;

		func = (FT_Set_MM_Blend_Func)driver->root.clazz->get_interface(
			FT_MODULE(driver), "set_mm_blend");
		if (func)
			error = func(face, num_coords, coords);
	}

	return error;
}

} // End of namespace FreeType213
} // End of namespace AGS3
