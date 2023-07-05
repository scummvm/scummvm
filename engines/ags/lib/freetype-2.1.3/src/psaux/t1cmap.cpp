/***************************************************************************/
/*                                                                         */
/*  t1cmap.c                                                               */
/*                                                                         */
/*    Type 1 character map support (body).                                 */
/*                                                                         */
/*  Copyright 2002 by                                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#include "t1cmap.h"

#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftdebug.h"

namespace AGS3 {
namespace FreeType213 {

/*************************************************************************/
/*************************************************************************/
/*****                                                               *****/
/*****          TYPE1 STANDARD (AND EXPERT) ENCODING CMAPS           *****/
/*****                                                               *****/
/*************************************************************************/
/*************************************************************************/

static void
t1_cmap_std_init( T1_CMapStd  cmap,
				  FT2_1_3_Int      is_expert ) {
	T1_Face          face    = (T1_Face)FT2_1_3_CMAP_FACE( cmap );
	PSNames_Service psnames = const_cast<PSNames_Service>(reinterpret_cast<const PSNames_Interface *>(face->psnames));

	cmap->num_glyphs    = face->type1.num_glyphs;
	cmap->glyph_names   = (const char* const*)face->type1.glyph_names;
	cmap->sid_to_string = psnames->adobe_std_strings;
	cmap->code_to_sid   = is_expert ? psnames->adobe_expert_encoding
						  : psnames->adobe_std_encoding;

	FT2_1_3_ASSERT( cmap->code_to_sid != NULL );
}


FT2_1_3_CALLBACK_DEF( void )
t1_cmap_std_done( T1_CMapStd  cmap ) {
	cmap->num_glyphs    = 0;
	cmap->glyph_names   = NULL;
	cmap->sid_to_string = NULL;
	cmap->code_to_sid   = NULL;
}


FT2_1_3_CALLBACK_DEF( FT2_1_3_UInt )
t1_cmap_std_char_index( T1_CMapStd  cmap,
						FT2_1_3_UInt32   char_code ) {
	FT2_1_3_UInt  result = 0;


	if ( char_code < 256 ) {
		FT2_1_3_UInt      code, n;
		const char*  glyph_name;


		/* convert character code to Adobe SID string */
		code       = cmap->code_to_sid[char_code];
		glyph_name = cmap->sid_to_string( code );

		/* look for the corresponding glyph name */
		for ( n = 0; n < cmap->num_glyphs; n++ ) {
			const char* gname = cmap->glyph_names[n];


			if ( gname && gname[0] == glyph_name[0]  &&
					ft_strcmp( gname, glyph_name ) == 0 ) {
				result = n;
				break;
			}
		}
	}

	return result;
}


FT2_1_3_CALLBACK_DEF( FT2_1_3_UInt )
t1_cmap_std_char_next( T1_CMapStd   cmap,
					   FT2_1_3_UInt32   *pchar_code ) {
	FT2_1_3_UInt    result    = 0;
	FT2_1_3_UInt32  char_code = *pchar_code + 1;


	while ( char_code < 256 ) {
		result = t1_cmap_std_char_index( cmap, char_code );
		if ( result != 0 )
			goto Exit;

		char_code++;
	}
	char_code = 0;

Exit:
	*pchar_code = char_code;
	return result;
}


FT2_1_3_CALLBACK_DEF( FT2_1_3_Error )
t1_cmap_standard_init( T1_CMapStd  cmap ) {
	t1_cmap_std_init( cmap, 0 );
	return 0;
}


FT2_1_3_CALLBACK_TABLE_DEF const FT2_1_3_CMap_ClassRec
t1_cmap_standard_class_rec = {
	sizeof ( T1_CMapStdRec ),

	(FT2_1_3_CMap_InitFunc)     t1_cmap_standard_init,
	(FT2_1_3_CMap_DoneFunc)     t1_cmap_std_done,
	(FT2_1_3_CMap_CharIndexFunc)t1_cmap_std_char_index,
	(FT2_1_3_CMap_CharNextFunc) t1_cmap_std_char_next
};


FT2_1_3_CALLBACK_DEF( FT2_1_3_Error )
t1_cmap_expert_init( T1_CMapStd  cmap ) {
	t1_cmap_std_init( cmap, 1 );
	return 0;
}

FT2_1_3_CALLBACK_TABLE_DEF const FT2_1_3_CMap_ClassRec
t1_cmap_expert_class_rec = {
	sizeof ( T1_CMapStdRec ),

	(FT2_1_3_CMap_InitFunc)     t1_cmap_expert_init,
	(FT2_1_3_CMap_DoneFunc)     t1_cmap_std_done,
	(FT2_1_3_CMap_CharIndexFunc)t1_cmap_std_char_index,
	(FT2_1_3_CMap_CharNextFunc) t1_cmap_std_char_next
};


/*************************************************************************/
/*************************************************************************/
/*****                                                               *****/
/*****                    TYPE1 CUSTOM ENCODING CMAP                 *****/
/*****                                                               *****/
/*************************************************************************/
/*************************************************************************/


FT2_1_3_CALLBACK_DEF( FT2_1_3_Error )
t1_cmap_custom_init( T1_CMapCustom  cmap ) {
	T1_Face      face     = (T1_Face)FT2_1_3_CMAP_FACE( cmap );
	T1_Encoding  encoding = &face->type1.encoding;


	cmap->first   = encoding->code_first;
	cmap->count   = (FT2_1_3_UInt)( encoding->code_last - cmap->first + 1 );
	cmap->indices = encoding->char_index;

	FT2_1_3_ASSERT( cmap->indices != NULL );
	FT2_1_3_ASSERT( encoding->code_first <= encoding->code_last );

	return 0;
}


FT2_1_3_CALLBACK_DEF( void )
t1_cmap_custom_done( T1_CMapCustom  cmap ) {
	cmap->indices = NULL;
	cmap->first   = 0;
	cmap->count   = 0;
}


FT2_1_3_CALLBACK_DEF( FT2_1_3_UInt )
t1_cmap_custom_char_index( T1_CMapCustom  cmap,
						   FT2_1_3_UInt32      char_code ) {
	FT2_1_3_UInt    result = 0;


	if ( ( char_code >= cmap->first )                  &&
			( char_code < ( cmap->first + cmap->count ) ) )
		result = cmap->indices[char_code];

	return result;
}


FT2_1_3_CALLBACK_DEF( FT2_1_3_UInt )
t1_cmap_custom_char_next( T1_CMapCustom  cmap,
						  FT2_1_3_UInt32     *pchar_code ) {
	FT2_1_3_UInt    result = 0;
	FT2_1_3_UInt32  char_code = *pchar_code;


	++char_code;

	if ( char_code < cmap->first )
		char_code = cmap->first;

	for ( ; char_code < ( cmap->first + cmap->count ); char_code++ ) {
		result = cmap->indices[char_code];
		if ( result != 0 )
			goto Exit;
	}

	char_code = 0;

Exit:
	*pchar_code = char_code;
	return result;
}


FT2_1_3_CALLBACK_TABLE_DEF const FT2_1_3_CMap_ClassRec
t1_cmap_custom_class_rec = {
	sizeof ( T1_CMapCustomRec ),

	(FT2_1_3_CMap_InitFunc)     t1_cmap_custom_init,
	(FT2_1_3_CMap_DoneFunc)     t1_cmap_custom_done,
	(FT2_1_3_CMap_CharIndexFunc)t1_cmap_custom_char_index,
	(FT2_1_3_CMap_CharNextFunc) t1_cmap_custom_char_next
};


/*************************************************************************/
/*************************************************************************/
/*****                                                               *****/
/*****            TYPE1 SYNTHETIC UNICODE ENCODING CMAP              *****/
/*****                                                               *****/
/*************************************************************************/
/*************************************************************************/

FT2_1_3_CALLBACK_DEF( FT2_1_3_Int )
t1_cmap_uni_pair_compare( const void*  pair1,
						  const void*  pair2 ) {
	FT2_1_3_UInt32  u1 = ((T1_CMapUniPair)pair1)->unicode;
	FT2_1_3_UInt32  u2 = ((T1_CMapUniPair)pair2)->unicode;


	if ( u1 < u2 )
		return -1;

	if ( u1 > u2 )
		return +1;

	return 0;
}


FT2_1_3_CALLBACK_DEF( FT2_1_3_Error )
t1_cmap_unicode_init( T1_CMapUnicode  cmap ) {
	FT2_1_3_Error         error;
	FT2_1_3_UInt          count;
	T1_Face          face    = (T1_Face)FT2_1_3_CMAP_FACE( cmap );
	FT2_1_3_Memory        memory  = FT2_1_3_FACE_MEMORY( face );
	PSNames_Service psnames = const_cast<PSNames_Service>(reinterpret_cast<const PSNames_Interface *>(face->psnames));

	cmap->num_pairs = 0;
	cmap->pairs     = NULL;

	count = face->type1.num_glyphs;

	if ( !FT2_1_3_NEW_ARRAY( cmap->pairs, count ) ) {
		FT2_1_3_UInt         n, new_count;
		T1_CMapUniPair  pair;
		FT2_1_3_UInt32       uni_code;


		pair = cmap->pairs;
		for ( n = 0; n < count; n++ ) {
			const char*  gname = face->type1.glyph_names[n];


			/* build unsorted pair table by matching glyph names */
			if ( gname ) {
				uni_code = psnames->unicode_value( gname );

				if ( uni_code != 0 ) {
					pair->unicode = uni_code;
					pair->gindex  = n;
					pair++;
				}
			}
		}

		new_count = (FT2_1_3_UInt)( pair - cmap->pairs );
		if ( new_count == 0 ) {
			/* there are no unicode characters in here! */
			FT2_1_3_FREE( cmap->pairs );
			error = FT2_1_3_Err_Invalid_Argument;
		} else {
			/* re-allocate if the new array is much smaller than the original */
			/* one                                                            */
			if ( new_count != count && new_count < count / 2 ) {
				(void)FT2_1_3_RENEW_ARRAY( cmap->pairs, count, new_count );
				error = 0;
			}

			/* sort the pairs table to allow efficient binary searches */
			ft_qsort( cmap->pairs,
					  new_count,
					  sizeof ( T1_CMapUniPairRec ),
					  t1_cmap_uni_pair_compare );

			cmap->num_pairs = new_count;
		}
	}

	return error;
}


FT2_1_3_CALLBACK_DEF( void )
t1_cmap_unicode_done( T1_CMapUnicode  cmap ) {
	FT2_1_3_Face    face   = FT2_1_3_CMAP_FACE(cmap);
	FT2_1_3_Memory  memory = FT2_1_3_FACE_MEMORY(face);

	FT2_1_3_FREE( cmap->pairs );
	cmap->num_pairs = 0;
}


FT2_1_3_CALLBACK_DEF( FT2_1_3_UInt )
t1_cmap_unicode_char_index( T1_CMapUnicode  cmap,
							FT2_1_3_UInt32       char_code ) {
	FT2_1_3_UInt         min = 0;
	FT2_1_3_UInt         max = cmap->num_pairs;
	FT2_1_3_UInt         mid;
	T1_CMapUniPair  pair;


	while ( min < max ) {
		mid  = min + ( max - min ) / 2;
		pair = cmap->pairs + mid;

		if ( pair->unicode == char_code )
			return pair->gindex;

		if ( pair->unicode < char_code )
			min = mid + 1;
		else
			max = mid;
	}
	return 0;
}


FT2_1_3_CALLBACK_DEF( FT2_1_3_UInt )
t1_cmap_unicode_char_next( T1_CMapUnicode  cmap,
						   FT2_1_3_UInt32      *pchar_code ) {
	FT2_1_3_UInt    result    = 0;
	FT2_1_3_UInt32  char_code = *pchar_code + 1;


Restart: {
		FT2_1_3_UInt         min = 0;
		FT2_1_3_UInt         max = cmap->num_pairs;
		FT2_1_3_UInt         mid;
		T1_CMapUniPair  pair;


		while ( min < max ) {
			mid  = min + ( ( max - min ) >> 1 );
			pair = cmap->pairs + mid;

			if ( pair->unicode == char_code ) {
				result = pair->gindex;
				if ( result != 0 )
					goto Exit;

				char_code++;
				goto Restart;
			}

			if ( pair->unicode < char_code )
				min = mid+1;
			else
				max = mid;
		}

		/* we didn't find it, but we have a pair just above it */
		char_code = 0;

		if ( min < cmap->num_pairs ) {
			pair   = cmap->pairs + min;
			result = pair->gindex;
			if ( result != 0 )
				char_code = pair->unicode;
		}
	}

Exit:
	*pchar_code = char_code;
	return result;
}


FT2_1_3_CALLBACK_TABLE_DEF const FT2_1_3_CMap_ClassRec
t1_cmap_unicode_class_rec = {
	sizeof ( T1_CMapUnicodeRec ),

	(FT2_1_3_CMap_InitFunc)     t1_cmap_unicode_init,
	(FT2_1_3_CMap_DoneFunc)     t1_cmap_unicode_done,
	(FT2_1_3_CMap_CharIndexFunc)t1_cmap_unicode_char_index,
	(FT2_1_3_CMap_CharNextFunc) t1_cmap_unicode_char_next
};

} // End of namespace FreeType213
} // End of namespace AGS3

/* END */
