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
/*  t1gload.c                                                              */
/*    Type 1 Glyph Loader (body).                                          */
/*                                                                         */
/***************************************************************************/


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/modules/type1/t1gload.h"
#include "engines/ags/lib/freetype-2.1.3/ftdebug.h"
#include "engines/ags/lib/freetype-2.1.3/ftstream.h"
#include "engines/ags/lib/freetype-2.1.3/ftoutln.h"
#include "engines/ags/lib/freetype-2.1.3/psaux.h"

#include "engines/ags/lib/freetype-2.1.3/modules/type1/t1errors.h"

#undef FT_COMPONENT
#define FT_COMPONENT trace_t1gload

namespace AGS3 {
namespace FreeType213 {


/**** COMPUTE THE MAXIMUM ADVANCE WIDTH ****/

FT_LOCAL_DEF(FT_Error)
T1_Parse_Glyph_And_Get_Char_String(T1_Decoder decoder, FT_UInt glyph_index, FT_Data *char_string) {
	T1_Face  face  = (T1_Face)decoder->builder.face;
	T1_Font  type1 = &face->type1;
	FT_Error error = 0;

	decoder->font_matrix = type1->font_matrix;
	decoder->font_offset = type1->font_offset;

#ifdef FT_CONFIG_OPTION_INCREMENTAL

	/* For incremental fonts get the character data using the */
	/* callback function.                                     */
	if (face->root.internal->incremental_interface)
		error = face->root.internal->incremental_interface->funcs->get_glyph_data(face->root.internal->incremental_interface->object,
																				  glyph_index, char_string);
	else

#endif

	/* For ordinary fonts get the character data stored in the face record. */
	{
		char_string->pointer = type1->charstrings[glyph_index];
		char_string->length = type1->charstrings_len[glyph_index];
	}

	if (!error)
		error = decoder->funcs.parse_charstrings(decoder, const_cast<FT_Byte *>(char_string->pointer), char_string->length);

#ifdef FT_CONFIG_OPTION_INCREMENTAL

	/* Incremental fonts can optionally override the metrics. */
	if (!error && face->root.internal->incremental_interface && face->root.internal->incremental_interface->funcs->get_glyph_metrics) {
		FT_Bool found = FALSE;
		FT_Incremental_MetricsRec metrics;

		error = face->root.internal->incremental_interface->funcs->get_glyph_metrics(face->root.internal->incremental_interface->object,
																					 glyph_index, FALSE, &metrics, &found);
		if (found) {
			decoder->builder.left_bearing.x = metrics.bearing_x;
			decoder->builder.left_bearing.y = metrics.bearing_y;
			decoder->builder.advance.x = metrics.advance;
			decoder->builder.advance.y = 0;
		}
	}

#endif

	return error;
}


FT_CALLBACK_DEF(FT_Error)
T1_Parse_Glyph(T1_Decoder decoder, FT_UInt glyph_index) {
	FT_Data  glyph_data;
	FT_Error error = T1_Parse_Glyph_And_Get_Char_String(decoder, glyph_index, &glyph_data);

#ifdef FT_CONFIG_OPTION_INCREMENTAL
	if (!error) {
		T1_Face face = (T1_Face)decoder->builder.face;

		if (face->root.internal->incremental_interface)
			face->root.internal->incremental_interface->funcs->free_glyph_data(face->root.internal->incremental_interface->object, &glyph_data);
	}
#endif

	return error;
}


FT_LOCAL_DEF(FT_Error)
T1_Compute_Max_Advance(T1_Face face, FT_Int *max_advance) {
	FT_Error 	   error;
	T1_DecoderRec  decoder;
	FT_Int 		   glyph_index;
	T1_Font 	   type1 = &face->type1;
	PSAux_Service  psaux = const_cast<PSAux_Service>(reinterpret_cast<const PSAux_ServiceRec_ *>(face->psaux));

	*max_advance = 0;

	/* initialize load decoder */
	error = psaux->t1_decoder_funcs->init(&decoder, (FT_Face)face, 0, 0, (FT_Byte **)type1->glyph_names,
										  face->blend, 0, FT_RENDER_MODE_NORMAL, T1_Parse_Glyph);
	if (error)
		return error;

	decoder.builder.metrics_only = 1;
	decoder.builder.load_points = 0;

	decoder.num_subrs = type1->num_subrs;
	decoder.subrs = type1->subrs;
	decoder.subrs_len = type1->subrs_len;

	*max_advance = 0;

	/* for each glyph, parse the glyph charstring and extract */
	/* the advance width                                      */
	for (glyph_index = 0; glyph_index < type1->num_glyphs; glyph_index++) {
		/* now get load the unscaled outline */
		error = T1_Parse_Glyph(&decoder, glyph_index);
		if (glyph_index == 0 || decoder.builder.advance.x > *max_advance)
			*max_advance = decoder.builder.advance.x;

		/* ignore the error if one occured - skip to next glyph */
	}

	return FT_Err_Ok;
}


/**** UNHINTED GLYPH LOADER ****/

FT_LOCAL_DEF(FT_Error)
T1_Load_Glyph(T1_GlyphSlot glyph, T1_Size size, FT_UInt glyph_index, FT_Int32 load_flags) {
	FT_Error 	   error;
	T1_DecoderRec  decoder;
	T1_Face 	   face = (T1_Face)glyph->root.face;
	FT_Bool 	   hinting;
	T1_Font 	   type1 = &face->type1;

	PSAux_Service psaux = const_cast<PSAux_Service>(reinterpret_cast<const PSAux_ServiceRec_ *>(face->psaux));
	const T1_Decoder_Funcs decoder_funcs = psaux->t1_decoder_funcs;

	FT_Matrix  font_matrix;
	FT_Vector  font_offset;
	FT_Data    glyph_data;
	FT_Bool    glyph_data_loaded = 0;

	if (load_flags & FT_LOAD_NO_RECURSE)
		load_flags |= FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING;

	glyph->x_scale = size->root.metrics.x_scale;
	glyph->y_scale = size->root.metrics.y_scale;

	glyph->root.outline.n_points = 0;
	glyph->root.outline.n_contours = 0;

	hinting = FT_BOOL((load_flags & FT_LOAD_NO_SCALE) == 0 && (load_flags & FT_LOAD_NO_HINTING) == 0);

	glyph->root.format = FT_GLYPH_FORMAT_OUTLINE;

	error = decoder_funcs->init(&decoder, (FT_Face)face, (FT_Size)size, (FT_GlyphSlot)glyph, (FT_Byte **)type1->glyph_names,
								face->blend, FT_BOOL(hinting), FT_LOAD_TARGET_MODE(load_flags), T1_Parse_Glyph);
	if (error)
		goto Exit;

	decoder.builder.no_recurse = FT_BOOL((load_flags & FT_LOAD_NO_RECURSE) != 0);

	decoder.num_subrs = type1->num_subrs;
	decoder.subrs 	  = type1->subrs;
	decoder.subrs_len = type1->subrs_len;

	/* now load the unscaled outline */
	error = T1_Parse_Glyph_And_Get_Char_String(&decoder, glyph_index, &glyph_data);
	if (error)
		goto Exit;
	glyph_data_loaded = 1;

	font_matrix = decoder.font_matrix;
	font_offset = decoder.font_offset;

	/* save new glyph tables */
	decoder_funcs->done(&decoder);

	/* now, set the metrics -- this is rather simple, as   */
	/* the left side bearing is the xMin, and the top side */
	/* bearing the yMax                                    */
	if (!error) {
		glyph->root.outline.flags &= FT_OUTLINE_OWNER;
		glyph->root.outline.flags |= FT_OUTLINE_REVERSE_FILL;

		/* for composite glyphs, return only left side bearing and */
		/* advance width                                           */
		if (load_flags & FT_LOAD_NO_RECURSE) {
			FT_Slot_Internal internal = glyph->root.internal;

			glyph->root.metrics.horiBearingX = decoder.builder.left_bearing.x;
			glyph->root.metrics.horiAdvance  = decoder.builder.advance.x;
			internal->glyph_matrix = font_matrix;
			internal->glyph_delta  = font_offset;
			internal->glyph_transformed = 1;
		} else {
			FT_BBox cbox;
			FT_Glyph_Metrics *metrics = &glyph->root.metrics;

			/* copy the _unscaled_ advance width */
			metrics->horiAdvance = decoder.builder.advance.x;
			glyph->root.linearHoriAdvance = decoder.builder.advance.x;
			glyph->root.internal->glyph_transformed = 0;

			/* make up vertical metrics */
			metrics->vertBearingX = 0;
			metrics->vertBearingY = 0;
			metrics->vertAdvance  = 0;

			glyph->root.linearVertAdvance = 0;

			glyph->root.format = FT_GLYPH_FORMAT_OUTLINE;

			if (size && size->root.metrics.y_ppem < 24)
				glyph->root.outline.flags |= FT_OUTLINE_HIGH_PRECISION;

#if 1
			/* apply the font matrix, if any */
			FT_Outline_Transform(&glyph->root.outline, &font_matrix);

			FT_Outline_Translate(&glyph->root.outline, font_offset.x, font_offset.y);
#endif

			if ((load_flags & FT_LOAD_NO_SCALE) == 0) {
				/* scale the outline and the metrics */
				FT_Int 	   n;
				FT_Outline *cur = decoder.builder.base;
				FT_Vector  *vec = cur->points;
				FT_Fixed   x_scale = glyph->x_scale;
				FT_Fixed   y_scale = glyph->y_scale;

				/* First of all, scale the points, if we are not hinting */
				if (!hinting)
					for (n = cur->n_points; n > 0; n--, vec++) {
						vec->x = FT2_1_3_MulFix(vec->x, x_scale);
						vec->y = FT2_1_3_MulFix(vec->y, y_scale);
					}

				FT_Outline_Get_CBox(&glyph->root.outline, &cbox);

				/* Then scale the metrics */
				metrics->horiAdvance = FT2_1_3_MulFix(metrics->horiAdvance, x_scale);
				metrics->vertAdvance = FT2_1_3_MulFix(metrics->vertAdvance, y_scale);

				metrics->vertBearingX = FT2_1_3_MulFix(metrics->vertBearingX, x_scale);
				metrics->vertBearingY = FT2_1_3_MulFix(metrics->vertBearingY, y_scale);

				if (hinting) {
					metrics->horiAdvance = (metrics->horiAdvance + 32) & -64;
					metrics->vertAdvance = (metrics->vertAdvance + 32) & -64;

					metrics->vertBearingX = (metrics->vertBearingX + 32) & -64;
					metrics->vertBearingY = (metrics->vertBearingY + 32) & -64;
				}
			}

			/* compute the other metrics */
			FT_Outline_Get_CBox(&glyph->root.outline, &cbox);

			/* grid fit the bounding box if necessary */
			if (hinting) {
				cbox.xMin &= -64;
				cbox.yMin &= -64;
				cbox.xMax = (cbox.xMax + 63) & -64;
				cbox.yMax = (cbox.yMax + 63) & -64;
			}

			metrics->width  = cbox.xMax - cbox.xMin;
			metrics->height = cbox.yMax - cbox.yMin;

			metrics->horiBearingX = cbox.xMin;
			metrics->horiBearingY = cbox.yMax;
		}

		/* Set control data to the glyph charstrings.  Note that this is */
		/* _not_ zero-terminated.                                        */
		glyph->root.control_data = const_cast<FT_Byte *>(glyph_data.pointer);
		glyph->root.control_len  = glyph_data.length;
	}

Exit:

#ifdef FT_CONFIG_OPTION_INCREMENTAL
	if (glyph_data_loaded && face->root.internal->incremental_interface) {
		face->root.internal->incremental_interface->funcs->free_glyph_data(face->root.internal->incremental_interface->object, &glyph_data);

		/* Set the control data to null - it is no longer available if   */
		/* loaded incrementally.                                         */
		glyph->root.control_data = 0;
		glyph->root.control_len  = 0;
	}
#endif
	(void)glyph_data_loaded;
	return error;
}

} // End of namespace FreeType213
} // End of namespace AGS3
