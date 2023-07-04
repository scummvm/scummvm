/***************************************************************************/
/*                                                                         */
/*  pfrcmap.c                                                              */
/*                                                                         */
/*    FreeType PFR cmap handling (body).                                   */
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


#include "pfrcmap.h"
#include "pfrobjs.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftdebug.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_CALLBACK_DEF( FT2_1_3_Error )
pfr_cmap_init( PFR_CMap  cmap ) {
	PFR_Face  face = (PFR_Face)FT2_1_3_CMAP_FACE( cmap );


	cmap->num_chars = face->phy_font.num_chars;
	cmap->chars     = face->phy_font.chars;

	/* just for safety, check that the character entries are correctly */
	/* sorted in increasing character code order                       */
	{
		FT2_1_3_UInt  n;


		for ( n = 1; n < cmap->num_chars; n++ ) {
			if ( cmap->chars[n - 1].char_code >= cmap->chars[n].char_code )
				FT2_1_3_ASSERT( 0 );
		}
	}

	return 0;
}


FT2_1_3_CALLBACK_DEF( void )
pfr_cmap_done( PFR_CMap  cmap ) {
	cmap->chars     = NULL;
	cmap->num_chars = 0;
}


FT2_1_3_CALLBACK_DEF( FT2_1_3_UInt )
pfr_cmap_char_index( PFR_CMap   cmap,
					 FT2_1_3_UInt32  char_code ) {
	FT2_1_3_UInt   min = 0;
	FT2_1_3_UInt   max = cmap->num_chars;
	FT2_1_3_UInt   mid;
	PFR_Char  gchar;


	while ( min < max ) {
		mid   = min + ( max - min ) / 2;
		gchar = cmap->chars + mid;

		if ( gchar->char_code == char_code )
			return mid + 1;

		if ( gchar->char_code < char_code )
			min = mid + 1;
		else
			max = mid;
	}
	return 0;
}


FT2_1_3_CALLBACK_DEF( FT2_1_3_UInt )
pfr_cmap_char_next( PFR_CMap    cmap,
					FT2_1_3_UInt32  *pchar_code ) {
	FT2_1_3_UInt    result    = 0;
	FT2_1_3_UInt32  char_code = *pchar_code + 1;


Restart: {
		FT2_1_3_UInt   min = 0;
		FT2_1_3_UInt   max = cmap->num_chars;
		FT2_1_3_UInt   mid;
		PFR_Char  gchar;


		while ( min < max ) {
			mid   = min + ( ( max - min ) >> 1 );
			gchar = cmap->chars + mid;

			if ( gchar->char_code == char_code ) {
				result = mid;
				if ( result != 0 ) {
					result++;
					goto Exit;
				}

				char_code++;
				goto Restart;
			}

			if ( gchar->char_code < char_code )
				min = mid+1;
			else
				max = mid;
		}

		/* we didn't find it, but we have a pair just above it */
		char_code = 0;

		if ( min < cmap->num_chars ) {
			gchar  = cmap->chars + min;
			result = min;
			if ( result != 0 ) {
				result++;
				char_code = gchar->char_code;
			}
		}
	}

Exit:
	*pchar_code = char_code;
	return result;
}


FT2_1_3_CALLBACK_TABLE_DEF const FT2_1_3_CMap_ClassRec
pfr_cmap_class_rec = {
	sizeof ( PFR_CMapRec ),

	(FT2_1_3_CMap_InitFunc)     pfr_cmap_init,
	(FT2_1_3_CMap_DoneFunc)     pfr_cmap_done,
	(FT2_1_3_CMap_CharIndexFunc)pfr_cmap_char_index,
	(FT2_1_3_CMap_CharNextFunc) pfr_cmap_char_next
};

} // End of namespace FreeType213
} // End of namespace AGS3

/* END */
