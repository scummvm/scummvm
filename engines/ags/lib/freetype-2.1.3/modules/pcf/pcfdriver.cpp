/*  pcfdriver.c

	FreeType font driver for pcf files

	Copyright (C) 2000-2001, 2002 by
	Francesco Zappa Nardelli

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"

#include "engines/ags/lib/freetype-2.1.3/ftdebug.h"
#include "engines/ags/lib/freetype-2.1.3/ftstream.h"
#include "engines/ags/lib/freetype-2.1.3/ftobjs.h"
//#include "engines/ags/lib/freetype-2.1.3/ftgzip.h"
#include "engines/ags/lib/freetype-2.1.3/fterrors.h"

#include "pcf.h"
#include "pcfdriver.h"
#include "pcfutil.h"

#include "pcferror.h"

#undef  FT2_1_3_COMPONENT
#define FT2_1_3_COMPONENT  trace_pcfread

namespace AGS3 {
namespace FreeType213 {

typedef struct  PCF_CMapRec_ {
	FT_CMapRec    cmap;
	FT_UInt       num_encodings;
	PCF_Encoding  encodings;

} PCF_CMapRec, *PCF_CMap;


FT2_1_3_CALLBACK_DEF( FT_Error )
pcf_cmap_init( PCF_CMap  cmap ) {
	PCF_Face  face = (PCF_Face)FT2_1_3_CMAP_FACE( cmap );


	cmap->num_encodings = (FT_UInt)face->nencodings;
	cmap->encodings     = face->encodings;

	return FT2_1_3_Err_Ok;
}


FT2_1_3_CALLBACK_DEF( void )
pcf_cmap_done( PCF_CMap  cmap ) {
	cmap->encodings     = NULL;
	cmap->num_encodings = 0;
}


FT2_1_3_CALLBACK_DEF( FT_UInt )
pcf_cmap_char_index( PCF_CMap   cmap,
					 FT_UInt32  charcode ) {
	PCF_Encoding  encodings = cmap->encodings;
	FT_UInt       min, max, mid;
	FT_UInt       result = 0;


	min = 0;
	max = cmap->num_encodings;

	while ( min < max ) {
		FT_UInt32  code;


		mid  = ( min + max ) >> 1;
		code = encodings[mid].enc;

		if ( charcode == code ) {
			result = encodings[mid].glyph + 1;
			break;
		}

		if ( charcode < code )
			max = mid;
		else
			min = mid + 1;
	}

	return result;
}


FT2_1_3_CALLBACK_DEF( FT_UInt )
pcf_cmap_char_next( PCF_CMap    cmap,
					FT_UInt32  *acharcode ) {
	PCF_Encoding  encodings = cmap->encodings;
	FT_UInt       min, max, mid;
	FT_UInt32     charcode = *acharcode + 1;
	FT_UInt       result   = 0;


	min = 0;
	max = cmap->num_encodings;

	while ( min < max ) {
		FT_UInt32  code;


		mid  = ( min + max ) >> 1;
		code = encodings[mid].enc;

		if ( charcode == code ) {
			result = encodings[mid].glyph + 1;
			goto Exit;
		}

		if ( charcode < code )
			max = mid;
		else
			min = mid + 1;
	}

	charcode = 0;
	if ( min < cmap->num_encodings ) {
		charcode = encodings[min].enc;
		result   = encodings[min].glyph + 1;
	}

Exit:
	*acharcode = charcode;
	return result;
}


FT2_1_3_CALLBACK_TABLE_DEF const FT_CMap_ClassRec  pcf_cmap_class = {
	sizeof( PCF_CMapRec ),
	(FT_CMap_InitFunc)     pcf_cmap_init,
	(FT_CMap_DoneFunc)     pcf_cmap_done,
	(FT_CMap_CharIndexFunc)pcf_cmap_char_index,
	(FT_CMap_CharNextFunc) pcf_cmap_char_next
};


/*************************************************************************/
/*                                                                       */
/* The macro FT2_1_3_COMPONENT is used in trace mode.  It is an implicit      */
/* parameter of the FT2_1_3_TRACE() and FT2_1_3_ERROR() macros, used to print/log  */
/* messages during execution.                                            */
/*                                                                       */
#undef  FT2_1_3_COMPONENT
#define FT2_1_3_COMPONENT  trace_pcfdriver


FT2_1_3_CALLBACK_DEF( FT_Error )
PCF_Face_Done( PCF_Face  face ) {
	FT_Memory  memory = FT2_1_3_FACE_MEMORY( face );


	FT2_1_3_FREE( face->encodings );
	FT2_1_3_FREE( face->metrics );

	/* free properties */
	{
		PCF_Property  prop = face->properties;
		FT_Int        i;


		for ( i = 0; i < face->nprops; i++ ) {
			prop = &face->properties[i];

			FT2_1_3_FREE( prop->name );
			if ( prop->isString )
				FT2_1_3_FREE( prop->value );
		}

		FT2_1_3_FREE( face->properties );
	}

	FT2_1_3_FREE( face->toc.tables );
	FT2_1_3_FREE( face->root.family_name );
	FT2_1_3_FREE( face->root.available_sizes );
	FT2_1_3_FREE( face->charset_encoding );
	FT2_1_3_FREE( face->charset_registry );

	FT2_1_3_TRACE4(( "PCF_Face_Done: done face\n" ));

	/* close gzip stream if any */
	if ( face->root.stream == &face->gzip_stream ) {
		FT_Stream_Close( &face->gzip_stream );
		face->root.stream = face->gzip_source;
	}

	return FT2_1_3_Err_Ok;
}


FT2_1_3_CALLBACK_DEF( FT_Error )
PCF_Face_Init( FT_Stream      stream,
			   PCF_Face       face,
			   FT_Int         face_index,
			   FT_Int         num_params,
			   FT_Parameter*  params ) {
	FT_Error  error = FT2_1_3_Err_Ok;

	FT2_1_3_UNUSED( num_params );
	FT2_1_3_UNUSED( params );
	FT2_1_3_UNUSED( face_index );


	error = pcf_load_font( stream, face );
	if ( error ) {
		// FT_Error  error2;

		// /* this didn't work, try gzip support !! */
		// error2 = FT_Stream_OpenGzip( &face->gzip_stream, stream );
		// if ( error2 == FT2_1_3_Err_Unimplemented_Feature )
		// 	goto Fail;

		// error = error2;
		// if ( error )
		// 	goto Fail;

		// face->gzip_source = stream;
		// face->root.stream = &face->gzip_stream;

		// stream = face->root.stream;

		// error = pcf_load_font( stream, face );
		if ( error )
			goto Fail;
	}

	/* set-up charmap */
	{
		FT_String  *charset_registry, *charset_encoding;
		FT_Bool     unicode_charmap  = 0;


		charset_registry = face->charset_registry;
		charset_encoding = face->charset_encoding;

		if ( ( charset_registry != NULL ) &&
				( charset_encoding != NULL ) ) {
			if ( !ft_strcmp( face->charset_registry, "ISO10646" )     ||
					( !ft_strcmp( face->charset_registry, "ISO8859" ) &&
					  !ft_strcmp( face->charset_encoding, "1" )       )  )
				unicode_charmap = 1;
		}

		{
			FT_CharMapRec  charmap;


			charmap.face        = FT2_1_3_FACE( face );
			charmap.encoding    = FT2_1_3_ENCODING_NONE;
			charmap.platform_id = 0;
			charmap.encoding_id = 0;

			if ( unicode_charmap ) {
				charmap.encoding    = FT2_1_3_ENCODING_UNICODE;
				charmap.platform_id = 3;
				charmap.encoding_id = 1;
			}

			error = FT_CMap_New( &pcf_cmap_class, NULL, &charmap, NULL );

#if 0
			/* Select default charmap */
			if (face->root.num_charmaps)
				face->root.charmap = face->root.charmaps[0];
#endif
		}
	}

Exit:
	return error;

Fail:
	FT2_1_3_TRACE2(( "[not a valid PCF file]\n" ));
	error = FT2_1_3_Err_Unknown_File_Format;  /* error */
	goto Exit;
}


static FT_Error
PCF_Set_Pixel_Size( FT_Size  size ) {
	PCF_Face face = (PCF_Face)FT2_1_3_SIZE_FACE( size );


	FT2_1_3_TRACE4(( "rec %d - pres %d\n", size->metrics.y_ppem,
				face->root.available_sizes->height ));

	if ( size->metrics.y_ppem == face->root.available_sizes->height ) {
		size->metrics.ascender    = face->accel.fontAscent << 6;
		size->metrics.descender   = face->accel.fontDescent * (-64);
#if 0
		size->metrics.height      = face->accel.maxbounds.ascent << 6;
#endif
		size->metrics.height      = size->metrics.ascender -
									size->metrics.descender;

		size->metrics.max_advance = face->accel.maxbounds.characterWidth << 6;

		return FT2_1_3_Err_Ok;
	} else {
		FT2_1_3_TRACE4(( "size WRONG\n" ));
		return FT2_1_3_Err_Invalid_Pixel_Size;
	}
}


static FT_Error
PCF_Glyph_Load( FT_GlyphSlot  slot,
				FT_Size       size,
				FT_UInt       glyph_index,
				FT_Int32      load_flags ) {
	PCF_Face    face   = (PCF_Face)FT2_1_3_SIZE_FACE( size );
	FT_Stream   stream = face->root.stream;
	FT_Error    error  = FT2_1_3_Err_Ok;
	FT_Memory   memory = FT2_1_3_FACE( face )->memory;
	FT_Bitmap*  bitmap = &slot->bitmap;
	PCF_Metric  metric;
	int         bytes;

	FT2_1_3_UNUSED( load_flags );


	FT2_1_3_TRACE4(( "load_glyph %d ---", glyph_index ));

	if ( !face ) {
		error = FT2_1_3_Err_Invalid_Argument;
		goto Exit;
	}

	if ( glyph_index > 0 )
		glyph_index--;

	metric = face->metrics + glyph_index;

	bitmap->rows       = metric->ascent + metric->descent;
	bitmap->width      = metric->rightSideBearing - metric->leftSideBearing;
	bitmap->num_grays  = 1;
	bitmap->pixel_mode = FT2_1_3_PIXEL_MODE_MONO;

	FT2_1_3_TRACE6(( "BIT_ORDER %d ; BYTE_ORDER %d ; GLYPH_PAD %d\n",
				PCF_BIT_ORDER( face->bitmapsFormat ),
				PCF_BYTE_ORDER( face->bitmapsFormat ),
				PCF_GLYPH_PAD( face->bitmapsFormat ) ));

	switch ( PCF_GLYPH_PAD( face->bitmapsFormat ) ) {
	case 1:
		bitmap->pitch = ( bitmap->width + 7 ) >> 3;
		break;

	case 2:
		bitmap->pitch = ( ( bitmap->width + 15 ) >> 4 ) << 1;
		break;

	case 4:
		bitmap->pitch = ( ( bitmap->width + 31 ) >> 5 ) << 2;
		break;

	case 8:
		bitmap->pitch = ( ( bitmap->width + 63 ) >> 6 ) << 3;
		break;

	default:
		return FT2_1_3_Err_Invalid_File_Format;
	}

	/* XXX: to do: are there cases that need repadding the bitmap? */
	bytes = bitmap->pitch * bitmap->rows;

	if ( FT2_1_3_ALLOC( bitmap->buffer, bytes ) )
		goto Exit;

	if ( FT2_1_3_STREAM_SEEK( metric->bits )          ||
			FT2_1_3_STREAM_READ( bitmap->buffer, bytes ) )
		goto Exit;

	if ( PCF_BIT_ORDER( face->bitmapsFormat ) != MSBFirst )
		BitOrderInvert( bitmap->buffer, bytes );

	if ( ( PCF_BYTE_ORDER( face->bitmapsFormat ) !=
			PCF_BIT_ORDER( face->bitmapsFormat )  ) ) {
		switch ( PCF_SCAN_UNIT( face->bitmapsFormat ) ) {
		case 1:
			break;

		case 2:
			TwoByteSwap( bitmap->buffer, bytes );
			break;

		case 4:
			FourByteSwap( bitmap->buffer, bytes );
			break;
		}
	}

	slot->bitmap_left = metric->leftSideBearing;
	slot->bitmap_top  = metric->ascent;

	slot->metrics.horiAdvance  = metric->characterWidth << 6 ;
	slot->metrics.horiBearingX = metric->leftSideBearing << 6 ;
	slot->metrics.horiBearingY = metric->ascent << 6 ;
	slot->metrics.width        = ( metric->rightSideBearing -
								   metric->leftSideBearing ) << 6;
	slot->metrics.height       = bitmap->rows << 6;

	slot->linearHoriAdvance = (FT_Fixed)bitmap->width << 16;
	slot->format            = FT2_1_3_GLYPH_FORMAT_BITMAP;
	slot->flags             = FT2_1_3_GLYPH_OWN_BITMAP;

	FT2_1_3_TRACE4(( " --- ok\n" ));

Exit:
	return error;
}


FT2_1_3_CALLBACK_TABLE_DEF
const FT_Driver_ClassRec  pcf_driver_class = {
	{
		ft_module_font_driver,
		sizeof ( FT_DriverRec ),

		"pcf",
		0x10000L,
		0x20000L,

		0,

		(FT_Module_Constructor)0,
		(FT_Module_Destructor) 0,
		(FT_Module_Requester)  0
	},

	sizeof( PCF_FaceRec ),
	sizeof( FT_SizeRec ),
	sizeof( FT_GlyphSlotRec ),

	(FT_Face_InitFunc)        PCF_Face_Init,
	(FT_Face_DoneFunc)        PCF_Face_Done,
	(FT_Size_InitFunc)        0,
	(FT_Size_DoneFunc)        0,
	(FT_Slot_InitFunc)        0,
	(FT_Slot_DoneFunc)        0,

	(FT_Size_ResetPointsFunc) PCF_Set_Pixel_Size,
	(FT_Size_ResetPixelsFunc) PCF_Set_Pixel_Size,

	(FT_Slot_LoadFunc)        PCF_Glyph_Load,

	(FT_Face_GetKerningFunc)  0,
	(FT_Face_AttachFunc)      0,
	(FT_Face_GetAdvancesFunc) 0
};

} // End of namespace FreeType213
} // End of namespace AGS3

/* END */
