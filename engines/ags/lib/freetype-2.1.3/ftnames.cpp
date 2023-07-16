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
/*  ftnames.c                                                              */
/*    Simple interface to access SFNT name tables (which are used          */
/*    to hold font names, copyright info, notices, etc.) (body).           */
/*                                                                         */
/***************************************************************************/

#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/ftsnames.h"
#include "engines/ags/lib/freetype-2.1.3/tttypes.h"
#include "engines/ags/lib/freetype-2.1.3/ftstream.h"

#ifdef TT_CONFIG_OPTION_SFNT_NAMES

namespace AGS3 {
namespace FreeType213 {


FT2_1_3_EXPORT_DEF(FT_UInt)
FT_Get_Sfnt_Name_Count(FT_Face face) {
	return (face && FT2_1_3_IS_SFNT(face)) ? ((TT_Face)face)->num_names : 0;
}

FT2_1_3_EXPORT_DEF(FT_Error)
FT_Get_Sfnt_Name(FT_Face face, FT_UInt idx, FT_SfntName *aname) {
	FT_Error error = FT2_1_3_Err_Invalid_Argument;

	if (aname && face && FT2_1_3_IS_SFNT(face)) {
		TT_Face ttface = (TT_Face)face;

		if (idx < (FT_UInt)ttface->num_names) {
			TT_NameEntryRec *entry = ttface->name_table.names + idx;

			/* load name on demand */
			if (entry->stringLength > 0 && entry->string == NULL) {
				FT_Memory memory = face->memory;
				FT_Stream stream = face->stream;

				if (FT2_1_3_NEW_ARRAY(entry->string, entry->stringLength) ||
					FT2_1_3_STREAM_SEEK(entry->stringOffset) ||
					FT2_1_3_STREAM_READ(entry->string, entry->stringLength)) {
					FT2_1_3_FREE(entry->string);
					entry->stringLength = 0;
				}
			}

			aname->platform_id = entry->platformID;
			aname->encoding_id = entry->encodingID;
			aname->language_id = entry->languageID;
			aname->name_id = entry->nameID;
			aname->string = (FT_Byte *)entry->string;
			aname->string_len = entry->stringLength;

			error = FT2_1_3_Err_Ok;
		}
	}

	return error;
}


} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* TT_CONFIG_OPTION_SFNT_NAMES */
