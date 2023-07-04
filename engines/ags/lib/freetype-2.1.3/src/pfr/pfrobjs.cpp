/***************************************************************************/
/*                                                                         */
/*  pfrobjs.c                                                              */
/*                                                                         */
/*    FreeType PFR object methods (body).                                  */
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


#include "pfrobjs.h"
#include "pfrload.h"
#include "pfrgload.h"
#include "pfrcmap.h"
#include "pfrsbit.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/ftoutln.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftdebug.h"

#include "pfrerror.h"

#undef  FT2_1_3_COMPONENT
#define FT2_1_3_COMPONENT  trace_pfr

namespace AGS3 {
namespace FreeType213 {

/*************************************************************************/
/*************************************************************************/
/*****                                                               *****/
/*****                     FACE OBJECT METHODS                       *****/
/*****                                                               *****/
/*************************************************************************/
/*************************************************************************/

FT2_1_3_LOCAL_DEF( void )
pfr_face_done( PFR_Face  face ) {
	/* finalize the physical font record */
	pfr_phy_font_done( &face->phy_font, FT2_1_3_FACE_MEMORY( face ) );

	/* no need to finalize the logical font or the header */
}


FT2_1_3_LOCAL_DEF( FT2_1_3_Error )
pfr_face_init( FT2_1_3_Stream      stream,
			   PFR_Face       face,
			   FT2_1_3_Int         face_index,
			   FT2_1_3_Int         num_params,
			   FT2_1_3_Parameter*  params ) {
	FT2_1_3_Error  error;

	FT2_1_3_UNUSED( num_params );
	FT2_1_3_UNUSED( params );


	/* load the header and check it */
	error = pfr_header_load( &face->header, stream );
	if ( error )
		goto Exit;

	if ( !pfr_header_check( &face->header ) ) {
		FT2_1_3_TRACE4(( "pfr_face_init: not a valid PFR font\n" ));
		error = PFR_Err_Unknown_File_Format;
		goto Exit;
	}

	/* check face index */
	{
		FT2_1_3_UInt  num_faces;


		error = pfr_log_font_count( stream,
									face->header.log_dir_offset,
									&num_faces );
		if ( error )
			goto Exit;

		face->root.num_faces = num_faces;
	}

	if ( face_index < 0 )
		goto Exit;

	if ( face_index >= face->root.num_faces ) {
		FT2_1_3_ERROR(( "pfr_face_init: invalid face index\n" ));
		error = PFR_Err_Invalid_Argument;
		goto Exit;
	}

	/* load the face */
	error = pfr_log_font_load(
				&face->log_font, stream, face_index,
				face->header.log_dir_offset,
				FT2_1_3_BOOL( face->header.phy_font_max_size_high != 0 ) );
	if ( error )
		goto Exit;

	/* now load the physical font descriptor */
	error = pfr_phy_font_load( &face->phy_font, stream,
							   face->log_font.phys_offset,
							   face->log_font.phys_size );
	if ( error )
		goto Exit;

	/* now, set-up all root face fields */
	{
		FT2_1_3_Face      root     = FT2_1_3_FACE( face );
		PFR_PhyFont  phy_font = &face->phy_font;


		root->face_index = face_index;
		root->num_glyphs = phy_font->num_chars;
		root->face_flags = FT2_1_3_FACE_FLAG_SCALABLE;

		if ( (phy_font->flags & PFR_PHY_PROPORTIONAL) == 0 )
			root->face_flags |= FT2_1_3_FACE_FLAG_FIXED_WIDTH;

		if ( phy_font->flags & PFR_PHY_VERTICAL )
			root->face_flags |= FT2_1_3_FACE_FLAG_VERTICAL;
		else
			root->face_flags |= FT2_1_3_FACE_FLAG_HORIZONTAL;

		if ( phy_font->num_strikes > 0 )
			root->face_flags |= FT2_1_3_FACE_FLAG_FIXED_SIZES;

		if ( phy_font->num_kern_pairs > 0 )
			root->face_flags |= FT2_1_3_FACE_FLAG_KERNING;

		root->family_name = phy_font->font_id;
		root->style_name  = NULL;  /* no style name in font file */

		root->num_fixed_sizes = 0;
		root->available_sizes = 0;

		root->bbox         = phy_font->bbox;
		root->units_per_EM = (FT2_1_3_UShort)phy_font->outline_resolution;
		root->ascender     = (FT2_1_3_Short) phy_font->bbox.yMax;
		root->descender    = (FT2_1_3_Short) phy_font->bbox.yMin;
		root->height       = (FT2_1_3_Short)
							 ( ( ( root->ascender - root->descender ) * 12 )
							   / 10 );

		/* now compute maximum advance width */
		if ( ( phy_font->flags & PFR_PHY_PROPORTIONAL ) == 0 )
			root->max_advance_width = (FT2_1_3_Short)phy_font->standard_advance;
		else {
			FT2_1_3_Int    max = 0;
			FT2_1_3_UInt   count = phy_font->num_chars;
			PFR_Char  gchar = phy_font->chars;


			for ( ; count > 0; count--, gchar++ ) {
				if ( max < gchar->advance )
					max = gchar->advance;
			}

			root->max_advance_width = (FT2_1_3_Short)max;
		}

		root->max_advance_height = root->height;

		root->underline_position  = (FT2_1_3_Short)( - root->units_per_EM / 10 );
		root->underline_thickness = (FT2_1_3_Short)(   root->units_per_EM / 30 );

		/* create charmap */
		{
			FT2_1_3_CharMapRec  charmap;


			charmap.face        = root;
			charmap.platform_id = 3;
			charmap.encoding_id = 1;
			charmap.encoding    = FT2_1_3_ENCODING_UNICODE;

			FT2_1_3_CMap_New( &pfr_cmap_class_rec, NULL, &charmap, NULL );

#if 0
			/* Select default charmap */
			if (root->num_charmaps)
				root->charmap = root->charmaps[0];
#endif
		}

		/* check whether we've loaded any kerning pairs */
		if ( phy_font->num_kern_pairs )
			root->face_flags |= FT2_1_3_FACE_FLAG_KERNING;
	}

Exit:
	return error;
}


/*************************************************************************/
/*************************************************************************/
/*****                                                               *****/
/*****                    SLOT OBJECT METHOD                         *****/
/*****                                                               *****/
/*************************************************************************/
/*************************************************************************/

FT2_1_3_LOCAL_DEF( FT2_1_3_Error )
pfr_slot_init( PFR_Slot  slot ) {
	FT2_1_3_GlyphLoader  loader = slot->root.internal->loader;

	pfr_glyph_init( &slot->glyph, loader );

	return 0;
}


FT2_1_3_LOCAL_DEF( void )
pfr_slot_done( PFR_Slot  slot ) {
	pfr_glyph_done( &slot->glyph );
}


FT2_1_3_LOCAL_DEF( FT2_1_3_Error )
pfr_slot_load( PFR_Slot  slot,
			   PFR_Size  size,
			   FT2_1_3_UInt   gindex,
			   FT2_1_3_Int32  load_flags ) {
	FT2_1_3_Error     error;
	PFR_Face     face    = (PFR_Face)slot->root.face;
	PFR_Char     gchar;
	FT2_1_3_Outline*  outline = &slot->root.outline;
	FT2_1_3_ULong     gps_offset;

	if (gindex > 0)
		gindex--;

	/* check that the glyph index is correct */
	FT2_1_3_ASSERT( gindex < face->phy_font.num_chars );

	/* try to load an embedded bitmap */
	if ( ( load_flags & ( FT2_1_3_LOAD_NO_SCALE | FT2_1_3_LOAD_NO_BITMAP ) ) == 0 ) {
		error = pfr_slot_load_bitmap( slot, size, gindex );
		if ( error == 0 )
			goto Exit;
	}

	gchar               = face->phy_font.chars + gindex;
	slot->root.format   = FT2_1_3_GLYPH_FORMAT_OUTLINE;
	outline->n_points   = 0;
	outline->n_contours = 0;
	gps_offset          = face->header.gps_section_offset;

	/* load the glyph outline (FT2_1_3_LOAD_NO_RECURSE isn't supported) */
	error = pfr_glyph_load( &slot->glyph, face->root.stream,
							gps_offset, gchar->gps_offset, gchar->gps_size );

	if ( !error ) {
		FT2_1_3_BBox            cbox;
		FT2_1_3_Glyph_Metrics*  metrics = &slot->root.metrics;
		FT2_1_3_Pos             advance;
		FT2_1_3_Int             em_metrics, em_outline;
		FT2_1_3_Bool            scaling;


		scaling = FT2_1_3_BOOL( ( load_flags & FT2_1_3_LOAD_NO_SCALE ) == 0 );

		/* copy outline data */
		*outline = slot->glyph.loader->base.outline;

		outline->flags &= ~FT2_1_3_OUTLINE_OWNER;
		outline->flags |= FT2_1_3_OUTLINE_REVERSE_FILL;

		if ( size && size->root.metrics.y_ppem < 24 )
			outline->flags |= FT2_1_3_OUTLINE_HIGH_PRECISION;

		/* compute the advance vector */
		metrics->horiAdvance = 0;
		metrics->vertAdvance = 0;

		advance    = gchar->advance;
		em_metrics = face->phy_font.metrics_resolution;
		em_outline = face->phy_font.outline_resolution;

		if ( em_metrics != em_outline )
			advance = FT2_1_3_MulDiv( advance, em_outline, em_metrics );

		if ( face->phy_font.flags & PFR_PHY_VERTICAL )
			metrics->vertAdvance = advance;
		else
			metrics->horiAdvance = advance;

		slot->root.linearHoriAdvance = metrics->horiAdvance;
		slot->root.linearVertAdvance = metrics->vertAdvance;

		/* make-up vertical metrics(?) */
		metrics->vertBearingX = 0;
		metrics->vertBearingY = 0;

		/* scale when needed */
		if ( scaling ) {
			FT2_1_3_Int      n;
			FT2_1_3_Fixed    x_scale = size->root.metrics.x_scale;
			FT2_1_3_Fixed    y_scale = size->root.metrics.y_scale;
			FT2_1_3_Vector*  vec     = outline->points;


			/* scale outline points */
			for ( n = 0; n < outline->n_points; n++, vec++ ) {
				vec->x = FT2_1_3_MulFix( vec->x, x_scale );
				vec->y = FT2_1_3_MulFix( vec->y, y_scale );
			}

			/* scale the advance */
			metrics->horiAdvance = FT2_1_3_MulFix( metrics->horiAdvance, x_scale );
			metrics->vertAdvance = FT2_1_3_MulFix( metrics->vertAdvance, y_scale );
		}

		/* compute the rest of the metrics */
		FT2_1_3_Outline_Get_CBox( outline, &cbox );

		metrics->width        = cbox.xMax - cbox.xMin;
		metrics->height       = cbox.yMax - cbox.yMin;
		metrics->horiBearingX = cbox.xMin;
		metrics->horiBearingY = cbox.yMax - metrics->height;
	}

Exit:
	return error;
}


/*************************************************************************/
/*************************************************************************/
/*****                                                               *****/
/*****                      KERNING METHOD                           *****/
/*****                                                               *****/
/*************************************************************************/
/*************************************************************************/

FT2_1_3_LOCAL_DEF( FT2_1_3_Error )
pfr_face_get_kerning( PFR_Face    face,
					  FT2_1_3_UInt     glyph1,
					  FT2_1_3_UInt     glyph2,
					  FT2_1_3_Vector*  kerning ) {
	FT2_1_3_Error      error;
	PFR_PhyFont   phy_font = &face->phy_font;
	PFR_KernItem  item     = phy_font->kern_items;
	FT2_1_3_UInt32     idx      = PFR_KERN_INDEX( glyph1, glyph2 );


	kerning->x = 0;
	kerning->y = 0;

	/* find the kerning item containing our pair */
	while ( item ) {
		if ( item->pair1 <= idx && idx <= item->pair2 )
			goto Found_Item;

		item = item->next;
	}

	/* not found */
	goto Exit;

Found_Item: {
		/* perform simply binary search within the item */
		FT2_1_3_UInt    min, mid, max;
		FT2_1_3_Stream  stream = face->root.stream;
		FT2_1_3_Byte*   p;


		if ( FT2_1_3_STREAM_SEEK( item->offset )                       ||
				FT2_1_3_FRAME_ENTER( item->pair_count * item->pair_size ) )
			goto Exit;

		min = 0;
		max = item->pair_count;
		while ( min < max ) {
			FT2_1_3_UInt  char1, char2, charcode;


			mid = ( min + max ) >> 1;
			p   = stream->cursor + mid*item->pair_size;

			if ( item->flags & PFR_KERN_2BYTE_CHAR ) {
				char1 = FT2_1_3_NEXT_USHORT( p );
				char2 = FT2_1_3_NEXT_USHORT( p );
			} else {
				char1 = FT2_1_3_NEXT_USHORT( p );
				char2 = FT2_1_3_NEXT_USHORT( p );
			}
			charcode = PFR_KERN_INDEX( char1, char2 );

			if ( idx == charcode ) {
				if ( item->flags & PFR_KERN_2BYTE_ADJ )
					kerning->x = item->base_adj + FT2_1_3_NEXT_SHORT( p );
				else
					kerning->x = item->base_adj + FT2_1_3_NEXT_CHAR( p );

				break;
			}
			if ( idx > charcode )
				min = mid + 1;
			else
				max = mid;
		}

		FT2_1_3_FRAME_EXIT();
	}

Exit:
	return 0;
}

} // End of namespace FreeType213
} // End of namespace AGS3

/* END */
