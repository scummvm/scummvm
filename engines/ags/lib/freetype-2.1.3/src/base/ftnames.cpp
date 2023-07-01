/***************************************************************************/
/*                                                                         */
/*  ftnames.c                                                              */
/*                                                                         */
/*    Simple interface to access SFNT name tables (which are used          */
/*    to hold font names, copyright info, notices, etc.) (body).           */
/*                                                                         */
/*    This is _not_ used to retrieve glyph names!                          */
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


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/ftsnames.h"
#include FT2_1_3_INTERNAL_TRUETYPE_TYPES_H
#include FT2_1_3_INTERNAL_STREAM_H


#ifdef TT_CONFIG_OPTION_SFNT_NAMES


/* documentation is in ftnames.h */

FT2_1_3_EXPORT_DEF( FT2_1_3_UInt )
FT2_1_3_Get_Sfnt_Name_Count( FT2_1_3_Face  face ) {
	return (face && FT2_1_3_IS_SFNT( face )) ? ((TT_Face)face)->num_names : 0;
}


/* documentation is in ftnames.h */

FT2_1_3_EXPORT_DEF( FT2_1_3_Error )
FT2_1_3_Get_Sfnt_Name( FT2_1_3_Face       face,
                  FT2_1_3_UInt       idx,
                  FT2_1_3_SfntName  *aname ) {
	FT2_1_3_Error  error = FT2_1_3_Err_Invalid_Argument;


	if ( aname && face && FT2_1_3_IS_SFNT( face ) ) {
		TT_Face  ttface = (TT_Face)face;


		if ( idx < (FT2_1_3_UInt)ttface->num_names ) {
			TT_NameEntryRec*  entry = ttface->name_table.names + idx;


			/* load name on demand */
			if ( entry->stringLength > 0 && entry->string == NULL ) {
				FT2_1_3_Memory  memory = face->memory;
				FT2_1_3_Stream  stream = face->stream;


				if ( FT2_1_3_NEW_ARRAY  ( entry->string, entry->stringLength ) ||
				        FT2_1_3_STREAM_SEEK( entry->stringOffset )                ||
				        FT2_1_3_STREAM_READ( entry->string, entry->stringLength ) ) {
					FT2_1_3_FREE( entry->string );
					entry->stringLength = 0;
				}
			}

			aname->platform_id = entry->platformID;
			aname->encoding_id = entry->encodingID;
			aname->language_id = entry->languageID;
			aname->name_id     = entry->nameID;
			aname->string      = (FT2_1_3_Byte*)entry->string;
			aname->string_len  = entry->stringLength;

			error = FT2_1_3_Err_Ok;
		}
	}

	return error;
}


#endif /* TT_CONFIG_OPTION_SFNT_NAMES */


/* END */
