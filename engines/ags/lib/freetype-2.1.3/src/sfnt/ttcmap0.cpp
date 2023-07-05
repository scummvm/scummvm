/***************************************************************************/
/*                                                                         */
/*  ttcmap0.c                                                              */
/*                                                                         */
/*    TrueType new character mapping table (cmap) support (body).          */
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


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftdebug.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftobjs.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftstream.h"
#include "ttload.h"
#include "ttcmap0.h"

#include "sferrors.h"

/*************************************************************************/
/*                                                                       */
/* The macro FT2_1_3_COMPONENT is used in trace mode.  It is an implicit      */
/* parameter of the FT2_1_3_TRACE() and FT2_1_3_ERROR() macros, used to print/log  */
/* messages during execution.                                            */
/*                                                                       */
#undef  FT2_1_3_COMPONENT
#define FT2_1_3_COMPONENT  trace_ttcmap

namespace AGS3 {
namespace FreeType213 {

#define TT_PEEK_SHORT   FT2_1_3_PEEK_SHORT
#define TT_PEEK_USHORT  FT2_1_3_PEEK_USHORT
#define TT_PEEK_LONG    FT2_1_3_PEEK_LONG
#define TT_PEEK_ULONG   FT2_1_3_PEEK_ULONG

#define TT_NEXT_SHORT   FT2_1_3_NEXT_SHORT
#define TT_NEXT_USHORT  FT2_1_3_NEXT_USHORT
#define TT_NEXT_LONG    FT2_1_3_NEXT_LONG
#define TT_NEXT_ULONG   FT2_1_3_NEXT_ULONG


FT2_1_3_CALLBACK_DEF( FT2_1_3_Error )
tt_cmap_init( TT_CMap   cmap,
			  FT2_1_3_Byte*  table ) {
	cmap->data = table;
	return 0;
}


/*************************************************************************/
/*************************************************************************/
/*****                                                               *****/
/*****                           FORMAT 0                            *****/
/*****                                                               *****/
/*************************************************************************/
/*************************************************************************/

/*************************************************************************/
/*                                                                       */
/* TABLE OVERVIEW                                                        */
/* --------------                                                        */
/*                                                                       */
/*   NAME        OFFSET         TYPE          DESCRIPTION                */
/*                                                                       */
/*   format      0              USHORT        must be 0                  */
/*   length      2              USHORT        table length in bytes      */
/*   language    4              USHORT        Mac language code          */
/*   glyph_ids   6              BYTE[256]     array of glyph indices     */
/*               262                                                     */
/*                                                                       */

#ifdef TT_CONFIG_CMAP_FORMAT_0

FT2_1_3_CALLBACK_DEF( void )
tt_cmap0_validate( FT2_1_3_Byte*      table,
				   FT2_1_3_Validator  valid ) {
	FT2_1_3_Byte*  p      = table + 2;
	FT2_1_3_UInt   length = TT_NEXT_USHORT( p );


	if ( table + length > valid->limit || length < 262 )
		FT2_1_3_INVALID_TOO_SHORT;

	/* check glyph indices whenever necessary */
	if ( valid->level >= FT2_1_3_VALIDATE_TIGHT ) {
		FT2_1_3_UInt  n, idx;


		p = table + 6;
		for ( n = 0; n < 256; n++ ) {
			idx = *p++;
			if ( idx >= TT_VALID_GLYPH_COUNT( valid ) )
				FT2_1_3_INVALID_GLYPH_ID;
		}
	}
}


FT2_1_3_CALLBACK_DEF( FT2_1_3_UInt )
tt_cmap0_char_index( TT_CMap    cmap,
					 FT2_1_3_UInt32  char_code ) {
	FT2_1_3_Byte*  table = cmap->data;


	return char_code < 256 ? table[6 + char_code] : 0;
}


FT2_1_3_CALLBACK_DEF( FT2_1_3_UInt )
tt_cmap0_char_next( TT_CMap     cmap,
					FT2_1_3_UInt32  *pchar_code ) {
	FT2_1_3_Byte*   table    = cmap->data;
	FT2_1_3_UInt32  charcode = *pchar_code;
	FT2_1_3_UInt32  result   = 0;
	FT2_1_3_UInt    gindex   = 0;


	table += 6;  /* go to glyph ids */
	while ( ++charcode < 256 ) {
		gindex = table[charcode];
		if ( gindex != 0 ) {
			result = charcode;
			break;
		}
	}

	*pchar_code = result;
	return gindex;
}


FT2_1_3_CALLBACK_TABLE_DEF
const TT_CMap_ClassRec  tt_cmap0_class_rec = {
	{
		sizeof( TT_CMapRec ),

		(FT2_1_3_CMap_InitFunc)     tt_cmap_init,
		(FT2_1_3_CMap_DoneFunc)     NULL,
		(FT2_1_3_CMap_CharIndexFunc)tt_cmap0_char_index,
		(FT2_1_3_CMap_CharNextFunc) tt_cmap0_char_next
	},
	0,
	(TT_CMap_ValidateFunc)   tt_cmap0_validate
};

#endif /* TT_CONFIG_CMAP_FORMAT_0 */


/*************************************************************************/
/*************************************************************************/
/*****                                                               *****/
/*****                          FORMAT 2                             *****/
/*****                                                               *****/
/***** This is used for certain CJK encodings that encode text in a  *****/
/***** mixed 8/16 bits encoding along the following lines:           *****/
/*****                                                               *****/
/***** * Certain byte values correspond to an 8-bit character code   *****/
/*****   (typically in the range 0..127 for ASCII compatibility).    *****/
/*****                                                               *****/
/***** * Certain byte values signal the first byte of a 2-byte       *****/
/*****   character code (but these values are also valid as the      *****/
/*****   second byte of a 2-byte character).                         *****/
/*****                                                               *****/
/***** The following charmap lookup and iteration functions all      *****/
/***** assume that the value "charcode" correspond to following:     *****/
/*****                                                               *****/
/*****   - For one byte characters, "charcode" is simply the         *****/
/*****     character code.                                           *****/
/*****                                                               *****/
/*****   - For two byte characters, "charcode" is the 2-byte         *****/
/*****     character code in big endian format.  More exactly:       *****/
/*****                                                               *****/
/*****       (charcode >> 8)    is the first byte value              *****/
/*****       (charcode & 0xFF)  is the second byte value             *****/
/*****                                                               *****/
/***** Note that not all values of "charcode" are valid according    *****/
/***** to these rules, and the function moderately check the         *****/
/***** arguments.                                                    *****/
/*****                                                               *****/
/*************************************************************************/
/*************************************************************************/

/*************************************************************************/
/*                                                                       */
/* TABLE OVERVIEW                                                        */
/* --------------                                                        */
/*                                                                       */
/*   NAME        OFFSET         TYPE            DESCRIPTION              */
/*                                                                       */
/*   format      0              USHORT          must be 2                */
/*   length      2              USHORT          table length in bytes    */
/*   language    4              USHORT          Mac language code        */
/*   keys        6              USHORT[256]     sub-header keys          */
/*   subs        518            SUBHEAD[NSUBS]  sub-headers array        */
/*   glyph_ids   518+NSUB*8     USHORT[]        glyph id array           */
/*                                                                       */
/* The `keys' table is used to map charcode high-bytes to sub-headers.   */
/* The value of `NSUBS' is the number of sub-headers defined in the      */
/* table and is computed by finding the maximum of the `keys' table.     */
/*                                                                       */
/* Note that for any n, `keys[n]' is a byte offset within the `subs'     */
/* table, i.e., it is the corresponding sub-header index multiplied      */
/* by 8.                                                                 */
/*                                                                       */
/* Each sub-header has the following format:                             */
/*                                                                       */
/*   NAME        OFFSET      TYPE            DESCRIPTION                 */
/*                                                                       */
/*   first       0           USHORT          first valid low-byte        */
/*   count       2           USHORT          number of valid low-bytes   */
/*   delta       4           SHORT           see below                   */
/*   offset      6           USHORT          see below                   */
/*                                                                       */
/* A sub-header defines, for each high-byte, the range of valid          */
/* low-bytes within the charmap.  Note that the range defined by `first' */
/* and `count' must be completely included in the interval [0..255]      */
/* according to the specification.                                       */
/*                                                                       */
/* If a character code is contained within a given sub-header, then      */
/* mapping it to a glyph index is done as follows:                       */
/*                                                                       */
/* * The value of `offset' is read.  This is a _byte_ distance from the  */
/*   location of the `offset' field itself into a slice of the           */
/*   `glyph_ids' table.  Let's call it `slice' (it's a USHORT[] too).    */
/*                                                                       */
/* * The value `slice[char.lo - first]' is read.  If it is 0, there is   */
/*   no glyph for the charcode.  Otherwise, the value of `delta' is      */
/*   added to it (modulo 65536) to form a new glyph index.               */
/*                                                                       */
/* It is up to the validation routine to check that all offsets fall     */
/* within the glyph ids table (and not within the `subs' table itself or */
/* outside of the CMap).                                                 */
/*                                                                       */

#ifdef TT_CONFIG_CMAP_FORMAT_2

FT2_1_3_CALLBACK_DEF( void )
tt_cmap2_validate( FT2_1_3_Byte*      table,
				   FT2_1_3_Validator  valid ) {
	FT2_1_3_Byte*  p      = table + 2;           /* skip format */
	FT2_1_3_UInt   length = TT_PEEK_USHORT( p );
	FT2_1_3_UInt   n, max_subs;
	FT2_1_3_Byte*  keys;                         /* keys table */
	FT2_1_3_Byte*  subs;                         /* sub-headers */
	FT2_1_3_Byte*  glyph_ids;                    /* glyph id array */


	if ( table + length > valid->limit || length < 6 + 512 )
		FT2_1_3_INVALID_TOO_SHORT;

	keys = table + 6;

	/* parse keys to compute sub-headers count */
	p        = keys;
	max_subs = 0;
	for ( n = 0; n < 256; n++ ) {
		FT2_1_3_UInt  idx = TT_NEXT_USHORT( p );


		/* value must be multiple of 8 */
		if ( valid->level >= FT2_1_3_VALIDATE_PARANOID && ( idx & 7 ) != 0 )
			FT2_1_3_INVALID_DATA;

		idx >>= 3;

		if ( idx > max_subs )
			max_subs = idx;
	}

	FT2_1_3_ASSERT( p == table + 518 );

	subs      = p;
	glyph_ids = subs + (max_subs + 1) * 8;
	if ( glyph_ids > valid->limit )
		FT2_1_3_INVALID_TOO_SHORT;

	/* parse sub-headers */
	for ( n = 0; n <= max_subs; n++ ) {
		FT2_1_3_UInt   first_code, code_count, offset;
		FT2_1_3_Int    delta;
		FT2_1_3_Byte*  ids;


		first_code = TT_NEXT_USHORT( p );
		code_count = TT_NEXT_USHORT( p );
		delta      = TT_NEXT_SHORT( p );
		offset     = TT_NEXT_USHORT( p );

		/* check range within 0..255 */
		if ( valid->level >= FT2_1_3_VALIDATE_PARANOID ) {
			if ( first_code >= 256 || first_code + code_count > 256 )
				FT2_1_3_INVALID_DATA;
		}

		/* check offset */
		if ( offset != 0 ) {
			ids = p - 2 + offset;
			if ( ids < glyph_ids || ids + code_count*2 > table + length )
				FT2_1_3_INVALID_OFFSET;

			/* check glyph ids */
			if ( valid->level >= FT2_1_3_VALIDATE_TIGHT ) {
				FT2_1_3_Byte*  limit = p + code_count * 2;
				FT2_1_3_UInt   idx;


				for ( ; p < limit; ) {
					idx = TT_NEXT_USHORT( p );
					if ( idx != 0 ) {
						idx = ( idx + delta ) & 0xFFFFU;
						if ( idx >= TT_VALID_GLYPH_COUNT( valid ) )
							FT2_1_3_INVALID_GLYPH_ID;
					}
				}
			}
		}
	}
}


/* return sub header corresponding to a given character code */
/* NULL on invalid charcode                                  */
static FT2_1_3_Byte*
tt_cmap2_get_subheader( FT2_1_3_Byte*   table,
						FT2_1_3_UInt32  char_code ) {
	FT2_1_3_Byte*  result = NULL;


	if ( char_code < 0x10000UL ) {
		FT2_1_3_UInt   char_lo = (FT2_1_3_UInt)( char_code & 0xFF );
		FT2_1_3_UInt   char_hi = (FT2_1_3_UInt)( char_code >> 8 );
		FT2_1_3_Byte*  p       = table + 6;    /* keys table */
		FT2_1_3_Byte*  subs    = table + 518;  /* subheaders table */
		FT2_1_3_Byte*  sub;


		if ( char_hi == 0 ) {
			/* an 8-bit character code -- we use subHeader 0 in this case */
			/* to test whether the character code is in the charmap       */
			/*                                                            */
			sub = subs;  /* jump to first sub-header */

			/* check that the sub-header for this byte is 0, which */
			/* indicates that it's really a valid one-byte value   */
			/* Otherwise, return 0                                 */
			/*                                                     */
			p += char_lo * 2;
			if ( TT_PEEK_USHORT( p ) != 0 )
				goto Exit;
		} else {
			/* a 16-bit character code */
			p  += char_hi * 2;                          /* jump to key entry  */
			sub = subs + ( TT_PEEK_USHORT( p ) & -8 );  /* jump to sub-header */

			/* check that the hi byte isn't a valid one-byte value */
			if ( sub == subs )
				goto Exit;
		}
		result = sub;
	}
Exit:
	return result;
}


FT2_1_3_CALLBACK_DEF( FT2_1_3_UInt )
tt_cmap2_char_index( TT_CMap    cmap,
					 FT2_1_3_UInt32  char_code ) {
	FT2_1_3_Byte*  table   = cmap->data;
	FT2_1_3_UInt   result  = 0;
	FT2_1_3_Byte*  subheader;


	subheader = tt_cmap2_get_subheader( table, char_code );
	if ( subheader ) {
		FT2_1_3_Byte*  p   = subheader;
		FT2_1_3_UInt   idx = (FT2_1_3_UInt)(char_code & 0xFF);
		FT2_1_3_UInt   start, count;
		FT2_1_3_Int    delta;
		FT2_1_3_UInt   offset;


		start  = TT_NEXT_USHORT( p );
		count  = TT_NEXT_USHORT( p );
		delta  = TT_NEXT_SHORT ( p );
		offset = TT_PEEK_USHORT( p );

		idx -= start;
		if ( idx < count && offset != 0 ) {
			p  += offset + 2 * idx;
			idx = TT_PEEK_USHORT( p );

			if ( idx != 0 )
				result = (FT2_1_3_UInt)( idx + delta ) & 0xFFFFU;
		}
	}
	return result;
}


FT2_1_3_CALLBACK_DEF( FT2_1_3_UInt )
tt_cmap2_char_next( TT_CMap     cmap,
					FT2_1_3_UInt32  *pcharcode ) {
	FT2_1_3_Byte*   table    = cmap->data;
	FT2_1_3_UInt    gindex   = 0;
	FT2_1_3_UInt32  result   = 0;
	FT2_1_3_UInt32  charcode = *pcharcode + 1;
	FT2_1_3_Byte*   subheader;


	while ( charcode < 0x10000UL ) {
		subheader = tt_cmap2_get_subheader( table, charcode );
		if ( subheader ) {
			FT2_1_3_Byte*  p       = subheader;
			FT2_1_3_UInt   start   = TT_NEXT_USHORT( p );
			FT2_1_3_UInt   count   = TT_NEXT_USHORT( p );
			FT2_1_3_Int    delta   = TT_NEXT_SHORT ( p );
			FT2_1_3_UInt   offset  = TT_PEEK_USHORT( p );
			FT2_1_3_UInt   char_lo = (FT2_1_3_UInt)( charcode & 0xFF );
			FT2_1_3_UInt   pos, idx;


			if ( offset == 0 )
				goto Next_SubHeader;

			if ( char_lo < start ) {
				char_lo = start;
				pos     = 0;
			} else
				pos = (FT2_1_3_UInt)( char_lo - start );

			p       += offset + pos * 2;
			charcode = ( charcode & -256 ) + char_lo;

			for ( ; pos < count; pos++, charcode++ ) {
				idx = TT_NEXT_USHORT( p );

				if ( idx != 0 ) {
					gindex = ( idx + delta ) & 0xFFFFU;
					if ( gindex != 0 ) {
						result = charcode;
						goto Exit;
					}
				}
			}
		}

		/* jump to next sub-header, i.e. higher byte value */
Next_SubHeader:
		charcode = ( charcode & -256 ) + 256;
	}

Exit:
	*pcharcode = result;

	return gindex;
}


FT2_1_3_CALLBACK_TABLE_DEF
const TT_CMap_ClassRec  tt_cmap2_class_rec = {
	{
		sizeof( TT_CMapRec ),

		(FT2_1_3_CMap_InitFunc)     tt_cmap_init,
		(FT2_1_3_CMap_DoneFunc)     NULL,
		(FT2_1_3_CMap_CharIndexFunc)tt_cmap2_char_index,
		(FT2_1_3_CMap_CharNextFunc) tt_cmap2_char_next
	},
	2,
	(TT_CMap_ValidateFunc)   tt_cmap2_validate
};

#endif /* TT_CONFIG_CMAP_FORMAT_2 */


/*************************************************************************/
/*************************************************************************/
/*****                                                               *****/
/*****                           FORMAT 4                            *****/
/*****                                                               *****/
/*************************************************************************/
/*************************************************************************/

/*************************************************************************/
/*                                                                       */
/* TABLE OVERVIEW                                                        */
/* --------------                                                        */
/*                                                                       */
/*   NAME          OFFSET         TYPE              DESCRIPTION          */
/*                                                                       */
/*   format        0              USHORT            must be 4            */
/*   length        2              USHORT            table length         */
/*                                                  in bytes             */
/*   language      4              USHORT            Mac language code    */
/*                                                                       */
/*   segCountX2    6              USHORT            2*NUM_SEGS           */
/*   searchRange   8              USHORT            2*(1 << LOG_SEGS)    */
/*   entrySelector 10             USHORT            LOG_SEGS             */
/*   rangeShift    12             USHORT            segCountX2 -         */
/*                                                    searchRange        */
/*                                                                       */
/*   endCount      14             USHORT[NUM_SEGS]  end charcode for     */
/*                                                  each segment; last   */
/*                                                  is 0xFFFF            */
/*                                                                       */
/*   pad           14+NUM_SEGS*2  USHORT            padding              */
/*                                                                       */
/*   startCount    16+NUM_SEGS*2  USHORT[NUM_SEGS]  first charcode for   */
/*                                                  each segment         */
/*                                                                       */
/*   idDelta       16+NUM_SEGS*4  SHORT[NUM_SEGS]   delta for each       */
/*                                                  segment              */
/*   idOffset      16+NUM_SEGS*6  SHORT[NUM_SEGS]   range offset for     */
/*                                                  each segment; can be */
/*                                                  zero                 */
/*                                                                       */
/*   glyphIds      16+NUM_SEGS*8  USHORT[]          array of glyph id    */
/*                                                  ranges               */
/*                                                                       */
/* Character codes are modelled by a series of ordered (increasing)      */
/* intervals called segments.  Each segment has start and end codes,     */
/* provided by the `startCount' and `endCount' arrays.  Segments must    */
/* not be overlapping and the last segment should always contain the     */
/* `0xFFFF' endCount.                                                    */
/*                                                                       */
/* The fields `searchRange', `entrySelector' and `rangeShift' are better */
/* ignored (they are traces of over-engineering in the TrueType          */
/* specification).                                                       */
/*                                                                       */
/* Each segment also has a signed `delta', as well as an optional offset */
/* within the `glyphIds' table.                                          */
/*                                                                       */
/* If a segment's idOffset is 0, the glyph index corresponding to any    */
/* charcode within the segment is obtained by adding the value of        */
/* `idDelta' directly to the charcode, modulo 65536.                     */
/*                                                                       */
/* Otherwise, a glyph index is taken from the glyph ids sub-array for    */
/* the segment, and the value of `idDelta' is added to it.               */
/*                                                                       */
/*                                                                       */
/* Finally, note that certain fonts contain invalid charmaps that        */
/* contain end=0xFFFF, start=0xFFFF, delta=0x0001, offset=0xFFFF at the  */
/* of their charmaps (e.g. opens___.ttf which comes with OpenOffice.org) */
/* we need special code to deal with them correctly...                   */
/*                                                                       */

#ifdef TT_CONFIG_CMAP_FORMAT_4

FT2_1_3_CALLBACK_DEF( void )
tt_cmap4_validate( FT2_1_3_Byte*      table,
				   FT2_1_3_Validator  valid ) {
	FT2_1_3_Byte*  p      = table + 2;               /* skip format */
	FT2_1_3_UInt   length = TT_NEXT_USHORT( p );
	FT2_1_3_Byte   *ends, *starts, *offsets, *deltas, *glyph_ids;
	FT2_1_3_UInt   num_segs;


	/* in certain fonts, the `length' field is invalid and goes */
	/* out of bound.  We try to correct this here...            */
	if ( length < 16 )
		FT2_1_3_INVALID_TOO_SHORT;

	if ( table + length > valid->limit ) {
		if ( valid->level >= FT2_1_3_VALIDATE_TIGHT )
			FT2_1_3_INVALID_TOO_SHORT;

		length = (FT2_1_3_UInt)( valid->limit - table );
	}

	p        = table + 6;
	num_segs = TT_NEXT_USHORT( p );   /* read segCountX2 */

	if ( valid->level >= FT2_1_3_VALIDATE_PARANOID ) {
		/* check that we have an even value here */
		if ( num_segs & 1 )
			FT2_1_3_INVALID_DATA;
	}

	num_segs /= 2;

	/* check the search parameters - even though we never use them */
	/*                                                             */
	if ( valid->level >= FT2_1_3_VALIDATE_PARANOID ) {
		/* check the values of 'searchRange', 'entrySelector', 'rangeShift' */
		FT2_1_3_UInt  search_range   = TT_NEXT_USHORT( p );
		FT2_1_3_UInt  entry_selector = TT_NEXT_USHORT( p );
		FT2_1_3_UInt  range_shift    = TT_NEXT_USHORT( p );


		if ( ( search_range | range_shift ) & 1 )  /* must be even values */
			FT2_1_3_INVALID_DATA;

		search_range /= 2;
		range_shift  /= 2;

		/* `search range' is the greatest power of 2 that is <= num_segs */

		if ( search_range                > num_segs                 ||
				search_range * 2            < num_segs                 ||
				search_range + range_shift != num_segs                 ||
				search_range               != ( 1U << entry_selector ) )
			FT2_1_3_INVALID_DATA;
	}

	ends      = table   + 14;
	starts    = table   + 16 + num_segs * 2;
	deltas    = starts  + num_segs * 2;
	offsets   = deltas  + num_segs * 2;
	glyph_ids = offsets + num_segs * 2;

	if ( glyph_ids > table + length )
		FT2_1_3_INVALID_TOO_SHORT;

	/* check last segment, its end count must be FFFF */
	if ( valid->level >= FT2_1_3_VALIDATE_PARANOID ) {
		p = ends + ( num_segs - 1 ) * 2;
		if ( TT_PEEK_USHORT( p ) != 0xFFFFU )
			FT2_1_3_INVALID_DATA;
	}

	/* check that segments are sorted in increasing order and do not */
	/* overlap; check also the offsets                               */
	{
		FT2_1_3_UInt  start, end, last = 0, offset, n;
		FT2_1_3_Int   delta;


		for ( n = 0; n < num_segs; n++ ) {
			p = starts + n * 2;
			start = TT_PEEK_USHORT( p );
			p = ends + n * 2;
			end = TT_PEEK_USHORT( p );
			p = deltas + n * 2;
			delta = TT_PEEK_SHORT( p );
			p = offsets + n * 2;
			offset = TT_PEEK_USHORT( p );

			if ( start > end )
				FT2_1_3_INVALID_DATA;

			/* this test should be performed at default validation level;  */
			/* unfortunately, some popular Asian fonts present overlapping */
			/* ranges in their charmaps                                    */
			/*                                                             */
			if ( valid->level >= FT2_1_3_VALIDATE_TIGHT ) {
				if ( n > 0 && start <= last )
					FT2_1_3_INVALID_DATA;
			}

			if ( offset && offset != 0xFFFFU ) {
				p += offset;  /* start of glyph id array */

				/* check that we point within the glyph ids table only */
				if ( p < glyph_ids                                ||
						p + ( end - start + 1 ) * 2 > table + length )
					FT2_1_3_INVALID_DATA;

				/* check glyph indices within the segment range */
				if ( valid->level >= FT2_1_3_VALIDATE_TIGHT ) {
					FT2_1_3_UInt  i, idx;


					for ( i = start; i < end; i++ ) {
						idx = FT2_1_3_NEXT_USHORT( p );
						if ( idx != 0 ) {
							idx = (FT2_1_3_UInt)( idx + delta ) & 0xFFFFU;

							if ( idx >= TT_VALID_GLYPH_COUNT( valid ) )
								FT2_1_3_INVALID_GLYPH_ID;
						}
					}
				}
			} else if ( offset == 0xFFFFU ) {
				/* Some fonts (erroneously?) use a range offset of 0xFFFF */
				/* to mean missing glyph in cmap table                    */
				/*                                                        */
				if ( valid->level >= FT2_1_3_VALIDATE_PARANOID                     ||
						n != num_segs - 1                                        ||
						!( start == 0xFFFFU && end == 0xFFFFU && delta == 0x1U ) )
					FT2_1_3_INVALID_DATA;
			}

			last = end;
		}
	}
}


FT2_1_3_CALLBACK_DEF( FT2_1_3_UInt )
tt_cmap4_char_index( TT_CMap    cmap,
					 FT2_1_3_UInt32  char_code ) {
	FT2_1_3_Byte*  table  = cmap->data;
	FT2_1_3_UInt   result = 0;


	if ( char_code < 0x10000UL ) {
		FT2_1_3_UInt   idx, num_segs2;
		FT2_1_3_Int    delta;
		FT2_1_3_UInt   code = (FT2_1_3_UInt)char_code;
		FT2_1_3_Byte*  p;


		p         = table + 6;
		num_segs2 = TT_PEEK_USHORT( p ) & -2;  /* be paranoid! */

#if 1
		/* Some fonts have more than 170 segments in their charmaps! */
		/* We changed this function to use a more efficient binary   */
		/* search for improving performance                          */
		{
			FT2_1_3_UInt  min = 0;
			FT2_1_3_UInt  max = num_segs2 >> 1;
			FT2_1_3_UInt  mid, start, end, offset;


			while ( min < max ) {
				mid   = ( min + max ) >> 1;
				p     = table + 14 + mid * 2;
				end   = TT_NEXT_USHORT( p );
				p    += num_segs2;
				start = TT_PEEK_USHORT( p);

				if ( code < start )
					max = mid;

				else if ( code > end )
					min = mid + 1;

				else {
					/* we found the segment */
					idx = code;

					p += num_segs2;
					delta = TT_PEEK_SHORT( p );

					p += num_segs2;
					offset = TT_PEEK_USHORT( p );

					if ( offset == 0xFFFFU )
						goto Exit;

					if ( offset != 0 ) {
						p  += offset + 2 * ( idx - start );
						idx = TT_PEEK_USHORT( p );
					}

					if ( idx != 0 )
						result = (FT2_1_3_UInt)( idx + delta ) & 0xFFFFU;

					goto Exit;
				}
			}
		}

#else /* 0 - old code */

		{
			FT2_1_3_UInt   n;
			FT2_1_3_Byte*  q;


			p = table + 14;               /* ends table   */
			q = table + 16 + num_segs2;   /* starts table */


			for ( n = 0; n < num_segs2; n += 2 ) {
				FT2_1_3_UInt  end   = TT_NEXT_USHORT( p );
				FT2_1_3_UInt  start = TT_NEXT_USHORT( q );
				FT2_1_3_UInt  offset;


				if ( code < start )
					break;

				if ( code <= end ) {
					idx = code;

					p = q + num_segs2 - 2;
					delta = TT_PEEK_SHORT( p );
					p += num_segs2;
					offset = TT_PEEK_USHORT( p );

					if ( offset == 0xFFFFU )
						goto Exit;

					if ( offset != 0 ) {
						p  += offset + 2 * ( idx - start );
						idx = TT_PEEK_USHORT( p );
					}

					if ( idx != 0 )
						result = (FT2_1_3_UInt)( idx + delta ) & 0xFFFFU;
				}
			}
		}

#endif /* 0 */

	}

Exit:
	return result;
}


FT2_1_3_CALLBACK_DEF( FT2_1_3_UInt )
tt_cmap4_char_next( TT_CMap     cmap,
					FT2_1_3_UInt32  *pchar_code ) {
	FT2_1_3_Byte*   table     = cmap->data;
	FT2_1_3_UInt32  result    = 0;
	FT2_1_3_UInt32  char_code = *pchar_code + 1;
	FT2_1_3_UInt    gindex    = 0;
	FT2_1_3_Byte*   p;
	FT2_1_3_Byte*   q;
	FT2_1_3_UInt    code, num_segs2;


	if ( char_code >= 0x10000UL )
		goto Exit;

	code      = (FT2_1_3_UInt)char_code;
	p         = table + 6;
	num_segs2 = TT_PEEK_USHORT(p) & -2;  /* ensure even-ness */

	for (;;) {
		FT2_1_3_UInt  offset, n;
		FT2_1_3_Int   delta;


		p = table + 14;              /* ends table  */
		q = table + 16 + num_segs2;  /* starts table */

		for ( n = 0; n < num_segs2; n += 2 ) {
			FT2_1_3_UInt  end   = TT_NEXT_USHORT( p );
			FT2_1_3_UInt  start = TT_NEXT_USHORT( q );


			if ( code < start )
				code = start;

			if ( code <= end ) {
				p = q + num_segs2 - 2;
				delta = TT_PEEK_SHORT( p );
				p += num_segs2;
				offset = TT_PEEK_USHORT( p );

				if ( offset != 0 && offset != 0xFFFFU ) {
					/* parse the glyph ids array for non-0 index */
					p += offset + ( code - start ) * 2;
					while ( code <= end ) {
						gindex = TT_NEXT_USHORT( p );
						if ( gindex != 0 ) {
							gindex = (FT2_1_3_UInt)( gindex + delta ) & 0xFFFFU;
							if ( gindex != 0 )
								break;
						}
						code++;
					}
				} else if ( offset == 0xFFFFU ) {
					/* an offset of 0xFFFF means an empty glyph in certain fonts! */
					code = end;
					break;
				} else
					gindex = (FT2_1_3_UInt)( code + delta ) & 0xFFFFU;

				if ( gindex == 0 )
					break;

				result = code;
				goto Exit;
			}
		}

		/* loop to next trial charcode */
		if ( code >= 0xFFFFU )
			break;

		code++;
	}
	return (FT2_1_3_UInt)result;

Exit:
	*pchar_code = result;
	return gindex;
}


FT2_1_3_CALLBACK_TABLE_DEF
const TT_CMap_ClassRec  tt_cmap4_class_rec = {
	{
		sizeof ( TT_CMapRec ),

		(FT2_1_3_CMap_InitFunc)     tt_cmap_init,
		(FT2_1_3_CMap_DoneFunc)     NULL,
		(FT2_1_3_CMap_CharIndexFunc)tt_cmap4_char_index,
		(FT2_1_3_CMap_CharNextFunc) tt_cmap4_char_next
	},
	4,
	(TT_CMap_ValidateFunc)   tt_cmap4_validate
};

#endif /* TT_CONFIG_CMAP_FORMAT_4 */


/*************************************************************************/
/*************************************************************************/
/*****                                                               *****/
/*****                          FORMAT 6                             *****/
/*****                                                               *****/
/*************************************************************************/
/*************************************************************************/

/*************************************************************************/
/*                                                                       */
/* TABLE OVERVIEW                                                        */
/* --------------                                                        */
/*                                                                       */
/*   NAME        OFFSET          TYPE             DESCRIPTION            */
/*                                                                       */
/*   format       0              USHORT           must be 4              */
/*   length       2              USHORT           table length in bytes  */
/*   language     4              USHORT           Mac language code      */
/*                                                                       */
/*   first        6              USHORT           first segment code     */
/*   count        8              USHORT           segment size in chars  */
/*   glyphIds     10             USHORT[count]    glyph ids              */
/*                                                                       */
/* A very simplified segment mapping.                                    */
/*                                                                       */

#ifdef TT_CONFIG_CMAP_FORMAT_6

FT2_1_3_CALLBACK_DEF( void )
tt_cmap6_validate( FT2_1_3_Byte*      table,
				   FT2_1_3_Validator  valid ) {
	FT2_1_3_Byte*  p;
	FT2_1_3_UInt   length, /*start,*/ count;


	if ( table + 10 > valid->limit )
		FT2_1_3_INVALID_TOO_SHORT;

	p      = table + 2;
	length = TT_NEXT_USHORT( p );

	p      = table + 6;             /* skip language */
	//start  = TT_NEXT_USHORT( p );
	count  = TT_NEXT_USHORT( p );

	if ( table + length > valid->limit || length < 10 + count * 2 )
		FT2_1_3_INVALID_TOO_SHORT;

	/* check glyph indices */
	if ( valid->level >= FT2_1_3_VALIDATE_TIGHT ) {
		FT2_1_3_UInt  gindex;


		for ( ; count > 0; count-- ) {
			gindex = TT_NEXT_USHORT( p );
			if ( gindex >= TT_VALID_GLYPH_COUNT( valid ) )
				FT2_1_3_INVALID_GLYPH_ID;
		}
	}
}


FT2_1_3_CALLBACK_DEF( FT2_1_3_UInt )
tt_cmap6_char_index( TT_CMap    cmap,
					 FT2_1_3_UInt32  char_code ) {
	FT2_1_3_Byte*  table  = cmap->data;
	FT2_1_3_UInt   result = 0;
	FT2_1_3_Byte*  p      = table + 6;
	FT2_1_3_UInt   start  = TT_NEXT_USHORT( p );
	FT2_1_3_UInt   count  = TT_NEXT_USHORT( p );
	FT2_1_3_UInt   idx    = (FT2_1_3_UInt)( char_code - start );


	if ( idx < count ) {
		p += 2 * idx;
		result = TT_PEEK_USHORT( p );
	}
	return result;
}


FT2_1_3_CALLBACK_DEF( FT2_1_3_UInt )
tt_cmap6_char_next( TT_CMap     cmap,
					FT2_1_3_UInt32  *pchar_code ) {
	FT2_1_3_Byte*   table     = cmap->data;
	FT2_1_3_UInt32  result    = 0;
	FT2_1_3_UInt32  char_code = *pchar_code + 1;
	FT2_1_3_UInt    gindex    = 0;

	FT2_1_3_Byte*   p         = table + 6;
	FT2_1_3_UInt    start     = TT_NEXT_USHORT( p );
	FT2_1_3_UInt    count     = TT_NEXT_USHORT( p );
	FT2_1_3_UInt    idx;


	if ( char_code >= 0x10000UL )
		goto Exit;

	if ( char_code < start )
		char_code = start;

	idx = (FT2_1_3_UInt)( char_code - start );
	p  += 2 * idx;

	for ( ; idx < count; idx++ ) {
		gindex = TT_NEXT_USHORT( p );
		if ( gindex != 0 ) {
			result = char_code;
			break;
		}
		char_code++;
	}

Exit:
	*pchar_code = result;
	return gindex;
}


FT2_1_3_CALLBACK_TABLE_DEF
const TT_CMap_ClassRec  tt_cmap6_class_rec = {
	{
		sizeof ( TT_CMapRec ),

		(FT2_1_3_CMap_InitFunc)     tt_cmap_init,
		(FT2_1_3_CMap_DoneFunc)     NULL,
		(FT2_1_3_CMap_CharIndexFunc)tt_cmap6_char_index,
		(FT2_1_3_CMap_CharNextFunc) tt_cmap6_char_next
	},
	6,
	(TT_CMap_ValidateFunc)   tt_cmap6_validate
};

#endif /* TT_CONFIG_CMAP_FORMAT_6 */


/*************************************************************************/
/*************************************************************************/
/*****                                                               *****/
/*****                          FORMAT 8                             *****/
/*****                                                               *****/
/***** It's hard to completely understand what the OpenType spec     *****/
/***** says about this format, but here is my conclusion.            *****/
/*****                                                               *****/
/***** The purpose of this format is to easily map UTF-16 text to    *****/
/***** glyph indices.  Basically, the `char_code' must be in one of  *****/
/***** the following formats:                                        *****/
/*****                                                               *****/
/*****   - A 16-bit value that isn't part of the Unicode Surrogates  *****/
/*****     Area (i.e. U+D800-U+DFFF).                                *****/
/*****                                                               *****/
/*****   - A 32-bit value, made of two surrogate values, i.e.. if    *****/
/*****     `char_code = (char_hi << 16) | char_lo', then both        *****/
/*****     `char_hi' and `char_lo' must be in the Surrogates Area.   *****/
/*****      Area.                                                    *****/
/*****                                                               *****/
/***** The 'is32' table embedded in the charmap indicates whether a  *****/
/***** given 16-bit value is in the surrogates area or not.          *****/
/*****                                                               *****/
/***** So, for any given `char_code', we can assert the following:   *****/
/*****                                                               *****/
/*****   If `char_hi == 0' then we must have `is32[char_lo] == 0'.   *****/
/*****                                                               *****/
/*****   If `char_hi != 0' then we must have both                    *****/
/*****   `is32[char_hi] != 0' and `is32[char_lo] != 0'.              *****/
/*****                                                               *****/
/*************************************************************************/
/*************************************************************************/

/*************************************************************************/
/*                                                                       */
/* TABLE OVERVIEW                                                        */
/* --------------                                                        */
/*                                                                       */
/*   NAME        OFFSET         TYPE        DESCRIPTION                  */
/*                                                                       */
/*   format      0              USHORT      must be 8                    */
/*   reseved     2              USHORT      reserved                     */
/*   length      4              ULONG       length in bytes              */
/*   language    8              ULONG       Mac language code            */
/*   is32        12             BYTE[8192]  32-bitness bitmap            */
/*   count       8204           ULONG       number of groups             */
/*                                                                       */
/* This header is followed by 'count' groups of the following format:    */
/*                                                                       */
/*   start       0              ULONG       first charcode               */
/*   end         4              ULONG       last charcode                */
/*   startId     8              ULONG       start glyph id for the group */
/*                                                                       */

#ifdef TT_CONFIG_CMAP_FORMAT_8

FT2_1_3_CALLBACK_DEF( void )
tt_cmap8_validate( FT2_1_3_Byte*      table,
				   FT2_1_3_Validator  valid ) {
	FT2_1_3_Byte*   p = table + 4;
	FT2_1_3_Byte*   is32;
	FT2_1_3_UInt32  length;
	FT2_1_3_UInt32  num_groups;


	if ( table + 16 + 8192 > valid->limit )
		FT2_1_3_INVALID_TOO_SHORT;

	length = TT_NEXT_ULONG( p );
	if ( table + length > valid->limit || length < 8208 )
		FT2_1_3_INVALID_TOO_SHORT;

	is32       = table + 12;
	p          = is32  + 8192;          /* skip `is32' array */
	num_groups = TT_NEXT_ULONG( p );

	if ( p + num_groups * 12 > valid->limit )
		FT2_1_3_INVALID_TOO_SHORT;

	/* check groups, they must be in increasing order */
	{
		FT2_1_3_UInt32  n, start, end, start_id, count, last = 0;


		for ( n = 0; n < num_groups; n++ ) {
			FT2_1_3_UInt   hi, lo;


			start    = TT_NEXT_ULONG( p );
			end      = TT_NEXT_ULONG( p );
			start_id = TT_NEXT_ULONG( p );

			if ( start > end )
				FT2_1_3_INVALID_DATA;

			if ( n > 0 && start <= last )
				FT2_1_3_INVALID_DATA;

			if ( valid->level >= FT2_1_3_VALIDATE_TIGHT ) {
				if ( start_id + end - start >= TT_VALID_GLYPH_COUNT( valid ) )
					FT2_1_3_INVALID_GLYPH_ID;

				count = (FT2_1_3_UInt32)( end - start + 1 );

				if ( start & ~0xFFFFU ) {
					/* start_hi != 0; check that is32[i] is 1 for each i in */
					/* the `hi' and `lo' of the range [start..end]          */
					for ( ; count > 0; count--, start++ ) {
						hi = (FT2_1_3_UInt)( start >> 16 );
						lo = (FT2_1_3_UInt)( start & 0xFFFFU );

						if ( (is32[hi >> 3] & ( 0x80 >> ( hi & 7 ) ) ) == 0 )
							FT2_1_3_INVALID_DATA;

						if ( (is32[lo >> 3] & ( 0x80 >> ( lo & 7 ) ) ) == 0 )
							FT2_1_3_INVALID_DATA;
					}
				} else {
					/* start_hi == 0; check that is32[i] is 0 for each i in */
					/* the range [start..end]                               */

					/* end_hi cannot be != 0! */
					if ( end & ~0xFFFFU )
						FT2_1_3_INVALID_DATA;

					for ( ; count > 0; count--, start++ ) {
						lo = (FT2_1_3_UInt)( start & 0xFFFFU );

						if ( (is32[lo >> 3] & ( 0x80 >> ( lo & 7 ) ) ) != 0 )
							FT2_1_3_INVALID_DATA;
					}
				}
			}

			last = end;
		}
	}
}


FT2_1_3_CALLBACK_DEF( FT2_1_3_UInt )
tt_cmap8_char_index( TT_CMap    cmap,
					 FT2_1_3_UInt32  char_code ) {
	FT2_1_3_Byte*   table      = cmap->data;
	FT2_1_3_UInt    result     = 0;
	FT2_1_3_Byte*   p          = table + 8204;
	FT2_1_3_UInt32  num_groups = TT_NEXT_ULONG( p );
	FT2_1_3_UInt32  start, end, start_id;


	for ( ; num_groups > 0; num_groups-- ) {
		start    = TT_NEXT_ULONG( p );
		end      = TT_NEXT_ULONG( p );
		start_id = TT_NEXT_ULONG( p );

		if ( char_code < start )
			break;

		if ( char_code <= end ) {
			result = (FT2_1_3_UInt)( start_id + char_code - start );
			break;
		}
	}
	return result;
}


FT2_1_3_CALLBACK_DEF( FT2_1_3_UInt )
tt_cmap8_char_next( TT_CMap     cmap,
					FT2_1_3_UInt32  *pchar_code ) {
	FT2_1_3_UInt32  result     = 0;
	FT2_1_3_UInt32  char_code  = *pchar_code + 1;
	FT2_1_3_UInt    gindex     = 0;
	FT2_1_3_Byte*   table      = cmap->data;
	FT2_1_3_Byte*   p          = table + 8204;
	FT2_1_3_UInt32  num_groups = TT_NEXT_ULONG( p );
	FT2_1_3_UInt32  start, end, start_id;


	p = table + 8208;

	for ( ; num_groups > 0; num_groups-- ) {
		start    = TT_NEXT_ULONG( p );
		end      = TT_NEXT_ULONG( p );
		start_id = TT_NEXT_ULONG( p );

		if ( char_code < start )
			char_code = start;

		if ( char_code <= end ) {
			gindex = (FT2_1_3_UInt)( char_code - start + start_id );
			if ( gindex != 0 ) {
				result = char_code;
				goto Exit;
			}
		}
	}

Exit:
	*pchar_code = result;
	return gindex;
}


FT2_1_3_CALLBACK_TABLE_DEF
const TT_CMap_ClassRec  tt_cmap8_class_rec = {
	{
		sizeof ( TT_CMapRec ),

		(FT2_1_3_CMap_InitFunc)     tt_cmap_init,
		(FT2_1_3_CMap_DoneFunc)     NULL,
		(FT2_1_3_CMap_CharIndexFunc)tt_cmap8_char_index,
		(FT2_1_3_CMap_CharNextFunc) tt_cmap8_char_next
	},
	8,
	(TT_CMap_ValidateFunc)   tt_cmap8_validate
};

#endif /* TT_CONFIG_CMAP_FORMAT_8 */


/*************************************************************************/
/*************************************************************************/
/*****                                                               *****/
/*****                          FORMAT 10                            *****/
/*****                                                               *****/
/*************************************************************************/
/*************************************************************************/

/*************************************************************************/
/*                                                                       */
/* TABLE OVERVIEW                                                        */
/* --------------                                                        */
/*                                                                       */
/*   NAME      OFFSET  TYPE               DESCRIPTION                    */
/*                                                                       */
/*   format     0      USHORT             must be 10                     */
/*   reserved   2      USHORT             reserved                       */
/*   length     4      ULONG              length in bytes                */
/*   language   8      ULONG              Mac language code              */
/*                                                                       */
/*   start     12      ULONG              first char in range            */
/*   count     16      ULONG              number of chars in range       */
/*   glyphIds  20      USHORT[count]      glyph indices covered          */
/*                                                                       */

#ifdef TT_CONFIG_CMAP_FORMAT_10

FT2_1_3_CALLBACK_DEF( void )
tt_cmap10_validate( FT2_1_3_Byte*      table,
					FT2_1_3_Validator  valid ) {
	FT2_1_3_Byte*  p = table + 4;
	FT2_1_3_ULong  length, start, count;


	if ( table + 20 > valid->limit )
		FT2_1_3_INVALID_TOO_SHORT;

	length = TT_NEXT_ULONG( p );
	p      = table + 12;
	start  = TT_NEXT_ULONG( p );
	count  = TT_NEXT_ULONG( p );

	if ( table + length > valid->limit || length < 20 + count * 2 )
		FT2_1_3_INVALID_TOO_SHORT;

	/* check glyph indices */
	if ( valid->level >= FT2_1_3_VALIDATE_TIGHT ) {
		FT2_1_3_UInt  gindex;


		for ( ; count > 0; count-- ) {
			gindex = TT_NEXT_USHORT( p );
			if ( gindex >= TT_VALID_GLYPH_COUNT( valid ) )
				FT2_1_3_INVALID_GLYPH_ID;
		}
	}
}


FT2_1_3_CALLBACK_DEF( FT2_1_3_UInt )
tt_cmap10_char_index( TT_CMap    cmap,
					  FT2_1_3_UInt32  char_code ) {
	FT2_1_3_Byte*   table  = cmap->data;
	FT2_1_3_UInt    result = 0;
	FT2_1_3_Byte*   p      = table + 12;
	FT2_1_3_UInt32  start  = TT_NEXT_ULONG( p );
	FT2_1_3_UInt32  count  = TT_NEXT_ULONG( p );
	FT2_1_3_UInt32  idx    = (FT2_1_3_ULong)( char_code - start );


	if ( idx < count ) {
		p     += 2 * idx;
		result = TT_PEEK_USHORT( p );
	}
	return result;
}


FT2_1_3_CALLBACK_DEF( FT2_1_3_UInt )
tt_cmap10_char_next( TT_CMap     cmap,
					 FT2_1_3_UInt32  *pchar_code ) {
	FT2_1_3_Byte*   table     = cmap->data;
	FT2_1_3_UInt32  result    = 0;
	FT2_1_3_UInt32  char_code = *pchar_code + 1;
	FT2_1_3_UInt    gindex    = 0;
	FT2_1_3_Byte*   p         = table + 12;
	FT2_1_3_UInt32  start     = TT_NEXT_ULONG( p );
	FT2_1_3_UInt32  count     = TT_NEXT_ULONG( p );
	FT2_1_3_UInt32  idx;


	if ( char_code < start )
		char_code = start;

	idx = (FT2_1_3_UInt32)( char_code - start );
	p  += 2 * idx;

	for ( ; idx < count; idx++ ) {
		gindex = TT_NEXT_USHORT( p );
		if ( gindex != 0 ) {
			result = char_code;
			break;
		}
		char_code++;
	}

	*pchar_code = char_code;
	return gindex;
}


FT2_1_3_CALLBACK_TABLE_DEF
const TT_CMap_ClassRec  tt_cmap10_class_rec = {
	{
		sizeof ( TT_CMapRec ),

		(FT2_1_3_CMap_InitFunc)     tt_cmap_init,
		(FT2_1_3_CMap_DoneFunc)     NULL,
		(FT2_1_3_CMap_CharIndexFunc)tt_cmap10_char_index,
		(FT2_1_3_CMap_CharNextFunc) tt_cmap10_char_next
	},
	10,
	(TT_CMap_ValidateFunc)   tt_cmap10_validate
};

#endif /* TT_CONFIG_CMAP_FORMAT_10 */


/*************************************************************************/
/*************************************************************************/
/*****                                                               *****/
/*****                          FORMAT 12                            *****/
/*****                                                               *****/
/*************************************************************************/
/*************************************************************************/

/*************************************************************************/
/*                                                                       */
/* TABLE OVERVIEW                                                        */
/* --------------                                                        */
/*                                                                       */
/*   NAME        OFFSET     TYPE       DESCRIPTION                       */
/*                                                                       */
/*   format      0          USHORT     must be 12                        */
/*   reserved    2          USHORT     reserved                          */
/*   length      4          ULONG      length in bytes                   */
/*   language    8          ULONG      Mac language code                 */
/*   count       12         ULONG      number of groups                  */
/*               16                                                      */
/*                                                                       */
/* This header is followed by `count' groups of the following format:    */
/*                                                                       */
/*   start       0          ULONG      first charcode                    */
/*   end         4          ULONG      last charcode                     */
/*   startId     8          ULONG      start glyph id for the group      */
/*                                                                       */

#ifdef TT_CONFIG_CMAP_FORMAT_12

FT2_1_3_CALLBACK_DEF( void )
tt_cmap12_validate( FT2_1_3_Byte*      table,
					FT2_1_3_Validator  valid ) {
	FT2_1_3_Byte*   p;
	FT2_1_3_ULong   length;
	FT2_1_3_ULong   num_groups;


	if ( table + 16 > valid->limit )
		FT2_1_3_INVALID_TOO_SHORT;

	p      = table + 4;
	length = TT_NEXT_ULONG( p );

	p          = table + 12;
	num_groups = TT_NEXT_ULONG( p );

	if ( table + length > valid->limit || length < 16 + 12 * num_groups )
		FT2_1_3_INVALID_TOO_SHORT;

	/* check groups, they must be in increasing order */
	{
		FT2_1_3_ULong  n, start, end, start_id, last = 0;


		for ( n = 0; n < num_groups; n++ ) {
			start    = TT_NEXT_ULONG( p );
			end      = TT_NEXT_ULONG( p );
			start_id = TT_NEXT_ULONG( p );

			if ( start > end )
				FT2_1_3_INVALID_DATA;

			if ( n > 0 && start <= last )
				FT2_1_3_INVALID_DATA;

			if ( valid->level >= FT2_1_3_VALIDATE_TIGHT ) {
				if ( start_id + end - start >= TT_VALID_GLYPH_COUNT( valid ) )
					FT2_1_3_INVALID_GLYPH_ID;
			}

			last = end;
		}
	}
}


FT2_1_3_CALLBACK_DEF( FT2_1_3_UInt )
tt_cmap12_char_index( TT_CMap    cmap,
					  FT2_1_3_UInt32  char_code ) {
	FT2_1_3_UInt    result     = 0;
	FT2_1_3_Byte*   table      = cmap->data;
	FT2_1_3_Byte*   p          = table + 12;
	FT2_1_3_UInt32  num_groups = TT_NEXT_ULONG( p );
	FT2_1_3_UInt32  start, end, start_id;


	for ( ; num_groups > 0; num_groups-- ) {
		start    = TT_NEXT_ULONG( p );
		end      = TT_NEXT_ULONG( p );
		start_id = TT_NEXT_ULONG( p );

		if ( char_code < start )
			break;

		if ( char_code <= end ) {
			result = (FT2_1_3_UInt)( start_id + char_code - start );
			break;
		}
	}
	return result;
}


FT2_1_3_CALLBACK_DEF( FT2_1_3_UInt )
tt_cmap12_char_next( TT_CMap     cmap,
					 FT2_1_3_UInt32  *pchar_code ) {
	FT2_1_3_Byte*   table      = cmap->data;
	FT2_1_3_UInt32  result     = 0;
	FT2_1_3_UInt32  char_code  = *pchar_code + 1;
	FT2_1_3_UInt    gindex     = 0;
	FT2_1_3_Byte*   p          = table + 12;
	FT2_1_3_UInt32  num_groups = TT_NEXT_ULONG( p );
	FT2_1_3_UInt32  start, end, start_id;


	p = table + 16;

	for ( ; num_groups > 0; num_groups-- ) {
		start    = TT_NEXT_ULONG( p );
		end      = TT_NEXT_ULONG( p );
		start_id = TT_NEXT_ULONG( p );

		if ( char_code < start )
			char_code = start;

		if ( char_code <= end ) {
			gindex = (FT2_1_3_UInt)(char_code - start + start_id);
			if ( gindex != 0 ) {
				result = char_code;
				goto Exit;
			}
		}
	}

Exit:
	*pchar_code = result;
	return gindex;
}


FT2_1_3_CALLBACK_TABLE_DEF
const TT_CMap_ClassRec  tt_cmap12_class_rec = {
	{
		sizeof ( TT_CMapRec ),

		(FT2_1_3_CMap_InitFunc)     tt_cmap_init,
		(FT2_1_3_CMap_DoneFunc)     NULL,
		(FT2_1_3_CMap_CharIndexFunc)tt_cmap12_char_index,
		(FT2_1_3_CMap_CharNextFunc) tt_cmap12_char_next
	},
	12,
	(TT_CMap_ValidateFunc)   tt_cmap12_validate
};


#endif /* TT_CONFIG_CMAP_FORMAT_12 */


static const TT_CMap_Class  tt_cmap_classes[] = {
#ifdef TT_CONFIG_CMAP_FORMAT_0
	&tt_cmap0_class_rec,
#endif

#ifdef TT_CONFIG_CMAP_FORMAT_2
	&tt_cmap2_class_rec,
#endif

#ifdef TT_CONFIG_CMAP_FORMAT_4
	&tt_cmap4_class_rec,
#endif

#ifdef TT_CONFIG_CMAP_FORMAT_6
	&tt_cmap6_class_rec,
#endif

#ifdef TT_CONFIG_CMAP_FORMAT_8
	&tt_cmap8_class_rec,
#endif

#ifdef TT_CONFIG_CMAP_FORMAT_10
	&tt_cmap10_class_rec,
#endif

#ifdef TT_CONFIG_CMAP_FORMAT_12
	&tt_cmap12_class_rec,
#endif

	NULL,
};


/* parse the `cmap' table and build the corresponding TT_CMap objects */
/* in the current face                                                */
/*                                                                    */
FT2_1_3_LOCAL_DEF( FT2_1_3_Error )
tt_face_build_cmaps( TT_Face  face ) {
	FT2_1_3_Byte*           table = face->cmap_table;
	FT2_1_3_Byte*           limit = table + face->cmap_size;
	FT2_1_3_UInt volatile   num_cmaps;
	FT2_1_3_Byte* volatile  p     = table;


	if ( p + 4 > limit )
		return FT2_1_3_Err_Invalid_Table;

	/* only recognize format 0 */
	if ( TT_NEXT_USHORT( p ) != 0 ) {
		p -= 2;
		FT2_1_3_ERROR(( "tt_face_build_cmaps: unsupported `cmap' table format = %d\n",
				   TT_PEEK_USHORT( p ) ));
		return FT2_1_3_Err_Invalid_Table;
	}

	num_cmaps = TT_NEXT_USHORT( p );

	for ( ; num_cmaps > 0 && p + 8 <= limit; num_cmaps-- ) {
		FT2_1_3_CharMapRec  charmap;
		FT2_1_3_UInt32      offset;


		charmap.platform_id = TT_NEXT_USHORT( p );
		charmap.encoding_id = TT_NEXT_USHORT( p );
		charmap.face        = FT2_1_3_FACE( face );
		charmap.encoding    = FT2_1_3_ENCODING_NONE;  /* will be filled later */
		offset              = TT_NEXT_ULONG( p );

		if ( offset && table + offset + 2 < limit ) {
			FT2_1_3_Byte*                       cmap   = table + offset;
			FT2_1_3_UInt                        format = TT_PEEK_USHORT( cmap );
			const TT_CMap_Class* volatile  pclazz = tt_cmap_classes;
			TT_CMap_Class                  clazz;


			for ( ; *pclazz; pclazz++ ) {
				clazz = *pclazz;
				if ( clazz->format == format ) {
					volatile TT_ValidatorRec  valid;


					ft_validator_init( FT2_1_3_VALIDATOR( &valid ), cmap, limit,
									   FT2_1_3_VALIDATE_DEFAULT );

					valid.num_glyphs = (FT2_1_3_UInt)face->root.num_glyphs;

					if ( ft_setjmp( FT2_1_3_VALIDATOR( &valid )->jump_buffer ) == 0 ) {
						/* validate this cmap sub-table */
						clazz->validate( cmap, FT2_1_3_VALIDATOR( &valid ) );
					}

					if ( valid.validator.error == 0 )
						(void)FT2_1_3_CMap_New( (FT2_1_3_CMap_Class)clazz, cmap, &charmap, NULL );
					else {
						FT2_1_3_ERROR(( "tt_face_build_cmaps:" ));
						FT2_1_3_ERROR(( " broken cmap sub-table ignored!\n" ));
					}
				}
			}
		}
	}

	return 0;
}

} // End of namespace FreeType213
} // End of namespace AGS3

/* END */
