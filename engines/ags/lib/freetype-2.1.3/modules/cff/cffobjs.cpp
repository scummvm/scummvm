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
/*  cffobjs.c                                                              */
/*    OpenType objects manager (body).                                     */
/*                                                                         */
/***************************************************************************/


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/ftdebug.h"
#include "engines/ags/lib/freetype-2.1.3/ftcalc.h"
#include "engines/ags/lib/freetype-2.1.3/ftstream.h"
#include "engines/ags/lib/freetype-2.1.3/fterrors.h"
#include "engines/ags/lib/freetype-2.1.3/ttnameid.h"
#include "engines/ags/lib/freetype-2.1.3/tttags.h"
#include "engines/ags/lib/freetype-2.1.3/sfnt.h"
#include "engines/ags/lib/freetype-2.1.3/psnames.h"
#include "engines/ags/lib/freetype-2.1.3/pshints.h"
#include "engines/ags/lib/freetype-2.1.3/modules/cff/cffobjs.h"
#include "engines/ags/lib/freetype-2.1.3/modules/cff/cffload.h"
#include "engines/ags/lib/freetype-2.1.3/modules/cff/cffcmap.h"
#include "engines/ags/lib/freetype-2.1.3/modules/cff/cfferrs.h"


#undef  FT_COMPONENT
#define FT_COMPONENT trace_cffobjs

namespace AGS3 {
namespace FreeType213 {


/**** SIZE FUNCTIONS ****/

static PSH_Globals_Funcs cff_size_get_globals_funcs(CFF_Size size) {
	CFF_Face face = (CFF_Face)size->face;
	CFF_Font font = (CFF_FontRec *)face->extra.data;
	PSHinter_Service pshinter = (PSHinter_Service)font->pshinter;
	FT_Module module;

	module = FT_Get_Module(size->face->driver->root.library, "pshinter");
	return (module && pshinter && pshinter->get_globals_funcs) ? pshinter->get_globals_funcs(module) : 0;
}

FT_LOCAL_DEF(void)
cff_size_done(CFF_Size size) {
	if (size->internal) {
		PSH_Globals_Funcs funcs;

		funcs = cff_size_get_globals_funcs(size);
		if (funcs)
			funcs->destroy((PSH_Globals)size->internal);

		size->internal = 0;
	}
}

FT_LOCAL_DEF(FT_Error)
cff_size_init(CFF_Size size) {
	FT_Error error = 0;
	PSH_Globals_Funcs funcs = cff_size_get_globals_funcs(size);

	if (funcs) {
		PSH_Globals globals;
		CFF_Face face = (CFF_Face)size->face;
		CFF_Font font = (CFF_FontRec *)face->extra.data;
		CFF_SubFont subfont = &font->top_font;

		CFF_Private cpriv = &subfont->private_dict;
		PS_PrivateRec priv;

		/* IMPORTANT: The CFF and Type1 private dictionaries have    */
		/*            slightly different structures; we need to      */
		/*            synthetize a type1 dictionary on the fly here. */

		{
			FT_UInt n, count;

			FT_MEM_ZERO(&priv, sizeof(priv));

			count = priv.num_blue_values = cpriv->num_blue_values;
			for (n = 0; n < count; n++)
				priv.blue_values[n] = (FT_Short)cpriv->blue_values[n];

			count = priv.num_other_blues = cpriv->num_other_blues;
			for (n = 0; n < count; n++)
				priv.other_blues[n] = (FT_Short)cpriv->other_blues[n];

			count = priv.num_family_blues = cpriv->num_family_blues;
			for (n = 0; n < count; n++)
				priv.family_blues[n] = (FT_Short)cpriv->family_blues[n];

			count = priv.num_family_other_blues = cpriv->num_family_other_blues;
			for (n = 0; n < count; n++)
				priv.family_other_blues[n] = (FT_Short)cpriv->family_other_blues[n];

			priv.blue_scale = cpriv->blue_scale;
			priv.blue_shift = (FT_Int)cpriv->blue_shift;
			priv.blue_fuzz  = (FT_Int)cpriv->blue_fuzz;

			priv.standard_width[0]  = (FT_UShort)cpriv->standard_width;
			priv.standard_height[0] = (FT_UShort)cpriv->standard_height;

			count = priv.num_snap_widths = cpriv->num_snap_widths;
			for (n = 0; n < count; n++)
				priv.snap_widths[n] = (FT_Short)cpriv->snap_widths[n];

			count = priv.num_snap_heights = cpriv->num_snap_heights;
			for (n = 0; n < count; n++)
				priv.snap_heights[n] = (FT_Short)cpriv->snap_heights[n];

			priv.force_bold = cpriv->force_bold;
			priv.language_group = cpriv->language_group;
			priv.lenIV = cpriv->lenIV;
		}

		error = funcs->create(size->face->memory, &priv, &globals);
		if (!error)
			size->internal = (FT_Size_Internal)(void *)globals;
	}

	return error;
}

FT_LOCAL_DEF(FT_Error)
cff_size_reset(CFF_Size size) {
	PSH_Globals_Funcs funcs = cff_size_get_globals_funcs(size);
	FT_Error 		  error = 0;

	if (funcs)
		error = funcs->set_scale((PSH_Globals)size->internal, size->metrics.x_scale, size->metrics.y_scale, 0, 0);
	return error;
}


FT_LOCAL(FT_Error)
cff_pixels_size_reset(CFF_Size size, FT_UInt pixel_width, FT_UInt pixel_height) {
	return cff_size_reset(size);
}

FT_LOCAL(FT_Error)
cff_points_size_reset(CFF_Size size, FT_F26Dot6 char_width, FT_F26Dot6 char_height, FT_UInt horz_resolution, FT_UInt vert_resolution) {
	return cff_size_reset(size);
}

/**** SLOT FUNCTIONS ****/

FT_LOCAL_DEF(void)
cff_slot_done(CFF_GlyphSlot slot) {
	slot->root.internal->glyph_hints = 0;
}

FT_LOCAL_DEF(FT_Error)
cff_slot_init(CFF_GlyphSlot slot) {
	CFF_Face face = (CFF_Face)slot->root.face;
	CFF_Font font = (CFF_FontRec *)face->extra.data;
	PSHinter_Service pshinter = (PSHinter_Service)font->pshinter;

	if (pshinter) {
		FT_Module module;

		module = FT_Get_Module(slot->root.face->driver->root.library, "pshinter");
		if (module) {
			T2_Hints_Funcs funcs;

			funcs = pshinter->get_t2_funcs(module);
			slot->root.internal->glyph_hints = (void *)const_cast<T2_Hints_FuncsRec_ *>(funcs);
		}
	}

	return 0;
}


/**** FACE FUNCTIONS ****/

static FT_String *cff_strcpy(FT_Memory memory, const FT_String *source) {
	FT_Error  error;
	FT_String *result = 0;
	FT_Int len = (FT_Int)ft_strlen(source);

	if (!FT_ALLOC(result, len + 1)) {
		FT_MEM_COPY(result, source, len);
		result[len] = 0;
	}

	FT_UNUSED(error);

	return result;
}

FT_LOCAL_DEF(FT_Error)
cff_face_init(FT_Stream stream, CFF_Face face, FT_Int face_index, FT_Int num_params, FT_Parameter *params) {
	FT_Error          error;
	SFNT_Service      sfnt;
	PSNames_Service   psnames;
	PSHinter_Service  pshinter;
	FT_Bool           pure_cff    = 1;
	FT_Bool           sfnt_format = 0;

	const void *tmp_ptr = FT_Get_Module_Interface(face->root.driver->root.library, "sfnt");
	sfnt = const_cast<SFNT_Service>(reinterpret_cast<const SFNT_Interface *>(tmp_ptr));
	if (!sfnt)
		goto Bad_Format;

	tmp_ptr = FT_Get_Module_Interface(face->root.driver->root.library, "psnames");
	psnames = const_cast<PSNames_Service>(reinterpret_cast<const PSNames_Interface *>(tmp_ptr));

	tmp_ptr = FT_Get_Module_Interface(face->root.driver->root.library, "pshinter");
	pshinter = const_cast<PSHinter_Service>(reinterpret_cast<const PSHinter_Interface *>(tmp_ptr));

	/* create input stream from resource */
	if (FT_STREAM_SEEK(0))
		goto Exit;

	/* check that we have a valid OpenType file */
	error = sfnt->init_face(stream, face, face_index, num_params, params);
	if (!error) {
		if (face->format_tag != 0x4F54544FL) { /* `OTTO'; OpenType/CFF font */
			FT_TRACE2(("[not a valid OpenType/CFF font]\n"));
			goto Bad_Format;
		}

		/* if we are performing a simple font format check, exit immediately */
		if (face_index < 0)
			return FT_Err_Ok;

		sfnt_format = 1;

		/* now, the font can be either an OpenType/CFF font, or an SVG CEF */
		/* font in the later case; it doesn't have a `head' table          */
		error = face->goto_table(face, TTAG_head, stream, 0);
		if (!error) {
			pure_cff = 0;

			/* load font directory */
			error = sfnt->load_face(stream, face, face_index, num_params, params);
			if (error)
				goto Exit;
		} else {
			/* load the `cmap' table by hand */
			error = sfnt->load_charmaps(face, stream);
			if (error)
				goto Exit;

			/* XXX: we don't load the GPOS table, as OpenType Layout     */
			/* support will be added later to a layout library on top of */
			/* FreeType 2                                                */
		}

		/* now, load the CFF part of the file */
		error = face->goto_table(face, TTAG_CFF, stream, 0);
		if (error)
			goto Exit;
	} else {
		/* rewind to start of file; we are going to load a pure-CFF font */
		if (FT_STREAM_SEEK(0))
			goto Exit;
		error = FT_Err_Ok;
	}

	/* now load and parse the CFF table in the file */
	{
		CFF_Font  cff;
		FT_Memory memory = face->root.memory;
		FT_Face   root;
		FT_Int32  flags;

		if (FT_NEW(cff))
			goto Exit;

		face->extra.data = cff;
		error = cff_font_load(stream, face_index, cff);
		if (error)
			goto Exit;

		cff->pshinter = pshinter;
		cff->psnames  = psnames;

		/* Complement the root flags with some interesting information. */
		/* Note that this is only necessary for pure CFF and CEF fonts. */

		root = &face->root;
		root->num_glyphs = cff->num_glyphs;

		if (pure_cff) {
			CFF_FontRecDict dict = &cff->top_font.font_dict;

			/* we need the `PSNames' module for pure-CFF and CEF formats */
			if (!psnames) {
				FT_ERROR(("cff_face_init:"));
				FT_ERROR((" cannot open CFF & CEF fonts\n"));
				FT_ERROR(("              "));
				FT_ERROR((" without the `PSNames' module\n"));
				goto Bad_Format;
			}

			/* Set up num_faces. */
			root->num_faces = cff->num_faces;

			/* compute number of glyphs */
			if (dict->cid_registry)
				root->num_glyphs = dict->cid_count;
			else
				root->num_glyphs = cff->charstrings_index.count;

			/* set global bbox, as well as EM size */
			root->bbox.xMin = dict->font_bbox.xMin >> 16;
			root->bbox.yMin = dict->font_bbox.yMin >> 16;
			root->bbox.xMax = (dict->font_bbox.xMax + 0xFFFFU) >> 16;
			root->bbox.yMax = (dict->font_bbox.yMax + 0xFFFFU) >> 16;

			root->ascender  = (FT_Short)(root->bbox.yMax);
			root->descender = (FT_Short)(root->bbox.yMin);
			root->height    = (FT_Short)(((root->ascender - root->descender) * 12) / 10);

			if (dict->units_per_em)
				root->units_per_EM = dict->units_per_em;
			else
				root->units_per_EM = 1000;

			/* retrieve font family & style name */
			root->family_name = cff_index_get_name(&cff->name_index, face_index);
			if (dict->cid_registry)
				root->style_name = cff_strcpy(memory, "Regular"); /* XXXX */
			else
				root->style_name = cff_index_get_sid_string(&cff->string_index, dict->weight, psnames);

			/*******************************************************************/
			/*                                                                 */
			/* Compute face flags.                                             */
			/*                                                                 */
			flags = FT_FACE_FLAG_SCALABLE |  /* scalable outlines */
					FT_FACE_FLAG_HORIZONTAL; /* horizontal data   */

			if (sfnt_format)
				flags |= FT_FACE_FLAG_SFNT;

			/* fixed width font? */
			if (dict->is_fixed_pitch)
				flags |= FT_FACE_FLAG_FIXED_WIDTH;

				/* XXX: WE DO NOT SUPPORT KERNING METRICS IN THE GPOS TABLE FOR NOW */
#if 0
			/* kerning available? */
			if ( face->kern_pairs )
				flags |= FT_FACE_FLAG_KERNING;
#endif

#ifndef FT_CONFIG_OPTION_NO_GLYPH_NAMES
			flags |= FT_FACE_FLAG_GLYPH_NAMES;
#endif

			root->face_flags = flags;

			/*******************************************************************/
			/*                                                                 */
			/* Compute style flags.                                            */
			/*                                                                 */
			flags = 0;

			if (dict->italic_angle)
				flags |= FT_STYLE_FLAG_ITALIC;

			/* XXX: may not be correct */
			if (cff->top_font.private_dict.force_bold)
				flags |= FT_STYLE_FLAG_BOLD;

			root->style_flags = flags;
		}

		/*******************************************************************/
		/*                                                                 */
		/* Compute char maps.                                              */
		/*                                                                 */

		/* Try to synthetize a Unicode charmap if there is none available */
		/* already.  If an OpenType font contains a Unicode "cmap", we    */
		/* will use it, whatever be in the CFF part of the file.          */
		{
			FT_CharMapRec cmaprec;
			FT_CharMap    cmap;
			FT_UInt       nn;
			CFF_Encoding  encoding = &cff->encoding;

			for (nn = 0; nn < (FT_UInt)root->num_charmaps; nn++) {
				cmap = root->charmaps[nn];

				/* Windows Unicode (3,1)? */
				if (cmap->platform_id == 3 && cmap->encoding_id == 1)
					goto Skip_Unicode;

				/* Deprecated Unicode platform id? */
				if (cmap->platform_id == 0)
					goto Skip_Unicode; /* Standard Unicode (deprecated) */
			}

			/* we didn't find a Unicode charmap, synthetize one */
			cmaprec.face = root;
			cmaprec.platform_id = 3;
			cmaprec.encoding_id = 1;
			cmaprec.encoding    = FT_ENCODING_UNICODE;

			nn = (FT_UInt)root->num_charmaps;

			FT_CMap_New(&cff_cmap_unicode_class_rec, NULL, &cmaprec, NULL);

			/* if no Unicode charmap was previously selected, select this one */
			if (root->charmap == NULL && nn != (FT_UInt)root->num_charmaps)
				root->charmap = root->charmaps[nn];

		Skip_Unicode:
			if (encoding->count > 0) {
				FT_CMap_Class clazz;

				cmaprec.face = root;
				cmaprec.platform_id = 7; /* Adobe platform id */

				if (encoding->offset == 0) {
					cmaprec.encoding_id = 0;
					cmaprec.encoding    = FT_ENCODING_ADOBE_STANDARD;
					clazz = &cff_cmap_encoding_class_rec;
				} else if (encoding->offset == 1) {
					cmaprec.encoding_id = 1;
					cmaprec.encoding    = FT_ENCODING_ADOBE_EXPERT;
					clazz = &cff_cmap_encoding_class_rec;
				} else {
					cmaprec.encoding_id = 3;
					cmaprec.encoding    = FT_ENCODING_ADOBE_CUSTOM;
					clazz = &cff_cmap_encoding_class_rec;
				}

				FT_CMap_New(clazz, NULL, &cmaprec, NULL);
			}
		}
	}

Exit:
	return error;

Bad_Format:
	error = FT_Err_Unknown_File_Format;
	goto Exit;
}

FT_LOCAL_DEF(void)
cff_face_done(CFF_Face face) {
	FT_Memory memory  = face->root.memory;
	SFNT_Service sfnt = (SFNT_Service)face->sfnt;

	if (sfnt)
		sfnt->done_face(face);

	{
		CFF_Font cff = (CFF_Font)face->extra.data;

		if (cff) {
			cff_font_done(cff);
			FT_FREE(face->extra.data);
		}
	}
}

FT_LOCAL_DEF(FT_Error)
cff_driver_init(CFF_Driver driver) {
	FT_UNUSED(driver);

	return FT_Err_Ok;
}

FT_LOCAL_DEF(void)
cff_driver_done(CFF_Driver driver) {
	FT_UNUSED(driver);
}

} // End of namespace FreeType213
} // End of namespace AGS3
