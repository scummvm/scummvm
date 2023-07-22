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
/*  ttcmap0.c                                                              */
/*    TrueType new character mapping table (cmap) support (body).          */
/*                                                                         */
/***************************************************************************/


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/ftdebug.h"
#include "engines/ags/lib/freetype-2.1.3/ftobjs.h"
#include "engines/ags/lib/freetype-2.1.3/ftstream.h"
#include "engines/ags/lib/freetype-2.1.3/modules/sfnt/ttload.h"
#include "engines/ags/lib/freetype-2.1.3/modules/sfnt/ttcmap0.h"

#include "engines/ags/lib/freetype-2.1.3/modules/sfnt/sferrors.h"

#undef  FT_COMPONENT
#define FT_COMPONENT  trace_ttcmap

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

FT_CALLBACK_DEF(FT_Error)
tt_cmap_init(TT_CMap cmap, FT_Byte *table) {
	cmap->data = table;
	return 0;
}


/**** FORMAT 0 ****/

#ifdef TT_CONFIG_CMAP_FORMAT_0

FT_CALLBACK_DEF(FT_Error)
tt_cmap0_validate(FT_Byte *table, FT_Validator valid) {
	FT_Byte *p = table + 2;
	FT_UInt length = TT_NEXT_USHORT(p);

	if (table + length > valid->limit || length < 262)
		FT2_1_3_INVALID_TOO_SHORT;

	/* check glyph indices whenever necessary */
	if (valid->level >= FT2_1_3_VALIDATE_TIGHT) {
		FT_UInt n, idx;

		p = table + 6;
		for (n = 0; n < 256; n++) {
			idx = *p++;
			if (idx >= TT_VALID_GLYPH_COUNT(valid))
				FT2_1_3_INVALID_GLYPH_ID;
		}
	}
	return 0;
}

FT_CALLBACK_DEF(FT_UInt)
tt_cmap0_char_index(TT_CMap cmap, FT_UInt32 char_code) {
	FT_Byte *table = cmap->data;

	return char_code < 256 ? table[6 + char_code] : 0;
}

FT_CALLBACK_DEF(FT_UInt)
tt_cmap0_char_next(TT_CMap cmap, FT_UInt32 *pchar_code) {
	FT_Byte   *table   = cmap->data;
	FT_UInt32 charcode = *pchar_code;
	FT_UInt32 result = 0;
	FT_UInt   gindex = 0;

	table += 6; /* go to glyph ids */
	while (++charcode < 256) {
		gindex = table[charcode];
		if (gindex != 0) {
			result = charcode;
			break;
		}
	}

	*pchar_code = result;
	return gindex;
}

FT_CALLBACK_TABLE_DEF
const TT_CMap_ClassRec tt_cmap0_class_rec = {
	{
		sizeof(TT_CMapRec),

		(FT_CMap_InitFunc)      tt_cmap_init,
		(FT_CMap_DoneFunc)      NULL,
		(FT_CMap_CharIndexFunc) tt_cmap0_char_index,
		(FT_CMap_CharNextFunc)  tt_cmap0_char_next
	},
	0,
	(TT_CMap_ValidateFunc) tt_cmap0_validate
};

#endif /* TT_CONFIG_CMAP_FORMAT_0 */


/**** FORMAT 2  ****/

#ifdef TT_CONFIG_CMAP_FORMAT_2

FT_CALLBACK_DEF(FT_Error)
tt_cmap2_validate(FT_Byte *table, FT_Validator valid) {
	FT_Byte *p = table + 2; /* skip format */
	FT_UInt length = TT_PEEK_USHORT(p);
	FT_UInt n, max_subs;
	FT_Byte *keys;      /* keys table */
	FT_Byte *subs;      /* sub-headers */
	FT_Byte *glyph_ids; /* glyph id array */

	if (table + length > valid->limit || length < 6 + 512)
		FT2_1_3_INVALID_TOO_SHORT;

	keys = table + 6;

	/* parse keys to compute sub-headers count */
	p = keys;
	max_subs = 0;
	for (n = 0; n < 256; n++) {
		FT_UInt idx = TT_NEXT_USHORT(p);

		/* value must be multiple of 8 */
		if (valid->level >= FT2_1_3_VALIDATE_PARANOID && (idx & 7) != 0)
			FT2_1_3_INVALID_DATA;

		idx >>= 3;

		if (idx > max_subs)
			max_subs = idx;
	}

	FT2_1_3_ASSERT(p == table + 518);

	subs = p;
	glyph_ids = subs + (max_subs + 1) * 8;
	if (glyph_ids > valid->limit)
		FT2_1_3_INVALID_TOO_SHORT;

	/* parse sub-headers */
	for (n = 0; n <= max_subs; n++) {
		FT_UInt first_code, code_count, offset;
		FT_Int  delta;
		FT_Byte *ids;

		first_code = TT_NEXT_USHORT(p);
		code_count = TT_NEXT_USHORT(p);
		delta  = TT_NEXT_SHORT(p);
		offset = TT_NEXT_USHORT(p);

		/* check range within 0..255 */
		if (valid->level >= FT2_1_3_VALIDATE_PARANOID) {
			if (first_code >= 256 || first_code + code_count > 256)
				FT2_1_3_INVALID_DATA;
		}

		/* check offset */
		if (offset != 0) {
			ids = p - 2 + offset;
			if (ids < glyph_ids || ids + code_count * 2 > table + length)
				FT2_1_3_INVALID_OFFSET;

			/* check glyph ids */
			if (valid->level >= FT2_1_3_VALIDATE_TIGHT) {
				FT_Byte *limit = p + code_count * 2;
				FT_UInt idx;

				for (; p < limit;) {
					idx = TT_NEXT_USHORT(p);
					if (idx != 0) {
						idx = (idx + delta) & 0xFFFFU;
						if (idx >= TT_VALID_GLYPH_COUNT(valid))
							FT2_1_3_INVALID_GLYPH_ID;
					}
				}
			}
		}
	}
	return 0;
}

/* return sub header corresponding to a given character code */
/* NULL on invalid charcode                                  */
static FT_Byte *tt_cmap2_get_subheader(FT_Byte *table, FT_UInt32 char_code) {
	FT_Byte *result = NULL;

	if (char_code < 0x10000UL) {
		FT_UInt char_lo = (FT_UInt)(char_code & 0xFF);
		FT_UInt char_hi = (FT_UInt)(char_code >> 8);
		FT_Byte *p = table + 6;      /* keys table */
		FT_Byte *subs = table + 518; /* subheaders table */
		FT_Byte *sub;

		if (char_hi == 0) {
			/* an 8-bit character code -- we use subHeader 0 in this case */
			/* to test whether the character code is in the charmap       */
			/*                                                            */
			sub = subs; /* jump to first sub-header */

			/* check that the sub-header for this byte is 0, which */
			/* indicates that it's really a valid one-byte value   */
			/* Otherwise, return 0                                 */
			/*                                                     */
			p += char_lo * 2;
			if (TT_PEEK_USHORT(p) != 0)
				goto Exit;
		} else {
			/* a 16-bit character code */
			p += char_hi * 2;                      /* jump to key entry  */
			sub = subs + (TT_PEEK_USHORT(p) & -8); /* jump to sub-header */

			/* check that the hi byte isn't a valid one-byte value */
			if (sub == subs)
				goto Exit;
		}
		result = sub;
	}
Exit:
	return result;
}

FT_CALLBACK_DEF(FT_UInt)
tt_cmap2_char_index(TT_CMap cmap, FT_UInt32 char_code) {
	FT_Byte *table = cmap->data;
	FT_UInt result = 0;
	FT_Byte *subheader;

	subheader = tt_cmap2_get_subheader(table, char_code);
	if (subheader) {
		FT_Byte *p  = subheader;
		FT_UInt idx = (FT_UInt)(char_code & 0xFF);
		FT_UInt start, count;
		FT_Int  delta;
		FT_UInt offset;

		start  = TT_NEXT_USHORT(p);
		count  = TT_NEXT_USHORT(p);
		delta  = TT_NEXT_SHORT(p);
		offset = TT_PEEK_USHORT(p);

		idx -= start;
		if (idx < count && offset != 0) {
			p += offset + 2 * idx;
			idx = TT_PEEK_USHORT(p);

			if (idx != 0)
				result = (FT_UInt)(idx + delta) & 0xFFFFU;
		}
	}
	return result;
}

FT_CALLBACK_DEF(FT_UInt)
tt_cmap2_char_next(TT_CMap cmap, FT_UInt32 *pcharcode) {
	FT_Byte   *table   = cmap->data;
	FT_UInt   gindex   = 0;
	FT_UInt32 result   = 0;
	FT_UInt32 charcode = *pcharcode + 1;
	FT_Byte   *subheader;

	while (charcode < 0x10000UL) {
		subheader = tt_cmap2_get_subheader(table, charcode);
		if (subheader) {
			FT_Byte *p = subheader;
			FT_UInt start   = TT_NEXT_USHORT(p);
			FT_UInt count   = TT_NEXT_USHORT(p);
			FT_Int  delta   = TT_NEXT_SHORT(p);
			FT_UInt offset  = TT_PEEK_USHORT(p);
			FT_UInt char_lo = (FT_UInt)(charcode & 0xFF);
			FT_UInt pos, idx;

			if (offset == 0)
				goto Next_SubHeader;

			if (char_lo < start) {
				char_lo = start;
				pos = 0;
			} else
				pos = (FT_UInt)(char_lo - start);

			p += offset + pos * 2;
			charcode = (charcode & -256) + char_lo;

			for (; pos < count; pos++, charcode++) {
				idx = TT_NEXT_USHORT(p);

				if (idx != 0) {
					gindex = (idx + delta) & 0xFFFFU;
					if (gindex != 0) {
						result = charcode;
						goto Exit;
					}
				}
			}
		}

		/* jump to next sub-header, i.e. higher byte value */
	Next_SubHeader:
		charcode = (charcode & -256) + 256;
	}

Exit:
	*pcharcode = result;

	return gindex;
}

FT_CALLBACK_TABLE_DEF
const TT_CMap_ClassRec tt_cmap2_class_rec = {
	{
		sizeof(TT_CMapRec),

		(FT_CMap_InitFunc)      tt_cmap_init,
		(FT_CMap_DoneFunc)      NULL,
		(FT_CMap_CharIndexFunc) tt_cmap2_char_index,
		(FT_CMap_CharNextFunc)  tt_cmap2_char_next
	},
	2,
	(TT_CMap_ValidateFunc)  tt_cmap2_validate
};

#endif /* TT_CONFIG_CMAP_FORMAT_2 */


/**** FORMAT 4 ****/

#ifdef TT_CONFIG_CMAP_FORMAT_4

FT_CALLBACK_DEF(FT_Error)
tt_cmap4_validate(FT_Byte *table, FT_Validator valid) {
	FT_Byte *p 	   = table + 2; /* skip format */
	FT_UInt length = TT_NEXT_USHORT(p);
	FT_Byte *ends, *starts, *offsets, *deltas, *glyph_ids;
	FT_UInt num_segs;

	/* in certain fonts, the `length' field is invalid and goes */
	/* out of bound.  We try to correct this here...            */
	if (length < 16)
		FT2_1_3_INVALID_TOO_SHORT;

	if (table + length > valid->limit) {
		if (valid->level >= FT2_1_3_VALIDATE_TIGHT)
			FT2_1_3_INVALID_TOO_SHORT;

		length = (FT_UInt)(valid->limit - table);
	}

	p = table + 6;
	num_segs = TT_NEXT_USHORT(p); /* read segCountX2 */

	if (valid->level >= FT2_1_3_VALIDATE_PARANOID) {
		/* check that we have an even value here */
		if (num_segs & 1)
			FT2_1_3_INVALID_DATA;
	}

	num_segs /= 2;

	/* check the search parameters - even though we never use them */
	/*                                                             */
	if (valid->level >= FT2_1_3_VALIDATE_PARANOID) {
		/* check the values of 'searchRange', 'entrySelector', 'rangeShift' */
		FT_UInt search_range   = TT_NEXT_USHORT(p);
		FT_UInt entry_selector = TT_NEXT_USHORT(p);
		FT_UInt range_shift    = TT_NEXT_USHORT(p);

		if ((search_range | range_shift) & 1) /* must be even values */
			FT2_1_3_INVALID_DATA;

		search_range /= 2;
		range_shift /= 2;

		/* `search range' is the greatest power of 2 that is <= num_segs */

		if (search_range > num_segs || search_range * 2 < num_segs || search_range + range_shift != num_segs || search_range != (1U << entry_selector))
			FT2_1_3_INVALID_DATA;
	}

	ends   	  = table + 14;
	starts 	  = table + 16 + num_segs * 2;
	deltas 	  = starts + num_segs * 2;
	offsets	  = deltas + num_segs * 2;
	glyph_ids = offsets + num_segs * 2;

	if (glyph_ids > table + length)
		FT2_1_3_INVALID_TOO_SHORT;

	/* check last segment, its end count must be FFFF */
	if (valid->level >= FT2_1_3_VALIDATE_PARANOID) {
		p = ends + (num_segs - 1) * 2;
		if (TT_PEEK_USHORT(p) != 0xFFFFU)
			FT2_1_3_INVALID_DATA;
	}

	/* check that segments are sorted in increasing order and do not */
	/* overlap; check also the offsets                               */
	{
		FT_UInt start, end, last = 0, offset, n;
		FT_Int delta;

		for (n = 0; n < num_segs; n++) {
			p = starts + n * 2;
			start = TT_PEEK_USHORT(p);
			p = ends + n * 2;
			end = TT_PEEK_USHORT(p);
			p = deltas + n * 2;
			delta = TT_PEEK_SHORT(p);
			p = offsets + n * 2;
			offset = TT_PEEK_USHORT(p);

			if (start > end)
				FT2_1_3_INVALID_DATA;

			/* this test should be performed at default validation level;  */
			/* unfortunately, some popular Asian fonts present overlapping */
			/* ranges in their charmaps                                    */
			/*                                                             */
			if (valid->level >= FT2_1_3_VALIDATE_TIGHT) {
				if (n > 0 && start <= last)
					FT2_1_3_INVALID_DATA;
			}

			if (offset && offset != 0xFFFFU) {
				p += offset; /* start of glyph id array */

				/* check that we point within the glyph ids table only */
				if (p < glyph_ids || p + (end - start + 1) * 2 > table + length)
					FT2_1_3_INVALID_DATA;

				/* check glyph indices within the segment range */
				if (valid->level >= FT2_1_3_VALIDATE_TIGHT) {
					FT_UInt i, idx;

					for (i = start; i < end; i++) {
						idx = FT2_1_3_NEXT_USHORT(p);
						if (idx != 0) {
							idx = (FT_UInt)(idx + delta) & 0xFFFFU;

							if (idx >= TT_VALID_GLYPH_COUNT(valid))
								FT2_1_3_INVALID_GLYPH_ID;
						}
					}
				}
			} else if (offset == 0xFFFFU) {
				/* Some fonts (erroneously?) use a range offset of 0xFFFF */
				/* to mean missing glyph in cmap table                    */
				/*                                                        */
				if (valid->level >= FT2_1_3_VALIDATE_PARANOID || n != num_segs - 1 || !(start == 0xFFFFU && end == 0xFFFFU && delta == 0x1U))
					FT2_1_3_INVALID_DATA;
			}

			last = end;
		}
	}
	return 0;
}

FT_CALLBACK_DEF(FT_UInt)
tt_cmap4_char_index(TT_CMap cmap, FT_UInt32 char_code) {
	FT_Byte *table = cmap->data;
	FT_UInt result = 0;

	if (char_code < 0x10000UL) {
		FT_UInt idx, num_segs2;
		FT_Int  delta;
		FT_UInt code = (FT_UInt)char_code;
		FT_Byte *p;

		p = table + 6;
		num_segs2 = TT_PEEK_USHORT(p) & -2; /* be paranoid! */

#if 1
		/* Some fonts have more than 170 segments in their charmaps! */
		/* We changed this function to use a more efficient binary   */
		/* search for improving performance                          */
		{
			FT_UInt min = 0;
			FT_UInt max = num_segs2 >> 1;
			FT_UInt mid, start, end, offset;

			while (min < max) {
				mid = (min + max) >> 1;
				p = table + 14 + mid * 2;
				end = TT_NEXT_USHORT(p);
				p += num_segs2;
				start = TT_PEEK_USHORT(p);

				if (code < start)
					max = mid;

				else if (code > end)
					min = mid + 1;

				else {
					/* we found the segment */
					idx = code;

					p += num_segs2;
					delta = TT_PEEK_SHORT(p);

					p += num_segs2;
					offset = TT_PEEK_USHORT(p);

					if (offset == 0xFFFFU)
						goto Exit;

					if (offset != 0) {
						p += offset + 2 * (idx - start);
						idx = TT_PEEK_USHORT(p);
					}

					if (idx != 0)
						result = (FT_UInt)(idx + delta) & 0xFFFFU;

					goto Exit;
				}
			}
		}

#else /* 0 - old code */

		{
			FT_UInt n;
			FT_Byte *q;

			p = table + 14;             /* ends table   */
			q = table + 16 + num_segs2; /* starts table */

			for (n = 0; n < num_segs2; n += 2) {
				FT_UInt end = TT_NEXT_USHORT(p);
				FT_UInt start = TT_NEXT_USHORT(q);
				FT_UInt offset;

				if (code < start)
					break;

				if (code <= end) {
					idx = code;

					p = q + num_segs2 - 2;
					delta = TT_PEEK_SHORT(p);
					p += num_segs2;
					offset = TT_PEEK_USHORT(p);

					if (offset == 0xFFFFU)
						goto Exit;

					if (offset != 0) {
						p += offset + 2 * (idx - start);
						idx = TT_PEEK_USHORT(p);
					}

					if (idx != 0)
						result = (FT_UInt)(idx + delta) & 0xFFFFU;
				}
			}
		}

#endif /* 0 */
	}

Exit:
	return result;
}

FT_CALLBACK_DEF(FT_UInt)
tt_cmap4_char_next(TT_CMap cmap, FT_UInt32 *pchar_code) {
	FT_Byte   *table = cmap->data;
	FT_UInt32 result = 0;
	FT_UInt32 char_code = *pchar_code + 1;
	FT_UInt   gindex = 0;
	FT_Byte   *p;
	FT_Byte   *q;
	FT_UInt   code, num_segs2;

	if (char_code >= 0x10000UL)
		goto Exit;

	code = (FT_UInt)char_code;
	p = table + 6;
	num_segs2 = TT_PEEK_USHORT(p) & -2; /* ensure even-ness */

	for (;;) {
		FT_UInt offset, n;
		FT_Int  delta;

		p = table + 14;             /* ends table  */
		q = table + 16 + num_segs2; /* starts table */

		for (n = 0; n < num_segs2; n += 2) {
			FT_UInt end   = TT_NEXT_USHORT(p);
			FT_UInt start = TT_NEXT_USHORT(q);

			if (code < start)
				code = start;

			if (code <= end) {
				p = q + num_segs2 - 2;
				delta = TT_PEEK_SHORT(p);
				p += num_segs2;
				offset = TT_PEEK_USHORT(p);

				if (offset != 0 && offset != 0xFFFFU) {
					/* parse the glyph ids array for non-0 index */
					p += offset + (code - start) * 2;
					while (code <= end) {
						gindex = TT_NEXT_USHORT(p);
						if (gindex != 0) {
							gindex = (FT_UInt)(gindex + delta) & 0xFFFFU;
							if (gindex != 0)
								break;
						}
						code++;
					}
				} else if (offset == 0xFFFFU) {
					/* an offset of 0xFFFF means an empty glyph in certain fonts! */
					code = end;
					break;
				} else
					gindex = (FT_UInt)(code + delta) & 0xFFFFU;

				if (gindex == 0)
					break;

				result = code;
				goto Exit;
			}
		}

		/* loop to next trial charcode */
		if (code >= 0xFFFFU)
			break;

		code++;
	}
	return (FT_UInt)result;

Exit:
	*pchar_code = result;
	return gindex;
}

FT_CALLBACK_TABLE_DEF
const TT_CMap_ClassRec tt_cmap4_class_rec = {
	{
		sizeof (TT_CMapRec),

		(FT_CMap_InitFunc)      tt_cmap_init,
		(FT_CMap_DoneFunc)      NULL,
		(FT_CMap_CharIndexFunc) tt_cmap4_char_index,
		(FT_CMap_CharNextFunc)  tt_cmap4_char_next
	},
	4,
	(TT_CMap_ValidateFunc)  tt_cmap4_validate
};

#endif /* TT_CONFIG_CMAP_FORMAT_4 */


/**** FORMAT 6 ****/

#ifdef TT_CONFIG_CMAP_FORMAT_6

FT_CALLBACK_DEF(FT_Error)
tt_cmap6_validate(FT_Byte *table, FT_Validator valid) {
	FT_Byte *p;
	FT_UInt length, /*start,*/ count;

	if (table + 10 > valid->limit)
		FT2_1_3_INVALID_TOO_SHORT;

	p = table + 2;
	length = TT_NEXT_USHORT(p);

	p = table + 6; /* skip language */
	// start  = TT_NEXT_USHORT( p );
	count = TT_NEXT_USHORT(p);

	if (table + length > valid->limit || length < 10 + count * 2)
		FT2_1_3_INVALID_TOO_SHORT;

	/* check glyph indices */
	if (valid->level >= FT2_1_3_VALIDATE_TIGHT) {
		FT_UInt gindex;

		for (; count > 0; count--) {
			gindex = TT_NEXT_USHORT(p);
			if (gindex >= TT_VALID_GLYPH_COUNT(valid))
				FT2_1_3_INVALID_GLYPH_ID;
		}
	}
	return 0;
}

FT_CALLBACK_DEF(FT_UInt)
tt_cmap6_char_index(TT_CMap cmap, FT_UInt32 char_code) {
	FT_Byte *table = cmap->data;
	FT_UInt result = 0;
	FT_Byte *p 	   = table + 6;
	FT_UInt start  = TT_NEXT_USHORT(p);
	FT_UInt count  = TT_NEXT_USHORT(p);
	FT_UInt idx    = (FT_UInt)(char_code - start);

	if (idx < count) {
		p += 2 * idx;
		result = TT_PEEK_USHORT(p);
	}
	return result;
}

FT_CALLBACK_DEF(FT_UInt)
tt_cmap6_char_next(TT_CMap cmap, FT_UInt32 *pchar_code) {
	FT_Byte   *table = cmap->data;
	FT_UInt32 result = 0;
	FT_UInt32 char_code = *pchar_code + 1;
	FT_UInt   gindex = 0;

	FT_Byte *p = table + 6;
	FT_UInt start = TT_NEXT_USHORT(p);
	FT_UInt count = TT_NEXT_USHORT(p);
	FT_UInt idx;

	if (char_code >= 0x10000UL)
		goto Exit;

	if (char_code < start)
		char_code = start;

	idx = (FT_UInt)(char_code - start);
	p += 2 * idx;

	for (; idx < count; idx++) {
		gindex = TT_NEXT_USHORT(p);
		if (gindex != 0) {
			result = char_code;
			break;
		}
		char_code++;
	}

Exit:
	*pchar_code = result;
	return gindex;
}

FT_CALLBACK_TABLE_DEF
const TT_CMap_ClassRec tt_cmap6_class_rec = {
	{
		sizeof (TT_CMapRec),

		(FT_CMap_InitFunc)      tt_cmap_init,
		(FT_CMap_DoneFunc)      NULL,
		(FT_CMap_CharIndexFunc) tt_cmap6_char_index,
		(FT_CMap_CharNextFunc)  tt_cmap6_char_next
	},
	6,
	(TT_CMap_ValidateFunc) tt_cmap6_validate
};

#endif /* TT_CONFIG_CMAP_FORMAT_6 */


#ifdef TT_CONFIG_CMAP_FORMAT_8

FT_CALLBACK_DEF(FT_Error)
tt_cmap8_validate(FT_Byte *table, FT_Validator valid) {
	FT_Byte   *p = table + 4;
	FT_Byte   *is32;
	FT_UInt32 length;
	FT_UInt32 num_groups;

	if (table + 16 + 8192 > valid->limit)
		FT2_1_3_INVALID_TOO_SHORT;

	length = TT_NEXT_ULONG(p);
	if (table + length > valid->limit || length < 8208)
		FT2_1_3_INVALID_TOO_SHORT;

	is32 = table + 12;
	p = is32 + 8192; /* skip `is32' array */
	num_groups = TT_NEXT_ULONG(p);

	if (p + num_groups * 12 > valid->limit)
		FT2_1_3_INVALID_TOO_SHORT;

	/* check groups, they must be in increasing order */
	{
		FT_UInt32 n, start, end, start_id, count, last = 0;

		for (n = 0; n < num_groups; n++) {
			FT_UInt hi, lo;

			start 	 = TT_NEXT_ULONG(p);
			end 	 = TT_NEXT_ULONG(p);
			start_id = TT_NEXT_ULONG(p);

			if (start > end)
				FT2_1_3_INVALID_DATA;

			if (n > 0 && start <= last)
				FT2_1_3_INVALID_DATA;

			if (valid->level >= FT2_1_3_VALIDATE_TIGHT) {
				if (start_id + end - start >= TT_VALID_GLYPH_COUNT(valid))
					FT2_1_3_INVALID_GLYPH_ID;

				count = (FT_UInt32)(end - start + 1);

				if (start & ~0xFFFFU) {
					/* start_hi != 0; check that is32[i] is 1 for each i in */
					/* the `hi' and `lo' of the range [start..end]          */
					for (; count > 0; count--, start++) {
						hi = (FT_UInt)(start >> 16);
						lo = (FT_UInt)(start & 0xFFFFU);

						if ((is32[hi >> 3] & (0x80 >> (hi & 7))) == 0)
							FT2_1_3_INVALID_DATA;

						if ((is32[lo >> 3] & (0x80 >> (lo & 7))) == 0)
							FT2_1_3_INVALID_DATA;
					}
				} else {
					/* start_hi == 0; check that is32[i] is 0 for each i in */
					/* the range [start..end]                               */

					/* end_hi cannot be != 0! */
					if (end & ~0xFFFFU)
						FT2_1_3_INVALID_DATA;

					for (; count > 0; count--, start++) {
						lo = (FT_UInt)(start & 0xFFFFU);

						if ((is32[lo >> 3] & (0x80 >> (lo & 7))) != 0)
							FT2_1_3_INVALID_DATA;
					}
				}
			}

			last = end;
		}
	}
	return 0;
}

FT_CALLBACK_DEF(FT_UInt)
tt_cmap8_char_index(TT_CMap cmap, FT_UInt32 char_code) {
	FT_Byte   *table = cmap->data;
	FT_UInt   result = 0;
	FT_Byte   *p = table + 8204;
	FT_UInt32 num_groups = TT_NEXT_ULONG(p);
	FT_UInt32 start, end, start_id;

	for (; num_groups > 0; num_groups--) {
		start    = TT_NEXT_ULONG(p);
		end 	 = TT_NEXT_ULONG(p);
		start_id = TT_NEXT_ULONG(p);

		if (char_code < start)
			break;

		if (char_code <= end) {
			result = (FT_UInt)(start_id + char_code - start);
			break;
		}
	}
	return result;
}

FT_CALLBACK_DEF(FT_UInt)
tt_cmap8_char_next(TT_CMap cmap, FT_UInt32 *pchar_code) {
	FT_UInt32 result = 0;
	FT_UInt32 char_code = *pchar_code + 1;
	FT_UInt   gindex = 0;
	FT_Byte   *table = cmap->data;
	FT_Byte   *p = table + 8204;
	FT_UInt32 num_groups = TT_NEXT_ULONG(p);
	FT_UInt32 start, end, start_id;

	p = table + 8208;

	for (; num_groups > 0; num_groups--) {
		start    = TT_NEXT_ULONG(p);
		end 	 = TT_NEXT_ULONG(p);
		start_id = TT_NEXT_ULONG(p);

		if (char_code < start)
			char_code = start;

		if (char_code <= end) {
			gindex = (FT_UInt)(char_code - start + start_id);
			if (gindex != 0) {
				result = char_code;
				goto Exit;
			}
		}
	}

Exit:
	*pchar_code = result;
	return gindex;
}

FT_CALLBACK_TABLE_DEF
const TT_CMap_ClassRec tt_cmap8_class_rec = {
	{
		sizeof (TT_CMapRec),

		(FT_CMap_InitFunc)      tt_cmap_init,
		(FT_CMap_DoneFunc)      NULL,
		(FT_CMap_CharIndexFunc) tt_cmap8_char_index,
		(FT_CMap_CharNextFunc)  tt_cmap8_char_next
	},
	8,
	(TT_CMap_ValidateFunc) tt_cmap8_validate
};

#endif /* TT_CONFIG_CMAP_FORMAT_8 */


/**** FORMAT 10 ****/

#ifdef TT_CONFIG_CMAP_FORMAT_10

FT_CALLBACK_DEF(FT_Error)
tt_cmap10_validate(FT_Byte *table, FT_Validator valid) {
	FT_Byte  *p = table + 4;
	FT_ULong length, /*start,*/ count;

	if (table + 20 > valid->limit)
		FT2_1_3_INVALID_TOO_SHORT;

	length = TT_NEXT_ULONG(p);
	p = table + 12;
	// start  = TT_NEXT_ULONG( p );
	count = TT_NEXT_ULONG(p);

	if (table + length > valid->limit || length < 20 + count * 2)
		FT2_1_3_INVALID_TOO_SHORT;

	/* check glyph indices */
	if (valid->level >= FT2_1_3_VALIDATE_TIGHT) {
		FT_UInt gindex;

		for (; count > 0; count--) {
			gindex = TT_NEXT_USHORT(p);
			if (gindex >= TT_VALID_GLYPH_COUNT(valid))
				FT2_1_3_INVALID_GLYPH_ID;
		}
	}
	return 0;
}

FT_CALLBACK_DEF(FT_UInt)
tt_cmap10_char_index(TT_CMap cmap, FT_UInt32 char_code) {
	FT_Byte   *table = cmap->data;
	FT_UInt   result = 0;
	FT_Byte   *p = table + 12;
	FT_UInt32 start = TT_NEXT_ULONG(p);
	FT_UInt32 count = TT_NEXT_ULONG(p);
	FT_UInt32 idx = (FT_ULong)(char_code - start);

	if (idx < count) {
		p += 2 * idx;
		result = TT_PEEK_USHORT(p);
	}
	return result;
}

FT_CALLBACK_DEF(FT_UInt)
tt_cmap10_char_next(TT_CMap cmap, FT_UInt32 *pchar_code) {
	FT_Byte   *table = cmap->data;
	// FT_UInt32  result    = 0;
	FT_UInt32 char_code = *pchar_code + 1;
	FT_UInt   gindex = 0;
	FT_Byte   *p = table + 12;
	FT_UInt32 start = TT_NEXT_ULONG(p);
	FT_UInt32 count = TT_NEXT_ULONG(p);
	FT_UInt32 idx;

	if (char_code < start)
		char_code = start;

	idx = (FT_UInt32)(char_code - start);
	p += 2 * idx;

	for (; idx < count; idx++) {
		gindex = TT_NEXT_USHORT(p);
		if (gindex != 0) {
			// result = char_code;
			break;
		}
		char_code++;
	}

	*pchar_code = char_code;
	return gindex;
}

FT_CALLBACK_TABLE_DEF
const TT_CMap_ClassRec tt_cmap10_class_rec = {
	{
		sizeof (TT_CMapRec),

		(FT_CMap_InitFunc)      tt_cmap_init,
		(FT_CMap_DoneFunc)      NULL,
		(FT_CMap_CharIndexFunc) tt_cmap10_char_index,
		(FT_CMap_CharNextFunc)  tt_cmap10_char_next
	},
	10,
	(TT_CMap_ValidateFunc)  tt_cmap10_validate
};

#endif /* TT_CONFIG_CMAP_FORMAT_10 */


/**** FORMAT 12 ****/

#ifdef TT_CONFIG_CMAP_FORMAT_12

FT_CALLBACK_DEF(FT_Error)
tt_cmap12_validate(FT_Byte *table, FT_Validator valid) {
	FT_Byte  *p;
	FT_ULong length;
	FT_ULong num_groups;

	if (table + 16 > valid->limit)
		FT2_1_3_INVALID_TOO_SHORT;

	p = table + 4;
	length = TT_NEXT_ULONG(p);

	p = table + 12;
	num_groups = TT_NEXT_ULONG(p);

	if (table + length > valid->limit || length < 16 + 12 * num_groups)
		FT2_1_3_INVALID_TOO_SHORT;

	/* check groups, they must be in increasing order */
	{
		FT_ULong n, start, end, start_id, last = 0;

		for (n = 0; n < num_groups; n++) {
			start 	 = TT_NEXT_ULONG(p);
			end 	 = TT_NEXT_ULONG(p);
			start_id = TT_NEXT_ULONG(p);

			if (start > end)
				FT2_1_3_INVALID_DATA;

			if (n > 0 && start <= last)
				FT2_1_3_INVALID_DATA;

			if (valid->level >= FT2_1_3_VALIDATE_TIGHT) {
				if (start_id + end - start >= TT_VALID_GLYPH_COUNT(valid))
					FT2_1_3_INVALID_GLYPH_ID;
			}

			last = end;
		}
	}
	return 0;
}

FT_CALLBACK_DEF(FT_UInt)
tt_cmap12_char_index(TT_CMap cmap, FT_UInt32 char_code) {
	FT_UInt   result = 0;
	FT_Byte   *table = cmap->data;
	FT_Byte   *p = table + 12;
	FT_UInt32 num_groups = TT_NEXT_ULONG(p);
	FT_UInt32 start, end, start_id;

	for (; num_groups > 0; num_groups--) {
		start 	 = TT_NEXT_ULONG(p);
		end 	 = TT_NEXT_ULONG(p);
		start_id = TT_NEXT_ULONG(p);

		if (char_code < start)
			break;

		if (char_code <= end) {
			result = (FT_UInt)(start_id + char_code - start);
			break;
		}
	}
	return result;
}

FT_CALLBACK_DEF(FT_UInt)
tt_cmap12_char_next(TT_CMap cmap, FT_UInt32 *pchar_code) {
	FT_Byte    *table = cmap->data;
	FT_UInt32  result = 0;
	FT_UInt32  char_code = *pchar_code + 1;
	FT_UInt    gindex = 0;
	FT_Byte    *p = table + 12;
	FT_UInt32  num_groups = TT_NEXT_ULONG(p);
	FT_UInt32  start, end, start_id;

	p = table + 16;

	for (; num_groups > 0; num_groups--) {
		start 	 = TT_NEXT_ULONG(p);
		end 	 = TT_NEXT_ULONG(p);
		start_id = TT_NEXT_ULONG(p);

		if (char_code < start)
			char_code = start;

		if (char_code <= end) {
			gindex = (FT_UInt)(char_code - start + start_id);
			if (gindex != 0) {
				result = char_code;
				goto Exit;
			}
		}
	}

Exit:
	*pchar_code = result;
	return gindex;
}

FT_CALLBACK_TABLE_DEF
const TT_CMap_ClassRec tt_cmap12_class_rec = {
	{
		sizeof (TT_CMapRec),

		(FT_CMap_InitFunc)      tt_cmap_init,
		(FT_CMap_DoneFunc)      NULL,
		(FT_CMap_CharIndexFunc) tt_cmap12_char_index,
		(FT_CMap_CharNextFunc)  tt_cmap12_char_next
	},
	12,
	(TT_CMap_ValidateFunc) tt_cmap12_validate
};


#endif /* TT_CONFIG_CMAP_FORMAT_12 */


static const TT_CMap_Class tt_cmap_classes[] = {
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
FT_LOCAL_DEF(FT_Error)
tt_face_build_cmaps(TT_Face face) {
	FT_Byte *table = face->cmap_table;
	FT_Byte *limit = table + face->cmap_size;
	FT_UInt volatile num_cmaps;
	FT_Byte *volatile p = table;

	if (p + 4 > limit)
		return FT2_1_3_Err_Invalid_Table;

	/* only recognize format 0 */
	if (TT_NEXT_USHORT(p) != 0) {
		p -= 2;
		FT_ERROR(("tt_face_build_cmaps: unsupported `cmap' table format = %d\n", TT_PEEK_USHORT(p)));
		return FT2_1_3_Err_Invalid_Table;
	}

	num_cmaps = TT_NEXT_USHORT(p);

	for (; num_cmaps > 0 && p + 8 <= limit; num_cmaps--) {
		FT_CharMapRec charmap;
		FT_UInt32 offset;

		charmap.platform_id = TT_NEXT_USHORT(p);
		charmap.encoding_id = TT_NEXT_USHORT(p);
		charmap.face = FT2_1_3_FACE(face);
		charmap.encoding = FT_ENCODING_NONE; /* will be filled later */
		offset = TT_NEXT_ULONG(p);

		if (offset && table + offset + 2 < limit) {
			FT_Byte *cmap  = table + offset;
			FT_UInt format = TT_PEEK_USHORT(cmap);
			const TT_CMap_Class *volatile pclazz = tt_cmap_classes;
			TT_CMap_Class clazz;

			for (; *pclazz; pclazz++) {
				clazz = *pclazz;
				if (clazz->format == format) {
					volatile TT_ValidatorRec valid;

					ft_validator_init(FT2_1_3_VALIDATOR(&valid), cmap, limit, FT2_1_3_VALIDATE_DEFAULT);

					valid.num_glyphs = (FT_UInt)face->root.num_glyphs;

					if (ft_setjmp(FT2_1_3_VALIDATOR(&valid)->jump_buffer) == 0) {
						/* validate this cmap sub-table */
						clazz->validate(cmap, FT2_1_3_VALIDATOR(&valid));
					}

					if (valid.validator.error == 0)
						(void)FT_CMap_New((FT_CMap_Class)clazz, cmap, &charmap, NULL);
					else {
						FT_ERROR(("tt_face_build_cmaps:"));
						FT_ERROR((" broken cmap sub-table ignored!\n"));
					}
				}
			}
		}
	}

	return 0;
}

} // End of namespace FreeType213
} // End of namespace AGS3
