/***************************************************************************/
/*                                                                         */
/*  ftoutln.c                                                              */
/*                                                                         */
/*    FreeType outline management (body).                                  */
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


/*************************************************************************/
/*                                                                       */
/* All functions are declared in freetype.h.                             */
/*                                                                       */
/*************************************************************************/


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/ftoutln.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftobjs.h"


/*************************************************************************/
/*                                                                       */
/* The macro FT2_1_3_COMPONENT is used in trace mode.  It is an implicit      */
/* parameter of the FT2_1_3_TRACE() and FT2_1_3_ERROR() macros, used to print/log  */
/* messages during execution.                                            */
/*                                                                       */
#undef  FT2_1_3_COMPONENT
#define FT2_1_3_COMPONENT  trace_outline

namespace AGS3 {
namespace FreeType213 {

static
const FT2_1_3_Outline  null_outline = { 0, 0, 0, 0, 0, 0 };


/* documentation is in ftoutln.h */

FT2_1_3_EXPORT_DEF( FT2_1_3_Error )
FT2_1_3_Outline_Decompose( FT2_1_3_Outline*              outline,
					  const FT2_1_3_Outline_Funcs*  func_interface,
					  void*                    user ) {
#undef SCALED
#define SCALED( x )  ( ( (x) << shift ) - delta )

	FT2_1_3_Vector   v_last;
	FT2_1_3_Vector   v_control;
	FT2_1_3_Vector   v_start;

	FT2_1_3_Vector*  point;
	FT2_1_3_Vector*  limit;
	char*       tags;

	FT2_1_3_Error    error;

	FT2_1_3_Int   n;         /* index of contour in outline     */
	FT2_1_3_UInt  first;     /* index of first point in contour */
	FT2_1_3_Int   tag;       /* current point's state           */

	FT2_1_3_Int   shift;
	FT2_1_3_Pos   delta;


	if ( !outline || !func_interface )
		return FT2_1_3_Err_Invalid_Argument;

	shift = func_interface->shift;
	delta = func_interface->delta;
	first = 0;

	for ( n = 0; n < outline->n_contours; n++ ) {
		FT2_1_3_Int  last;  /* index of last point in contour */


		last  = outline->contours[n];
		limit = outline->points + last;

		v_start = outline->points[first];
		v_last  = outline->points[last];

		v_start.x = SCALED( v_start.x );
		v_start.y = SCALED( v_start.y );
		v_last.x  = SCALED( v_last.x );
		v_last.y  = SCALED( v_last.y );

		v_control = v_start;

		point = outline->points + first;
		tags  = outline->tags  + first;
		tag   = FT2_1_3_CURVE_TAG( tags[0] );

		/* A contour cannot start with a cubic control point! */
		if ( tag == FT2_1_3_CURVE_TAG_CUBIC )
			goto Invalid_Outline;

		/* check first point to determine origin */
		if ( tag == FT2_1_3_CURVE_TAG_CONIC ) {
			/* first point is conic control.  Yes, this happens. */
			if ( FT2_1_3_CURVE_TAG( outline->tags[last] ) == FT2_1_3_CURVE_TAG_ON ) {
				/* start at last point if it is on the curve */
				v_start = v_last;
				limit--;
			} else {
				/* if both first and last points are conic,         */
				/* start at their middle and record its position    */
				/* for closure                                      */
				v_start.x = ( v_start.x + v_last.x ) / 2;
				v_start.y = ( v_start.y + v_last.y ) / 2;

				v_last = v_start;
			}
			point--;
			tags--;
		}

		error = func_interface->move_to( &v_start, user );
		if ( error )
			goto Exit;

		while ( point < limit ) {
			point++;
			tags++;

			tag = FT2_1_3_CURVE_TAG( tags[0] );
			switch ( tag ) {
			case FT2_1_3_CURVE_TAG_ON: { /* emit a single line_to */
				FT2_1_3_Vector  vec;


				vec.x = SCALED( point->x );
				vec.y = SCALED( point->y );

				error = func_interface->line_to( &vec, user );
				if ( error )
					goto Exit;
				continue;
			}

			case FT2_1_3_CURVE_TAG_CONIC:  /* consume conic arcs */
				v_control.x = SCALED( point->x );
				v_control.y = SCALED( point->y );

Do_Conic:
				if ( point < limit ) {
					FT2_1_3_Vector  vec;
					FT2_1_3_Vector  v_middle;


					point++;
					tags++;
					tag = FT2_1_3_CURVE_TAG( tags[0] );

					vec.x = SCALED( point->x );
					vec.y = SCALED( point->y );

					if ( tag == FT2_1_3_CURVE_TAG_ON ) {
						error = func_interface->conic_to( &v_control, &vec, user );
						if ( error )
							goto Exit;
						continue;
					}

					if ( tag != FT2_1_3_CURVE_TAG_CONIC )
						goto Invalid_Outline;

					v_middle.x = ( v_control.x + vec.x ) / 2;
					v_middle.y = ( v_control.y + vec.y ) / 2;

					error = func_interface->conic_to( &v_control, &v_middle, user );
					if ( error )
						goto Exit;

					v_control = vec;
					goto Do_Conic;
				}

				error = func_interface->conic_to( &v_control, &v_start, user );
				goto Close;

			default: { /* FT2_1_3_CURVE_TAG_CUBIC */
				FT2_1_3_Vector  vec1, vec2;


				if ( point + 1 > limit                             ||
						FT2_1_3_CURVE_TAG( tags[1] ) != FT2_1_3_CURVE_TAG_CUBIC )
					goto Invalid_Outline;

				point += 2;
				tags  += 2;

				vec1.x = SCALED( point[-2].x );
				vec1.y = SCALED( point[-2].y );
				vec2.x = SCALED( point[-1].x );
				vec2.y = SCALED( point[-1].y );

				if ( point <= limit ) {
					FT2_1_3_Vector  vec;


					vec.x = SCALED( point->x );
					vec.y = SCALED( point->y );

					error = func_interface->cubic_to( &vec1, &vec2, &vec, user );
					if ( error )
						goto Exit;
					continue;
				}

				error = func_interface->cubic_to( &vec1, &vec2, &v_start, user );
				goto Close;
			}
			}
		}

		/* close the contour with a line segment */
		error = func_interface->line_to( &v_start, user );

Close:
		if ( error )
			goto Exit;

		first = last + 1;
	}

	return 0;

Exit:
	return error;

Invalid_Outline:
	return FT2_1_3_Err_Invalid_Outline;
}


FT2_1_3_EXPORT_DEF( FT2_1_3_Error )
FT2_1_3_Outline_New_Internal( FT2_1_3_Memory    memory,
						 FT2_1_3_UInt      numPoints,
						 FT2_1_3_Int       numContours,
						 FT2_1_3_Outline  *anoutline ) {
	FT2_1_3_Error  error;


	if ( !anoutline || !memory )
		return FT2_1_3_Err_Invalid_Argument;

	*anoutline = null_outline;

	if ( FT2_1_3_NEW_ARRAY( anoutline->points,   numPoints * 2L ) ||
			FT2_1_3_NEW_ARRAY( anoutline->tags,     numPoints      ) ||
			FT2_1_3_NEW_ARRAY( anoutline->contours, numContours    ) )
		goto Fail;

	anoutline->n_points    = (FT2_1_3_UShort)numPoints;
	anoutline->n_contours  = (FT2_1_3_Short)numContours;
	anoutline->flags      |= FT2_1_3_OUTLINE_OWNER;

	return FT2_1_3_Err_Ok;

Fail:
	anoutline->flags |= FT2_1_3_OUTLINE_OWNER;
	FT2_1_3_Outline_Done_Internal( memory, anoutline );

	return error;
}


/* documentation is in ftoutln.h */

FT2_1_3_EXPORT_DEF( FT2_1_3_Error )
FT2_1_3_Outline_New( FT2_1_3_Library   library,
				FT2_1_3_UInt      numPoints,
				FT2_1_3_Int       numContours,
				FT2_1_3_Outline  *anoutline ) {
	if ( !library )
		return FT2_1_3_Err_Invalid_Library_Handle;

	return FT2_1_3_Outline_New_Internal( library->memory, numPoints,
									numContours, anoutline );
}


/* documentation is in ftoutln.h */

FT2_1_3_EXPORT_DEF( FT2_1_3_Error )
FT2_1_3_Outline_Check( FT2_1_3_Outline*  outline ) {
	if ( outline ) {
		FT2_1_3_Int  n_points   = outline->n_points;
		FT2_1_3_Int  n_contours = outline->n_contours;
		FT2_1_3_Int  end0, end;
		FT2_1_3_Int  n;


		/* empty glyph? */
		if ( n_points == 0 && n_contours == 0 )
			return 0;

		/* check point and contour counts */
		if ( n_points <= 0 || n_contours <= 0 )
			goto Bad;

		end0 = end = -1;
		for ( n = 0; n < n_contours; n++ ) {
			end = outline->contours[n];

			/* note that we don't accept empty contours */
			if ( end <= end0 || end >= n_points )
				goto Bad;

			end0 = end;
		}

		if ( end != n_points - 1 )
			goto Bad;

		/* XXX: check the tags array */
		return 0;
	}

Bad:
	return FT2_1_3_Err_Invalid_Argument;
}


/* documentation is in ftoutln.h */

FT2_1_3_EXPORT_DEF( FT2_1_3_Error )
FT2_1_3_Outline_Copy( FT2_1_3_Outline*  source,
				 FT2_1_3_Outline  *target ) {
	FT2_1_3_Int  is_owner;


	if ( !source            || !target            ||
			source->n_points   != target->n_points   ||
			source->n_contours != target->n_contours )
		return FT2_1_3_Err_Invalid_Argument;

	FT2_1_3_MEM_COPY( target->points, source->points,
				 source->n_points * sizeof ( FT2_1_3_Vector ) );

	FT2_1_3_MEM_COPY( target->tags, source->tags,
				 source->n_points * sizeof ( FT2_1_3_Byte ) );

	FT2_1_3_MEM_COPY( target->contours, source->contours,
				 source->n_contours * sizeof ( FT2_1_3_Short ) );

	/* copy all flags, except the `FT2_1_3_OUTLINE_OWNER' one */
	is_owner      = target->flags & FT2_1_3_OUTLINE_OWNER;
	target->flags = source->flags;

	target->flags &= ~FT2_1_3_OUTLINE_OWNER;
	target->flags |= is_owner;

	return FT2_1_3_Err_Ok;
}


FT2_1_3_EXPORT_DEF( FT2_1_3_Error )
FT2_1_3_Outline_Done_Internal( FT2_1_3_Memory    memory,
						  FT2_1_3_Outline*  outline ) {
	if ( outline ) {
		if ( outline->flags & FT2_1_3_OUTLINE_OWNER ) {
			FT2_1_3_FREE( outline->points   );
			FT2_1_3_FREE( outline->tags     );
			FT2_1_3_FREE( outline->contours );
		}
		*outline = null_outline;

		return FT2_1_3_Err_Ok;
	} else
		return FT2_1_3_Err_Invalid_Argument;
}


/* documentation is in ftoutln.h */

FT2_1_3_EXPORT_DEF( FT2_1_3_Error )
FT2_1_3_Outline_Done( FT2_1_3_Library   library,
				 FT2_1_3_Outline*  outline ) {
	/* check for valid `outline' in FT2_1_3_Outline_Done_Internal() */

	if ( !library )
		return FT2_1_3_Err_Invalid_Library_Handle;

	return FT2_1_3_Outline_Done_Internal( library->memory, outline );
}


/* documentation is in ftoutln.h */

FT2_1_3_EXPORT_DEF( void )
FT2_1_3_Outline_Get_CBox( FT2_1_3_Outline*  outline,
					 FT2_1_3_BBox     *acbox ) {
	FT2_1_3_Pos  xMin, yMin, xMax, yMax;


	if ( outline && acbox ) {
		if ( outline->n_points == 0 ) {
			xMin = 0;
			yMin = 0;
			xMax = 0;
			yMax = 0;
		} else {
			FT2_1_3_Vector*  vec   = outline->points;
			FT2_1_3_Vector*  limit = vec + outline->n_points;


			xMin = xMax = vec->x;
			yMin = yMax = vec->y;
			vec++;

			for ( ; vec < limit; vec++ ) {
				FT2_1_3_Pos  x, y;


				x = vec->x;
				if ( x < xMin ) xMin = x;
				if ( x > xMax ) xMax = x;

				y = vec->y;
				if ( y < yMin ) yMin = y;
				if ( y > yMax ) yMax = y;
			}
		}
		acbox->xMin = xMin;
		acbox->xMax = xMax;
		acbox->yMin = yMin;
		acbox->yMax = yMax;
	}
}


/* documentation is in ftoutln.h */

FT2_1_3_EXPORT_DEF( void )
FT2_1_3_Outline_Translate( FT2_1_3_Outline*  outline,
					  FT2_1_3_Pos       xOffset,
					  FT2_1_3_Pos       yOffset ) {
	FT2_1_3_UShort   n;
	FT2_1_3_Vector*  vec = outline->points;


	for ( n = 0; n < outline->n_points; n++ ) {
		vec->x += xOffset;
		vec->y += yOffset;
		vec++;
	}
}


/* documentation is in ftoutln.h */

FT2_1_3_EXPORT_DEF( void )
FT2_1_3_Outline_Reverse( FT2_1_3_Outline*  outline ) {
	FT2_1_3_UShort  n;
	FT2_1_3_Int     first, last;


	first = 0;

	for ( n = 0; n < outline->n_contours; n++ ) {
		last  = outline->contours[n];

		/* reverse point table */
		{
			FT2_1_3_Vector*  p = outline->points + first;
			FT2_1_3_Vector*  q = outline->points + last;
			FT2_1_3_Vector   swap;


			while ( p < q ) {
				swap = *p;
				*p   = *q;
				*q   = swap;
				p++;
				q--;
			}
		}

		/* reverse tags table */
		{
			char*  p = outline->tags + first;
			char*  q = outline->tags + last;
			char   swap;


			while ( p < q ) {
				swap = *p;
				*p   = *q;
				*q   = swap;
				p++;
				q--;
			}
		}

		first = last + 1;
	}

	outline->flags ^= FT2_1_3_OUTLINE_REVERSE_FILL;
}


/* documentation is in ftoutln.h */

FT2_1_3_EXPORT_DEF( FT2_1_3_Error )
FT2_1_3_Outline_Render( FT2_1_3_Library         library,
				   FT2_1_3_Outline*        outline,
				   FT2_1_3_Raster_Params*  params ) {
	FT2_1_3_Error     error;
	FT2_1_3_Bool      update = 0;
	FT2_1_3_Renderer  renderer;
	FT2_1_3_ListNode  node;


	if ( !library )
		return FT2_1_3_Err_Invalid_Library_Handle;

	if ( !params )
		return FT2_1_3_Err_Invalid_Argument;

	renderer = library->cur_renderer;
	node     = library->renderers.head;

	params->source = (void*)outline;

	error = FT2_1_3_Err_Cannot_Render_Glyph;
	while ( renderer ) {
		error = renderer->raster_render( renderer->raster, params );
		if ( !error || FT2_1_3_ERROR_BASE( error ) != FT2_1_3_Err_Cannot_Render_Glyph )
			break;

		/* FT2_1_3_Err_Cannot_Render_Glyph is returned if the render mode   */
		/* is unsupported by the current renderer for this glyph image */
		/* format                                                      */

		/* now, look for another renderer that supports the same */
		/* format                                                */
		renderer = FT2_1_3_Lookup_Renderer( library, FT2_1_3_GLYPH_FORMAT_OUTLINE,
									   &node );
		update   = 1;
	}

	/* if we changed the current renderer for the glyph image format */
	/* we need to select it as the next current one                  */
	if ( !error && update && renderer )
		FT2_1_3_Set_Renderer( library, renderer, 0, 0 );

	return error;
}


/* documentation is in ftoutln.h */

FT2_1_3_EXPORT_DEF( FT2_1_3_Error )
FT2_1_3_Outline_Get_Bitmap( FT2_1_3_Library   library,
					   FT2_1_3_Outline*  outline,
					   FT2_1_3_Bitmap   *abitmap ) {
	FT2_1_3_Raster_Params  params;


	if ( !abitmap )
		return FT2_1_3_Err_Invalid_Argument;

	/* other checks are delayed to FT2_1_3_Outline_Render() */

	params.target = abitmap;
	params.flags  = 0;

	if ( abitmap->pixel_mode == FT2_1_3_PIXEL_MODE_GRAY  ||
			abitmap->pixel_mode == FT2_1_3_PIXEL_MODE_LCD   ||
			abitmap->pixel_mode == FT2_1_3_PIXEL_MODE_LCD_V )
		params.flags |= FT2_1_3_RASTER_FLAG_AA;

	return FT2_1_3_Outline_Render( library, outline, &params );
}


/* documentation is in ftoutln.h */

FT2_1_3_EXPORT_DEF( void )
FT2_1_3_Vector_Transform( FT2_1_3_Vector*  vector,
					 FT2_1_3_Matrix*  matrix ) {
	FT2_1_3_Pos xz, yz;


	if ( !vector || !matrix )
		return;

	xz = FT2_1_3_MulFix( vector->x, matrix->xx ) +
		 FT2_1_3_MulFix( vector->y, matrix->xy );

	yz = FT2_1_3_MulFix( vector->x, matrix->yx ) +
		 FT2_1_3_MulFix( vector->y, matrix->yy );

	vector->x = xz;
	vector->y = yz;
}


/* documentation is in ftoutln.h */

FT2_1_3_EXPORT_DEF( void )
FT2_1_3_Outline_Transform( FT2_1_3_Outline*  outline,
					  FT2_1_3_Matrix*   matrix ) {
	FT2_1_3_Vector*  vec = outline->points;
	FT2_1_3_Vector*  limit = vec + outline->n_points;


	for ( ; vec < limit; vec++ )
		FT2_1_3_Vector_Transform( vec, matrix );
}

} // End of namespace FreeType213
} // End of namespace AGS3

/* END */
