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
/*  t1driver.c                                                             */
/*    Type 1 driver interface (body).                                      */
/*                                                                         */
/***************************************************************************/


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/modules/type1/t1driver.h"
#include "engines/ags/lib/freetype-2.1.3/modules/type1/t1gload.h"
#include "engines/ags/lib/freetype-2.1.3/modules/type1/t1load.h"

#include "engines/ags/lib/freetype-2.1.3/modules/type1/t1errors.h"

#ifndef T1_CONFIG_OPTION_NO_AFM
#include "engines/ags/lib/freetype-2.1.3/modules/type1/t1afm.h"
#endif

#include "engines/ags/lib/freetype-2.1.3/ftdebug.h"
#include "engines/ags/lib/freetype-2.1.3/ftstream.h"
#include "engines/ags/lib/freetype-2.1.3/psnames.h"

#undef  FT_COMPONENT
#define FT_COMPONENT  trace_t1driver

namespace AGS3 {
namespace FreeType213 {


static FT_Error t1_get_glyph_name(T1_Face face, FT_UInt glyph_index, FT_Pointer buffer, FT_UInt buffer_max) {
	FT_String *gname;

	gname = face->type1.glyph_names[glyph_index];

	if (buffer_max > 0) {
		FT_UInt len = (FT_UInt)(ft_strlen(gname));

		if (len >= buffer_max)
			len = buffer_max - 1;

		FT_MEM_COPY(buffer, gname, len);
		((FT_Byte *)buffer)[len] = 0;
	}

	return FT_Err_Ok;
}


static FT_UInt t1_get_name_index(T1_Face face, FT_String *glyph_name) {
	FT_Int i;
	FT_String *gname;

	for (i = 0; i < face->type1.num_glyphs; i++) {
		gname = face->type1.glyph_names[i];

		if (!ft_strcmp(glyph_name, gname))
			return (FT_UInt)i;
	}

	return 0;
}


static const char *t1_get_ps_name(T1_Face face) {
	return (const char *)face->type1.font_name;
}


static FT_Module_Interface Get_Interface(FT_Driver driver, const FT_String *t1_interface) {
	FT_UNUSED(driver);
	FT_UNUSED(t1_interface);

	if (ft_strcmp((const char *)t1_interface, "glyph_name") == 0)
		return (FT_Module_Interface)t1_get_glyph_name;

	if (ft_strcmp((const char *)t1_interface, "name_index") == 0)
		return (FT_Module_Interface)t1_get_name_index;

	if (ft_strcmp((const char *)t1_interface, "postscript_name") == 0)
		return (FT_Module_Interface)t1_get_ps_name;

#ifndef T1_CONFIG_OPTION_NO_MM_SUPPORT
	if (ft_strcmp((const char *)t1_interface, "get_mm") == 0)
		return (FT_Module_Interface)T1_Get_Multi_Master;

	if (ft_strcmp((const char *)t1_interface, "set_mm_design") == 0)
		return (FT_Module_Interface)T1_Set_MM_Design;

	if (ft_strcmp((const char *)t1_interface, "set_mm_blend") == 0)
		return (FT_Module_Interface)T1_Set_MM_Blend;
#endif
	return 0;
}


#ifndef T1_CONFIG_OPTION_NO_AFM

static FT_Error Get_Kerning(T1_Face face, FT_UInt left_glyph, FT_UInt right_glyph, FT_Vector *kerning) {
	T1_AFM *afm;

	kerning->x = 0;
	kerning->y = 0;

	afm = const_cast<T1_AFM *>(static_cast<const T1_AFM *>(face->afm_data));
	if (afm)
		T1_Get_Kerning(afm, left_glyph, right_glyph, kerning);

	return FT_Err_Ok;
}

#endif /* T1_CONFIG_OPTION_NO_AFM */


FT_CALLBACK_TABLE_DEF
const FT_Driver_ClassRec t1_driver_class = {
	{
		ft_module_font_driver | ft_module_driver_scalable | ft_module_driver_has_hinter,

		sizeof(FT_DriverRec),

		"type1",
		0x10000L,
		0x20000L,

		0, /* format interface */

		(FT_Module_Constructor) T1_Driver_Init,
		(FT_Module_Destructor)	T1_Driver_Done,
		(FT_Module_Requester)	Get_Interface,
	},

	sizeof(T1_FaceRec),
	sizeof(T1_SizeRec),
	sizeof(T1_GlyphSlotRec),

	(FT_Face_InitFunc) T1_Face_Init,
	(FT_Face_DoneFunc) T1_Face_Done,
	(FT_Size_InitFunc) T1_Size_Init,
	(FT_Size_DoneFunc) T1_Size_Done,
	(FT_Slot_InitFunc) T1_GlyphSlot_Init,
	(FT_Slot_DoneFunc) T1_GlyphSlot_Done,

	(FT_Size_ResetPointsFunc) T1_Points_Size_Reset,
	(FT_Size_ResetPixelsFunc) T1_Pixels_Size_Reset,
	(FT_Slot_LoadFunc)		  T1_Load_Glyph,

#ifdef T1_CONFIG_OPTION_NO_AFM
	(FT_Face_GetKerningFunc) 0,
	(FT_Face_AttachFunc) 0,
#else
	(FT_Face_GetKerningFunc) Get_Kerning,
	(FT_Face_AttachFunc)	 T1_Read_AFM,
#endif
	(FT_Face_GetAdvancesFunc) 0
};


} // End of namespace FreeType213
} // End of namespace AGS3
