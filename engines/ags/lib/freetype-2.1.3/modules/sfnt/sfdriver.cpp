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
/*  sfdriver.c                                                             */
/*    High-level SFNT driver interface (body).                             */
/*                                                                         */
/***************************************************************************/


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/sfnt.h"
#include "engines/ags/lib/freetype-2.1.3/ftobjs.h"

#include "engines/ags/lib/freetype-2.1.3/modules/sfnt/sfdriver.h"
#include "engines/ags/lib/freetype-2.1.3/modules/sfnt/ttload.h"
#include "engines/ags/lib/freetype-2.1.3/modules/sfnt/ttcmap.h"
#include "engines/ags/lib/freetype-2.1.3/modules/sfnt/sfobjs.h"

#ifdef TT_CONFIG_OPTION_EMBEDDED_BITMAPS
#include "engines/ags/lib/freetype-2.1.3/modules/sfnt/ttsbit.h"
#endif

#ifdef TT_CONFIG_OPTION_POSTSCRIPT_NAMES
#include "engines/ags/lib/freetype-2.1.3/modules/sfnt/ttpost.h"
#endif

namespace AGS3 {
namespace FreeType213 {


static void *get_sfnt_table(TT_Face face, FT_Sfnt_Tag tag) {
	void *table;

	switch (tag) {
	case ft_sfnt_head:
		table = &face->header;
		break;

	case ft_sfnt_hhea:
		table = &face->horizontal;
		break;

	case ft_sfnt_vhea:
		table = face->vertical_info ? &face->vertical : 0;
		break;

	case ft_sfnt_os2:
		table = face->os2.version == 0xFFFFU ? 0 : &face->os2;
		break;

	case ft_sfnt_post:
		table = &face->postscript;
		break;

	case ft_sfnt_maxp:
		table = &face->max_profile;
		break;

	case ft_sfnt_pclt:
		table = face->pclt.Version ? &face->pclt : 0;
		break;

	default:
		table = 0;
	}

	return table;
}

#ifdef TT_CONFIG_OPTION_POSTSCRIPT_NAMES

static FT_Error get_sfnt_glyph_name(TT_Face face, FT_UInt glyph_index, FT_Pointer buffer, FT_UInt buffer_max) {
	FT_String *gname;
	FT_Error error;

	error = tt_face_get_ps_name(face, glyph_index, &gname);
	if (!error && buffer_max > 0) {
		FT_UInt len = (FT_UInt)(ft_strlen(gname));

		if (len >= buffer_max)
			len = buffer_max - 1;

		FT_MEM_COPY(buffer, gname, len);
		((FT_Byte *)buffer)[len] = 0;
	}

	return error;
}

static const char *get_sfnt_postscript_name(TT_Face face) {
	FT_Int n, found_win, found_apple;
	const char *result = NULL;

	/* shouldn't happen, but just in case to avoid memory leaks */
	if (face->root.internal->postscript_name)
		return face->root.internal->postscript_name;

	/* scan the name table to see whether we have a Postscript name here, */
	/* either in Macintosh or Windows platform encodings                  */
	found_win = -1;
	found_apple = -1;

	for (n = 0; n < face->num_names; n++) {
		TT_NameEntryRec *name = face->name_table.names + n;

		if (name->nameID == 6 && name->stringLength > 0) {
			if (name->platformID == 3 && name->encodingID == 1 && name->languageID == 0x409)
				found_win = n;

			if (name->platformID == 1 && name->encodingID == 0 && name->languageID == 0)
				found_apple = n;
		}
	}

	if (found_win != -1) {
		FT_Memory memory = face->root.memory;
		TT_NameEntryRec *name = face->name_table.names + found_win;
		FT_UInt len = name->stringLength / 2;
		FT_Error error;

		if (!FT_ALLOC(result, name->stringLength + 1)) {
			FT_Stream stream = face->name_table.stream;
			FT_String *r = const_cast<FT_String *>(result);
			FT_Byte *p = (FT_Byte *)name->string;

			if (FT_STREAM_SEEK(name->stringOffset) || FT_FRAME_ENTER(name->stringLength)) {
				FT_FREE(result);
				name->stringLength = 0;
				name->stringOffset = 0;
				FT_FREE(name->string);

				goto Exit;
			}

			p = (FT_Byte *)stream->cursor;

			for (; len > 0; len--, p += 2) {
				if (p[0] == 0 && p[1] >= 32 && p[1] < 128)
					*r++ = p[1];
			}
			*r = '\0';

			FT_FRAME_EXIT();
		}
		goto Exit;
	}

	if (found_apple != -1) {
		FT_Memory memory = face->root.memory;
		TT_NameEntryRec *name = face->name_table.names + found_apple;
		FT_UInt len = name->stringLength;
		FT_Error error;

		if (!FT_ALLOC(result, len + 1)) {
			FT_Stream stream = face->name_table.stream;

			if (FT_STREAM_SEEK(name->stringOffset) ||
				((error = (FT_Stream_Read(stream, (FT_Byte *)const_cast<char *>(result), len))) != 0)) {
				name->stringOffset = 0;
				name->stringLength = 0;
				FT_FREE(name->string);
				FT_FREE(result);
				goto Exit;
			}
			const_cast<char *>(result)[len] = '\0';
		}
	}

Exit:
	face->root.internal->postscript_name = result;
	return result;
}

#endif /* TT_CONFIG_OPTION_POSTSCRIPT_NAMES */


FT_CALLBACK_DEF(FT_Module_Interface)
sfnt_get_interface(FT_Module module, const char *module_interface) {
	FT_UNUSED(module);

	if (ft_strcmp(module_interface, "get_sfnt") == 0)
		return (FT_Module_Interface)get_sfnt_table;

#ifdef TT_CONFIG_OPTION_POSTSCRIPT_NAMES
	if (ft_strcmp(module_interface, "glyph_name") == 0)
		return (FT_Module_Interface)get_sfnt_glyph_name;
#endif

	if (ft_strcmp(module_interface, "postscript_name") == 0)
		return (FT_Module_Interface)get_sfnt_postscript_name;

	return 0;
}


static const SFNT_Interface sfnt_interface = {
	tt_face_goto_table,

	sfnt_init_face,
	sfnt_load_face,
	sfnt_done_face,
	sfnt_get_interface,

	tt_face_load_any,
	tt_face_load_sfnt_header,
	tt_face_load_directory,

	tt_face_load_header,
	tt_face_load_metrics_header,
	tt_face_load_cmap,
	tt_face_load_max_profile,
	tt_face_load_os2,
	tt_face_load_postscript,

	tt_face_load_names,
	tt_face_free_names,

	tt_face_load_hdmx,
	tt_face_free_hdmx,

	tt_face_load_kern,
	tt_face_load_gasp,
	tt_face_load_pclt,

#ifdef TT_CONFIG_OPTION_EMBEDDED_BITMAPS

	/* see `ttload.h' */
	tt_face_load_bitmap_header,

	/* see `ttsbit.h' */
	tt_face_set_sbit_strike,
	tt_face_load_sbit_strikes,
	tt_face_load_sbit_image,
	tt_face_free_sbit_strikes,

#else /* TT_CONFIG_OPTION_EMBEDDED_BITMAPS */

	0,
	0,
	0,
	0,
	0,

#endif /* TT_CONFIG_OPTION_EMBEDDED_BITMAPS */

#ifdef TT_CONFIG_OPTION_POSTSCRIPT_NAMES

	/* see `ttpost.h' */
	tt_face_get_ps_name,
	tt_face_free_ps_names,

#else /* TT_CONFIG_OPTION_POSTSCRIPT_NAMES */

	0,
	0,

#endif /* TT_CONFIG_OPTION_POSTSCRIPT_NAMES */

	/* see `ttcmap.h' */
	tt_face_load_charmap,
	tt_face_free_charmap,
};


FT_CALLBACK_TABLE_DEF
const FT_Module_Class sfnt_module_class = {
	0, /* not a font driver or renderer */
	sizeof(FT_ModuleRec),

	"sfnt",   /* driver name                            */
	0x10000L, /* driver version 1.0                     */
	0x20000L, /* driver requires FreeType 2.0 or higher */

	(const void *) &sfnt_interface, /* module specific interface */

	(FT_Module_Constructor) 0,
	(FT_Module_Destructor)  0,
	(FT_Module_Requester)   sfnt_get_interface
};


} // End of namespace FreeType213
} // End of namespace AGS3
