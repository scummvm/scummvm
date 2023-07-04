/***************************************************************************/
/*                                                                         */
/*  t42objs.c                                                              */
/*                                                                         */
/*    Type 42 objects manager (body).                                      */
/*                                                                         */
/*  Copyright 2002 by Roberto Alameda.                                     */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#include "t42objs.h"
#include "t42parse.h"
#include "t42error.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftdebug.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftstream.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/ftlist.h"


#undef  FT2_1_3_COMPONENT
#define FT2_1_3_COMPONENT  trace_t42

namespace AGS3 {
namespace FreeType213 {

static FT2_1_3_Error
T42_Open_Face( T42_Face  face ) {
	T42_LoaderRec  loader;
	T42_Parser     parser;
	T1_Font        type1 = &face->type1;
	FT2_1_3_Memory      memory = face->root.memory;
	FT2_1_3_Error       error;

	PSAux_Service  psaux  = (PSAux_Service)face->psaux;


	t42_loader_init( &loader, face );

	parser = &loader.parser;

	if ( FT2_1_3_ALLOC( face->ttf_data, 12 ) )
		goto Exit;

	error = t42_parser_init( parser,
							 face->root.stream,
							 memory,
							 psaux);
	if ( error )
		goto Exit;

	error = t42_parse_dict( face, &loader, parser->base_dict, parser->base_len );

	if ( type1->font_type != 42 ) {
		error = FT2_1_3_Err_Unknown_File_Format;
		goto Exit;
	}

	/* now, propagate the charstrings and glyphnames tables */
	/* to the Type1 data                                    */
	type1->num_glyphs = loader.num_glyphs;

	if ( !loader.charstrings.init ) {
		FT2_1_3_ERROR(( "T42_Open_Face: no charstrings array in face!\n" ));
		error = FT2_1_3_Err_Invalid_File_Format;
	}

	loader.charstrings.init  = 0;
	type1->charstrings_block = loader.charstrings.block;
	type1->charstrings       = loader.charstrings.elements;
	type1->charstrings_len   = loader.charstrings.lengths;

	/* we copy the glyph names `block' and `elements' fields; */
	/* the `lengths' field must be released later             */
	type1->glyph_names_block    = loader.glyph_names.block;
	type1->glyph_names          = (FT2_1_3_String**)loader.glyph_names.elements;
	loader.glyph_names.block    = 0;
	loader.glyph_names.elements = 0;

	/* we must now build type1.encoding when we have a custom array */
	if ( type1->encoding_type == T1_ENCODING_TYPE_ARRAY ) {
		FT2_1_3_Int    charcode, idx, min_char, max_char;
		FT2_1_3_Byte*  char_name;
		FT2_1_3_Byte*  glyph_name;


		/* OK, we do the following: for each element in the encoding   */
		/* table, look up the index of the glyph having the same name  */
		/* as defined in the CharStrings array.                        */
		/* The index is then stored in type1.encoding.char_index, and  */
		/* the name in type1.encoding.char_name                        */

		min_char = +32000;
		max_char = -32000;

		charcode = 0;
		for ( ; charcode < loader.encoding_table.max_elems; charcode++ ) {
			type1->encoding.char_index[charcode] = 0;
			type1->encoding.char_name [charcode] = (char *)".notdef";

			char_name = loader.encoding_table.elements[charcode];
			if ( char_name )
				for ( idx = 0; idx < type1->num_glyphs; idx++ ) {
					glyph_name = (FT2_1_3_Byte*)type1->glyph_names[idx];
					if ( ft_strcmp( (const char*)char_name,
									(const char*)glyph_name ) == 0 ) {
						type1->encoding.char_index[charcode] = (FT2_1_3_UShort)idx;
						type1->encoding.char_name [charcode] = (char*)glyph_name;

						/* Change min/max encoded char only if glyph name is */
						/* not /.notdef                                      */
						if ( ft_strcmp( (const char*)".notdef",
										(const char*)glyph_name ) != 0 ) {
							if ( charcode < min_char ) min_char = charcode;
							if ( charcode > max_char ) max_char = charcode;
						}
						break;
					}
				}
		}
		type1->encoding.code_first = min_char;
		type1->encoding.code_last  = max_char;
		type1->encoding.num_chars  = loader.num_chars;
	}

Exit:
	t42_loader_done( &loader );
	return error;
}


/***************** Driver Functions *************/


FT2_1_3_LOCAL_DEF( FT2_1_3_Error )
T42_Face_Init( FT2_1_3_Stream      stream,
			   T42_Face       face,
			   FT2_1_3_Int         face_index,
			   FT2_1_3_Int         num_params,
			   FT2_1_3_Parameter*  params ) {
	FT2_1_3_Error          error;
	PSNames_Service   psnames;
	PSAux_Service     psaux;
	FT2_1_3_Face           root    = (FT2_1_3_Face)&face->root;

	FT2_1_3_UNUSED( num_params );
	FT2_1_3_UNUSED( params );
	FT2_1_3_UNUSED( face_index );
	FT2_1_3_UNUSED( stream );


	face->ttf_face       = NULL;
	face->root.num_faces = 1;

	face->psnames = FT2_1_3_Get_Module_Interface( FT2_1_3_FACE_LIBRARY( face ),
					"psnames" );
	psnames = (PSNames_Service)face->psnames;

	face->psaux = FT2_1_3_Get_Module_Interface( FT2_1_3_FACE_LIBRARY( face ),
										   "psaux" );
	psaux = (PSAux_Service)face->psaux;

	/* open the tokenizer, this will also check the font format */
	error = T42_Open_Face( face );
	if ( error )
		goto Exit;

	/* if we just wanted to check the format, leave successfully now */
	if ( face_index < 0 )
		goto Exit;

	/* check the face index */
	if ( face_index != 0 ) {
		FT2_1_3_ERROR(( "T42_Face_Init: invalid face index\n" ));
		error = FT2_1_3_Err_Invalid_Argument;
		goto Exit;
	}

	/* Now, load the font program into the face object */

	/* Init the face object fields */
	/* Now set up root face fields */

	root->num_glyphs   = face->type1.num_glyphs;
	root->num_charmaps = 0;
	root->face_index   = face_index;

	root->face_flags  = FT2_1_3_FACE_FLAG_SCALABLE;
	root->face_flags |= FT2_1_3_FACE_FLAG_HORIZONTAL;
	root->face_flags |= FT2_1_3_FACE_FLAG_GLYPH_NAMES;

	if ( face->type1.font_info.is_fixed_pitch )
		root->face_flags |= FT2_1_3_FACE_FLAG_FIXED_WIDTH;

	/* XXX: TODO -- add kerning with .afm support */

	/* get style name -- be careful, some broken fonts only */
	/* have a `/FontName' dictionary entry!                 */
	root->family_name = face->type1.font_info.family_name;
	if ( root->family_name ) {
		char*  full   = face->type1.font_info.full_name;
		char*  family = root->family_name;


		if ( full ) {
			while ( *family && *full == *family ) {
				family++;
				full++;
			}

			root->style_name = ( *full == ' ' ? full + 1
								 : (char *)"Regular" );
		} else
			root->style_name = (char *)"Regular";
	} else {
		/* do we have a `/FontName'? */
		if ( face->type1.font_name ) {
			root->family_name = face->type1.font_name;
			root->style_name  = (char *)"Regular";
		}
	}

	/* no embedded bitmap support */
	root->num_fixed_sizes = 0;
	root->available_sizes = 0;

	/* Load the TTF font embedded in the T42 font */
	error = FT2_1_3_New_Memory_Face( FT2_1_3_FACE_LIBRARY( face ),
								face->ttf_data,
								face->ttf_size,
								0,
								&face->ttf_face );
	if ( error )
		goto Exit;

	FT2_1_3_Done_Size( face->ttf_face->size );

	/* Ignore info in FontInfo dictionary and use the info from the  */
	/* loaded TTF font.  The PostScript interpreter also ignores it. */
	root->bbox         = face->ttf_face->bbox;
	root->units_per_EM = face->ttf_face->units_per_EM;

	root->ascender  = face->ttf_face->ascender;
	root->descender = face->ttf_face->descender;
	root->height    = face->ttf_face->height;

	root->max_advance_width  = face->ttf_face->max_advance_width;
	root->max_advance_height = face->ttf_face->max_advance_height;

	root->underline_position  = face->type1.font_info.underline_position;
	root->underline_thickness = face->type1.font_info.underline_thickness;

	root->internal->max_points   = 0;
	root->internal->max_contours = 0;

	/* compute style flags */
	root->style_flags = 0;
	if ( face->type1.font_info.italic_angle )
		root->style_flags |= FT2_1_3_STYLE_FLAG_ITALIC;

	if ( face->ttf_face->style_flags & FT2_1_3_STYLE_FLAG_BOLD )
		root->style_flags |= FT2_1_3_STYLE_FLAG_BOLD;

	if ( face->ttf_face->face_flags & FT2_1_3_FACE_FLAG_VERTICAL )
		root->face_flags |= FT2_1_3_FACE_FLAG_VERTICAL;

	{
		if ( psnames && psaux ) {
			FT2_1_3_CharMapRec    charmap;
			T1_CMap_Classes  cmap_classes = psaux->t1_cmap_classes;
			FT2_1_3_CMap_Class    clazz;


			charmap.face = root;

			/* first of all, try to synthetize a Unicode charmap */
			charmap.platform_id = 3;
			charmap.encoding_id = 1;
			charmap.encoding    = FT2_1_3_ENCODING_UNICODE;

			FT2_1_3_CMap_New( cmap_classes->unicode, NULL, &charmap, NULL );

			/* now, generate an Adobe Standard encoding when appropriate */
			charmap.platform_id = 7;
			clazz               = NULL;

			switch ( face->type1.encoding_type ) {
			case T1_ENCODING_TYPE_STANDARD:
				charmap.encoding    = FT2_1_3_ENCODING_ADOBE_STANDARD;
				charmap.encoding_id = 0;
				clazz               = cmap_classes->standard;
				break;

			case T1_ENCODING_TYPE_EXPERT:
				charmap.encoding    = FT2_1_3_ENCODING_ADOBE_EXPERT;
				charmap.encoding_id = 1;
				clazz               = cmap_classes->expert;
				break;

			case T1_ENCODING_TYPE_ARRAY:
				charmap.encoding    = FT2_1_3_ENCODING_ADOBE_CUSTOM;
				charmap.encoding_id = 2;
				clazz               = cmap_classes->custom;
				break;

			case T1_ENCODING_TYPE_ISOLATIN1:
				charmap.encoding    = FT2_1_3_ENCODING_ADOBE_LATIN_1;
				charmap.encoding_id = 3;
				clazz               = cmap_classes->unicode;
				break;

			default:
				;
			}

			if ( clazz )
				FT2_1_3_CMap_New( clazz, NULL, &charmap, NULL );

#if 0
			/* Select default charmap */
			if (root->num_charmaps)
				root->charmap = root->charmaps[0];
#endif
		}
	}
Exit:
	return error;
}


FT2_1_3_LOCAL_DEF( void )
T42_Face_Done( T42_Face  face ) {
	T1_Font      type1;
	PS_FontInfo  info;
	FT2_1_3_Memory    memory;


	if ( face ) {
		type1  = &face->type1;
		info   = &type1->font_info;
		memory = face->root.memory;

		/* delete internal ttf face prior to freeing face->ttf_data */
		if ( face->ttf_face )
			FT2_1_3_Done_Face( face->ttf_face );

		/* release font info strings */
		FT2_1_3_FREE( info->version );
		FT2_1_3_FREE( info->notice );
		FT2_1_3_FREE( info->full_name );
		FT2_1_3_FREE( info->family_name );
		FT2_1_3_FREE( info->weight );

		/* release top dictionary */
		FT2_1_3_FREE( type1->charstrings_len );
		FT2_1_3_FREE( type1->charstrings );
		FT2_1_3_FREE( type1->glyph_names );

		FT2_1_3_FREE( type1->charstrings_block );
		FT2_1_3_FREE( type1->glyph_names_block );

		FT2_1_3_FREE( type1->encoding.char_index );
		FT2_1_3_FREE( type1->encoding.char_name );
		FT2_1_3_FREE( type1->font_name );

		FT2_1_3_FREE( face->ttf_data );

#if 0
		/* release afm data if present */
		if ( face->afm_data )
			T1_Done_AFM( memory, (T1_AFM*)face->afm_data );
#endif

		/* release unicode map, if any */
		FT2_1_3_FREE( face->unicode_map.maps );
		face->unicode_map.num_maps = 0;

		face->root.family_name = 0;
		face->root.style_name  = 0;
	}
}


/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    T42_Driver_Init                                                    */
/*                                                                       */
/* <Description>                                                         */
/*    Initializes a given Type 42 driver object.                         */
/*                                                                       */
/* <Input>                                                               */
/*    driver :: A handle to the target driver object.                    */
/*                                                                       */
/* <Return>                                                              */
/*    FreeType error code.  0 means success.                             */
/*                                                                       */
FT2_1_3_LOCAL_DEF( FT2_1_3_Error )
T42_Driver_Init( T42_Driver  driver ) {
	FT2_1_3_Module  ttmodule;


	ttmodule = FT2_1_3_Get_Module( FT2_1_3_MODULE(driver)->library, "truetype" );
	driver->ttclazz = (FT2_1_3_Driver_Class)ttmodule->clazz;

	return FT2_1_3_Err_Ok;
}


FT2_1_3_LOCAL_DEF( void )
T42_Driver_Done( T42_Driver  driver ) {
	FT2_1_3_UNUSED( driver );
}




FT2_1_3_LOCAL_DEF( FT2_1_3_Error )
T42_Size_Init( T42_Size  size ) {
	FT2_1_3_Face   face = size->root.face;
	T42_Face  t42face = (T42_Face)face;
	FT2_1_3_Size   ttsize;
	FT2_1_3_Error  error   = FT2_1_3_Err_Ok;


	error = FT2_1_3_New_Size( t42face->ttf_face, &ttsize );
	size->ttsize = ttsize;

	FT2_1_3_Activate_Size( ttsize );

	return error;
}


FT2_1_3_LOCAL_DEF( void )
T42_Size_Done( T42_Size  size ) {
	FT2_1_3_Face      face    = size->root.face;
	T42_Face     t42face = (T42_Face)face;
	FT2_1_3_ListNode  node;


	node = FT2_1_3_List_Find( &t42face->ttf_face->sizes_list, size->ttsize );
	if ( node ) {
		FT2_1_3_Done_Size( size->ttsize );
		size->ttsize = NULL;
	}
}


FT2_1_3_LOCAL_DEF( FT2_1_3_Error )
T42_GlyphSlot_Init( T42_GlyphSlot  slot ) {
	FT2_1_3_Face       face    = slot->root.face;
	T42_Face      t42face = (T42_Face)face;
	FT2_1_3_GlyphSlot  ttslot;
	FT2_1_3_Error      error   = FT2_1_3_Err_Ok;


	if ( face->glyph == NULL ) {
		/* First glyph slot for this face */
		slot->ttslot = t42face->ttf_face->glyph;
	} else {
		error = FT2_1_3_New_GlyphSlot( t42face->ttf_face, &ttslot );
		slot->ttslot = ttslot;
	}

	return error;
}


FT2_1_3_LOCAL_DEF( void )
T42_GlyphSlot_Done( T42_GlyphSlot slot ) {
	FT2_1_3_Face       face    = slot->root.face;
	T42_Face      t42face = (T42_Face)face;
	FT2_1_3_GlyphSlot  cur     = t42face->ttf_face->glyph;


	while ( cur ) {
		if ( cur == slot->ttslot ) {
			FT2_1_3_Done_GlyphSlot( slot->ttslot );
			break;
		}

		cur = cur->next;
	}
}



FT2_1_3_LOCAL_DEF( FT2_1_3_Error )
T42_Size_SetChars( T42_Size    size,
				   FT2_1_3_F26Dot6  char_width,
				   FT2_1_3_F26Dot6  char_height,
				   FT2_1_3_UInt     horz_resolution,
				   FT2_1_3_UInt     vert_resolution ) {
	FT2_1_3_Face   face    = size->root.face;
	T42_Face  t42face = (T42_Face)face;


	FT2_1_3_Activate_Size(size->ttsize);

	return FT2_1_3_Set_Char_Size( t42face->ttf_face,
							 char_width,
							 char_height,
							 horz_resolution,
							 vert_resolution );
}


FT2_1_3_LOCAL_DEF( FT2_1_3_Error )
T42_Size_SetPixels( T42_Size  size,
					FT2_1_3_UInt   pixel_width,
					FT2_1_3_UInt   pixel_height ) {
	FT2_1_3_Face   face    = size->root.face;
	T42_Face  t42face = (T42_Face)face;


	FT2_1_3_Activate_Size(size->ttsize);

	return FT2_1_3_Set_Pixel_Sizes( t42face->ttf_face,
							   pixel_width,
							   pixel_height );
}


static void
ft_glyphslot_clear( FT2_1_3_GlyphSlot  slot ) {
	/* free bitmap if needed */
	if ( slot->flags & FT2_1_3_GLYPH_OWN_BITMAP ) {
		FT2_1_3_Memory  memory = FT2_1_3_FACE_MEMORY( slot->face );


		FT2_1_3_FREE( slot->bitmap.buffer );
		slot->flags &= ~FT2_1_3_GLYPH_OWN_BITMAP;
	}

	/* clear all public fields in the glyph slot */
	FT2_1_3_ZERO( &slot->metrics );
	FT2_1_3_ZERO( &slot->outline );
	FT2_1_3_ZERO( &slot->bitmap );

	slot->bitmap_left   = 0;
	slot->bitmap_top    = 0;
	slot->num_subglyphs = 0;
	slot->subglyphs     = 0;
	slot->control_data  = 0;
	slot->control_len   = 0;
	slot->other         = 0;
	slot->format        = FT2_1_3_GLYPH_FORMAT_NONE;

	slot->linearHoriAdvance = 0;
	slot->linearVertAdvance = 0;
}


FT2_1_3_LOCAL_DEF( FT2_1_3_Error )
T42_GlyphSlot_Load( FT2_1_3_GlyphSlot  glyph,
					FT2_1_3_Size       size,
					FT2_1_3_Int        glyph_index,
					FT2_1_3_Int32      load_flags ) {
	FT2_1_3_Error         error;
	T42_GlyphSlot    t42slot = (T42_GlyphSlot)glyph;
	T42_Size         t42size = (T42_Size)size;
	FT2_1_3_Driver_Class  ttclazz = ((T42_Driver)glyph->face->driver)->ttclazz;


	ft_glyphslot_clear( t42slot->ttslot );
	error = ttclazz->load_glyph( t42slot->ttslot,
								 t42size->ttsize,
								 glyph_index,
								 load_flags | FT2_1_3_LOAD_NO_BITMAP );

	if ( !error ) {
		glyph->metrics = t42slot->ttslot->metrics;

		glyph->linearHoriAdvance = t42slot->ttslot->linearHoriAdvance;
		glyph->linearVertAdvance = t42slot->ttslot->linearVertAdvance;

		glyph->format  = t42slot->ttslot->format;
		glyph->outline = t42slot->ttslot->outline;

		glyph->bitmap      = t42slot->ttslot->bitmap;
		glyph->bitmap_left = t42slot->ttslot->bitmap_left;
		glyph->bitmap_top  = t42slot->ttslot->bitmap_top;

		glyph->num_subglyphs = t42slot->ttslot->num_subglyphs;
		glyph->subglyphs     = t42slot->ttslot->subglyphs;

		glyph->control_data  = t42slot->ttslot->control_data;
		glyph->control_len   = t42slot->ttslot->control_len;
	}

	return error;
}

} // End of namespace FreeType213
} // End of namespace AGS3

/* END */
