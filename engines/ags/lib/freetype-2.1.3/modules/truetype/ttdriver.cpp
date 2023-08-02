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
/*  ttdriver.c                                                             */
/*                                                                         */
/*    TrueType font driver implementation (body).                          */
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
#include "engines/ags/lib/freetype-2.1.3/ftdebug.h"
#include "engines/ags/lib/freetype-2.1.3/ftstream.h"
#include "engines/ags/lib/freetype-2.1.3/sfnt.h"
#include "engines/ags/lib/freetype-2.1.3/ttnameid.h"

#include "engines/ags/lib/freetype-2.1.3/modules/truetype/ttdriver.h"
#include "engines/ags/lib/freetype-2.1.3/modules/truetype/ttgload.h"

#include "engines/ags/lib/freetype-2.1.3/modules/truetype/tterrors.h"


#undef  FT_COMPONENT
#define FT_COMPONENT  trace_ttdriver

namespace AGS3 {
namespace FreeType213 {


/**** FACES ****/

#undef  PAIR_TAG
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
			FT_Int middle = left + ((right - left) >> 1);
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


/**** SIZES ****/

static FT_Error Set_Char_Sizes(TT_Size size, FT_F26Dot6 char_width, FT_F26Dot6 char_height, FT_UInt horz_resolution, FT_UInt vert_resolution) {
	FT_Size_Metrics *metrics = &size->root.metrics;
	TT_Face 		face = (TT_Face)size->root.face;
	FT_Long dim_x, dim_y;

	/* This bit flag, when set, indicates that the pixel size must be */
	/* truncated to an integer.  Nearly all TrueType fonts have this  */
	/* bit set, as hinting won't work really well otherwise.          */
	/*                                                                */
	/* However, for those rare fonts who do not set it, we override   */
	/* the default computations performed by the base layer.  I       */
	/* really don't know whether this is useful, but hey, that's the  */
	/* spec :-)                                                       */
	/*                                                                */
	if ((face->header.Flags & 8) == 0) {
		/* Compute pixel sizes in 26.6 units */
		dim_x = (char_width * horz_resolution + 36) / 72;
		dim_y = (char_height * vert_resolution + 36) / 72;

		metrics->x_scale = FT_DivFix(dim_x, face->root.units_per_EM);
		metrics->y_scale = FT_DivFix(dim_y, face->root.units_per_EM);

		metrics->x_ppem = (FT_UShort)(dim_x >> 6);
		metrics->y_ppem = (FT_UShort)(dim_y >> 6);
	}

	size->ttmetrics.valid = FALSE;
#ifdef TT_CONFIG_OPTION_EMBEDDED_BITMAPS
	size->strike_index = 0xFFFF;
#endif

	return tt_size_reset(size);
}


static FT_Error Set_Pixel_Sizes(TT_Size size, FT_UInt pixel_width, FT_UInt pixel_height) {
	FT_UNUSED(pixel_width);
	FT_UNUSED(pixel_height);

	/* many things have been pre-computed by the base layer */

	size->ttmetrics.valid = FALSE;
#ifdef TT_CONFIG_OPTION_EMBEDDED_BITMAPS
	size->strike_index = 0xFFFF;
#endif

	return tt_size_reset(size);
}


static FT_Error Load_Glyph(TT_GlyphSlot slot, TT_Size size, FT_UInt glyph_index, FT_Int32 load_flags) {
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
		if (size->root.face != slot->face)
			return FT_Err_Invalid_Face_Handle;

		if (!size->ttmetrics.valid) {
			if (FT_SET_ERROR(tt_size_reset(size)))
				return error;
		}
	}

	/* now load the glyph outline if necessary */
	error = TT_Load_Glyph(size, slot, glyph_index, load_flags);

	/* force drop-out mode to 2 - irrelevant now */
	/* slot->outline.dropout_mode = 2; */

	return error;
}


/**** DRIVER INTERFACE ****/

static FT_Module_Interface tt_get_interface(TT_Driver driver, const char *tt_interface) {
	FT_Module sfntd = FT_Get_Module(driver->root.root.library, "sfnt");
	SFNT_Service sfnt;

	/* only return the default interface from the SFNT module */
	if (sfntd) {
		sfnt = const_cast<SFNT_Service>(static_cast<const SFNT_Interface *>(sfntd->clazz->module_interface));
		if (sfnt)
			return sfnt->get_interface(FT_MODULE(driver), tt_interface);
	}

	return 0;
}


FT_CALLBACK_TABLE_DEF
const FT_Driver_ClassRec tt_driver_class = {
	{
		ft_module_font_driver |
		ft_module_driver_scalable |
#ifdef TT_CONFIG_OPTION_BYTECODE_INTERPRETER
		ft_module_driver_has_hinter,
#else
		0,
#endif

		sizeof(TT_DriverRec),

		"truetype", /* driver name                           */
		0x10000L,   /* driver version == 1.0                 */
		0x20000L,   /* driver requires FreeType 2.0 or above */

		(void *)0, /* driver specific interface */

		(FT_Module_Constructor)	tt_driver_init,
		(FT_Module_Destructor)	tt_driver_done,
		(FT_Module_Requester)	tt_get_interface,
	},

	sizeof(TT_FaceRec),
	sizeof(TT_SizeRec),
	sizeof(FT_GlyphSlotRec),

	(FT_Face_InitFunc) tt_face_init,
	(FT_Face_DoneFunc) tt_face_done,
	(FT_Size_InitFunc) tt_size_init,
	(FT_Size_DoneFunc) tt_size_done,
	(FT_Slot_InitFunc) 0,
	(FT_Slot_DoneFunc) 0,

	(FT_Size_ResetPointsFunc) Set_Char_Sizes,
	(FT_Size_ResetPixelsFunc) Set_Pixel_Sizes,
	(FT_Slot_LoadFunc)		  Load_Glyph,

	(FT_Face_GetKerningFunc)  Get_Kerning,
	(FT_Face_AttachFunc)	  0,
	(FT_Face_GetAdvancesFunc) 0
};


} // End of namespace FreeType213
} // End of namespace AGS3
