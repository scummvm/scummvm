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
/*  fttype1.c                                                              */
/*    FreeType utility file for PS names support (body).                   */
/*                                                                         */
/***************************************************************************/


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/t1types.h"
#include "engines/ags/lib/freetype-2.1.3/t42types.h"
#include "engines/ags/lib/freetype-2.1.3/ftobjs.h"

namespace AGS3 {
namespace FreeType213 {


FT_EXPORT_DEF(FT_Error)
FT_Get_PS_Font_Info(FT_Face face, PS_FontInfoRec *afont_info) {
	PS_FontInfo font_info = NULL;
	FT_Error error = FT_Err_Invalid_Argument;
	const char *driver_name;

	if (face && face->driver && face->driver->root.clazz) {
		driver_name = face->driver->root.clazz->module_name;
		if (ft_strcmp(driver_name, "type1") == 0)
			font_info = &((T1_Face)face)->type1.font_info;
		else if (ft_strcmp(driver_name, "t1cid") == 0)
			font_info = &((CID_Face)face)->cid.font_info;
		else if (ft_strcmp(driver_name, "type42") == 0)
			font_info = &((T42_Face)face)->type1.font_info;
	}
	if (font_info != NULL) {
		*afont_info = *font_info;
		error = FT_Err_Ok;
	}

	return error;
}


FT_EXPORT_DEF(FT_Int)
FT_Has_PS_Glyph_Names(FT_Face face) {
	FT_Int result = 0;
	const char *driver_name;

	if (face && face->driver && face->driver->root.clazz) {
		/* Currently, only the type1, type42, and cff drivers provide */
		/* reliable glyph names...                                    */

		/* We could probably hack the TrueType driver to recognize    */
		/* certain cases where the glyph names are most certainly     */
		/* correct (e.g. using a 20 or 22 format `post' table), but   */
		/* this will probably happen later...                         */

		driver_name = face->driver->root.clazz->module_name;
		result = (ft_strcmp(driver_name, "type1") == 0 ||
				  ft_strcmp(driver_name, "type42") == 0 ||
				  ft_strcmp(driver_name, "cff") == 0);
	}

	return result;
}


} // End of namespace FreeType213
} // End of namespace AGS3
