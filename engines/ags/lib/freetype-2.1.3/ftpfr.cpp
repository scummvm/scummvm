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
/*  ftpfr.c                                                                */
/*    FreeType API for accessing PFR-specific data                         */
/*                                                                         */
/***************************************************************************/

#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/pfr.h"
#include "engines/ags/lib/freetype-2.1.3/ftobjs.h"

namespace AGS3 {
namespace FreeType213 {


/* check the format */
static FT_Error ft_pfr_check(FT_Face face, FT2_1_3_PFR_Service *aservice) {
	FT_Error error = FT_Err_Bad_Argument;

	if (face && face->driver) {
		FT_Module module = (FT_Module)face->driver;
		const char *name = module->clazz->module_name;

		if (name[0] == 'p' && name[1] == 'f' && name[2] == 'r' && name[4] == 0) {
			*aservice = const_cast<FT2_1_3_PFR_Service>(reinterpret_cast<const FT2_1_3_PFR_ServiceRec_ *>(module->clazz->module_interface));
			error = 0;
		}
	}
	return error;
}

FT_EXPORT_DEF(FT_Error)
FT_Get_PFR_Metrics(FT_Face face, FT_UInt *aoutline_resolution, FT_UInt *ametrics_resolution,
				   FT_Fixed *ametrics_x_scale, FT_Fixed *ametrics_y_scale) {
	FT_Error error;
	FT2_1_3_PFR_Service service;

	error = ft_pfr_check(face, &service);
	if (!error) {
		error = service->get_metrics(face, aoutline_resolution, ametrics_resolution, ametrics_x_scale, ametrics_y_scale);
	}
	return error;
}

FT_EXPORT_DEF(FT_Error)
FT_Get_PFR_Kerning(FT_Face face, FT_UInt left, FT_UInt right, FT_Vector *avector) {
	FT_Error error;
	FT2_1_3_PFR_Service service;

	error = ft_pfr_check(face, &service);
	if (!error) {
		error = service->get_kerning(face, left, right, avector);
	}
	return error;
}

FT_EXPORT_DEF(FT_Error)
FT_Get_PFR_Advance(FT_Face face, FT_UInt gindex, FT_Pos *aadvance) {
	FT_Error error;
	FT2_1_3_PFR_Service service;

	error = ft_pfr_check(face, &service);
	if (!error) {
		error = service->get_advance(face, gindex, aadvance);
	}
	return error;
}

} // End of namespace FreeType213
} // End of namespace AGS3
