/***************************************************************************/
/*                                                                         */
/*  ftgloadr.c                                                             */
/*                                                                         */
/*    The FreeType glyph loader (body).                                    */
/*                                                                         */
/*  Copyright 2002 by                                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg                       */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftgloadr.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftmemory.h"

#undef  FT2_1_3_COMPONENT
#define FT2_1_3_COMPONENT  trace_gloader


/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
/*****                                                               *****/
/*****                                                               *****/
/*****                    G L Y P H   L O A D E R                    *****/
/*****                                                               *****/
/*****                                                               *****/
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/

/*************************************************************************/
/*                                                                       */
/* The glyph loader is a simple object which is used to load a set of    */
/* glyphs easily.  It is critical for the correct loading of composites. */
/*                                                                       */
/* Ideally, one can see it as a stack of abstract `glyph' objects.       */
/*                                                                       */
/*   loader.base     Is really the bottom of the stack.  It describes a  */
/*                   single glyph image made of the juxtaposition of     */
/*                   several glyphs (those `in the stack').              */
/*                                                                       */
/*   loader.current  Describes the top of the stack, on which a new      */
/*                   glyph can be loaded.                                */
/*                                                                       */
/*   Rewind          Clears the stack.                                   */
/*   Prepare         Set up `loader.current' for addition of a new glyph */
/*                   image.                                              */
/*   Add             Add the `current' glyph image to the `base' one,    */
/*                   and prepare for another one.                        */
/*                                                                       */
/* The glyph loader is now a base object.  Each driver used to           */
/* re-implement it in one way or the other, which wasted code and        */
/* energy.                                                               */
/*                                                                       */
/*************************************************************************/

namespace AGS3 {
namespace FreeType213 {

/* create a new glyph loader */
FT2_1_3_BASE_DEF( FT_Error )
FT2_1_3_GlyphLoader_New( FT2_1_3_Memory        memory,
					FT2_1_3_GlyphLoader  *aloader ) {
	FT2_1_3_GlyphLoader  loader;
	FT_Error        error;


	if ( !FT2_1_3_NEW( loader ) ) {
		loader->memory = memory;
		*aloader       = loader;
	}
	return error;
}


/* rewind the glyph loader - reset counters to 0 */
FT2_1_3_BASE_DEF( void )
FT2_1_3_GlyphLoader_Rewind( FT2_1_3_GlyphLoader  loader ) {
	FT2_1_3_GlyphLoad  base    = &loader->base;
	FT2_1_3_GlyphLoad  current = &loader->current;


	base->outline.n_points   = 0;
	base->outline.n_contours = 0;
	base->num_subglyphs      = 0;

	*current = *base;
}


/* reset the glyph loader, frees all allocated tables */
/* and starts from zero                               */
FT2_1_3_BASE_DEF( void )
FT2_1_3_GlyphLoader_Reset( FT2_1_3_GlyphLoader  loader ) {
	FT2_1_3_Memory memory = loader->memory;


	FT2_1_3_FREE( loader->base.outline.points );
	FT2_1_3_FREE( loader->base.outline.tags );
	FT2_1_3_FREE( loader->base.outline.contours );
	FT2_1_3_FREE( loader->base.extra_points );
	FT2_1_3_FREE( loader->base.subglyphs );

	loader->max_points    = 0;
	loader->max_contours  = 0;
	loader->max_subglyphs = 0;

	FT2_1_3_GlyphLoader_Rewind( loader );
}


/* delete a glyph loader */
FT2_1_3_BASE_DEF( void )
FT2_1_3_GlyphLoader_Done( FT2_1_3_GlyphLoader  loader ) {
	if ( loader ) {
		FT2_1_3_Memory memory = loader->memory;


		FT2_1_3_GlyphLoader_Reset( loader );
		FT2_1_3_FREE( loader );
	}
}


/* re-adjust the `current' outline fields */
static void
FT2_1_3_GlyphLoader_Adjust_Points( FT2_1_3_GlyphLoader  loader ) {
	FT2_1_3_Outline*  base    = &loader->base.outline;
	FT2_1_3_Outline*  current = &loader->current.outline;


	current->points   = base->points   + base->n_points;
	current->tags     = base->tags     + base->n_points;
	current->contours = base->contours + base->n_contours;

	/* handle extra points table - if any */
	if ( loader->use_extra )
		loader->current.extra_points =
			loader->base.extra_points + base->n_points;
}


FT2_1_3_BASE_DEF( FT_Error )
FT2_1_3_GlyphLoader_CreateExtra( FT2_1_3_GlyphLoader  loader ) {
	FT_Error   error;
	FT2_1_3_Memory  memory = loader->memory;


	if ( !FT2_1_3_NEW_ARRAY( loader->base.extra_points, loader->max_points ) ) {
		loader->use_extra = 1;
		FT2_1_3_GlyphLoader_Adjust_Points( loader );
	}
	return error;
}


/* re-adjust the `current' subglyphs field */
static void
FT2_1_3_GlyphLoader_Adjust_Subglyphs( FT2_1_3_GlyphLoader  loader ) {
	FT2_1_3_GlyphLoad  base    = &loader->base;
	FT2_1_3_GlyphLoad  current = &loader->current;


	current->subglyphs = base->subglyphs + base->num_subglyphs;
}


/* Ensure that we can add `n_points' and `n_contours' to our glyph. this */
/* function reallocates its outline tables if necessary.  Note that it   */
/* DOESN'T change the number of points within the loader!                */
/*                                                                       */
FT2_1_3_BASE_DEF( FT_Error )
FT2_1_3_GlyphLoader_CheckPoints( FT2_1_3_GlyphLoader  loader,
							FT_UInt        n_points,
							FT_UInt        n_contours ) {
	FT2_1_3_Memory    memory  = loader->memory;
	FT_Error     error   = FT2_1_3_Err_Ok;
	FT2_1_3_Outline*  base    = &loader->base.outline;
	FT2_1_3_Outline*  current = &loader->current.outline;
	FT_Bool      adjust  = 1;

	FT_UInt      new_max, old_max;


	/* check points & tags */
	new_max = base->n_points + current->n_points + n_points;
	old_max = loader->max_points;

	if ( new_max > old_max ) {
		new_max = ( new_max + 7 ) & -8;

		if ( FT2_1_3_RENEW_ARRAY( base->points, old_max, new_max ) ||
				FT2_1_3_RENEW_ARRAY( base->tags,   old_max, new_max ) )
			goto Exit;

		if ( loader->use_extra &&
				FT2_1_3_RENEW_ARRAY( loader->base.extra_points, old_max, new_max ) )
			goto Exit;

		adjust = 1;
		loader->max_points = new_max;
	}

	/* check contours */
	old_max = loader->max_contours;
	new_max = base->n_contours + current->n_contours +
			  n_contours;
	if ( new_max > old_max ) {
		new_max = ( new_max + 3 ) & -4;
		if ( FT2_1_3_RENEW_ARRAY( base->contours, old_max, new_max ) )
			goto Exit;

		adjust = 1;
		loader->max_contours = new_max;
	}

	if ( adjust )
		FT2_1_3_GlyphLoader_Adjust_Points( loader );

Exit:
	return error;
}


/* Ensure that we can add `n_subglyphs' to our glyph. this function */
/* reallocates its subglyphs table if necessary.  Note that it DOES */
/* NOT change the number of subglyphs within the loader!            */
/*                                                                  */
FT2_1_3_BASE_DEF( FT_Error )
FT2_1_3_GlyphLoader_CheckSubGlyphs( FT2_1_3_GlyphLoader  loader,
							   FT_UInt         n_subs ) {
	FT2_1_3_Memory     memory = loader->memory;
	FT_Error      error  = FT2_1_3_Err_Ok;
	FT_UInt       new_max, old_max;

	FT2_1_3_GlyphLoad  base    = &loader->base;
	FT2_1_3_GlyphLoad  current = &loader->current;


	new_max = base->num_subglyphs + current->num_subglyphs + n_subs;
	old_max = loader->max_subglyphs;
	if ( new_max > old_max ) {
		new_max = ( new_max + 1 ) & -2;
		if ( FT2_1_3_RENEW_ARRAY( base->subglyphs, old_max, new_max ) )
			goto Exit;

		loader->max_subglyphs = new_max;

		FT2_1_3_GlyphLoader_Adjust_Subglyphs( loader );
	}

Exit:
	return error;
}


/* prepare loader for the addition of a new glyph on top of the base one */
FT2_1_3_BASE_DEF( void )
FT2_1_3_GlyphLoader_Prepare( FT2_1_3_GlyphLoader  loader ) {
	FT2_1_3_GlyphLoad  current = &loader->current;


	current->outline.n_points   = 0;
	current->outline.n_contours = 0;
	current->num_subglyphs      = 0;

	FT2_1_3_GlyphLoader_Adjust_Points   ( loader );
	FT2_1_3_GlyphLoader_Adjust_Subglyphs( loader );
}


/* add current glyph to the base image - and prepare for another */
FT2_1_3_BASE_DEF( void )
FT2_1_3_GlyphLoader_Add( FT2_1_3_GlyphLoader  loader ) {
	FT2_1_3_GlyphLoad  base    = &loader->base;
	FT2_1_3_GlyphLoad  current = &loader->current;

	FT_UInt       n_curr_contours = current->outline.n_contours;
	FT_UInt       n_base_points   = base->outline.n_points;
	FT_UInt       n;


	base->outline.n_points =
		(short)( base->outline.n_points + current->outline.n_points );
	base->outline.n_contours =
		(short)( base->outline.n_contours + current->outline.n_contours );

	base->num_subglyphs += current->num_subglyphs;

	/* adjust contours count in newest outline */
	for ( n = 0; n < n_curr_contours; n++ )
		current->outline.contours[n] =
			(short)( current->outline.contours[n] + n_base_points );

	/* prepare for another new glyph image */
	FT2_1_3_GlyphLoader_Prepare( loader );
}


FT2_1_3_BASE_DEF( FT_Error )
FT2_1_3_GlyphLoader_CopyPoints( FT2_1_3_GlyphLoader  target,
						   FT2_1_3_GlyphLoader  source ) {
	FT_Error  error;
	FT_UInt   num_points   = source->base.outline.n_points;
	FT_UInt   num_contours = source->base.outline.n_contours;


	error = FT2_1_3_GlyphLoader_CheckPoints( target, num_points, num_contours );
	if ( !error ) {
		FT2_1_3_Outline*  out = &target->base.outline;
		FT2_1_3_Outline*  in  = &source->base.outline;


		FT2_1_3_MEM_COPY( out->points, in->points,
					 num_points * sizeof ( FT_Vector ) );
		FT2_1_3_MEM_COPY( out->tags, in->tags,
					 num_points * sizeof ( char ) );
		FT2_1_3_MEM_COPY( out->contours, in->contours,
					 num_contours * sizeof ( short ) );

		/* do we need to copy the extra points? */
		if ( target->use_extra && source->use_extra )
			FT2_1_3_MEM_COPY( target->base.extra_points, source->base.extra_points,
						 num_points * sizeof ( FT_Vector ) );

		out->n_points   = (short)num_points;
		out->n_contours = (short)num_contours;

		FT2_1_3_GlyphLoader_Adjust_Points( target );
	}

	return error;
}

} // End of namespace FreeType213
} // End of namespace AGS3

/* END */
