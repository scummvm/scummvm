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
/*  cffdrivr.c                                                             */
/*    OpenType font driver implementation (body).                          */
/*                                                                         */
/***************************************************************************/


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/freetype.h"
#include "engines/ags/lib/freetype-2.1.3/ftdebug.h"
#include "engines/ags/lib/freetype-2.1.3/ftstream.h"
#include "engines/ags/lib/freetype-2.1.3/sfnt.h"
#include "engines/ags/lib/freetype-2.1.3/ttnameid.h"
#include "engines/ags/lib/freetype-2.1.3/psnames.h"

#include "engines/ags/lib/freetype-2.1.3/modules/cff/cffdrivr.h"
#include "engines/ags/lib/freetype-2.1.3/modules/cff/cffgload.h"
#include "engines/ags/lib/freetype-2.1.3/modules/cff/cffload.h"

#include "engines/ags/lib/freetype-2.1.3/modules/cff/cfferrs.h"

#undef FT_COMPONENT
#define FT_COMPONENT trace_cffdriver

namespace AGS3 {
namespace FreeType213 {


/**** FACES ****/

#undef PAIR_TAG
#define PAIR_TAG(left, right) (((FT_ULong)left << 16) | (FT_ULong)right)

static FT_Error Get_Kerning(TT_Face face, FT_UInt left_glyph, FT_UInt right_glyph, FT_Vector *kerning) {
	TT_Kern0_Pair pair;

	if (!face)
		return FT_Err_Invalid_Face_Handle;

	kerning->x = 0;
	kerning->y = 0;

	if (face->kern_pairs) {
		/* there are some kerning pairs in this font file! */
		FT_ULong search_tag = PAIR_TAG(left_glyph, right_glyph);
		FT_Long left, right;

		left = 0;
		right = face->num_kern_pairs - 1;

		while (left <= right) {
			FT_Long middle = left + ((right - left) >> 1);
			FT_ULong cur_pair;

			pair = face->kern_pairs + middle;
			cur_pair = PAIR_TAG(pair->left, pair->right);

			if (cur_pair == search_tag)
				goto Found;

			if (cur_pair < search_tag)
				left = middle + 1;
			else
				right = middle - 1;
		}
	}

Exit:
	return FT_Err_Ok;

Found:
	kerning->x = pair->value;
	goto Exit;
}

#undef PAIR_TAG


static FT_Error Load_Glyph(CFF_GlyphSlot slot, CFF_Size size, FT_UInt glyph_index, FT_Int32 load_flags) {
	FT_Error error;

	if (!slot)
		return FT_Err_Invalid_Slot_Handle;

	/* check whether we want a scaled outline or bitmap */
	if (!size)
		load_flags |= FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING;

	if (load_flags & FT_LOAD_NO_SCALE)
		size = NULL;

	/* reset the size object if necessary */
	if (size) {
		/* these two object must have the same parent */
		if (size->face != slot->root.face)
			return FT_Err_Invalid_Face_Handle;
	}

	/* now load the glyph outline if necessary */
	error = cff_slot_load(slot, size, glyph_index, load_flags);

	/* force drop-out mode to 2 - irrelevant now */
	/* slot->outline.dropout_mode = 2; */

	return error;
}


/**** CHARACTER MAPPINGS ****/

static FT_Error cff_get_glyph_name(CFF_Face face, FT_UInt glyph_index, FT_Pointer buffer, FT_UInt buffer_max) {
	CFF_Font font =  (CFF_Font)face->extra.data;
	FT_Memory        memory = FT_FACE_MEMORY(face);
	FT_String        *gname;
	FT_UShort        sid;
	PSNames_Service  psnames;
	FT_Error         error;

	const void *psnames_tmp = FT_Get_Module_Interface(face->root.driver->root.library, "psnames");
	psnames = const_cast<PSNames_Service>(static_cast<const PSNames_Interface *>(psnames_tmp));

	if (!psnames) {
		FT_ERROR(("cff_get_glyph_name:"));
		FT_ERROR((" cannot open CFF & CEF fonts\n"));
		FT_ERROR(("                   "));
		FT_ERROR((" without the `PSNames' module\n"));
		error = FT_Err_Unknown_File_Format;
		goto Exit;
	}

	/* first, locate the sid in the charset table */
	sid = font->charset.sids[glyph_index];

	/* now, lookup the name itself */
	gname = cff_index_get_sid_string(&font->string_index, sid, psnames);

	if (buffer_max > 0) {
		FT_UInt len = (FT_UInt)ft_strlen(gname);

		if (len >= buffer_max)
			len = buffer_max - 1;

		FT_MEM_COPY(buffer, gname, len);
		((FT_Byte *)buffer)[len] = 0;
	}

	FT_FREE(gname);
	error = FT_Err_Ok;

Exit:
	return error;
}

static FT_UInt cff_get_name_index(CFF_Face face, FT_String *glyph_name) {
	CFF_Font 		 cff;
	CFF_Charset      charset;
	PSNames_Service  psnames;
	FT_Memory        memory = FT_FACE_MEMORY(face);
	FT_String        *name;
	FT_UShort        sid;
	FT_UInt          i;
	FT_Int           result;

	cff = (CFF_FontRec *)face->extra.data;
	charset = &cff->charset;

	const void *psnames_tmp = FT_Get_Module_Interface(face->root.driver->root.library, "psnames");
	psnames = const_cast<PSNames_Service>(static_cast<const PSNames_Interface *>(psnames_tmp));

	for (i = 0; i < cff->num_glyphs; i++) {
		sid = charset->sids[i];

		if (sid > 390)
			name = cff_index_get_name(&cff->string_index, sid - 391);
		else
			name = const_cast<FT_String *>(psnames->adobe_std_strings(sid));

		result = ft_strcmp(glyph_name, name);

		if (sid > 390)
			FT_FREE(name);

		if (!result)
			return i;
	}

	return 0;
}


/**** DRIVER INTERFACE ****/

static FT_Module_Interface cff_get_interface(CFF_Driver driver, const char *module_interface) {
	FT_Module sfnt;

#ifndef FT_CONFIG_OPTION_NO_GLYPH_NAMES

	if (ft_strcmp((const char *)module_interface, "glyph_name") == 0)
		return (FT_Module_Interface)cff_get_glyph_name;

	if (ft_strcmp((const char *)module_interface, "name_index") == 0)
		return (FT_Module_Interface)cff_get_name_index;

#endif

	/* we simply pass our request to the `sfnt' module */
	sfnt = FT_Get_Module(driver->root.root.library, "sfnt");

	return sfnt ? sfnt->clazz->get_interface(sfnt, module_interface) : 0;
}


FT_CALLBACK_TABLE_DEF
const FT_Driver_ClassRec cff_driver_class = {
	/* begin with the FT_Module_Class fields */
	{
		ft_module_font_driver       |
		ft_module_driver_scalable   |
		ft_module_driver_has_hinter,

		sizeof(CFF_DriverRec),
		"cff",
		0x10000L,
		0x20000L,

		0,   /* module-specific interface */

		(FT_Module_Constructor) cff_driver_init,
		(FT_Module_Destructor)  cff_driver_done,
		(FT_Module_Requester)   cff_get_interface,
	},

	/* now the specific driver fields */
	sizeof(TT_FaceRec),
	sizeof(FT_SizeRec),
	sizeof(CFF_GlyphSlotRec),

	(FT_Face_InitFunc) cff_face_init,
	(FT_Face_DoneFunc) cff_face_done,
	(FT_Size_InitFunc) cff_size_init,
	(FT_Size_DoneFunc) cff_size_done,
	(FT_Slot_InitFunc) cff_slot_init,
	(FT_Slot_DoneFunc) cff_slot_done,

	(FT_Size_ResetPointsFunc) cff_points_size_reset,
	(FT_Size_ResetPixelsFunc) cff_pixels_size_reset,

	(FT_Slot_LoadFunc) Load_Glyph,

	(FT_Face_GetKerningFunc)  Get_Kerning,
	(FT_Face_AttachFunc)      0,
	(FT_Face_GetAdvancesFunc) 0,
};


} // End of namespace FreeType213
} // End of namespace AGS3
