/***************************************************************************/
/*                                                                         */
/*  pfrsbit.c                                                              */
/*                                                                         */
/*    FreeType PFR bitmap loader (body).                                   */
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


#include "pfrsbit.h"
#include "pfrload.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftdebug.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftstream.h"

#include "pfrerror.h"

#undef  FT2_1_3_COMPONENT
#define FT2_1_3_COMPONENT  trace_pfr

namespace AGS3 {
namespace FreeType213 {

/*************************************************************************/
/*************************************************************************/
/*****                                                               *****/
/*****                      PFR BIT WRITER                           *****/
/*****                                                               *****/
/*************************************************************************/
/*************************************************************************/

typedef struct  PFR_BitWriter_ {
	FT2_1_3_Byte*  line;      /* current line start                    */
	FT2_1_3_Int    pitch;     /* line size in bytes                    */
	FT2_1_3_Int    width;     /* width in pixels/bits                  */
	FT2_1_3_Int    rows;      /* number of remaining rows to scan      */
	FT2_1_3_Int    total;     /* total number of bits to draw          */

} PFR_BitWriterRec, *PFR_BitWriter;


static void
pfr_bitwriter_init( PFR_BitWriter  writer,
					FT2_1_3_Bitmap*     target,
					FT2_1_3_Bool        decreasing ) {
	writer->line   = target->buffer;
	writer->pitch  = target->pitch;
	writer->width  = target->width;
	writer->rows   = target->rows;
	writer->total  = writer->width * writer->rows;

	if ( !decreasing ) {
		writer->line += writer->pitch * ( target->rows-1 );
		writer->pitch = -writer->pitch;
	}
}


static void
pfr_bitwriter_decode_bytes( PFR_BitWriter  writer,
							FT2_1_3_Byte*       p,
							FT2_1_3_Byte*       limit ) {
	FT2_1_3_Int    n, reload;
	FT2_1_3_Int    left = writer->width;
	FT2_1_3_Byte*  cur  = writer->line;
	FT2_1_3_UInt   mask = 0x80;
	FT2_1_3_UInt   val  = 0;
	FT2_1_3_UInt   c    = 0;


	n = (FT2_1_3_Int)( limit - p ) * 8;
	if ( n > writer->total )
		n = writer->total;

	reload = n & 7;

	for ( ; n > 0; n-- ) {
		if ( ( n & 7 ) == reload )
			val = *p++;

		if ( val & 0x80 )
			c |= mask;

		val  <<= 1;
		mask >>= 1;

		if ( --left <= 0 ) {
			cur[0] = (FT2_1_3_Byte)c;
			left   = writer->width;
			mask   = 0x80;

			writer->line += writer->pitch;
			cur           = writer->line;
			c             = 0;
		} else if ( mask == 0 ) {
			cur[0] = c;
			mask   = 0x80;
			c      = 0;
			cur ++;
		}
	}

	if ( mask != 0x80 )
		cur[0] = c;
}


static void
pfr_bitwriter_decode_rle1( PFR_BitWriter  writer,
						   FT2_1_3_Byte*       p,
						   FT2_1_3_Byte*       limit ) {
	FT2_1_3_Int    n, phase, count, counts[2], reload;
	FT2_1_3_Int    left = writer->width;
	FT2_1_3_Byte*  cur  = writer->line;
	FT2_1_3_UInt   mask = 0x80;
	FT2_1_3_UInt   c    = 0;


	n = writer->total;

	phase     = 1;
	counts[0] = 0;
	counts[1] = 0;
	count     = 0;
	reload    = 1;

	for ( ; n > 0; n-- ) {
		if ( reload ) {
			do {
				if ( phase ) {
					FT2_1_3_Int  v;


					if ( p >= limit )
						break;

					v         = *p++;
					counts[0] = v >> 4;
					counts[1] = v & 15;
					phase     = 0;
					count     = counts[0];
				} else {
					phase = 1;
					count = counts[1];
				}

			} while ( count == 0 );
		}

		if ( phase )
			c |= mask;

		mask >>= 1;

		if ( --left <= 0 ) {
			cur[0] = (FT2_1_3_Byte) c;
			left   = writer->width;
			mask   = 0x80;

			writer->line += writer->pitch;
			cur           = writer->line;
			c             = 0;
		} else if ( mask == 0 ) {
			cur[0] = c;
			mask   = 0x80;
			c      = 0;
			cur ++;
		}

		reload = ( --count <= 0 );
	}

	if ( mask != 0x80 )
		cur[0] = (FT2_1_3_Byte) c;
}


static void
pfr_bitwriter_decode_rle2( PFR_BitWriter  writer,
						   FT2_1_3_Byte*       p,
						   FT2_1_3_Byte*       limit ) {
	FT2_1_3_Int    n, phase, count, reload;
	FT2_1_3_Int    left = writer->width;
	FT2_1_3_Byte*  cur  = writer->line;
	FT2_1_3_UInt   mask = 0x80;
	FT2_1_3_UInt   c    = 0;


	n = writer->total;

	phase  = 1;
	count  = 0;
	reload = 1;

	for ( ; n > 0; n-- ) {
		if ( reload ) {
			do {
				if ( p >= limit )
					break;

				count = *p++;
				phase = phase ^ 1;

			} while ( count == 0 );
		}

		if ( phase )
			c |= mask;

		mask >>= 1;

		if ( --left <= 0 ) {
			cur[0] = (FT2_1_3_Byte) c;
			c      = 0;
			mask   = 0x80;
			left   = writer->width;

			writer->line += writer->pitch;
			cur           = writer->line;
		} else if ( mask == 0 ) {
			cur[0] = c;
			c      = 0;
			mask   = 0x80;
			cur ++;
		}

		reload = ( --count <= 0 );
	}

	if ( mask != 0x80 )
		cur[0] = (FT2_1_3_Byte) c;
}


/*************************************************************************/
/*************************************************************************/
/*****                                                               *****/
/*****                  BITMAP DATA DECODING                         *****/
/*****                                                               *****/
/*************************************************************************/
/*************************************************************************/

static void
pfr_lookup_bitmap_data( FT2_1_3_Byte*   base,
						FT2_1_3_Byte*   limit,
						FT2_1_3_Int     count,
						FT2_1_3_Byte    flags,
						FT2_1_3_UInt    char_code,
						FT2_1_3_ULong*  found_offset,
						FT2_1_3_ULong*  found_size ) {
	FT2_1_3_UInt   left, right, char_len;
	FT2_1_3_Bool   two = flags & 1;
	FT2_1_3_Byte*  buff;


	char_len = 4;
	if ( two )       char_len += 1;
	if ( flags & 2 ) char_len += 1;
	if ( flags & 4 ) char_len += 1;

	left  = 0;
	right = count;

	while ( left < right ) {
		FT2_1_3_UInt  middle, code;


		middle = ( left + right ) >> 1;
		buff   = base + middle * char_len;

		/* check that we are not outside of the table -- */
		/* this is possible with broken fonts...         */
		if ( buff + char_len > limit )
			goto Fail;

		if ( two )
			code = PFR_NEXT_USHORT( buff );
		else
			code = PFR_NEXT_BYTE( buff );

		if ( code == char_code )
			goto Found_It;

		if ( code < char_code )
			left = middle;
		else
			right = middle;
	}

Fail:
	/* Not found */
	*found_size   = 0;
	*found_offset = 0;
	return;

Found_It:
	if ( flags & 2 )
		*found_size = PFR_NEXT_USHORT( buff );
	else
		*found_size = PFR_NEXT_BYTE( buff );

	if ( flags & 4 )
		*found_offset = PFR_NEXT_ULONG( buff );
	else
		*found_offset = PFR_NEXT_USHORT( buff );
}


/* load bitmap metrics.  "*padvance" must be set to the default value */
/* before calling this function...                                    */
/*                                                                    */
static FT2_1_3_Error
pfr_load_bitmap_metrics( FT2_1_3_Byte**  pdata,
						 FT2_1_3_Byte*   limit,
						 FT2_1_3_Long    scaled_advance,
						 FT2_1_3_Long   *axpos,
						 FT2_1_3_Long   *aypos,
						 FT2_1_3_UInt   *axsize,
						 FT2_1_3_UInt   *aysize,
						 FT2_1_3_Long   *aadvance,
						 FT2_1_3_UInt   *aformat ) {
	FT2_1_3_Error  error = 0;
	FT2_1_3_Byte   flags;
	FT2_1_3_Char   b;
	FT2_1_3_Byte*  p = *pdata;
	FT2_1_3_Long   xpos, ypos, advance;
	FT2_1_3_UInt   xsize, ysize;


	PFR_CHECK( 1 );
	flags = PFR_NEXT_BYTE( p );

	xpos    = 0;
	ypos    = 0;
	xsize   = 0;
	ysize   = 0;
	advance = 0;

	switch ( flags & 3 ) {
	case 0:
		PFR_CHECK( 1 );
		b    = PFR_NEXT_INT8( p );
		xpos = b >> 4;
		ypos = ( (FT2_1_3_Char)( b << 4 ) ) >> 4;
		break;

	case 1:
		PFR_CHECK( 2 );
		xpos = PFR_NEXT_INT8( p );
		ypos = PFR_NEXT_INT8( p );
		break;

	case 2:
		PFR_CHECK( 4 );
		xpos = PFR_NEXT_SHORT( p );
		ypos = PFR_NEXT_SHORT( p );
		break;

	case 3:
		PFR_CHECK( 6 );
		xpos = PFR_NEXT_LONG( p );
		ypos = PFR_NEXT_LONG( p );
		break;

	default:
		;
	}

	flags >>= 2;
	switch ( flags & 3 ) {
	case 0:
		/* blank image */
		xsize = 0;
		ysize = 0;
		break;

	case 1:
		PFR_CHECK( 1 );
		b     = PFR_NEXT_BYTE( p );
		xsize = ( b >> 4 ) & 0xF;
		ysize = b & 0xF;
		break;

	case 2:
		PFR_CHECK( 2 );
		xsize = PFR_NEXT_BYTE( p );
		ysize = PFR_NEXT_BYTE( p );
		break;

	case 3:
		PFR_CHECK( 4 );
		xsize = PFR_NEXT_USHORT( p );
		ysize = PFR_NEXT_USHORT( p );
		break;

	default:
		;
	}

	flags >>= 2;
	switch ( flags & 3 ) {
	case 0:
		advance = scaled_advance;
		break;

	case 1:
		PFR_CHECK( 1 );
		advance = PFR_NEXT_INT8( p ) << 8;
		break;

	case 2:
		PFR_CHECK( 2 );
		advance = PFR_NEXT_SHORT( p );
		break;

	case 3:
		PFR_CHECK( 3 );
		advance = PFR_NEXT_LONG( p );
		break;

	default:
		;
	}

	*axpos    = xpos;
	*aypos    = ypos;
	*axsize   = xsize;
	*aysize   = ysize;
	*aadvance = advance;
	*aformat  = flags >> 2;
	*pdata    = p;

Exit:
	return error;

Too_Short:
	error = PFR_Err_Invalid_Table;
	FT2_1_3_ERROR(( "pfr_load_bitmap_metrics: invalid glyph data\n" ));
	goto Exit;
}


static FT2_1_3_Error
pfr_load_bitmap_bits( FT2_1_3_Byte*    p,
					  FT2_1_3_Byte*    limit,
					  FT2_1_3_UInt     format,
					  FT2_1_3_UInt     decreasing,
					  FT2_1_3_Bitmap*  target ) {
	FT2_1_3_Error          error = 0;
	PFR_BitWriterRec  writer;


	if ( target->rows > 0 && target->width > 0 ) {
		pfr_bitwriter_init( &writer, target, decreasing );

		switch ( format ) {
		case 0: /* packed bits */
			pfr_bitwriter_decode_bytes( &writer, p, limit );
			break;

		case 1: /* RLE1 */
			pfr_bitwriter_decode_rle1( &writer, p, limit );
			break;

		case 2: /* RLE2 */
			pfr_bitwriter_decode_rle2( &writer, p, limit );
			break;

		default:
			FT2_1_3_ERROR(( "pfr_read_bitmap_data: invalid image type\n" ));
			error = FT2_1_3_Err_Invalid_File_Format;
		}
	}

	return error;
}


/*************************************************************************/
/*************************************************************************/
/*****                                                               *****/
/*****                     BITMAP LOADING                            *****/
/*****                                                               *****/
/*************************************************************************/
/*************************************************************************/

FT2_1_3_LOCAL( FT2_1_3_Error )
pfr_slot_load_bitmap( PFR_Slot  glyph,
					  PFR_Size  size,
					  FT2_1_3_UInt   glyph_index ) {
	FT2_1_3_Error     error;
	PFR_Face     face   = (PFR_Face) glyph->root.face;
	FT2_1_3_Stream    stream = face->root.stream;
	PFR_PhyFont  phys   = &face->phy_font;
	FT2_1_3_ULong     gps_offset;
	FT2_1_3_ULong     gps_size;
	PFR_Char     character;
	PFR_Strike   strike;


	character = &phys->chars[glyph_index];

	/* Look-up a bitmap strike corresponding to the current */
	/* character dimensions                                 */
	{
		FT2_1_3_UInt  n;


		strike = phys->strikes;
		for ( n = 0; n < phys->num_strikes; n++ ) {
			if ( strike->x_ppm == (FT2_1_3_UInt)size->root.metrics.x_ppem &&
					strike->y_ppm == (FT2_1_3_UInt)size->root.metrics.y_ppem ) {
				goto Found_Strike;
			}

			strike++;
		}

		/* couldn't find it */
		return FT2_1_3_Err_Invalid_Argument;
	}

Found_Strike:

	/* Now lookup the glyph's position within the file */
	{
		FT2_1_3_UInt  char_len;


		char_len = 4;
		if ( strike->flags & 1 ) char_len += 1;
		if ( strike->flags & 2 ) char_len += 1;
		if ( strike->flags & 4 ) char_len += 1;

		/* Access data directly in the frame to speed lookups */
		if ( FT2_1_3_STREAM_SEEK( phys->bct_offset + strike->bct_offset ) ||
				FT2_1_3_FRAME_ENTER( char_len * strike->num_bitmaps )        )
			goto Exit;

		pfr_lookup_bitmap_data( stream->cursor,
								stream->limit,
								strike->num_bitmaps,
								strike->flags,
								character->char_code,
								&gps_offset,
								&gps_size );

		FT2_1_3_FRAME_EXIT();

		if ( gps_size == 0 ) {
			/* Could not find a bitmap program string for this glyph */
			error = FT2_1_3_Err_Invalid_Argument;
			goto Exit;
		}
	}

	/* get the bitmap metrics */
	{
		FT2_1_3_Long   xpos, ypos, advance;
		FT2_1_3_UInt   xsize, ysize, format;
		FT2_1_3_Byte*  p;


		advance = FT2_1_3_MulDiv( size->root.metrics.x_ppem << 8,
							 character->advance,
							 phys->metrics_resolution );

		/* XXX: handle linearHoriAdvance correctly! */

		if ( FT2_1_3_STREAM_SEEK( face->header.gps_section_offset + gps_offset ) ||
				FT2_1_3_FRAME_ENTER( gps_size )                                     )
			goto Exit;

		p     = stream->cursor;
		error = pfr_load_bitmap_metrics( &p, stream->limit,
										 advance,
										 &xpos, &ypos,
										 &xsize, &ysize,
										 &advance, &format );
		if ( !error ) {
			glyph->root.format = FT2_1_3_GLYPH_FORMAT_BITMAP;

			/* Set up glyph bitmap and metrics */
			glyph->root.bitmap.width      = (FT2_1_3_Int)xsize;
			glyph->root.bitmap.rows       = (FT2_1_3_Int)ysize;
			glyph->root.bitmap.pitch      = (FT2_1_3_Long)( xsize + 7 ) >> 3;
			glyph->root.bitmap.pixel_mode = FT2_1_3_PIXEL_MODE_MONO;

			glyph->root.metrics.width        = (FT2_1_3_Long)xsize << 6;
			glyph->root.metrics.height       = (FT2_1_3_Long)ysize << 6;
			glyph->root.metrics.horiBearingX = xpos << 6;
			glyph->root.metrics.horiBearingY = ypos << 6;
			glyph->root.metrics.horiAdvance  = ( ( advance >> 2 ) + 32 ) & -64;
			glyph->root.metrics.vertBearingX = - glyph->root.metrics.width >> 1;
			glyph->root.metrics.vertBearingY = 0;
			glyph->root.metrics.vertAdvance  = size->root.metrics.height;

			glyph->root.bitmap_left = xpos;
			glyph->root.bitmap_top  = ypos + ysize;

			/* Allocate and read bitmap data */
			{
				FT2_1_3_Memory  memory = face->root.memory;
				FT2_1_3_Long    len    = glyph->root.bitmap.pitch * ysize;


				if ( !FT2_1_3_ALLOC( glyph->root.bitmap.buffer, len ) ) {
					error = pfr_load_bitmap_bits( p,
												  stream->limit,
												  format,
												  face->header.color_flags & 2,
												  &glyph->root.bitmap );
				}
			}
		}

		FT2_1_3_FRAME_EXIT();
	}

Exit:
	return error;
}

} // End of namespace FreeType213
} // End of namespace AGS3

/* END */
