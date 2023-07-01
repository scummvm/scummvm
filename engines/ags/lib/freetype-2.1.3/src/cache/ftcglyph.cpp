/***************************************************************************/
/*                                                                         */
/*  ftcglyph.c                                                             */
/*                                                                         */
/*    FreeType Glyph Image (FT2_1_3_Glyph) cache (body).                        */
/*                                                                         */
/*  Copyright 2000-2001 by                                                 */
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
#include "engines/ags/lib/freetype-2.1.3/include/freetype/ftcache.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/cache/ftcglyph.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/fterrors.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/ftlist.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftobjs.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftdebug.h"

#include "ftcerror.h"


/* create a new chunk node, setting its cache index and ref count */
FT2_1_3_EXPORT_DEF( void )
ftc_glyph_node_init( FTC_GlyphNode     gnode,
                     FT2_1_3_UInt           gindex,
                     FTC_GlyphFamily   gfam ) {
	FT2_1_3_UInt  len;
	FT2_1_3_UInt  start = FTC_GLYPH_FAMILY_START( gfam, gindex );


	gnode->item_start = (FT2_1_3_UShort)start;

	len = gfam->item_total - start;
	if ( len > gfam->item_count )
		len = gfam->item_count;

	gnode->item_count = (FT2_1_3_UShort)len;
	gfam->family.num_nodes++;
}


FT2_1_3_EXPORT_DEF( void )
ftc_glyph_node_done( FTC_GlyphNode  gnode,
                     FTC_Cache      cache ) {
	/* finalize the node */
	gnode->item_count = 0;
	gnode->item_start = 0;

	ftc_node_done( FTC_NODE( gnode ), cache );
}


FT2_1_3_EXPORT_DEF( FT2_1_3_Bool )
ftc_glyph_node_compare( FTC_GlyphNode   gnode,
                        FTC_GlyphQuery  gquery ) {
	FT2_1_3_UInt  start = (FT2_1_3_UInt)gnode->item_start;
	FT2_1_3_UInt  count = (FT2_1_3_UInt)gnode->item_count;

	return FT2_1_3_BOOL( (FT2_1_3_UInt)( gquery->gindex - start ) < count );
}


/*************************************************************************/
/*************************************************************************/
/*****                                                               *****/
/*****                      CHUNK SETS                               *****/
/*****                                                               *****/
/*************************************************************************/
/*************************************************************************/


FT2_1_3_EXPORT_DEF( FT2_1_3_Error )
ftc_glyph_family_init( FTC_GlyphFamily  gfam,
                       FT2_1_3_UInt32        hash,
                       FT2_1_3_UInt          item_count,
                       FT2_1_3_UInt          item_total,
                       FTC_GlyphQuery   gquery,
                       FTC_Cache        cache ) {
	FT2_1_3_Error  error;


	error = ftc_family_init( FTC_FAMILY( gfam ), FTC_QUERY( gquery ), cache );
	if ( !error ) {
		gfam->hash       = hash;
		gfam->item_total = item_total;
		gfam->item_count = item_count;

		FTC_GLYPH_FAMILY_FOUND( gfam, gquery );
	}

	return error;
}


FT2_1_3_EXPORT_DEF( void )
ftc_glyph_family_done( FTC_GlyphFamily  gfam ) {
	ftc_family_done( FTC_FAMILY( gfam ) );
}


/* END */
