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
/*  ftbdf.c                                                                */
/*                                                                         */
/*    FreeType API for accessing BDF-specific strings (body).              */
/*                                                                         */
/***************************************************************************/


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/bdftypes.h"
#include "engines/ags/lib/freetype-2.1.3/ftobjs.h"

namespace AGS3 {
namespace FreeType213 {

FT_EXPORT_DEF(FT_Error)
FT_Get_BDF_Charset_ID(FT_Face face, const char **acharset_encoding, const char **acharset_registry) {
	FT_Error error;
	const char *encoding = NULL;
	const char *registry = NULL;

	error = FT_Err_Invalid_Argument;

	if (face != NULL && face->driver != NULL) {
		FT_Module driver = (FT_Module)face->driver;

		if (driver->clazz && driver->clazz->module_name &&
			ft_strcmp(driver->clazz->module_name, "bdf") == 0) {
			BDF_Public_Face bdf_face = (BDF_Public_Face)face;

			encoding = (const char *)bdf_face->charset_encoding;
			registry = (const char *)bdf_face->charset_registry;
			error = 0;
		}
	}

	if (acharset_encoding)
		*acharset_encoding = encoding;

	if (acharset_registry)
		*acharset_registry = registry;

	return error;
}

} // End of namespace FreeType213
} // End of namespace AGS3
