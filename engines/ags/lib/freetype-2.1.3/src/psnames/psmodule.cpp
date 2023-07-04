/***************************************************************************/
/*                                                                         */
/*  psmodule.c                                                             */
/*                                                                         */
/*    PSNames module implementation (body).                                */
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
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/psnames.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftobjs.h"

#include "psmodule.h"
#include "pstables.h"

#include "psnamerr.h"

namespace AGS3 {
namespace FreeType213 {

#ifndef FT2_1_3_CONFIG_OPTION_NO_POSTSCRIPT_NAMES


#ifdef FT2_1_3_CONFIG_OPTION_ADOBE_GLYPH_LIST


/* return the Unicode value corresponding to a given glyph.  Note that */
/* we do deal with glyph variants by detecting a non-initial dot in    */
/* the name, as in `A.swash' or `e.final', etc.                        */
/*                                                                     */
static FT2_1_3_UInt32
ps_unicode_value( const char*  glyph_name ) {
	FT2_1_3_Int  n;
	char    first = glyph_name[0];
	char    temp[64];


	/* if the name begins with `uni', then the glyph name may be a */
	/* hard-coded unicode character code.                          */
	if ( glyph_name[0] == 'u' &&
			glyph_name[1] == 'n' &&
			glyph_name[2] == 'i' ) {
		/* determine whether the next four characters following are */
		/* hexadecimal.                                             */

		/* XXX: Add code to deal with ligatures, i.e. glyph names like */
		/*      `uniXXXXYYYYZZZZ'...                                   */

		FT2_1_3_Int       count;
		FT2_1_3_ULong     value = 0;
		const char*  p     = glyph_name + 3;


		for ( count = 4; count > 0; count--, p++ ) {
			char          c = *p;
			unsigned int  d;


			d = (unsigned char)c - '0';
			if ( d >= 10 ) {
				d = (unsigned char)c - 'A';
				if ( d >= 6 )
					d = 16;
				else
					d += 10;
			}

			/* exit if a non-uppercase hexadecimal character was found */
			if ( d >= 16 )
				break;

			value = ( value << 4 ) + d;
		}
		if ( count == 0 )
			return value;
	}

	/* look for a non-initial dot in the glyph name in order to */
	/* sort-out variants like `A.swash', `e.final', etc.        */
	{
		const char*  p;
		int          len;


		p = glyph_name;

		while ( *p && *p != '.' )
			p++;

		len = (int)( p - glyph_name );

		if ( *p && len < 64 ) {
			ft_strncpy( temp, glyph_name, len );
			temp[len]  = 0;
			glyph_name = temp;
		}
	}

	/* now, look up the glyph in the Adobe Glyph List */
	for ( n = 0; n < NUM_ADOBE_GLYPHS; n++ ) {
		const char*  name = sid_standard_names[n];


		if ( first == name[0] && ft_strcmp( glyph_name, name ) == 0 )
			return ps_names_to_unicode[n];
	}

	/* not found, there is probably no Unicode value for this glyph name */
	return 0;
}


/* ft_qsort callback to sort the unicode map */
FT2_1_3_CALLBACK_DEF( int )
compare_uni_maps( const void*  a,
				  const void*  b ) {
	PS_UniMap*  map1 = (PS_UniMap*)a;
	PS_UniMap*  map2 = (PS_UniMap*)b;


	return ( map1->unicode - map2->unicode );
}


/* Builds a table that maps Unicode values to glyph indices */
static FT2_1_3_Error
ps_build_unicode_table( FT2_1_3_Memory     memory,
						FT2_1_3_UInt       num_glyphs,
						const char**  glyph_names,
						PS_Unicodes*  table ) {
	FT2_1_3_Error  error;


	/* we first allocate the table */
	table->num_maps = 0;
	table->maps     = 0;

	if ( !FT2_1_3_NEW_ARRAY( table->maps, num_glyphs ) ) {
		FT2_1_3_UInt     n;
		FT2_1_3_UInt     count;
		PS_UniMap*  map;
		FT2_1_3_UInt32   uni_char;


		map = table->maps;

		for ( n = 0; n < num_glyphs; n++ ) {
			const char*  gname = glyph_names[n];


			if ( gname ) {
				uni_char = ps_unicode_value( gname );

				if ( uni_char != 0 && uni_char != 0xFFFF ) {
					map->unicode     = uni_char;
					map->glyph_index = n;
					map++;
				}
			}
		}

		/* now, compress the table a bit */
		count = (FT2_1_3_UInt)( map - table->maps );

		if ( count > 0 && FT2_1_3_REALLOC( table->maps,
									  num_glyphs * sizeof ( PS_UniMap ),
									  count * sizeof ( PS_UniMap ) ) )
			count = 0;

		if ( count == 0 ) {
			FT2_1_3_FREE( table->maps );
			if ( !error )
				error = FT2_1_3_Err_Invalid_Argument;  /* no unicode chars here! */
		} else
			/* sort the table in increasing order of unicode values */
			ft_qsort( table->maps, count, sizeof ( PS_UniMap ), compare_uni_maps );

		table->num_maps = count;
	}

	return error;
}


static FT2_1_3_UInt
ps_lookup_unicode( PS_Unicodes*  table,
				   FT2_1_3_ULong      unicode ) {
	PS_UniMap  *min, *max, *mid;


	/* perform a binary search on the table */

	min = table->maps;
	max = min + table->num_maps - 1;

	while ( min <= max ) {
		mid = min + ( max - min ) / 2;
		if ( mid->unicode == unicode )
			return mid->glyph_index;

		if ( min == max )
			break;

		if ( mid->unicode < unicode )
			min = mid + 1;
		else
			max = mid - 1;
	}

	return 0xFFFF;
}


static FT2_1_3_ULong
ps_next_unicode( PS_Unicodes*  table,
				 FT2_1_3_ULong      unicode ) {
	PS_UniMap  *min, *max, *mid;


	unicode++;
	/* perform a binary search on the table */

	min = table->maps;
	max = min + table->num_maps - 1;

	while ( min <= max ) {
		mid = min + ( max - min ) / 2;
		if ( mid->unicode == unicode )
			return unicode;

		if ( min == max )
			break;

		if ( mid->unicode < unicode )
			min = mid + 1;
		else
			max = mid - 1;
	}

	if ( max < table->maps )
		max = table->maps;

	while ( max < table->maps + table->num_maps ) {
		if ( unicode < max->unicode )
			return max->unicode;
		max++;
	}

	return 0;
}


#endif /* FT2_1_3_CONFIG_OPTION_ADOBE_GLYPH_LIST */


static const char*
ps_get_macintosh_name( FT2_1_3_UInt  name_index ) {
	if ( name_index >= 258 )
		name_index = 0;

	return ps_glyph_names[mac_standard_names[name_index]];
}


static const char*
ps_get_standard_strings( FT2_1_3_UInt  sid ) {
	return ( sid < NUM_SID_GLYPHS ? sid_standard_names[sid] : 0 );
}


static
const PSNames_Interface  psnames_interface = {
#ifdef FT2_1_3_CONFIG_OPTION_ADOBE_GLYPH_LIST

	(PS_Unicode_Value_Func)    ps_unicode_value,
	(PS_Build_Unicodes_Func)   ps_build_unicode_table,
	(PS_Lookup_Unicode_Func)   ps_lookup_unicode,

#else

	0,
	0,
	0,

#endif /* FT2_1_3_CONFIG_OPTION_ADOBE_GLYPH_LIST */

	(PS_Macintosh_Name_Func)    ps_get_macintosh_name,
	(PS_Adobe_Std_Strings_Func) ps_get_standard_strings,

	t1_standard_encoding,
	t1_expert_encoding,

#ifdef FT2_1_3_CONFIG_OPTION_ADOBE_GLYPH_LIST
	(PS_Next_Unicode_Func)     ps_next_unicode
#else
	0
#endif /* FT2_1_3_CONFIG_OPTION_ADOBE_GLYPH_LIST */

};


#endif /* !FT2_1_3_CONFIG_OPTION_NO_POSTSCRIPT_NAMES */


FT2_1_3_CALLBACK_TABLE_DEF
const FT2_1_3_Module_Class  psnames_module_class = {
	0,  /* this is not a font driver, nor a renderer */
	sizeof ( FT2_1_3_ModuleRec ),

	"psnames",  /* driver name                         */
	0x10000L,   /* driver version                      */
	0x20000L,   /* driver requires FreeType 2 or above */

#ifdef FT2_1_3_CONFIG_OPTION_NO_POSTSCRIPT_NAMES
	0,
#else
	(void*)&psnames_interface,   /* module specific interface */
#endif

	(FT2_1_3_Module_Constructor)0,
	(FT2_1_3_Module_Destructor) 0,
	(FT2_1_3_Module_Requester)  0
};

} // End of namespace FreeType213
} // End of namespace AGS3

/* END */
