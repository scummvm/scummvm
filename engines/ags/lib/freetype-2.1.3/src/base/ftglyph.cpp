/***************************************************************************/
/*                                                                         */
/*  ftglyph.c                                                              */
/*                                                                         */
/*    FreeType convenience functions to handle glyphs (body).              */
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
/*  This file contains the definition of several convenience functions   */
/*  that can be used by client applications to easily retrieve glyph     */
/*  bitmaps and outlines from a given face.                              */
/*                                                                       */
/*  These functions should be optional if you are writing a font server  */
/*  or text layout engine on top of FreeType.  However, they are pretty  */
/*  handy for many other simple uses of the library.                     */
/*                                                                       */
/*************************************************************************/


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include FT2_1_3_GLYPH_H
#include FT2_1_3_OUTLINE_H
#include FT2_1_3_INTERNAL_OBJECTS_H


/*************************************************************************/
/*                                                                       */
/* The macro FT2_1_3_COMPONENT is used in trace mode.  It is an implicit      */
/* parameter of the FT2_1_3_TRACE() and FT2_1_3_ERROR() macros, used to print/log  */
/* messages during execution.                                            */
/*                                                                       */
#undef  FT2_1_3_COMPONENT
#define FT2_1_3_COMPONENT  trace_glyph


/*************************************************************************/
/*************************************************************************/
/****                                                                 ****/
/****   Convenience functions                                         ****/
/****                                                                 ****/
/*************************************************************************/
/*************************************************************************/


/* documentation is in ftglyph.h */

FT2_1_3_EXPORT_DEF( void )
FT2_1_3_Matrix_Multiply( FT2_1_3_Matrix*  a,
                    FT2_1_3_Matrix*  b ) {
	FT2_1_3_Fixed  xx, xy, yx, yy;


	if ( !a || !b )
		return;

	xx = FT2_1_3_MulFix( a->xx, b->xx ) + FT2_1_3_MulFix( a->xy, b->yx );
	xy = FT2_1_3_MulFix( a->xx, b->xy ) + FT2_1_3_MulFix( a->xy, b->yy );
	yx = FT2_1_3_MulFix( a->yx, b->xx ) + FT2_1_3_MulFix( a->yy, b->yx );
	yy = FT2_1_3_MulFix( a->yx, b->xy ) + FT2_1_3_MulFix( a->yy, b->yy );

	b->xx = xx;
	b->xy = xy;
	b->yx = yx;
	b->yy = yy;
}


/* documentation is in ftglyph.h */

FT2_1_3_EXPORT_DEF( FT2_1_3_Error )
FT2_1_3_Matrix_Invert( FT2_1_3_Matrix*  matrix ) {
	FT2_1_3_Pos  delta, xx, yy;


	if ( !matrix )
		return FT2_1_3_Err_Invalid_Argument;

	/* compute discriminant */
	delta = FT2_1_3_MulFix( matrix->xx, matrix->yy ) -
	        FT2_1_3_MulFix( matrix->xy, matrix->yx );

	if ( !delta )
		return FT2_1_3_Err_Invalid_Argument;  /* matrix can't be inverted */

	matrix->xy = - FT2_1_3_DivFix( matrix->xy, delta );
	matrix->yx = - FT2_1_3_DivFix( matrix->yx, delta );

	xx = matrix->xx;
	yy = matrix->yy;

	matrix->xx = FT2_1_3_DivFix( yy, delta );
	matrix->yy = FT2_1_3_DivFix( xx, delta );

	return FT2_1_3_Err_Ok;
}


/*************************************************************************/
/*************************************************************************/
/****                                                                 ****/
/****   FT2_1_3_BitmapGlyph support                                        ****/
/****                                                                 ****/
/*************************************************************************/
/*************************************************************************/

static FT2_1_3_Error
ft_bitmap_copy( FT2_1_3_Memory   memory,
                FT2_1_3_Bitmap*  source,
                FT2_1_3_Bitmap*  target ) {
	FT2_1_3_Error  error;
	FT2_1_3_Int    pitch = source->pitch;
	FT2_1_3_ULong  size;


	*target = *source;

	if ( pitch < 0 )
		pitch = -pitch;

	size = (FT2_1_3_ULong)( pitch * source->rows );

	if ( !FT2_1_3_ALLOC( target->buffer, size ) )
		FT2_1_3_MEM_COPY( target->buffer, source->buffer, size );

	return error;
}


static FT2_1_3_Error
ft_bitmap_glyph_init( FT2_1_3_BitmapGlyph  glyph,
                      FT2_1_3_GlyphSlot    slot ) {
	FT2_1_3_Error    error   = FT2_1_3_Err_Ok;
	FT2_1_3_Library  library = FT2_1_3_GLYPH(glyph)->library;
	FT2_1_3_Memory   memory  = library->memory;


	if ( slot->format != FT2_1_3_GLYPH_FORMAT_BITMAP ) {
		error = FT2_1_3_Err_Invalid_Glyph_Format;
		goto Exit;
	}

	/* grab the bitmap in the slot - do lazy copying whenever possible */
	glyph->bitmap = slot->bitmap;
	glyph->left   = slot->bitmap_left;
	glyph->top    = slot->bitmap_top;

	if ( slot->flags & FT2_1_3_GLYPH_OWN_BITMAP )
		slot->flags &= ~FT2_1_3_GLYPH_OWN_BITMAP;
	else {
		/* copy the bitmap into a new buffer */
		error = ft_bitmap_copy( memory, &slot->bitmap, &glyph->bitmap );
	}

Exit:
	return error;
}


static FT2_1_3_Error
ft_bitmap_glyph_copy( FT2_1_3_BitmapGlyph  source,
                      FT2_1_3_BitmapGlyph  target ) {
	FT2_1_3_Memory  memory = source->root.library->memory;


	target->left = source->left;
	target->top  = source->top;

	return ft_bitmap_copy( memory, &source->bitmap, &target->bitmap );
}


static void
ft_bitmap_glyph_done( FT2_1_3_BitmapGlyph  glyph ) {
	FT2_1_3_Memory  memory = FT2_1_3_GLYPH(glyph)->library->memory;


	FT2_1_3_FREE( glyph->bitmap.buffer );
}


static void
ft_bitmap_glyph_bbox( FT2_1_3_BitmapGlyph  glyph,
                      FT2_1_3_BBox*        cbox ) {
	cbox->xMin = glyph->left << 6;
	cbox->xMax = cbox->xMin + ( glyph->bitmap.width << 6 );
	cbox->yMax = glyph->top << 6;
	cbox->yMin = cbox->yMax - ( glyph->bitmap.rows << 6 );
}


const FT2_1_3_Glyph_Class  ft_bitmap_glyph_class = {
	sizeof( FT2_1_3_BitmapGlyphRec ),
	FT2_1_3_GLYPH_FORMAT_BITMAP,

	(FT2_1_3_Glyph_InitFunc)     ft_bitmap_glyph_init,
	(FT2_1_3_Glyph_DoneFunc)     ft_bitmap_glyph_done,
	(FT2_1_3_Glyph_CopyFunc)     ft_bitmap_glyph_copy,
	(FT2_1_3_Glyph_TransformFunc)0,
	(FT2_1_3_Glyph_GetBBoxFunc)  ft_bitmap_glyph_bbox,
	(FT2_1_3_Glyph_PrepareFunc)  0
};


/*************************************************************************/
/*************************************************************************/
/****                                                                 ****/
/****   FT2_1_3_OutlineGlyph support                                       ****/
/****                                                                 ****/
/*************************************************************************/
/*************************************************************************/


static FT2_1_3_Error
ft_outline_glyph_init( FT2_1_3_OutlineGlyph  glyph,
                       FT2_1_3_GlyphSlot     slot ) {
	FT2_1_3_Error     error   = FT2_1_3_Err_Ok;
	FT2_1_3_Library   library = FT2_1_3_GLYPH(glyph)->library;
	FT2_1_3_Outline*  source  = &slot->outline;
	FT2_1_3_Outline*  target  = &glyph->outline;


	/* check format in glyph slot */
	if ( slot->format != FT2_1_3_GLYPH_FORMAT_OUTLINE ) {
		error = FT2_1_3_Err_Invalid_Glyph_Format;
		goto Exit;
	}

	/* allocate new outline */
	error = FT2_1_3_Outline_New( library, source->n_points, source->n_contours,
	                        &glyph->outline );
	if ( error )
		goto Exit;

	/* copy it */
	FT2_1_3_MEM_COPY( target->points, source->points,
	             source->n_points * sizeof ( FT2_1_3_Vector ) );

	FT2_1_3_MEM_COPY( target->tags, source->tags,
	             source->n_points * sizeof ( FT2_1_3_Byte ) );

	FT2_1_3_MEM_COPY( target->contours, source->contours,
	             source->n_contours * sizeof ( FT2_1_3_Short ) );

	/* copy all flags, except the `FT2_1_3_OUTLINE_OWNER' one */
	target->flags = source->flags | FT2_1_3_OUTLINE_OWNER;

Exit:
	return error;
}


static void
ft_outline_glyph_done( FT2_1_3_OutlineGlyph  glyph ) {
	FT2_1_3_Outline_Done( FT2_1_3_GLYPH( glyph )->library, &glyph->outline );
}


static FT2_1_3_Error
ft_outline_glyph_copy( FT2_1_3_OutlineGlyph  source,
                       FT2_1_3_OutlineGlyph  target ) {
	FT2_1_3_Error    error;
	FT2_1_3_Library  library = FT2_1_3_GLYPH( source )->library;


	error = FT2_1_3_Outline_New( library, source->outline.n_points,
	                        source->outline.n_contours, &target->outline );
	if ( !error )
		FT2_1_3_Outline_Copy( &source->outline, &target->outline );

	return error;
}


static void
ft_outline_glyph_transform( FT2_1_3_OutlineGlyph  glyph,
                            FT2_1_3_Matrix*       matrix,
                            FT2_1_3_Vector*       delta ) {
	if ( matrix )
		FT2_1_3_Outline_Transform( &glyph->outline, matrix );

	if ( delta )
		FT2_1_3_Outline_Translate( &glyph->outline, delta->x, delta->y );
}


static void
ft_outline_glyph_bbox( FT2_1_3_OutlineGlyph  glyph,
                       FT2_1_3_BBox*         bbox ) {
	FT2_1_3_Outline_Get_CBox( &glyph->outline, bbox );
}


static FT2_1_3_Error
ft_outline_glyph_prepare( FT2_1_3_OutlineGlyph  glyph,
                          FT2_1_3_GlyphSlot     slot ) {
	slot->format         = FT2_1_3_GLYPH_FORMAT_OUTLINE;
	slot->outline        = glyph->outline;
	slot->outline.flags &= ~FT2_1_3_OUTLINE_OWNER;

	return FT2_1_3_Err_Ok;
}


const FT2_1_3_Glyph_Class  ft_outline_glyph_class = {
	sizeof( FT2_1_3_OutlineGlyphRec ),
	FT2_1_3_GLYPH_FORMAT_OUTLINE,

	(FT2_1_3_Glyph_InitFunc)     ft_outline_glyph_init,
	(FT2_1_3_Glyph_DoneFunc)     ft_outline_glyph_done,
	(FT2_1_3_Glyph_CopyFunc)     ft_outline_glyph_copy,
	(FT2_1_3_Glyph_TransformFunc)ft_outline_glyph_transform,
	(FT2_1_3_Glyph_GetBBoxFunc)  ft_outline_glyph_bbox,
	(FT2_1_3_Glyph_PrepareFunc)  ft_outline_glyph_prepare
};


/*************************************************************************/
/*************************************************************************/
/****                                                                 ****/
/****   FT2_1_3_Glyph class and API                                        ****/
/****                                                                 ****/
/*************************************************************************/
/*************************************************************************/

static FT2_1_3_Error
ft_new_glyph( FT2_1_3_Library             library,
              const FT2_1_3_Glyph_Class*  clazz,
              FT2_1_3_Glyph*              aglyph ) {
	FT2_1_3_Memory  memory = library->memory;
	FT2_1_3_Error   error;
	FT2_1_3_Glyph   glyph;


	*aglyph = 0;

	if ( !FT2_1_3_ALLOC( glyph, clazz->glyph_size ) ) {
		glyph->library = library;
		glyph->clazz   = clazz;
		glyph->format  = clazz->glyph_format;

		*aglyph = glyph;
	}

	return error;
}


/* documentation is in ftglyph.h */

FT2_1_3_EXPORT_DEF( FT2_1_3_Error )
FT2_1_3_Glyph_Copy( FT2_1_3_Glyph   source,
               FT2_1_3_Glyph  *target ) {
	FT2_1_3_Glyph               copy;
	FT2_1_3_Error               error;
	const FT2_1_3_Glyph_Class*  clazz;


	/* check arguments */
	if ( !target || !source || !source->clazz ) {
		error = FT2_1_3_Err_Invalid_Argument;
		goto Exit;
	}

	*target = 0;

	clazz = source->clazz;
	error = ft_new_glyph( source->library, clazz, &copy );
	if ( error )
		goto Exit;

	copy->advance = source->advance;
	copy->format  = source->format;

	if ( clazz->glyph_copy )
		error = clazz->glyph_copy( source, copy );

	if ( error )
		FT2_1_3_Done_Glyph( copy );
	else
		*target = copy;

Exit:
	return error;
}


/* documentation is in ftglyph.h */

FT2_1_3_EXPORT_DEF( FT2_1_3_Error )
FT2_1_3_Get_Glyph( FT2_1_3_GlyphSlot  slot,
              FT2_1_3_Glyph     *aglyph ) {
	FT2_1_3_Library  library = slot->library;
	FT2_1_3_Error    error;
	FT2_1_3_Glyph    glyph;

	const FT2_1_3_Glyph_Class*  clazz = 0;


	if ( !slot )
		return FT2_1_3_Err_Invalid_Slot_Handle;

	if ( !aglyph )
		return FT2_1_3_Err_Invalid_Argument;

	/* if it is a bitmap, that's easy :-) */
	if ( slot->format == FT2_1_3_GLYPH_FORMAT_BITMAP )
		clazz = &ft_bitmap_glyph_class;

	/* it it is an outline too */
	else if ( slot->format == FT2_1_3_GLYPH_FORMAT_OUTLINE )
		clazz = &ft_outline_glyph_class;

	else {
		/* try to find a renderer that supports the glyph image format */
		FT2_1_3_Renderer  render = FT2_1_3_Lookup_Renderer( library, slot->format, 0 );


		if ( render )
			clazz = &render->glyph_class;
	}

	if ( !clazz ) {
		error = FT2_1_3_Err_Invalid_Glyph_Format;
		goto Exit;
	}

	/* create FT2_1_3_Glyph object */
	error = ft_new_glyph( library, clazz, &glyph );
	if ( error )
		goto Exit;

	/* copy advance while converting it to 16.16 format */
	glyph->advance.x = slot->advance.x << 10;
	glyph->advance.y = slot->advance.y << 10;

	/* now import the image from the glyph slot */
	error = clazz->glyph_init( glyph, slot );

	/* if an error occurred, destroy the glyph */
	if ( error )
		FT2_1_3_Done_Glyph( glyph );
	else
		*aglyph = glyph;

Exit:
	return error;
}


/* documentation is in ftglyph.h */

FT2_1_3_EXPORT_DEF( FT2_1_3_Error )
FT2_1_3_Glyph_Transform( FT2_1_3_Glyph    glyph,
                    FT2_1_3_Matrix*  matrix,
                    FT2_1_3_Vector*  delta ) {
	const FT2_1_3_Glyph_Class*  clazz;
	FT2_1_3_Error               error = FT2_1_3_Err_Ok;


	if ( !glyph || !glyph->clazz )
		error = FT2_1_3_Err_Invalid_Argument;
	else {
		clazz = glyph->clazz;
		if ( clazz->glyph_transform ) {
			/* transform glyph image */
			clazz->glyph_transform( glyph, matrix, delta );

			/* transform advance vector */
			if ( matrix )
				FT2_1_3_Vector_Transform( &glyph->advance, matrix );
		} else
			error = FT2_1_3_Err_Invalid_Glyph_Format;
	}
	return error;
}


/* documentation is in ftglyph.h */

FT2_1_3_EXPORT_DEF( void )
FT2_1_3_Glyph_Get_CBox( FT2_1_3_Glyph  glyph,
                   FT2_1_3_UInt   bbox_mode,
                   FT2_1_3_BBox  *acbox ) {
	const FT2_1_3_Glyph_Class*  clazz;


	if ( !acbox )
		return;

	acbox->xMin = acbox->yMin = acbox->xMax = acbox->yMax = 0;

	if ( !glyph || !glyph->clazz )
		return;
	else {
		clazz = glyph->clazz;
		if ( !clazz->glyph_bbox )
			return;
		else {
			/* retrieve bbox in 26.6 coordinates */
			clazz->glyph_bbox( glyph, acbox );

			/* perform grid fitting if needed */
			if ( bbox_mode & ft_glyph_bbox_gridfit ) {
				acbox->xMin &= -64;
				acbox->yMin &= -64;
				acbox->xMax  = ( acbox->xMax + 63 ) & -64;
				acbox->yMax  = ( acbox->yMax + 63 ) & -64;
			}

			/* convert to integer pixels if needed */
			if ( bbox_mode & ft_glyph_bbox_truncate ) {
				acbox->xMin >>= 6;
				acbox->yMin >>= 6;
				acbox->xMax >>= 6;
				acbox->yMax >>= 6;
			}
		}
	}
	return;
}


/* documentation is in ftglyph.h */

FT2_1_3_EXPORT_DEF( FT2_1_3_Error )
FT2_1_3_Glyph_To_Bitmap( FT2_1_3_Glyph*       the_glyph,
                    FT2_1_3_Render_Mode  render_mode,
                    FT2_1_3_Vector*      origin,
                    FT2_1_3_Bool         destroy ) {
	FT2_1_3_GlyphSlotRec  dummy;
	FT2_1_3_Error         error = FT2_1_3_Err_Ok;
	FT2_1_3_Glyph         glyph;
	FT2_1_3_BitmapGlyph   bitmap = NULL;

	const FT2_1_3_Glyph_Class*  clazz;


	/* check argument */
	if ( !the_glyph )
		goto Bad;

	/* we render the glyph into a glyph bitmap using a `dummy' glyph slot */
	/* then calling FT2_1_3_Render_Glyph_Internal()                            */

	glyph = *the_glyph;
	if ( !glyph )
		goto Bad;

	clazz = glyph->clazz;

	/* when called with a bitmap glyph, do nothing and return succesfully */
	if ( clazz == &ft_bitmap_glyph_class )
		goto Exit;

	if ( !clazz || !clazz->glyph_prepare )
		goto Bad;

	FT2_1_3_MEM_ZERO( &dummy, sizeof ( dummy ) );
	dummy.library = glyph->library;
	dummy.format  = clazz->glyph_format;

	/* create result bitmap glyph */
	error = ft_new_glyph( glyph->library, &ft_bitmap_glyph_class,
	                      (FT2_1_3_Glyph*)&bitmap );
	if ( error )
		goto Exit;

#if 0
	/* if `origin' is set, translate the glyph image */
	if ( origin )
		FT2_1_3_Glyph_Transform( glyph, 0, origin );
#else
	FT2_1_3_UNUSED( origin );
#endif

	/* prepare dummy slot for rendering */
	error = clazz->glyph_prepare( glyph, &dummy );
	if ( !error )
		error = FT2_1_3_Render_Glyph_Internal( glyph->library, &dummy, render_mode );

#if 0
	if ( !destroy && origin ) {
		FT2_1_3_Vector  v;


		v.x = -origin->x;
		v.y = -origin->y;
		FT2_1_3_Glyph_Transform( glyph, 0, &v );
	}
#endif

	if ( error )
		goto Exit;

	/* in case of success, copy the bitmap to the glyph bitmap */
	error = ft_bitmap_glyph_init( bitmap, &dummy );
	if ( error )
		goto Exit;

	/* copy advance */
	bitmap->root.advance = glyph->advance;

	if ( destroy )
		FT2_1_3_Done_Glyph( glyph );

	*the_glyph = FT2_1_3_GLYPH( bitmap );

Exit:
	if ( error && bitmap )
		FT2_1_3_Done_Glyph( FT2_1_3_GLYPH( bitmap ) );

	return error;

Bad:
	error = FT2_1_3_Err_Invalid_Argument;
	goto Exit;
}


/* documentation is in ftglyph.h */

FT2_1_3_EXPORT_DEF( void )
FT2_1_3_Done_Glyph( FT2_1_3_Glyph  glyph ) {
	if ( glyph ) {
		FT2_1_3_Memory              memory = glyph->library->memory;
		const FT2_1_3_Glyph_Class*  clazz  = glyph->clazz;


		if ( clazz->glyph_done )
			clazz->glyph_done( glyph );

		FT2_1_3_FREE( glyph );
	}
}


/* END */
