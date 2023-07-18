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
/*  ttpload.c                                                              */
/*    TrueType glyph data/program tables loader (body).                    */
/*                                                                         */
/***************************************************************************/


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/ftdebug.h"
#include "engines/ags/lib/freetype-2.1.3/ftobjs.h"
#include "engines/ags/lib/freetype-2.1.3/ftstream.h"
#include "engines/ags/lib/freetype-2.1.3/tttags.h"

#include "engines/ags/lib/freetype-2.1.3/modules/truetype/ttpload.h"

#include "engines/ags/lib/freetype-2.1.3/modules/truetype/tterrors.h"

#undef  FT2_1_3_COMPONENT
#define FT2_1_3_COMPONENT  trace_ttpload

namespace AGS3 {
namespace FreeType213 {


FT2_1_3_LOCAL_DEF(FT_Error)
tt_face_load_loca(TT_Face face, FT_Stream stream) {
	FT_Error error;
	FT_Memory memory = stream->memory;
	FT_Short LongOffsets;
	FT_ULong table_len;

	FT2_1_3_TRACE2(("Locations "));
	LongOffsets = face->header.Index_To_Loc_Format;

	error = face->goto_table(face, TTAG_loca, stream, &table_len);
	if (error) {
		error = FT2_1_3_Err_Locations_Missing;
		goto Exit;
	}

	if (LongOffsets != 0) {
		face->num_locations = (FT_UShort)(table_len >> 2);

		FT2_1_3_TRACE2(("(32bit offsets): %12d ", face->num_locations));

		if (FT2_1_3_NEW_ARRAY(face->glyph_locations, face->num_locations))
			goto Exit;

		if (FT2_1_3_FRAME_ENTER(face->num_locations * 4L))
			goto Exit;

		{
			FT_Long *loc = face->glyph_locations;
			FT_Long *limit = loc + face->num_locations;

			for (; loc < limit; loc++)
				*loc = FT2_1_3_GET_LONG();
		}

		FT2_1_3_FRAME_EXIT();
	} else {
		face->num_locations = (FT_UShort)(table_len >> 1);

		FT2_1_3_TRACE2(("(16bit offsets): %12d ", face->num_locations));

		if (FT2_1_3_NEW_ARRAY(face->glyph_locations, face->num_locations))
			goto Exit;

		if (FT2_1_3_FRAME_ENTER(face->num_locations * 2L))
			goto Exit;
		{
			FT_Long *loc = face->glyph_locations;
			FT_Long *limit = loc + face->num_locations;

			for (; loc < limit; loc++)
				*loc = (FT_Long)((FT_ULong)FT2_1_3_GET_USHORT() * 2);
		}
		FT2_1_3_FRAME_EXIT();
	}

	FT2_1_3_TRACE2(("loaded\n"));

Exit:
	return error;
}


FT2_1_3_LOCAL_DEF(FT_Error)
tt_face_load_cvt(TT_Face face, FT_Stream stream) {
	FT_Error error;
	FT_Memory memory = stream->memory;
	FT_ULong table_len;

	FT2_1_3_TRACE2(("CVT "));

	error = face->goto_table(face, TTAG_cvt, stream, &table_len);
	if (error) {
		FT2_1_3_TRACE2(("is missing!\n"));

		face->cvt_size = 0;
		face->cvt = NULL;
		error = FT2_1_3_Err_Ok;

		goto Exit;
	}

	face->cvt_size = table_len / 2;

	if (FT2_1_3_NEW_ARRAY(face->cvt, face->cvt_size))
		goto Exit;

	if (FT2_1_3_FRAME_ENTER(face->cvt_size * 2L))
		goto Exit;

	{
		FT_Short *cur = face->cvt;
		FT_Short *limit = cur + face->cvt_size;

		for (; cur < limit; cur++)
			*cur = FT2_1_3_GET_SHORT();
	}

	FT2_1_3_FRAME_EXIT();
	FT2_1_3_TRACE2(("loaded\n"));

Exit:
	return error;
}


FT2_1_3_LOCAL_DEF(FT_Error)
tt_face_load_fpgm(TT_Face face, FT_Stream stream) {
	FT_Error error;
	FT_ULong table_len;

	FT2_1_3_TRACE2(("Font program "));

	/* The font program is optional */
	error = face->goto_table(face, TTAG_fpgm, stream, &table_len);
	if (error) {
		face->font_program = NULL;
		face->font_program_size = 0;

		FT2_1_3_TRACE2(("is missing!\n"));
	} else {
		face->font_program_size = table_len;
		if (FT2_1_3_FRAME_EXTRACT(table_len, face->font_program))
			goto Exit;

		FT2_1_3_TRACE2(("loaded, %12d bytes\n", face->font_program_size));
	}

	FT2_1_3_TRACE2(("Prep program "));

	error = face->goto_table(face, TTAG_prep, stream, &table_len);
	if (error) {
		face->cvt_program = NULL;
		face->cvt_program_size = 0;
		error = FT2_1_3_Err_Ok;

		FT2_1_3_TRACE2(("is missing!\n"));
	} else {
		face->cvt_program_size = table_len;
		if (FT2_1_3_FRAME_EXTRACT(table_len, face->cvt_program))
			goto Exit;

		FT2_1_3_TRACE2(("loaded, %12d bytes\n", face->cvt_program_size));
	}

Exit:
	return error;
}


} // End of namespace FreeType213
} // End of namespace AGS3
